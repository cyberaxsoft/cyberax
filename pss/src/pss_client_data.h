#ifndef PSS_CLIENT_DATA_H_
#define PSS_CLIENT_DATA_H_

#define	 PSS_MAX_CLIENT_COUNT		255

typedef enum _PSSClientState
{
	cs_Free								= 0x00,
	cs_Busy								= 0x01,
	cs_Active							= 0x02,
	cs_Destroying						= 0x03,
}PSSClientState;

typedef enum _PSSClientExchangeState
{
	ces_Undefined						= 0x00,
	ces_Logged							= 0x01,
	ces_FcStatus						= 0x02,
	ces_FcInstallStatus					= 0x03,
	ces_FpStatus						= 0x04,
	ces_FpSupTransBufStatus				= 0x05,
	ces_TgsStatus						= 0x06,
	ces_FcPriceSetStatus				= 0x07,
	ces_FcOperationModeStatus			= 0x08,
	ces_FcPosConnectionStatus			= 0x09,
	ces_DoorSwitch						= 0x0A,
	ces_Idle							= 0x0B,
}PSSClientExchangeState;

typedef enum _PSSClientType
{
	ct_Undefined						= 0x00,
	ct_FPControl						= 0x01,
	ct_TGSControl						= 0x02,
}PSSClientType;

typedef struct _PSSClientDevice
{
	PSSServerDeviceType			device_type;
	gint32 						sock;
	guint8						id_device;
	guint16						port;
	gchar*						ip_address;
	GThread* 					thread;
	ThreadStatus				thread_status;
	guint32						message_id;
	PSSServerDeviceUnits 		unit_ids;

	HardwareServerCommand		current_command;
	guint64						current_command_sent_time;
	PSSCommand					pss_command;
	guint8						pss_command_subcode;
	gboolean					ex_mode;

}PSSClientDevice;

typedef struct _PSSClientInfo
{
	guint32						port;
	gint32 						sock;
	guint32						ip_address;
	PSSClientState				state;

	guint64						last_exchange_time;

	PSSClientExchangeState		exchange_status;
	PSSClientType				type;
	GMutex						mutex;
	guint8						fp_status_mode;
	guint8						tgs_status_mode;
	guint8						tr_buf_status_mode;

	PSSClientDevice 			devices[MAX_SERVER_DEVICE_COUNT];
}PSSClientInfo;


void clients_init();

guint32 get_client_port(guint8 client_index);
guint8 get_client_fp_status_mode(guint8 client_index);
guint8 get_client_tgs_status_mode(guint8 client_index);
guint8 get_client_tr_buf_status_mode(guint8 client_index);
void set_client_fp_status_mode(guint8 client_index, guint8 new_status_mode);
void set_client_tgs_status_mode(guint8 client_index, guint8 new_status_mode);
void set_client_tr_buf_status_mode(guint8 client_index, guint8 new_status_mode);
guint32 get_client_device_sock(guint8 client_index, guint8 device_index);
guint32 get_client_device_message_id(guint8 client_index, guint8 device_index);
void increment_client_device_message_id(guint8 client_index, guint8 device_index);
gboolean get_client_device_index_by_id(guint8 client_index, guint8 id, guint8* device_index, PSSServerDeviceType device_type);
PSSServerDeviceType get_client_device_type_by_index(guint8 client_index, guint8 device_index);
ThreadStatus get_client_device_thread_status(guint8 client_index, guint8 device_index);
void free_client_device_thread(guint8 client_index, guint8 device_index);
void set_client_device_thread_status(guint8 client_index, guint8 device_index, ThreadStatus new_value);
void set_client_port(guint8 client_index, guint32 new_value);
PSSClientState get_client_state(guint8 client_index);
void set_client_state(guint8 client_index, PSSClientState new_value);
void clear_client(guint8 client_index);
guint8 find_next_client_index();
PSSClientExchangeState get_client_exchange_status(guint8 client_index);
void set_client_exchange_status(guint8 client_index, PSSClientExchangeState new_value);
guint64 get_client_last_exchange_time(guint8 client_index);
void set_client_last_exchange_time(guint8 client_index, guint64 new_value);
PSSClientType get_client_type(guint8 client_index);
void set_client_type(guint8 client_index, PSSClientType new_value);
void socket_client_send_with_mutex(PSSClientThreadFuncParam* params, guchar* buffer, guint32 size);
void socket_client_device_send_with_mutex(PSSClientThreadFuncParam* params, guint8 device_index, guchar* buffer, guint32 size);
void client_socket_close(guint8 client_index);
guint32 get_client_ip_address(guint8 client_index);
void set_client_ip_address(guint8 client_index, guint32 new_value);


gint32 reconnect_to_server_device(PSSClientThreadFuncParam* client_params, guint8 device_index);
void connect_to_server_device(PSSClientThreadFuncParam* param, guint8 device_index, PSSServerDevice* server_device);
void disconnect_server_devices(PSSClientThreadFuncParam* param);
void connect_to_server_devices(PSSClientThreadFuncParam* client_params);

guint32 get_client_sock(guint8 client_index);
void set_client_sock(guint8 client_index, guint32 new_value);


HardwareServerCommand get_current_command(guint8 client_index, guint8 device_index);
void set_current_command(guint8 client_index, guint8 device_index, HardwareServerCommand new_value);

guint64 get_current_command_sent_time(guint8 client_index, guint8 device_index);
void set_current_command_sent_time(guint8 client_index, guint8 device_index, guint64 new_value);

PSSCommand get_pss_command(guint8 client_index, guint8 device_index);
void set_pss_command(guint8 client_index, guint8 device_index, PSSCommand new_value);

guint8 get_pss_command_subcode(guint8 client_index, guint8 device_index);
void set_pss_command_subcode(guint8 client_index, guint8 device_index, guint8 new_value);
guint8 get_pss_ex_mode(guint8 client_index, guint8 device_index);
void set_pss_ex_mode(guint8 client_index, guint8 device_index, gboolean new_value);
#endif /* PSS_CLIENT_DATA_H_ */
