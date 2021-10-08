#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "dc_device.h"
#include "dc_device_data.h"
#include "tlv.h"

DCClient dc_clients[MAX_DEVICE_CLIENT_COUNT];
GMutex	dcc_mutex;

void dc_client_init()
{
	g_mutex_init(&dcc_mutex);
}

guint8 get_client_dci(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&dcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = dc_clients[index].dispencer_controller_index;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

ClientState get_dc_client_state(guint8 index)
{
	ClientState result = cls_Free;

	g_mutex_lock(&dcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = dc_clients[index].state;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;

}

void set_dc_client_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&dcc_mutex);

	if (index < MAX_DEVICE_CLIENT_COUNT)
	{
		dc_clients[index].sock = sock;
	}

	g_mutex_unlock(&dcc_mutex);

}

void set_dc_clients_status_is_destroying()
{
	g_mutex_lock(&dcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (dc_clients[i].state == cls_Active)
		{
			shutdown(dc_clients[i].sock, SHUT_RDWR);
			dc_clients[i].state = cls_Destroying;
		}
	}
	g_mutex_unlock(&dcc_mutex);

}

void set_dc_device_status_is_changed_for_all_clients(guint8 dci, gboolean value, DcDeviceStatus status )
{
	g_mutex_lock(&dcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (dc_clients[i].state == cls_Active && dc_clients[i].dispencer_controller_index == dci)
		{
			dc_clients[i].device_status_is_change = value;
			//dc_clients[i].device_status = status;
		}
	}

	g_mutex_unlock(&dcc_mutex);
}

gboolean get_dc_device_status_is_changed_for_client(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (dc_clients[client_index].state == cls_Active)
		{
			result = dc_clients[client_index].device_status_is_change;
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

void set_dc_device_status_is_changed_for_client(guint8 client_index, gboolean value)
{
	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (dc_clients[client_index].state == cls_Active)
		{
			dc_clients[client_index].device_status_is_change = value;
		}
	}

	g_mutex_unlock(&dcc_mutex);
}

void set_dc_disp_status_is_changed_for_all_clients(guint8 dci, guint8 index_disp, gboolean value)
{
	g_mutex_lock(&dcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (dc_clients[i].state == cls_Active && dc_clients[i].dispencer_controller_index == dci)
		{
			if (index_disp < dc_clients[i].dispencer_info_count)
			{
				//TODOg_printf("Client %d set status changed", i);
				dc_clients[i].dispencer_info[index_disp].status_is_change = value;
			}
		}
	}

	g_mutex_unlock(&dcc_mutex);

}

void get_dc_client_command_result(guint8 index_client, guint8 index_disp, guint32* message_id, HardwareServerCommand* command, DcDeviceError* device_error)
{
	g_mutex_lock(&dcc_mutex);

	if( index_client < MAX_DEVICE_CLIENT_COUNT &&
		dc_clients[index_client].state == cls_Active &&
		index_disp < dc_clients[index_client].dispencer_info_count)
	{
		*message_id = dc_clients[index_client].dispencer_info[index_disp].message_id;
		*command = dc_clients[index_client].dispencer_info[index_disp].command;
		*device_error = dc_clients[index_client].dispencer_info[index_disp].command_result;
	}

	g_mutex_unlock(&dcc_mutex);
}

gint8 find_dc_client_command(guint8 dci, guint8 index_disp, HardwareServerCommand* command, guint32* disp_num, guint8* nozzle_num, guint32* price, guint32* volume, guint32* amount,
		guint8* index_ext_func, DCPricePacks* price_packs)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&dcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (dc_clients[i].state == cls_Active && dc_clients[i].dispencer_controller_index == dci)
		{
			if (index_disp < dc_clients[i].dispencer_info_count)
			{
				if (dc_clients[i].dispencer_info[index_disp].command != hsc_None &&
						dc_clients[i].dispencer_info[index_disp].command_state == cs_New)
				{
					*command = dc_clients[i].dispencer_info[index_disp].command;
					*disp_num = dc_clients[i].dispencer_info[index_disp].num;
					*nozzle_num = dc_clients[i].dispencer_info[index_disp].param_nozzle_num;
					*price = dc_clients[i].dispencer_info[index_disp].param_price;
					*volume = dc_clients[i].dispencer_info[index_disp].param_volume;
					*amount = dc_clients[i].dispencer_info[index_disp].param_amount;
					*index_ext_func = dc_clients[i].dispencer_info[index_disp].param_index_ext_func;
					*price_packs = dc_clients[i].dispencer_info[index_disp].param_price_packs;
					dc_clients[i].dispencer_info[index_disp].command_state = cs_Run;
					result = i;
					break;
				}
			}
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

guint8 get_client_dispencer_info_count( guint8 client_index )
{
	gint8 result = 0;

	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		result = dc_clients[client_index].dispencer_info_count;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

guint8 get_client_dispencer_status_is_changed( guint8 client_index, guint8 disp_index)
{
	gint8 result = 0;

	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		if (disp_index < dc_clients[client_index].dispencer_info_count)
		{
			result = dc_clients[client_index].dispencer_info[disp_index].status_is_change;
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

void set_client_dispencer_status_is_changed(guint8 client_index, guint8 disp_index, gboolean value)
{
	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		if (disp_index < dc_clients[client_index].dispencer_info_count)
		{
			dc_clients[client_index].dispencer_info[disp_index].status_is_change = value;
		}
	}

	g_mutex_unlock(&dcc_mutex);
}

guint32 get_client_dispencer_num(guint8 client_index, guint8 disp_index)
{
	gint32 result = 0;

	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		if (disp_index < dc_clients[client_index].dispencer_info_count)
		{
			result = dc_clients[client_index].dispencer_info[disp_index].num;
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

CommandState get_client_dispencer_command_state(guint8 client_index, guint8 disp_index)
{
	CommandState result = cs_Free;

	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		if (disp_index < dc_clients[client_index].dispencer_info_count)
		{
			result = dc_clients[client_index].dispencer_info[disp_index].command_state;
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

void set_client_dispencer_command_state(guint8 client_index, guint8 disp_index, CommandState value)
{

	g_mutex_lock(&dcc_mutex);

	if( client_index < MAX_DEVICE_CLIENT_COUNT && dc_clients[client_index].state == cls_Active)
	{
		if (disp_index < dc_clients[client_index].dispencer_info_count)
		{
			dc_clients[client_index].dispencer_info[disp_index].command_state = value;
		}
	}

	g_mutex_unlock(&dcc_mutex);
}

gint8 find_new_dc_client_index(gboolean clients_filtering)
{
	gint8 result = NO_CLIENT;

	g_mutex_lock(&dcc_mutex);

	for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
	{
		if (dc_clients[i].state == cls_Free)
		{
			dc_clients[i].state = cls_Busy;

			if (clients_filtering)
			{
				dc_clients[i].logged = FALSE;
				dc_clients[i].access_level = cal_Disable;
			}
			else
			{
				dc_clients[i].logged = TRUE;
				dc_clients[i].access_level = cal_Unlim;
			}
			result = i;
			break;
		}
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

gboolean get_dc_client_logged(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = dc_clients[client_index].logged;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;

}

void set_dc_client_logged(guint8 client_index, gboolean new_value)
{
	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		dc_clients[client_index].logged = new_value;
	}

	g_mutex_unlock(&dcc_mutex);

}

guint8 get_dc_client_access_level_param(guint8 client_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		result = dc_clients[client_index].access_level;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;

}

void set_dc_client_access_level_param(guint8 client_index, guint8 new_value)
{
	g_mutex_lock(&dcc_mutex);

	if (client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		dc_clients[client_index].access_level = new_value;
	}

	g_mutex_unlock(&dcc_mutex);

}

gboolean clear_dc_client_command(guint8 client_index, guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT &&
		 disp_index < dc_clients[client_index].dispencer_info_count)
	{
		dc_clients[client_index].dispencer_info[disp_index].command = hsc_None;
		dc_clients[client_index].dispencer_info[disp_index].command_result = dce_Undefined;
		dc_clients[client_index].dispencer_info[disp_index].message_id = 0;
		dc_clients[client_index].dispencer_info[disp_index].param_nozzle_num = 0;
		dc_clients[client_index].dispencer_info[disp_index].param_price = 0;
		dc_clients[client_index].dispencer_info[disp_index].param_volume = 0;
		dc_clients[client_index].dispencer_info[disp_index].param_amount = 0;
		dc_clients[client_index].dispencer_info[disp_index].param_index_ext_func = 0;

		memset(&dc_clients[client_index].dispencer_info[disp_index].param_price_packs, 0x00, sizeof(dc_clients[client_index].dispencer_info[disp_index].param_price_packs));

		dc_clients[client_index].dispencer_info[disp_index].command_state = cs_Free;

		result = TRUE;

	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

ExchangeError set_dc_client_command( guint8 client_index, guint8 disp_index, HardwareServerCommand command, guint32 message_id, guint8 nozzle_num, guint32 price,
		guint32 volume ,guint32 amount, guint8 index_ext_func, DCPricePacks price_packs)
{
	ExchangeError result = ee_TaskBufferError;

	g_mutex_lock(&dcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{


		if ( dc_clients[client_index].state == cls_Active &&
			 disp_index < dc_clients[client_index].dispencer_info_count)
		{
				if(  dc_clients[client_index].dispencer_info[disp_index].command == hsc_None || dc_clients[client_index].dispencer_info[disp_index].command == hsc_DCReset)
				{
					dc_clients[client_index].dispencer_info[disp_index].command = command;
					dc_clients[client_index].dispencer_info[disp_index].command_result = dce_Undefined;
					dc_clients[client_index].dispencer_info[disp_index].message_id = message_id;
					dc_clients[client_index].dispencer_info[disp_index].param_nozzle_num = nozzle_num;
					dc_clients[client_index].dispencer_info[disp_index].param_price = price;
					dc_clients[client_index].dispencer_info[disp_index].param_volume = volume;
					dc_clients[client_index].dispencer_info[disp_index].param_amount = amount;
					dc_clients[client_index].dispencer_info[disp_index].param_index_ext_func = index_ext_func;
					dc_clients[client_index].dispencer_info[disp_index].param_price_packs = price_packs;
					dc_clients[client_index].dispencer_info[disp_index].command_state = cs_New;

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


	g_mutex_unlock(&dcc_mutex);

	return result;

}


void set_new_dc_client_param( guint8 client_index, guint8 dci)
{
	guint8 disp_count = get_dc_disp_count(dci);

	g_mutex_lock(&dcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		dc_clients[client_index].dispencer_controller_index = dci;
		dc_clients[client_index].dispencer_info_count = disp_count;
		dc_clients[client_index].device_status_is_change = TRUE;

		if (disp_count > 0)
		{
			for (guint8 i = 0; i < disp_count; i++)
			{
				guint32 num = 0;
				guint8 addr = 0;
				guint8 nozzle_count = 0;
				get_dc_disp_info(dci, i, &num, &addr, &nozzle_count );

				DispencerClientInfo* info = &dc_clients[client_index].dispencer_info[i];
				info->num = num;
				info->command = hsc_None;
				info->command_result = dce_NoError;
				info->command_state = cs_Free;
				info->message_id = 0;
				info->param_amount = 0;
				info->param_index_ext_func = 0;
				info->param_nozzle_num = 0;
				info->param_price = 0;
				info->param_volume = 0;
				info->status_is_change = TRUE;
			}
		}
		dc_clients[client_index].read_thread_is_active = FALSE;
		dc_clients[client_index].state  = cls_Active;
	}

	g_mutex_unlock(&dcc_mutex);
}

void destroy_dc_client( guint8 client_index)
{

	g_mutex_lock(&dcc_mutex);

	if ( client_index < MAX_DEVICE_CLIENT_COUNT)
	{
		if (dc_clients[client_index].dispencer_info_count > 0)
		{
			for (guint8 i = 0; i < dc_clients[client_index].dispencer_info_count && i < MAX_DISPENCER_COUNT; i++)
			{
				DispencerClientInfo* info = &dc_clients[client_index].dispencer_info[i];
				info->command = hsc_None;
				info->command_result = dce_NoError;
				info->command_state = cs_Free;
				info->message_id = 0;
				info->param_amount = 0;
				info->param_index_ext_func = 0;
				info->param_nozzle_num = 0;
				info->param_price = 0;
				info->param_volume = 0;
				info->status_is_change = TRUE;
			}
		}

		dc_clients[client_index].dispencer_controller_index = 0;
		dc_clients[client_index].dispencer_info_count = 0;
		dc_clients[client_index].device_status_is_change = FALSE;
		dc_clients[client_index].read_thread_is_active = FALSE;
		dc_clients[client_index].state = cls_Free;

	}

	g_mutex_unlock(&dcc_mutex);

}

gboolean get_dc_client_read_thread_is_active(guint8 index_client)
{

	gboolean result = FALSE;

	g_mutex_lock(&dcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		result = dc_clients[index_client].read_thread_is_active;
	}

	g_mutex_unlock(&dcc_mutex);

	return result;
}

void set_dc_client_read_thread_is_active(guint8 index_client, gboolean value)
{

	g_mutex_lock(&dcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		dc_clients[index_client].read_thread_is_active = value;
	}

	g_mutex_unlock(&dcc_mutex);
}

void set_dc_client_command_result(guint8 index_client, guint8 index_disp, DcDeviceError result)
{
	g_mutex_lock(&dcc_mutex);

	if (index_client < MAX_DEVICE_CLIENT_COUNT)
	{
		if (index_disp < dc_clients[index_client].dispencer_info_count)
		{
			dc_clients[index_client].dispencer_info[index_disp].command_result = result;
			dc_clients[index_client].dispencer_info[index_disp].command_state = cs_Complete;
		}
	}

	g_mutex_unlock(&dcc_mutex);
}


