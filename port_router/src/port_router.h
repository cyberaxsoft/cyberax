#ifndef PORT_ROUTER_H_
#define PORT_ROUTER_H_

#define False						0
#define True						1

#define MAX_DEVICE_COUNT			16
#define MAX_PORT_COUNT				4
#define MAX_CLIENT_COUNT			128

#define SOCKET_BUFFER_SIZE			1024
#define IDC_HEADER_LENGTH			11

#define IDC_START_MARKER			0x1B
#define IDC_END_MARKER				0x1D
#define IDC_ESCAPE_SYMBOL			0xDC
#define IDC_ESCAPED_START_MARKER	0xAB
#define IDC_ESCAPED_ESCAPE			0xAC
#define IDC_ESCAPED_END_MARKER		0xAD

#define IDC_USB_ADDRESS				0xFF

#define DM_ID						0x11
#define CFG_ID						0x12
#define PORT_TRANSFER_ID			0x1C
#define SENSORS_ID					0x1A

#define IDC_CMD_RECV				0x01
#define IDC_CMD_SEND				0x02
#define IDC_CMD_ACK					0x03

#define IDC_CMD_SET_UART_CFG		0x0A
#define IDC_CMD_GET_UART_CFG		0x0B
#define IDC_CMD_UART_CFG			0x0C
#define IDC_CMD_RESET				0x03
#define IDC_CMD_RESET_REPLY			0x04

#define IDC_CMD_SET_SENSOR_CFG		0x1A
#define IDC_CMD_GET_SENSOR_CFG		0x1B
#define IDC_CMD_SENSOR_CFG			0x1C



#define CRC16_POLY       			0x1021
#define CRC16_INIT       			0x00

#define IDC_INTERFACE_ID_OFFSET		0
#define	IDC_SOURCE_OFFSET			1
#define IDC_DESTINATION_OFFSET		2
#define IDC_COMMAND_OFFSET			3
#define IDC_PORT_OFFSET				4
#define IDC_DATA_LENGTH_OFFSET		5
#define IDC_DATA_OFFSET				6

#define IDC_FRAME_LENGTH			64
#define DATA_MAX_FRAME_LENGTH		50

#define SEARCH_TIMEOUT				1000

typedef enum _ClientState
{
	cs_Free						= -1,
	cs_ConnectReady				= -2,
}ClientState;

typedef enum _IdcExchangeState
{
	ies_WaitStart				= 0,
	ies_WaitEnd					= 1,
}IdcExchangeState;

typedef enum _UsbDeviceStage
{
	uds_Undefined				= 0,
	uds_Configuration			= 1,
	uds_Reset					= 2,
	uds_Idle					= 3,
}UsbDeviceStage;

typedef enum _UsbDevicePortStage
{
	udps_Undefined				= 0,
	udps_WaitConfiguration		= 1,
	udps_WaitReset				= 2,
	udps_Idle					= 3,

}UsbDevicePortStage;

typedef struct _UsbDevicePort
{
	GMutex				mutex;
	guint8				interface_id;
	guint8 				num;
	guint16 			ip_port;
	gint32				sock;
	GThread*			serv_thread;
	gint32				client_socks[MAX_CLIENT_COUNT];
	guint8				max_client_count;
	guint32				baudrate;
	guint8				byte_size;
	guint8				parity;
	guint8				stop_bits;

	guint8				duplex;
	guint8				debug;
	guint8				passtrough;
	guint8				invert_rx;
	guint8				invert_tx;
	guint8				req_timeout;
	guint8				recv_timeout;
	guint8				application;

	UsbDevicePortStage	stage;

}UsbDevicePort;

typedef struct _UsbDevice
{
	guint16 		vendor_id;
	guint16 		product_id;
	gchar*			serial_number;
	guint8			idc_id;
	UsbDevicePort 	ports[MAX_PORT_COUNT];
	guint8			port_count;
	hid_device*		handle;
	GMutex			hid_mutex;
	GThread*		usb_device_thread;
	GThread*		configuration_thread;
	gboolean		is_connected;
	gboolean		stoping;

	UsbDeviceStage	stage;
	gboolean 		usb_message_sended;

}UsbDevice;

typedef struct _MultDeviceUnit
{
	gchar*			serial_number;
	guint8 			num;
	guint8			idc_id;
	gint8			device_index;
}MultDeviceUnit;

typedef struct _MultDeviceUnits
{
	MultDeviceUnit		units[MAX_DEVICE_COUNT];
	guint8 				unit_count;
}MultDeviceUnits;

typedef struct _MultDevice
{
	GMutex				mutex;
	guint16 			ip_port;
	gint32				sock;
	GThread*			serv_thread;
	gint32				client_socks[MAX_CLIENT_COUNT];
	guint8				max_client_count;
	MultDeviceUnits		units;
}MultDevice;

typedef struct _PortRouterConfig
{
	gboolean		log_enable;
	gchar*			log_dir;
	LogOptions		log_options;
	UsbDevice		devices[MAX_DEVICE_COUNT];
	guint8			device_count;
	MultDevice		mult_devices[MAX_DEVICE_COUNT];
	guint8			mult_device_count;


}PortRouterConfig;

typedef struct _ServThreadParam
{
	guint8 device_index;
	guint8 port_index;
	gchar* serial_number;
	guint8 max_client_count;
	LogOptions log_options;
}ServThreadParam;

typedef struct _MultServThreadParam
{
	guint8 device_index;
	guint32 sock;
	guint8 max_client_count;
	MultDeviceUnits	units;
	LogOptions log_options;
}MultServThreadParam;


typedef struct _ClientThreadParam
{
	guint8 device_index;
	guint8 port_index;
	guint8 client_index;
	gchar* serial_number;
	LogOptions log_options;
}ClientThreadParam;

typedef struct _MultClientThreadParam
{
	guint8 device_index;
	guint8 client_index;
	guint32 sock;
	MultDeviceUnits	units;
	LogOptions log_options;
}MultClientThreadParam;

typedef struct _UsbThreadParam
{
	guint8 device_index;
	guint16 vendor_id;
	guint16 product_id;
	gchar* serial_number;
	LogOptions log_options;
}UsbThreadParam;

#endif /* PORT_ROUTER_H_ */
