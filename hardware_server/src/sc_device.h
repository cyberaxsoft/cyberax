#ifndef SC_DEVICE_H_
#define SC_DEVICE_H_

#define	MAX_SC_CONTROL_COUNT	16
#define	SENSOR_PARAM_VALUE_SIZE	8

typedef enum _ScDeviceError
{
	sce_NoError				= 0x00,
	sce_WrongCommand		= 0x01,
	sce_ErrorConnection 	= 0x02,
	sce_ErrorConfiguration	= 0x03,
	sce_ErrorLogging		= 0x04,
	sce_FaultSensorIndex	= 0x05,
	sce_FaultParamIndex  	= 0x06,
	sce_Undefined			= 0x07,
	sce_OutOfRange			= 0x08,
	sce_AlreadyInit			= 0x09,
	sce_NotInitializeDriver	= 0x0A,
	sce_FaultSensorNum		= 0x0B,
	sce_FaultParamNum		= 0x0C,
}ScDeviceError;

typedef enum _ScDeviceStatus
{
	scs_NoError				= 0x00,
	scs_ErrorConnection		= 0x01,
	scs_ErrorConfiguration	= 0x02,
	scs_ErrorLogging		= 0x03,
	scs_UndefinedError		= 0x04,
	scs_NotInitializeDriver	= 0x05,
}ScDeviceStatus;

typedef enum _SensorParamType
{
	spt_Float				= 0x00,
}SensorParamType;

typedef struct _SensorParam
{
	guint8					num;
	SensorParamType			type;
	guint8					value[SENSOR_PARAM_VALUE_SIZE];
}SensorParam;

typedef struct _SensorParams
{
	SensorParam				params[MAX_SENSOR_PARAM_COUNT];
	guint8					param_count;
}SensorParams;


typedef struct _Sensor
{
	guint32					num;
	guint8					addr;

	SensorParams			params;

	gboolean				online;

	gboolean				data_is_changed;

}Sensor;

typedef struct _SensorController
{
	gchar*					name;
	gint32					port;
	gboolean				device_is_working;
	ScDeviceStatus			device_status;
	gboolean				device_status_is_changed;
	ScDeviceError			last_device_error;

	Sensor					sensors[MAX_SENSOR_COUNT];
	guint8					sensor_count;

	LogParams				log_params;
	gboolean				log_trace;

	gint32					sock;
	SocketStatus 			sock_status;
	ThreadStatus			main_sock_status;
}SensorController;


typedef struct _SensorClientInfo
{
	guint32					num;
	gboolean				status_is_change;
}SensorClientInfo;

typedef struct _SCClient
{
	guint8 					sensor_controller_index;
	ClientState				state;
	gboolean				device_status_is_change;
	gboolean				logged;
	guint8					access_level;
	gboolean				read_thread_is_active;
	gint32					sock;

	SensorClientInfo		sensor_info[MAX_SENSOR_COUNT];
	gint8					sensor_info_count;
}SCClient;

typedef struct _SCClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	guint8					sensor_controller_index;
	gchar* 					device_name;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_enable;
	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}SCClientInfo;


void set_sensor_controller_device_is_working(guint8 index, gboolean value);
gpointer sensor_controller_device_thread_func(gpointer data);

#endif
