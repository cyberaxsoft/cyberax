#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "fr_device.h"
#include "fr_device_data.h"
#include "tlv.h"

FRClient fr_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	frc_mutex;

void fr_client_init()
{
	g_mutex_init(&frc_mutex);
}

guint8 get_client_fri(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&frc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = fr_clients[index].fiscal_register_index;
	}

	g_mutex_unlock(&frc_mutex);

	return result;
}

ClientState get_fr_client_state(guint8 index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&frc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = fr_clients[index].state;
	}

	g_mutex_unlock(&frc_mutex);

	return result;

}

void set_fr_device_status_is_changed_for_all_clients(guint8 fri, gboolean value, FrDeviceStatus status )
{
	g_mutex_lock(&frc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (fr_clients[i].state == cls_Active && fr_clients[i].fiscal_register_index == fri)
		{
			fr_clients[i].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&frc_mutex);
}




void set_fr_clients_status_is_destroying()
{
	g_mutex_lock(&frc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (fr_clients[i].state == cls_Active)
		{
			shutdown(fr_clients[i].sock, SHUT_RDWR);
			fr_clients[i].state = cls_Destroying;
		}
	}
	g_mutex_unlock(&frc_mutex);

}
