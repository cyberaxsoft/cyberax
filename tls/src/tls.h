#ifndef TLS_H_
#define TLS_H_


#define MAX_TANK_COUNT				32
#define MAX_LOG_SIZE				1048576
#define	READ_BUFFER_SIZE			256

#define MAX_UART_ERROR				5

#define UART_BUFFER_WRITE_SIZE		256

#define REPLY_DATA_OFFSET			26
#define REPLY_DATA_FIELD_LENGTH		8
#define REPLY_CHANNEL_OFFSET		5
#define REPLY_CHANNEL_LENGTH		2


typedef enum _TlsExchangeState
{
	es_WaitSTX				= 0,
	es_CommPefix			= 1,
	es_WaitTilda1			= 2,
	es_WaitTilda2			= 3,
	es_WaitCrc1				= 4,
	es_WaitCrc2				= 5,
	es_WaitCrc3				= 6,
	es_WaitCrc4				= 7,
	es_WaitEtx				= 8,

}TlsExchangeState;

typedef enum _TankExchangeState
{
	tes_Free				= 0x00,
	tes_SendCommand			= 0x01,
}TankExchangeState;


typedef enum _TlsCtrl
{
	ctrl_STX				= 0x01,
	ctrl_ETX				= 0x03,
	ctrl_Tilda				= 0x26,
	ctrl_ErrorPrefix		= 0x39,
	ctrl_CommPrefix			= 0x69,
}TlsCtrl;


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
	TankExchangeState	exchange_state;

	guint8				sending_error_counter;
	guint8				uart_error_counter;
}Tank;



ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* TLS_H_ */
