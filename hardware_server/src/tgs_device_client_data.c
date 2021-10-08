#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tgs_device.h"
#include "tgs_device_data.h"
#include "tlv.h"

TGSClient tgs_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	tgsc_mutex;

void tgs_client_init()
{
	g_mutex_init(&tgsc_mutex);
}

guint8 get_client_tgsi(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&tgsc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = tgs_clients[index].tgs_index;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

ClientState get_tgs_client_state(guint8 index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&tgsc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = tgs_clients[index].state;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;

}

void set_tgs_client_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&tgsc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[index].sock = sock;
	}

	g_mutex_unlock(&tgsc_mutex);

}

void set_tgs_clients_status_is_destroying()
{
	g_mutex_lock(&tgsc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (tgs_clients[i].state == cls_Active)
		{
			shutdown(tgs_clients[i].sock, SHUT_RDWR);
			tgs_clients[i].state = cls_Destroying;
		}
	}
	g_mutex_unlock(&tgsc_mutex);

}

void set_tgs_device_status_is_changed_for_all_clients(guint8 tgsi, gboolean value, TgsDeviceStatus status )
{
	g_mutex_lock(&tgsc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (tgs_clients[i].state == cls_Active && tgs_clients[i].tgs_index == tgsi)
		{
			tgs_clients[i].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&tgsc_mutex);
}

gboolean get_tgs_device_status_is_changed_for_client(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (tgs_clients[client_index].state == cls_Active)
		{
			result = tgs_clients[client_index].device_status_is_change;
		}
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

void set_tgs_device_status_is_changed_for_client(guint8 client_index, gboolean value)
{
	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (tgs_clients[client_index].state == cls_Active)
		{
			tgs_clients[client_index].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&tgsc_mutex);
}

gint8 find_new_tgs_client_index(gboolean clients_filtering)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&tgsc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (tgs_clients[i].state == cls_Free)
		{
			tgs_clients[i].state = cls_Busy;

			if (clients_filtering)
			{
				tgs_clients[i].logged = FALSE;
				tgs_clients[i].access_level = cal_Disable;
			}
			else
			{
				tgs_clients[i].logged = TRUE;
				tgs_clients[i].access_level = cal_Unlim;
			}
			result = i;
			break;
		}
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

gboolean get_tgs_client_logged(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = tgs_clients[client_index].logged;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;

}

void set_tgs_client_logged(guint8 client_index, gboolean new_value)
{
	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[client_index].logged = new_value;
	}

	g_mutex_unlock(&tgsc_mutex);

}

guint8 get_tgs_client_access_level_param(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = tgs_clients[client_index].access_level;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;

}

void set_tgs_client_access_level_param(guint8 client_index, guint8 new_value)
{
	g_mutex_lock(&tgsc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[client_index].access_level = new_value;
	}

	g_mutex_unlock(&tgsc_mutex);

}

void set_new_tgs_client_param( guint8 client_index, guint8 tgsi)
{
	guint8 tank_count = get_tgs_tank_count(tgsi);

	g_mutex_lock(&tgsc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[client_index].tgs_index = tgsi;
		tgs_clients[client_index].tank_info_count = tank_count;
		tgs_clients[client_index].device_status_is_change = TRUE;

		if (tank_count > 0)
		{
			for (guint8 i = 0; i < tank_count; i++)
			{
				guint32 num = 0;
				guint8 channel = 0;
				get_tgs_tank_info(tgsi, i, &num, &channel );

				TankClientInfo* info = &tgs_clients[client_index].tank_info[i];
				info->num = num;
				info->status_is_change = TRUE;
			}
		}
		tgs_clients[client_index].read_thread_is_active = FALSE;
		tgs_clients[client_index].state  = cls_Active;
	}

	g_mutex_unlock(&tgsc_mutex);
}

guint8 get_client_tank_info_count( guint8 client_index )
{
	gint8 result = 0;

	g_mutex_lock(&tgsc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && tgs_clients[client_index].state == cls_Active)
	{
		result = tgs_clients[client_index].tank_info_count;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

guint8 get_client_tank_status_is_changed( guint8 client_index, guint8 tank_index)
{
	gint8 result = 0;

	g_mutex_lock(&tgsc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && tgs_clients[client_index].state == cls_Active)
	{
		if (tank_index < tgs_clients[client_index].tank_info_count)
		{
			result = tgs_clients[client_index].tank_info[tank_index].status_is_change;
		}
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

void set_client_tank_status_is_changed(guint8 client_index, guint8 tank_index, gboolean value)
{
	g_mutex_lock(&tgsc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && tgs_clients[client_index].state == cls_Active)
	{
		if (tank_index < tgs_clients[client_index].tank_info_count)
		{
			tgs_clients[client_index].tank_info[tank_index].status_is_change = value;
		}
	}

	g_mutex_unlock(&tgsc_mutex);
}

guint32 get_client_tank_num(guint8 client_index, guint8 tank_index)
{
	gint32 result = 0;

	g_mutex_lock(&tgsc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && tgs_clients[client_index].state == cls_Active)
	{
		if (tank_index < tgs_clients[client_index].tank_info_count)
		{
			result = tgs_clients[client_index].tank_info[tank_index].num;
		}
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}


void destroy_tgs_client( guint8 client_index)
{

	g_mutex_lock(&tgsc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[client_index].tgs_index = 0;
		tgs_clients[client_index].device_status_is_change = FALSE;
		tgs_clients[client_index].read_thread_is_active = FALSE;
		tgs_clients[client_index].state = cls_Free;

	}

	g_mutex_unlock(&tgsc_mutex);

}

gboolean get_tgs_client_read_thread_is_active(guint8 index_client)
{

	gboolean result = FALSE;

	g_mutex_lock(&tgsc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		result = tgs_clients[index_client].read_thread_is_active;
	}

	g_mutex_unlock(&tgsc_mutex);

	return result;
}

void set_tgs_client_read_thread_is_active(guint8 index_client, gboolean value)
{

	g_mutex_lock(&tgsc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		tgs_clients[index_client].read_thread_is_active = value;
	}

	g_mutex_unlock(&tgsc_mutex);
}

