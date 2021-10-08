#include <glib.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "tls.h"
#include "config.h"


guint8			active_tank_index;
guint8			last_sended_tank_index;
guint8			tank_count;
Tank			tanks[MAX_TANK_COUNT];
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

	tank_count = configuration->tank_count;

	if (tank_count > 0)
	{
		for (guint8 i = 0; i < tank_count; i++)
		{
			tanks[i].num = configuration->tanks[i].num;
			tanks[i].channel = configuration->tanks[i].channel;

			tanks[i].height = 0;
			tanks[i].volume = 0;
			tanks[i].density = 0;
			tanks[i].temperature = 0;
			tanks[i].water_level = 0;
			tanks[i].weight = 0;

			tanks[i].online = FALSE;

			tanks[i].sending_error_counter = 0;
			tanks[i].uart_error_counter = 0;

		}
	}

	g_mutex_unlock(&driver_mutex);
}

//------------------------------------------------------acitve_tank_index--------------------------------------------------------------
void safe_set_active_tank_index(guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	active_tank_index = new_value;

	g_mutex_unlock(&driver_mutex);
}

guint8 safe_get_active_tank_index()
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	result = active_tank_index;

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_increment_active_tank_index()
{
	g_mutex_lock(&driver_mutex);

	active_tank_index++;

	if (active_tank_index >= tank_count)
	{
		active_tank_index = 0;
	}

	g_mutex_unlock(&driver_mutex);
}

//------------------------------------------------------last_sended_tank_index--------------------------------------------------------------

void safe_set_last_sended_tank_index(guint8 tank_index)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		last_sended_tank_index = tank_index;
	}

	g_mutex_unlock(&driver_mutex);

}

guint8 safe_get_last_sended_tank_index()
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	result = last_sended_tank_index;

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  tank_count --------------------------------------------------------------------
guint8 safe_get_tank_count()
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	result = tank_count;

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  tank_channel --------------------------------------------------------------------

DriverError safe_get_tank_channel(guint8 tank_index, guint8* tank_channel)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		*tank_channel = tanks[tank_index].channel;

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  online --------------------------------------------------------------------

gboolean safe_get_tank_online(guint8 tank_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		result = tanks[tank_index].online;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_tank_online(guint8 tank_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].online = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  uart_error_counter --------------------------------------------------------------------

guint8 safe_get_uart_error_counter(guint8 tank_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		result = tanks[tank_index].uart_error_counter;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_uart_error_counter(guint8 tank_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].uart_error_counter = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_increment_uart_error_counter(guint8 tank_index)
{

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].uart_error_counter++;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  exchange_state --------------------------------------------------------------------

TankExchangeState safe_get_exchange_state(guint8 tank_index)
{
	TankExchangeState result = tes_Free;

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		result = tanks[tank_index].exchange_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_exchange_state(guint8 tank_index, TankExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].exchange_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  volume --------------------------------------------------------------------
void safe_set_tank_volume(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].volume = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  weight --------------------------------------------------------------------
void safe_set_tank_weight(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].weight = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  density --------------------------------------------------------------------
void safe_set_tank_density(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].density = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  height --------------------------------------------------------------------
void safe_set_tank_height(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].height = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  water --------------------------------------------------------------------
void safe_set_tank_water(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].water_level = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  temperature --------------------------------------------------------------------
void safe_set_tank_temperature(guint8 tank_index, gfloat new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].temperature = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  sending_error_counter --------------------------------------------------------------------

guint8 safe_get_sending_error_counter(guint8 tank_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		result = tanks[tank_index].sending_error_counter;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_sending_error_counter(guint8 tank_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].sending_error_counter = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_increment_sending_error_counter(guint8 tank_index)
{
	g_mutex_lock(&driver_mutex);

	if (tank_index  < tank_count)
	{
		tanks[tank_index].sending_error_counter++;
	}

	g_mutex_unlock(&driver_mutex);
}

DriverError safe_get_tank_data(guint8 tank_index, gfloat* height, gfloat* volume, gfloat* density, gfloat* weight, gfloat* temperature, gfloat* water_level, gboolean* online)
{
	DriverError result = de_OutOfRange;

	g_mutex_lock(&driver_mutex);

	if (tank_index < tank_count)
	{
		*height = tanks[tank_index].height;
		*volume = tanks[tank_index].volume;
		*density = tanks[tank_index].density;
		*weight = tanks[tank_index].weight;
		*temperature = tanks[tank_index].temperature;
		*water_level = tanks[tank_index].water_level;
		*online = tanks[tank_index].online;

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}
