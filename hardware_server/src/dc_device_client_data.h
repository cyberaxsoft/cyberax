#ifndef DC_DEVICE_CLIENT_DATA_H_
#define DC_DEVICE_CLIENT_DATA_H_

void dc_client_init();
guint8 get_client_dci(guint8 index);
ClientState get_dc_client_state(guint8 index);
void set_dc_client_sock(guint8 index, gint32 sock);
void set_dc_clients_status_is_destroying();
void set_dc_device_status_is_changed_for_all_clients(guint8 dci, gboolean value, DcDeviceStatus status );
gboolean get_dc_device_status_is_changed_for_client(guint8 client_index);
void set_dc_device_status_is_changed_for_client(guint8 client_index, gboolean value);
void set_dc_disp_status_is_changed_for_all_clients(guint8 dci, guint8 index_disp, gboolean value);
void get_dc_client_command_result(guint8 index_client, guint8 index_disp, guint32* message_id, HardwareServerCommand* command, DcDeviceError* device_error);
gint8 find_dc_client_command(guint8 dci, guint8 index_disp, HardwareServerCommand* command, guint32* disp_num, guint8* nozzle_num, guint32* price, guint32* volume, guint32* amount,
		guint8* index_ext_func, DCPricePacks* price_packs);
guint8 get_client_dispencer_info_count( guint8 client_index );
guint8 get_client_dispencer_status_is_changed( guint8 client_index, guint8 disp_index);
void set_client_dispencer_status_is_changed(guint8 client_index, guint8 disp_index, gboolean value);
guint32 get_client_dispencer_num(guint8 client_index, guint8 disp_index);
CommandState get_client_dispencer_command_state(guint8 client_index, guint8 disp_index);
void set_client_dispencer_command_state(guint8 client_index, guint8 disp_index, CommandState value);
gint8 find_new_dc_client_index(gboolean clients_filtering);
gboolean get_dc_client_logged(guint8 client_index);
void set_dc_client_logged(guint8 client_index, gboolean new_value);
guint8 get_dc_client_access_level_param(guint8 client_index);
void set_dc_client_access_level_param(guint8 client_index, guint8 new_value);
gboolean clear_dc_client_command(guint8 client_index, guint8 disp_index);
ExchangeError set_dc_client_command( guint8 client_index, guint8 disp_index, HardwareServerCommand command, guint32 message_id, guint8 nozzle_num, guint32 price,
		guint32 volume ,guint32 amount, guint8 index_ext_func, DCPricePacks price_packs);
void set_new_dc_client_param( guint8 client_index, guint8 dci);
void destroy_dc_client( guint8 client_index);
gboolean get_dc_client_read_thread_is_active(guint8 index_client);
void set_dc_client_read_thread_is_active(guint8 index_client, gboolean value);
void set_dc_client_command_result(guint8 index_client, guint8 index_disp, DcDeviceError result);






#endif /* DC_DEVICE_CLIENT_DATA_H_ */
