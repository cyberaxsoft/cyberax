#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <dlfcn.h>
#include <unistd.h>

#include "logger.h"
#include "configuration.h"
#include "tlv.h"

#include "system_func.h"
#include "socket_func.h"
#include "sc_device.h"

#include "sc_device_data.h"
#include "sc_device_client_data.h"
#include "sc_device_driver_func.h"
#include "sc_device_message_func.h"

void close_sc_sock(guint8 index)
{
	if (get_sc_sock(index) >= 0)
	{
		gint32 sock = get_sc_sock(index);

		shutdown(sock, SHUT_RDWR);

		set_sc_sock_status(index, ss_Disconnected);

		set_sc_clients_status_is_destroying();

		while(TRUE)
		{
			gboolean exit_ready = TRUE;

			for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
			{
				if ( get_client_sci(i) == index && get_sc_client_state(i) != cls_Free)
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

ExchangeError ParseSCRequest(guchar* buffer, guint16 buffer_length, guchar* guid, guint32* message_id, MessageType* message_type, HardwareServerCommand* command, guint32* sensor_num, SCClientInfo* client_info, gchar* log_prefix)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d Parsing frame (result = %d): ", client_info->socket, result);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tst_GuidClient:
					if (unit->length > 0 && unit->length < GUID_LENGTH)
					{
						memcpy(guid, unit->value, unit->length);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Guid = %s", client_info->socket, unit->tag, unit->length, guid);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageId:
					if (unit->length == sizeof(guint32))
					{
						*message_id = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) MessageId = %d", client_info->socket, unit->tag, unit->length, *message_id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_CommandCode:
					if (unit->length == sizeof(guint32))
					{
						*command = (HardwareServerCommand)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) CommandCode = %08X (%s)", client_info->socket, unit->tag, unit->length, *command, server_command_to_str(*command) );
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageType:
					if (unit->length == sizeof(guint8))
					{
						*message_type = (MessageType)unit->value[0];
						gchar* message_type_description = return_message_type_name((MessageType)unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Message type = %s", client_info->socket, unit->tag, unit->length, message_type_description);
						if (message_type_description != NULL)
						{
							g_free(message_type_description);
						}
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_DeviceNumber:
					if (unit->length == sizeof(guint32))
					{
						*sensor_num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Device number = %0d", client_info->socket, unit->tag, unit->length, *sensor_num);
					}
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}

	return result;

}

void get_sc_client_configuration(ProfilesConf* configuration, guchar* guid, gboolean* is_present, guint8* access_level )
{
	if (configuration->enable == FALSE)
	{
		*is_present = TRUE;
		*access_level = cal_Unlim;

	}
	else
	{
		*is_present = FALSE;
		*access_level = cal_Disable;
		if ( configuration->profiles_count > 0 )
		{
			for (guint8 i = 0; i < configuration->profiles_count; i++)
			{
	        	if (strcmp((gchar*)guid,configuration->profiles[i].guid) == 0)
	        	{
	        		*is_present = TRUE;
	        		*access_level = configuration->profiles[i].access_level;
	        		break;
	        	}
			}
		}
	}
}

gpointer sc_device_client_read_thread_func(gpointer data)
{
	guchar bufrd[EXCHANGE_BUFFER_SIZE];
	guint16 pos_bufrd_tmp = 0;
	guint16 pos_bufrd = 0;

	guchar bufr[SOCKET_BUFFER_SIZE];

	gboolean bad_frame = FALSE;
	guint8 bad_frame_count = 0;
	gboolean finish_message = FALSE;
	guint16 crc = 0;
	guint16 calc_crc = 0;
	guint32 data_len = 0;

	SCClientInfo client_info = *(SCClientInfo*)data;

	SockClientInfo socket_client_info = {.device_name = g_strdup(client_info.device_name),
										.ip_address = g_strdup(client_info.ip_address),
										.log_params = client_info.log_params,
										.log_frames = client_info.log_frames,
										.log_parsing = client_info.log_parsing,
										.log_trace = client_info.log_trace,
										.socket = client_info.socket,
										.socket_mutex = &client_info.socket_mutex};

	ProfilesConf profiles = {0x00};

	get_profiles_conf(&profiles);

	ExchangeState es = es_WaitStartMessage;

	gchar* log_prefix = g_strdup_printf("%s_connect", client_info.device_name);

	while(TRUE)
	{

		int bytes_read = recv(client_info.socket, bufr, 8192, 0);
		if(bytes_read <= 0)
		{
			break;
		}
		else
		{
			for (guint32 i = 0; i < bytes_read; i++)
			{
				switch(es)
				{
					case es_None:
						break;

					case es_WaitStartMessage:
						if (bufr[i] == ctrl_STX)
						{
							memset(bufrd, 0x00, EXCHANGE_BUFFER_SIZE);
							pos_bufrd_tmp = 0;
							pos_bufrd = 0;
							bad_frame = FALSE;
							bad_frame_count = 0;
							finish_message = FALSE;
							crc = 0;
							calc_crc = 0;
							es = es_ReadLength1;
						}
						break;

					case es_WaitSTX:
						if (bufr[i] == ctrl_STX)
						{

							bad_frame = FALSE;
							crc = 0;
							calc_crc = 0;
							es = es_ReadLength1;
						}
						break;

					case es_ReadLength1:
						data_len = bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength2;
						break;

					case es_ReadLength2:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength3;
						break;

					case es_ReadLength3:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength4;
						break;

					case es_ReadLength4:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);

						if (data_len > 0)
						{
							es = es_ReadData;
						}
						else
						{
							es = es_WaitEndMessage;
						}
						break;

					case es_ReadData:
						bufrd[pos_bufrd_tmp++] = bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						data_len--;
						if (data_len == 0)
						{
							es = es_WaitEndMessage;
						}
						break;

					case es_WaitEndMessage:
						calc_crc = calc_crc_next(bufr[i], calc_crc);

						switch (bufr[i])
						{
							case ctrl_ETX:
								finish_message = TRUE;
								break;

							case ctrl_ETB:
								finish_message = FALSE;
								break;

							default:
								bad_frame = TRUE;

						}
						es = es_WaitCRC1;
						break;

					case es_WaitCRC1:
						crc = bufr[i];
						es = es_WaitCRC2;
						break;

					case es_WaitCRC2:
						crc = (crc << 8) | bufr[i];

						if (crc == calc_crc && !bad_frame)
						{
							pos_bufrd = pos_bufrd_tmp;

							if (finish_message)
							{
								if (client_info.log_frames)
								{
									add_log(client_info.log_params, TRUE, FALSE, client_info.log_trace, client_info.log_frames, "%d << ", client_info.socket);

									for (guint16 i = 0; i < pos_bufrd; i++)
									{
										add_log(client_info.log_params, FALSE, FALSE, client_info.log_trace, client_info.log_frames, " %02X", bufrd[i]);

									}
									add_log(client_info.log_params, FALSE, TRUE, client_info.log_trace, client_info.log_frames, "");
								}

								guchar guid[GUID_LENGTH] = {0x00};
								guint32 message_id = 0;
								MessageType message_type = mt_Undefined;
								HardwareServerCommand command = hsc_None;
								guint32 sensor_num = 0;

								ExchangeError parse_result = ParseSCRequest(bufrd, pos_bufrd, guid, &message_id, &message_type, &command, &sensor_num, &client_info, log_prefix);

								add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, client_info.log_parsing, "%d Parse request result: %d", client_info.socket, parse_result);

								if (parse_result == ee_None  && message_type == mt_Request)
								{
									gboolean client_is_present = FALSE;

									guint8 client_access_level = get_sc_client_access_level_param(client_info.client_index);

									if (strlen((gchar*)guid) > 0)
									{
										get_sc_client_configuration(&profiles, guid, &client_is_present, &client_access_level);

										set_sc_client_access_level_param(client_info.client_index, client_access_level);

										if (client_is_present)
										{
											set_sc_client_logged(client_info.client_index, TRUE);
										}
									}
									else
									{
										if (get_sc_client_logged(client_info.client_index))
										{
											client_is_present = TRUE;
										}
									}

									if (client_is_present)
									{
										set_sc_client_logged(client_info.client_index, TRUE);

										switch(command)
										{
											case hsc_GetDeviceStatus:
												send_sc_device_status_message(client_info.client_index, client_info.sensor_controller_index,&socket_client_info, message_id, mt_Reply, log_prefix);
												break;

											case hsc_GetConfiguration:
												send_sc_device_configuration_message(client_info.client_index, client_info.sensor_controller_index,&socket_client_info, message_id, mt_Reply, log_prefix);
												break;

											case hsc_SCGetSensorData:
												{
													guint8 sensor_index = 0;
													if (sensor_num == 0 || sc_sensor_num_is_present(client_info.sensor_controller_index, sensor_num, &sensor_index))
													{
														send_sc_device_data_message(client_info.client_index, client_info.sensor_controller_index,&socket_client_info, message_id, mt_Reply, sensor_num, sensor_index, log_prefix);
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												break;

											default:
												send_short_message(&socket_client_info, mt_Nak, ee_UndefineCommandCode);
												break;

										}
									}
								}
								else
								{
									send_short_message(&socket_client_info, mt_Nak, parse_result);
								}
								es = es_WaitStartMessage;
							}
							else
							{
								send_short_message(&socket_client_info, mt_Ack, ee_None);
							}
						}
						else
						{
							pos_bufrd_tmp = pos_bufrd;
							bad_frame_count++;

							if (bad_frame_count >= MAX_BAD_FRAME)
							{
								send_short_message(&socket_client_info, mt_Eot, ee_LimitBadFrame);
								es = es_WaitStartMessage;
							}
							else
							{
								if (bad_frame)
								{
									send_short_message(&socket_client_info, mt_Nak, ee_Format);
								}
								else
								{
									send_short_message(&socket_client_info, mt_Nak, ee_Crc);
								}
								es = es_WaitSTX;
							}
						}
						break;
				}
			}
		}
	}

	set_sc_client_read_thread_is_active(client_info.client_index, FALSE);

	g_free(log_prefix);

	return NULL;
}

gpointer sc_device_client_thread_func(gpointer data)
{
	SCClientInfo client_info = *(SCClientInfo*)data;

	g_mutex_init(&client_info.socket_mutex);

	SockClientInfo socket_client_info = {.device_name = g_strdup(client_info.device_name),
										.ip_address = g_strdup(client_info.ip_address),
										.log_params = client_info.log_params,
										.log_frames = client_info.log_frames,
										.log_parsing = client_info.log_parsing,
										.log_trace = client_info.log_trace,
										.socket = client_info.socket,
										.socket_mutex = &client_info.socket_mutex};

	gchar* log_prefix = g_strdup_printf("%s_connect",client_info.device_name);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d starting main client thread", client_info.socket );

	GThread* client_read_thread = g_thread_new("sc_client_read_thread", sc_device_client_read_thread_func, &client_info);

	if (client_read_thread == NULL)
	{
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d error starting client read thread", client_info.socket);
	}
	else
	{
		client_read_thread->priority = G_THREAD_PRIORITY_LOW;
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d set read thread priority is low", client_info.socket );
		set_sc_client_read_thread_is_active(client_info.client_index, TRUE);
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d set read thread is active", client_info.socket );

	}

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d main loop", client_info.socket );

	while(get_sc_client_read_thread_is_active(client_info.client_index))
	{
		if (get_sc_device_status_is_changed_for_client(client_info.client_index) && get_sc_client_logged(client_info.client_index))
		{
			send_sc_device_status_message(client_info.client_index,client_info.sensor_controller_index,&socket_client_info, 0, mt_Event, log_prefix);

			set_sc_device_status_is_changed_for_client(client_info.client_index, FALSE);
		}

		guint8 sensor_count = get_client_sensor_info_count( client_info.client_index );

		if (sensor_count > 0)
		{
			for (guint i = 0; i < sensor_count; i++)
			{
				guint32 sensor_num = get_client_sensor_num( client_info.client_index, i);

				if (get_client_sensor_status_is_changed( client_info.client_index, i) && get_sc_client_logged(client_info.client_index))
				{
					send_sc_device_data_message(client_info.client_index,client_info.sensor_controller_index,&socket_client_info, 0, mt_Event, sensor_num, i, log_prefix);
					set_client_sensor_status_is_changed( client_info.client_index, i, FALSE);
				}
			}
		}
	}

	g_thread_join(client_read_thread);

	destroy_sc_client(client_info.client_index);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d destroy sc client", client_info.socket);

	close(client_info.socket);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d close socket", client_info.socket);

	set_sc_client_sock(client_info.client_index, -1);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d thread ended", client_info.socket);

	g_free(log_prefix);

	return NULL;
}

void connect_sc_socket(guint8 index, LogParams* log_params, gboolean log_trace, guint32 port)
{
    gint32 old_sock = get_sc_sock(index);

	gint32 sock = create_serv_sock(old_sock, port);

	set_sc_sock(index, sock);

    if(sock < 0)
    {
    	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "SC socket create error");
    	set_sc_sock_status(index, ss_ConnectReady);
    }
    else
    {
    	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "SC socket create");
    	set_sc_sock_status(index, ss_Connected);
    }

}

gpointer sc_main_socket_thread_func(gpointer data)
{
	guint8 sci = *(guint8*)data;

	gchar* device_name = get_sc_device_name(sci);
	guint32 device_port = get_sc_device_port(sci);

	LogParams log_params = {0x00};

	CommonConf common_conf = {0x00};
	get_common_conf(&common_conf);

	gchar* log_prefix = g_strdup_printf("%s_connect", device_name);

	create_log_params(&log_params, common_conf.conn_log_enable, common_conf.conn_log_dir, log_prefix, common_conf.file_size, common_conf.save_days);

	if (common_conf.conn_log_enable)
	{
		g_printf("%s : start connect logger\n",log_prefix);

		if (!init_log_dir(&log_params))
		{
			g_printf("%s : initialization log error\n",log_prefix);
		}

		g_printf("%s : system log path: %s\n",log_prefix, common_conf.conn_log_dir);

		create_log(&log_params);

		if (log_params.log == NULL)
		{
			g_printf("%s : log create error\n",log_prefix);
		}
	}

	connect_sc_socket(sci, &log_params, common_conf.conn_log_trace, device_port);
	guint64 last_connect_time = get_date_time();

	set_sc_main_sock_thread_status(sci, ts_Active);

	gboolean accept_error_sended = FALSE;

	while(get_sc_sock_status(sci)!= ss_Disconnected)
	{
   		if (get_sc_sock_status(sci) == ss_ConnectReady && get_date_time() > last_connect_time + SOCKET_RECONNECT_TIMEOUT  && device_port > 0)
   		{
   			connect_sc_socket(sci, &log_params, common_conf.conn_log_trace, device_port);
        	last_connect_time = get_date_time();
   			accept_error_sended = FALSE;
        }

        if (get_sc_sock_status(sci) == ss_Connected )
        {
        	struct sockaddr client_addr;
        	socklen_t client_addr_len = 0;

        	gint32 sc_sock = get_sc_sock(sci);

       		gint32 client_socket = accept(sc_sock, &client_addr, &client_addr_len);

       		if(client_socket < 0)
       		{
       			if (!accept_error_sended)
       			{
       				add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client socket accepting error");
       				accept_error_sended = TRUE;
       			}
       		}
       		else
       		{
               	gboolean pe = TRUE;
               	get_profiles_enable(&pe);

               	guint8 client_index = find_new_sc_client_index(pe);

               	if (client_index != NO_CLIENT)
               	{
               		set_new_sc_client_param(client_index, sci);

               		set_sc_client_sock(client_index, client_socket);

        			struct sockaddr_in addr;
        			socklen_t addr_size = sizeof(struct sockaddr_in);
        			getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);

        			add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "accepted client socket %d (%s)", client_socket, inet_ntoa(addr.sin_addr));

    				SCClientInfo info = { .client_index = client_index,
    															.sensor_controller_index = sci,
    															.socket = client_socket,
    															.device_name = device_name,
    															.ip_address = g_strdup( inet_ntoa(addr.sin_addr)),
																.log_params = &log_params,
																.log_enable = common_conf.conn_log_enable,
																.log_frames = common_conf.conn_log_frames,
																.log_parsing = common_conf.conn_log_parsing,
																.log_trace = common_conf.conn_log_trace
    															};

    				GThread* client_thread = g_thread_new("sc_client_thread", sc_device_client_thread_func, &info);

    				if (client_thread == NULL)
    				{
    					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "%d error starting client thread socket (%s)", client_socket, inet_ntoa(addr.sin_addr));
    				}
    				else
    				{
    					client_thread->priority = G_THREAD_PRIORITY_LOW;
    				}
        		}
               	else
               	{
					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client list is full. Disconnecting client");
               		close(client_socket);
               	}
        	}
        }
	}

    g_free(device_name);
    g_free(log_prefix);

    close(get_sc_sock(sci));

    set_sc_sock(sci, -1);

    return NULL;
}

gpointer sensor_controller_device_thread_func(gpointer data)
{
	guchar (*init_lib_func)(SCLibConfig config);
	guchar (*close_lib_func)();

	guint8 device_index = *(guint8*)data;

	unlock_threads_mutex();

	ScConf device = {0x00};
	get_sensor_controller_conf(device_index, &device);

	guint8 sci = 0;

	if (!get_new_sc_index(&sci))
	{
		g_printf("%s : error getting new sc index\n", device.name);
		set_sc_thread_status(device_index, ts_Finished);
		g_printf("%s : set thread status is finished\n", device.name);

		return NULL;
	}

	g_printf("%s : sc index = %d\n", device.name, sci);

	set_sc_index(device_index, sci);

	if (!init_sc_log_settings(sci, device.log_dir, device.name, device.log_enable, device.log_trace, device.file_size, device.save_days))
	{
		g_printf("%s : error init log settings\n", device.name);
		set_sc_index(device_index, 0);
		set_sc_thread_status(device_index, ts_Finished);
		return NULL;
	}

	LogParams* log_params = NULL;
	gboolean log_trace = FALSE;

	get_sc_log_settings(sci, &log_params, &log_trace);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "device = %d", sci);

	set_sc_device_status(sci, scs_NotInitializeDriver);
	set_sc_device_name(sci, device.name);
	set_sc_device_port(sci, device.port);

	set_sc_thread_status(device_index, ts_Active);
	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "set thread status is active");

	GThread* main_sock_thread = g_thread_new("sc_main_sock_thread", sc_main_socket_thread_func, &sci);

	if (main_sock_thread == NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error starting main socket thread");
	}
	else
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "main socket thread is started");
		main_sock_thread->priority = G_THREAD_PRIORITY_LOW;
	}

	set_sc_device_is_working(sci, TRUE);

	long init_timer = get_date_time();

	while(get_sc_device_is_working(sci))
	{
		if (get_date_time() > init_timer + INIT_DEVICE_TIMEOUT)
		{
			init_timer = get_date_time();

			void* handle_lib = dlopen((gchar*)device.module_name, RTLD_LAZY );

			if (!handle_lib)
			{
				add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "error load module %s", device.module_name);
				set_sc_index(device_index, 0);
				set_sc_thread_status(device_index, ts_Finished);
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

					set_sc_device_last_error(sci, device_error);

					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "initialization complete with error = %d", device_error);

					if (device_error == sce_NoError)
					{
						set_sensor_config_from_driver(handle_lib, device.name, sci, log_params, log_trace);

						add_log(log_params, TRUE, TRUE, log_trace, TRUE, "set sensor controller device working is true");

						guint64 timer = get_date_time();

						while(get_sc_device_is_working(sci))
						{
							if (get_date_time() > timer + device.interval)
							{
								timer = get_date_time();

								set_sc_device_status_from_driver(handle_lib, device.name, sci);

								ScDeviceStatus device_status = get_sc_device_status(sci);

								if (device_status == scs_NoError)
								{
									guint8 sensor_count = get_sc_sensor_count(sci);

									if (sensor_count > 0)
									{
										for (guint8 i = 0; i < sensor_count; i++)
										{
											set_sensor_state_from_driver(handle_lib, device.name, sci, i, log_params, log_trace);

											add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Get sensor index %d",i);

											if (get_sc_sensor_data_is_change(sci, i))
											{
												add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Update sensor state for all clients for sensor index %d",i);
												set_sc_sensor_status_is_changed_for_all_clients(sci, i, TRUE);
												set_sc_sensor_data_is_change(sci, i, FALSE);
											}
										}
									}
									else
									{
										break;
									}
								}

								if (get_sc_device_status_is_changed(sci))
								{
									add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set device status is changed for all clients");
									set_sc_device_status_is_changed_for_all_clients(sci, TRUE, device_status);
									set_sc_device_status_is_changed(sci, FALSE);
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
		set_sc_main_sock_thread_status(sci, ts_Finished);

		//--------------------------------------- stopping all client threads ------------------------------------------
		if (get_sc_sock(sci) >= 0)
		{
			close_sc_sock(sci);
		}

		g_thread_join(main_sock_thread);
		//--------------------------------------------------------------------------------------------------------------
	}

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finish device thread");

	set_sc_index(device_index, 0);

	set_sc_thread_status(device_index, ts_Finished);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finished");

	close_log(log_params);

	return NULL;
}

