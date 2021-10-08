#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "tokheim.h"
#include "tokheim_func.h"
#include "config.h"
#include "driver_state.h"
#include "tokheim_frames.h"

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

	guint8 repl_len = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && disp_count > 0)
		{
			if (safe_get_command_sended())
			{
				guint8 last_sended_disp_index = safe_get_last_sended_disp_index();

				if (safe_get_fault_reply_counter(last_sended_disp_index) > MAX_FAULT_REPLY_COUNT)
				{
					if (safe_get_exchange_state(last_sended_disp_index) == es_GetCounters)
					{
						safe_set_disp_counters_enable(last_sended_disp_index, FALSE);
					}
					safe_set_dispencer_state(last_sended_disp_index, ds_NotInitialize);
					safe_set_exchange_state(last_sended_disp_index,es_Undefined);

					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Reset exchange index %d", last_sended_disp_index);
				}
				else
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Increment fault reply index %d", last_sended_disp_index);
					safe_inc_fault_reply_counter(last_sended_disp_index);
				}

				safe_set_command_sended(FALSE);
			}
			guint8 disp_addr = 0;
			guint8 adi = safe_get_active_disp_index();

			if (safe_get_disp_addr(adi, &disp_addr) == de_NoError)
			{
				switch (safe_get_exchange_state(adi))
				{
					case es_Undefined:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get fuelling point ID disp addr %d", disp_addr);
						buffer_length = prepare_get_fuelling_point_id_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointId_req);
						break;

					case es_GetStartStatus:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get start status disp addr %d", disp_addr);
						buffer_length = prepare_get_fuelling_point_status_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointStatus_req);
						break;

					case es_ClearDisplay:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Clear display disp addr %d", disp_addr);
						buffer_length = prepare_send_data_for_fuelling_point_frame(buffer, disp_addr, safe_get_current_volume(adi), safe_get_current_price(adi));
						repl_len = set_reply_length(adi, tc_SendDataForFuellingPoint);
						break;


					case es_GetCounters:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get counters disp addr %d", disp_addr);
						buffer_length = prepare_fuelling_point_totals_frame(buffer, adi, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointTotals_req);
						break;

					case es_DeactivateNozzle:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send deactivate nozzle disp addr %d", disp_addr);
						buffer_length = prepare_request_deactivated_hose_frame(buffer, disp_addr);
						repl_len = set_reply_aux_length(adi, tac_DeactivatedHose_req);
						break;

					case es_GetStatus:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get status disp addr %d", disp_addr);
						buffer_length = prepare_get_fuelling_point_status_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointStatus_req);
						break;

					case es_GetNozzle:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send activate nozzle disp addr %d", disp_addr);
						buffer_length = prepare_request_activated_hose_frame(buffer, disp_addr);
						repl_len = set_reply_aux_length(adi, tac_ActivatedHose_req);
						break;

					case es_WaitPreset:

#ifdef DEBUG_MODE
						safe_set_disp_preset(1, 1, 5080, 1000, 0, ot_Volume);
						safe_set_disp_start(adi, TRUE);
#endif

						if (safe_get_preset_order_type(adi) != ot_Free &&
							safe_get_active_nozzle_index(adi) == safe_get_preset_nozzle_index(adi) &&
							safe_get_disp_start(adi))
						{

							gint8	nozzle_index = -1;
							guint32 price = 0;
							guint32 volume = 0;
							guint32 amount = 0;


							safe_get_preset(adi, &nozzle_index, &price, &volume, &amount);

							guint8 price_dp = 0;
							guint8 volume_dp = 0;
							guint8 amount_dp = 0;

							safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);


							switch(safe_get_preset_order_type(adi))
							{
								case ot_Free:
									break;

								case ot_Volume:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send volume preset disp addr %d", disp_addr);
									buffer_length = prepare_authorize_fuelling_point_frame(buffer, disp_addr, price, volume);
									repl_len = set_reply_length(adi, tc_AuthorizeFuellingPoint);
									break;

								case ot_Sum:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send amount preset disp addr %d", disp_addr);
									volume = prepare_authorize_fuelling_point_frame(buffer, disp_addr, price, div_amount_price(price, amount, volume_dp));
									repl_len = set_reply_length(adi, tc_AuthorizeFuellingPoint);
									break;

								case ot_Unlim:
									add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send unlim preset disp addr %d", disp_addr);
									buffer_length = prepare_authorize_fuelling_point_frame(buffer, disp_addr, price, volume);
									repl_len = set_reply_length(adi, tc_AuthorizeFuellingPoint);
									break;
							}

							safe_set_exchange_state(adi,es_SendPreset);
						}
						else
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get status disp addr %d", disp_addr);
							buffer_length = prepare_get_fuelling_point_status_frame(buffer, disp_addr);
							repl_len = set_reply_length(adi, tc_FuellingPointStatus_req);

						}
						break;

					case es_SendPreset:
						break;

					case es_WaitStart:
						if (safe_get_start_filling_filter(adi) > 0)
						{
							safe_dec_start_filling_filter(adi);
							add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Wait start disp addr %d (%d)", disp_addr, safe_get_start_filling_filter(adi));
							buffer_length = prepare_get_fuelling_point_status_frame(buffer, disp_addr);
							repl_len = set_reply_length(adi, tc_FuellingPointStatus_req);
						}
						else
						{
							safe_set_exchange_state(adi,es_GetFillingInfo );
							add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get fillling info disp addr %d", disp_addr);
							buffer_length = prepare_get_fuelling_point_display_data_frame(buffer, disp_addr);
							repl_len = set_reply_length(adi, tc_FuellingPointDisplayData_req);
						}
						break;

					case es_GetFillingInfo:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get fillling info disp addr %d", disp_addr);
						buffer_length = prepare_get_fuelling_point_display_data_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointDisplayData_req);
						break;

					case es_Stopping:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send suspend disp addr %d", disp_addr);
						buffer_length = prepare_suspend_fuelling_point_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_SuspendFuellingPoint);
						break;

					case es_Resume:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send resume disp addr %d", disp_addr);
						buffer_length = prepare_resume_fuelling_point_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_ResumeFuellingPoint);
						break;

					case es_GetTotCounters:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Get total counters disp addr %d", disp_addr);
						buffer_length = prepare_fuelling_point_totals_frame(buffer, adi, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointTotals_req);
						break;


					case es_WaitPayment:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Wait payment disp addr %d", disp_addr);

#ifdef DEBUG_MODE
						safe_set_is_pay(adi, TRUE);
#endif

						buffer_length = prepare_get_fuelling_point_display_data_frame(buffer, disp_addr);
						repl_len = set_reply_length(adi, tc_FuellingPointDisplayData_req);
						break;

					case es_SendPrices:
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Send prices disp addr %d", disp_addr);
						buffer_length = prepare_send_aux_cash_prices_frame(buffer, adi, disp_addr);
						repl_len = set_reply_aux_length(adi, tac_SendCashPrices_req);
						break;

				}

				if (buffer_length > 0)
				{
					safe_set_rx_stage(rs_Value);

					if (send_func(buffer, buffer_length, log_options))
					{
						safe_set_last_sended_disp_index(adi);

						if (repl_len > 0)
						{
							safe_set_command_sended(TRUE);

						}
					}
				}

			}

			safe_increment_active_disp_index();
			exec_time = get_date_time();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}

