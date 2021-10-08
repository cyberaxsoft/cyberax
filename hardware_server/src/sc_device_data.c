
#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "sc_device.h"
//#include "sc_device_data.h"

SensorController sensor_controllers[MAX_SC_CONTROL_COUNT];
guint8  sensor_controller_count = 0;
GMutex	 sensor_controller_mutex;

void sc_init()
{
	g_mutex_init(&sensor_controller_mutex);
}

ThreadStatus get_sc_main_sock_thread_status(guint8 index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].main_sock_status;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

void set_sc_main_sock_thread_status(guint8 index, ThreadStatus status)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].main_sock_status = status;
	}

	g_mutex_unlock(&sensor_controller_mutex);
}

gint32 get_sc_sock(guint8 index)
{
	gint32 result = -1;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].sock;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

void set_sc_sock_status(guint8 index, SocketStatus sock_status)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].sock_status = sock_status;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

SocketStatus get_sc_sock_status(guint8 index)
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].sock_status;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

gboolean get_new_sc_index(guint8* index)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (sensor_controller_count < MAX_SC_CONTROL_COUNT)
	{
		*index = sensor_controller_count;

		sensor_controller_count++;
		result = TRUE;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

//------------------------ device_is_working--------------------------------------

void set_sc_device_is_working(guint8 index, gboolean value)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].device_is_working = value;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

gboolean get_sc_device_is_working(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].device_is_working;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

//------------------------ device_status--------------------------------------

void set_sc_device_status(guint8 index, ScDeviceStatus status)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		if (sensor_controllers[index].device_status != status)
		{
			sensor_controllers[index].device_status = status;
			sensor_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

ScDeviceStatus get_sc_device_status(guint8 index)
{
	ScDeviceStatus result = scs_UndefinedError;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].device_status;

	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

//------------------------ device_last_error--------------------------------------

void set_sc_device_last_error(guint8 index, ScDeviceError error)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		if (sensor_controllers[index].last_device_error != error)
		{
			sensor_controllers[index].last_device_error = error;
			sensor_controllers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&sensor_controller_mutex);



}

ScDeviceError get_sc_device_last_erorr(guint8 index)
{
	ScDeviceError result = sce_Undefined;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].last_device_error;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

//------------------------ device_status_is_changed--------------------------------------

void set_sc_device_status_is_changed(guint8 index, gboolean value)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].device_status_is_changed = value;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

gboolean get_sc_device_status_is_changed(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].device_status_is_changed;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

//----------------------- all sc data -------------------------------

gboolean get_sc_sensor_data(guint8 index, guint8 sensor_index, guint32* num, SensorParams* params, gboolean* online)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		if (sensor_index < sensor_controllers[index].sensor_count)
		{
			*num = sensor_controllers[index].sensors[sensor_index].num;
			memcpy(params, &sensor_controllers[index].sensors[sensor_index].params, sizeof(SensorParams));
			*online = sensor_controllers[index].sensors[sensor_index].online;

			result = TRUE;
		}
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

void set_sc_sensor_data(guint8 index, guint8 sensor_index, SensorParams* params, gboolean online )
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		if (sensor_index < sensor_controllers[index].sensor_count)
		{
			if (memcmp(params, &sensor_controllers[index].sensors[sensor_index].params, sizeof(SensorParams)) != 0)
			{
				memcpy(&sensor_controllers[index].sensors[sensor_index].params, params, sizeof(SensorParams));
				sensor_controllers[index].sensors[sensor_index].data_is_changed = TRUE;
			}
			if (sensor_controllers[index].sensors[sensor_index].online  != online)
			{
				sensor_controllers[index].sensors[sensor_index].online = online;
				sensor_controllers[index].sensors[sensor_index].data_is_changed = TRUE;
			}
		}
	}

	g_mutex_unlock(&sensor_controller_mutex);
}


//------------------------ device_name--------------------------------------

void set_sc_device_name(guint8 index, gchar* name)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		if (sensor_controllers[index].name != NULL)
		{
			g_free(sensor_controllers[index].name);
			sensor_controllers[index].name = NULL;
		}
		sensor_controllers[index].name = g_strdup(name);
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

gchar* get_sc_device_name(guint8 index)
{
	gchar* result = NULL;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = g_strdup(sensor_controllers[index].name);
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

void set_sc_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].sock = sock;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

gboolean init_sc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		destroy_log_params(&sensor_controllers[index].log_params);

		sensor_controllers[index].log_trace = log_trace;

		create_log_params(&sensor_controllers[index].log_params, log_enable, log_dir, device_name, file_size, save_days);

		if (log_enable)
		{
			//create log
			if (!init_log_dir(&sensor_controllers[index].log_params))
			{
				g_printf("%s : error creating log dir\n", device_name);
				return result;
			}

			gchar* log_filename = g_strdup_printf("%s.log", device_name);

			if (sensor_controllers[index].log_params.log != NULL)
			{
				close_log(&sensor_controllers[index].log_params);

			}

			create_log(&sensor_controllers[index].log_params);
			if (sensor_controllers[index].log_params.log == NULL)
			{
				g_printf("%s : log create error\n", device_name);
				return result;
			}

			g_free(log_filename);

			result = TRUE;
		}
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;

}

void get_sc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		*log_params = &sensor_controllers[index].log_params;
		*log_trace = sensor_controllers[index].log_trace;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

guint32 get_sc_sensor_num(guint8 index, guint8 sensor_index)
{
	guint32 result = 0;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count && sensor_index < sensor_controllers[index].sensor_count)
	{
		result = sensor_controllers[index].sensors[sensor_index].num;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

gboolean sc_sensor_num_is_present(guint8 index, guint32 sensor_num, guint8* sensor_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index <  sensor_controller_count && sensor_controllers[index].sensor_count > 0)
	{
		for (guint8 i = 0; i < sensor_controllers[index].sensor_count; i++)
		{
			if (sensor_num == sensor_controllers[index].sensors[i].num)
			{
				result = TRUE;
				*sensor_index = i;
				break;
			}

		}
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

gboolean get_sc_sensor_data_is_change(guint8 index, guint8 sensor_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count && sensor_index < sensor_controllers[index].sensor_count)
	{
		result = sensor_controllers[index].sensors[sensor_index].data_is_changed;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

void set_sc_sensor_data_is_change(guint8 index, guint8 sensor_index, gboolean value)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count && sensor_index < sensor_controllers[index].sensor_count)
	{
		sensor_controllers[index].sensors[sensor_index].data_is_changed = value;
	}

	g_mutex_unlock(&sensor_controller_mutex);
}

void set_sc_device_port(guint8 index, guint32 port)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].port = port;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

gint32 get_sc_device_port(guint8 index)
{
	gint32 result = 0;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].port;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

void set_sc_sensor_count(guint8 index, guint8 count)
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		sensor_controllers[index].sensor_count = count;
	}

	g_mutex_unlock(&sensor_controller_mutex);

}

guint8 get_sc_sensor_count(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count)
	{
		result = sensor_controllers[index].sensor_count;
	}

	g_mutex_unlock(&sensor_controller_mutex);

	return result;
}

void set_sc_sensor_info(guint8 index, guint8 sensor_index, guint32 num, guint8 addr )
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < MAX_SC_CONTROL_COUNT && sensor_index < MAX_SENSOR_COUNT)
	{
		memset(&sensor_controllers[index].sensors[sensor_index].params, 0x00, sizeof(SensorParams));

		sensor_controllers[index].sensors[sensor_index].num = num;
		sensor_controllers[index].sensors[sensor_index].addr = addr;
	}

	g_mutex_unlock(&sensor_controller_mutex);
}

void get_sc_sensor_info(guint8 index, guint8 sensor_index, guint32* num, guint8* addr, SensorParams* params )
{
	g_mutex_lock(&sensor_controller_mutex);

	if (index < sensor_controller_count && sensor_index < sensor_controllers[index].sensor_count)
	{
		*num = sensor_controllers[index].sensors[sensor_index].num;
		*addr = sensor_controllers[index].sensors[sensor_index].addr;

		*params = sensor_controllers[index].sensors[sensor_index].params;
	}

	g_mutex_unlock(&sensor_controller_mutex);
}

