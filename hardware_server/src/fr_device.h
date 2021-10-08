
#ifndef FR_DEVICE_H_
#define FR_DEVICE_H_

#define	MAX_FR_COUNT	16


typedef enum _FrDeviceError
{
	fre_NoError					= 0x00,
	fre_WrongCommand			= 0x01,
	fre_ErrorConnection 		= 0x02,
	fre_ErrorConfiguration 		= 0x03,
	fre_ErrorLogging			= 0x04,
	fre_Undefined				= 0x05,
	fre_OutOfRange				= 0x06,
	fre_AlreadyInit				= 0x07,
	fre_NotInitializeDriver 	= 0x08,

}FrDeviceError;

typedef enum _FrDeviceStatus
{
	frs_NoError				= 0x00,
	frs_ErrorConnection		= 0x01,
	frs_ErrorConfiguration	= 0x02,
	frs_ErrorLogging		= 0x03,
	frs_UndefinedError		= 0x04,
	frs_NotInitializeDriver	= 0x05,
}FrDeviceStatus;

typedef struct _FrTask
{
	guint32					dispencer_number;
	guint8					nozzle_number;
	HardwareServerCommand	command;
	float					price;
	float					volume;
	float					sum;
	guint8					index_ext_func;
	guint32					message_id;

}FrTask;


typedef struct _FiscalRegister
{
	gchar*					name;
	gint32					port;
	gboolean				device_is_working;
	FrDeviceStatus			device_status;
	gboolean				device_status_is_changed;
	FrDeviceError			last_device_error;

	LogParams				log_params;
	gboolean				log_trace;

	gint32					sock;
	SocketStatus 			sock_status;
	ThreadStatus			main_sock_status;

}FiscalRegister;

typedef struct _FiscalRegisterClientInfo
{
//	guint32					num;
//
//	gboolean				status_is_change;
//	HardwareServerCommand	command;
//	DCPricePacks			param_price_packs;
//	guint8					param_nozzle_num;
//	guint32					param_price;
//	guint32					param_volume;
//	guint32					param_amount;
//	guint8					param_index_ext_func;
//
//
//	CommandState			command_state;
//	DcDeviceError			command_result;
//
	guint32					message_id;
}FiscalRegisterClientInfo;


typedef struct _FRClient
{
	guint8 						fiscal_register_index;
	ClientState					state;
	gboolean					device_status_is_change;
	gboolean					logged;
	guint8						access_level;
//	FiscalRegisterClientInfo	dispencer_info[MAX_DISPENCER_COUNT];
	gboolean					read_thread_is_active;
	gint32						sock;
}FRClient;

typedef struct _FRClientInfo
{
	guint8					client_index;
	gint32 					socket;
	GMutex					socket_mutex;
	guint8					fiscal_register_index;
	gchar* 					device_name;
	gchar* 					ip_address;

	LogParams*				log_params;

	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;

}FRClientInfo;

typedef enum _FiscalReceiptItemType
{
	frit_Article			= 0,
	frit_Text				= 1,
}FiscalReceiptItemType;

typedef enum _FiscalReceiptTextType
{
	frtt_Normal				= 0,
	frtt_Bold				= 1,
}FiscalReceiptTextType;

typedef enum _FiscalReceiptType
{
	frt_Sale				= 0,
	frt_ReturnSale			= 1,
}FiscalReceiptType;


typedef struct _FiscalReceiptItemArticle
{
	gchar*					code;
	gchar* 					name;
	guint32					price;
	guint32					amount;
	guint8					product_type;
	gchar*					marker;
	gchar*					description;
	gchar*					tax_code;

}FiscalReceiptItemArticle;

typedef struct _FiscalReceiptItemText
{
	FiscalReceiptTextType	type;
	guint8					size;
	gchar*					text;
}FiscalReceiptItemText;

typedef struct _FiscalReceiptItem
{
	FiscalReceiptItemType 	type;
	gpointer				item;
	gpointer				next;
}FiscalReceiptItem;

typedef struct _FiscalReceiptPayment
{
	gchar*					payment_code;
	guint32					sum;
	gpointer				next;
}FiscalReceiptPayment;


typedef struct _FiscalReceipt
{
	guint32 				income_cash_sum;
	guint8					payment_type;
	FiscalReceiptType		receipt_type;
	FiscalReceiptItem*		first_item;
	FiscalReceiptPayment*	first_payment;
}FiscalReceipt;

typedef struct _TextDoc
{
	guint32					num;
	gchar*					title;
	FiscalReceiptItemText*	first_item;
}TextDoc;

void set_fr_device_is_working(guint8 index, gboolean value);

gpointer fr_device_thread_func(gpointer data);


#endif /* FR_DEVICE_H_ */
