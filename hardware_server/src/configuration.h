
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define MAX_NOZZLE_COUNT			8
#define MAX_DISP_COUNT				32
#define MAX_TANK_COUNT				16
#define MAX_PRICE_POLE_COUNT		16
#define MAX_CLIENT_COUNT			255
#define MAX_DEVICE_COUNT			255
#define	MAX_SENSOR_PARAM_COUNT		16
#define	MAX_SENSOR_COUNT			255


typedef enum _ConnectionType
{
	ct_Uart					= 0x00,
	ct_TcpIp				= 0x01,
}ConnectionType;

typedef enum _ThreadStatus
{
	ts_Undefined			= 0x00,
	ts_Active				= 0x01,
	ts_Finished				= 0x02,
}ThreadStatus;

//-------------------------------------------- profiles -----------------------------------------------------

typedef struct _Profile
{
	guint8			id;
	gchar*			name;
	gboolean		enable;
	gchar* 			guid;
	guint8			access_level;

	gchar*			ip_address;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;
	gboolean		log_frames;
	gboolean		log_parsing;
}Profile;

typedef struct _ProfilesConf
{
	Profile			profiles[MAX_CLIENT_COUNT];
	guint8 			profiles_count;
	gboolean 		enable;
}ProfilesConf;

//-------------------------------------------- library ---------------------------------------------------

typedef struct LibLogOptions
{
	gboolean		enable;
	gchar*			dir;

	guint32			file_size;
	guint32			save_days;

	gboolean 		trace;
	gboolean 		system;
	gboolean 		requests;
	gboolean 		frames;
	gboolean 		parsing;

}LibLogOptions;

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

//-------------------------------------------- dispencer controllers ---------------------------------------------------

typedef struct _DCDecimalPointOptions
{
	guint8			dp_price;
	guint8			dp_volume;
	guint8			dp_amount;
}DCDecimalPointOptions;


typedef struct _NozzleConf
{
	guint8			num;
	guint8			grade;
}NozzleConf;

typedef struct _DispencerConf
{
	guint32					num;
	guint8					addr;
	guint8					nozzle_count;
	NozzleConf				nozzles[MAX_NOZZLE_COUNT];
}DispencerConf;

typedef struct _DCLibConfig
{
	LibLogOptions			log_options;
	ConnOptions				conn_options;
	TimeoutOptions 			timeout_options;
	DCDecimalPointOptions	decimal_point_options;

	gboolean				counters_enable;
	gboolean				auto_start;
	gboolean				auto_payment;
	guint32					full_tank_volume;

	guint8					dispencer_count;
	DispencerConf			dispencers[MAX_DISP_COUNT];

}DCLibConfig;

typedef struct _DispencerControllerConf
{
	guint8			index;
	gchar			id;
	gchar* 			name;
	guint32 		port;
	gboolean		enable;
	guint32 		command_timeout;
	guint32		 	interval;
	gchar*			module_name;
	DCLibConfig		module_config;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	GThread* 		device_thread;
	ThreadStatus	thread_status;
	guint8			dispencer_controller_index;

	gboolean 		changed;

}DispencerControllerConf;

//-------------------------------------------- tgs ---------------------------------------------------

typedef struct _TankConf
{
	guint32				num;
	guint8				channel;
}TankConf;

typedef struct _TGSLibConfig
{
	LibLogOptions		log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;

	guint8				tank_count;
	TankConf			tanks[MAX_TANK_COUNT];

}TGSLibConfig;

typedef struct _TgsConf
{
	guint8			index;
	gchar			id;
	gchar* 			name;
	guint32 		port;
	gboolean		enable;
	guint32 		command_timeout;
	guint32		 	interval;
	gchar*			module_name;
	TGSLibConfig	module_config;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	GThread* 		device_thread;
	ThreadStatus	thread_status;
	guint8			tgs_index;

	gboolean 		changed;

}TgsConf;

//-------------------------------------------- fiscal registers ---------------------------------------------------

typedef struct _FRLibConfig
{
	LibLogOptions			log_options;
	ConnOptions				conn_options;
	TimeoutOptions 			timeout_options;

	guint8					protocol_type;
	gboolean				auto_drawer;
	gboolean				auto_cutting;
	guint8					cash_num;
	guint8					bn_num;
	guint8					bonus_num;
	gboolean				time_sync;

}FRLibConfig;

typedef struct _FiscalRegisterConf
{
	guint8			index;
	gchar			id;
	gchar* 			name;
	guint32 		port;
	gboolean		enable;
	guint32 		command_timeout;
	guint32		 	interval;
	gchar*			module_name;
	FRLibConfig		module_config;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	GThread* 		device_thread;
	ThreadStatus	thread_status;
	guint8			fiscal_register_index;

	gboolean 		changed;

}FiscalRegisterConf;


//-------------------------------------------- price pole controllers ---------------------------------------------------


typedef struct _PPCDecimalPointOptions
{
	guint8			dp_price;
}PPCDecimalPointOptions;


typedef struct _PricePoleConf
{
	guint8				num;
	guint8				grade;
	guint8				symbol_count;
}PricePoleConf;

typedef struct _PPCLibConfig
{
	LibLogOptions			log_options;
	ConnOptions				conn_options;
	TimeoutOptions 			timeout_options;
	PPCDecimalPointOptions	decimal_point_options;
	guint8					price_pole_count;
	PricePoleConf			price_poles[MAX_PRICE_POLE_COUNT];

}PPCLibConfig;

typedef struct _PpcConf
{
	guint8			index;
	gchar			id;
	gchar* 			name;
	guint32 		port;
	gboolean		enable;
	guint32 		command_timeout;
	guint32		 	interval;
	gchar*			module_name;
	PPCLibConfig	module_config;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	GThread* 		device_thread;
	ThreadStatus	thread_status;
	guint8			ppc_index;

	gboolean 		changed;

}PpcConf;

//-------------------------------------------- sensor_controller ---------------------------------------------------

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

typedef struct _SCLibConfig
{
	LibLogOptions		log_options;
	ConnOptions			conn_options;
	TimeoutOptions 		timeout_options;

	guint8				sensor_count;
	SensorConf			sensors[MAX_SENSOR_COUNT];

}SCLibConfig;

typedef struct _ScConf
{
	guint8			index;
	gchar			id;
	gchar* 			name;
	guint32 		port;
	gboolean		enable;
	guint32 		command_timeout;
	guint32		 	interval;
	gchar*			module_name;
	SCLibConfig		module_config;
	gchar*			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	GThread* 		device_thread;
	ThreadStatus	thread_status;
	guint8			sc_index;

	gboolean 		changed;

}ScConf;


// ----------------------------------------------------- devices -----------------------------------------------

typedef struct _DeviceConfs
{
	DispencerControllerConf		dispencer_controllers[MAX_DEVICE_COUNT];
	guint8 						dispencer_controller_count;

	TgsConf						tgs[MAX_DEVICE_COUNT];
	guint8 						tgs_count;

	FiscalRegisterConf			fiscal_registers[MAX_DEVICE_COUNT];
	guint8 						fiscal_register_count;

	PpcConf						price_pole_controllers[MAX_DEVICE_COUNT];
	guint8 						price_pole_controller_count;

	ScConf						sensor_controllers[MAX_DEVICE_COUNT];
	guint8 						sensor_controller_count;

}DeviceConfs;

// ----------------------------------------------------- common -----------------------------------------------

typedef struct _CommonConf
{
	gchar* 			server_name;
	guint32 		port;
	gchar* 			log_dir;
	gboolean		log_enable;
	gboolean		log_trace;

	guint32			file_size;
	guint32			save_days;

	gchar* 			conn_log_dir;
	gboolean		conn_log_enable;
	gboolean		conn_log_trace;
	gboolean		conn_log_frames;
	gboolean		conn_log_parsing;

	guint32			conn_log_file_size;
	guint32			conn_log_save_days;

	gboolean		changed;
}CommonConf;

// ----------------------------------------------------- server config -----------------------------------------------

typedef struct _ServConfig
{
	CommonConf		common_conf;
	ProfilesConf	profiles_conf;
	DeviceConfs		devices;

}ServConfig;

//--------------------------------------------------------- functions ------------------------------------------------

void configuration_initialization(gchar* filename_conf);
gboolean read_conf();
void write_conf();
void set_reset();

void get_common_conf( CommonConf* result);
gboolean get_common_conf_changed();
void set_common_conf_changed(gboolean new_value);
void update_common_conf(CommonConf src);

void get_connection_log_conf(gchar** log_dir, gboolean* log_enable, gboolean* log_trace, gboolean* log_frames, gboolean* log_parsing);

void get_device_confs(DeviceConfs* result);
void set_device_changed();

void get_profiles_conf( ProfilesConf* result);
gboolean add_client_profile(Profile profile);
gboolean update_client_profile(Profile profile);
gboolean delete_client_profile(Profile client_conf);

void set_profiles_enable(gboolean new_value);
void get_profiles_enable(gboolean* result);

guint8 get_dc_count();
gboolean get_dc_conf_changed(guint8 index);
void set_dc_conf_changed(guint8 index, gboolean new_value);
void get_dc_conf( guint8 index, DispencerControllerConf* result);
gboolean add_dc_conf(DispencerControllerConf dc_conf);
gboolean update_dc_conf(DispencerControllerConf dc_conf);
gboolean delete_dc_conf(DispencerControllerConf dc_conf);
void get_dispencer_controller_conf(guint8 index, DispencerControllerConf* result);
ThreadStatus get_dc_thread_status(guchar device_index);
void set_dc_thread_status(guchar device_id, ThreadStatus new_status);
void set_dispencer_controller_index(guint8 device_index, guint8 dispencer_controller_index);
guint8 get_dispencer_controller_index(guint8 device_index);
void join_dc_thread(guint8 index);
gboolean start_dc_thread(guint8 index, guint8 device_index);

guint8 get_tgs_count();
gboolean get_tgs_conf_changed(guint8 index);
void set_tgs_conf_changed(guint8 index, gboolean new_value);
void get_tgs_conf( guint8 index, TgsConf* result);
gboolean add_tgs_conf(TgsConf tgs_conf);
gboolean update_tgs_conf(TgsConf tgs_conf);
gboolean delete_tgs_conf(TgsConf tgs_conf);
void get_tank_gauge_system_conf(guint8 index, TgsConf* result);
ThreadStatus get_tgs_thread_status(guchar device_index);
void set_tgs_thread_status(guchar device_id, ThreadStatus new_status);
void set_tgs_index(guint8 device_index, guint8 tgs_index);
guint8 get_tgs_index(guint8 device_index);
void join_tgs_thread(guint8 index);
gboolean start_tgs_thread(guint8 index, guint8 device_index);

guint8 get_fr_count();
gboolean get_fr_conf_changed(guint8 index);
void set_fr_conf_changed(guint8 index, gboolean new_value);
void get_fr_conf( guint8 index, FiscalRegisterConf* result);
gboolean add_fr_conf(FiscalRegisterConf fr_conf);
gboolean update_fr_conf(FiscalRegisterConf fr_conf);
gboolean delete_fr_conf(FiscalRegisterConf fr_conf);
void get_fiscal_register_conf(guint8 index, FiscalRegisterConf* result);
ThreadStatus get_fr_thread_status(guchar device_index);
void set_fr_thread_status(guchar device_id, ThreadStatus new_status);
void set_fiscal_register_index(guint8 device_index, guint8 fiscal_register_index);
guint8 get_fiscal_register_index(guint8 device_index);
void join_fr_thread(guint8 index);
gboolean start_fr_thread(guint8 index, guint8 device_index);


guint8 get_ppc_count();
gboolean get_ppc_conf_changed(guint8 index);
void set_ppc_conf_changed(guint8 index, gboolean new_value);
void get_ppc_conf( guint8 index, PpcConf* result);
gboolean add_ppc_conf(PpcConf tgs_conf);
gboolean update_ppc_conf(PpcConf tgs_conf);
gboolean delete_ppc_conf(PpcConf tgs_conf);
void get_price_pole_controller_conf(guint8 index, PpcConf* result);
ThreadStatus get_ppc_thread_status(guchar device_index);
void set_ppc_thread_status(guchar device_id, ThreadStatus new_status);
void set_ppc_index(guint8 device_index, guint8 ppc_index);
guint8 get_ppc_index(guint8 device_index);
void join_ppc_thread(guint8 index);
gboolean start_ppc_thread(guint8 index, guint8 device_index);

guint8 get_sc_count();
gboolean get_sc_conf_changed(guint8 index);
void set_sc_conf_changed(guint8 index, gboolean new_value);
void get_sc_conf( guint8 index, ScConf* result);
gboolean add_sc_conf(ScConf tgs_conf);
gboolean update_sc_conf(ScConf tgs_conf);
gboolean delete_sc_conf(ScConf tgs_conf);
void get_sensor_controller_conf(guint8 index, ScConf* result);
ThreadStatus get_sc_thread_status(guchar device_index);
void set_sc_thread_status(guchar device_id, ThreadStatus new_status);
void set_sc_index(guint8 device_index, guint8 tgs_index);
guint8 get_sc_index(guint8 device_index);
void join_sc_thread(guint8 index);
gboolean start_sc_thread(guint8 index, guint8 device_index);



#endif /* CONFIGURATION_H_ */
