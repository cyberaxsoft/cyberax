#ifndef SYSTEM_FUNC_H_
#define SYSTEM_FUNC_H_

#define INIT_DEVICE_TIMEOUT			5000
#define SOCKET_RECONNECT_TIMEOUT	5000

#define  MAX_STRING_LENGTH			255
#define  MAX_GUID_LENGTH			36

#define  MAX_DEVICE_CLIENT_COUNT	254
#define  NO_CLIENT					0xFF

#define  EXCHANGE_BUFFER_SIZE		16384
#define  SOCKET_BUFFER_SIZE			1024
#define  MAX_BAD_FRAME				3

#define  GUID_LENGTH				40

typedef enum _SocketStatus
{
	ss_Disconnected				= 0,
	ss_ConnectReady				= 1,
	ss_Connected				= 2,
}SocketStatus;

typedef enum _ClientState
{
	cls_Free					= 0,
	cls_Busy					= 1,
	cls_Active					= 2,
	cls_Destroying				= 3,
}ClientState;

typedef enum _DeviceType
{
	dt_System				= 0x00,
	dt_DispencerController	= 0x01,
	dt_TankGaugeSystem		= 0x02,
	dt_FiscalRegister 		= 0x03,
	dt_Terminal				= 0x04,
	dt_CustomerDisplay		= 0x05,
	dt_PricePole			= 0x06,
	dt_BillValidator		= 0x07,
	dt_InputController		= 0x08,
	dt_SensorController		= 0x09,
}DeviceType;

typedef enum _ClientAccessLevel
{
	cal_Disable				= 0x00,
	cal_ReadOnly			= 0x01,
	cal_Control				= 0x02,
	cal_Congiguration		= 0x03,
	cal_Unlim				= 0x04,
}ClientAccessLevel;

typedef struct _SystemClient
{
	ClientState				state;
	gboolean				logged;
	guint8					access_level;
	gboolean				read_thread_is_active;
	gint32					sock;

}SystemClient;

typedef struct _SystemClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_enable;
	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}SystemClientInfo;

typedef enum _HardwareServerCommand
{
	hsc_None						= 0x00000000,
	hsc_GetDeviceStatus				= 0x00000001,
	hsc_GetConfiguration			= 0x00000002,

	hsc_DCGetData					= 0x01010001,
	hsc_DCGetCounters				= 0x01010002,
	hsc_DCSetVolumeDose				= 0x01010003,
	hsc_DCSetSumDose				= 0x01010004,
	hsc_DCSetUnlimDose				= 0x01010005,
	hsc_DCStart						= 0x01010006,
	hsc_DCStop						= 0x01010007,
	hsc_DCPayment					= 0x01010008,
	hsc_DCReset						= 0x01010009,
	hsc_DCExecuteExtendedFunction	= 0x0101000A,
	hsc_DCPriceUpdate				= 0x0101000B,
	hsc_DCSuspend					= 0x0101000C,
	hsc_DCResume					= 0x0101000D,

	hsc_TGSGetTankData				= 0x01020001,
	hsc_TGSGetAlarms				= 0x01020002,

	hsc_FRPlaySound					= 0x01030001,
	hsc_FRCutting					= 0x01030002,
	hsc_FROpenDrawer				= 0x01030003,
	hsc_FRSetDateTime				= 0x01030004,
	hsc_FRXReport					= 0x01030005,
	hsc_FRZReport					= 0x01030006,
	hsc_FRMoneyIncome				= 0x01030007,
	hsc_FRMoneyOutput				= 0x01030008,
	hsc_FRFiscalReceipt				= 0x01030009,
	hsc_FRCancelFiscalReceipt		= 0x0103000A,
	hsc_FRRepeatDocument			= 0x0103000B,
	hsc_FRTextDocument				= 0x0103000C,
	hsc_FROpenShift					= 0x0103000D,
	hsc_FRCorrectionReceipt			= 0x0103000E,

	hsc_CDClear						= 0x01050001,
	hsc_CDAdd						= 0x01050002,
	hsc_CDEdit						= 0x01050003,
	hsc_CDDelete					= 0x01050004,
	hsc_CDShowMessage				= 0x01050005,

	hsc_PPGetData					= 0x01060001,
	hsc_PPSetPrices					= 0x01060002,

	hsc_BVReset						= 0x01070001,
	hsc_BVEnable					= 0x01070002,
	hsc_BVDisable					= 0x01070003,

	hsc_ICGetData					= 0x01080001,

	hsc_SCGetSensorData				= 0x01090001,

	hsc_Reset						= 0x02000000,

	hsc_GetCommonServerConfig		= 0x02000001,
	hsc_UpdateCommonServerConfig	= 0x02000002,

	hsc_GetClientProfiles			= 0x02000003,
	hsc_AddClientProfile			= 0x02000004,
	hsc_UpdateClientProfile			= 0x02000005,
	hsc_DeleteClientProfile			= 0x02000006,

	hsc_GetDispencerControllerConfig= 0x02000007,
	hsc_AddDispencerController		= 0x02000008,
	hsc_UpdateDispencerController   = 0x02000009,
	hsc_DeleteDispencerController	= 0x0200000A,

	hsc_GetTgsConfig				= 0x0200000B,
	hsc_AddTgs						= 0x0200000C,
	hsc_UpdateTgs 					= 0x0200000D,
	hsc_DeleteTgs					= 0x0200000E,

	hsc_GetPricePoleControllerConfig= 0x0200000F,
	hsc_AddPricePoleController		= 0x02000010,
	hsc_UpdatePricePoleController   = 0x02000011,
	hsc_DeletePricePoleController	= 0x02000012,

	hsc_GetSensorControllerConfig	= 0x02000013,
	hsc_AddSensorController			= 0x02000014,
	hsc_UpdateSensorController   	= 0x02000015,
	hsc_DeleteSensorController		= 0x02000016,


}HardwareServerCommand;

typedef enum _CommandState
{
	cs_Free							= 0,
	cs_New							= 1,
	cs_Run							= 2,
	cs_Complete						= 3,
}CommandState;

void threads_mutex_init();

void lock_threads_mutex();
void unlock_threads_mutex();
void wait_threads_mutex();

const gchar* bool_to_str(gboolean value);
gboolean compare_strings(gchar* str1, gchar* str2);
guint64 get_date_time(void);
const gchar* device_type_to_str(DeviceType value);
const gchar* server_command_to_str(HardwareServerCommand command);
guint count_utf8_code_points(gchar* buffer);

#endif 
