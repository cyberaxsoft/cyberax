/*
 ============================================================================
 Name        : pss.c
 Author      : Konstantin Mazalov
 Version     :
 Copyright   : (C) Copyright 2019 All rights reserved
 Description : C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include <dlfcn.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "pss.h"
#include "pss_tlv.h"
#include "pss_client_thread.h"
#include "pss_client_data.h"

#include "hardware_serv.h"
#include "pss_command_line.h"
#include "pss_data.h"
#include "pss_port_thread.h"

gchar* 	   config_filename	= NULL;
GMutex	   config_filename_mutex;

GMutex	   threads_mutex;

void get_conf_filename(gchar** value)
{
	g_mutex_lock(&config_filename_mutex);

	*value = g_strdup(config_filename);

	g_mutex_unlock(&config_filename_mutex);
}


void lock_threads_mutex()
{
	g_mutex_lock(&threads_mutex);
}

void unlock_threads_mutex()
{
	g_mutex_unlock(&threads_mutex);
}

gint main(gint argc, gchar *argv[])
{
	gchar* 		log_dir								= NULL;

	gboolean 	log_enable							= TRUE;
	gboolean 	log_trace 							= TRUE;
	gchar* 		ip_address 							= NULL;
	guint32 	ip_port 							= 0;
	guint32 	sensor_num 							= 0;
	guint32		sensor_param_num 					= 0;
	gchar* 		guid				 				= NULL;
	guint32		file_size							= 0;
	guint32		save_days							= 0;

	g_printf("PSS starting...\n");

	if (g_thread_supported ())
	{
		g_mutex_init(&config_filename_mutex);
		g_mutex_init(&threads_mutex);


		init_pss_data_mutex();
		init_hs_mutex();
		clients_init();
		init_sc_mutex();

		if (!parse_command_line(argc, argv, &log_dir, &config_filename, &guid, &ip_address, &log_enable, &log_trace, &ip_port, &sensor_num, &sensor_param_num, &file_size, &save_days))
		{
		    return EXIT_FAILURE;
		}

		set_fc_log_configuration(log_dir, log_enable, log_trace);

		set_door_settings(sensor_num, sensor_param_num);

		g_printf("Load PSS data %s\n", config_filename);

		if (load_pss_data())
		{
			LogParams log_params = {0x00};

	 		create_log_params(&log_params, log_enable, log_dir, SYSTEM_LOG_PREFIX, file_size, save_days);

			delete_old_logs(&log_params);

			if (log_enable && log_params.log==NULL )
			{
				if (!init_log_dir(&log_params))
				{
					g_printf("PSS : initialization log error\n");
					return -1;
				}

				g_printf("PSS : system log path: %s\n", log_dir);

				create_log(&log_params);

				if (log_params.log == NULL)
				{
					g_printf("System : log create error\n");
					return -1;
				}

				add_log(&log_params, TRUE, TRUE, log_trace, log_enable, "starting...");

			}

			PSSServConfThreadParam serv_conf_param = {.ip_address = g_strdup(ip_address), .port = ip_port, .guid = g_strdup(guid), .log_params = &log_params,
														.log_enable = log_enable, .log_trace = log_trace};

			GThread* serv_conf_thread = g_thread_new("serv_conf_thread", serv_conf_thread_func, &serv_conf_param);

			if (serv_conf_thread == NULL)
			{
				g_printf("Error starting server config thread");
			}
			else
			{
				serv_conf_thread = G_THREAD_PRIORITY_LOW;
			}

			while (!get_server_conf_is_load());

			PSSThreadParam param_supervised = {.port = SUPERVISED_PORT, .log_dir = g_strdup(log_dir), .log_enable = log_enable, .log_trace = log_trace, .file_size = file_size, .save_days = save_days};
			PSSThreadParam param_supervised_msg = {.port = SUPERVISED_MESSAGES_PORT, .log_dir = g_strdup(log_dir), .log_enable = log_enable, .log_trace = log_trace, .file_size = file_size, .save_days = save_days};
			PSSThreadParam param_unsupervised = {.port = UNSUPERVISED_PORT, .log_dir = g_strdup(log_dir), .log_enable = log_enable, .log_trace = log_trace, .file_size = file_size, .save_days = save_days};
			PSSThreadParam param_unsupervised_msg = {.port = UNSUPERVISED_MESSAGES_PORT, .log_dir = g_strdup(log_dir), .log_enable = log_enable, .log_trace = log_trace, .file_size = file_size, .save_days = save_days};

			GThread* supervised_sock_thread = g_thread_new("supervised_main_sock_thread", main_thread_func, &param_supervised);

			if (supervised_sock_thread == NULL)
			{
				add_log(&log_params, TRUE, TRUE, log_trace, TRUE, "error starting supervised socket thread");
			}
			else
			{
				supervised_sock_thread->priority = G_THREAD_PRIORITY_LOW;
			}

			GThread* supervised_messages_sock_thread = g_thread_new("supervised_messages_main_sock_thread", main_thread_func, &param_supervised_msg);

			if (supervised_messages_sock_thread == NULL)
			{
				add_log(&log_params, TRUE, TRUE, log_trace, TRUE, "error starting supervised messages socket thread");
			}
			else
			{
				supervised_messages_sock_thread->priority = G_THREAD_PRIORITY_LOW;
			}

			GThread* unsupervised_sock_thread = g_thread_new("unsupervised_main_sock_thread", main_thread_func, &param_unsupervised);

			if (unsupervised_sock_thread == NULL)
			{
				add_log(&log_params, TRUE, TRUE, log_trace, TRUE, "error starting unsupervised socket thread");
			}
			else
			{
				unsupervised_sock_thread->priority = G_THREAD_PRIORITY_LOW;
			}

			GThread* unsupervised_messages_sock_thread = g_thread_new("unsupervised_messages_main_sock_thread", main_thread_func, &param_unsupervised_msg);

			if (unsupervised_messages_sock_thread == NULL)
			{
				add_log(&log_params, TRUE, TRUE, log_trace, TRUE, "error starting unsupervised messages socket thread");
			}
			else
			{
				unsupervised_messages_sock_thread->priority = G_THREAD_PRIORITY_LOW;
			}
		}

	}
	while(TRUE);

}
