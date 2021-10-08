#ifndef TOPAZ_H_
#define TOPAZ_H_


#define TOPAZ_EXTENDED_START
//#define TOPAZ_OLD_START

#define MAX_DISP_COUNT				32
#define MAX_NOZZLE_COUNT			8

#define	READ_BUFFER_SIZE			255
#define WRITE_BUFFER_SIZE		255

#define MAX_UART_ERROR				5

#define ctrl_STX					0x02
#define ctrl_ETX					0x03
#define ctrl_BEL					0x07
#define ctrl_ACK					0x06
#define ctrl_NAK					0x15
#define ctrl_CAN					0x18
#define ctrl_DEL					0x7F

#define STX_REPLY_OFFSET			1
#define DATA_REPLY_OFFSET			2
#define VERSION_LENGTH				8
#define VOLUME_PRESET_SIZE			5
#define SUM_PRESET_SIZE				6
#define VALVE_PRESET_SIZE			3
#define PRICE_PRESET_SIZE			4
#define BMU_PRESET_SIZE				6

#define DEF_VOLUME_LENGTH			6
#define DEF_PRICE_LENGTH			4
#define DEF_AMOUNT_LENGTH			6

typedef enum _ChannelExchangeState
{
	ces_Undefined			= 0,
	ces_GetVersion			= 1,
	ces_GetCounter			= 2,
	ces_Idle				= 3,
	ces_SendPrice			= 4,
	ces_Preset				= 5,
	ces_Start				= 6,
	ces_WaitStart			= 7,
	ces_ConfirmStart		= 8,
	ces_ActiveFilling		= 9,
	ces_GetFillingData		= 10,
	ces_WaitPayment			= 11,
	ces_GetTotalCounter		= 12,
	ces_ConfirmPayment		= 13,
	ces_EmergencyStop		= 14,



}ChannelExchangeState;

typedef enum _FrameStage
{
	fs_WaitDEL				= 0,
	fs_WaitSTX				= 1,
	fs_ReadData				= 2,
	fs_ReadDataCompl		= 3,
	fs_WaitSecondStop		= 4,
	fs_WaitCRC				= 5,
}FrameStage;

typedef enum _AztDispencerStatus
{
	ads_Free				= 0x30,
	ads_NozzleOff			= 0x31,
	ads_FillingEnable		= 0x32,
	ads_Filling				= 0x33,
	ads_Stoped				= 0x34,
	ads_BMU					= 0x38,
}AztDispencerStatus;

typedef enum _AztCommandCode
{
	acc_GetStatusReq		= 0x31,
	acc_EnableFillingReq	= 0x32,
	acc_StopFillingReq		= 0x33,
	acc_FillingDataReq		= 0x34,
	acc_FullFillingDataReq	= 0x35,
	acc_TotalCountersReq	= 0x36,
	acc_PumpTypeReq			= 0x37,
	acc_ConfirmFilling		= 0x38,

	acc_GetVersion			= 0x50,
	acc_PricePreset			= 0x51,
	acc_ValvePreset			= 0x52,
	acc_MoneyPreset			= 0x53,
	acc_LitresPreset		= 0x54,

	acc_StartFilling		= 0x56,
	acc_GetDose				= 0x58,
}AztCommandCode;


typedef struct _Nozzle
{
	guint8					num;
	guint32					price;
	guint32					counter;

	ChannelExchangeState	state;
	guint8					volume_length;
	guint8					price_length;
	guint8					amount_length;
	guint32					version;
	gboolean				command_send;
}Nozzle;

typedef struct _Dispencer
{
	guint32				num;
	guint8				addr;
	guint8				nozzle_count;
	Nozzle				nozzles[MAX_NOZZLE_COUNT];

	guint8				current_nozzle_index;

	DispencerState		dispencer_state;

	DriverCommand		current_command;

	OrderType 			preset_order_type;
	gint8				preset_nozzle_index;
	guint32				preset_price;
	guint32				preset_volume;
	guint32				preset_amount;

	OrderType 			order_type;
	gint8				active_nozzle_index;
	guint32				current_price;
	guint32				current_volume;
	guint32				current_amount;

	guint8				error;

	gboolean			start;
	gboolean			is_pay;
	gboolean			emergency_stop;
	gboolean			reset;

	gboolean			suspend;
	gboolean			resume;

	gboolean			get_counters;

	gboolean			nozzle_was_on;


}Dispencer;


ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* TOPAZ_H_ */
