#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "topaz.h"
#include "config.h"
#include "topaz_func.h"
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

void interpret_pump_type(guint8* buffer, guint8 length, guint8 disp_index, guint8 nozzle_index, LogOptions log_options)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d type: %02X", disp_index, nozzle_index,buffer[ DATA_REPLY_OFFSET]);
	switch (buffer[DATA_REPLY_OFFSET])
	{
		case 0x3A:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 4, 6);
			break;

		case 0x3B:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 6, 8);
			break;

		case 0x3C:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 4, 6);
			break;

		case 0x3D:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 4, 6);
			break;

		case 0x3E:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 4, 6);
			break;

		case 0x3F:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 6, 8);
			break;

		case 0x40:
			safe_set_nozzle_param(disp_index, nozzle_index, 6, 6, 8);
			break;

		case 0x41:
			safe_set_nozzle_param(disp_index, nozzle_index, 5, 4, 7);
			break;

	}

}

void interpret_get_version(guint8* buffer, guint8 length, guint8 disp_index, guint8 nozzle_index, LogOptions log_options)
{
	guint32 version = 0;

	for (guint8 i = DATA_REPLY_OFFSET; i < DATA_REPLY_OFFSET + VERSION_LENGTH; i++)
	{
		version = (version * 10) + (buffer[i] - 0x30);
	}

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d version: %d", disp_index, nozzle_index, version);

	safe_set_nozzle_version(disp_index, nozzle_index, version);

}

void interpret_get_counter(guint8* buffer, guint8 length, guint8 disp_index, guint8 nozzle_index, LogOptions log_options)
{
	guint8 counter_length = (length - (DATA_REPLY_OFFSET + 1)) / 2;

	guint32 counter_value = 0;

	for (guint8 i = DATA_REPLY_OFFSET; i < DATA_REPLY_OFFSET + counter_length; i++)
	{
		counter_value = (counter_value * 10) + (buffer[i] - 0x30);
	}

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d counter: %d", disp_index, nozzle_index, counter_value);

	safe_set_nozzle_counter(disp_index, nozzle_index, counter_value);
}

void interpret_get_status(guint8* buffer, guint8 length, guint8 disp_index, guint8 nozzle_index, LogOptions log_options)
{
	switch (buffer[DATA_REPLY_OFFSET])
	{
		case ads_Free:
			if ((safe_get_active_nozzle_index(disp_index) == - 1) || safe_get_active_nozzle_index(disp_index) == nozzle_index || safe_get_dispencer_state(disp_index) == ds_Finish)
			{
				if (safe_get_channel_exchange_state(disp_index, nozzle_index) > ces_GetCounter)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set state ds_Free", disp_index, nozzle_index);
					safe_set_active_nozzle_index(disp_index, -1);
					safe_set_nozzle_was_on(disp_index, TRUE);
					safe_set_dispencer_state(disp_index, ds_Free);

				}
			}
			break;

		case ads_NozzleOff:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set state ds_NozzleOff", disp_index, nozzle_index);
			safe_set_active_nozzle_index(disp_index, nozzle_index);
			safe_set_dispencer_state(disp_index, ds_NozzleOut);
			break;

		case ads_FillingEnable:
			safe_set_active_nozzle_index(disp_index, nozzle_index);
			safe_copy_disp_order_type(disp_index);

			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d status filling enable", disp_index, nozzle_index);

			if (safe_get_channel_exchange_state(disp_index, nozzle_index) == ces_WaitStart)
			{
#ifdef TOPAZ_EXTENDED_START
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d change state to ConfirmStart", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ConfirmStart);
#endif
#ifdef TOPAZ_OLD_START
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d change state to ActiveFilling", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ActiveFilling);
#endif
			}
			break;

		case ads_Filling:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set state ds_Filling", disp_index, nozzle_index);
			safe_set_active_nozzle_index(disp_index, nozzle_index);
			safe_set_dispencer_state(disp_index, ds_Filling);
			break;

		case ads_Stoped:
			safe_set_active_nozzle_index(disp_index, nozzle_index);
			if (buffer[DATA_REPLY_OFFSET + 1] == 0x30)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set state ds_Finish", disp_index, nozzle_index);
				safe_set_dispencer_state(disp_index, ds_Finish);
			}
			else
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set state ds_Stopped", disp_index, nozzle_index);
				safe_set_dispencer_state(disp_index, ds_Stopped);
			}
			if (!safe_compare_disp_values(disp_index) && safe_get_is_pay(disp_index))
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set is pay to false", disp_index, nozzle_index);
				safe_set_is_pay(disp_index, FALSE);
			}
			break;

		case ads_BMU:
			//TODO
//			SafeSetActiveNozzleIndex(current_disp_index, current_nozzle_index, true);
//			SafeSetDispencerState(current_disp_index, disps_Busy);
			break;
	}

}

void interpret_filling_data(guint8* buffer, guint8 length, guint8 disp_index, guint8 nozzle_index, LogOptions log_options)
{

	guint8 volume_length = 0;
	guint8 price_length = 0;
	guint8 amount_length = 0;

	safe_get_nozzle_param(disp_index, nozzle_index, &volume_length, &price_length, &amount_length);

	guint8 pos = DATA_REPLY_OFFSET;

	guint32 volume = azt_unpacked_bcd_to_bin(&buffer[pos], volume_length); pos += volume_length;
	guint32 amount = azt_unpacked_bcd_to_bin(&buffer[pos], amount_length); pos += amount_length;
	guint32 price = azt_unpacked_bcd_to_bin(&buffer[pos], price_length); pos += price_length;

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d set filling data (p = %d, v = %d, a = %d)", disp_index, nozzle_index, price, volume, amount);


	safe_set_current_volume(disp_index, volume / 10);
	safe_set_current_amount(disp_index, amount);
	safe_set_current_price(disp_index, price);
}

void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	guint8 disp_index = 0;
	guint8 nozzle_index = 0;

	safe_restore_last_indexes(&disp_index, &nozzle_index);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d interpret reply", disp_index, nozzle_index);

	switch(safe_get_channel_exchange_state(disp_index, nozzle_index))
	{
		case ces_Undefined:
			interpret_pump_type(buffer, length, disp_index, nozzle_index, log_options);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d Undefined -> GetVersion", disp_index, nozzle_index);
			safe_set_channel_exchange_state(disp_index, nozzle_index, ces_GetVersion);
			break;

		case ces_GetVersion:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d GetVersion -> GetCounter", disp_index, nozzle_index);
			interpret_get_version(buffer, length, disp_index, nozzle_index, log_options);
			safe_set_channel_exchange_state(disp_index, nozzle_index, ces_GetCounter);
			break;

		case ces_GetCounter:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d GetCounter -> Idle", disp_index, nozzle_index);
			interpret_get_counter(buffer, length, disp_index, nozzle_index, log_options);
			safe_set_channel_exchange_state(disp_index, nozzle_index, ces_Idle);
			break;

		case ces_Idle:
			interpret_get_status(buffer, length, disp_index, nozzle_index, log_options);

			if (safe_get_dispencer_state(disp_index) == ds_Finish && safe_get_preset_order_type(disp_index) == ot_Free)
			{
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ConfirmPayment);
			}
			break;

		case ces_SendPrice:
			break;

		case ces_Preset:
			break;

		case ces_Start:
			if (safe_get_reset(disp_index))
			{
				safe_set_reset(disp_index, FALSE);
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d Start -> EmergencyStop", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_EmergencyStop);
			}

			interpret_get_status(buffer, length, disp_index, nozzle_index, log_options);
			break;

		case ces_WaitStart:
			interpret_get_status(buffer, length, disp_index, nozzle_index, log_options);
			break;

		case ces_ConfirmStart:
			break;

		case ces_ActiveFilling:
			interpret_get_status(buffer, length, disp_index, nozzle_index, log_options);
			safe_set_channel_exchange_state(disp_index, nozzle_index, ces_GetFillingData);
			break;

		case ces_GetFillingData:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d GetFillingData -> ActiveFilling", disp_index, nozzle_index);
			interpret_filling_data(buffer, length, disp_index, nozzle_index, log_options);

			if (safe_get_emergency_stop(disp_index))
			{
				safe_set_emergency_stop(disp_index, FALSE);
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ActiveFilling -> EmergencyStop", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_EmergencyStop);
			}
			else
			{
				if (safe_get_dispencer_state(disp_index) == ds_Filling)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ActiveFilling -> GetFillingData", disp_index, nozzle_index);
					safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ActiveFilling);
				}
				else if (safe_get_dispencer_state(disp_index) == ds_Stopped)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ActiveFilling -> WaitPayment", disp_index, nozzle_index);
					safe_set_channel_exchange_state(disp_index, nozzle_index, ces_WaitPayment);
				}
				else if (safe_get_dispencer_state(disp_index) == ds_Finish)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ActiveFilling -> WaitPayment", disp_index, nozzle_index);
					safe_set_channel_exchange_state(disp_index, nozzle_index, ces_WaitPayment);
				}
				else
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ActiveFilling -> ActiveFilling", disp_index, nozzle_index);
					safe_set_channel_exchange_state(disp_index, nozzle_index, ces_GetFillingData);

				}
			}
			break;

		case ces_WaitPayment:
			interpret_filling_data(buffer, length, disp_index, nozzle_index, log_options);

			if (safe_get_is_pay(disp_index) || safe_get_reset(disp_index))
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d WaitPayment -> ConfirmPayment", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ConfirmPayment);
			}
			break;

		case ces_ConfirmPayment:

			break;


		case ces_GetTotalCounter:
			interpret_get_counter(buffer, length, disp_index, nozzle_index, log_options);
			safe_disp_clear(disp_index);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d GetTotalCounter -> Idle", disp_index, nozzle_index);
			safe_set_channel_exchange_state(disp_index, nozzle_index, ces_Idle);
			break;

		case ces_EmergencyStop:

			break;
	}

	safe_set_command_send(disp_index, nozzle_index, FALSE);
}

void interpret_short_reply(guint8* buffer, guint8 length, LogOptions log_options)
{

	guint8 disp_index = 0;
	guint8 nozzle_index = 0;

	safe_restore_last_indexes(&disp_index, &nozzle_index);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d interpret short reply", disp_index, nozzle_index);

	if (buffer[STX_REPLY_OFFSET] == ctrl_ACK || buffer[STX_REPLY_OFFSET] == ctrl_CAN)
	{
		switch(safe_get_channel_exchange_state(disp_index, nozzle_index))
		{
			case ces_Undefined:
				break;

			case ces_GetVersion:
				break;

			case ces_GetCounter:
				break;

			case ces_Idle:
				break;

			case ces_SendPrice:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d SendPrice -> Preset", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_Preset);
				break;

			case ces_Preset:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d Preset -> Start", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_Start);
				break;

			case ces_Start:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d Start -> WaitStart", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_WaitStart);
				break;

			case ces_WaitStart:
				break;

			case ces_ConfirmStart:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ConfirmStart -> ActiveFilling", disp_index, nozzle_index);
				safe_set_dispencer_state(disp_index, ds_Filling);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ActiveFilling);
				break;

			case ces_ActiveFilling:
				break;

			case ces_GetFillingData:
				break;

			case ces_WaitPayment:
				break;

			case ces_ConfirmPayment:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d ConfirmPayment -> GetTotalCounter", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_GetTotalCounter);
				break;

			case ces_GetTotalCounter:
				break;

			case ces_EmergencyStop:
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer %d nozzle %d EmergencyStop -> ActiveFilling", disp_index, nozzle_index);
				safe_set_channel_exchange_state(disp_index, nozzle_index, ces_ActiveFilling);
				break;

		}
	}

	safe_set_command_send(disp_index, nozzle_index, FALSE);

}

void save_log(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");
}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	FrameStage fs = fs_WaitDEL;

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch (fs)
				{
					case fs_WaitDEL:
						if (buff[i] == ctrl_DEL)
						{
							length = 0;
							buffer[length++] = buff[i];
							fs = fs_WaitSTX;
						}
						break;

					case fs_WaitSTX:
						buffer[length++] = buff[i];
						switch (buff[i])
						{
							case ctrl_STX:
								fs = fs_ReadData;
								break;
							case ctrl_ACK:
							case ctrl_CAN:
								save_log(buffer, length, log_options);
								interpret_short_reply(buffer, length, log_options);
								fs = fs_WaitDEL;
								break;
						}

						break;

					case fs_ReadData:
						buffer[length++] = buff[i];
						fs = fs_ReadDataCompl;
						if (buff[i] == ctrl_ETX)
						{
							fs = fs_WaitSecondStop;
						}
						break;

					case fs_ReadDataCompl:
						fs = fs_ReadData;
						break;

					case fs_WaitSecondStop:
						fs = fs_WaitCRC;
						break;

					case fs_WaitCRC:
						save_log(buffer, length, log_options);
						if (buff[i] == calc_crc(buffer, length))
						{
							interpret_reply(buffer, length, log_options);
						}
						else
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "CRC error: calc=%02X repl=%02X ", calc_crc(buffer, length), buff[i]);
						}
						fs = fs_WaitDEL;
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
