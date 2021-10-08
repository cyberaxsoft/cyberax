#include <glib.h>
#include <glib/gstdio.h>
#include <dlfcn.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "sc_device.h"
#include "sc_device_data.h"

void set_sensor_config_from_driver(void* handle_lib, gchar* device_name, guint8 sci, LogParams* log_params, gboolean log_trace)
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting sensor configuration...");

	guchar (*get_sensor_count)(guchar* count);
	guchar (*get_sensor_info)(guint8 index, SensorConf* sensor_conf);

	guint8 count = 0;

	get_sensor_count = dlsym( handle_lib, "get_sensor_count" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
		g_free(error);
	}
	else
	{
		g_free(error);

		guchar device_error = (*get_sensor_count)(&count);

		set_sc_device_last_error(sci, device_error);

		if (device_error!= sce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get sensor count error. Activated default settings.");
		}
		else
		{
			if (count > 0)
			{
				get_sensor_info = dlsym( handle_lib, "get_sensor_info" );
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
						SensorConf sensor_conf = {0x00};

						device_error = (*get_sensor_info)(i, &sensor_conf);

						set_sc_device_last_error(sci, device_error);

						if (device_error!= sce_NoError)
						{
							add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting sensor %d info error.", i );
							count = 0;
						}
						else
						{
							set_sc_sensor_info(sci, i, sensor_conf.num, sensor_conf.addr);
						}
					}
				}
			}
			set_sc_sensor_count(sci, count);
		}
	}

}

void set_sc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 sci)
{
	guchar (*lib_func)();

	ScDeviceStatus status = scs_UndefinedError;

	lib_func = dlsym( handle_lib, "get_status" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		status = (*lib_func)();

		set_sc_device_status(sci, status);
	}

	g_free(error);
}

void set_sensor_state_from_driver(void* handle_lib, gchar* device_name, guint8 sci, guint8  sensor_index, LogParams* log_params, gboolean log_trace)
{
	guchar (*get_sensor_state_func)(guint32 p_sensor_num, SensorParams* params, gboolean* online);


	guint32 sensor_num = get_sc_sensor_num(sci, sensor_index);

	get_sensor_state_func = dlsym( handle_lib, "get_sensor_state" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get_sensor_state for sensor %d error: %s\n", sensor_num,  error );

		g_free(error);
	}
	else
	{
		SensorParams params = {0x00};
		gboolean online = FALSE;

		guchar result = (*get_sensor_state_func)(sensor_num, &params, &online);

//		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Param count: %d\n", params.param_count);
//
//		if (params.param_count > 0)
//		{
//			for (guint8 i = 0; i < params.param_count; i++)
//			{
//				add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Param %d: %02X%02X%02X%02X%02X%02X%02X%02X\n", i, params.params[i].value[0], params.params[i].value[1], params.params[i].value[2],
//						params.params[i].value[3], params.params[i].value[4], params.params[i].value[5], params.params[i].value[6], params.params[i].value[7]);
//			}
//		}

		if (result == scs_NoError)
		{
			set_sc_sensor_data(sci, sensor_index, &params, online);
		}
	}
	g_free(error);

}

