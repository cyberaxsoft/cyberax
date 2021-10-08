/*
 ============================================================================
 Name        : hardware_server.c
 Author      : Konstantin Mazalov
*/

#include <glib.h>
#include <stdio.h>
#include <glib/gstdio.h>

#include "configuration.h"
#include "logger.h"
#include "tlv.h"

#include "socket_func.h"
#include "system_func.h"
#include "system_thread.h"

#include "dc_device.h"
#include "dc_device_data.h"
#include "dc_device_client_data.h"

#include "tgs_device.h"
#include "tgs_device_data.h"
#include "tgs_device_client_data.h"

#include "fr_device.h"
#include "fr_device_data.h"
#include "fr_device_client_data.h"

#include "ppc_device.h"
#include "ppc_device_data.h"
#include "ppc_device_client_data.h"

#include "sc_device.h"
#include "sc_device_data.h"
#include "sc_device_client_data.h"

LogParams log_params = {0x00};

GThread* 		system_thread = NULL;

int main(int argc, char *argv[])
{
	g_printf("system : starting...\n");

	configuration_initialization(argv[1]);

	sock_mutex_init();
	threads_mutex_init();

	if (g_thread_supported ())
	{
		system_init();

		dc_init();
		dc_client_init();

		tgs_init();
		tgs_client_init();

		ppc_init();
		ppc_client_init();

		sc_init();
		sc_client_init();

		if (read_conf())
		{
			while(TRUE)
			{
				if (get_common_conf_changed())
				{
					g_printf("System : common configuration is changed\n");

					CommonConf common_conf = {0x00};
					get_common_conf(&common_conf);

					g_printf("System : stopping system socket\n");

					close_system_sock();

					g_printf("System : stopping logger\n");

					if (log_params.log!=NULL)
					{
						close_log(&log_params);
					}

					destroy_log_params(&log_params);

			 		create_log_params(&log_params, common_conf.log_enable, common_conf.log_dir, SYSTEM_LOG_PREFIX, common_conf.file_size, common_conf.save_days);

					delete_old_logs(&log_params);

					if (common_conf.log_enable && log_params.log==NULL )
					{
						g_printf("System : start logger\n");

						gchar* system_log_path = g_strdup(common_conf.log_dir);

						if (!init_log_dir(&log_params))
						{
							g_printf("System : initialization log error\n");

							g_free(system_log_path);

							return -1;
						}

						g_printf("System : system log path: %s\n", system_log_path);

						create_log(&log_params);
						if (log_params.log == NULL)
						{
							g_printf("System : log create error\n");

							g_free(system_log_path);

							return -1;
						}

						g_free(system_log_path);

					}

					add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "Set system thread status is CONNECT_READY");
					set_system_sock_status(ss_ConnectReady);

					if (system_thread == NULL)
					{
						add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "Starting system thread...");

						system_thread = g_thread_new("system_thread", system_main_socket_thread_func, NULL);

						if (system_thread == NULL)
						{
							add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "error system thread starting");
						}
						else
						{
							add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "system thread successfully started");
							system_thread->priority = G_THREAD_PRIORITY_LOW;
						}
					}
					else
					{
						add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "system thread successfully restarted");
					}

					set_common_conf_changed(FALSE);

				}

				// dispencers
				guint8 dc_count = get_dc_count();

				if (dc_count > 0)
				{
					for (guint8 i = 0; i < dc_count; i++)
					{
						if (get_dc_conf_changed(i))
						{
							CommonConf common_conf = {0x00};
							get_common_conf(&common_conf);

							DispencerControllerConf dc_device = {0x00};
							get_dc_conf(i, &dc_device);

							if (get_dc_thread_status(i) == ts_Active)
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s is active. Stoppind thread...", dc_device.name);

								guint8 dci = get_dispencer_controller_index(i);

								set_dc_device_is_working(dci, FALSE);

								while(get_dc_thread_status(i) != ts_Finished);

								join_dc_thread(i);

								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s thread stopped", dc_device.name);

							}

							if (dc_device.enable )
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s enable. Starting thread device index %d...", dc_device.name, dc_device.index);

								lock_threads_mutex();

								if (start_dc_thread(i, dc_device.index))
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE,  "device %s thread started", dc_device.name);
								}
								else
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s error starting thread", dc_device.name);
								}

								wait_threads_mutex();

							}

							set_dc_conf_changed(i, FALSE);

						}
					}
				}

				// tgs
				guint8 tgs_count = get_tgs_count();

				if (tgs_count > 0)
				{
					for (guint8 i = 0; i < tgs_count; i++)
					{
						if (get_tgs_conf_changed(i))
						{
							CommonConf common_conf = {0x00};
							get_common_conf(&common_conf);

							TgsConf tgs_device = {0x00};
							get_tgs_conf(i, &tgs_device);

							if (get_tgs_thread_status(i) == ts_Active)
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s is active. Stoppind thread...", tgs_device.name);

								guint8 tgsi = get_tgs_index(i);

								set_tgs_device_is_working(tgsi, FALSE);

								while(get_tgs_thread_status(i) != ts_Finished);

								join_tgs_thread(i);

								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s thread stopped", tgs_device.name);
							}

							if (tgs_device.enable )
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s enable. Starting thread device index %d...", tgs_device.name, tgs_device.index);

								lock_threads_mutex();

								if (start_tgs_thread(i, tgs_device.index))
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE,  "device %s thread started", tgs_device.name);
								}
								else
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s error starting thread", tgs_device.name);
								}

								wait_threads_mutex();

							}

							set_tgs_conf_changed(i, FALSE);

						}
					}
				}
				// price pole controllers
				guint8 ppc_count = get_ppc_count();

				if (ppc_count > 0)
				{
					for (guint8 i = 0; i < ppc_count; i++)
					{
						if (get_ppc_conf_changed(i))
						{
							CommonConf common_conf = {0x00};
							get_common_conf(&common_conf);

							PpcConf ppc_device = {0x00};
							get_ppc_conf(i, &ppc_device);

							if (get_ppc_thread_status(i) == ts_Active)
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s is active. Stoppind thread...", ppc_device.name);

								guint8 ppci = get_ppc_index(i);

								set_ppc_device_is_working(ppci, FALSE);

								while(get_ppc_thread_status(i) != ts_Finished);

								join_ppc_thread(i);

								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s thread stopped", ppc_device.name);
							}

							if (ppc_device.enable )
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s enable. Starting thread device index %d...", ppc_device.name, ppc_device.index);

								lock_threads_mutex();

								if (start_ppc_thread(i, ppc_device.index))
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE,  "device %s thread started", ppc_device.name);
								}
								else
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s error starting thread", ppc_device.name);
								}

								wait_threads_mutex();

							}

							set_ppc_conf_changed(i, FALSE);

						}
					}
				}
				// sensor controllers
				guint8 sc_count = get_sc_count();

				if (sc_count > 0)
				{
					for (guint8 i = 0; i < sc_count; i++)
					{
						if (get_sc_conf_changed(i))
						{
							CommonConf common_conf = {0x00};
							get_common_conf(&common_conf);

							ScConf sc_device = {0x00};
							get_sc_conf(i, &sc_device);

							if (get_sc_thread_status(i) == ts_Active)
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s is active. Stoppind thread...", sc_device.name);

								guint8 sci = get_sc_index(i);

								set_sc_device_is_working(sci, FALSE);

								while(get_sc_thread_status(i) != ts_Finished);

								join_sc_thread(i);

								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s thread stopped", sc_device.name);
							}

							if (sc_device.enable )
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s enable. Starting thread device index %d...", sc_device.name, sc_device.index);

								lock_threads_mutex();

								if (start_sc_thread(i, sc_device.index))
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE,  "device %s thread started", sc_device.name);
								}
								else
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s error starting thread", sc_device.name);
								}

								wait_threads_mutex();

							}

							set_sc_conf_changed(i, FALSE);

						}
					}
				}

				// fiscal registers
				guint8 fr_count = get_fr_count();

				if (fr_count > 0)
				{
					for (guint8 i = 0; i < fr_count; i++)
					{
						if (get_fr_conf_changed(i))
						{
							CommonConf common_conf = {0x00};
							get_common_conf(&common_conf);

							FiscalRegisterConf fr_device = {0x00};
							get_fr_conf(i, &fr_device);

							if (get_fr_thread_status(i) == ts_Active)
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s is active. Stoppind thread...", fr_device.name);

								guint8 fri = get_fiscal_register_index(i);

								set_fr_device_is_working(fri, FALSE);

								while(get_fr_thread_status(i) != ts_Finished);

								join_fr_thread(i);

								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s thread stopped", fr_device.name);
							}

							if (fr_device.enable )
							{
								add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s enable. Starting thread device index %d...", fr_device.name, fr_device.index);

								lock_threads_mutex();

								if (start_fr_thread(i, fr_device.index))
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE,  "device %s thread started", fr_device.name);
								}
								else
								{
									add_log(&log_params, TRUE, TRUE, common_conf.log_trace, TRUE, "device %s error starting thread", fr_device.name);
								}

								wait_threads_mutex();

							}

							set_fr_conf_changed(i, FALSE);

						}
					}
				}

			}

			close_log(&log_params);

			g_printf("System : server stopped\n");

		}

		return EXIT_SUCCESS;

	}
	else
	{
		g_printf("system : threads not supported\n");

		return EXIT_FAILURE;

	}

}
