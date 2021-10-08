#include <glib.h>
#include <glib/gstdio.h>
#include <dlfcn.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tgs_device.h"
#include "tgs_device_data.h"

void set_tank_config_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi, LogParams* log_params, gboolean log_trace)
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting tank configuration...");

	guchar (*get_tank_count)(guchar* count);
	guchar (*get_tank_info)(guint8 index, guint32* num, guint8* channel);

	guint8 count = 0;

	get_tank_count = dlsym( handle_lib, "get_tank_count" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
		g_free(error);
	}
	else
	{
		g_free(error);

		guchar device_error = (*get_tank_count)(&count);

		set_tgs_device_last_error(tgsi, device_error);

		if (device_error!= tgse_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get tank count error. Activated default settings.");
		}
		else
		{
			if (count > 0)
			{
				get_tank_info = dlsym( handle_lib, "get_tank_info" );
				error = dlerror();

				if (error != NULL)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error );
					g_free(error);
				}
				else
				{
					g_free(error);


					for (guint8 i = 0; i < count; i++)
					{
						guint32 tank_num = 0;
						guint8 tank_channel = 0;

						device_error = (*get_tank_info)(i, &tank_num, &tank_channel);

						set_tgs_device_last_error(tgsi, device_error);

						if (device_error!= tgse_NoError)
						{
							add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting tank %d info error.", i );
							count = 0;
						}
						else
						{
							set_tgs_tank_info(tgsi, i, tank_num, tank_channel);
						}
					}
				}
			}
			set_tgs_tank_count(tgsi, count);
		}
	}

}

void set_tgs_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi)
{
	guchar (*lib_func)();

	TgsDeviceStatus status = tgss_UndefinedError;

	lib_func = dlsym( handle_lib, "get_status" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		status = (*lib_func)();

		set_tgs_device_status(tgsi, status);
	}

	g_free(error);
}

void set_tank_state_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi, guint8  index_tank, LogParams* log_params, gboolean log_trace)
{
	guchar (*get_tank_state_func)(guint32 p_tank_num, gfloat* height, gfloat* volume, gfloat* density, gfloat* weight, gfloat* temperature, gfloat* water_level, gboolean* online);


	guint32 tank_num = get_tgs_tank_num(tgsi, index_tank);

	get_tank_state_func = dlsym( handle_lib, "get_tank_state" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get_tank_state for tank %d error: %s\n", tank_num,  error );

		g_free(error);
	}
	else
	{
		gfloat height = 0;
		gfloat volume = 0;
		gfloat density = 0;
		gfloat weight = 0;
		gfloat temperature = 0;
		gfloat water_level = 0;
		gboolean online = FALSE;

		guchar result = (*get_tank_state_func)(tank_num, &height, &volume, &density, &weight, &temperature, &water_level, &online);

		if (result == tgss_NoError)
		{
			set_tgs_tank_data(tgsi, index_tank, height, volume, weight, density, temperature, water_level, online);
		}
	}
	g_free(error);

}
