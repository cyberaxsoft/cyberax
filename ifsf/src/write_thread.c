#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "ifsf.h"
#include "ifsf_func.h"
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
	guint64 healthbit_time = get_date_time();

	guint32 timeout_write = safe_get_timeout_write();
	guint8 disp_count = safe_get_disp_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[UART_BUFFER_WRITE_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > healthbit_time + (HEALTHBIT_TIMEOUT * 1000))
		{
			buffer_length = prepare_healthbit_message(buffer);
			if (send_func(buffer, buffer_length, log_options))
			{
				healthbit_time = get_date_time();
			}
		}
		if (get_date_time() > exec_time + timeout_write && disp_count > 0)
		{
			guint8 adi = safe_get_active_disp_index();
			guint8 ni = safe_get_node_index(adi);

		//	printf("# %d,%d\n", adi, ni);

			if ((get_date_time() > safe_get_last_heartbeat_time(ni) + (guint64)((HEALTHBIT_TIMEOUT * 3) * 1000)) && safe_get_node_stage(ni) > ifsfns_Offline)
			{
				safe_set_dispencer_state(adi, ds_NotInitialize);

				safe_set_node_stage(ni, ifsfns_Offline);
			}
			guint8 disp_addr = 0;

			if (safe_get_disp_addr(adi, &disp_addr) == de_NoError )
			{
				switch(safe_get_node_exchange_state(ni))
				{
					case ifsfnes_Free:

					//	printf("ns = %d, %d\n", ni, safe_get_node_stage(ni));

						switch (safe_get_node_stage(ni))
						{
							case ifsfns_Offline:
								buffer_length = 0;
								safe_set_exchange_stage(adi, esg_SendZeroFunc);
								break;

							case ifsfns_ConnectReady:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send addr message:");
								buffer_length = prepare_addr_message(buffer, ni);
								break;

							case ifsfns_TimeoutReady:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send timeout message:");
								buffer_length = prepare_timeout_message(buffer, ni);
								break;

							case ifsfns_CalculatorConfReady:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send calculator configuration message:");
								buffer_length = prepare_calculator_conf_message(buffer, ni);
								break;

							case ifsfns_ConfigReady:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send config message:");
								buffer_length = prepare_config_message(buffer, ni);
								break;

							case ifsfns_LoadProduct:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send load product %d message:", safe_get_config_product_index(ni));
								buffer_length = prepare_set_id_product_message(buffer, ni, safe_get_config_product_index(ni));
								break;

							case ifsfns_SetProductParams:
								add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send set product %d param %d message:", safe_get_config_product_index(ni), safe_get_config_fuelling_mode_index(ni));
								buffer_length = prepare_set_product_param_message(buffer, ni, safe_get_config_product_index(ni), safe_get_config_fuelling_mode_index(ni));
								break;

							case ifsfns_Online:
								switch(safe_get_exchange_stage(adi))
								{
									case esg_SendZeroFunc:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send zero func message:");
										buffer_length = prepare_zero_func_message(buffer, ni, adi);
										break;

									case esg_SetNozzleConf:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send nozzle conf message:");
										buffer_length = prepare_nozzle_conf_message(buffer, ni, adi, safe_get_current_nozzle_index(adi));
										break;

									case esg_SetFpParam:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send FP param conf message:");
										buffer_length = prepare_fp_param_conf_message(buffer, ni, adi);
										break;

									case esg_GetCounters:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send get counters message:");
										buffer_length = prepare_nozzle_total_message(buffer, ni, adi);
										break;

									case esg_Work:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send pump state message:");
										buffer_length = prepare_get_pump_state_message(buffer, ni, adi);
										break;

									case esg_OpenFP:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send open fp message:");
										buffer_length = prepare_open_fp_message(buffer, ni, adi);
										break;

									case esg_WaitOpen:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Wait open:");
										buffer_length = prepare_get_pump_state_message(buffer, ni, adi);
										break;

									case esg_AllowedNozzle:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send allowed nozzle:");
										buffer_length = prepare_allowed_nozzles_message(buffer, ni, adi, TRUE);
										break;

									case esg_Idle:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send idle pump state message:");
										buffer_length = prepare_get_pump_state_message(buffer, ni, adi);
										break;

									case esg_SetPrice:
										if (safe_get_send_price_disp_index(ni) < 0)
										{
											if (safe_get_active_nozzle_index(adi) >= 0)
											{
												gint8 nozzle_index = -1;
												guint32 price = 0;
												guint32 volume = 0;
												guint32 amount = 0;

												safe_get_preset(adi, &nozzle_index, &price, &volume, &amount);

												add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send price message:");
												buffer_length = prepare_set_price_message(buffer, ni, adi, price);

												safe_set_send_price_disp_index(ni, adi);
											}
											else
											{
												buffer_length = 0;
											}
										}
										else
										{
											buffer_length = 0;
										}

										break;

									case esg_AuthorizeNozzle:
										if (safe_get_active_nozzle_index(adi) >= 0)
										{
											add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send authorize nozzle message:");
											buffer_length = prepare_authorize_nozzle_message(buffer, ni, adi);
										}
										else
										{
											buffer_length = 0;
										}
										break;


									case esg_Preset:
										if (safe_get_active_nozzle_index(adi) >= 0)
										{
											gint8 nozzle_index = -1;
											guint32 price = 0;
											guint32 volume = 0;
											guint32 amount = 0;

											safe_get_preset(adi, &nozzle_index, &price, &volume, &amount);

											switch(safe_get_preset_order_type(adi))
											{
												case ot_Free:
													buffer_length = 0;
													break;

												case ot_Volume:
													add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send volume preset message:");
													buffer_length = prepare_preset_volume_message(buffer, ni, adi, volume);
													break;

												case ot_Amount:
													add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send amount preset message:");
													buffer_length = prepare_preset_amount_message(buffer, ni, adi, amount);
													break;

												case ot_Unlim:
													buffer_length = 0;
													break;

											}
										}
										else
										{
											buffer_length = 0;
										}
										break;

									case esg_WaitStart:
										if (safe_get_disp_start(adi))
										{
											add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send start message:");
											buffer_length = prepare_start_message(buffer, ni, adi);
										}
										else
										{
											add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send wait start pump state message:");
											buffer_length = prepare_get_pump_state_message(buffer, ni, adi);
										}
										break;

									case esg_Filling:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send get filling data message:");
										buffer_length = prepare_get_filling_data_message(buffer, ni, adi);
										break;

									case esg_GetTransaction:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send get transaction message:");
										buffer_length = prepare_get_transaction_message(buffer, ni, adi);
										break;

									case esg_WaitPaying:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send wait paying pump state message:");
										buffer_length = prepare_get_pump_state_message(buffer, ni, adi);
										break;
									case esg_LockTransaction:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send lock transaction message:");
										buffer_length = prepare_lock_transaction_message(buffer, ni, adi);
										break;

									case esg_GetFinishCounters:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send get counters message:");
										buffer_length = prepare_nozzle_total_message(buffer, ni, adi);
										break;

									case esg_ClearTransaction:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send clear transaction message:");
										buffer_length = prepare_clear_transaction_message(buffer, ni, adi);
										break;

									case esg_Suspend:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send suspend message:");
										buffer_length = prepare_suspend_fp_message(buffer, ni, adi);
										break;

									case esg_Resume:
										add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send resume message:");
										buffer_length = prepare_resume_fp_message(buffer, ni, adi);
										break;

									case esg_SetPrices:
										if (safe_get_send_price_disp_index(ni) < 0)
										{
											add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Send set price message:");
											buffer_length = prepare_product_price_message(buffer, ni, adi);

											safe_set_send_price_disp_index(ni, adi);
										}
										else
										{
											buffer_length = 0;
										}
										break;

									default: buffer_length = 0;
								}
								break;

							default: buffer_length = 0;

						}
						if (buffer_length > 0)
						{
							if (send_func(buffer, buffer_length, log_options))
							{
								safe_set_node_exchange_state(ni, ifsfnes_SendCommand);
								safe_set_send_command_timeout(ni, 0);
							}
						}
						break;

					case ifsfnes_SendCommand:

						safe_increment_send_command_timeout(ni);

						if (safe_get_send_command_timeout(ni) > SEND_COMMAND_TIMEOUT)
						{
							safe_set_send_command_timeout(ni, 0);
							safe_set_node_exchange_state(ni, ifsfnes_Free);
						}
						break;

					case ifsfnes_SendReply:

						break;
				}

			}
			safe_increment_active_disp_index();
			exec_time = get_date_time();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}
