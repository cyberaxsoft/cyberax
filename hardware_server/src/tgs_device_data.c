#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tgs_device.h"
//#include "tgs_device_data.h"

Tgs tgs[MAX_TGS_CONTROL_COUNT];
guint8 tgs_count = 0;
GMutex	tgs_mutex;

void tgs_init()
{
	g_mutex_init(&tgs_mutex);
}

ThreadStatus get_tgs_main_sock_thread_status(guint8 index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].main_sock_status;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;

}

void set_tgs_main_sock_thread_status(guint8 index, ThreadStatus status)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].main_sock_status = status;
	}

	g_mutex_unlock(&tgs_mutex);
}

gint32 get_tgs_sock(guint8 index)
{
	gint32 result = -1;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].sock;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;

}

void set_tgs_sock_status(guint8 index, SocketStatus sock_status)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].sock_status = sock_status;
	}

	g_mutex_unlock(&tgs_mutex);

}

SocketStatus get_tgs_sock_status(guint8 index)
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].sock_status;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

gboolean get_new_tgs_index(guint8* index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (tgs_count < MAX_TGS_CONTROL_COUNT)
	{
		*index = tgs_count;

		tgs_count++;
		result = TRUE;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

//------------------------ device_is_working--------------------------------------

void set_tgs_device_is_working(guint8 index, gboolean value)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].device_is_working = value;
	}

	g_mutex_unlock(&tgs_mutex);

}

gboolean get_tgs_device_is_working(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].device_is_working;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

//------------------------ device_status--------------------------------------

void set_tgs_device_status(guint8 index, TgsDeviceStatus status)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		if (tgs[index].device_status != status)
		{
			tgs[index].device_status = status;
			tgs[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&tgs_mutex);

}

TgsDeviceStatus get_tgs_device_status(guint8 index)
{
	TgsDeviceStatus result = tgss_UndefinedError;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].device_status;

	}

	g_mutex_unlock(&tgs_mutex);

	return result;

}

//------------------------ device_last_error--------------------------------------

void set_tgs_device_last_error(guint8 index, TgsDeviceError error)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		if (tgs[index].last_device_error != error)
		{
			tgs[index].last_device_error = error;
			tgs[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&tgs_mutex);



}

TgsDeviceError get_tgs_device_last_erorr(guint8 index)
{
	TgsDeviceError result = tgse_Undefined;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].last_device_error;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;

}

//------------------------ device_status_is_changed--------------------------------------

void set_tgs_device_status_is_changed(guint8 index, gboolean value)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].device_status_is_changed = value;
	}

	g_mutex_unlock(&tgs_mutex);

}

gboolean get_tgs_device_status_is_changed(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].device_status_is_changed;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;

}

//----------------------- all tgs data -------------------------------

gboolean get_tgs_tank_data(guint8 index, guint8 tank_index, guint32* num, gfloat* height, gfloat* volume, gfloat* weight, gfloat* density, gfloat* temperature, gfloat* water_level, gboolean* online )
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		if (tank_index < tgs[index].tank_count)
		{
			*num = tgs[index].tanks[tank_index].num;
			*height = tgs[index].tanks[tank_index].height;
			*volume = tgs[index].tanks[tank_index].volume;
			*weight = tgs[index].tanks[tank_index].weight;
			*density = tgs[index].tanks[tank_index].density;
			*temperature = tgs[index].tanks[tank_index].temperature;
			*water_level = tgs[index].tanks[tank_index].water_level;
			*online = tgs[index].tanks[tank_index].online;

			result = TRUE;
		}
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

void set_tgs_tank_data(guint8 index, guint8 tank_index, gfloat height, gfloat volume, gfloat weight, gfloat density, gfloat temperature, gfloat water_level, gboolean online )
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		if (tank_index < tgs[index].tank_count)
		{
			if (tgs[index].tanks[tank_index].height != height)
			{
				tgs[index].tanks[tank_index].height = height;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if(tgs[index].tanks[tank_index].volume != volume)
			{
				tgs[index].tanks[tank_index].volume = volume;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if (tgs[index].tanks[tank_index].weight != weight)
			{
				tgs[index].tanks[tank_index].weight = weight;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if (tgs[index].tanks[tank_index].density != density)
			{
				tgs[index].tanks[tank_index].density = density;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if (tgs[index].tanks[tank_index].temperature != temperature)
			{
				tgs[index].tanks[tank_index].temperature = temperature;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if (tgs[index].tanks[tank_index].water_level != water_level)
			{
				tgs[index].tanks[tank_index].water_level = water_level;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
			if (tgs[index].tanks[tank_index].online != online)
			{
				tgs[index].tanks[tank_index].online = online;
				tgs[index].tanks[tank_index].data_is_changed = TRUE;
			}
		}
	}

	g_mutex_unlock(&tgs_mutex);
}


//------------------------ device_name--------------------------------------

void set_tgs_device_name(guint8 index, gchar* name)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		if (tgs[index].name != NULL)
		{
			g_free(tgs[index].name);
			tgs[index].name = NULL;
		}
		tgs[index].name = g_strdup(name);
	}

	g_mutex_unlock(&tgs_mutex);

}

gchar* get_tgs_device_name(guint8 index)
{
	gchar* result = NULL;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = g_strdup(tgs[index].name);
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

void set_tgs_sock(guint8 index, gint32 sock)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].sock = sock;
	}

	g_mutex_unlock(&tgs_mutex);

}

gboolean init_tgs_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		destroy_log_params(&tgs[index].log_params);

		tgs[index].log_trace = log_trace;

		create_log_params(&tgs[index].log_params, log_enable, log_dir, device_name, file_size, save_days);

		if (log_enable)
		{
			//create log
			if (!init_log_dir(&tgs[index].log_params))
			{
				g_printf("%s : error creating log dir\n", device_name);
				return result;
			}

			gchar* log_filename = g_strdup_printf("%s.log", device_name);

			if (tgs[index].log_params.log != NULL)
			{
				close_log(&tgs[index].log_params);

			}

			create_log(&tgs[index].log_params);
			if (tgs[index].log_params.log == NULL)
			{
				g_printf("%s : log create error\n", device_name);
				return result;
			}

			g_free(log_filename);

			result = TRUE;
		}


	}


	g_mutex_unlock(&tgs_mutex);

	return result;

}

void get_tgs_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		*log_params = &tgs[index].log_params;
		*log_trace = tgs[index].log_trace;
	}

	g_mutex_unlock(&tgs_mutex);

}


guint32 get_tgs_tank_num(guint8 index, guint8 index_tank)
{
	guint32 result = 0;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count && index_tank < tgs[index].tank_count)
	{
		result = tgs[index].tanks[index_tank].num;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}


gboolean tgs_tank_num_is_present(guint8 index, guint32 tank_num, guint8* index_tank)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index <  tgs_count && tgs[index].tank_count > 0)
	{
		for (guint8 i = 0; i < tgs[index].tank_count; i++)
		{
			if (tank_num == tgs[index].tanks[i].num)
			{
				result = TRUE;
				*index_tank = i;
				break;
			}

		}
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

gboolean get_tgs_tank_data_is_change(guint8 index, guint8 index_tank)
{
	gboolean result = FALSE;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count && index_tank < tgs[index].tank_count)
	{
		result = tgs[index].tanks[index_tank].data_is_changed;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

void set_tgs_tank_data_is_change(guint8 index, guint8 index_tank, gboolean value)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count && index_tank < tgs[index].tank_count)
	{
		tgs[index].tanks[index_tank].data_is_changed = value;
	}

	g_mutex_unlock(&tgs_mutex);
}

void set_tgs_device_port(guint8 index, guint32 port)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].port = port;
	}

	g_mutex_unlock(&tgs_mutex);

}

gint32 get_tgs_device_port(guint8 index)
{
	gint32 result = 0;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].port;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

void set_tgs_tank_count(guint8 index, guint8 count)
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		tgs[index].tank_count = count;
	}

	g_mutex_unlock(&tgs_mutex);

}

guint8 get_tgs_tank_count(guint8 index)
{
	guint8 result = 0;

	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count)
	{
		result = tgs[index].tank_count;
	}

	g_mutex_unlock(&tgs_mutex);

	return result;
}

void set_tgs_tank_info(guint8 index, guint8 index_tank, guint32 num, guint8 channel )
{
	g_mutex_lock(&tgs_mutex);

	if (index < MAX_TGS_CONTROL_COUNT && index_tank < MAX_TANK_COUNT)
	{
		tgs[index].tanks[index_tank].height = 0;
		tgs[index].tanks[index_tank].volume = 0;
		tgs[index].tanks[index_tank].weight = 0;
		tgs[index].tanks[index_tank].density = 0;
		tgs[index].tanks[index_tank].online = FALSE;
		tgs[index].tanks[index_tank].water_level = 0;
		tgs[index].tanks[index_tank].temperature = 0;

		tgs[index].tanks[index_tank].num = num;
		tgs[index].tanks[index_tank].channel = channel;
	}

	g_mutex_unlock(&tgs_mutex);
}

void get_tgs_tank_info(guint8 index, guint8 index_tank, guint32* num, guint8* channel )
{
	g_mutex_lock(&tgs_mutex);

	if (index < tgs_count && index_tank < tgs[index].tank_count)
	{
		*num = tgs[index].tanks[index_tank].num;
		*channel = tgs[index].tanks[index_tank].channel;
	}

	g_mutex_unlock(&tgs_mutex);
}

