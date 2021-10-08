#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "fr_device.h"
//#include "dc_device_data.h"
//#include "dc_func.h"

FiscalRegister fiscal_registers[MAX_FR_COUNT];
guint8 fiscal_register_count = 0;
GMutex	fr_mutex;

void fr_init()
{
	g_mutex_init(&fr_mutex);
}


gboolean get_new_fiscal_register_index(guint8* index)
{
	gboolean result = FALSE;

	g_mutex_lock(&fr_mutex);

	if (fiscal_register_count < MAX_FR_COUNT)
	{
		*index = fiscal_register_count;

		fiscal_register_count++;
		result = TRUE;
	}

	g_mutex_unlock(&fr_mutex);

	return result;
}

gboolean init_fr_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days)
{
	gboolean result = FALSE;

	g_mutex_lock(&fr_mutex);


	if (index < fiscal_register_count)
	{
		destroy_log_params(&fiscal_registers[index].log_params);

		fiscal_registers[index].log_trace = log_trace;

		create_log_params(&fiscal_registers[index].log_params, log_enable, log_dir, device_name, file_size, save_days);

		if (log_enable)
		{
			//create log
			if (!init_log_dir(&fiscal_registers[index].log_params))
			{
				g_printf("%s : error creating log dir\n", device_name);
				return result;
			}

			gchar* log_filename = g_strdup_printf("%s.log", device_name);

			if (fiscal_registers[index].log_params.log != NULL)
			{
				close_log(&fiscal_registers[index].log_params);

			}

			create_log(&fiscal_registers[index].log_params);
			if (fiscal_registers[index].log_params.log == NULL)
			{
				g_printf("%s : log create error\n", device_name);
				return result;
			}

			g_free(log_filename);

			result = TRUE;
		}


	}


	g_mutex_unlock(&fr_mutex);

	return result;

}

void get_fr_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		*log_params = &fiscal_registers[index].log_params;
		*log_trace = fiscal_registers[index].log_trace;
	}

	g_mutex_unlock(&fr_mutex);

}

void set_fr_device_status(guint8 index, FrDeviceStatus status)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		if (fiscal_registers[index].device_status != status)
		{
			fiscal_registers[index].device_status = status;
			fiscal_registers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&fr_mutex);

}

void set_fr_device_name(guint8 index, gchar* name)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		if (fiscal_registers[index].name != NULL)
		{
			g_free(fiscal_registers[index].name);
			fiscal_registers[index].name = NULL;
		}
		fiscal_registers[index].name = g_strdup(name);
	}

	g_mutex_unlock(&fr_mutex);

}

void set_fr_device_port(guint8 index, guint32 port)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		fiscal_registers[index].port = port;
	}

	g_mutex_unlock(&fr_mutex);

}

void set_fr_device_is_working(guint8 index, gboolean value)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		fiscal_registers[index].device_is_working = value;
	}

	g_mutex_unlock(&fr_mutex);

}

gboolean get_fr_device_is_working(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].device_is_working;
	}

	g_mutex_unlock(&fr_mutex);

	return result;
}

void set_fr_device_last_error(guint8 index, FrDeviceError error)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		if (fiscal_registers[index].last_device_error != error)
		{
			fiscal_registers[index].last_device_error = error;
			fiscal_registers[index].device_status_is_changed = TRUE;
		}
	}

	g_mutex_unlock(&fr_mutex);
}

FrDeviceError get_fr_device_last_erorr(guint8 index)
{
	FrDeviceError result = fre_Undefined;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].last_device_error;
	}

	g_mutex_unlock(&fr_mutex);

	return result;

}

FrDeviceStatus get_fr_device_status(guint8 index)
{
	FrDeviceStatus result = frs_UndefinedError;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].device_status;

	}

	g_mutex_unlock(&fr_mutex);

	return result;

}

ThreadStatus get_fr_main_sock_thread_status(guint8 index)
{
	ThreadStatus result = ts_Undefined;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].main_sock_status;
	}

	g_mutex_unlock(&fr_mutex);

	return result;

}

void set_fr_main_sock_thread_status(guint8 index, ThreadStatus status)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		fiscal_registers[index].main_sock_status = status;
	}

	g_mutex_unlock(&fr_mutex);
}

gint32 get_fr_sock(guint8 index)
{
	gint32 result = -1;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].sock;
	}

	g_mutex_unlock(&fr_mutex);

	return result;

}

void set_fr_sock_status(guint8 index, SocketStatus sock_status)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		fiscal_registers[index].sock_status = sock_status;
	}

	g_mutex_unlock(&fr_mutex);

}

SocketStatus get_fr_sock_status(guint8 index)
{
	SocketStatus result = ss_Disconnected;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].sock_status;
	}

	g_mutex_unlock(&fr_mutex);

	return result;
}

gboolean get_fr_device_status_is_changed(guint8 index)
{
	gboolean result = FALSE;

	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		result = fiscal_registers[index].device_status_is_changed;
	}

	g_mutex_unlock(&fr_mutex);

	return result;

}

void set_fr_device_status_is_changed(guint8 index, gboolean value)
{
	g_mutex_lock(&fr_mutex);

	if (index < fiscal_register_count)
	{
		fiscal_registers[index].device_status_is_changed = value;
	}

	g_mutex_unlock(&fr_mutex);

}

