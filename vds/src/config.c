#include <glib.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "vds.h"
#include "config.h"
#include "driver_state.h"

LibConfig configuration;
GMutex	conf_mutex;

void init_conf_mutex()
{
	if (conf_mutex.p == NULL)
	{
		g_mutex_init(&conf_mutex);
	}
}

void safe_set_configuration(LibConfig new_configuration)
{
	g_mutex_lock(&conf_mutex);

	configuration = new_configuration;

	g_mutex_unlock(&conf_mutex);
}

void safe_get_log_options(LogOptions* log_options)
{
	g_mutex_lock(&conf_mutex);

	*log_options = configuration.log_options;

	g_mutex_unlock(&conf_mutex);

}

void safe_set_driver_settings()
{
	g_mutex_lock(&conf_mutex);

	safe_set_driver_state_from_settings(&configuration);

	g_mutex_unlock(&conf_mutex);

}

//connection type

ConnectionType safe_get_connection_type()
{
	ConnectionType result = ct_Uart;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.connection_type;

	g_mutex_unlock(&conf_mutex);

	return result;
}

gchar* safe_get_port()
{
	gchar* result = NULL;

	g_mutex_lock(&conf_mutex);

	result = g_strdup(configuration.conn_options.port);

	g_mutex_unlock(&conf_mutex);

	return result;
}

gchar* safe_get_ip_address()
{
	gchar* result = NULL;

	g_mutex_lock(&conf_mutex);

	result = g_strdup(configuration.conn_options.ip_address);

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint16 safe_get_ip_port()
{
	guint16 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.ip_port;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_uart_baudrate()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.uart_baudrate;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint8 safe_get_uart_byte_size()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.uart_byte_size;

	g_mutex_unlock(&conf_mutex);

	return result;
}

gchar* safe_get_uart_parity()
{
	gchar* result = NULL;

	g_mutex_lock(&conf_mutex);

	result = g_strdup(configuration.conn_options.uart_parity);

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint8 safe_get_uart_stop_bits()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.uart_stop_bits;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_timeout_read()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.timeout_options.t_read;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_timeout_write()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.timeout_options.t_write;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------decimal_point_positions--------------------------------------------------------------

void safe_get_decimal_point_positions(guint8* price_dp)
{
	g_mutex_lock(&conf_mutex);

	*price_dp = configuration.decimal_point_options.dp_price;

	g_mutex_unlock(&conf_mutex);
}

//--------------------------------------------------------  disp_count --------------------------------------------------------------------
guint8 safe_get_price_pole_count()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.price_pole_count;

	g_mutex_unlock(&conf_mutex);

	return result;
}

DriverError safe_get_price_pole_index_by_num(guint32 num, guint8* price_pole_index)
{
	DriverError result = de_OutOfRange;

	g_mutex_lock(&conf_mutex);

	if (configuration.price_pole_count > 0)
	{
		for (guint8 i = 0; i < configuration.price_pole_count; i++)
		{
			if (configuration.price_poles[i].num == num)
			{
				*price_pole_index = i;
				result = de_NoError;
				break;
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

DriverError safe_get_price_pole_index_by_grade(guint8 grade, guint8* price_pole_index)
{
	DriverError result = de_OutOfRange;

	g_mutex_lock(&conf_mutex);

	if (configuration.price_pole_count > 0)
	{
		for (guint8 i = 0; i < configuration.price_pole_count; i++)
		{
			if (configuration.price_poles[i].grade == grade)
			{
				*price_pole_index = i;
				result = de_NoError;
				break;
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}

DriverError safe_get_price_pole_info(guint8 index_price_pole, guint8* num, guint8* grade, guint8* symbol_count)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (index_price_pole < configuration.price_pole_count)
	{
		*num = configuration.price_poles[index_price_pole].num;
		*grade = configuration.price_poles[index_price_pole].grade;
		*symbol_count = configuration.price_poles[index_price_pole].symbol_count;

		result = de_NoError;
	}
	else
	{
		result = de_OutOfRange;

	}
	g_mutex_unlock(&conf_mutex);

	return result;

}

guint8 safe_get_price_pole_num(guint8 index_price_pole)
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	if (index_price_pole < configuration.price_pole_count)
	{
		result = configuration.price_poles[index_price_pole].num;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint8 safe_get_price_pole_grade(guint8 index_price_pole)
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	if (index_price_pole < configuration.price_pole_count)
	{
		result = configuration.price_poles[index_price_pole].grade;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}


guint8 safe_get_price_pole_symbol_count(guint8 index_price_pole)
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	if (index_price_pole < configuration.price_pole_count)
	{
		result = configuration.price_poles[index_price_pole].symbol_count;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_price_pole_symbol_count(guint8 index_price_pole, guint8 new_value)
{
	g_mutex_lock(&conf_mutex);

	if (index_price_pole < configuration.price_pole_count)
	{
		configuration.price_poles[index_price_pole].symbol_count = new_value;
	}

	g_mutex_unlock(&conf_mutex);
}
