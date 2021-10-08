#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "ppc_device.h"
#include "ppc_device_data.h"
#include "tlv.h"

PPCClient ppc_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	ppcc_mutex;

void ppc_client_init()
{
	g_mutex_init(&ppcc_mutex);
}

guint8 get_client_ppci(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&ppcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = ppc_clients[index].price_pole_controller_index;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

ClientState get_ppc_client_state(guint8 index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&ppcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = ppc_clients[index].state;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;

}

void set_ppc_client_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&ppcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[index].sock = sock;
	}

	g_mutex_unlock(&ppcc_mutex);

}

void set_ppc_clients_status_is_destroying()
{
	g_mutex_lock(&ppcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (ppc_clients[i].state == cls_Active)
		{
			shutdown(ppc_clients[i].sock, SHUT_RDWR);
			ppc_clients[i].state = cls_Destroying;
		}
	}
	g_mutex_unlock(&ppcc_mutex);

}

void set_ppc_device_status_is_changed_for_all_clients(guint8 ppci, gboolean value, PpcDeviceStatus status )
{
	g_mutex_lock(&ppcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (ppc_clients[i].state == cls_Active && ppc_clients[i].price_pole_controller_index == ppci)
		{
			ppc_clients[i].device_status_is_change = value;

		}
	}

	g_mutex_unlock(&ppcc_mutex);
}

gboolean get_ppc_device_status_is_changed_for_client(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (ppc_clients[client_index].state == cls_Active)
		{
			result = ppc_clients[client_index].device_status_is_change;
		}
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

void set_ppc_device_status_is_changed_for_client(guint8 client_index, gboolean value)
{
	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (ppc_clients[client_index].state == cls_Active)
		{
			ppc_clients[client_index].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&ppcc_mutex);
}

gint8 find_new_ppc_client_index(gboolean clients_filtering)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&ppcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (ppc_clients[i].state == cls_Free)
		{
			ppc_clients[i].state = cls_Busy;

			if (clients_filtering)
			{
				ppc_clients[i].logged = FALSE;
				ppc_clients[i].access_level = cal_Disable;
			}
			else
			{
				ppc_clients[i].logged = TRUE;
				ppc_clients[i].access_level = cal_Unlim;
			}
			result = i;
			break;
		}
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

gboolean get_ppc_client_logged(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = ppc_clients[client_index].logged;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;

}

void set_ppc_client_logged(guint8 client_index, gboolean new_value)
{
	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[client_index].logged = new_value;
	}

	g_mutex_unlock(&ppcc_mutex);

}

guint8 get_ppc_client_access_level_param(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = ppc_clients[client_index].access_level;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;

}

void set_ppc_client_access_level_param(guint8 client_index, guint8 new_value)
{
	g_mutex_lock(&ppcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[client_index].access_level = new_value;
	}

	g_mutex_unlock(&ppcc_mutex);

}

void set_new_ppc_client_param( guint8 client_index, guint8 ppci)
{
	guint8 price_pole_count = get_ppc_price_pole_count(ppci);

	g_mutex_lock(&ppcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[client_index].price_pole_controller_index = ppci;
		ppc_clients[client_index].price_pole_info_count = price_pole_count;
		ppc_clients[client_index].device_status_is_change = TRUE;

		ppc_clients[client_index].command = hsc_None;
		memset(&ppc_clients[client_index].param_price_packs, 0x00, sizeof(PPCPricePacks));

		ppc_clients[client_index].command_state = cs_Free;
		ppc_clients[client_index].command_result = ppce_NoError;

		ppc_clients[client_index].message_id = 0;

		ppc_clients[client_index].status_is_change = TRUE;

		if (price_pole_count > 0)
		{
			for (guint8 i = 0; i < price_pole_count; i++)
			{
				guint8 num = 0;
				guint8 grade = 0;
				guint8 symbol_count = 0;
				get_ppc_price_pole_info(ppci, i, &num, &grade, &symbol_count );

				PricePoleClientInfo* info = &ppc_clients[client_index].price_pole_info[i];
				info->num = num;
			}
		}
		ppc_clients[client_index].read_thread_is_active = FALSE;
		ppc_clients[client_index].state  = cls_Active;
	}

	g_mutex_unlock(&ppcc_mutex);
}

guint8 get_client_price_pole_info_count( guint8 client_index )
{
	gint8 result = 0;

	g_mutex_lock(&ppcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && ppc_clients[client_index].state == cls_Active)
	{
		result = ppc_clients[client_index].price_pole_info_count;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

guint8 get_client_price_pole_status_is_changed( guint8 client_index)
{
	gint8 result = 0;

	g_mutex_lock(&ppcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && ppc_clients[client_index].state == cls_Active)
	{
		result = ppc_clients[client_index].status_is_change;

	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

void set_client_price_pole_status_is_changed(guint8 client_index, gboolean value)
{
	g_mutex_lock(&ppcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && ppc_clients[client_index].state == cls_Active)
	{
		ppc_clients[client_index].status_is_change = value;
	}

	g_mutex_unlock(&ppcc_mutex);
}

guint32 get_client_price_pole_num(guint8 client_index, guint8 price_pole_index)
{
	gint32 result = 0;

	g_mutex_lock(&ppcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && ppc_clients[client_index].state == cls_Active)
	{
		if (price_pole_index < ppc_clients[client_index].price_pole_info_count)
		{
			result = ppc_clients[client_index].price_pole_info[price_pole_index].num;
		}
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}


void destroy_ppc_client( guint8 client_index)
{

	g_mutex_lock(&ppcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[client_index].price_pole_controller_index = 0;
		ppc_clients[client_index].device_status_is_change = FALSE;
		ppc_clients[client_index].read_thread_is_active = FALSE;
		ppc_clients[client_index].state = cls_Free;

		ppc_clients[client_index].command = hsc_None;
		memset(&ppc_clients[client_index].param_price_packs, 0x00, sizeof(PPCPricePacks));

		ppc_clients[client_index].command_state = cs_Free;
		ppc_clients[client_index].command_result = ppce_NoError;

		ppc_clients[client_index].message_id = 0;

	}

	g_mutex_unlock(&ppcc_mutex);

}

gboolean get_ppc_client_read_thread_is_active(guint8 index_client)
{

	gboolean result = FALSE;

	g_mutex_lock(&ppcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		result = ppc_clients[index_client].read_thread_is_active;
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

void set_ppc_client_read_thread_is_active(guint8 index_client, gboolean value)
{

	g_mutex_lock(&ppcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[index_client].read_thread_is_active = value;
	}

	g_mutex_unlock(&ppcc_mutex);
}

gint8 find_ppc_client_command(guint8 ppci, HardwareServerCommand* command, PPCPricePacks* price_packs)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&ppcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (ppc_clients[i].state == cls_Active && ppc_clients[i].price_pole_controller_index == ppci)
		{
			if (ppc_clients[i].command != hsc_None && ppc_clients[i].command_state == cs_New)
			{
				*command = ppc_clients[i].command;
				*price_packs = ppc_clients[i].param_price_packs;
				ppc_clients[i].command_state = cs_Run;
				result = i;
				break;
			}
		}
	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

void set_ppc_client_command_result(guint8 index_client, PpcDeviceError result)
{
	g_mutex_lock(&ppcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		ppc_clients[index_client].command_result = result;
		ppc_clients[index_client].command_state = cs_Complete;
	}

	g_mutex_unlock(&ppcc_mutex);
}

void set_ppc_price_pole_status_is_changed_for_all_clients(guint8 ppci, gboolean value)
{
	g_mutex_lock(&ppcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (ppc_clients[i].state == cls_Active && ppc_clients[i].price_pole_controller_index == ppci)
		{
			ppc_clients[i].status_is_change = value;
		}
	}

	g_mutex_unlock(&ppcc_mutex);

}

CommandState get_client_price_pole_controller_command_state(guint8 client_index)
{
	CommandState result = cs_Free;

	g_mutex_lock(&ppcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && ppc_clients[client_index].state == cls_Active)
	{
		result = ppc_clients[client_index].command_state;

	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

void get_ppc_client_command_result(guint8 index_client, guint32* message_id, HardwareServerCommand* command, PpcDeviceError* device_error)
{
	g_mutex_lock(&ppcc_mutex);

	if( index_client < MAX_DEVICE_CLIENT_COUNT &&
			ppc_clients[index_client].state == cls_Active )
	{
		*message_id = ppc_clients[index_client].message_id;
		*command = ppc_clients[index_client].command;
		*device_error = ppc_clients[index_client].command_result;
	}

	g_mutex_unlock(&ppcc_mutex);
}

gboolean clear_ppc_client_command(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT )
	{
		ppc_clients[client_index].command = hsc_None;
		ppc_clients[client_index].command_result = ppce_Undefined;
		ppc_clients[client_index].message_id = 0;

		memset(&ppc_clients[client_index].param_price_packs, 0x00, sizeof(ppc_clients[client_index].param_price_packs));

		ppc_clients[client_index].command_state = cs_Free;

		result = TRUE;

	}

	g_mutex_unlock(&ppcc_mutex);

	return result;
}

ExchangeError set_ppc_client_command( guint8 client_index, HardwareServerCommand command, guint32 message_id,  PPCPricePacks price_packs)
{
	ExchangeError result = ee_TaskBufferError;

	g_mutex_lock(&ppcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if ( ppc_clients[client_index].state == cls_Active )
		{
				if(  ppc_clients[client_index].command == hsc_None)
				{
					ppc_clients[client_index].command = command;
					ppc_clients[client_index].command_result = ppce_Undefined;
					ppc_clients[client_index].message_id = message_id;
					ppc_clients[client_index].param_price_packs = price_packs;
					ppc_clients[client_index].command_state = cs_New;

					result = ee_None;
				}
				else
				{
					result = ee_DeviceIsBusy;
				}
		}
		else
		{
			result = ee_FaultParam;
		}
	}


	g_mutex_unlock(&ppcc_mutex);

	return result;

}
