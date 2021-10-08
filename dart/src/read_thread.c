#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "dart.h"
#include "dart_func.h"
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

void interpret_status(guint8 disp_index, DartDispencerState status)
{

	DispencerState current_state = safe_get_dispencer_state(disp_index) ;

	switch(status)
	{
		case dds_NotProgrammed:
			if (!safe_get_get_counters(disp_index) && (current_state == ds_NotInitialize ||  current_state == ds_ConnectionError))
			{
				if (safe_get_active_nozzle_num(disp_index) > 0)
				{
					safe_set_dispencer_state(disp_index, ds_NozzleOut);
				}
				else
				{
					safe_set_dispencer_state(disp_index, ds_Free);
				}
			}
			break;

		case dds_Reset:
			if (!safe_get_get_counters(disp_index))
			{
				if (safe_get_active_nozzle_num(disp_index) > 0)
				{
					safe_set_dispencer_state(disp_index, ds_NozzleOut);
				}
				else
				{
					safe_set_dispencer_state(disp_index, ds_Free);
				}
			}
			break;

		case dds_Authorized:
			safe_set_dispencer_state(disp_index, ds_Filling);
			break;

		case dds_Filling:
			safe_set_dispencer_state(disp_index, ds_Filling);
			break;

		case dds_FillingComplete:
			if (current_state == ds_NotInitialize)
			{
				safe_set_reset(disp_index, TRUE);
			}
			safe_set_dispencer_state(disp_index, ds_Finish);
			break;

		case dds_MaxVolReached:
			safe_set_dispencer_state(disp_index, ds_Stopped);
			break;

		case dds_SwitchedOff:
			safe_set_dispencer_state(disp_index, ds_ConnectionError);
			break;
	}
}

void interpret_tag(guint8 disp_index, guint8 tag, guint8 len, guint8* buffer, LogOptions log_options)
{
	DispencerState current_state = safe_get_dispencer_state(disp_index) ;

	guchar text[DRIVER_DESCRIPTION_LENGTH] = {0x00};

	guint8* data = buffer;

	switch(tag)
	{
		case ddrt_DC0_None:
			break;

		case ddrt_DC1_PumpStatus:
			get_original_pump_status_description(data[0],text);
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing,"	Tag %02X Pump status: %d (%s)", tag, data[0], text);
			safe_set_original_state(disp_index, data[0]);
			break;

		case ddrt_DC2_FilledVolumeAndAmount:
			{
				guint32 volume = packed_bcd_to_bin(data, DART_VOLUME_LENGTH);
				data+=DART_VOLUME_LENGTH;
				guint32 amount = packed_bcd_to_bin(data, DART_AMOUNT_LENGTH);

				safe_set_current_volume(disp_index, volume);
				safe_set_current_amount(disp_index, amount);

				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Tag %02X Volume = %d, Amount = %d", tag, volume, amount);

				if (current_state > ds_NozzleOut && current_state < ds_Finish)
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Filling data dispencer %d: v: %d a: %d", disp_index, volume, amount);

			}
			break;

		case ddrt_DC3_NozzleStatusAndPrice:
			{
				guint32 current_price = packed_bcd_to_bin(data, DART_PRICE_LENGTH);
				data+=DART_PRICE_LENGTH;
				guint8 active_grade = data[0] & NOZZLE_NUM_MASK;

				safe_set_current_price(disp_index, current_price);
				safe_set_original_active_grade(disp_index, active_grade);

				if ((data[0] & NOZZLE_STATE_MASK) > 0)
				{
					safe_set_original_nozzle_state(disp_index, TRUE);
				}
				else
				{
					safe_set_original_nozzle_state(disp_index, FALSE);
				}
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing,"	Tag %02X Price = %d Active grade = %d %s", tag, current_price, active_grade, data[0] & NOZZLE_STATE_MASK ? "NozzleOFF":"NozzleON");
			}

			break;

		case ddrt_DC5_AlarmCode:
			safe_set_error(disp_index, data[0]);
			break;

		case ddrt_DC7_Pump_Pararmeters:

			break;

		case ddrt_DC9_PumpIdentity:

			break;

		case ddrt_DC14_SuspentReply:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	SUSPEND");
			break;

		case ddrt_DC15_ResumeReply:
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	RESUME");

			break;

		case ddrt_DC101_TotalCounters:
			{
				guint32 total = packed_bcd_to_bin(&data[1], DART_TOT_COUNTER_LENGTH);
				safe_set_nozzle_counter(disp_index, data[0] - 1, total);
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Tag %02X Total grade %d = %d", tag, data[0], total);

			}
			break;

	}

}

void interpret_data_frame(guint8 disp_index, guint8* buffer, guint32 length, LogOptions log_options)
{
	guint32 pos = 0;
	DartParseStage dps = dps_WaitTag;

	guint8	tag = 0;
	guint8 	len = 0;

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Parse frame disp index %d:", disp_index);

	while(pos < length)
	{
		switch(dps)
		{
			case dps_WaitTag:
				tag = *buffer++;
				pos++;
				dps = dps_WaitLen;
				break;

			case dps_WaitLen:
				len = *buffer++;
				pos++;
				if (len == 0)
				{
					interpret_tag(disp_index, tag, len, NULL, log_options);
				}
				else
				{
					interpret_tag(disp_index, tag, len, buffer, log_options);
				}
				buffer+=len;
				pos+=len;
				dps = dps_WaitTag;
				break;
		}
	}

	gint8 ani = safe_get_active_nozzle_index(disp_index);

	if (safe_get_original_nozzle_state(disp_index))
	{
		gint8 tmp = safe_get_nozzle_index_by_grade(disp_index, safe_get_original_active_grade(disp_index));

		if (tmp!=ani)
		{
			safe_set_active_nozzle_index(disp_index, tmp);

			if (tmp >=0)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d Nozzle OFF", disp_index);
				if (safe_get_dispencer_state(disp_index) == ds_Free)
				{
					safe_set_dispencer_state(disp_index, ds_NozzleOut);
				}
			}
		}

	}
	else
	{
		if (ani > -1)
		{
			safe_set_active_nozzle_index(disp_index, -1);
			if (safe_get_dispencer_state(disp_index) == ds_NozzleOut)
			{
				safe_set_dispencer_state(disp_index, ds_Free);
			}
			else if (safe_get_dispencer_state(disp_index) == ds_Filling)
			{
				safe_set_dispencer_state(disp_index, ds_Finish);
			}

			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d Nozzle ON", disp_index);
		}
	}

}

void set_complete_command(guint8 disp_index, LogOptions log_options)
{
	switch(safe_get_current_command(disp_index))
	{
		case drc_Stop:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Stop command complete for disp index %d", disp_index);
			safe_set_emergency_stop(disp_index, FALSE);
			break;

		case drc_Reset:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Reset command complete for disp index %d", disp_index);
			safe_set_reset(disp_index, FALSE);
			safe_disp_reset(disp_index);
			if (safe_get_counters_enable())
			{
				safe_set_get_counters(disp_index, TRUE);
			}
			break;

		case drc_Suspend:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Suspend command complete for disp index %d", disp_index);
			safe_set_suspend(disp_index, FALSE);
			safe_set_resume(disp_index, FALSE);
			break;

		case drc_Resume:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Resume command complete for disp index %d", disp_index);
			safe_set_suspend(disp_index, FALSE);
			safe_set_resume(disp_index, FALSE);
			break;

		case drc_PresetAmount:
		case drc_PresetVolume:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Preset command complete for disp index %d", disp_index);
			safe_copy_disp_order_type(disp_index);
			break;

		case drc_SendPrices:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send prices command complete for disp index %d", disp_index);
			safe_set_send_prices(disp_index, FALSE);
			break;

		case drc_Payment:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Payment command complete for disp index %d", disp_index);
			if (safe_get_counters_enable())
			{
				safe_set_get_counters(disp_index, TRUE);
			}
			safe_disp_clear(disp_index);
			break;

		case drc_Start:
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Start command complete for disp index %d", disp_index);
			break;

		case drc_Free:

			break;
	}


}

void select_next_command(guint8 disp_index, LogOptions log_options)
{
	if (safe_get_emergency_stop(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Stop command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_Stop);
	}
	else if (safe_get_reset(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Reset command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_Reset);
	}
	else if (safe_get_send_prices(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send prices command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_SendPrices);
	}
	else if (safe_get_suspend(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send suspend command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_Suspend);
	}
	else if (safe_get_resume(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send resume command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_Resume);
	}

	else if (safe_get_preset_order_type(disp_index) != ot_Free && safe_get_order_type(disp_index) == ot_Free)
	{
		switch (safe_get_preset_order_type(disp_index))
		{
			case ot_Volume:
			case ot_Unlim:
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Preset volume command activate for disp index %d", disp_index);
				safe_set_current_command(disp_index, drc_PresetVolume);
				break;

			case ot_Sum:
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Preset amount command activate for disp index %d", disp_index);
				safe_set_current_command(disp_index, drc_PresetAmount);
				break;

			case ot_Free:
				break;
		}
	}


	else if (safe_get_original_state(disp_index) < dds_Authorized && safe_get_active_nozzle_index(disp_index) == safe_get_preset_nozzle_index(disp_index) &&
			safe_get_order_type(disp_index) != ot_Free && (safe_get_auto_start() || safe_get_disp_start(disp_index)))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Start command activate for disp index %d", disp_index);
		safe_set_current_command(disp_index, drc_Start);
	}

	if (safe_get_current_command(disp_index) == drc_Free)
	{
		if (safe_get_original_state(disp_index) > dds_Filling && safe_get_dispencer_state(disp_index) == ds_Filling)
		{
			if (safe_get_counters_enable())
			{
				safe_set_get_counters(disp_index, TRUE);
			}

			if (safe_compare_disp_values(disp_index))
			{
				if (safe_get_is_pay(disp_index))
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Payment command activate for disp index %d", disp_index);
					safe_set_current_command(disp_index, drc_Payment);
				}
			}
			else
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Deactivate paymant mode for disp index %d", disp_index);
				safe_set_is_pay(disp_index, FALSE);
			}
		}
		else if (safe_get_original_state(disp_index) > dds_Filling && safe_get_dispencer_state(disp_index) > ds_Filling && safe_get_is_pay(disp_index))
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Payment command activate for disp  index%d", disp_index);
			safe_set_current_command(disp_index, drc_Payment);
		}

		interpret_status(disp_index, safe_get_original_state(disp_index));

	}
}

void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");

	guint8 disp_index;

	if (safe_get_disp_index_by_addr((buffer[ADDR_OFFSET] - 0x50) + 1, &disp_index) == de_NoError)
	{
		safe_set_uart_error_counter(disp_index,0);

		safe_set_reply_block_sequence_number(disp_index, buffer[CONTROL_TYPE_OFFSET] & BLOCK_NUMBER_MASK);

		switch(buffer[CONTROL_TYPE_OFFSET] & CONTROL_TYPE_MASK)
		{
			case dct_Poll:
				break;

			case dct_Data:
				if (calc_crc(buffer, length - FRAME_SYSTEM_SYMBOL_LEN) == ((buffer[length - 3] << 8) | buffer[length - 4]))
				{
					interpret_data_frame(disp_index, &buffer[DATA_OFFSET], length - 6, log_options);
					safe_set_exchange_state(disp_index, des_ReadyAck);
				}
				else
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "CRC error! Disp index %d", disp_index);
					safe_set_exchange_state(disp_index, des_ReadyNak);
				}
				safe_set_exchange_mode(disp_index, dem_Pooling);
				break;

			case dct_Ack:
				if (safe_get_exchange_mode(disp_index) == dem_SendCommand)
				{
					safe_set_exchange_state(disp_index, des_Free);
					safe_set_exchange_mode(disp_index, dem_Pooling);
				}
				break;

			case dct_Nak:
				safe_increment_sending_error_counter(disp_index);

				if (safe_get_sending_error_counter(disp_index) > MAX_UART_ERROR && safe_get_exchange_mode(disp_index) == dem_SendCommand)
				{
					switch(safe_get_current_command(disp_index))
					{
						case drc_Free:
						case drc_Reset:
						case drc_Payment:
						case drc_Stop:
						case drc_Start:
						case drc_SendPrices:
						case drc_Suspend:
						case drc_Resume:
							break;

						case drc_PresetVolume:
						case drc_PresetAmount:
							safe_set_preset(disp_index, -1, 0, 0, 0, ot_Free);
							break;

					}
					safe_set_exchange_state(disp_index, des_Free);
					safe_set_exchange_mode(disp_index, dem_Pooling);
				}
				break;

			case dct_Eot:
				switch(safe_get_exchange_mode(disp_index))
				{
					case dem_Pooling:

						set_complete_command(disp_index, log_options);

						safe_set_current_command(disp_index, drc_Free);

						select_next_command(disp_index, log_options);

						safe_set_exchange_mode(disp_index, dem_SendCommand);
						break;

					case dem_SendCommand:
						safe_set_exchange_mode(disp_index, dem_Pooling);
						break;
				}
				safe_set_exchange_state(disp_index, des_Free);
				break;

			case dct_AckPoll:
				break;
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Addr error!");
	}

}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				if (length == 0)
				{
					if ((buff[i] & DART_ADDRESS_MASK) == DART_ADDRESS_OFFSET)
					{
						buffer[length++] = buff[i];
					}
				}
				else
				{
					buffer[length++] = buff[i];

					if (buff[i] == CTRL_SF)
					{
						if (buffer[length - 2] == CTRL_DLE)
						{
							buffer[length - 2] = CTRL_SF;
							length--;
						}
						else
						{
							interpret_reply(buffer, length, log_options);
							length = 0;
						}
					}
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
		else
		{
		//	add_log(&log_file, TRUE, TRUE, "Read timeout");
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}
