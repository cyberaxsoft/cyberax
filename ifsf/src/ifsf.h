#ifndef IFSF_H_
#define IFSF_H_

#define MAX_DISP_COUNT				32
#define MAX_NOZZLE_COUNT			8

#define MAX_LOG_SIZE				1048576
#define	READ_BUFFER_SIZE			256
#define	IFSF_BUFFER_SIZE			1024

#define UART_BUFFER_WRITE_SIZE		256
#define MAX_UART_ERROR				5

#define IFSF_MAX_DATA_LENGTH		1024
#define IFSF_DATA_OFFSET			10

#define LON_TRANSPORT_HEADER_SIZE   8

#define MAX_EXCHANGE_NODE_COUNT		8
#define MAX_PRODUCT_ID_COUNT 		8
#define MAX_NODE_COUNT 				16

#define SEND_COMMAND_TIMEOUT		10

typedef enum _ExchangeStage
{
	esg_SendZeroFunc		= 0,
	esg_SetNozzleConf		= 1,
	esg_SetFpParam			= 2,
	esg_GetCounters			= 3,
	esg_Work				= 4,
	esg_OpenFP				= 5,
	esg_WaitOpen			= 6,
	esg_AllowedNozzle		= 7,
	esg_Idle				= 8,
	esg_SetPrice			= 9,
	esg_AuthorizeNozzle		= 10,
	esg_Preset				= 11,
	esg_WaitStart			= 12,
	esg_Filling				= 13,
	esg_GetTransaction		= 14,
	esg_WaitPaying			= 15,
	esg_LockTransaction		= 16,
	esg_GetFinishCounters	= 17,
	esg_ClearTransaction	= 18,
	esg_Suspend				= 19,
	esg_Resume				= 20,
	esg_SetPrices			= 21,
}ExchangeStage;


typedef enum _IFSF_FPState
{
	ifsffps_Unknown					= 0x00,
	ifsffps_Inoperative				= 0x01,
	ifsffps_Closed					= 0x02,
	ifsffps_Idle					= 0x03,
	ifsffps_Calling					= 0x04,
	ifsffps_Authorized				= 0x05,
	ifsffps_Started					= 0x06,
	ifsffps_SuspendedStarted		= 0x07,
	ifsffps_Fuelling				= 0x08,
	ifsffps_SuspendedFuelling		= 0x09,

}IFSF_FPState;

typedef enum _IFSF_TRState
{
	ifsftrs_Unknown					= 0x00,
	ifsftrs_Cleared					= 0x01,
	ifsftrs_Payable					= 0x02,
	ifsftrs_Locked					= 0x03,

}IFSF_TRState;



//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct _Nozzle
{
	guint8			num;
	guint8			grade;
	guint32			price;
	guint32			counter;
	guint8			product_id;
}Nozzle;


typedef struct _Dispencer
{
	guint32				num;
	guint8				addr;
	guint8				nozzle_count;
	Nozzle				nozzles[MAX_NOZZLE_COUNT];

	guint8				node_index;

	DispencerState		dispencer_state;
	IFSF_FPState		original_state;
	IFSF_TRState		transaction_state;
	guint16				transaction_num;

	DriverCommand		current_command;

	ExchangeStage		exchange_stage;

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

	guint8				current_nozzle_index;

	gboolean			start;
	gboolean			is_pay;
	gboolean			reset;
	gboolean			send_prices;
	gboolean			suspend;
	gboolean			resume;

	guint8				error;

}Dispencer;

typedef enum _IFSFParseStage
{
	ifsfps_WaitComm					= 0,
	ifsfps_WaitLength				= 1,
	ifsfps_ReadData					= 11,

}IFSFParseStage;

typedef enum _IFSFParseIfsfStage
{
	ifsfpis_ReadTransportMC			= 0,
	ifsfpis_ReadDestSubnet			= 1,
	ifsfpis_ReadDestNode			= 2,
	ifsfpis_ReadSrcSubnet			= 3,
	ifsfpis_ReadSrcNode				= 4,
	ifsfpis_ReadMC					= 5,
	ifsfpis_ReadBL					= 6,
	ifsfpis_ReadStatus				= 7,
	ifsfpis_ReadLen1				= 8,
	ifsfpis_ReadLen2				= 9,
	ifsfpis_ReadLenDb				= 10,
	ifsfpis_ReadDb					= 11,

}IFSFParseIfsfStage;


typedef enum _IFSFMessageType
{
	ifsfmt_Read						= 0x00,
	ifsfmt_Answer					= 0x01,
	ifsfmt_Write					= 0x02,
	ifsfmt_UnsWAck					= 0x03,
	ifsfmt_Uns						= 0x04,
	ifsfmt_Ack						= 0x07,
}IFSFMessageType;


typedef enum _IFSFParseMessageStage
{
	ifsfpms_ReadId					= 0,
	ifsfpms_ReadLen					= 1,
	ifsfpms_ReadLenEx1				= 2,
	ifsfpms_ReadLenEx2				= 3,
	ifsfpms_ReadData				= 4,

}IFSFParseMessageStage;

typedef enum _IFSFMessageAcknowledgeStatus
{
	ifsfmas_Ack						= 0,
	ifsfmas_NodeNotReachable		= 1,
	ifsfmas_ApplOutOfOrder			= 2,
	ifsfmas_BlockMissing			= 3,
	ifsfmas_DataNotAcceptable		= 5,
	ifsfmas_UnknownDbAddress		= 6,
	ifsfmas_DeviceBusy				= 7,
	ifsfmas_MessageUnexpected		= 8,
	ifsfmas_DeviceAlreadyLocked		= 9,
}IFSFMessageAcknowledgeStatus;

typedef enum _IFSFDataAcknowledgeStatus
{
	ifsfdas_Ack						= 0,
	ifsfdas_InvalidValue			= 1,
	ifsfdas_NotWritable				= 2,
	ifsfdas_CommandRefused			= 3,
	ifsfdas_DataNotExist			= 4,
	ifsfdas_CommandNotUnderstood	= 5,
	ifsfdas_CommandNotAccepted		= 6,
}IFSFDataAcknowledgeStatus;



typedef enum _IFSFDataType
{
	ifsfdt_Undefined				= 0,
	ifsfdt_Binary					= 1,
	ifsfdt_Bcd						= 2,
	ifsfdt_AddrTable 				= 3,
	ifsfdt_Ascii 					= 4,
	ifsfdt_Volume 					= 5,
	ifsfdt_Amount 					= 6,
	ifsfdt_UnitPrice				= 7,
	ifsfdt_Temp						= 8,
	ifsfdt_Date						= 9,
	ifsfdt_LongVolume				= 10,
	ifsfdt_LongAmount				= 11,
	ifsfdt_LongNumber				= 12,
}IFSFDataType;

typedef enum _IFSFDatabaseId
{
	ifsfdbid_CommunicationService	= 0x00,
	ifsfdbid_Calculator				= 0x01,

	ifsfdbid_FuellingPointId1		= 0x21,
	ifsfdbid_FuellingPointId2		= 0x22,
	ifsfdbid_FuellingPointId3		= 0x23,
	ifsfdbid_FuellingPointId4		= 0x24,


	ifsfdbid_Product1				= 0x41,
	ifsfdbid_Product2				= 0x42,
	ifsfdbid_Product3				= 0x43,
	ifsfdbid_Product4				= 0x44,
	ifsfdbid_Product5				= 0x45,
	ifsfdbid_Product6				= 0x46,
	ifsfdbid_Product7				= 0x47,
	ifsfdbid_Product8				= 0x48,

	ifsfdbid_ProductData			= 0x61,

	ifsfdbid_Meter1					= 0x81,
	ifsfdbid_Meter2					= 0x82,
	ifsfdbid_Meter3					= 0x83,
	ifsfdbid_Meter4					= 0x84,
	ifsfdbid_Meter5					= 0x85,
	ifsfdbid_Meter6					= 0x86,
	ifsfdbid_Meter7					= 0x87,
	ifsfdbid_Meter8					= 0x88,
	ifsfdbid_Meter9					= 0x89,
	ifsfdbid_Meter10				= 0x8A,
	ifsfdbid_Meter11				= 0x8B,
	ifsfdbid_Meter12				= 0x8C,
	ifsfdbid_Meter13				= 0x8D,
	ifsfdbid_Meter14				= 0x8E,
	ifsfdbid_Meter15				= 0x8F,
	ifsfdbid_Meter16				= 0x90,
}IFSFDatabaseId;

typedef enum _IFSFNodeStage
{
	ifsfns_Offline					= 0,
	ifsfns_ConnectReady				= 1,
	ifsfns_TimeoutReady				= 2,
	ifsfns_CalculatorConfReady		= 3,
	ifsfns_ConfigReady				= 4,
	ifsfns_LoadProduct				= 5,
	ifsfns_SetProductParams			= 6,
	ifsfns_Online					= 7,
}IFSFNodeStage;

typedef enum _IFSFNodeExchangeState
{
	ifsfnes_Free					= 0,
	ifsfnes_SendCommand				= 1,
	ifsfnes_SendReply				= 2,
}IFSFNodeExchangeState;

typedef struct _IFSFNode
{
	IFSFNodeStage			stage;

	IFSFNodeExchangeState	exchange_state;

	guint8					transport_token;
	guint8					ifsf_token;

	guint8					reply_transport_token;
	guint8					reply_ifsf_token;

	guint64					last_heartbeat_time;

	guint8					product_count;
	guint8					fuelling_mode_count;
	guint8					fuelling_point_count;

	guint8					config_product_index;
	guint8					config_fuelling_mode_index;

	guint8					send_command_timeout;

	gint					send_price_disp_index;
}IFSFNode;


typedef struct _LonFrame
{
	guint8 				command;
	guint8 				length;
	guint8* 			data;
}LonFrame;

typedef struct _LonTransportFrame
{
	guint8 				flags;
	guint8 				format;
	guint8 				src_subnet;
	guint8 				src_node;
	guint8 				dest_subnet;
	guint8 				dest_node;
	guint8 				group;
	guint8 				token;
	guint8* 			data;
}LonTransportFrame;

typedef struct _IFSFDbAddress
{
	guint8				main_address;
	guint8				logial_nozzle_identifier;
	guint8				fuelling_point_detal;
	guint16				transaction_sequence_number;
	guint8				error_identifier;
	guint32				product_number;
	guint8				fuelling_mode_identifier;

}IFSFDbAddress;

typedef struct _IFSFMessage
{
	guint8				destination_subnet;
	guint8 				destination_node;
	guint8				source_subnet;;
	guint8				source_node;
	guint8				message_code;
	IFSFMessageType		message_type;
	guint8				token;
	guint16				message_length;
	guint8				db_address_length;
	IFSFDbAddress		db_address;
	guint8*				data;

}IFSFMessage;



typedef struct _IFSFNodeData
{
	guint8				buffer[IFSF_BUFFER_SIZE];
	guint8				buffer_length;

	guint8 				dest_subnet;
	guint8	 			dest_node;
	guint8 				src_subnet;
	guint8 				src_node;
	guint8 				message_code;
	guint8	 			bl;
	IFSFMessageType 	message_type;
	guint8 				token;
	guint16				message_length;
	guint8				address_length;
	IFSFDbAddress		database_address;

}IFSFNodeData;


typedef struct _IFSFNodesData
{
	IFSFNodeData		units[MAX_EXCHANGE_NODE_COUNT];
}IFSFNodesData;

ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* TOKHEIM_H_ */
