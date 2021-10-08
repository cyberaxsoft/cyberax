#ifndef DC_DEVICE_H_
#define DC_DEVICE_H_

#define	MAX_DISP_CONTROL_COUNT	16

#define MAX_NOZZLE_COUNT		8
#define MAX_DISPENCER_COUNT		128
#define MAX_DC_EXT_FUNC_COUNT	128
#define MAX_DC_STR_LENGTH		128


#define DEF_DC_PRICE_DP			2
#define DEF_DC_VOLUME_DP		2
#define DEF_DC_AMOUNT_DP		2

typedef enum _DcDeviceError
{
	dce_NoError					= 0x00,
	dce_WrongCommand			= 0x01,
	dce_ErrorConnection 		= 0x02,
	dce_ErrorConfiguration 		= 0x03,
	dce_ErrorLogging			= 0x04,
	dce_FaultDispencerIndex 	= 0x05,
	dce_FaultNozzleIndex  		= 0x06,
	dce_Undefined				= 0x07,
	dce_OutOfRange				= 0x08,
	dce_AlreadyInit				= 0x09,
	dce_FaultExtFuncIndex		= 0x0A,
	dce_NotInitializeDriver 	= 0x0B,
	dce_FaultDispencerNum		= 0x0C,
	dce_FaultNozzleNum			= 0x0D,
	dce_FaultDispencerState		= 0x0E,
	dce_FaultNozzleState		= 0x0F,
	dce_DispencerBusy			= 0x10,

}DcDeviceError;

typedef enum _DcDeviceStatus
{
	dcs_NoError				= 0x00,
	dcs_ErrorConnection		= 0x01,
	dcs_ErrorConfiguration	= 0x02,
	dcs_ErrorLogging		= 0x03,
	dcs_UndefinedError		= 0x04,
	dcs_NotInitializeDriver	= 0x05,
}DcDeviceStatus;

typedef enum _DispencerState
{
	ds_NotInitialize		= 0x00,
	ds_Busy					= 0x01,
	ds_Free					= 0x02,
	ds_NozzleOut			= 0x03,
	ds_Filling				= 0x04,
	ds_Stopped				= 0x05,
	ds_Finish				= 0x06,
	ds_ConnectionError		= 0x07,
}DispencerState;

typedef enum _OrderType
{
	ot_Free					= 0x00,
	ot_Volume				= 0x01,
	ot_Sum					= 0x02,
	ot_Unlim				= 0x03,
}OrderType;


typedef struct _DispencerTask
{
	guint32					dispencer_number;
	guint8					nozzle_number;
	HardwareServerCommand	command;
	float					price;
	float					volume;
	float					sum;
	guint8					index_ext_func;
	guint32					message_id;

}DispencerTask;

typedef struct _Nozzle
{
	guint8					num;
	guint8					grade;
	guint32					counter;
}Nozzle;

typedef struct _Dispencer
{
	guint32					num;
	guint8					addr;
	Nozzle					nozzles[MAX_NOZZLE_COUNT];
	guint8					nozzle_count;

	DispencerState			state;

	OrderType				order_type;
	OrderType				preset_order_type;

	gboolean				is_pay;

	guint8					preset_nozzle_num;
	guint8					active_nozzle_num;

	guint32					preset_price;
	guint32					preset_volume;
	guint32					preset_amount;

	guint32					current_price;
	guint32					current_volume;
	guint32 				current_amount;

	guchar					error;
	guchar					error_description[MAX_DC_STR_LENGTH];

	gboolean				data_is_changed;

}Dispencer;

typedef struct _DispencerExtFunction
{
	gchar*					name;
}DispencerExtFunction;

typedef struct _DispencerController
{
	gchar*					name;
	gint32					port;
	gboolean				device_is_working;
	DcDeviceStatus			device_status;
	gboolean				device_status_is_changed;
	DcDeviceError			last_device_error;

	Dispencer				dispencers[MAX_DISPENCER_COUNT];
	guint8					dispencers_count;

	DispencerExtFunction    ext_functions[MAX_DC_EXT_FUNC_COUNT];
	guint8					ext_functions_count;

	guint8					price_dp;
	guint8					volume_dp;
	guint8					amount_dp;

	LogParams				log_params;
	gboolean				log_trace;

	gint32					sock;
	SocketStatus 			sock_status;
	ThreadStatus			main_sock_status;

}DispencerController;

typedef struct _DCNozzlePricePack
{
	guint8					nozzle_num;
	guint32					price;
}DCNozzlePricePack;

typedef struct _DCPricePacks
{
	DCNozzlePricePack		nozzle_price_packs[MAX_NOZZLE_COUNT];
	guint8					nozzle_price_pack_count;
}DCPricePacks;

typedef struct _DispencerClientInfo
{
	guint32					num;

	gboolean				status_is_change;
	HardwareServerCommand	command;
	DCPricePacks			param_price_packs;
	guint8					param_nozzle_num;
	guint32					param_price;
	guint32					param_volume;
	guint32					param_amount;
	guint8					param_index_ext_func;


	CommandState			command_state;
	DcDeviceError			command_result;

	guint32					message_id;
}DispencerClientInfo;


typedef struct _DCClient
{
	guint8 					dispencer_controller_index;
	ClientState				state;
	gboolean				device_status_is_change;
	gboolean				logged;
	guint8					access_level;
	DispencerClientInfo		dispencer_info[MAX_DISPENCER_COUNT];
	gint8					dispencer_info_count;
	gboolean				read_thread_is_active;
	gint32					sock;
}DCClient;

typedef struct _DCClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	guint8					dispencer_controller_index;
	gchar* 					device_name;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}DCClientInfo;

//void set_dc_device_is_working(guint8 index, gboolean value);

gpointer dc_device_thread_func(gpointer data);

#endif /* DC_DEVICE_H_ */
