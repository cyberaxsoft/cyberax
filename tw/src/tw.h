#ifndef TW_H_
#define TW_H_

#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include "logger.h"

#define LOG_PREFIX					"TW"
#define LOG_FILENAME				"tw.log"

#define DRIVER_DESCRIPTION_LENGTH	128

#define MAX_DISP_COUNT				32
#define MAX_NOZZLE_COUNT			8
#define MAX_LOG_SIZE				1048576
#define	READ_BUFFER_SIZE			256
#define	TW_BUFFER_SIZE				1024

#define UART_BUFFER_WRITE_SIZE		256
#define MAX_UART_ERROR				5

typedef enum _DriverError
{
	de_NoError				= 0x00,
	de_WrongCommand			= 0x01,
	de_ErrorConnection 		= 0x02,
	de_ErrorConfiguration 	= 0x03,
	de_ErrorLogging			= 0x04,
	de_FaultDispencerIndex 	= 0x05,
	de_FaultNozzleIndex  	= 0x06,
	de_Undefined			= 0x07,
	de_OutOfRange			= 0x08,
	de_AlreadyInit			= 0x09,
	de_FaultExtFuncIndex	= 0x0A,
	de_NotInitializeDriver 	= 0x0B,
	de_FaultDispencerNum	= 0x0C,
	de_FaultNozzleNum		= 0x0D,
	de_FaultDispencerState	= 0x0E,
	de_FaultNozzleState		= 0x0F,
	de_DispencerBusy		= 0x10,

}DriverError;

typedef enum _DriverStatus
{
	drs_NoError				= 0x00,
	drs_ErrorConnection		= 0x01,
	drs_ErrorConfiguration	= 0x02,
	drs_ErrorLogging		= 0x03,
	drs_UndefinedError		= 0x04,
	drs_NotInitializeDriver	= 0x05,
}DriverStatus;

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
	ot_Amount				= 0x02,
	ot_Unlim				= 0x03,
}OrderType;

typedef enum _ConnectionType
{
	ct_Uart					= 0x00,
	ct_TcpIp				= 0x01,
}ConnectionType;

typedef enum _DriverCommand
{
 	drc_Free				= 0x00,
 	drc_PresetVolume		= 0x01,
 	drc_PresetAmount		= 0x02,
 	drc_Start				= 0x03,
 	drc_Stop				= 0x04,
 	drc_Payment				= 0x05,
 	drc_Reset				= 0x06,
 	drc_SendPrices			= 0x07,
}DriverCommand;


// 
// typedef enum _DartDispencerState
// {
// 	dds_NotProgrammed		= 0x00,
// 	dds_Reset				= 0x01,
// 	dds_Authorized			= 0x02,
// 	dds_Filling				= 0x04,
// 	dds_FillingComplete		= 0x05,
// 	dds_MaxVolReached		= 0x06,
// 	dds_SwitchedOff			= 0x07,
// }DartDispencerState;
// 
// typedef enum _DartExchangeMode
// {
// 	dem_Pooling				= 0x00,
// 	dem_SendCommand			= 0x01,
// }DartExchangeMode;
// 
// typedef enum _DartExchangeState
// {
// 	des_Free				= 0x00,
// 	des_SendCommand			= 0x01,
// 	des_ReadyAck			= 0x02,
// 	des_ReadyNak			= 0x03,
// 	des_ReadyEot			= 0x04,
// 	des_ReplyNak			= 0x05,
// }DartExchangeState;
// 
// typedef enum _DartCommand
// {
// 	dc_NoCommand			= 0x00,
// 	dc_CommandToPump		= 0x01,
// 	dc_AllowedNozzle		= 0x02,
// 	dc_PresetVolume			= 0x03,
// 	dc_PresetAmount			= 0x04,
// 	dc_PriceUpdate			= 0x05,
// 	dc_TotalCounters		= 0x65,
// }DartCommand;
// 
// typedef enum _DartCommandPump
// {
// 	dcp_ReturnStatus		= 0x00,
// 	dcp_ReturnParameters	= 0x02,
// 	dcp_ReturnIdentity		= 0x03,
// 	dcp_ReturnFillingInfo	= 0x04,
// 	dcp_Reset				= 0x05,
// 	dcp_Authorize			= 0x06,
// 	dcp_Stop				= 0x08,
// 	dcp_SwitchOff			= 0x0A,
// }DartCommandPump;
// 
// typedef enum _DartParseStage
// {
// 	dps_WaitTag				= 0,
// 	dps_WaitLen				= 1,
// }DartParseStage;
// 
// 
// typedef enum _DartControlType
// {
// 	dct_Poll				= 0x20,
// 	dct_Data				= 0x30,
// 	dct_Ack					= 0xC0,
// 	dct_Nak					= 0x50,
// 	dct_Eot					= 0x70,
// 	dct_AckPoll				= 0xE0,
// }DartControlType;
// 
// typedef enum _DartDispencerReplyType
// {
// 	ddrt_DC0_None					= 0x00,
// 	ddrt_DC1_PumpStatus				= 0x01,
// 	ddrt_DC2_FilledVolumeAndAmount	= 0x02,
// 	ddrt_DC3_NozzleStatusAndPrice	= 0x03,
// 	ddrt_DC5_AlarmCode				= 0x05,
// 	ddrt_DC7_Pump_Pararmeters		= 0x07,
// 	ddrt_DC9_PumpIdentity			= 0x09,
// 	ddrt_DC14_SuspentReply			= 0x14,
// 	ddrt_DC15_ResumeReply			= 0x15,
// 	ddrt_DC101_TotalCounters		= 0x65,
// }DartDispencerReplyType;
// 
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

// 	DartDispencerState	original_state;
// 	gboolean			original_nozzle_state;
// 	guint8				original_active_grade;
// 
 	DispencerState		dispencer_state;
// 
// 	DartExchangeMode	exchange_mode;
// 	DartExchangeState	exchange_state;
// 	guint8				block_sequence_number;
// 	guint8				reply_block_sequence_number;
// 
 	DriverCommand		current_command;
// 
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

// 	gboolean			get_counters;
// 	guint8				current_counter_index;
// 
// 	guint8				sending_error_counter;
// 	guint8				uart_error_counter;
// 
 	gboolean			start;
 	gboolean			is_pay;
 	gboolean			reset;
 	gboolean			send_prices;

	gboolean			suspend;
	gboolean			resume;


 	guint8				error;

}Dispencer;

typedef struct _LogOptions
{
	gboolean		enable;
 	gchar*			dir;

 	gboolean 		trace;
 	gboolean 		system;
 	gboolean 		requests;
 	gboolean 		frames;
 	gboolean 		parsing;

}LogOptions;

typedef struct _ConnOptions
{
	ConnectionType	connection_type;
 	gchar*			port;
 	gchar*			ip_address;
 	guint32			ip_port;
}ConnOptions;

typedef struct _TimeoutOptions
{
 	guint32			t_read;
 	guint32			t_write;
}TimeoutOptions;

typedef struct _DecimalPointOptions
{
 	guint8			dp_price;
 	guint8			dp_volume;
 	guint8			dp_amount;
}DecimalPointOptions;

typedef struct _NozzleConf
{
 	guint8			num;
 	guint8			grade;
}NozzleConf;

typedef struct _DispencerConf
{
	guint32				num;
	guint8				addr;
	guint8				nozzle_count;
	NozzleConf			nozzles[MAX_NOZZLE_COUNT];
}DispencerConf;

typedef struct _LibConfig
{
	LogOptions			log_options;
	ConnOptions			conn_options;
 	TimeoutOptions 		timeout_options;
 	DecimalPointOptions	decimal_point_options;

 	gboolean			counters_enable;
 	gboolean			auto_start;
 	gboolean			auto_payment;
 	guint32				full_tank_volume;

 	guint8				dispencer_count;
 	DispencerConf		dispencers[MAX_DISP_COUNT];

}LibConfig;
// 
// guint64 get_date_time(void);
// void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size);
// guint32 packet_bcd_to_bin(guint8* buff, guint8 size);
// guint16 calc_crc(guint8* buffer, guint8 length);
// 
// guint8 prepare_preset_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number,
// 		DartCommand command, FILE** log_file, LogOptions log_options, GMutex* log_mutex);
// guint8 prepare_set_prices_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, FILE** log_file, LogOptions log_options, GMutex* log_mutex);
// 
// const gchar* bool_to_str(gboolean value);
// 
// //gboolean read_settings(const gchar* filename, DartConfig* configuration, const gchar* prefix);
// //gboolean write_settings(const gchar* filename, DartConfig* configuration, const gchar* prefix);
// 
gint open_uart(gchar* port_name, FILE** log, LogOptions log_options, GMutex* log_mutex, const gchar* prefix);
void set_settings_uart(gint port_descriptor, guint32 timeout, FILE** log, LogOptions log_options, GMutex* log_mutex, const gchar* prefix);
void close_uart(gint port_descriptor);

// 
// void safe_set_active_disp_index(guint8 new_value);
// guint8 safe_get_active_disp_index();
// void safe_get_preset(guint8 disp_index, gint8* nozzle_index, guint32* price, guint32* volume, guint32* amount);
// guint8 safe_get_nozzle_num(guint8 disp_index, guint8 nozzle_index);
// guint8 safe_get_nozzle_count(guint8 disp_index);
// guint32 safe_get_nozzle_price(guint8 disp_index, guint8 nozzle_index);
// DriverError safe_get_disp_addr(guint8 disp_index, guint8* disp_addr);
// DartExchangeState safe_get_exchange_state(guint8 disp_index);
// void safe_set_exchange_state(guint8 disp_index, DartExchangeState new_value);
// void safe_increment_uart_error_counter(guint8 disp_index);
// guint8 safe_get_uart_error_counter(guint8 disp_index);
// void safe_set_uart_error_counter(guint8 disp_index, guint8 new_value);
// void safe_set_block_sequence_number(guint8 disp_index, guint8 new_value);
// guint8 safe_get_block_sequence_number(guint8 disp_index);
// DartExchangeMode safe_get_exchange_mode(guint8 disp_index);
// void safe_set_exchange_mode(guint8 disp_index, DartExchangeMode new_value);
// DriverCommand safe_get_current_command(guint8 disp_index);
// void safe_set_current_command(guint8 disp_index, DriverCommand new_value);
// DispencerState safe_get_dispencer_state(guint8 disp_index);
// void safe_set_dispencer_state(guint8 disp_index, DispencerState new_value);
// gboolean safe_get_get_counters(guint8 disp_index);
// void safe_set_get_counters(guint8 disp_index, gboolean new_value);
// guint8 safe_get_uart_error_counter(guint8 disp_index);
// void safe_set_uart_error_counter(guint8 disp_index, guint8 new_value);
// void safe_set_reply_block_sequence_number(guint8 disp_index, guint8 new_value);
// guint8 safe_get_reply_block_sequence_number(guint8 disp_index);
// void safe_set_counters_enable(gboolean new_value);
// gboolean safe_get_counters_enable();
// guint8 safe_get_current_counter_index(guint8 disp_index);
// void safe_set_current_counter_index(guint8 disp_index, guint8 new_value);
// void safe_increment_current_counter_index(guint8 disp_index);
// void safe_disp_clear(guint8 disp_index);
// guint8 safe_get_sending_error_counter(guint8 disp_index);
// void safe_set_sending_error_counter(guint8 disp_index, guint8 new_value);
// void safe_increment_sending_error_counter(guint8 disp_index);
// void safe_set_last_sended_disp_index(guint8 disp_index);
// guint8 safe_get_last_sended_disp_index();
// gboolean safe_get_emergency_stop(guint8 disp_index);
// void safe_set_emergency_stop(guint8 disp_index, gboolean new_value);
// gboolean opriginal_nozzle_state(guint8 disp_index);
// void safe_set_reset(guint8 disp_index, gboolean new_value);
// DartDispencerState safe_get_original_state(guint8 disp_index);
// void safe_set_original_state(guint8 disp_index, DartDispencerState new_value);
// guint32 safe_get_current_volume(guint8 disp_index);
// void safe_set_current_volume(guint8 disp_index, guint32 new_value);
// guint32 safe_get_current_amount(guint8 disp_index);
// void safe_set_current_amount(guint8 disp_index, guint32 new_value);
// guint32 safe_get_current_price(guint8 disp_index);
// void safe_set_current_price(guint8 disp_index, guint32 new_value);
// guint8 safe_get_original_active_grade(guint8 disp_index);
// void safe_set_original_active_grade(guint8 disp_index, guint8 new_value);
// gboolean safe_get_original_nozzle_state(guint8 disp_index);
// void safe_set_original_nozzle_state(guint8 disp_index, gboolean new_value);
// guint8 safe_get_error(guint8 disp_index);
// void safe_set_error(guint8 disp_index, guint8 new_value);
// 
// void safe_set_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32 value);
// 
// void safe_set_active_nozzle_index(guint8 disp_index, gint8 new_value);
// 
// gint8 safe_get_nozzle_index_by_grade(guint8 disp_index, guint8 nozzle_grade);
// 
// DriverError safe_get_disp_index_by_addr(guint8 disp_addr, guint8* disp_index);
// 
// guint8 prepare_pool_frame(guint8* buffer, guint8 disp_addr);
// guint8 prepare_status_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
// guint8 prepare_command_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number);
// guint8 prepare_ack_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
// guint8 prepare_nak_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
// guint8 prepare_get_counter_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number, guint8 nozzle_index);
// guint8 prepare_ack_pool_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
// guint8 prepare_preset_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, DartCommand command, FILE** log, LogOptions log_options, GMutex* log_mutex);
// 
// void get_error_description(guint8 error_code, guchar* buffer);
// void get_original_pump_status_description(guint8 status, guchar* buffer);
// 
// const gchar* ds_to_str(DispencerState state);

#endif /* TW_H_ */
