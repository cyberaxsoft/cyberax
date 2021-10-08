#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_data.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_func.h"

#include "pss_dc_device.h"
#include "pss_tgs_device.h"
#include "pss_ppc_device.h"
#include "pss_sc_device.h"

PSSClientInfo pss_clients[PSS_MAX_CLIENT_COUNT] = {0x00};

void clients_init()
{
	for (guint8 i = 0; i < PSS_MAX_CLIENT_COUNT; i++)
	{
		pss_clients[i].sock = -1;
		pss_clients[i].ip_address = 0;
		pss_clients[i].state = cs_Free;
		pss_clients[i].exchange_status  = ces_Undefined;
		pss_clients[i].port = 0;
		pss_clients[i].fp_status_mode = 3;
		pss_clients[i].tgs_status_mode = 1;
		pss_clients[i].tr_buf_status_mode = 1;

		if (pss_clients[i].mutex.p == NULL)
		{
			g_mutex_init(&pss_clients[i].mutex);
		}

		for (guint8 j = 0; j < MAX_SERVER_DEVICE_COUNT; j++)
		{
			pss_clients[i].devices[j].id_device = 0;
			pss_clients[i].devices[j].sock = -1;
			pss_clients[i].devices[j].port = 0;
			pss_clients[i].devices[j].ip_address = NULL;
			pss_clients[i].devices[j].thread = NULL;
			pss_clients[i].devices[j].message_id = 0;
			pss_clients[i].devices[j].thread_status = ts_Undefined;

			pss_clients[i].devices[j].current_command = hsc_None;
			pss_clients[i].devices[j].current_command_sent_time = 0;
			pss_clients[i].devices[j].pss_command = pssc_None;
			pss_clients[i].devices[j].pss_command_subcode = 0;
			pss_clients[i].devices[j].ex_mode = FALSE;

			pss_clients[i].devices[j].unit_ids.count = 0;
			memset(pss_clients[i].devices[j].unit_ids.units, 0x00, sizeof(pss_clients[i].devices[j].unit_ids.units));

		}
	}
}

guint32 get_client_port(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].port;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

guint32 get_client_sock(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].sock;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_sock(guint8 client_index, guint32 new_value)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].sock = new_value;

	g_mutex_unlock(&pss_clients[client_index].mutex);
}

guint8 get_client_fp_status_mode(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].fp_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

guint8 get_client_tgs_status_mode(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].tgs_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

guint8 get_client_tr_buf_status_mode(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].tr_buf_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_fp_status_mode(guint8 client_index, guint8 new_status_mode)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].fp_status_mode = new_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);
}

void set_client_tgs_status_mode(guint8 client_index, guint8 new_status_mode)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].tgs_status_mode = new_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);
}

void set_client_tr_buf_status_mode(guint8 client_index, guint8 new_status_mode)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].tr_buf_status_mode = new_status_mode;

	g_mutex_unlock(&pss_clients[client_index].mutex);
}


guint32 get_client_device_sock(guint8 client_index, guint8 device_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].devices[device_index].sock;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

guint32 get_client_device_message_id(guint8 client_index, guint8 device_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].devices[device_index].message_id;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void increment_client_device_message_id(guint8 client_index, guint8 device_index)
{

	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].devices[device_index].message_id++;

	g_mutex_unlock(&pss_clients[client_index].mutex);

}

HardwareServerCommand get_current_command(guint8 client_index, guint8 device_index)
{
	HardwareServerCommand result = hsc_None;

	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		result = pss_clients[client_index].devices[device_index].current_command;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
	return result;
}

void set_current_command(guint8 client_index, guint8 device_index, HardwareServerCommand new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].devices[device_index].current_command = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

PSSCommand get_pss_command(guint8 client_index, guint8 device_index)
{
	PSSCommand result = hsc_None;

	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		result = pss_clients[client_index].devices[device_index].pss_command;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
	return result;
}

void set_pss_command(guint8 client_index, guint8 device_index, PSSCommand new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].devices[device_index].pss_command = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

guint8 get_pss_command_subcode(guint8 client_index, guint8 device_index)
{
	guint8 result = 0;

	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		result = pss_clients[client_index].devices[device_index].pss_command_subcode;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
	return result;
}

void set_pss_command_subcode(guint8 client_index, guint8 device_index, guint8 new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].devices[device_index].pss_command_subcode = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

guint8 get_pss_ex_mode(guint8 client_index, guint8 device_index)
{
	gboolean result = FALSE;

	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		result = pss_clients[client_index].devices[device_index].ex_mode;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
	return result;
}

void set_pss_ex_mode(guint8 client_index, guint8 device_index, gboolean new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].devices[device_index].ex_mode = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

guint64 get_current_command_sent_time(guint8 client_index, guint8 device_index)
{
	guint64 result = 0;

	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		result = pss_clients[client_index].devices[device_index].current_command_sent_time;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
	return result;
}

void set_current_command_sent_time(guint8 client_index, guint8 device_index, guint64 new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].devices[device_index].current_command_sent_time = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}


gboolean get_client_device_index_by_id(guint8 client_index, guint8 id, guint8* device_index, PSSServerDeviceType device_type)
{
	gboolean result = FALSE;

	g_mutex_lock(&pss_clients[client_index].mutex);

	for (guint8 i = 0; i < MAX_SERVER_DEVICE_COUNT; i++)
	{
		if (pss_clients[client_index].devices[i].sock > 0  && pss_clients[client_index].devices[i].device_type == device_type)
		{
			if (pss_clients[client_index].devices[i].unit_ids.count > 0)
			{
				for (guint8 j = 0; j < pss_clients[client_index].devices[i].unit_ids.count; j++)
				{
					if (pss_clients[client_index].devices[i].unit_ids.units[j] == id)
					{
						*device_index = i;
						result = TRUE;
						break;
					}
				}
			}
		}
		if (result) break;
	}

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

PSSServerDeviceType get_client_device_type_by_index(guint8 client_index, guint8 device_index)
{
	PSSServerDeviceType result = psdt_System;

	g_mutex_lock(&pss_clients[client_index].mutex);

	if ((client_index < PSS_MAX_CLIENT_COUNT) && device_index < MAX_SERVER_DEVICE_COUNT)
	{
		result = pss_clients[client_index].devices[device_index].device_type;

	}

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

ThreadStatus get_client_device_thread_status(guint8 client_index, guint8 device_index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].devices[device_index].thread_status;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;

}

void free_client_device_thread(guint8 client_index, guint8 device_index)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	g_thread_join(pss_clients[client_index].devices[device_index].thread);
	pss_clients[client_index].devices[device_index].thread = NULL;

	pss_clients[client_index].devices[device_index].thread_status = ts_Undefined;

	g_mutex_unlock(&pss_clients[client_index].mutex);

}


void set_client_device_thread_status(guint8 client_index, guint8 device_index, ThreadStatus new_value)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	pss_clients[client_index].devices[device_index].thread_status = new_value;

	g_mutex_unlock(&pss_clients[client_index].mutex);

}

void set_client_port(guint8 client_index, guint32 new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].port = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

PSSClientState get_client_state(guint8 client_index)
{
	PSSClientState result = cs_Free;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].state;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_state(guint8 client_index, PSSClientState new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].state = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

void clear_client(guint8 client_index)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].state = cs_Free;
		pss_clients[client_index].ip_address = 0;
		pss_clients[client_index].port = 0;
		pss_clients[client_index].sock = -1;
		pss_clients[client_index].exchange_status= ces_Undefined;

		for (guint8 j = 0; j < MAX_SERVER_DEVICE_COUNT; j++)
		{
			pss_clients[client_index].devices[j].device_type = psdt_System;
			pss_clients[client_index].devices[j].id_device = 0;
			pss_clients[client_index].devices[j].sock = -1;
			pss_clients[client_index].devices[j].port = 0;
			pss_clients[client_index].devices[j].ip_address = NULL;

			if (pss_clients[client_index].devices[j].thread !=NULL)
			{
				pss_clients[client_index].devices[j].thread_status = ts_Destroing;

				g_thread_join(pss_clients[client_index].devices[j].thread );
				pss_clients[client_index].devices[j].thread = NULL;

				pss_clients[client_index].devices[j].thread_status = ts_Undefined;

				pss_clients[client_index].devices[j].unit_ids.count = 0;
				memset(pss_clients[client_index].devices[j].unit_ids.units, 0x00, sizeof(pss_clients[client_index].devices[j].unit_ids.units));
			}
		}


		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

guint32 get_client_ip_address(guint8 client_index)
{
	guint32 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].ip_address;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_ip_address(guint8 client_index, guint32 new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].ip_address = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

gboolean set_busy_client(guint8 client_index)
{
	gboolean result= FALSE;

	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		if (pss_clients[client_index].state == cs_Free)
		{
			pss_clients[client_index].state = cs_Busy;
			result = TRUE;
		}

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}

	return result;
}

guint8 find_next_client_index()
{
	for (guint8 i = 0; i < PSS_MAX_CLIENT_COUNT; i++)
	{
		if (set_busy_client(i))
		{
			return i;
		}
	}
	return PSS_MAX_CLIENT_COUNT;
}


PSSClientExchangeState get_client_exchange_status(guint8 client_index)
{
	PSSClientExchangeState result = ces_Undefined;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].exchange_status;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_exchange_status(guint8 client_index, PSSClientExchangeState new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].exchange_status = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

guint64 get_client_last_exchange_time(guint8 client_index)
{
	guint64 result = 0;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].last_exchange_time;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_last_exchange_time(guint8 client_index, guint64 new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].last_exchange_time = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}

PSSClientType get_client_type(guint8 client_index)
{
	PSSClientType result = ct_Undefined;

	g_mutex_lock(&pss_clients[client_index].mutex);

	result = pss_clients[client_index].type;

	g_mutex_unlock(&pss_clients[client_index].mutex);

	return result;
}

void set_client_type(guint8 client_index, PSSClientType new_value)
{
	if (client_index < PSS_MAX_CLIENT_COUNT)
	{
		g_mutex_lock(&pss_clients[client_index].mutex);

		pss_clients[client_index].type = new_value;

		g_mutex_unlock(&pss_clients[client_index].mutex);
	}
}


void socket_client_send_with_mutex(PSSClientThreadFuncParam* params, guchar* buffer, guint32 size)
{
	LogParams* log_params = params->log_params;

	guint8 buf[EXCHANGE_BUFFER_SIZE] = {0x00};

	gint32 len = prepare_dpl_frame(buf, buffer, size);

	g_mutex_lock(&pss_clients[params->client_index].mutex);

	socket_send(pss_clients[params->client_index].sock, buf, len);

	add_log(log_params, TRUE, FALSE, params->log_trace, params->log_enable, "client %d >>", params->client_index);

	for (guint16 i = 0; i < len; i++)
	{
		add_log(log_params, FALSE, FALSE, params->log_trace, params->log_enable, " %02X", buf[i]);
	}
	add_log(log_params, FALSE, TRUE, params->log_trace, params->log_enable, "");

	g_mutex_unlock(&pss_clients[params->client_index].mutex);
}

void socket_client_device_send_with_mutex(PSSClientThreadFuncParam* params, guint8 device_index, guchar* buffer, guint32 size)
{
	LogParams* log_params = params->log_params;

	g_mutex_lock(&pss_clients[params->client_index].mutex);

	socket_send(pss_clients[params->client_index].devices[device_index].sock, buffer, size);

	add_log(log_params, TRUE, FALSE, params->log_trace, params->log_enable, "client %d device %d >>", params->client_index, device_index);

	for (guint16 i = 0; i < size; i++)
	{
		add_log(log_params, FALSE, FALSE, params->log_trace, params->log_enable, " %02X", buffer[i]);
	}
	add_log(log_params, FALSE, TRUE, params->log_trace, params->log_enable, "");

	g_mutex_unlock(&pss_clients[params->client_index].mutex);
}

void client_socket_close(guint8 client_index)
{
	g_mutex_lock(&pss_clients[client_index].mutex);

	close(pss_clients[client_index].sock);
	pss_clients[client_index].sock = -1;

	g_mutex_unlock(&pss_clients[client_index].mutex);
}

gint32 reconnect_to_server_device(PSSClientThreadFuncParam* params, guint8 device_index)
{
	LogParams* log_params = params->log_params;

	guint8 c_index = params->client_index;

	struct	sockaddr_in addr;

	gint32 result = -1;

	g_mutex_lock(&pss_clients[c_index].mutex);


	PSSClientDevice* device = &pss_clients[c_index].devices[device_index];

	set_fps_main_state(&device->unit_ids,fpms_Unavailable);
	set_fps_sub_state(&device->unit_ids, 0);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d reconnect device ID %d",
			c_index,device->id_device);

	if (device->sock > 0)
	{
		close(device->sock);
		device->sock = -1;
	}

	gint32 sock = socket(AF_INET, SOCK_STREAM, 0);

	if( sock < 0)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d socket create error",c_index);

	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d socket successfully create",c_index);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d set connection params: address %s port %d",
				c_index, device->id_device, device->ip_address, device->port);

		addr.sin_port = htons( device->port);
		addr.sin_addr.s_addr = inet_addr(device->ip_address);
		addr.sin_family = AF_INET;

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d connection params ready",
				c_index, device->id_device);

		if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d connection error (address = %s, port = %d)",
					c_index, device->id_device, device->ip_address, device->port);
			sock = -1;
		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,"client %d device ID %d is connected",
					c_index, device->id_device);
		}

		result = sock;
	}

	g_mutex_unlock(&pss_clients[c_index].mutex);

	return result;
}


void connect_to_server_device(PSSClientThreadFuncParam* params, guint8 device_index, PSSServerDevice* server_device)
{
	LogParams* log_params = params->log_params;

	struct	sockaddr_in addr;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d connect to server device ID %d", params->client_index,server_device->id);

	g_mutex_lock(&pss_clients[params->client_index].mutex);


	pss_clients[params->client_index].devices[device_index].sock = socket(AF_INET, SOCK_STREAM, 0);

	if(pss_clients[params->client_index].devices[device_index].sock < 0)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d socket create error", params->client_index,server_device->id);
	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d socket successfully create", params->client_index,server_device->id);
	}

	addr.sin_port = htons( server_device->port);
	addr.sin_addr.s_addr = inet_addr(server_device->ip_address);
	addr.sin_family = AF_INET;

	if(connect(pss_clients[params->client_index].devices[device_index].sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d connection error", params->client_index,server_device->id);
		pss_clients[params->client_index].devices[device_index].sock = -1;

		//set device fpms_Unavailable

		set_fps_main_state(&server_device->units,fpms_Unavailable);
		set_fps_sub_state(&server_device->units, 0);
	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d is connected", params->client_index,server_device->id);

	}

	pss_clients[params->client_index].devices[device_index].unit_ids = server_device->units;
	pss_clients[params->client_index].devices[device_index].id_device = server_device->id;
	pss_clients[params->client_index].devices[device_index].port = server_device->port;
	if (pss_clients[params->client_index].devices[device_index].ip_address!=NULL)
	{
		g_free(pss_clients[params->client_index].devices[device_index].ip_address);
		pss_clients[params->client_index].devices[device_index].ip_address = NULL;
	}
	pss_clients[params->client_index].devices[device_index].ip_address = g_strdup(server_device->ip_address);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d settings stored", params->client_index,server_device->id);

	g_mutex_unlock(&pss_clients[params->client_index].mutex);
}

void disconnect_server_devices(PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d disconnect server devices", params->client_index);

	g_mutex_lock(&pss_clients[params->client_index].mutex);

	for (guint8 i = 0; i < MAX_SERVER_DEVICE_COUNT; i++)
	{
		if (pss_clients[params->client_index].devices[i].thread_status == ts_Active)
		{
			pss_clients[params->client_index].devices[i].thread_status = ts_Destroing;
		}


		if (pss_clients[params->client_index].devices[i].sock > 0)
		{
			close(pss_clients[params->client_index].devices[i].sock);
			pss_clients[params->client_index].devices[i].sock = -1;
			pss_clients[params->client_index].devices[i].id_device = 0;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device ID %d socket close", params->client_index,pss_clients[params->client_index].devices[i].id_device);

		}

	}
	g_mutex_unlock(&pss_clients[params->client_index].mutex);

	gboolean all_device_threads_is_destroyed = FALSE;

	while (!all_device_threads_is_destroyed)
	{
		all_device_threads_is_destroyed = TRUE;

		for (guint8 i = 0; i < MAX_SERVER_DEVICE_COUNT; i++)
		{
			ThreadStatus ts = get_client_device_thread_status(params->client_index, i);

			if (ts == ts_Active || ts == ts_Destroing)
			{
				all_device_threads_is_destroyed = FALSE;
			}
			if (ts == ts_Destroyed)
			{
				free_client_device_thread(params->client_index, i);
			}
		}
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d disconnected server devices", params->client_index);

}



void connect_to_server_devices(PSSClientThreadFuncParam* client_params)
{
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d connect to server devices", client_params->client_index);

	PSSServerDevices server_devices = {0x00};

	get_server_devices(&server_devices);

	if (server_devices.count > 0)
	{
		for (guint8 i = 0; i < server_devices.count; i++)
		{
			if (server_devices.units[i].id > 0)
			{
				connect_to_server_device(client_params, i, &server_devices.units[i]);

				lock_threads_mutex();

				PSSClientDeviceParam device_params = {.client_params = client_params, .device_index = i, .device_id = server_devices.units[i].id};

				pss_clients[client_params->client_index].devices[i].device_type = server_devices.units[i].device_type;

				switch (server_devices.units[i].device_type)
				{
					case psdt_DispencerController:
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d starting dispencer controller device thread", client_params->client_index);

						g_mutex_lock(&pss_clients[client_params->client_index].mutex);

						pss_clients[client_params->client_index].devices[i].thread = g_thread_new("dc_read_thread", dc_device_func, &device_params);

						if (pss_clients[client_params->client_index].devices[i].thread == NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d error starting dispencer controller ID %d read thread", client_params->client_index, server_devices.units[i].id);
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Undefined;
						}
						else
						{
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Active;
							pss_clients[client_params->client_index].devices[i].thread->priority = G_THREAD_PRIORITY_LOW;
						}

						g_mutex_unlock(&pss_clients[client_params->client_index].mutex);

						break;

					case psdt_System:

						break;

					case psdt_TankGaugeSystem:
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d starting tgs device thread", client_params->client_index);

						g_mutex_lock(&pss_clients[client_params->client_index].mutex);

						pss_clients[client_params->client_index].devices[i].thread = g_thread_new("tgs_read_thread", tgs_device_func, &device_params);

						if (pss_clients[client_params->client_index].devices[i].thread == NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d error starting tgs ID %d read thread", client_params->client_index, server_devices.units[i].id);
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Undefined;
						}
						else
						{
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Active;
							pss_clients[client_params->client_index].devices[i].thread->priority = G_THREAD_PRIORITY_LOW;
						}

						g_mutex_unlock(&pss_clients[client_params->client_index].mutex);

						break;

					case psdt_PricePoleController:
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d starting pp device thread", client_params->client_index);

						g_mutex_lock(&pss_clients[client_params->client_index].mutex);

						pss_clients[client_params->client_index].devices[i].thread = g_thread_new("ppc_read_thread", ppc_device_func, &device_params);

						if (pss_clients[client_params->client_index].devices[i].thread == NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d error starting ppc ID %d read thread", client_params->client_index, server_devices.units[i].id);
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Undefined;
						}
						else
						{
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Active;
							pss_clients[client_params->client_index].devices[i].thread->priority = G_THREAD_PRIORITY_LOW;
						}

						g_mutex_unlock(&pss_clients[client_params->client_index].mutex);
						break;


					case psdt_FiscalRegister:
					case psdt_Terminal:
					case psdt_CustomerDisplay:
					case psdt_BillValidator:
					case psdt_InputController:
						break;

					case psdt_SensorController:
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d starting sc device thread", client_params->client_index);

						g_mutex_lock(&pss_clients[client_params->client_index].mutex);

						pss_clients[client_params->client_index].devices[i].thread = g_thread_new("sc_read_thread", sc_device_func, &device_params);

						if (pss_clients[client_params->client_index].devices[i].thread == NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d error starting sc ID %d read thread", client_params->client_index, server_devices.units[i].id);
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Undefined;
						}
						else
						{
							pss_clients[client_params->client_index].devices[i].thread_status = ts_Active;
							pss_clients[client_params->client_index].devices[i].thread->priority = G_THREAD_PRIORITY_LOW;
						}

						g_mutex_unlock(&pss_clients[client_params->client_index].mutex);
						break;

				}
			}
		}
	}
}
