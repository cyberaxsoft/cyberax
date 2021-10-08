#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "ppc_device.h"
#include "ppc_device_data.h"
#include "ppc_func.h"

PricePoleController price_pole_controllers[MAX_PRICE_POLE_CONTROL_COUNT];
guint8 price_pole_controller_count = 0;
GMutex	ppc_mutex;

void ppc_init()
{
	g_mutex_init(&ppc_mutex);
}

ThreadStatus get_ppc_main_sock_thread_status(guint8 index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].main_sock_status;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;

}

void set_ppc_main_sock_thread_status(guint8 index, ThreadStatus status)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].main_sock_status = status;
	}

	g_mutex_unlock(&ppc_mutex);
}

gint32 get_ppc_sock(guint8 index)
{
	gint32 result = -1;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].sock;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;

}

void set_ppc_sock_status(guint8 index, SocketStatus sock_status)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].sock_status = sock_status;
	}

	g_mutex_unlock(&ppc_mutex);

}

SocketStatus get_ppc_sock_status(guint8 index)
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].sock_status;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

gboolean get_new_ppc_index(guint8* index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (price_pole_controller_count < MAX_PRICE_POLE_CONTROL_COUNT)
	{
		*index = price_pole_controller_count;

		price_pole_controller_count++;
		result = TRUE;

	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

//------------------------ device_is_working--------------------------------------

void set_ppc_device_is_working(guint8 index, gboolean value)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].device_is_working = value;
	}

	g_mutex_unlock(&ppc_mutex);

}

gboolean get_ppc_device_is_working(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].device_is_working;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

//------------------------ device_status--------------------------------------

void set_ppc_device_status(guint8 index, PpcDeviceStatus status)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		if (price_pole_controllers[index].device_status != status)
		{
			price_pole_controllers[index].device_status = status;
			price_pole_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&ppc_mutex);

}

PpcDeviceStatus get_ppc_device_status(guint8 index)
{
	PpcDeviceStatus result = ppcds_UndefinedError;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].device_status;

	}

	g_mutex_unlock(&ppc_mutex);

	return result;

}

//------------------------ device_last_error--------------------------------------

void set_ppc_device_last_error(guint8 index, PpcDeviceError error)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		if (price_pole_controllers[index].last_device_error != error)
		{
			price_pole_controllers[index].last_device_error = error;
			price_pole_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&ppc_mutex);



}

PpcDeviceError get_ppc_device_last_erorr(guint8 index)
{
	PpcDeviceError result = ppce_Undefined;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].last_device_error;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;

}

//------------------------ device_status_is_changed--------------------------------------

void set_ppc_device_status_is_changed(guint8 index, gboolean value)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].device_status_is_changed = value;
	}

	g_mutex_unlock(&ppc_mutex);

}

gboolean get_ppc_device_status_is_changed(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].device_status_is_changed;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;

}

//----------------------- all ppc data -------------------------------

gboolean get_ppc_price_pole_data(guint8 index, guint8 price_pole_index, guint8* num, guint8* grade, guint32* price, PricePoleState* state )
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		if (price_pole_index < price_pole_controllers[index].price_pole_count)
		{
			*num = price_pole_controllers[index].price_poles[price_pole_index].num;
			*grade = price_pole_controllers[index].price_poles[price_pole_index].grade;
			*price = price_pole_controllers[index].price_poles[price_pole_index].price;
			*state = price_pole_controllers[index].price_poles[price_pole_index].state;

			result = TRUE;
		}
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

void set_ppc_price_pole_data(guint8 index, guint8 price_pole_index, guint32 price, PricePoleState state )
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		if (price_pole_index < price_pole_controllers[index].price_pole_count)
		{
			gboolean change_flag = FALSE;

			if (price_pole_controllers[index].price_poles[price_pole_index].price != price)
			{
				change_flag = TRUE;
				price_pole_controllers[index].price_poles[price_pole_index].price = price;

			}
			if (price_pole_controllers[index].price_poles[price_pole_index].state != state)
			{
				change_flag = TRUE;
				price_pole_controllers[index].price_poles[price_pole_index].state = state;
			}
			price_pole_controllers[index].data_is_changed = change_flag;
		}
	}

	g_mutex_unlock(&ppc_mutex);
}


//------------------------ device_name--------------------------------------

void set_ppc_device_name(guint8 index, gchar* name)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		if (price_pole_controllers[index].name != NULL)
		{
			g_free(price_pole_controllers[index].name);
			price_pole_controllers[index].name = NULL;
		}
		price_pole_controllers[index].name = g_strdup(name);
	}

	g_mutex_unlock(&ppc_mutex);

}

gchar* get_ppc_device_name(guint8 index)
{
	gchar* result = NULL;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = g_strdup(price_pole_controllers[index].name);
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

void set_ppc_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].sock = sock;
	}

	g_mutex_unlock(&ppc_mutex);

}

gboolean init_ppc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		destroy_log_params(&price_pole_controllers[index].log_params);

		price_pole_controllers[index].log_trace = log_trace;

		create_log_params(&price_pole_controllers[index].log_params, log_enable, log_dir, device_name, file_size, save_days);

		if (log_enable)
		{
			//create log
			if (!init_log_dir(&price_pole_controllers[index].log_params))
			{
				g_printf("%s : error creating log dir\n", device_name);
				return result;
			}

			gchar* log_filename = g_strdup_printf("%s.log", device_name);

			if (price_pole_controllers[index].log_params.log != NULL)
			{
				close_log(&price_pole_controllers[index].log_params);

			}

			create_log(&price_pole_controllers[index].log_params);
			if (price_pole_controllers[index].log_params.log == NULL)
			{
				g_printf("%s : log create error\n", device_name);
				return result;
			}

			g_free(log_filename);

			result = TRUE;
		}


	}


	g_mutex_unlock(&ppc_mutex);

	return result;

}

void get_ppc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		*log_params = &price_pole_controllers[index].log_params;
		*log_trace = price_pole_controllers[index].log_trace;
	}

	g_mutex_unlock(&ppc_mutex);

}


guint8 get_ppc_price_pole_num(guint8 index, guint8 index_price_pole)
{
	guint8 result = 0;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count && index_price_pole < price_pole_controllers[index].price_pole_count)
	{
		result = price_pole_controllers[index].price_poles[index_price_pole].num;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}


gboolean ppc_price_pole_num_is_present(guint8 index, guint8 price_pole_num, guint8* index_price_pole)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index <  price_pole_controller_count && price_pole_controllers[index].price_pole_count > 0)
	{
		for (guint8 i = 0; i < price_pole_controllers[index].price_pole_count; i++)
		{
			if (price_pole_num == price_pole_controllers[index].price_poles[i].num)
			{
				result = TRUE;
				*index_price_pole = i;
				break;
			}

		}
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

gboolean get_ppc_price_pole_data_is_change(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count )
	{
		result = price_pole_controllers[index].data_is_changed;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

void set_ppc_price_pole_data_is_change(guint8 index, gboolean value)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].data_is_changed = value;
	}

	g_mutex_unlock(&ppc_mutex);
}

void set_ppc_device_port(guint8 index, guint32 port)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].port = port;
	}

	g_mutex_unlock(&ppc_mutex);

}

gint32 get_ppc_device_port(guint8 index)
{
	gint32 result = 0;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].port;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

void set_ppc_price_pole_count(guint8 index, guint8 count)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].price_pole_count = count;
	}

	g_mutex_unlock(&ppc_mutex);

}

guint8 get_ppc_price_pole_count(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		result = price_pole_controllers[index].price_pole_count;
	}

	g_mutex_unlock(&ppc_mutex);

	return result;
}

void set_ppc_price_pole_info(guint8 index, guint8 index_price_pole, guint8 num, guint8 grade, guint8 symbol_count )
{
	g_mutex_lock(&ppc_mutex);

	if (index < MAX_PRICE_POLE_CONTROL_COUNT && index_price_pole < MAX_PRICE_POLE_COUNT)
	{
		price_pole_controllers[index].price_poles[index_price_pole].price = 0;
		price_pole_controllers[index].price_poles[index_price_pole].state = pps_NotInitialize;

		price_pole_controllers[index].price_poles[index_price_pole].num = num;
		price_pole_controllers[index].price_poles[index_price_pole].grade = grade;
		price_pole_controllers[index].price_poles[index_price_pole].symbol_count = symbol_count;
	}

	g_mutex_unlock(&ppc_mutex);
}

void get_ppc_price_pole_info(guint8 index, guint8 index_price_pole, guint8* num, guint8* grade, guint8* symbol_count)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count && index_price_pole < price_pole_controllers[index].price_pole_count)
	{
		*num = price_pole_controllers[index].price_poles[index_price_pole].num;
		*grade = price_pole_controllers[index].price_poles[index_price_pole].grade;
		*symbol_count = price_pole_controllers[index].price_poles[index_price_pole].symbol_count;
	}

	g_mutex_unlock(&ppc_mutex);
}

void set_ppc_decimal_pointers(guint8 index, guint8 price_dp)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		price_pole_controllers[index].price_dp = price_dp;
	}

	g_mutex_unlock(&ppc_mutex);

}

void get_ppc_decimal_pointers(guint8 index, guint8* price_dp)
{
	g_mutex_lock(&ppc_mutex);

	if (index < price_pole_controller_count)
	{
		*price_dp = price_pole_controllers[index].price_dp;
	}

	g_mutex_unlock(&ppc_mutex);

}


