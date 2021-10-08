#ifndef PPC_DEVICE_H_
#define PPC_DEVICE_H_

#define DEF_PPC_PRICE_DP			2

typedef enum _PpcDeviceError
{
	ppce_NoError				= 0x00,
	ppce_WrongCommand			= 0x01,
	ppce_ErrorConnection 		= 0x02,
	ppce_ErrorConfiguration 	= 0x03,
	ppce_ErrorLogging			= 0x04,
	ppce_Undefined				= 0x05,
	ppce_OutOfRange				= 0x06,
	ppce_AlreadyInit			= 0x07,
	ppce_NotInitializeDriver 	= 0x08,
}PpcDeviceError;

typedef enum _PpcDeviceStatus
{
	ppcds_NoError				= 0x00,
	ppcds_ErrorConnection		= 0x01,
	ppcds_ErrorConfiguration		= 0x02,
	ppcds_ErrorLogging			= 0x03,
	ppcds_UndefinedError			= 0x04,
	ppcds_NotInitializeDriver	= 0x05,
}PpcDeviceStatus;

typedef enum _PricePoleState
{
	pps_NotInitialize			= 0x00,
	pps_Online					= 0x01,
	pps_Offline					= 0x02,
}PricePoleState;

typedef struct _PricePole
{
	guint8					num;
	guint8					grade;
	guint32					price;
	PricePoleState			state;
	guint8					symbol_count;

}PricePole;

typedef struct _PricePoleController
{
	gchar*					name;
	gint32					port;
	gboolean				device_is_working;
	PpcDeviceStatus			device_status;
	gboolean				device_status_is_changed;
	PpcDeviceError			last_device_error;

	PricePole				price_poles[MAX_PRICE_POLE_COUNT];
	guint8					price_pole_count;

	gboolean				data_is_changed;

	guint8					price_dp;

	LogParams				log_params;
	gboolean				log_trace;

	gint32					sock;
	SocketStatus 			sock_status;
	ThreadStatus			main_sock_status;

}PricePoleController;

typedef struct _PPCGradePricePack
{
	guint8					grade;
	guint32					price;
}PPCGradePricePack;

typedef struct _PPCPricePacks
{
	PPCGradePricePack		grade_price_packs[MAX_PRICE_POLE_COUNT];
	guint8					grade_price_pack_count;
}PPCPricePacks;

typedef struct _PricePoleClientInfo
{
	guint8					num;

}PricePoleClientInfo;

typedef struct _PPCClient
{
	guint8 					price_pole_controller_index;
	ClientState				state;
	gboolean				device_status_is_change;
	gboolean				logged;
	guint8					access_level;
	gboolean				read_thread_is_active;
	gint32					sock;

	HardwareServerCommand	command;
	PPCPricePacks			param_price_packs;

	CommandState			command_state;
	PpcDeviceError			command_result;

	guint32					message_id;

	gboolean				status_is_change;

	PricePoleClientInfo		price_pole_info[MAX_TANK_COUNT];
	gint8					price_pole_info_count;
}PPCClient;

typedef struct _PPCClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	guint8					price_pole_controller_index;
	gchar* 					device_name;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_enable;
	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}PPCClientInfo;

gpointer ppc_device_thread_func(gpointer data);
void set_ppc_device_is_working(guint8 index, gboolean value);

#endif
