#ifndef PPC_DEVICE_CLIENT_DATA_H_
#define PPC_DEVICE_CLIENT_DATA_H_

void ppc_client_init();
guint8 get_client_ppci(guint8 index);
ClientState get_ppc_client_state(guint8 index);
void set_ppc_client_sock(guint8 index, gint32 sock);
void set_ppc_clients_status_is_destroying();
void set_ppc_device_status_is_changed_for_all_clients(guint8 ppci, gboolean value, PpcDeviceStatus status );
gboolean get_ppc_device_status_is_changed_for_client(guint8 client_index);
void set_ppc_device_status_is_changed_for_client(guint8 client_index, gboolean value);
gint8 find_new_ppc_client_index(gboolean clients_filtering);
gboolean get_ppc_client_logged(guint8 client_index);
void set_ppc_client_logged(guint8 client_index, gboolean new_value);
guint8 get_ppc_client_access_level_param(guint8 client_index);
void set_ppc_client_access_level_param(guint8 client_index, guint8 new_value);
void set_new_ppc_client_param( guint8 client_index, guint8 ppci);
guint8 get_client_price_pole_info_count( guint8 client_index );
guint8 get_client_price_pole_status_is_changed( guint8 client_index);
void set_client_price_pole_status_is_changed(guint8 client_index, gboolean value);
guint32 get_client_price_pole_num(guint8 client_index, guint8 price_pole_index);
void destroy_ppc_client( guint8 client_index);
gboolean get_ppc_client_read_thread_is_active(guint8 index_client);
void set_ppc_client_read_thread_is_active(guint8 index_client, gboolean value);

gint8 find_ppc_client_command(guint8 ppci, HardwareServerCommand* command, PPCPricePacks* price_packs);
void set_ppc_client_command_result(guint8 index_client, PpcDeviceError result);
void set_ppc_price_pole_status_is_changed_for_all_clients(guint8 ppci, gboolean value);
CommandState get_client_price_pole_controller_command_state(guint8 client_index);
void get_ppc_client_command_result(guint8 index_client, guint32* message_id, HardwareServerCommand* command, PpcDeviceError* device_error);
gboolean clear_ppc_client_command(guint8 client_index);
ExchangeError set_ppc_client_command( guint8 client_index, HardwareServerCommand command, guint32 message_id,  PPCPricePacks price_packs);
#endif /* PPC_DEVICE_CLIENT_DATA_H_ */
