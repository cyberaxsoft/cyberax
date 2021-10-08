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

typedef struct _DecimalPointOptions
{
	guint8			dp_price;
}DecimalPointOptions;

typedef struct _PricePoleConf
{
	guint8				num;
	guint8				grade;
	guint8				symbol_count;
}PricePoleConf;

typedef struct _LibConfig
{
	LogOptions			log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;
	DecimalPointOptions	decimal_point_options;

	guint8				price_pole_count;
	PricePoleConf		price_poles[MAX_PRICE_POLE_COUNT];

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
void safe_get_decimal_point_positions(guint8* price_dp);
guint8 safe_get_price_pole_count();
DriverError safe_get_price_pole_index_by_num(guint32 num, guint8* price_pole_index);
DriverError safe_get_price_pole_index_by_grade(guint8 grade, guint8* price_pole_index);
DriverError safe_get_price_pole_info(guint8 index_price_pole, guint8* num, guint8* grade, guint8* symbol_count);
guint8 safe_get_price_pole_num(guint8 index_price_pole);
guint8 safe_get_price_pole_grade(guint8 index_price_pole);
guint8 safe_get_price_pole_symbol_count(guint8 index_price_pole);
void safe_set_price_pole_symbol_count(guint8 index_price_pole, guint8 new_value);

#endif /* CONFIG_H_ */
