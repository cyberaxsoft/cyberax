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
	guint8			dp_volume;
	guint8			dp_amount;
}DecimalPointOptions;

typedef struct _NozzleConf
{
	guint8			num;
	guint8			grade;
}NozzleConf;

typedef struct _DispencerConf
{
	guint32				num;
	guint8				addr;
	guint8				nozzle_count;
	NozzleConf				nozzles[MAX_NOZZLE_COUNT];
}DispencerConf;

typedef struct _LibConfig
{
	LogOptions			log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;
	DecimalPointOptions	decimal_point_options;

	gboolean			counters_enable;
	gboolean			auto_start;
	gboolean			auto_payment;
	guint32				full_tank_volume;

	guint8				dispencer_count;
	DispencerConf		dispencers[MAX_DISP_COUNT];

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
guint32 safe_get_full_tank_volume();
gboolean safe_get_counters_enable();
gboolean safe_get_auto_start();
guint8 safe_get_disp_count();
void safe_get_decimal_point_positions(guint8* price_dp, guint8* volume_dp, guint8* amount_dp );
gint8 safe_get_nozzle_index_by_grade(guint8 disp_index, guint8 nozzle_grade);
DriverError safe_get_disp_index_by_num(guint32 disp_num, guint8* disp_index);
DriverError safe_get_nozzle_index_by_num(guint8 disp_index, guint8 nozzle_num, guint8* nozzle_index);
DriverError safe_get_disp_index_by_addr(guint8 disp_addr, guint8* disp_index);
DriverError safe_get_disp_info(guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count);
DriverError safe_get_nozzle_info(guint8 index_disp, guint8 index_nozzle,  guint8* num, guint8* grade);


#endif /* CONFIG_H_ */
