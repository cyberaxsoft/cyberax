#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct _ConnOptions
{
	ConnectionType		connection_type;
	gchar*				port;
	gchar*				ip_address;
	guint16				ip_port;
	guint32				uart_baudrate;
	guint8				uart_byte_size;
	gchar*				uart_parity;
	guint8				uart_stop_bits;
}ConnOptions;

typedef struct _TimeoutOptions
{
	guint32				t_read;
	guint32				t_write;
}TimeoutOptions;

typedef struct _SensorParamConf
{
	guint8				num;
	guint8				type;
}SensorParamConf;

typedef struct _SensorConf
{
	guint32				num;
	guint8				addr;

	guint8				param_count;
	SensorParamConf		params[MAX_SENSOR_PARAM_COUNT];
}SensorConf;

typedef struct _LibConfig
{
	LogOptions		log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;

	guint8				sensor_count;
	SensorConf			sensors[MAX_SENSOR_COUNT];

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

DriverError safe_get_sensor_index_by_num(guint32 sensor_num, guint8* sensor_index);
DriverError safe_get_sensor_index_by_addr(guint8 sensor_addr, guint8* sensor_index);
DriverError safe_get_sensor_info( guint8 index_sensor, SensorConf* sensor_conf);

DriverError  safe_get_param_index_by_num(guint8 sensor_index, guint8 param_num, guint8* param_index);
#endif /* CONFIG_H_ */
