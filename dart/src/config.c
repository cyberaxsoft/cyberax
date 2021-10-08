#include <glib.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "dart.h"
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

guint32 safe_get_full_tank_volume()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.full_tank_volume;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------counters_enable--------------------------------------------------------------
gboolean safe_get_counters_enable()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.counters_enable;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------auto_start--------------------------------------------------------------

gboolean safe_get_auto_start()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.auto_start;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------decimal_point_positions--------------------------------------------------------------

void safe_get_decimal_point_positions(guint8* price_dp, guint8* volume_dp, guint8* amount_dp )
{
	g_mutex_lock(&conf_mutex);

	*price_dp = configuration.decimal_point_options.dp_price;
	*volume_dp = configuration.decimal_point_options.dp_volume;
	*amount_dp = configuration.decimal_point_options.dp_amount;

	g_mutex_unlock(&conf_mutex);
}

//--------------------------------------------------------  disp_count --------------------------------------------------------------------
guint8 safe_get_disp_count()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.dispencer_count;

	g_mutex_unlock(&conf_mutex);

	return result;
}


gint8 safe_get_nozzle_index_by_grade(guint8 disp_index, guint8 nozzle_grade)
{
	gint8 result = -1;

	g_mutex_lock(&conf_mutex);

	if (disp_index  < configuration.dispencer_count)
	{
		if (configuration.dispencers[disp_index].nozzle_count > 0)
		{
			for (guint8 i = 0; i < configuration.dispencers[disp_index].nozzle_count; i++)
			{
				if (configuration.dispencers[disp_index].nozzles[i].grade == nozzle_grade)
				{
					result = i;
					break;
				}
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}


DriverError safe_get_disp_index_by_num(guint32 disp_num, guint8* disp_index)
{
	DriverError result = de_OutOfRange;

	g_mutex_lock(&conf_mutex);

	if (configuration.dispencer_count > 0)
	{
		for (guint8 i = 0; i < configuration.dispencer_count; i++)
		{
			if (configuration.dispencers[i].num == disp_num)
			{
				*disp_index = i;
				result = de_NoError;
				break;
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}
DriverError safe_get_nozzle_index_by_num(guint8 disp_index, guint8 nozzle_num, guint8* nozzle_index)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (disp_index  < configuration.dispencer_count)
	{
		if (configuration.dispencers[disp_index].nozzle_count > 0)
		{
			for (guint8 i = 0; i < configuration.dispencers[disp_index].nozzle_count; i++ )
			{
				if (configuration.dispencers[disp_index].nozzles[i].num == nozzle_num)
				{
					*nozzle_index = i;
					result = de_NoError;
					break;
				}
			}
		}
		else
		{
			result = de_FaultNozzleNum;
		}
	}
	else
	{
		result = de_FaultDispencerIndex;
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}


DriverError safe_get_disp_index_by_addr(guint8 disp_addr, guint8* disp_index)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (configuration.dispencer_count > 0)
	{
		for (guint8 i = 0; i < configuration.dispencer_count; i++)
		{
			if (configuration.dispencers[i].addr == disp_addr)
			{
				*disp_index = i;
				result = de_NoError;
				break;
			}
		}

	}
	else
	{
		result = de_OutOfRange;
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}

DriverError safe_get_disp_info(guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (index_disp < configuration.dispencer_count)
	{
		*num = configuration.dispencers[index_disp].num;
		*addr = configuration.dispencers[index_disp].addr;
		*nozzle_count = configuration.dispencers[index_disp].nozzle_count;

		result = de_NoError;
	}
	else
	{
		result = de_OutOfRange;

	}
	g_mutex_unlock(&conf_mutex);

	return result;

}

DriverError safe_get_nozzle_info(guint8 index_disp, guint8 index_nozzle,  guint8* num, guint8* grade)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (index_disp < configuration.dispencer_count)
	{
		if (index_nozzle < configuration.dispencers[index_disp].nozzle_count)
		{
			*num = configuration.dispencers[index_disp].nozzles[index_nozzle].num;
			*grade = configuration.dispencers[index_disp].nozzles[index_nozzle].grade;

			result = de_NoError;
		}
		else
		{
			result = de_OutOfRange;
		}
	}
	else
	{
		result = de_OutOfRange;
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}
