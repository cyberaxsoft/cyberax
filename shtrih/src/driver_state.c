#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "shtrih.h"
#include "shtrih_func.h"

GMutex			driver_mutex;

ExchangeState exchange_state = es_Ready;



void init_driver_mutex()
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}
}

ExchangeState safe_get_exchange_state()
{
	ExchangeState result = es_Ready;

	g_mutex_lock(&driver_mutex);

	result = exchange_state;

	g_mutex_unlock(&driver_mutex);

	return result;
}


void safe_set_exchange_state(ExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	exchange_state = new_value;

	g_mutex_unlock(&driver_mutex);
}



