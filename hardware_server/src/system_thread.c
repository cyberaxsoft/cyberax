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
#include <unistd.h>

#include "logger.h"
#include "configuration.h"
#include "tlv.h"

#include "system_func.h"
#include "socket_func.h"
#include "system_message_func.h"
#include "sc_device.h"
#include "sc_func.h"

SystemClient system_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	system_clients_mutex;

gint32 main_system_sock = -1;
GMutex main_system_sock_mutex;

SocketStatus system_sock_status = ss_Disconnected;
GMutex system_sock_status_mutex;

void system_init()
{
	g_mutex_init(&system_clients_mutex);
	g_mutex_init(&system_sock_status_mutex);
	g_mutex_init(&main_system_sock_mutex);
}

gint32 get_main_system_sock()
{
	gint32 result = -1;

	g_mutex_lock(&main_system_sock_mutex);

	result = main_system_sock;

	g_mutex_unlock(&main_system_sock_mutex);

	return result;
}

void set_main_system_sock(gint32 new_value)
{
	g_mutex_lock(&main_system_sock_mutex);

	main_system_sock = new_value;

	g_mutex_unlock(&main_system_sock_mutex);
}

SocketStatus get_system_sock_status()
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&system_sock_status_mutex);

	result = system_sock_status;

	g_mutex_unlock(&system_sock_status_mutex);

	return result;
}

void set_system_sock_status(SocketStatus new_value)
{
	g_mutex_lock(&system_sock_status_mutex);

	system_sock_status = new_value;

	g_mutex_unlock(&system_sock_status_mutex);

}

gint8 find_new_system_client_index(gboolean clients_filtering)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&system_clients_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (system_clients[i].state == cls_Free)
		{
			system_clients[i].state = cls_Busy;

			if (clients_filtering)
			{
				system_clients[i].logged = FALSE;
				system_clients[i].access_level = cal_Disable;
			}
			else
			{
				system_clients[i].logged = TRUE;
				system_clients[i].access_level = cal_Unlim;
			}
			result = i;
			break;
		}
	}

	g_mutex_unlock(&system_clients_mutex);

	return result;
}

void set_new_system_client_param( guint8 client_index)
{
	g_mutex_lock(&system_clients_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[client_index].read_thread_is_active = FALSE;
		system_clients[client_index].state  = cls_Active;
	}

	g_mutex_unlock(&system_clients_mutex);
}

void set_client_state(guint8 client_index, ClientState new_value)
{
	g_mutex_lock(&system_clients_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[client_index].state  = new_value;
	}

	g_mutex_unlock(&system_clients_mutex);
}

ClientState get_client_state(guint8 client_index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&system_clients_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = system_clients[client_index].state;
	}

	g_mutex_unlock(&system_clients_mutex);

	return result;

}

void close_system_sock()
{
	gint32 sock = get_main_system_sock();

	if (sock >= 0)
	{

		shutdown(sock, SHUT_RDWR);

		set_system_sock_status(ss_Disconnected);

		g_mutex_lock(&system_clients_mutex);

		for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
		{
			if (system_clients[i].state > cls_Free &&
				system_clients[i].state < cls_Destroying)
			{
				if (system_clients[i].sock >= 0)
					shutdown(system_clients[i].sock, SHUT_RDWR);
				system_clients[i].state = cls_Destroying;
			}
		}
		g_mutex_unlock(&system_clients_mutex);


		while(TRUE)
		{
			gboolean exit_ready = TRUE;

			for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
			{
				if (get_client_state(i) != cls_Free)
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

gboolean get_system_client_read_thread_is_active(guint8 index_client)
{

	gboolean result = FALSE;

	g_mutex_lock(&system_clients_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		result = system_clients[index_client].read_thread_is_active;
	}

	g_mutex_unlock(&system_clients_mutex);

	return result;
}

void destroy_system_client( guint8 client_index)
{

	g_mutex_lock(&system_clients_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		close(system_clients[client_index].sock);

		system_clients[client_index].state = cls_Free;

	}

	g_mutex_unlock(&system_clients_mutex);

}

guint8 get_system_client_access_level_param(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&system_clients_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = system_clients[client_index].access_level;
	}

	g_mutex_unlock(&system_clients_mutex);

	return result;

}

void set_system_client_access_level_param(guint8 client_index, guint8 new_value)
{
	g_mutex_lock(&system_clients_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[client_index].access_level = new_value;
	}

	g_mutex_unlock(&system_clients_mutex);

}

void get_system_client_configuration(ProfilesConf* configuration, guchar* guid, gboolean* is_present, guint8* access_level )
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

gboolean get_system_client_logged(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&system_clients_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = system_clients[client_index].logged;
	}

	g_mutex_unlock(&system_clients_mutex);

	return result;

}

void set_system_client_logged(guint8 client_index, gboolean new_value)
{
	g_mutex_lock(&system_clients_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[client_index].logged = new_value;
	}

	g_mutex_unlock(&system_clients_mutex);

}

void set_system_client_read_thread_is_active(guint8 index_client, gboolean value)
{

	g_mutex_lock(&system_clients_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[index_client].read_thread_is_active = value;

	}

	g_mutex_unlock(&system_clients_mutex);
}

void set_system_client_sock(guint8 index_client, gint32 value)
{

	g_mutex_lock(&system_clients_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		system_clients[index_client].sock = value;

	}

	g_mutex_unlock(&system_clients_mutex);
}

ExchangeError ParseModuleLogSettingsPack(guchar* buffer, guint16 buffer_length, LibLogOptions* log_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Log config: ");


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmlst_Enable:
					if (unit->length == sizeof(guint8))
					{
						log_config->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, log_config->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_Directory:
					if (unit->length > 0)
					{
						if (log_config->dir!=NULL)
						{
							g_free(log_config->dir);
							log_config->dir = NULL;
						}

						log_config->dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (log_config->dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Tag %04x (length = %d) Dir = %s",client_info->socket, unit->tag, unit->length, log_config->dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Dir field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tmlst_Trace:
					if (unit->length == sizeof(guint8))
					{
						log_config->trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Trace = %0d",client_info->socket, unit->tag, unit->length, log_config->trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Trace field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_System:
					if (unit->length == sizeof(guint8))
					{
						log_config->system = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) System = %0d",client_info->socket, unit->tag, unit->length, log_config->system);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						System field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_Requests:
					if (unit->length == sizeof(guint8))
					{
						log_config->requests = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Requests = %0d",client_info->socket, unit->tag, unit->length, log_config->requests);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Requests field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_Frames:
					if (unit->length == sizeof(guint8))
					{
						log_config->frames = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Frames = %0d",client_info->socket, unit->tag, unit->length, log_config->frames);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Frames field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_Parsing:
					if (unit->length == sizeof(guint8))
					{
						log_config->parsing = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Parsing = %0d",client_info->socket, unit->tag, unit->length, log_config->parsing);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Parsing field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_FileSize:
					if (unit->length == sizeof(guint32))
					{
						log_config->file_size = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) File size = %d",client_info->socket, unit->tag, unit->length, log_config->file_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						File size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmlst_SaveDays:
					if (unit->length == sizeof(guint32))
					{
						log_config->save_days = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Save days = %d",client_info->socket, unit->tag, unit->length, log_config->save_days);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Save days field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseModuleConnSettingsPack(guchar* buffer, guint16 buffer_length, ConnOptions* conn_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Connection config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmcst_Type:
					if (unit->length == sizeof(guint8))
					{
						conn_config->connection_type = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Type = %0d",client_info->socket, unit->tag, unit->length, conn_config->connection_type);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Connection type field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmcst_Port:
					if (unit->length > 0)
					{
						if (conn_config->port!=NULL)
						{
							g_free(conn_config->port);
							conn_config->port = NULL;
						}

						conn_config->port = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (conn_config->port !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Tag %04x (length = %d) Port = %s",client_info->socket, unit->tag, unit->length, conn_config->port);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Port field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tmcst_IPAddress:
					if (unit->length > 0)
					{
						if (conn_config->ip_address!=NULL)
						{
							g_free(conn_config->ip_address);
							conn_config->ip_address = NULL;
						}

						conn_config->ip_address = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (conn_config->ip_address !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Tag %04x (length = %d) IP Address = %s",client_info->socket, unit->tag, unit->length, conn_config->ip_address);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						IP Address field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tmcst_IPPort:
					if (unit->length == sizeof(guint16))
					{
						conn_config->ip_port = (guint16)((unit->value[0] << 8 ) | unit->value[1]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) IP port = %d",client_info->socket, unit->tag, unit->length, conn_config->ip_port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						IP port field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmcst_UartBaudrate:
					if (unit->length == sizeof(guint32))
					{
						conn_config->uart_baudrate = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Uart baudrate= %d",client_info->socket, unit->tag, unit->length, conn_config->uart_baudrate);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Uart speed field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmcst_UartByteSize:
					if (unit->length == sizeof(guint8))
					{
						conn_config->uart_byte_size = unit->value[0];
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Uart byte size= %d",client_info->socket, unit->tag, unit->length, conn_config->uart_byte_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Uart byte size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmcst_UartParity:
					if (unit->length > 0)
					{
						if (conn_config->uart_parity!=NULL)
						{
							g_free(conn_config->uart_parity);
							conn_config->uart_parity = NULL;
						}

						conn_config->uart_parity = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (conn_config->uart_parity !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Tag %04x (length = %d) Uart parity= %s",client_info->socket, unit->tag, unit->length, conn_config->uart_parity);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d						Uart parity field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tmcst_UartStopBits:
					if (unit->length == sizeof(guint8))
					{
						conn_config->uart_stop_bits = unit->value[0];
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Uart stop bits= %d",client_info->socket, unit->tag, unit->length, conn_config->uart_stop_bits);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Uart stop bits field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseModuleTimeoutSettingsPack(guchar* buffer, guint16 buffer_length, TimeoutOptions* timeout_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Timeout config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmtst_Read:
					if (unit->length == sizeof(guint32))
					{
						timeout_config->t_read = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) TRead = %d",client_info->socket, unit->tag, unit->length, timeout_config->t_read);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						TRead field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmtst_Write:
					if (unit->length == sizeof(guint32))
					{
						timeout_config->t_write = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) TWrite = %d",client_info->socket, unit->tag, unit->length, timeout_config->t_read);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						TWrite field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseDCModuleDPSettingsPack(guchar* buffer, guint16 buffer_length, DCDecimalPointOptions* dp_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Decimal points config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tdcmdpst_Price:
					if (unit->length == sizeof(guint8))
					{
						dp_config->dp_price = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Price = %d",client_info->socket, unit->tag, unit->length, dp_config->dp_price);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Price field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdcmdpst_Volume:
					if (unit->length == sizeof(guint8))
					{
						dp_config->dp_volume = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Volume = %d",client_info->socket, unit->tag, unit->length, dp_config->dp_volume);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Volume field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdcmdpst_Amount:
					if (unit->length == sizeof(guint8))
					{
						dp_config->dp_amount = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Amount = %d",client_info->socket, unit->tag, unit->length, dp_config->dp_amount);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Amount field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParsePPModuleDPSettingsPack(guchar* buffer, guint16 buffer_length, PPCDecimalPointOptions* dp_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Decimal points config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmppcdpst_Price:
					if (unit->length == sizeof(guint8))
					{
						dp_config->dp_price = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Price = %d",client_info->socket, unit->tag, unit->length, dp_config->dp_price);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Price field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseNozzleConfigurationPack(guchar* buffer, guint16 buffer_length, NozzleConf* nozzle_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d							Nozzle: ",client_info->socket);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmnct_Number:
					if (unit->length == sizeof(guint32))
					{
						nozzle_config->num =  (guint8)(unit->value[0] );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, nozzle_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmnct_Grade:
					if (unit->length == sizeof(guint8))
					{
						nozzle_config->grade =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Tag %04x (length = %d) Grade = %d",client_info->socket, unit->tag, unit->length, nozzle_config->grade);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Grade field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d								Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseDispencerConfigurationPack(guchar* buffer, guint16 buffer_length, DispencerConf* disp_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d						Dispencer: ",client_info->socket);


	disp_config->nozzle_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmdct_Number:
					if (unit->length == sizeof(guint32))
					{
						disp_config->num =  (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, disp_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmdct_Address:
					if (unit->length == sizeof(guint8))
					{
						disp_config->addr =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Address = %d",client_info->socket, unit->tag, unit->length, disp_config->addr);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Address field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmdct_NozzleCount:
					if (unit->length == sizeof(guint8))
					{
						guint8 nozzle_count =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Nozzle count = %d",client_info->socket, unit->tag, unit->length, nozzle_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Nozzle count field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmdct_Nozzle:
					result = ParseNozzleConfigurationPack(unit->value, unit->length, &disp_config->nozzles[disp_config->nozzle_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					disp_config->nozzle_count++;


					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d							Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseTankConfigurationPack(guchar* buffer, guint16 buffer_length, TankConf* tank_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d						Tank: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmtct_Number:
					if (unit->length == sizeof(guint32))
					{
						tank_config->num =  (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, tank_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmtct_Channel:
					if (unit->length == sizeof(guint8))
					{
						tank_config->channel =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Channel = %d",client_info->socket, unit->tag, unit->length, tank_config->channel);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Channel field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d							Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParsePricePoleConfigurationPack(guchar* buffer, guint16 buffer_length, PricePoleConf* price_pole_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d						Price pole: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tppmct_Number:
					if (unit->length == sizeof(guint8))
					{
						price_pole_config->num =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, price_pole_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tppmct_Grade:
					if (unit->length == sizeof(guint8))
					{
						price_pole_config->grade =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Grade = %d",client_info->socket, unit->tag, unit->length, price_pole_config->grade);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Channel field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tppmct_SymbolCount:
					if (unit->length == sizeof(guint8))
					{
						price_pole_config->symbol_count =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) SYmbol count = %d",client_info->socket, unit->tag, unit->length, price_pole_config->symbol_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Channel field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d							Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseSensorParamConfigurationPack(guchar* buffer, guint16 buffer_length, SensorParamConf* param_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d							Param: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmspct_Number:
					if (unit->length == sizeof(guint8))
					{
						param_config->num =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, param_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d								Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmspct_Type:
					if (unit->length == sizeof(guint8))
					{
						param_config->type =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Type = %d (%s)",client_info->socket, unit->tag, unit->length, param_config->type, spt_to_str( param_config->type) );
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Type field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d							Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseSensorConfigurationPack(guchar* buffer, guint16 buffer_length, SensorConf* sensor_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d						Sensor: ",client_info->socket);

	sensor_config->param_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tmsct_Number:
					if (unit->length == sizeof(guint32))
					{
						sensor_config->num =  (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Number = %d",client_info->socket, unit->tag, unit->length, sensor_config->num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Number field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmsct_Addr:
					if (unit->length == sizeof(guint8))
					{
						sensor_config->addr =  (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Tag %04x (length = %d) Addr = %d",client_info->socket, unit->tag, unit->length, sensor_config->addr);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d							Addr field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tmsct_ParamConfiguration:
					result = ParseSensorParamConfigurationPack(unit->value, unit->length, &sensor_config->params[sensor_config->param_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					sensor_config->param_count++;




					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d							Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseDcModuleMappingPack(guchar* buffer, guint16 buffer_length, DCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Module mapping: ",client_info->socket);


	module_config->dispencer_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tdmmt_DispencerCount:
					if (unit->length == sizeof(guint8))
					{
						guint8 dispencer_count = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Dispencer count = %d",client_info->socket, unit->tag, unit->length, dispencer_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Dispencer count field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdmmt_DispencerConfiguration:
					result = ParseDispencerConfigurationPack(unit->value, unit->length, &module_config->dispencers[module_config->dispencer_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					module_config->dispencer_count++;


					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseTgsModuleMappingPack(guchar* buffer, guint16 buffer_length, TGSLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Module mapping: ",client_info->socket);


	module_config->tank_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case ttmmt_TankCount:
					if (unit->length == sizeof(guint8))
					{
						guint8 dispencer_count = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Dispencer count = %d",client_info->socket, unit->tag, unit->length, dispencer_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Dispencer count field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttmmt_TankConfiguration:
					result = ParseTankConfigurationPack(unit->value, unit->length, &module_config->tanks[module_config->tank_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					module_config->tank_count++;


					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParsePpcModuleMappingPack(guchar* buffer, guint16 buffer_length, PPCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Module mapping: ",client_info->socket);

	module_config->price_pole_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tppcmmt_PricePoleCount:
					if (unit->length == sizeof(guint8))
					{
						guint8 price_pole_count = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Price pole count = %d",client_info->socket, unit->tag, unit->length, price_pole_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Price pole count field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tppcmmt_PricePoleConfiguration:
					result = ParsePricePoleConfigurationPack(unit->value, unit->length, &module_config->price_poles[module_config->price_pole_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					module_config->price_pole_count++;


					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseScModuleMappingPack(guchar* buffer, guint16 buffer_length, SCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d					Module mapping: ",client_info->socket);

	module_config->sensor_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{

				case tscmmt_SensorCount:
					if (unit->length == sizeof(guint8))
					{
						guint8 sensor_count = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Sensor count = %d",client_info->socket, unit->tag, unit->length, sensor_count);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Sensor count field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscmmt_SensorConfiguration:
					result = ParseSensorConfigurationPack(unit->value, unit->length, &module_config->sensors[module_config->sensor_count], client_info);

					if (result != ee_None)
					{
						return result;
					}

					module_config->sensor_count++;


					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d						Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}


ExchangeError ParseDcModuleSettingsPack(guchar* buffer, guint16 buffer_length, DCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d				Module config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcmst_LogSettings:
					result = ParseModuleLogSettingsPack(unit->value, unit->length, &module_config->log_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_ConnSettings:
					result = ParseModuleConnSettingsPack(unit->value, unit->length, &module_config->conn_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_TimeoutSettings:
					result = ParseModuleTimeoutSettingsPack(unit->value, unit->length, &module_config->timeout_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_DPSettings:
					result = ParseDCModuleDPSettingsPack(unit->value, unit->length, &module_config->decimal_point_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_CountersEnable:
					if (unit->length == sizeof(guint8))
					{
						module_config->counters_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Tag %04x (length = %d) Counters enable = %0d",client_info->socket, unit->tag, unit->length, module_config->counters_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Counters enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdcmst_AutoStart:
					if (unit->length == sizeof(guint8))
					{
						module_config->auto_start = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Tag %04x (length = %d) Auto start = %0d",client_info->socket, unit->tag, unit->length, module_config->auto_start);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Auto start field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdcmst_AutoPayment:
					if (unit->length == sizeof(guint8))
					{
						module_config->auto_payment = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Tag %04x (length = %d) Auto payment = %d",client_info->socket, unit->tag, unit->length, module_config->auto_payment);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Auto payment field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdcmst_FullTankVolume:
					if (unit->length == sizeof(guint8))
					{
						module_config->full_tank_volume = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Tag %04x (length = %d) Full tank volume = %d",client_info->socket, unit->tag, unit->length, module_config->full_tank_volume);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d					Full tank volume field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;



				case tdcmst_ModuleMapping:
					result = ParseDcModuleMappingPack(unit->value, unit->length, module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d					Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;



}

ExchangeError ParseTgsModuleSettingsPack(guchar* buffer, guint16 buffer_length, TGSLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d				Module config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case ttgsmst_LogSettings:
					result = ParseModuleLogSettingsPack(unit->value, unit->length, &module_config->log_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case ttgsmst_ConnSettings:
					result = ParseModuleConnSettingsPack(unit->value, unit->length, &module_config->conn_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case ttgsmst_TimeoutSettings:
					result = ParseModuleTimeoutSettingsPack(unit->value, unit->length, &module_config->timeout_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;


				case ttgsmst_ModuleMapping:
					result = ParseTgsModuleMappingPack(unit->value, unit->length, module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d					Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;



}

ExchangeError ParsePpcModuleSettingsPack(guchar* buffer, guint16 buffer_length, PPCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d				Module config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tppcmst_LogSettings:
					result = ParseModuleLogSettingsPack(unit->value, unit->length, &module_config->log_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_ConnSettings:
					result = ParseModuleConnSettingsPack(unit->value, unit->length, &module_config->conn_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_TimeoutSettings:
					result = ParseModuleTimeoutSettingsPack(unit->value, unit->length, &module_config->timeout_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_DpSettings:
					result = ParsePPModuleDPSettingsPack(unit->value, unit->length, &module_config->decimal_point_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_ModuleMapping:
					result = ParsePpcModuleMappingPack(unit->value, unit->length, module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d					Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;



}

ExchangeError ParseScModuleSettingsPack(guchar* buffer, guint16 buffer_length, SCLibConfig* module_config, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d				Module config: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tscmst_LogSettings:
					result = ParseModuleLogSettingsPack(unit->value, unit->length, &module_config->log_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_ConnSettings:
					result = ParseModuleConnSettingsPack(unit->value, unit->length, &module_config->conn_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_TimeoutSettings:
					result = ParseModuleTimeoutSettingsPack(unit->value, unit->length, &module_config->timeout_options, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_ModuleMapping:
					result = ParseScModuleMappingPack(unit->value, unit->length, module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d					Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;



}


ExchangeError ParseDcConfPack(guchar* buffer, guint16 buffer_length, DispencerControllerConf* dc_conf, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Dispencer controller configuration pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdst_Id:
					if (unit->length == sizeof(guint8))
					{
						dc_conf->id = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Id = %0d",client_info->socket, unit->tag, unit->length, dc_conf->id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Id field parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case tdst_Name:
					if (unit->length > 0)
					{
						if (dc_conf->name!=NULL)
						{
							g_free(dc_conf->name);
							dc_conf->name = NULL;
						}

						dc_conf->name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (dc_conf->name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Name = %s",client_info->socket, unit->tag, unit->length, dc_conf->name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tdst_Port:
					if (unit->length == sizeof(guint16))
					{
						dc_conf->port = (guint16)((unit->value[0] << 8 ) | unit->value[1]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Port = %0d",client_info->socket, unit->tag, unit->length, dc_conf->port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Port field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_Enable:
					if (unit->length == sizeof(guint8))
					{
						dc_conf->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, dc_conf->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				case tdst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						dc_conf->command_timeout = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Command timeout = %0d",client_info->socket, unit->tag, unit->length, dc_conf->command_timeout);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Command timeout field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_Interval:
					if (unit->length == sizeof(guint32))
					{
						dc_conf->interval = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Interval = %0d",client_info->socket, unit->tag, unit->length, dc_conf->interval);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Interval field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_Module:
					if (unit->length > 0)
					{
						if (dc_conf->module_name!=NULL)
						{
							g_free(dc_conf->module_name);
							dc_conf->module_name = NULL;
						}

						dc_conf->module_name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (dc_conf->module_name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Module name = %s",client_info->socket, unit->tag, unit->length, dc_conf->module_name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Module name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tdst_LogDir:
					if (unit->length > 0)
					{
						if (dc_conf->log_dir!=NULL)
						{
							g_free(dc_conf->log_dir);
							dc_conf->log_dir = NULL;
						}

						dc_conf->log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (dc_conf->log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Log dir = %s",client_info->socket, unit->tag, unit->length, dc_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Log dir field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tdst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						dc_conf->log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log Enable = %0d",client_info->socket, unit->tag, unit->length, dc_conf->log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						dc_conf->log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log trace = %0d",client_info->socket, unit->tag, unit->length, dc_conf->log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log trace field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						dc_conf->file_size = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) File size = %d",client_info->socket, unit->tag, unit->length, dc_conf->file_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						File size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						dc_conf->save_days = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Save days = %d",client_info->socket, unit->tag, unit->length, dc_conf->save_days);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Save days field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tdst_ModuleSettings:
					result = ParseDcModuleSettingsPack(unit->value, unit->length, &dc_conf->module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;


}

ExchangeError ParseTgsConfPack(guchar* buffer, guint16 buffer_length, TgsConf* tgs_conf, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Dispencer controller configuration pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case ttst_Id:
					if (unit->length == sizeof(guint8))
					{
						tgs_conf->id = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Id = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Id field parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case ttst_Name:
					if (unit->length > 0)
					{
						if (tgs_conf->name!=NULL)
						{
							g_free(tgs_conf->name);
							tgs_conf->name = NULL;
						}

						tgs_conf->name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (tgs_conf->name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Name = %s",client_info->socket, unit->tag, unit->length, tgs_conf->name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case ttst_Port:
					if (unit->length == sizeof(guint16))
					{
						tgs_conf->port = (guint16)((unit->value[0] << 8 ) | unit->value[1]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Port = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Port field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_Enable:
					if (unit->length == sizeof(guint8))
					{
						tgs_conf->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				case ttst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						tgs_conf->command_timeout = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Command timeout = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->command_timeout);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Command timeout field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_Interval:
					if (unit->length == sizeof(guint32))
					{
						tgs_conf->interval = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Interval = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->interval);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Interval field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_Module:
					if (unit->length > 0)
					{
						if (tgs_conf->module_name!=NULL)
						{
							g_free(tgs_conf->module_name);
							tgs_conf->module_name = NULL;
						}

						tgs_conf->module_name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (tgs_conf->module_name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Module name = %s",client_info->socket, unit->tag, unit->length, tgs_conf->module_name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Module name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case ttst_LogDir:
					if (unit->length > 0)
					{
						if (tgs_conf->log_dir!=NULL)
						{
							g_free(tgs_conf->log_dir);
							tgs_conf->log_dir = NULL;
						}

						tgs_conf->log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (tgs_conf->log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Log dir = %s",client_info->socket, unit->tag, unit->length, tgs_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Log dir field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case ttst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						tgs_conf->log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log Enable = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						tgs_conf->log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log trace = %0d",client_info->socket, unit->tag, unit->length, tgs_conf->log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log trace field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						tgs_conf->file_size = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) File size = %d",client_info->socket, unit->tag, unit->length, tgs_conf->file_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						File size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						tgs_conf->save_days = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Save days = %d",client_info->socket, unit->tag, unit->length, tgs_conf->save_days);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Save days field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case ttst_ModuleSettings:
					result = ParseTgsModuleSettingsPack(unit->value, unit->length, &tgs_conf->module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;


}

ExchangeError ParsePpcConfPack(guchar* buffer, guint16 buffer_length, PpcConf* ppc_conf, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Price pole controller configuration pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tspt_Id:
					if (unit->length == sizeof(guint8))
					{
						ppc_conf->id = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Id = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Id field parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case tspt_Name:
					if (unit->length > 0)
					{
						if (ppc_conf->name!=NULL)
						{
							g_free(ppc_conf->name);
							ppc_conf->name = NULL;
						}

						ppc_conf->name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (ppc_conf->name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Name = %s",client_info->socket, unit->tag, unit->length, ppc_conf->name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tspt_Port:
					if (unit->length == sizeof(guint16))
					{
						ppc_conf->port = (guint16)((unit->value[0] << 8 ) | unit->value[1]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Port = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Port field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_Enable:
					if (unit->length == sizeof(guint8))
					{
						ppc_conf->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				case tspt_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						ppc_conf->command_timeout = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Command timeout = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->command_timeout);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Command timeout field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_Interval:
					if (unit->length == sizeof(guint32))
					{
						ppc_conf->interval = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Interval = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->interval);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Interval field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_Module:
					if (unit->length > 0)
					{
						if (ppc_conf->module_name!=NULL)
						{
							g_free(ppc_conf->module_name);
							ppc_conf->module_name = NULL;
						}

						ppc_conf->module_name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (ppc_conf->module_name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Module name = %s",client_info->socket, unit->tag, unit->length, ppc_conf->module_name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Module name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tspt_LogDir:
					if (unit->length > 0)
					{
						if (ppc_conf->log_dir!=NULL)
						{
							g_free(ppc_conf->log_dir);
							ppc_conf->log_dir = NULL;
						}

						ppc_conf->log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (ppc_conf->log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Log dir = %s",client_info->socket, unit->tag, unit->length, ppc_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Log dir field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tspt_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						ppc_conf->log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log Enable = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						ppc_conf->log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log trace = %0d",client_info->socket, unit->tag, unit->length, ppc_conf->log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log trace field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						ppc_conf->file_size = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) File size = %d",client_info->socket, unit->tag, unit->length, ppc_conf->file_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						File size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						ppc_conf->save_days = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Save days = %d",client_info->socket, unit->tag, unit->length, ppc_conf->save_days);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Save days field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspt_ModuleSettings:
					result = ParsePpcModuleSettingsPack(unit->value, unit->length, &ppc_conf->module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;


}

ExchangeError ParseScConfPack(guchar* buffer, guint16 buffer_length, ScConf* sc_conf, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Sensor controller configuration pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tscst_Id:
					if (unit->length == sizeof(guint8))
					{
						sc_conf->id = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Id = %0d",client_info->socket, unit->tag, unit->length, sc_conf->id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Id field parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case tscst_Name:
					if (unit->length > 0)
					{
						if (sc_conf->name!=NULL)
						{
							g_free(sc_conf->name);
							sc_conf->name = NULL;
						}

						sc_conf->name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (sc_conf->name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Name = %s",client_info->socket, unit->tag, unit->length, sc_conf->name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tscst_Port:
					if (unit->length == sizeof(guint16))
					{
						sc_conf->port = (guint16)((unit->value[0] << 8 ) | unit->value[1]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Port = %0d",client_info->socket, unit->tag, unit->length, sc_conf->port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Port field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_Enable:
					if (unit->length == sizeof(guint8))
					{
						sc_conf->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, sc_conf->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;


				case tscst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						sc_conf->command_timeout = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Command timeout = %0d",client_info->socket, unit->tag, unit->length, sc_conf->command_timeout);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Command timeout field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_Interval:
					if (unit->length == sizeof(guint32))
					{
						sc_conf->interval = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Interval = %0d",client_info->socket, unit->tag, unit->length, sc_conf->interval);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Interval field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_Module:
					if (unit->length > 0)
					{
						if (sc_conf->module_name!=NULL)
						{
							g_free(sc_conf->module_name);
							sc_conf->module_name = NULL;
						}

						sc_conf->module_name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (sc_conf->module_name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Module name = %s",client_info->socket, unit->tag, unit->length, sc_conf->module_name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Module name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tscst_LogDir:
					if (unit->length > 0)
					{
						if (sc_conf->log_dir!=NULL)
						{
							g_free(sc_conf->log_dir);
							sc_conf->log_dir = NULL;
						}

						sc_conf->log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (sc_conf->log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Log dir = %s",client_info->socket, unit->tag, unit->length, sc_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Log dir field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tscst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						sc_conf->log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log Enable = %0d",client_info->socket, unit->tag, unit->length, sc_conf->log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						sc_conf->log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log trace = %0d",client_info->socket, unit->tag, unit->length, sc_conf->log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log trace field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						sc_conf->file_size = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) File size = %d",client_info->socket, unit->tag, unit->length, sc_conf->file_size);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						File size field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						sc_conf->save_days = (guint32)((unit->value[0] << 24 ) | (unit->value[1] << 16 ) | (unit->value[2] << 8 ) | unit->value[3]  );
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Tag %04x (length = %d) Save days = %d",client_info->socket, unit->tag, unit->length, sc_conf->save_days);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d						Save days field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tscst_ModuleSettings:
					result = ParseScModuleSettingsPack(unit->value, unit->length, &sc_conf->module_config, client_info);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;


}

ExchangeError ParseClientProfilePack(guchar* buffer, guint16 buffer_length, Profile* profile, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Server common settings pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tspct_Id:
					if (unit->length == sizeof(guint8))
					{
						profile->id = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Id = %0d",client_info->socket, unit->tag, unit->length, profile->id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Id field parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case tspct_Name:
					if (unit->length > 0)
					{
						if (profile->name!=NULL)
						{
							g_free(profile->name);
							profile->name = NULL;
						}

						profile->name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (profile->name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Name = %s",client_info->socket, unit->tag, unit->length, profile->name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Name field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tspct_Enable:
					if (unit->length == sizeof(guint8))
					{
						profile->enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Enable = %0d",client_info->socket, unit->tag, unit->length, profile->enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Enable field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tspct_Guid:
					if (unit->length > 0)
					{
						if (profile->guid!=NULL)
						{
							g_free(profile->guid);
							profile->guid = NULL;
						}

						profile->guid = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (profile->guid !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) GUID = %s",client_info->socket, unit->tag, unit->length, profile->guid);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				GUID field parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tspct_AccessLevel:
					if (unit->length == sizeof(guint8))
					{
						profile->access_level = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Access level = %d",client_info->socket, unit->tag, unit->length, profile->access_level);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Access level field parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;

			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;

}

ExchangeError ParseServerCommonSettingsPack(guchar* buffer, guint16 buffer_length, CommonConf* common_conf, gboolean* clients_filtering, SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d			Server common settings pack: ",client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tsct_ProfilesEnable:
					if (unit->length == sizeof(guint8))
					{
						*clients_filtering = (guint8)(unit->value[0]);
						add_log(client_info->log_params,TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Clients filtering = %0d",client_info->socket, unit->tag, unit->length, *clients_filtering);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Clients filtering parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_ServerName:
					if (unit->length > 0)
					{
						if (common_conf->server_name!=NULL)
						{
							g_free(common_conf->server_name);
							common_conf->server_name = NULL;
						}

						common_conf->server_name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (common_conf->server_name !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Server name = %s",client_info->socket, unit->tag, unit->length, common_conf->server_name);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Server name parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tsct_ConfigurationPort:
					if (unit->length == sizeof(guint16))
					{
						common_conf->port = (guint16)((unit->value[0] << 8) | unit->value[1]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Configuration port = %d",client_info->socket, unit->tag, unit->length, common_conf->port);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Configuration port parse error!",client_info->socket);
						return ee_Parse;
					}

					break;

				case tsct_LogDir:
					if (unit->length > 0)
					{
						if (common_conf->log_dir!=NULL)
						{
							g_free(common_conf->log_dir);
							common_conf->log_dir = NULL;
						}

						common_conf->log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (common_conf->log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Log dir = %s",client_info->socket, unit->tag, unit->length, common_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Log dir parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tsct_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						common_conf->log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log enable = %0d",client_info->socket, unit->tag, unit->length, common_conf->log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log enable parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						common_conf->log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Log trace = %0d",client_info->socket, unit->tag, unit->length, common_conf->log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log trace parse error!");
						return ee_Parse;
					}
					break;

				case tsct_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						common_conf->file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) LogFileSize = %d",client_info->socket, unit->tag, unit->length, common_conf->file_size);

					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log file size parse error!");
						return ee_Parse;
					}
					break;

				case tsct_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						common_conf->save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) LogSaveDays = %d",client_info->socket, unit->tag, unit->length, common_conf->save_days);

					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Log save days parse error!");
						return ee_Parse;
					}
					break;

				case tsct_ConnLogDir:
					if (unit->length > 0)
					{
						if (common_conf->conn_log_dir!=NULL)
						{
							g_free(common_conf->conn_log_dir);
							common_conf->conn_log_dir = NULL;
						}

						common_conf->conn_log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (common_conf->conn_log_dir !=NULL)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Tag %04x (length = %d) Conn log dir = %s",client_info->socket, unit->tag, unit->length, common_conf->log_dir);

						}
						else
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
									"%d				Conn log dir parse error!",client_info->socket);
							return ee_Parse;
						}

					}
					break;

				case tsct_ConnLogEnable:
					if (unit->length == sizeof(guint8))
					{
						common_conf->conn_log_enable = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Conn log enable = %0d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_enable);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log enable parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_ConnLogTrace:
					if (unit->length == sizeof(guint8))
					{
						common_conf->conn_log_trace = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Conn log trace = %0d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_trace);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log trace parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_ConnLogFrames:
					if (unit->length == sizeof(guint8))
					{
						common_conf->conn_log_frames = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Conn log frames = %0d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_frames);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log frames parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_ConnLogParsing:
					if (unit->length == sizeof(guint8))
					{
						common_conf->conn_log_parsing = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Tag %04x (length = %d) Conn log parsing = %0d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_parsing);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log parsing parse error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tsct_ConnLogFileSize:
					if (unit->length == sizeof(guint32))
					{
						common_conf->conn_log_file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) ConnLogFileSize = %d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_file_size);

					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log file size parse error!");
						return ee_Parse;
					}
					break;

				case tsct_ConnLogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						common_conf->conn_log_save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) ConnLogSaveDays = %d",client_info->socket, unit->tag, unit->length, common_conf->conn_log_save_days);

					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d				Conn log save days parse error!");
						return ee_Parse;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d				Undefined Tag %04x (length = %d)",client_info->socket, unit->tag, unit->length);
					break;



			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}
ExchangeError ParseSystemRequest(guchar* buffer, guint16 buffer_length, guchar* guid, guint32* message_id, MessageType* message_type,
										HardwareServerCommand* command, CommonConf* common_conf, gboolean* clients_filtering, gboolean* common_conf_present,
										Profile* profile, gboolean* profile_present, DispencerControllerConf* dc_conf, gboolean* dc_conf_present,
										TgsConf* tgs_conf, gboolean* tgs_conf_present,
										PpcConf* ppc_conf, gboolean* ppc_conf_present,
										ScConf* sc_conf, gboolean* sc_conf_present,
										SystemClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params,TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d		Parsing frame (result = %d): ",client_info->socket, result);

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
								"%d			Tag %04x (length = %d) Guid = %s",client_info->socket, unit->tag, unit->length, guid);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Guid client parse Error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageId:
					if (unit->length == sizeof(guint32))
					{
						*message_id = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) MessageId = %d",client_info->socket, unit->tag, unit->length, *message_id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Message ID parse Error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_CommandCode:
					if (unit->length == sizeof(guint32))
					{
						*command = (HardwareServerCommand)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) CommandCode = %08X (%s)",client_info->socket, unit->tag, unit->length, *command, server_command_to_str(*command) );
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Command code parse Error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageType:
					if (unit->length == sizeof(guint8))
					{
						*message_type = (MessageType)unit->value[0];
						gchar* message_type_description = return_message_type_name((MessageType)unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Tag %04x (length = %d) Message type = %s",client_info->socket, unit->tag, unit->length, message_type_description);
						if (message_type_description != NULL)
						{
							g_free(message_type_description);
						}
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d			Message type parse Error!",client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_ServerCommonSettings:
					result = ParseServerCommonSettingsPack(unit->value, unit->length, common_conf, clients_filtering, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*common_conf_present = TRUE;
					}

					break;

				case tst_ClientProfile:
					result = ParseClientProfilePack(unit->value, unit->length, profile, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*profile_present = TRUE;
					}

					break;

				case tst_DispencerControllerConfig:
					result = ParseDcConfPack(unit->value, unit->length, dc_conf, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*dc_conf_present = TRUE;
					}
					break;

				case tst_TgsConfig:
					result = ParseTgsConfPack(unit->value, unit->length, tgs_conf, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*tgs_conf_present = TRUE;
					}
					break;

				case tst_PpcConfig:
					result = ParsePpcConfPack(unit->value, unit->length, ppc_conf, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*ppc_conf_present = TRUE;
					}
					break;

				case tst_ScConfig:
					result = ParseScConfPack(unit->value, unit->length, sc_conf, client_info);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						*sc_conf_present = TRUE;
					}
					break;

				default:
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
													"%d			Undefined Tag %04x (length = %d)",client_info->socket, client_info->socket, unit->tag, unit->length);
					break;
			}
			unit = unit->next;

		}while(unit != NULL);
	}
	return result;
}

gpointer system_client_read_thread_func(gpointer data)
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

	SystemClientInfo client_info = *(SystemClientInfo*)data;

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d starting read thread ", client_info.socket);

	ProfilesConf profiles;

	get_profiles_conf(&profiles);

	SockClientInfo socket_client_info = {.device_name = g_strdup_printf("%d", client_info.socket),
										.ip_address = g_strdup(client_info.ip_address),
										.log_params = client_info.log_params,
										.log_frames = client_info.log_frames,
										.log_parsing = client_info.log_parsing,
										.log_trace = client_info.log_trace,
										.socket = client_info.socket,
										.socket_mutex = &client_info.socket_mutex};
	ExchangeState es = es_WaitStartMessage;

	while(TRUE)
	{

		int bytes_read = recv(client_info.socket, bufr, 1024, 0);
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
								gboolean profiles_enable = FALSE;

								CommonConf common_conf = {.conn_log_dir = NULL, .conn_log_enable = FALSE, .conn_log_frames = FALSE, .conn_log_parsing = FALSE, .conn_log_trace = FALSE,
											.log_dir = NULL, .log_enable = FALSE, .log_trace = FALSE, .port = 0, .server_name = NULL};

								gboolean common_conf_present = FALSE;

								Profile profile = {.id = 0, .name = NULL, .enable = FALSE, .guid = NULL, .access_level = cal_Disable};

								gboolean profile_present = FALSE;

								DispencerControllerConf dc_conf = {0x00};

								gboolean dc_conf_present = FALSE;

								TgsConf tgs_conf = {0x00};

								gboolean tgs_conf_present = FALSE;

								PpcConf ppc_conf = {0x00};

								gboolean ppc_conf_present = FALSE;

								ScConf sc_conf = {0x00};

								gboolean sc_conf_present = FALSE;


								ExchangeError parse_result = ParseSystemRequest(bufrd, pos_bufrd, guid, &message_id, &message_type, &command,
													&common_conf, &profiles_enable, &common_conf_present,
													&profile, &profile_present,
													&dc_conf, &dc_conf_present,
													&tgs_conf, &tgs_conf_present,
													&ppc_conf, &ppc_conf_present,
													&sc_conf, &sc_conf_present,
													&client_info);

								add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, client_info.log_parsing, "%d Parse request result: %d", client_info.socket, parse_result);


								if (parse_result == ee_None && message_type == mt_Request && command!= hsc_None)
								{

									gboolean client_is_present = FALSE;

									guint8 client_access_level = get_system_client_access_level_param(client_info.client_index);

									if (strlen((gchar*)guid) > 0)
									{
										get_system_client_configuration(&profiles, guid, &client_is_present, &client_access_level);

										set_system_client_access_level_param(client_info.client_index, client_access_level);

										if (client_is_present)
										{
											set_system_client_logged(client_info.client_index, TRUE);
										}
									}
									else
									{
										if (get_system_client_logged(client_info.client_index))
										{
											client_is_present = TRUE;
										}
									}


									if (client_is_present)
									{
										set_system_client_logged(client_info.client_index, TRUE);

										switch(command)
										{
											case hsc_Reset:
												set_reset();
												break;

											case hsc_GetCommonServerConfig:
												send_system_common_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_UpdateCommonServerConfig:
												if (common_conf_present)
												{
													update_common_conf(common_conf);
													set_profiles_enable(profiles_enable);
													write_conf();
													send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
												}
												break;

											case hsc_GetClientProfiles:
												send_system_client_profile_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_AddClientProfile:
												if (profile_present)
												{
													if (profile.id > 0)
													{
														if (add_client_profile(profile))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_UpdateClientProfile:
												if (profile_present)
												{
													if (profile.id > 0)
													{
														if (update_client_profile(profile))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_DeleteClientProfile:
												if (profile_present)
												{
													if (profile.id > 0)
													{
														if (delete_client_profile(profile))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_GetDispencerControllerConfig:
												send_system_dispencer_controller_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_AddDispencerController:
												if (dc_conf_present)
												{
													if (dc_conf.id > 0)
													{
														if (add_dc_conf(dc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}

												break;

											case hsc_UpdateDispencerController:
												if (dc_conf_present)
												{
													if (dc_conf.id > 0)
													{
														if (update_dc_conf(dc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_DeleteDispencerController:
												if (dc_conf_present)
												{
													if (dc_conf.id > 0)
													{
														if (delete_dc_conf(dc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_GetTgsConfig:
												send_system_tgs_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_AddTgs:
												if (tgs_conf_present)
												{
													if (tgs_conf.id > 0)
													{
														if (add_tgs_conf(tgs_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}

												break;

											case hsc_UpdateTgs:
												if (tgs_conf_present)
												{
													if (tgs_conf.id > 0)
													{
														if (update_tgs_conf(tgs_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_DeleteTgs:
												if (tgs_conf_present)
												{
													if (tgs_conf.id > 0)
													{
														if (delete_tgs_conf(tgs_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_GetPricePoleControllerConfig:
												send_system_price_pole_controller_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_AddPricePoleController:
												if (ppc_conf_present)
												{
													if (ppc_conf.id > 0)
													{
														if (add_ppc_conf(ppc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}

												break;

											case hsc_UpdatePricePoleController:
												if (ppc_conf_present)
												{
													if (ppc_conf.id > 0)
													{
														if (update_ppc_conf(ppc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_DeletePricePoleController:
												if (ppc_conf_present)
												{
													if (ppc_conf.id > 0)
													{
														if (delete_ppc_conf(ppc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_GetSensorControllerConfig:
												send_system_sensor_controller_configuration_message(&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_AddSensorController:
												if (sc_conf_present)
												{
													if (sc_conf.id > 0)
													{
														if (add_sc_conf(sc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}

												break;

											case hsc_UpdateSensorController:
												if (sc_conf_present)
												{
													if (sc_conf.id > 0)
													{
														if (update_sc_conf(sc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

													}
												}
												break;

											case hsc_DeleteSensorController:
												if (sc_conf_present)
												{
													if (sc_conf.id > 0)
													{
														if (delete_sc_conf(sc_conf))
														{
															write_conf();
															send_system_command_result_message(&socket_client_info, message_id, command, ee_None);
														}
														else
														{
															send_system_command_result_message(&socket_client_info, message_id, command, ee_OutOfRange);
														}
													}
													else
													{
														send_system_command_result_message(&socket_client_info, message_id, command, ee_FaultParam);

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

	set_system_client_read_thread_is_active(client_info.client_index, FALSE);

	return NULL;
}

gpointer system_client_thread_func(gpointer data)
{
	SystemClientInfo client_info = *(SystemClientInfo*)data;

	set_new_system_client_param(client_info.client_index);

	g_mutex_init(&client_info.socket_mutex);

	add_log(client_info.log_params,  TRUE, TRUE, client_info.log_trace, TRUE, "%d starting main system client thread socket", client_info.socket);

	GThread* client_read_thread = g_thread_new("system_client_read_thread", system_client_read_thread_func, &client_info);

	if (client_read_thread == NULL)
	{
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d error starting system client read thread", client_info.socket);
	}
	else
	{
		set_system_client_read_thread_is_active(client_info.client_index, TRUE);

		client_read_thread->priority = G_THREAD_PRIORITY_LOW;
	}

	while(get_system_client_read_thread_is_active(client_info.client_index));

	g_thread_join(client_read_thread);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d disconnected client socket", client_info.socket);

	destroy_system_client(client_info.client_index);

	set_system_client_sock(client_info.client_index, -1);

	return NULL;
}

void connect_main_system_socket(LogParams* log_params, gboolean log_trace, guint32 port)
{
    gint32 old_sock = get_main_system_sock();

	gint32 sock = create_serv_sock(old_sock, port);

    set_main_system_sock(sock);

    if(sock < 0)
    {
    	add_log(log_params,  TRUE, TRUE, log_trace, TRUE, "main system socket create error");
    	set_system_sock_status(ss_ConnectReady);

    }
    else
    {
    	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "main system socket create");
       	set_system_sock_status(ss_Connected);
    }
}

gpointer system_main_socket_thread_func(gpointer data)
{
	LogParams log_params = {0x00};
    CommonConf common_conf = {0x00};

	guint64 last_connect_time = 0;

  	while(TRUE)
   	{

  		if (get_system_sock_status() == ss_ConnectReady && get_date_time() > last_connect_time + SOCKET_RECONNECT_TIMEOUT)
   		{
  			get_common_conf(&common_conf);

  			if ( common_conf.port > 0)
  			{
  				if (log_params.log!=NULL)
  				{
  					close_log(&log_params);
  				}

  				destroy_log_params(&log_params);

  				create_log_params(&log_params, common_conf.conn_log_enable, common_conf.conn_log_dir, SYSTEM_CONNECT_LOG_PREFIX, common_conf.conn_log_file_size, common_conf.conn_log_save_days);

  				delete_old_logs(&log_params);

  				if (common_conf.conn_log_enable && log_params.log==NULL )
  				{
  					g_printf("%s : start system connect logger\n",SYSTEM_CONNECT_LOG_PREFIX);

  					gchar* system_log_path = g_strdup(common_conf.conn_log_dir);

  					if (!init_log_dir(&log_params))
  					{
  						g_printf("%s : initialization system connect log error\n",SYSTEM_CONNECT_LOG_PREFIX);
  					}

  					g_printf("%s : system connect log path: %s\n",SYSTEM_CONNECT_LOG_PREFIX, system_log_path);

  					create_log(&log_params);

  					if (log_params.log == NULL)
  					{
  						g_printf("%s : system connect log create error\n",SYSTEM_CONNECT_LOG_PREFIX);
  					}

  					g_free(system_log_path);
  				}

  				connect_main_system_socket(&log_params, common_conf.conn_log_trace, common_conf.port);
  				last_connect_time = get_date_time();
  			}
        }

        if (get_system_sock_status() == ss_Connected )
        {
        	struct sockaddr client_addr;
        	socklen_t client_addr_len = 0;

        	gint32 main_sock = get_main_system_sock();

       		gint32 sock = accept(main_sock, &client_addr, &client_addr_len);

       		if(sock < 0)
       		{
       			add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace,TRUE,  "main system socket accepting error");
       		}
       		else
       		{
               	gboolean pe = TRUE;
               	get_profiles_enable(&pe);

               	guint8 client_index = find_new_system_client_index(pe);

               	if (client_index != NO_CLIENT)
               	{
        			struct sockaddr_in addr;
        			socklen_t addr_size = sizeof(struct sockaddr_in);
        			getpeername(sock, (struct sockaddr *)&addr, &addr_size);

        			set_system_client_sock(client_index, sock);

        			add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace,TRUE,  "accepted client socket %d (%s)", sock, inet_ntoa(addr.sin_addr));

        			SystemClientInfo info = {.client_index = client_index,
        									 .socket = sock,
											 .ip_address = g_strdup( inet_ntoa(addr.sin_addr)),
											 .log_params = &log_params,
        									 .log_trace = common_conf.conn_log_trace,
        									 .log_frames = common_conf.conn_log_frames,
        									 .log_parsing = common_conf.conn_log_parsing};

        			GThread* client_thread = g_thread_new("system_client_thread", system_client_thread_func, &info);

        			if (client_thread == NULL)
        			{
        				add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE, "%d error starting system client thread socket (%s)", sock, inet_ntoa(addr.sin_addr));
        			}
        			else
        			{
        				client_thread->priority = G_THREAD_PRIORITY_LOW;
        			}
        		}
               	else
               	{
               		add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace,TRUE,  "client list is full. Disconnecting client");
               		close(sock);
               	}
        	}
        }
    }

    return NULL;
}


