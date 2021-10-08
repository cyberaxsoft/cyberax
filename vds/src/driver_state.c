#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "vds.h"
#include "config.h"

guint8			current_price_pole_index;
guint8			price_pole_count;
VdsPricePole	price_pole[MAX_PRICE_POLE_COUNT];

GMutex			driver_mutex;

void init_driver_mutex()
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}
}

//-------------------------------------------------   setters and getters -----------------------------------------------------------

void safe_set_driver_state_from_settings(LibConfig* configuration)
{
	g_mutex_lock(&driver_mutex);

	price_pole_count = configuration->price_pole_count;

	if (price_pole_count > 0)
	{
		for (guint8 i = 0; i < price_pole_count; i++)
		{
			PricePoleConf* price_pole_conf = &configuration->price_poles[i];

			price_pole[i].grade = price_pole_conf->grade;
			price_pole[i].price = 0;//(i+1) * 100;
			price_pole[i].state = pps_NotInitialize;
			price_pole[i].exchange_state = ves_GetPrice;
			price_pole[i].uart_error = 0;
			price_pole[i].command_sent = FALSE;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   current_price_pole_index -----------------------------------------------------------

guint8 safe_get_current_price_pole_index()
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	result = current_price_pole_index;

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_current_price_pole_index(guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	current_price_pole_index = new_value;

	g_mutex_unlock(&driver_mutex);
}

void safe_inc_current_price_pole_index()
{
	g_mutex_lock(&driver_mutex);

	current_price_pole_index++;

	if (current_price_pole_index >= price_pole_count)
	{
		current_price_pole_index = 0;
	}

	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   price  -----------------------------------------------------------

guint32 safe_get_price(guint8 price_pole_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		result = price_pole[price_pole_index].price;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_price(guint8 price_pole_index, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].price = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_full_price(guint8 grade, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_count > 0)
	{
		for (guint8 i = 0; i < price_pole_count; i++)
		{
			if (price_pole[i].grade == grade)
			{
				price_pole[i].price = new_value;
			}
		}
	}
	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   state  -----------------------------------------------------------

PricePoleState safe_get_state(guint8 price_pole_index)
{
	PricePoleState result = pps_NotInitialize;

	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		result = price_pole[price_pole_index].state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_state(guint8 price_pole_index, PricePoleState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].state = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   exchange_state  -----------------------------------------------------------

VdsExchangeState safe_get_exchange_state(guint8 price_pole_index)
{
	VdsExchangeState result = ves_GetPrice;

	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		result = price_pole[price_pole_index].exchange_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_exchange_state(guint8 price_pole_index, VdsExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].exchange_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   uart_error  -----------------------------------------------------------

guint8 safe_get_uart_error(guint8 price_pole_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		result = price_pole[price_pole_index].uart_error;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_uart_error(guint8 price_pole_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].uart_error = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_inc_uart_error(guint8 price_pole_index)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].uart_error++;
	}

	g_mutex_unlock(&driver_mutex);
}

//-------------------------------------------------   command_sent  -----------------------------------------------------------

gboolean safe_get_command_sent(guint8 price_pole_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		result = price_pole[price_pole_index].command_sent;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_command_sent(guint8 price_pole_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (price_pole_index < price_pole_count)
	{
		price_pole[price_pole_index].command_sent = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}
