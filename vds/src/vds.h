#ifndef VDS_H_
#define VDS_H_

#define MAX_PRICE_POLE_COUNT		8

#define	READ_BUFFER_SIZE			24
#define WRITE_BUFFER_SIZE			24

#define MAX_UART_ERROR				3

#define ctrl_STX					0x02
#define ctrl_ETX					0x03

#define LAYER_OFFSET				1
#define POLE_NUM_OFFSET				3
#define COMMAND_OFFSET				5
#define SYMBOL_COUNT_OFFSET			7
#define PRICE_OFFSET				9

#define LAYER_LENGTH				2
#define POLE_NUM_LENGTH				2
#define COMMAND_LENGTH				2
#define SYMBOL_COUNT_LENGTH			2

#define DEF_LAYER					1

typedef enum _VdsExchangeState
{
	ves_GetPrice					= 0,
	ves_SetPrice					= 1,
}VdsExchangeState;

typedef enum _VdsFrameStage
{
	vfs_WaitSTX						= 0,
	vfs_WaitETX						= 1,
	vfs_WaitLRC						= 2,
}VdsFrameStage;

typedef enum _VdsCommandCode
{
	vcc_SetPrice					= 0,
	vcc_GetPrice					= 1,
}VdsCommandCode;

typedef struct _VdsPricePole
{
	guint8 							grade;
	guint32							price;
	PricePoleState					state;
	VdsExchangeState				exchange_state;
	guint8 							uart_error;
	gboolean						command_sent;
}VdsPricePole;

ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* VDS_H_ */
