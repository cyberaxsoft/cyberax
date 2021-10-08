#ifndef TOKHEIM_H_
#define TOKHEIM_H_

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


#define FUELLING_POINT_ID_REPLY_LENGTH					1
#define FUELLING_POINT_DISPLAY_DATA_REPLY_LENGTH		9
#define FUELLING_POINT_STATUS_REPLY_LENGTH				1
#define SUSPEND_FUELLING_POINT_REPLY_LENGTH				1
#define RESUME_FUELLING_POINT_REPLY_LENGTH				1
#define AUTHORIZE_FUELLING_POINT_REPLY_LENGTH			1
#define SEND_DATA_FOR_FUELLING_POINT_REPLY_LENGTH		1
#define RESET_FUELLING_POINT_REPLY_LENGTH				0
#define FUELLING_POINT_SINGLE_TOTALS_REPLY_LENGTH		17
#define FUELLING_POINT_MULT_TOTALS_REPLY_LENGTH			41
#define SET_FUELLING_POINT_DISPLAY_CONTROL_REPLY_LENGTH	0

#define ACTIVATED_HOSE_ID_REPLY_LENGTH					1
#define DEACTIVATED_HOSE_ID_REPLY_LENGTH				1
#define SEND_CASH_PRICES_REPLY_LENGTH					1

typedef enum _ExchangeState
{
	es_Undefined			= 0,
	es_GetStartStatus		= 1,
	es_ClearDisplay			= 2,
	es_GetCounters			= 3,
	es_DeactivateNozzle		= 4,
	es_GetStatus			= 5,
	es_GetNozzle			= 6,
	es_WaitPreset			= 7,
	es_SendPreset			= 8,
	es_WaitStart			= 9,
	es_GetFillingInfo		= 10,
	es_Stopping				= 11,
	es_Resume				= 12,
	es_GetTotCounters		= 13,
	es_WaitPayment			= 14,
	es_SendPrices			= 15,
}ExchangeState;

typedef enum _RxStage
{
	rs_Undefined			= 0,
	rs_Value				= 1,
	rs_Complement			= 2,
}RxStage;

typedef enum _TokheimStatus
{
	ts_Uninitialized				= 0x2F,
	ts_Idle							= 0x20,
	ts_NozleOff						= 0xA0,
	ts_Authorized					= 0x90,
	ts_Starting						= 0xD0,
	ts_Started						= 0xF0,
	ts_Terminating					= 0x91,
	ts_Terminated					= 0x95,
	ts_Stopping						= 0x98,
	ts_Stopped						= 0x9C,
	ts_CommandCompleted				= 0xB0,
}TokheimStatus;

typedef enum _TokheimGroup
{
	tg_I							= 0,
	tg_II							= 1,
	tg_III							= 2,
}TokheimGroup;

typedef enum _TokheimCommand
{
	tc_FuellingPointId_req				= 0xA0,
	tc_AuxFuellingPointId_req			= 0xB0,
	tc_FuellingPointDisplayData_req		= 0xA1,
	tc_FuellingPointStatus_req			= 0xA2,
	tc_SuspendFuellingPoint				= 0xA3,
	tc_ResumeFuellingPoint				= 0xA4,
	tc_AuthorizeFuellingPoint			= 0xA5,
	tc_AuthorizeFuellingPoint2			= 0xB5,
	tc_SendDataForFuellingPoint			= 0xA6,
	tc_ResetFuellingPoint				= 0xA7,
	tc_FuellingPointTotals_req			= 0xA9,
	tc_SetFuellingPointDisplayControl	= 0xAA,
}TokheimCommand;

typedef enum _TokheimAuxCommand
{
	tac_ActivatedHose_req				= 0xA1,
	tac_DeactivatedHose_req				= 0xA2,
	tac_SendCashPrices_req				= 0xA3,
}TokheimAuxCommand;


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

	guint8				fp_id;
	guint8				start_filling_filter;

	guint8				fault_reply_counter;

	gboolean			counters_enable;

	ExchangeState		exchange_state;

	guint8				fuelling_point_id;
	TokheimGroup		tokheim_group;

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
