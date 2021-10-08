#include <glib.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "config.h"
#include "shtrih.h"
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

//	safe_set_driver_state_from_settings(&configuration);

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

guint8 safe_get_protocol_type()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.protocol_type;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------auto_drawer--------------------------------------------------------------
gboolean safe_get_auto_drawer()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.auto_drawer;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------auto_cutting--------------------------------------------------------------

gboolean safe_get_auto_cutting()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.auto_cutting;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------cash_num--------------------------------------------------------------

guint8 safe_get_cash_num()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.cash_num;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------bn_num--------------------------------------------------------------

guint8 safe_get_bn_num()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.bn_num;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------time_sync--------------------------------------------------------------

gboolean safe_get_time_sync()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.time_sync;

	g_mutex_unlock(&conf_mutex);

	return result;
}

