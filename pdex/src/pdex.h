#ifndef PDEX_H_
#define PDEX_H_

#define MAX_DISP_COUNT						32
#define MAX_NOZZLE_COUNT					8

#define	READ_BUFFER_SIZE					255
#define WRITE_BUFFER_SIZE					255

#define SLOW_FLOW_OFFSET					5

#define FP_MASK								0xF0
#define EXT_MASK							0xC0
#define EXT2_MASK							0xD0


#define PRICE_FIELD_SIZE					2
#define AMOUNT_FIELD_SIZE					3
#define VOLUME_FIELD_SIZE					3

#define SINGLE_COUNTER_LENGTH				4
#define MULT_COUNTER_LENGTH					5

#define START_FILLLING_FILTER_VALUE			3

#define MAX_FAULT_REPLY_COUNT				3

#define	PDEX_AUTHORIZATION_CODE_LENGTH		8
#define	PDEX_START_ADDRESS					0x20

#define	PDEX_ADDRESS_OFFSET					0
#define	PDEX_CTRL_OFFSET					1
#define	PDEX_COMM_OFFSET					2
#define	PDEX_PARAM_OFFSET					2
#define	PDEX_DATA_OFFSET					3

#define	PDEX_ERROR_CODE_LENGTH				2
#define	PDEX_TRANSACTION_NUMBER_LENGTH		3
#define	PDEX_NOZZLE_LENGTH					1
#define	PDEX_VOLUME_LENGTH					6
#define	PDEX_AMOUNT_LENGTH					6
#define	PDEX_PRICE_LENGTH					4
#define	PDEX_REGISTER_LENGTH				12

#define PDEX_INIT_ERROR_CODE				99

#define TRK_AUTH_CODE_LENGTH				8

typedef enum _ExchangeState
{
	es_Undefined							= 0,
	es_Initialize							= 1,
	es_Status								= 2,
	es_DataReq								= 3,
	es_GetCounters							= 4,
	es_Idle									= 5,
	es_Stop									= 6,
}ExchangeState;

typedef enum _PdexCtrlChar
{
	ctrl_Undefined							= 0x00,
	ctrl_SOH								= 0x01,
	ctrl_STX								= 0x02,
	ctrl_ETX								= 0x03,
	ctrl_ENQ								= 0x05,
	ctrl_ACK								= 0x06,
	ctrl_NAK								= 0x15,
	ctrl_SYN								= 0x16,
	ctrl_CAN								= 0x18,
}PdexCtrlChar;

typedef enum _PdexDispState
{
	pds_Locked								= 0,
	pds_Idle								= 1,
	pds_NozzleUp							= 2,
	pds_Fuelling							= 3,
	pds_EndOfFuelling						= 4,
}
PdexDispState;

typedef enum _PdexControl
{
	pc_Undefined							= 0x00,
	pc_Lock									= 0x30,
	pc_Unlock								= 0x31,
	pc_Payment								= 0x32,
	pc_LightsOn								= 0x33,
	pc_LightsOff							= 0x34,
	pc_Reset								= 0x35,
	pc_GreenLight							= 0x36,
	pc_Preset								= 0x39,
}PdexControl;

typedef enum _PdexFrameStage
{
	pfs_ReadSyn								= 0,
	pfs_WaitAddr							= 1,
	pfs_WaitData							= 2,
	pfs_WaitEtx								= 3,
	pfs_WaitCrc1							= 4,
	pfs_WaitCrc2							= 5,
}PdexFrameStage;

typedef enum _PdexCommandCode
{
	pcc_Authorization						= 0x41,
	pcc_StatusControl						= 0x43,
	pcc_DisplayDataRequest 					= 0x44,
	pcc_Initialization						= 0x49,
	pcc_SetDate								= 0x4E,
	pcc_Status								= 0x53,
	pcc_GetCounter							= 0x58,
}PdexCommandCode;

typedef enum _PdexReplyCode
{
	prc_DisplayDataRequest					= 0x64,
	prc_Error								= 0x65,
	prc_Initialization	 					= 0x69,
	prc_Status								= 0x73,
	prc_GetCounter							= 0x78,
}PdexReplyCode;

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

	DispencerState		dispencer_state;
	guint8				start_filling_filter;
	guint8				fault_reply_counter;
	ExchangeState		exchange_state;
	PdexCtrlChar		char_ready;
	gboolean			command_sended;
	guint8				authorization_code[TRK_AUTH_CODE_LENGTH];

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

	gboolean			start;
	gboolean			is_pay;
	gboolean			emergency_stop;
	gboolean			reset;
	gboolean			send_prices;
	gboolean			get_counters;

	gboolean			suspend;
	gboolean			resume;

	guint8				error;

}Dispencer;


ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* DART_H_ */
