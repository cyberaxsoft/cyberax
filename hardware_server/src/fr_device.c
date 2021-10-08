#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

#include "logger.h"
#include "configuration.h"
#include "tlv.h"
#include "system_func.h"
#include "socket_func.h"
#include "fr_device.h"
#include "fr_device_data.h"
#include "fr_device_client_data.h"
#include "fr_device_driver_func.h"
//#include "fr_device_message_func.h"

void close_fr_sock(guint8 index)
{
	gint32 sock = get_fr_sock(index);

	if (sock >= 0)
	{

		shutdown(sock, SHUT_RDWR);

		set_fr_sock_status(index, ss_Disconnected);

		set_fr_clients_status_is_destroying();

		while(TRUE)
		{
			gboolean exit_ready = TRUE;

			for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
			{
				if ( get_client_fri(i) == index && get_fr_client_state(i) != cls_Free)
				{
					exit_ready = FALSE;
				}
			}

			if (exit_ready)
			{
				break;
			}
		}
	}
}


gpointer fr_main_socket_thread_func(gpointer data)
{

 //   guint8 fri = *(guint8*)data;

//	gchar* device_name = get_dc_device_name(dci);
//	guint32 device_port = get_dc_device_port(dci);
//
//	LogParams log_params = {0x00};
//
//	CommonConf common_conf = {0x00};
//	get_common_conf(&common_conf);
//
//	gchar* log_prefix = g_strdup_printf("%s_connect", device_name);
//
//	create_log_params(&log_params, common_conf.conn_log_enable, common_conf.conn_log_dir, log_prefix, common_conf.file_size, common_conf.save_days);
//
//	if (common_conf.conn_log_enable)
//	{
//		g_printf("%s : start connect logger\n",log_prefix);
//
//
//		if (!init_log_dir(&log_params))
//		{
//			g_printf("%s : initialization log error\n",log_prefix);
//		}
//
//		g_printf("%s : system log path: %s\n",log_prefix, common_conf.conn_log_dir);
//
//		create_log(&log_params);
//
//		if (log_params.log == NULL)
//		{
//			g_printf("%s : log create error\n",log_prefix);
//		}
//	}
//
//	connect_dc_socket(dci, &log_params, common_conf.conn_log_trace, device_port);
//	guint64 last_connect_time = get_date_time();
//
//	set_dc_main_sock_thread_status(dci, ts_Active);
//
//	gboolean accept_error_sended = FALSE;
//
//	while(get_dc_sock_status(dci)!= ss_Disconnected)
//	{
//   		if (get_dc_sock_status(dci) == ss_ConnectReady && get_date_time() > last_connect_time + SOCKET_RECONNECT_TIMEOUT && device_port > 0)
//   		{
//   			connect_dc_socket(dci, &log_params, common_conf.conn_log_trace, device_port);
//        	last_connect_time = get_date_time();
//   			accept_error_sended = FALSE;
//        }
//
//        if (get_dc_sock_status(dci) == ss_Connected )
//        {
//        	struct sockaddr client_addr;
//        	socklen_t client_addr_len = 0;
//
//        	gint32 dc_sock = get_dc_sock(dci);
//
//       		gint32 client_socket = accept(dc_sock, &client_addr, &client_addr_len);
//
//       		if(client_socket < 0)
//       		{
//       			if (!accept_error_sended)
//       			{
//       				add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client socket accepting error");
//       				accept_error_sended = TRUE;
//       			}
//       		}
//       		else
//       		{
//               	gboolean pe = TRUE;
//               	get_profiles_enable(&pe);
//
//               	guint8 client_index = find_new_dc_client_index(pe);
//
//               	if (client_index != NO_CLIENT)
//               	{
//               		set_new_dc_client_param(client_index, dci);
//
//               		set_dc_client_sock(client_index, client_socket);
//
//        			struct sockaddr_in addr;
//        			socklen_t addr_size = sizeof(struct sockaddr_in);
//        			getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);
//
//        			add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "accepted client socket %d (%s)", client_socket, inet_ntoa(addr.sin_addr));
//
//    				DCClientInfo info = { .client_index = client_index,
//    															.dispencer_controller_index = dci,
//    															.socket = client_socket,
//    															.device_name = device_name,
//    															.ip_address = g_strdup( inet_ntoa(addr.sin_addr)),
//																.log_params = &log_params,
//																.log_frames = common_conf.conn_log_frames,
//																.log_parsing = common_conf.conn_log_parsing,
//																.log_trace = common_conf.conn_log_trace
//    															};
//
//    				GThread* client_thread = g_thread_new("dc_client_thread", dc_device_client_thread_func, &info);
//
//    				if (client_thread == NULL)
//    				{
//    					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "%d error starting client thread socket (%s)", client_socket, inet_ntoa(addr.sin_addr));
//    				}
//    				else
//    				{
//    					client_thread->priority = G_THREAD_PRIORITY_LOW;
//    				}
//        		}
//               	else
//               	{
//					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client list is full. Disconnecting client");
//               		close(client_socket);
//               	}
//        	}
//        }
//	}
//
//    g_free(device_name);
//    g_free(log_prefix);
//
//    close(get_dc_sock(dci));
//
//    set_dc_sock(dci, -1);

    return NULL;
}



gpointer fr_device_thread_func(gpointer data)
{
	guchar (*init_lib_func)(FRLibConfig config);
	guchar (*close_lib_func)();

	guint8 device_index = *(guint8*)data;

	unlock_threads_mutex();

	FiscalRegisterConf device = {0x00};
	get_fr_conf(device_index, &device);

	guint8 fri = 0;

	if (!get_new_fiscal_register_index(&fri))
	{
		g_printf("%s : error getting new fiscal register index\n", device.name);
		set_fr_thread_status(device_index, ts_Finished);
		g_printf("%s : set thread status is finished\n", device.name);

		return NULL;
	}
	else
	{
		g_printf("%s : fri = %d, %d\n", device.name, fri, device.file_size);
	}

	set_fiscal_register_index(device_index, fri);

	if (!init_fr_log_settings(fri, device.log_dir, device.name, device.log_enable, device.log_trace, device.file_size, device.save_days))
	{
		g_printf("%s : error init log settings\n", device.name);
		set_fiscal_register_index(device_index, 0);
		set_fr_thread_status(device_index, ts_Finished);
		return NULL;
	}

	LogParams* log_params = NULL;
	gboolean log_trace = FALSE;

	get_fr_log_settings(fri, &log_params, &log_trace);

	g_printf("%d, %d, %p, %s, %s, %d\n", log_params->enable, log_params->file_size, log_params->log, log_params->path, log_params->prefix, log_params->save_days);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "fiscal register index = %d", fri);

	set_fr_device_status(fri, frs_NotInitializeDriver);
	set_fr_device_name(fri, device.name);
	set_fr_device_port(fri, device.port);

	set_fr_thread_status(device_index, ts_Active);
	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "set thread status is active");

	GThread* main_sock_thread = g_thread_new("fr_main_sock_thread", fr_main_socket_thread_func, &fri);

	if (main_sock_thread == NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error starting main socket thread");
	}
	else
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "main socket thread is started");
		main_sock_thread->priority = G_THREAD_PRIORITY_LOW;
	}

	set_fr_device_is_working(fri, TRUE);

	long init_timer = get_date_time();

	while(get_fr_device_is_working(fri))
	{
		if (get_date_time() > init_timer + INIT_DEVICE_TIMEOUT)
		{
			init_timer = get_date_time();

			void* handle_lib = dlopen((gchar*)device.module_name, RTLD_LAZY );

			if (!handle_lib)
			{
				add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "error load module %s", device.module_name);
				set_fiscal_register_index(device_index, 0);
				set_fr_thread_status(device_index, ts_Finished);
				break;
			}
			else
			{
				add_log(log_params, TRUE, TRUE,log_trace, TRUE, "successfully load module %s", device.module_name);

				init_lib_func = dlsym( handle_lib, "init_lib" );
				gchar* error = dlerror();

				if (error != NULL)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "init function not found (%s)", error);
					g_free(error);
				}
				else
				{
					g_free(error);
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "initialization");

					guchar device_error = (*init_lib_func)(device.module_config);

					set_fr_device_last_error(fri, device_error);

					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "initialization complete with error = %d", device_error);

					if (device_error == fre_NoError)
					{
						add_log(log_params, TRUE, TRUE, log_trace, TRUE, "set fiscal register device working is true");

						guint64 timer = get_date_time();

						while(get_fr_device_is_working(fri))
						{
							if (get_date_time() > timer + device.interval)
							{
								timer = get_date_time();

								set_fr_device_status_from_driver(handle_lib, device.name, fri);

								FrDeviceStatus device_status = get_fr_device_status(fri);

								if (device_status == frs_NoError)
								{
//									HardwareServerCommand command = hsc_None;
//
//									guint64 datetime = 0;
//									gchar* operator_name = NULL;
//									gchar* operator_inn = NULL;
//									guint32 sum = 0;
//									FiscalReceipt	fiscal_receipt = {0x00};
//									guint32	doc_num = 0;
//									TextDoc text_doc = {0x00};

//											guint32 disp_num = 0;
//											guint8 nozzle_num = 0;
//											guint32 price = 0;
//											guint32 volume = 0;
//											guint32 amount = 0;
//											guint8 index_ext_func = 0;
//											DCPricePacks price_packs = {0x00};
//
//											guint8 index_client = find_dc_client_command(dci, i, &command, &disp_num, &nozzle_num, &price, &volume, &amount, &index_ext_func, &price_packs);
//
//											if (index_client != NO_CLIENT)
//											{
//												add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Finded client index %d command = %d (%s)", index_client, command,server_command_to_str(command));
//
//												switch(command)
//												{
//
//													case hsc_DCSetVolumeDose:
//														{
//															DcDeviceError result = send_volume_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price, volume);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set volume dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
//																	disp_num, nozzle_num, price, volume, result);
//														}
//														break;
//
//													case hsc_DCSetSumDose:
//														{
//															DcDeviceError result = send_sum_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price, amount);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set amount dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
//																	disp_num, nozzle_num, price, volume, result);
//														}
//														break;
//
//													case hsc_DCSetUnlimDose:
//														{
//															DcDeviceError result = send_full_tank_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set unlim dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
//																	disp_num, nozzle_num, price, volume, result);
//														}
//														break;
//
//													case hsc_DCStart:
//														{
//															DcDeviceError result = send_start_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set start for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCStop:
//														{
//															DcDeviceError result = send_stop_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client,  i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCSuspend:
//														{
//															DcDeviceError result = send_suspend_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client,  i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCResume:
//														{
//															DcDeviceError result = send_resume_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client,  i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCPayment:
//														{
//															DcDeviceError result = send_payment_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set payment for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCReset:
//														{
//															DcDeviceError result = send_reset_to_driver(handle_lib, device.name, dci, disp_num);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set reset for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//													case hsc_DCPriceUpdate:
//														{
//															DcDeviceError result = send_update_prices_to_driver(handle_lib, device.name, dci, disp_num, &price_packs);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set update prices for disp %d (result = %d)",
//																	disp_num, result);
//														}
//														break;
//
//
//													case hsc_DCExecuteExtendedFunction:
//														{
//															DcDeviceError result = send_extended_func_to_driver(handle_lib, device.name, dci, disp_num, index_ext_func);
//															set_dc_client_command_result(index_client, i, result);
//															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set extended command for disp %d, command index %d (result = %d)", disp_num, index_ext_func, result);
//														}
//														break;
//
//													default:
//														set_dc_client_command_result(index_client, i, dce_WrongCommand);
//														add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Wrond command for disp index %d (command = %d)",i, command);
//														break;
//												}
//											}
//
//											set_dispencer_state_from_driver(handle_lib, device.name, dci, i, log_params, log_trace);
//
//											if (get_dc_dispencer_data_is_change(dci, i))
//											{
//												add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Update dispencer state for all clients for disp index %d",i);
//												set_dc_disp_status_is_changed_for_all_clients(dci, i, TRUE);
//												set_dc_dispencer_data_is_change(dci, i, FALSE);
//											}
								}
								else
								{
									add_log(log_params, TRUE, TRUE, log_trace, TRUE, "DEVICE STATUS ERROR");
								}

								if (get_fr_device_status_is_changed(fri))
								{
									add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set device status is changed for all clients");
									set_fr_device_status_is_changed_for_all_clients(fri, TRUE, device_status);
									set_fr_device_status_is_changed(fri, FALSE);
								}
							}
						}

						add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "Send close lib");

						close_lib_func = dlsym( handle_lib, "close_lib" );
						error = dlerror();

						if (error != NULL)
						{
							add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "driver close func not found (%s)", error);
							g_free(error);
						}
						else
						{
							g_free(error);

							guchar device_error = (*close_lib_func)();
							add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "driver close func return %d", device_error);
						}
					}
				}
				dlclose(handle_lib);
			}
		}
	}
	if (main_sock_thread !=NULL)
	{
		set_fr_main_sock_thread_status(fri, ts_Finished);

		//--------------------------------------- stopping all client threads ------------------------------------------
		if (get_fr_sock(fri) >= 0)
		{
			close_fr_sock(fri);
		}

		g_thread_join(main_sock_thread);
		//--------------------------------------------------------------------------------------------------------------
	}

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finish device thread");

	set_fiscal_register_index(device_index, 0);

	set_fr_thread_status(device_index, ts_Finished);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finished");

	close_log(log_params);

	return NULL;
}
