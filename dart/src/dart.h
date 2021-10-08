#ifndef DART_H_
#define DART_H_

#define MAX_DISP_COUNT				32
#define MAX_NOZZLE_COUNT			8
#define	READ_BUFFER_SIZE			255

#define DART_ADDRESS_MASK			0xF0
#define DART_ADDRESS_OFFSET			0x50
#define DART_PRICE_LENGTH			3
#define DART_VOLUME_LENGTH			4
#define DART_AMOUNT_LENGTH			4
#define DART_PRICE_LENGTH			3
#define DART_TOT_COUNTER_LENGTH		5
#define BLOCK_NUMBER_MASK			0x0F
#define NOZZLE_NUM_MASK				0x0F
#define CONTROL_TYPE_MASK			0xF0
#define NOZZLE_STATE_MASK			0x10

#define CTRL_ETX					0x03
#define CTRL_DLE					0x10
#define	CTRL_SF						0xFA

#define MAX_UART_ERROR				5

#define UART_BUFFER_WRITE_SIZE		255

#define ADDR_OFFSET					0
#define CONTROL_TYPE_OFFSET			1
#define DATA_OFFSET					2
#define FRAME_SYSTEM_SYMBOL_LEN		4


typedef enum _DartDispencerState
{
	dds_NotProgrammed				= 0x00,
	dds_Reset						= 0x01,
	dds_Authorized					= 0x02,
	dds_Filling						= 0x04,
	dds_FillingComplete				= 0x05,
	dds_MaxVolReached				= 0x06,
	dds_SwitchedOff					= 0x07,
}DartDispencerState;

typedef enum _DartExchangeMode
{
	dem_Pooling						= 0x00,
	dem_SendCommand					= 0x01,
}DartExchangeMode;

typedef enum _DartExchangeState
{
	des_Free						= 0x00,
	des_SendCommand					= 0x01,
	des_ReadyAck					= 0x02,
	des_ReadyNak					= 0x03,
}DartExchangeState;

typedef enum _DartCommand
{
	dc_NoCommand					= 0x00,
	dc_CommandToPump				= 0x01,
	dc_AllowedNozzle				= 0x02,
	dc_PresetVolume					= 0x03,
	dc_PresetAmount					= 0x04,
	dc_PriceUpdate					= 0x05,
	dc_TotalCounters				= 0x65,
}DartCommand;

typedef enum _DartCommandPump
{
	dcp_ReturnStatus				= 0x00,
	dcp_ReturnParameters			= 0x02,
	dcp_ReturnIdentity				= 0x03,
	dcp_ReturnFillingInfo			= 0x04,
	dcp_Reset						= 0x05,
	dcp_Authorize					= 0x06,
	dcp_Stop						= 0x08,
	dcp_SwitchOff					= 0x0A,
	dcp_Suspend						= 0x0E,
	dcp_Resume						= 0x0F,
}DartCommandPump;

typedef enum _DartParseStage
{
	dps_WaitTag						= 0,
	dps_WaitLen						= 1,
}DartParseStage;


typedef enum _DartControlType
{
	dct_Poll						= 0x20,
	dct_Data						= 0x30,
	dct_Ack							= 0xC0,
	dct_Nak							= 0x50,
	dct_Eot							= 0x70,
	dct_AckPoll						= 0xE0,
}DartControlType;

typedef enum _DartDispencerReplyType
{
	ddrt_DC0_None					= 0x00,
	ddrt_DC1_PumpStatus				= 0x01,
	ddrt_DC2_FilledVolumeAndAmount	= 0x02,
	ddrt_DC3_NozzleStatusAndPrice	= 0x03,
	ddrt_DC5_AlarmCode				= 0x05,
	ddrt_DC7_Pump_Pararmeters		= 0x07,
	ddrt_DC9_PumpIdentity			= 0x09,
	ddrt_DC14_SuspentReply			= 0x14,
	ddrt_DC15_ResumeReply			= 0x15,
	ddrt_DC101_TotalCounters		= 0x65,
}DartDispencerReplyType;

typedef struct _Nozzle
{
	guint8			num;
	guint8			grade;
	guint32			price;
	guint32			counter;
}Nozzle;

typedef struct _Dispencer
{
	guint32				num;
	guint8				addr;
	guint8				nozzle_count;
	Nozzle				nozzles[MAX_NOZZLE_COUNT];

	DartDispencerState	original_state;
	gboolean			original_nozzle_state;
	guint8				original_active_grade;

	DispencerState		dispencer_state;

	DartExchangeMode	exchange_mode;
	DartExchangeState	exchange_state;

	guint8				block_sequence_number;
	guint8				reply_block_sequence_number;

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

	gboolean			get_counters;
	guint8				current_counter_index;

	guint8				sending_error_counter;
	guint8				uart_error_counter;

	gboolean			start;
	gboolean			is_pay;
	gboolean			emergency_stop;
	gboolean			reset;
	gboolean			send_prices;

	gboolean			suspend;
	gboolean			resume;

	guint8				error;

}Dispencer;


ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* DART_H_ */
