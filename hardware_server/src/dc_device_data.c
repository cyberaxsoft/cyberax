#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "dc_device.h"
#include "dc_device_data.h"
#include "dc_func.h"

DispencerController dispencer_controllers[MAX_DISP_CONTROL_COUNT];
guint8 dispencer_controller_count = 0;
GMutex	dc_mutex;


void dc_init()
{
	g_mutex_init(&dc_mutex);
}

ThreadStatus get_dc_main_sock_thread_status(guint8 index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].main_sock_status;
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

void set_dc_main_sock_thread_status(guint8 index, ThreadStatus status)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].main_sock_status = status;
	}

	g_mutex_unlock(&dc_mutex);
}

gint32 get_dc_sock(guint8 index)
{
	gint32 result = -1;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].sock;
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

void set_dc_sock_status(guint8 index, SocketStatus sock_status)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].sock_status = sock_status;
	}

	g_mutex_unlock(&dc_mutex);

}

SocketStatus get_dc_sock_status(guint8 index)
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].sock_status;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

gboolean get_new_dispencer_controller_index(guint8* index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (dispencer_controller_count < MAX_DISP_CONTROL_COUNT)
	{
		*index = dispencer_controller_count;

		dispencer_controller_count++;
		result = TRUE;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_device_is_working(guint8 index, gboolean value)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].device_is_working = value;
	}

	g_mutex_unlock(&dc_mutex);

}

gboolean get_dc_device_is_working(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].device_is_working;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_device_status(guint8 index, DcDeviceStatus status)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (dispencer_controllers[index].device_status != status)
		{
			dispencer_controllers[index].device_status = status;
			dispencer_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&dc_mutex);

}

DcDeviceStatus get_dc_device_status(guint8 index)
{
	DcDeviceStatus result = dcs_UndefinedError;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].device_status;

	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

void set_dc_device_last_error(guint8 index, DcDeviceError error)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (dispencer_controllers[index].last_device_error != error)
		{
			dispencer_controllers[index].last_device_error = error;
			dispencer_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&dc_mutex);
}

DcDeviceError get_dc_device_last_erorr(guint8 index)
{
	DcDeviceError result = dce_Undefined;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].last_device_error;
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

void set_dc_device_status_is_changed(guint8 index, gboolean value)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].device_status_is_changed = value;
	}

	g_mutex_unlock(&dc_mutex);

}

gboolean get_dc_device_status_is_changed(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].device_status_is_changed;
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}
DispencerState get_dc_dispencer_state(guint8 index, guint8 disp_index)
{
	DispencerState result = ds_NotInitialize;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (disp_index < dispencer_controllers[index].dispencers_count)
		{
			result = dispencer_controllers[index].dispencers[disp_index].state;
		}
	}
	g_mutex_unlock(&dc_mutex);

	return result;
}

gboolean get_dc_dispencer_data(guint8 index, guint8 disp_index, guint32* num, DispencerState* state, OrderType*	order_type, OrderType* preset_order_type,
		gboolean* is_pay, guint8* preset_nozzle_num, guint8* active_nozzle_num, guint32* preset_price, guint32* preset_volume, guint32* preset_amount,
		guint32* current_price, guint32* current_volume, guint32* current_amount, guchar* error, gchar* error_description, guint8* error_description_length)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (disp_index < dispencer_controllers[index].dispencers_count)
		{
			*num = dispencer_controllers[index].dispencers[disp_index].num;
			*state = dispencer_controllers[index].dispencers[disp_index].state;
			*order_type = dispencer_controllers[index].dispencers[disp_index].order_type;
			*preset_order_type = dispencer_controllers[index].dispencers[disp_index].preset_order_type;
			*is_pay = dispencer_controllers[index].dispencers[disp_index].is_pay;
			*preset_nozzle_num = dispencer_controllers[index].dispencers[disp_index].preset_nozzle_num;
			*active_nozzle_num = dispencer_controllers[index].dispencers[disp_index].active_nozzle_num;
			*preset_price = dispencer_controllers[index].dispencers[disp_index].preset_price;
			*preset_volume = dispencer_controllers[index].dispencers[disp_index].preset_volume;
			*preset_amount = dispencer_controllers[index].dispencers[disp_index].preset_amount;
			*current_price = dispencer_controllers[index].dispencers[disp_index].current_price;
			*current_volume = dispencer_controllers[index].dispencers[disp_index].current_volume;
			*current_amount = dispencer_controllers[index].dispencers[disp_index].current_amount;
			*error = dispencer_controllers[index].dispencers[disp_index].error;

			if (error_description!=NULL)
			{
				g_free(error_description);
				error_description = NULL;
			}
			error_description = g_strdup((gchar*)dispencer_controllers[index].dispencers[disp_index].error_description);

			*error_description_length = strlen((gchar*)dispencer_controllers[index].dispencers[disp_index].error_description);

			result = TRUE;
		}
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_device_name(guint8 index, gchar* name)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (dispencer_controllers[index].name != NULL)
		{
			g_free(dispencer_controllers[index].name);
			dispencer_controllers[index].name = NULL;
		}
		dispencer_controllers[index].name = g_strdup(name);
	}

	g_mutex_unlock(&dc_mutex);

}

gchar* get_dc_device_name(guint8 index)
{
	gchar* result = NULL;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = g_strdup(dispencer_controllers[index].name);
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].sock = sock;
	}

	g_mutex_unlock(&dc_mutex);

}

gboolean init_dc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);


	if (index < dispencer_controller_count)
	{
		destroy_log_params(&dispencer_controllers[index].log_params);

		dispencer_controllers[index].log_trace = log_trace;

		create_log_params(&dispencer_controllers[index].log_params, log_enable, log_dir, device_name, file_size, save_days);

		if (log_enable)
		{
			//create log
			if (!init_log_dir(&dispencer_controllers[index].log_params))
			{
				g_printf("%s : error creating log dir\n", device_name);
				return result;
			}

			gchar* log_filename = g_strdup_printf("%s.log", device_name);

			if (dispencer_controllers[index].log_params.log != NULL)
			{
				close_log(&dispencer_controllers[index].log_params);

			}

			create_log(&dispencer_controllers[index].log_params);
			if (dispencer_controllers[index].log_params.log == NULL)
			{
				g_printf("%s : log create error\n", device_name);
				return result;
			}

			g_free(log_filename);

			result = TRUE;
		}


	}


	g_mutex_unlock(&dc_mutex);

	return result;

}

void get_dc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		*log_params = &dispencer_controllers[index].log_params;
		*log_trace = dispencer_controllers[index].log_trace;
	}

	g_mutex_unlock(&dc_mutex);

}

gchar* get_dc_log_dir(guint8 index)
{

	gchar* result = NULL;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = g_strdup(dispencer_controllers[index].log_params.path);
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

guint32 get_dc_dispencer_num(guint8 index, guint8 index_disp)
{
	guint32 result = 0;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && index_disp < dispencer_controllers[index].dispencers_count)
	{
		result = dispencer_controllers[index].dispencers[index_disp].num;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

guint8 get_dc_dispencer_nozzle_count(guint8 index, guint8 index_disp)
{
	guint8 result = 0;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && index_disp < dispencer_controllers[index].dispencers_count)
	{
		result = dispencer_controllers[index].dispencers[index_disp].nozzle_count;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

gboolean dc_dispencer_num_is_present(guint8 index, guint32 disp_num, guint8* index_disp)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index <  dispencer_controller_count && dispencer_controllers[index].dispencers_count > 0)
	{
		for (guint8 i = 0; i < dispencer_controllers[index].dispencers_count; i++)
		{
			if (disp_num == dispencer_controllers[index].dispencers[i].num)
			{
				result = TRUE;
				*index_disp = i;
				break;
			}

		}
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

gboolean dc_nozzle_num_is_present(guint8 index, guint32 disp_num, guint8 nozzle_num, guint8* disp_index, guint8* nozzle_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index <  dispencer_controller_count && dispencer_controllers[index].dispencers_count > 0)
	{
		for (guint8 i = 0; i < dispencer_controllers[index].dispencers_count; i++)
		{
			if (disp_num == 0)
			{
				result = TRUE;
				break;
			}
			else
			{
				if (disp_num == dispencer_controllers[index].dispencers[i].num)
				{
					*disp_index = i;

					if (dispencer_controllers[index].dispencers[i].nozzle_count > 0)
					{
						if (nozzle_num == 0)
						{
							result = TRUE;
						}
						else
						{
							for (guint8 j = 0; j < dispencer_controllers[index].dispencers[i].nozzle_count; j++)
							{
								if (nozzle_num == dispencer_controllers[index].dispencers[i].nozzles[j].num)
								{
									result = TRUE;
									*nozzle_index = j;
									break;
								}
							}
							if (result) break;
						}
					}
				}
			}
		}
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

gboolean get_dc_dispencer_data_is_change(guint8 index, guint8 index_disp)
{
	gboolean result = FALSE;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && index_disp < dispencer_controllers[index].dispencers_count)
	{
		result = dispencer_controllers[index].dispencers[index_disp].data_is_changed;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_dispencer_data_is_change(guint8 index, guint8 index_disp, gboolean value)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && index_disp < dispencer_controllers[index].dispencers_count)
	{
		dispencer_controllers[index].dispencers[index_disp].data_is_changed = value;
	}

	g_mutex_unlock(&dc_mutex);
}

void set_dc_device_port(guint8 index, guint32 port)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].port = port;
	}

	g_mutex_unlock(&dc_mutex);

}

gint32 get_dc_device_port(guint8 index)
{
	gint32 result = 0;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].port;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_decimal_pointers(guint8 index, guint8 price_dp, guint8 volume_dp, guint8 amount_dp)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].price_dp = price_dp;
		dispencer_controllers[index].volume_dp = volume_dp;
		dispencer_controllers[index].amount_dp = amount_dp;
	}

	g_mutex_unlock(&dc_mutex);

}

void get_dc_decimal_pointers(guint8 index, guint8* price_dp, guint8* volume_dp, guint8* amount_dp)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		*price_dp = dispencer_controllers[index].price_dp;
		*volume_dp = dispencer_controllers[index].volume_dp;
		*amount_dp = dispencer_controllers[index].amount_dp;
	}

	g_mutex_unlock(&dc_mutex);

}

void set_dc_ext_func_count(guint8 index, guint8 count)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].ext_functions_count = count;
	}

	g_mutex_unlock(&dc_mutex);

}

guint8 get_dc_ext_func_count(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].ext_functions_count;
	}

	g_mutex_unlock(&dc_mutex);

	return result;

}

void set_dc_ext_func_name(guint8 index, guint8 func_index, gchar* name)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		if (dispencer_controllers[index].ext_functions[func_index].name == NULL)
		{
			g_free(dispencer_controllers[index].ext_functions[func_index].name);
			dispencer_controllers[index].ext_functions[func_index].name = NULL;
		}
		dispencer_controllers[index].ext_functions[func_index].name = g_strdup(name);
	}

	g_mutex_unlock(&dc_mutex);

}

gchar* get_dc_ext_func_name(guint8 index, guint8 func_index)
{
	gchar* result = NULL;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && func_index < dispencer_controllers[index].ext_functions_count)
	{
		result = g_strdup(dispencer_controllers[index].ext_functions[func_index].name);
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_disp_count(guint8 index, guint8 count)
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		dispencer_controllers[index].dispencers_count = count;
	}

	g_mutex_unlock(&dc_mutex);

}

guint8 get_dc_disp_count(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count)
	{
		result = dispencer_controllers[index].dispencers_count;
	}

	g_mutex_unlock(&dc_mutex);

	return result;
}

void set_dc_disp_info(guint8 index, guint8 index_disp, guint32 num, guint8 addr, guint8 nozzle_count )
{
	g_mutex_lock(&dc_mutex);

	if (index < MAX_DISP_CONTROL_COUNT && index_disp < MAX_DISPENCER_COUNT)
	{
		dispencer_controllers[index].dispencers[index_disp].active_nozzle_num = 0;
		dispencer_controllers[index].dispencers[index_disp].current_amount = 0;
		dispencer_controllers[index].dispencers[index_disp].current_price = 0;
		dispencer_controllers[index].dispencers[index_disp].current_volume = 0;
		dispencer_controllers[index].dispencers[index_disp].data_is_changed = FALSE;
		dispencer_controllers[index].dispencers[index_disp].error = 0;
		memset(dispencer_controllers[index].dispencers[index_disp].error_description, 0x00, MAX_DC_STR_LENGTH);
		dispencer_controllers[index].dispencers[index_disp].is_pay = FALSE;
		dispencer_controllers[index].dispencers[index_disp].order_type = ot_Free;
		dispencer_controllers[index].dispencers[index_disp].preset_amount = 0;
		dispencer_controllers[index].dispencers[index_disp].preset_nozzle_num = 0;
		dispencer_controllers[index].dispencers[index_disp].preset_order_type = ot_Free;
		dispencer_controllers[index].dispencers[index_disp].preset_price = 0;
		dispencer_controllers[index].dispencers[index_disp].preset_volume = 0;
		dispencer_controllers[index].dispencers[index_disp].state = ds_NotInitialize;

		dispencer_controllers[index].dispencers[index_disp].num = num;
		dispencer_controllers[index].dispencers[index_disp].addr = addr;
		dispencer_controllers[index].dispencers[index_disp].nozzle_count = nozzle_count;
	}

	g_mutex_unlock(&dc_mutex);
}

void get_dc_disp_info(guint8 index, guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count )
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count && index_disp < dispencer_controllers[index].dispencers_count)
	{
		*num = dispencer_controllers[index].dispencers[index_disp].num;
		*addr = dispencer_controllers[index].dispencers[index_disp].addr;
		*nozzle_count = dispencer_controllers[index].dispencers[index_disp].nozzle_count;
	}

	g_mutex_unlock(&dc_mutex);
}

void set_dc_nozzle_info(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8 num, guint8 grade )
{
	g_mutex_lock(&dc_mutex);

	if (index < MAX_DISP_CONTROL_COUNT &&
		index_disp < MAX_DISPENCER_COUNT &&
		index_nozzle < MAX_NOZZLE_COUNT)
	{
		dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].num = num;
		dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].grade = grade;
	}

	g_mutex_unlock(&dc_mutex);
}

void get_dc_nozzle_info(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8* num, guint8* grade )
{
	g_mutex_lock(&dc_mutex);

	if (index < MAX_DISP_CONTROL_COUNT &&
		index_disp < MAX_DISPENCER_COUNT &&
		index_nozzle < MAX_NOZZLE_COUNT)
	{
		*num = dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].num;
		*grade = dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].grade;
	}

	g_mutex_unlock(&dc_mutex);
}

void get_dc_nozzle_counter(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8* nozzle_num, guint32* counter )
{
	g_mutex_lock(&dc_mutex);

	if (index < dispencer_controller_count &&
		index_disp < dispencer_controllers[index].dispencers_count &&
		index_nozzle < dispencer_controllers[index].dispencers[index_disp].nozzle_count)
	{
		*counter = dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].counter;
		*nozzle_num = dispencer_controllers[index].dispencers[index_disp].nozzles[index_nozzle].num;
	}

	g_mutex_unlock(&dc_mutex);
}

void set_dc_dispencer_data(guint8 dci, guint8 disp_index, gchar* device_name, guint32 disp_num, DcDeviceError last_device_error, DispencerState disp_state,
							OrderType preset_order_type, guint8 preset_nozzle_num, guint32 preset_price, guint32 preset_volume, guint32 preset_amount,
							OrderType	order_type,  guint8 active_nozzle_num, guint32 current_price, guint32 current_volume, guint32 current_amount,
							gboolean is_pay, guchar error, guchar* error_description, gboolean counters_ready, guint8 nozzle_count, guint32 counters[MAX_NOZZLE_COUNT], LogParams* log_params, gboolean log_trace )
{
	g_mutex_lock(&dc_mutex);

	dispencer_controllers[dci].last_device_error = last_device_error;

	if (dci < dispencer_controller_count && disp_index < dispencer_controllers[dci].dispencers_count)
	{
		if (dispencer_controllers[dci].dispencers[disp_index].state != disp_state)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change state: %d -> %d (%s -> %s)",
					disp_num,dispencer_controllers[dci].dispencers[disp_index].state,  disp_state, dispencer_state_to_str(dispencer_controllers[dci].dispencers[disp_index].state),  dispencer_state_to_str(disp_state));

			dispencer_controllers[dci].dispencers[disp_index].state = disp_state;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].preset_order_type != preset_order_type)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change preset order type:  %d -> %d (%s -> %s)",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].preset_order_type, preset_order_type,
					order_type_to_str(dispencer_controllers[dci].dispencers[disp_index].preset_order_type), order_type_to_str(preset_order_type));
			dispencer_controllers[dci].dispencers[disp_index].preset_order_type = preset_order_type;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].preset_nozzle_num != preset_nozzle_num)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change preset nozzle: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].preset_nozzle_num, preset_nozzle_num);
			dispencer_controllers[dci].dispencers[disp_index].preset_nozzle_num = preset_nozzle_num;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].preset_price != preset_price)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change preset price: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].preset_price, preset_price);
			dispencer_controllers[dci].dispencers[disp_index].preset_price = preset_price;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].preset_volume != preset_volume)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change preset volume: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].preset_volume, preset_volume);
			dispencer_controllers[dci].dispencers[disp_index].preset_volume = preset_volume;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].preset_amount != preset_amount)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change preset amount: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].preset_amount, preset_amount);
			dispencer_controllers[dci].dispencers[disp_index].preset_amount = preset_amount;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].order_type != order_type)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change order type:  %d -> %d (%s -> %s)",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].order_type, order_type,
					order_type_to_str(dispencer_controllers[dci].dispencers[disp_index].order_type), order_type_to_str(order_type));

			dispencer_controllers[dci].dispencers[disp_index].order_type = order_type;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].active_nozzle_num != active_nozzle_num)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change active nozzle: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].active_nozzle_num, active_nozzle_num);

			dispencer_controllers[dci].dispencers[disp_index].active_nozzle_num = active_nozzle_num;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].current_price != current_price)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change current price: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].current_price, current_price);
			dispencer_controllers[dci].dispencers[disp_index].current_price = current_price;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].current_volume != current_volume)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change current volume: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].current_volume, current_volume);
			dispencer_controllers[dci].dispencers[disp_index].current_volume = current_volume;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].current_amount != current_amount)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change current amount: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].current_amount, current_amount);
			dispencer_controllers[dci].dispencers[disp_index].current_amount = current_amount;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].is_pay != is_pay)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change is pay: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].is_pay, is_pay);
			dispencer_controllers[dci].dispencers[disp_index].is_pay = is_pay;
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (dispencer_controllers[dci].dispencers[disp_index].error != error)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d change error: %d - > %d",
					disp_num, dispencer_controllers[dci].dispencers[disp_index].error, error);
			dispencer_controllers[dci].dispencers[disp_index].error = error;
			memcpy(&dispencer_controllers[dci].dispencers[disp_index].error_description, &error_description, MAX_DC_STR_LENGTH);
			dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
		}

		if (counters_ready && nozzle_count > 0)
		{
			for (guint8 i = 0; i < nozzle_count && i < MAX_NOZZLE_COUNT; i++)
			{
				if (dispencer_controllers[dci].dispencers[disp_index].nozzles[i].counter != counters[i])
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Dispencer %d  nozzle %d change counter: %d - > %d",
							disp_num,dispencer_controllers[dci].dispencers[disp_index].nozzles[i].num, dispencer_controllers[dci].dispencers[disp_index].nozzles[i].counter, counters[i]);
					dispencer_controllers[dci].dispencers[disp_index].nozzles[i].counter = counters[i];
					dispencer_controllers[dci].dispencers[disp_index].data_is_changed = TRUE;
				}

			}
		}



	}

	g_mutex_unlock(&dc_mutex);

}

