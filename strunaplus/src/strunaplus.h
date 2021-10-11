#ifndef TLS_H_
#define TLS_H_


#define MAX_TANK_COUNT			32
#define MAX_LOG_SIZE			1048576
#define	BUFFER_SIZE				128

#define MAX_UART_ERROR			5
#define STRUNA_ADDRESS 			0x50

#define STRUNA_CRC_POLY      	0xA001
#define STRUNA_CRC_INIT      	0xFFFF
#define STRUNA_COMMAND_MASK		0x7F
#define STRUNA_EXCEPTION_MASK	0x80

#define ADDRESS_OFFSET			0
#define COMMAND_OFFSET			1
#define ERROR_OFFSET			2
#define CHANNEL_OFFSET 			5

#define CHANNEL_TYPE_OFFSET 	3
#define CHANNEL_OFFSET_1		4
#define CONFIG_MASK_OFFSET 		6

#define LEVEL_MASK 				0x01
#define VOLUME_MASK 			0x04
#define DENSITY_MASK 			0x08
#define TEMPERATURE_MASK 		0x10
#define WATER_MASK 				0x20

#define DATA_OFFSET 			3
#define DATA_ERROR_OFFSET 		5
#define DATA_VALUE_SIZE 		6



typedef enum _StrunaPlusExchangeState
{
	es_Idle							= 0,
	es_SelectChannel				= 1,
	es_RequestChannelConfiguration	= 2,
	es_WaitChannelConfiguration		= 3,
	es_RequestParameters			= 4,
	es_WaitParameters				= 5,
}StrunaPlusExchangeState;

typedef enum _StrunaPlusRxState
{
	rs_None							= 0,
	rs_WaitAddress					= 1,
	rs_WaitCommand					= 2,
	rs_WaitLength					= 3,
	rs_ReadData						= 4,
	rs_WaitCrcLow					= 5,
	rs_WaitCrcHigh					= 6,
}StrunaPlusRxState;

//----------------------------------------------------------------------------
//			StrunaPlusCommand
//----------------------------------------------------------------------------

typedef enum _StrunaPlusCommand
{
	sc_ReadHoldingRegisters 		= 0x03,
	sc_ReadInputRegisters			= 0x04,
	sc_ForceSingleCoil				= 0x05,
	sc_PresetSingleRegister			= 0x06,
	sc_Diagnostic					= 0x08,
	sc_ReadWriteHoldingRegisters 	= 0x17,
}
StrunaPlusCommand;


typedef struct _Tank
{
	guint32				num;
	guint8				channel;

	gfloat				height;
	gfloat				volume;
	gfloat				density;
	gfloat				weight;
	gfloat				temperature;
	gfloat				water_level;

	gboolean			online;

	guint8				sending_error_counter;
	guint8				uart_error_counter;
}Tank;



ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* TLS_H_ */
