#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "dart.h"
#include "dart_func.h"
#include "config.h"
#include "driver_state.h"

gboolean 		write_thread_terminating = FALSE;
gboolean 		write_thread_terminated = FALSE;
GMutex			write_thread_mutex;

void init_write_thread_mutex()
{
	if (write_thread_mutex.p == NULL)
	{
		g_mutex_init(&write_thread_mutex);
	}
}

gboolean safe_get_write_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminating;

	g_mutex_unlock(&write_thread_mutex);

	return result;

}

gboolean safe_get_write_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminated;

	g_mutex_unlock(&write_thread_mutex);

	return result;

}

void safe_set_write_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminating = new_value;

	g_mutex_unlock(&write_thread_mutex);

}

void safe_set_write_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminated = new_value;

	g_mutex_unlock(&write_thread_mutex);

}

gpointer write_thread_func(gpointer data)
{
	safe_set_write_thread_terminated(FALSE);

	guint64 exec_time = get_date_time();
	guint32 timeout_write = safe_get_timeout_write();
	guint8 disp_count = safe_get_disp_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[UART_BUFFER_WRITE_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && disp_count > 0)
		{
			guint8 disp_addr = 0;
			guint8 adi = safe_get_active_disp_index();

			if (safe_get_disp_addr(adi, &disp_addr) == de_NoError)
			{
				if (safe_get_exchange_state(adi) == des_SendCommand)
				{
					exec_time = get_date_time();

					safe_increment_uart_error_counter(adi);

					if (safe_get_uart_error_counter(adi) > MAX_UART_ERROR)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Timeout error disp index: %d", adi);

						safe_set_block_sequence_number(adi, 0);
						safe_set_reply_block_sequence_number(adi, 0);
						safe_set_exchange_state(adi, des_Free);
						safe_set_exchange_mode(adi, dem_Pooling);
						safe_set_current_command(adi, drc_Free);
						safe_set_dispencer_state(adi, ds_ConnectionError);
						safe_set_get_counters(adi, TRUE);
						safe_set_current_counter_index(adi,0);

						safe_set_uart_error_counter(adi, 0);
					}
				}
				else
				{
					switch(safe_get_exchange_mode(adi))
					{
						case dem_Pooling:
							switch(safe_get_exchange_state(adi))
							{
								case des_Free:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Pool disp index: %d", adi);

									buffer_length = prepare_pool_frame(buffer, disp_addr);
									safe_set_exchange_state(adi, des_SendCommand);
									break;

								case des_ReadyAck:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Ack disp index: %d", adi);
									buffer_length = prepare_ack_frame(buffer, disp_addr, safe_get_reply_block_sequence_number(adi));
									safe_set_exchange_state(adi, des_Free);
									break;

								case des_ReadyNak:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Nak disp index: %d", adi);
									buffer_length = prepare_nak_frame(buffer, disp_addr, safe_get_reply_block_sequence_number(adi));
									break;

								default:
									break;
							}
							break;

						case dem_SendCommand:
							if (safe_get_counters_enable() &&
								safe_get_get_counters(adi) &&
								safe_get_current_counter_index(adi) < safe_get_nozzle_count(adi))
							{
								guint8 counter_index = safe_get_current_counter_index(adi);
								add_log(TRUE, TRUE, log_options.trace, log_options.system, "Get counter disp index: %d - %d", adi, counter_index);
								buffer_length = prepare_get_counter_frame(buffer, disp_addr, safe_get_block_sequence_number(adi), counter_index);
								if (safe_get_exchange_state(adi) == des_Free)
								{
									safe_increment_current_counter_index(adi);
								}
							}
							else
							{

								if(safe_get_get_counters(adi))
								{
									safe_set_get_counters(adi, FALSE);
									safe_set_current_counter_index(adi, 0);
									add_log(TRUE, TRUE, log_options.trace, log_options.system, "Counters getted disp index: %d", adi);
								}

								switch(safe_get_current_command(adi))
								{
									case drc_Free:
										{
											guint8 bsn =  safe_get_block_sequence_number(adi);
											add_log(TRUE, TRUE,log_options.trace, log_options.parsing, "Get status disp index: %d (bsn = %d)", adi, bsn);
											buffer_length = prepare_status_frame(buffer, disp_addr, bsn);
										}
										break;

									case drc_PresetVolume:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Preset volume disp index: %d", adi);
										buffer_length = prepare_preset_frame(buffer, adi, disp_addr, safe_get_block_sequence_number(adi), dc_PresetVolume, log_options);
										break;

									case drc_PresetAmount:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Preset amount disp index: %d", adi);
										buffer_length = prepare_preset_frame(buffer, adi, disp_addr, safe_get_block_sequence_number(adi), dc_PresetAmount,log_options);
										break;

									case drc_Start:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Authorize disp index: %d", adi);
										buffer_length = prepare_command_pump_frame(buffer, disp_addr, dcp_Authorize, safe_get_block_sequence_number(adi));
										break;

									case drc_Stop:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Stop disp index: %d", adi);
										buffer_length = prepare_command_pump_frame(buffer, disp_addr, dcp_Stop, safe_get_block_sequence_number(adi));
										break;

									case drc_Suspend:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Suspend disp index: %d", adi);
										buffer_length = prepare_suspend_pump_frame(buffer, disp_addr, dcp_Suspend, safe_get_block_sequence_number(adi), adi);
										break;

									case drc_Resume:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Resume disp index: %d", adi);
										buffer_length = prepare_resume_pump_frame(buffer, disp_addr, dcp_Resume, safe_get_block_sequence_number(adi), adi);
										break;

									case drc_Payment:
									case drc_Reset:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Reset disp index: %d", adi);
										buffer_length = prepare_command_pump_frame(buffer, disp_addr, dcp_Reset, safe_get_block_sequence_number(adi));
										break;

									case drc_SendPrices:
										add_log(TRUE, TRUE, log_options.trace, log_options.system, "Reset disp index: %d", adi);
										buffer_length = prepare_set_prices_frame(buffer, adi, disp_addr, safe_get_block_sequence_number(adi), log_options);
										break;

									default:
										add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get status (default) disp index: %d", adi);
										buffer_length = prepare_status_frame(buffer, disp_addr, safe_get_block_sequence_number(adi));
										break;
								}
							}
							if (safe_get_exchange_state(adi) == des_Free)
							{
								safe_set_sending_error_counter(adi, 0);
								safe_increment_block_sequence_number(adi);
								safe_set_exchange_state(adi, des_SendCommand);
							}
							break;
					}

					if (send_func(buffer, buffer_length, log_options))
					{
						exec_time = get_date_time();
						safe_set_uart_error_counter(adi, 0);
						safe_set_last_sended_disp_index(adi);
					}
				}
			}
			safe_increment_active_disp_index();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}


