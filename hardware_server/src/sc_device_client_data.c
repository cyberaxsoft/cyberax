

#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "sc_device.h"
#include "sc_device_data.h"
#include "tlv.h"

SCClient sc_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	scc_mutex;

void sc_client_init()
{
	g_mutex_init(&scc_mutex);
}

guint8 get_client_sci(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&scc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = sc_clients[index].sensor_controller_index;
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

ClientState get_sc_client_state(guint8 index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&scc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = sc_clients[index].state;
	}

	g_mutex_unlock(&scc_mutex);

	return result;

}

void set_sc_client_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&scc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[index].sock = sock;
	}

	g_mutex_unlock(&scc_mutex);

}

void set_sc_clients_status_is_destroying()
{
	g_mutex_lock(&scc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (sc_clients[i].state == cls_Active)
		{
			shutdown(sc_clients[i].sock, SHUT_RDWR);
			sc_clients[i].state = cls_Destroying;
		}
	}
	g_mutex_unlock(&scc_mutex);

}

void set_sc_device_status_is_changed_for_all_clients(guint8 sci, gboolean value, ScDeviceStatus status )
{
	g_mutex_lock(&scc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (sc_clients[i].state == cls_Active && sc_clients[i].sensor_controller_index == sci)
		{
			sc_clients[i].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&scc_mutex);
}

gboolean get_sc_device_status_is_changed_for_client(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (sc_clients[client_index].state == cls_Active)
		{
			result = sc_clients[client_index].device_status_is_change;
		}
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

void set_sc_device_status_is_changed_for_client(guint8 client_index, gboolean value)
{
	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (sc_clients[client_index].state == cls_Active)
		{
			sc_clients[client_index].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&scc_mutex);
}

gint8 find_new_sc_client_index(gboolean clients_filtering)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&scc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (sc_clients[i].state == cls_Free)
		{
			sc_clients[i].state = cls_Busy;

			if (clients_filtering)
			{
				sc_clients[i].logged = FALSE;
				sc_clients[i].access_level = cal_Disable;
			}
			else
			{
				sc_clients[i].logged = TRUE;
				sc_clients[i].access_level = cal_Unlim;
			}
			result = i;
			break;
		}
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

gboolean get_sc_client_logged(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = sc_clients[client_index].logged;
	}

	g_mutex_unlock(&scc_mutex);

	return result;

}

void set_sc_client_logged(guint8 client_index, gboolean new_value)
{
	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[client_index].logged = new_value;
	}

	g_mutex_unlock(&scc_mutex);

}

guint8 get_sc_client_access_level_param(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = sc_clients[client_index].access_level;
	}

	g_mutex_unlock(&scc_mutex);

	return result;

}

void set_sc_client_access_level_param(guint8 client_index, guint8 new_value)
{
	g_mutex_lock(&scc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[client_index].access_level = new_value;
	}

	g_mutex_unlock(&scc_mutex);

}

void set_new_sc_client_param( guint8 client_index, guint8 sci)
{
	guint8 sensor_count = get_sc_sensor_count(sci);

	g_mutex_lock(&scc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[client_index].sensor_controller_index = sci;
		sc_clients[client_index].sensor_info_count = sensor_count;
		sc_clients[client_index].device_status_is_change = TRUE;

		if (sensor_count > 0)
		{
			for (guint8 i = 0; i < sensor_count; i++)
			{
				guint32 num = 0;
				guint8 addr = 0;
				SensorParams params = {0x00};

				get_sc_sensor_info(sci, i, &num, &addr, &params);

				SensorClientInfo* info = &sc_clients[client_index].sensor_info[i];
				info->num = num;
				info->status_is_change = TRUE;
			}
		}
		sc_clients[client_index].read_thread_is_active = FALSE;
		sc_clients[client_index].state  = cls_Active;
	}

	g_mutex_unlock(&scc_mutex);
}

guint8 get_client_sensor_info_count( guint8 client_index )
{
	gint8 result = 0;

	g_mutex_lock(&scc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && sc_clients[client_index].state == cls_Active)
	{
		result = sc_clients[client_index].sensor_info_count;
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

guint8 get_client_sensor_status_is_changed( guint8 client_index, guint8 sensor_index)
{
	gint8 result = 0;

	g_mutex_lock(&scc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && sc_clients[client_index].state == cls_Active)
	{
		if (sensor_index < sc_clients[client_index].sensor_info_count)
		{
			result = sc_clients[client_index].sensor_info[sensor_index].status_is_change;
		}
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

void set_client_sensor_status_is_changed(guint8 client_index, guint8 sensor_index, gboolean value)
{
	g_mutex_lock(&scc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && sc_clients[client_index].state == cls_Active)
	{
		if (sensor_index < sc_clients[client_index].sensor_info_count)
		{
			sc_clients[client_index].sensor_info[sensor_index].status_is_change = value;
		}
	}

	g_mutex_unlock(&scc_mutex);
}

void set_sc_sensor_status_is_changed_for_all_clients(guint8 sci, guint8 index_sensor, gboolean value)
{
	g_mutex_lock(&scc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (sc_clients[i].state == cls_Active && sc_clients[i].sensor_controller_index == sci)
		{
			if (index_sensor < sc_clients[i].sensor_info_count)
			{
				//TODOg_printf("Client %d set status changed", i);
				sc_clients[i].sensor_info[index_sensor].status_is_change = value;
			}
		}
	}

	g_mutex_unlock(&scc_mutex);

}


guint32 get_client_sensor_num(guint8 client_index, guint8 sensor_index)
{
	gint32 result = 0;

	g_mutex_lock(&scc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && sc_clients[client_index].state == cls_Active)
	{
		if (sensor_index < sc_clients[client_index].sensor_info_count)
		{
			result = sc_clients[client_index].sensor_info[sensor_index].num;
		}
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}


void destroy_sc_client( guint8 client_index)
{

	g_mutex_lock(&scc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[client_index].sensor_controller_index = 0;
		sc_clients[client_index].device_status_is_change = FALSE;
		sc_clients[client_index].read_thread_is_active = FALSE;
		sc_clients[client_index].state = cls_Free;

	}

	g_mutex_unlock(&scc_mutex);

}

gboolean get_sc_client_read_thread_is_active(guint8 index_client)
{

	gboolean result = FALSE;

	g_mutex_lock(&scc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		result = sc_clients[index_client].read_thread_is_active;
	}

	g_mutex_unlock(&scc_mutex);

	return result;
}

void set_sc_client_read_thread_is_active(guint8 index_client, gboolean value)
{

	g_mutex_lock(&scc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		sc_clients[index_client].read_thread_is_active = value;
	}

	g_mutex_unlock(&scc_mutex);
}

