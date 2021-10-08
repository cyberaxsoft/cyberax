#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct _ConnOptions
{
	ConnectionType	connection_type;
	gchar*			port;
	gchar*			ip_address;
	guint16			ip_port;
	guint32			uart_baudrate;
	guint8			uart_byte_size;
	gchar*			uart_parity;
	guint8			uart_stop_bits;
}ConnOptions;

typedef struct _TimeoutOptions
{
	guint32			t_read;
	guint32			t_write;
}TimeoutOptions;

typedef struct _LibConfig
{
	LogOptions			log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;

	guint8				protocol_type;
	gboolean			auto_drawer;
	gboolean			auto_cutting;
	guint8				cash_num;
	guint8				bn_num;
	gboolean			time_sync;

}LibConfig;



void init_conf_mutex();
void safe_set_configuration(LibConfig new_configuration);
void safe_get_log_options(LogOptions* log_options);
void safe_set_driver_settings();
ConnectionType safe_get_connection_type();
gchar* safe_get_port();
gchar* safe_get_ip_address();
guint16 safe_get_ip_port();
guint32 safe_get_uart_baudrate();
guint8 safe_get_uart_byte_size();
gchar* safe_get_uart_parity();
guint8 safe_get_uart_stop_bits();
guint32 safe_get_timeout_read();
guint32 safe_get_timeout_write();
guint8 safe_get_protocol_type();
gboolean safe_get_auto_drawer();
gboolean safe_get_auto_cutting();
guint8 safe_get_cash_num();
guint8 safe_get_bn_num();
gboolean safe_get_time_sync();

#endif /* CONFIG_H_ */
