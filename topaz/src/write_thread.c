#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "topaz.h"
#include "config.h"
#include "driver_state.h"
#include "topaz_func.h"

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

	guint8 buffer[WRITE_BUFFER_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && disp_count > 0)
		{
			guint8 curr_disp_index = 0;
			guint8 curr_nozzle_index = 0;
			safe_get_current_nozzle(&curr_disp_index, &curr_nozzle_index);

			guint8 channel  = safe_get_nozzle_grade(curr_disp_index, curr_nozzle_index);

			if (safe_get_command_send(curr_disp_index, curr_nozzle_index))
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Lost reply channel %d", channel);
				safe_set_channel_exchange_state(curr_disp_index, curr_nozzle_index, ces_Undefined);
				safe_set_dispencer_state(curr_disp_index, ds_NotInitialize);
				safe_set_command_send(curr_disp_index, curr_nozzle_index, FALSE);
			}


			switch(safe_get_channel_exchange_state(curr_disp_index, curr_nozzle_index))
			{
				case ces_Undefined:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send pump type request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_PumpTypeReq);
					break;

				case ces_GetVersion:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send get version request  channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_GetVersion);
					break;

				case ces_GetCounter:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send get counter request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_TotalCountersReq);
					break;

				case ces_Idle:
					if (safe_get_preset_order_type(curr_disp_index) != ot_Free &&
						safe_get_preset_nozzle_index(curr_disp_index) == curr_nozzle_index &&
						safe_get_active_nozzle_index(curr_disp_index) == safe_get_preset_nozzle_index(curr_disp_index) &&
						safe_get_nozzle_was_on(curr_disp_index))
					{
						safe_set_nozzle_was_on(curr_disp_index, FALSE);

						gint8 nozzle = 0;
						guint32 price = 0;
						guint32 volume = 0;
						guint32 amount = 0;

						safe_get_preset(curr_disp_index, &nozzle, &price, &volume, &amount);

						safe_set_nozzle_price(curr_disp_index, curr_nozzle_index, price);

						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send price %d: %d", channel, price);

						prepare_command_param_request(buffer, channel, &buffer_length, acc_PricePreset, price, PRICE_PRESET_SIZE);
						safe_set_channel_exchange_state(curr_disp_index, curr_nozzle_index, ces_SendPrice);
					}
					else
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request channel %d", channel);
						prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					}
					break;

				case ces_SendPrice:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					break;

				case ces_Preset:
					if (safe_get_preset_order_type(curr_disp_index) != ot_Free && safe_get_preset_nozzle_index(curr_disp_index) == curr_nozzle_index)
					{
						gint8 nozzle = 0;
						guint32 price = 0;
						guint32 volume = 0;
						guint32 amount = 0;

						safe_get_preset(curr_disp_index, &nozzle, &price, &volume, &amount);

						switch (safe_get_preset_order_type(curr_disp_index))
						{
							case ot_Volume:
								add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send litres request channel %d (%d)", channel, volume);
								prepare_command_param_request(buffer, channel, &buffer_length, acc_LitresPreset, volume, VOLUME_PRESET_SIZE);
								break;

							case ot_Sum:
								add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send amount request channel %d (%d)", channel, amount);
								prepare_command_param_request(buffer, channel, &buffer_length, acc_MoneyPreset, amount, SUM_PRESET_SIZE);
								break;

							case ot_Unlim:
								add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send unlim litres request channel %d (%d)", channel, volume);
								prepare_command_param_request(buffer, channel, &buffer_length, acc_LitresPreset, volume, VOLUME_PRESET_SIZE);
								break;

							case ot_Free:
								add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request channel %d", channel);
								safe_set_channel_exchange_state(curr_disp_index, curr_nozzle_index, ces_Idle);
								prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
								break;

							default:
								safe_set_channel_exchange_state(curr_disp_index, curr_nozzle_index, ces_Idle);
								prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
								break;
						}
					}
					else
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request channel %d", channel);
						safe_set_channel_exchange_state(curr_disp_index, curr_nozzle_index, ces_Idle);
						prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					}
					break;

				case ces_Start:
					if (safe_get_auto_start() || safe_get_disp_start(curr_disp_index))
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send start channel %d", channel);
						prepare_command_request(buffer, channel, &buffer_length, acc_EnableFillingReq);
					}
					else
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request (wait start) channel %d", channel);
						prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					}
					break;

				case ces_WaitStart:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status (wait start) request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					break;

				case ces_ConfirmStart:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send start channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_StartFilling);
					break;

				case ces_ActiveFilling:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send status request (filling) channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					break;

				case ces_GetFillingData:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send get filling data request (filling) channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_FullFillingDataReq);
					break;


				case ces_WaitPayment:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send get full filling data request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_FullFillingDataReq);
					break;

				case ces_GetTotalCounter:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send get counter request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_TotalCountersReq);
					break;

				case ces_ConfirmPayment:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send confirm filling channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_ConfirmFilling);
					break;

				case ces_EmergencyStop:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send stop channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_StopFillingReq);
					break;
				break;

				default:
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send default start request channel %d", channel);
					prepare_command_request(buffer, channel, &buffer_length, acc_GetStatusReq);
					break;

			}

			if (send_func(buffer, buffer_length, log_options))
			{
				safe_store_last_indexes(curr_disp_index, curr_nozzle_index);
				safe_set_command_send(curr_disp_index, curr_nozzle_index, TRUE);
			}

			exec_time = get_date_time();

			safe_change_channel();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}


