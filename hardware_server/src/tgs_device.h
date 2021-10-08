#ifndef TGS_DEVICE_H_
#define TGS_DEVICE_H_

#define	MAX_TGS_CONTROL_COUNT	16

typedef enum _TgsDeviceError
{
	tgse_NoError			= 0x00,
	tgse_WrongCommand		= 0x01,
	tgse_ErrorConnection 	= 0x02,
	tgse_ErrorConfiguration = 0x03,
	tgse_ErrorLogging		= 0x04,
	tgse_FaultDispencerIndex= 0x05,
	tgse_FaultNozzleIndex  	= 0x06,
	tgse_Undefined			= 0x07,
	tgse_OutOfRange			= 0x08,
	tgse_AlreadyInit		= 0x09,
	tgse_FaultExtFuncIndex	= 0x0A,
	tgse_NotInitializeDriver= 0x0B,
	tgse_FaultDispencerNum	= 0x0C,
	tgse_FaultNozzleNum		= 0x0D,
	tgse_FaultDispencerState= 0x0E,
	tgse_FaultNozzleState	= 0x0F,
	tgse_DispencerBusy		= 0x10,

}TgsDeviceError;

typedef enum _TgsDeviceStatus
{
	tgss_NoError			= 0x00,
	tgss_ErrorConnection	= 0x01,
	tgss_ErrorConfiguration	= 0x02,
	tgss_ErrorLogging		= 0x03,
	tgss_UndefinedError		= 0x04,
	tgss_NotInitializeDriver= 0x05,
}TgsDeviceStatus;

typedef struct _Tank
{
	guint32					num;
	guint8					channel;

	gfloat					height;
	gfloat					volume;
	gfloat					weight;
	gfloat					density;
	gfloat					temperature;
	gfloat					water_level;

	gboolean				online;

	gboolean				data_is_changed;

}Tank;

typedef struct _Tgs
{
	gchar*					name;
	gint32					port;
	gboolean				device_is_working;
	TgsDeviceStatus			device_status;
	gboolean				device_status_is_changed;
	TgsDeviceError			last_device_error;

	Tank					tanks[MAX_TANK_COUNT];
	guint8					tank_count;

	LogParams				log_params;
	gboolean				log_trace;

	gint32					sock;
	SocketStatus 			sock_status;
	ThreadStatus			main_sock_status;

}Tgs;


typedef struct _TankClientInfo
{
	guint32					num;
	gboolean				status_is_change;
}TankClientInfo;

typedef struct _TGSClient
{
	guint8 					tgs_index;
	ClientState				state;
	gboolean				device_status_is_change;
	gboolean				logged;
	guint8					access_level;
	gboolean				read_thread_is_active;
	gint32					sock;

	TankClientInfo			tank_info[MAX_TANK_COUNT];
	gint8					tank_info_count;
}TGSClient;

typedef struct _TGSClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	guint8					tgs_index;
	gchar* 					device_name;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_enable;
	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}TGSClientInfo;


void set_tgs_device_is_working(guint8 index, gboolean value);
gpointer tgs_device_thread_func(gpointer data);

#endif /* TGS_DEVICE_H_ */
