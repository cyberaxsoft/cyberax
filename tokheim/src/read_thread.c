#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "tokheim.h"
#include "tokheim_func.h"
#include "config.h"
#include "driver_state.h"

gboolean 		read_thread_terminating = FALSE;
gboolean 		read_thread_terminated = FALSE;
GMutex			read_thread_mutex;

void init_read_thread_mutex()
{
	if (read_thread_mutex.p == NULL)
	{
		g_mutex_init(&read_thread_mutex);
	}
}

gboolean safe_get_read_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminating;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}

gboolean safe_get_read_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminated;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}

void safe_set_read_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminating = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void safe_set_read_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminated = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void interpret_fp_id(guint8* buffer, guint8 length, LogOptions log_options, guint8 disp_index)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d parse FP id: %02X", disp_index, buffer[0]);
	safe_set_fp_id(disp_index, buffer[0]);
}

void interpret_counters(guint8* buffer, guint8 length, LogOptions log_options, guint8 disp_index)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d Parse counters:", disp_index);

	if (safe_get_nozzle_count(disp_index) == 1)
	{
		guint32 counter = packed_bcd_to_bin(&buffer[0], SINGLE_COUNTER_LENGTH);
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Nozzle 1: %d", counter);
		safe_set_nozzle_counter(disp_index, 0, counter);
	}
	else
	{
		for (guint8 i = 0; i < 4; i++)
		{
			guint32 counter = packed_bcd_to_bin(&buffer[i * 10], MULT_COUNTER_LENGTH);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Nozzle %d: %d", i + 1, counter);
			safe_set_nozzle_counter(disp_index, i, counter);
		}
	}
}

void adjust_disp_finish(guint8 disp_index, LogOptions log_options)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d adjust disp finish", disp_index);
	if (safe_compare_disp_values(disp_index) || safe_get_reset(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d compare finish return true", disp_index);
		if (safe_get_is_pay(disp_index) || safe_get_reset(disp_index))
		{
			safe_disp_clear(disp_index);
			safe_set_exchange_state(disp_index,es_DeactivateNozzle);
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d wait payment", disp_index);
			safe_set_exchange_state(disp_index,es_WaitPayment);
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d wait payment", disp_index);
		safe_set_exchange_state(disp_index,es_WaitPayment);
	}
}


void interpret_status(guint8* buffer, guint8 length, LogOptions log_options, guint8 disp_index)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d parse status: %02X", disp_index, buffer[0]);

	switch(buffer[0])
	{
		case ts_Uninitialized:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Unitialized");
			break;

		case ts_Idle:
			if (safe_get_dispencer_state(disp_index) == ds_Stopped )
			{
				safe_set_exchange_state(disp_index, es_Resume);
			}
			else if (safe_get_dispencer_state(disp_index) == ds_Filling)
			{
				if (safe_get_disp_counters_enable(disp_index))
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetFillingInfo -> es_GetTotCounters", disp_index);
					safe_set_exchange_state(disp_index,es_GetTotCounters);
				}
				else
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state:  ds_Filling -> ds_Finish", disp_index);
					safe_set_dispencer_state(disp_index, ds_Finish);

					adjust_disp_finish(disp_index, log_options);
				}
			}
			else if (safe_get_dispencer_state(disp_index) == ds_Finish)
			{

			}
			else
			{
				if (safe_get_exchange_state(disp_index) > es_GetCounters)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Idle");
					safe_set_dispencer_state(disp_index, ds_Free);
					safe_set_active_nozzle_index(disp_index, -1);
				}
			}
			break;

		case ts_NozleOff:
			if (safe_get_dispencer_state(disp_index) != ds_NozzleOut)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state: ds_Free -> ds_NozzleOut", disp_index);
				safe_set_dispencer_state(disp_index, ds_NozzleOut);
			}
			safe_set_exchange_state(disp_index,es_GetNozzle);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  -> es_GetNozzle", disp_index);

			break;

		case ts_Authorized:
		case ts_Starting:
		case ts_Started:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Started");
			if (safe_get_dispencer_state(disp_index) == ds_Stopped || safe_get_dispencer_state(disp_index) == ds_NozzleOut )
			{
				safe_set_dispencer_state(disp_index, ds_Filling);
			}
			break;

		case ts_Terminating:
		case ts_Terminated:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Terminated");
			if (safe_get_dispencer_state(disp_index) > ds_Filling)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state: -> ds_Finish", disp_index);
				safe_set_dispencer_state(disp_index, ds_Finish);
			}
			break;

		case ts_Stopping:
		case ts_Stopped:
			if (safe_get_dispencer_state(disp_index) == ds_Filling)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state: ds_Filling -> ds_Stopped", disp_index);
				safe_set_dispencer_state(disp_index, ds_Stopped);
			}
			break;

		case ts_CommandCompleted:

			break;
	}
}

void interpret_nozzle(guint8* buffer, guint8 length, LogOptions log_options, guint8 disp_index)
{
	guint8 nozzle = buffer[0] & 0x7F;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Parse nozzle: %02X", nozzle);

	guint8 nozzle_index = 0;

	if (safe_get_nozzle_index_by_num(disp_index, nozzle, &nozzle_index) == de_NoError)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d set active nozzle index: %d", disp_index, nozzle_index);

		safe_set_active_nozzle_index(disp_index, nozzle_index);
		safe_set_exchange_state(disp_index,es_WaitPreset);
	}
	else
	{
		safe_set_exchange_state(disp_index,es_GetStatus);
	}

}
void interpret_filling_info(guint8* buffer, guint8 length, LogOptions log_options, guint8 disp_index)
{
	guint8 pos = 0;

	guint32 price = packed_bcd_to_bin(&buffer[pos], PRICE_FIELD_SIZE); pos+=PRICE_FIELD_SIZE;
	guint32 amount = packed_bcd_to_bin(&buffer[pos], AMOUNT_FIELD_SIZE);pos+=AMOUNT_FIELD_SIZE;
	guint32 volume = packed_bcd_to_bin(&buffer[pos], VOLUME_FIELD_SIZE);pos+=VOLUME_FIELD_SIZE;

	safe_set_current_volume(disp_index, volume);
	safe_set_current_price(disp_index, price);
	safe_set_current_amount(disp_index, amount);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d current values: price = %d, volume = %d, amount = %d", disp_index, price, volume, amount);

	interpret_status(&buffer[pos], FUELLING_POINT_STATUS_REPLY_LENGTH, log_options, disp_index);


}


void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");

	guint8 disp_index = safe_get_last_sended_disp_index();

	switch(safe_get_exchange_state(disp_index))
	{
		case es_Undefined:
			interpret_fp_id(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_Undefined -> es_GetStartStatus", disp_index);
			safe_set_exchange_state(disp_index,es_GetStartStatus);
			break;

		case es_GetStartStatus:
			interpret_status(buffer, length, log_options, disp_index);
			safe_set_exchange_state(disp_index,es_ClearDisplay);
			break;

		case es_ClearDisplay:
			if (safe_get_disp_counters_enable(disp_index))
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_ClearDisplay -> es_GetCounters (start)", disp_index);
				safe_set_exchange_state(disp_index,es_GetCounters);
			}
			else
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_ClearDisplay -> es_GetStatus", disp_index);
				safe_set_exchange_state(disp_index,es_DeactivateNozzle);
			}
			break;

		case es_GetCounters:
			interpret_counters(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetCounters -> es_GetStatus", disp_index);
			safe_set_exchange_state(disp_index,es_DeactivateNozzle);
			break;

		case es_DeactivateNozzle:
			interpret_status(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_DeactivateNozzle -> es_GetStatus", disp_index);
			safe_set_exchange_state(disp_index,es_GetStatus);
			break;

		case es_GetStatus:
			interpret_status(buffer, length, log_options, disp_index);
			if (safe_get_dispencer_state(disp_index) == ds_Free)
			{
				if (safe_get_send_prices(disp_index))
				{
					safe_set_send_prices(disp_index, FALSE);
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetStatus -> es_SendPrices", disp_index);
					safe_set_exchange_state(disp_index,es_SendPrices);
				}
			}
			break;


		case es_GetNozzle:
			interpret_nozzle(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetNozzle -> es_WaitPreset", disp_index);
			safe_set_exchange_state(disp_index,es_WaitPreset);
			break;

		case es_WaitPreset:
			interpret_status(buffer, length, log_options, disp_index);
			break;

		case es_SendPreset:
			interpret_status(buffer, length, log_options, disp_index);
			safe_copy_disp_order_type(disp_index);
			safe_set_exchange_state(disp_index,es_WaitStart);
			break;

		case es_WaitStart:
			interpret_status(buffer, length, log_options, disp_index);

			if (safe_get_dispencer_state(disp_index) == ds_Filling)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state: ds_NozzleOut -> ds_Filling", disp_index);
				safe_copy_disp_order_type(disp_index);
				safe_set_start_filling_filter(disp_index, START_FILLLING_FILTER_VALUE);

			}

			if (safe_get_start_filling_filter(disp_index) == 0)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_WaitStart -> es_GetFillingInfo", disp_index);
				safe_set_exchange_state(disp_index,es_GetFillingInfo);
			}
			break;

		case es_GetFillingInfo:
			interpret_filling_info(buffer, length, log_options, disp_index);

			if (safe_get_dispencer_state(disp_index) != ds_Finish)
			{
				if( safe_get_emergency_stop(disp_index))
				{
					safe_set_emergency_stop(disp_index, FALSE);
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Emergency stop! Disp index %d change exchange state:  es_GetFillingInfo -> es_Stopping", disp_index);
					safe_set_exchange_state(disp_index,es_Stopping);
				}
				else if (safe_get_suspend(disp_index))
				{
					safe_set_suspend(disp_index, FALSE);
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetFillingInfo -> es_Stopping", disp_index);
					safe_set_exchange_state(disp_index,es_Stopping);
				}
				else if (safe_get_resume(disp_index))
				{
					safe_set_resume(disp_index, FALSE);
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetFillingInfo -> es_Resume", disp_index);
					safe_set_exchange_state(disp_index,es_Resume);
				}
			}
			break;

		case es_Stopping:
			interpret_status(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_Stopping -> es_GetFillingInfo", disp_index);
			safe_set_exchange_state(disp_index,es_GetFillingInfo);
			break;

		case es_Resume:
			interpret_status(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_Resume -> es_GetFillingInfo", disp_index);
			if (safe_get_dispencer_state(disp_index) == ds_Stopped)
			{
				safe_set_exchange_state(disp_index,es_GetTotCounters);
			}
			else
			{
				safe_set_exchange_state(disp_index,es_GetFillingInfo);
			}
			break;

		case es_GetTotCounters:
			interpret_counters(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_GetTotCounters -> es_WaitPayment", disp_index);

			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change dispencer state:  ds_Filling -> ds_Finish", disp_index);
			safe_set_dispencer_state(disp_index, ds_Finish);

			adjust_disp_finish(disp_index, log_options);
			break;

		case es_WaitPayment:
			interpret_filling_info(buffer, length, log_options, disp_index);
			if (safe_get_is_pay(disp_index) || safe_get_reset(disp_index))
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_WaitPayment -> es_GetStatus", disp_index);
				safe_set_exchange_state(disp_index,es_DeactivateNozzle);
				safe_disp_clear(disp_index);
			}
			break;

		case es_SendPrices:
			interpret_status(buffer, length, log_options, disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Disp index %d change exchange state:  es_SendPrices -> es_GetStatus", disp_index);
			safe_set_exchange_state(disp_index,es_GetStatus);
			break;

	}
	safe_set_fault_reply_counter(disp_index,0);
	safe_set_command_sended(FALSE);
}


gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "START READ");

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
			//	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "<< %02X", buff[i]);
				switch(safe_get_rx_stage())
				{
					case rs_Undefined:
						length = 0;
						break;

					case rs_Value:
						buffer[length] = buff[i];
						safe_set_rx_stage(rs_Complement);
						break;

					case rs_Complement:
						if ((buffer[length] ^ 0xFF) == buff[i])
						{
							length++;

							if (length < safe_get_reply_length())
							{
								safe_set_rx_stage(rs_Value);
							}
							else
							{
								safe_set_rx_stage(rs_Value);
								interpret_reply(buffer, length, log_options);
								length = 0;
							}
						}
						else
						{
							safe_set_rx_stage(rs_Undefined);
						}
						break;
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}

