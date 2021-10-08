#include <glib.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "idcsensor.h"
#include "config.h"
#include "driver_state.h"



guint8			sensor_count;
Sensor			sensors[MAX_SENSOR_COUNT];

gboolean		command_sended;

GMutex			driver_mutex;

void init_driver_mutex()
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}
}

void safe_set_start_driver_state(LibConfig* configuration)
{
	g_mutex_lock(&driver_mutex);

	sensor_count = configuration->sensor_count;

	if (sensor_count > 0)
	{
		for (guint8 i = 0; i < sensor_count; i++)
		{
			sensors[i].num = configuration->sensors[i].num;
			sensors[i].addr = configuration->sensors[i].addr;

			memset(&sensors[i].params, 0x00, sizeof(SensorParams));

			sensors[i].params.param_count = configuration->sensors[i].param_count;

			if (configuration->sensors[i].param_count > 0)
			{
				for (guint8 j = 0; j < configuration->sensors[i].param_count; j++)
				{
					sensors[i].params.params[j].num = configuration->sensors[i].params[j].num;
					sensors[i].params.params[j].type = configuration->sensors[i].params[j].type;

				}
			}

			sensors[i].online = FALSE;

		}
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  sensor_count --------------------------------------------------------------------
guint8 safe_get_sensor_count()
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	result = sensor_count;

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  sensor_addr --------------------------------------------------------------------

DriverError safe_get_sensor_addr(guint8 sensor_index, guint8* sensor_addr)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (sensor_index  < sensor_count)
	{
		*sensor_addr = sensors[sensor_index].addr;

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  command_sended --------------------------------------------------------------------

gboolean safe_get_command_sended()
{
	gboolean result = 0;

	g_mutex_lock(&driver_mutex);

	result = command_sended;

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_command_sended(gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	command_sended = new_value;

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  online --------------------------------------------------------------------

gboolean safe_get_sensor_online(guint8 sensor_index)
{
	gboolean result = 0;

	g_mutex_lock(&driver_mutex);

	if (sensor_index  < sensor_count)
	{
		result = sensors[sensor_index].online;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_sensor_online(guint8 sensor_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (sensor_index  < sensor_count)
	{
		sensors[sensor_index].online = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_all_sensor_online(gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (sensor_count > 0)
	{
		for (guint8 i = 0; i < sensor_count; i++)
		{
			sensors[i].online = new_value;

		}
	}

	g_mutex_unlock(&driver_mutex);

}

void safe_set_sensor_param(guint8 sensor_index, guint8 param_index, guint8* buffer)
{
	g_mutex_lock(&driver_mutex);

	if (sensor_index  < sensor_count && param_index < sensors[sensor_index].params.param_count)
	{
		memcpy(&sensors[sensor_index].params.params[param_index].value[4], buffer, sizeof(gfloat));
	}

	g_mutex_unlock(&driver_mutex);

}

DriverError safe_get_sensor_data(guint8 sensor_index, SensorParams* params, gboolean* online)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (sensor_index  < sensor_count)
	{
		memcpy(params, &sensors[sensor_index].params, sizeof(SensorParams));

		*online = sensors[sensor_index].online;

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}





