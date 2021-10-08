#ifndef TGS_DEVICE_CLIENT_DATA_H_
#define TGS_DEVICE_CLIENT_DATA_H_

void tgs_client_init();

guint8 get_client_tgsi(guint8 index);
ClientState get_tgs_client_state(guint8 index);
void set_tgs_client_sock(guint8 index, gint32 sock);
void set_tgs_clients_status_is_destroying();

void set_tgs_device_status_is_changed_for_all_clients(guint8 tgsi, gboolean value, TgsDeviceStatus status );
gboolean get_tgs_device_status_is_changed_for_client(guint8 client_index);
void set_tgs_device_status_is_changed_for_client(guint8 client_index, gboolean value);
gint8 find_new_tgs_client_index(gboolean clients_filtering);
gboolean get_tgs_client_logged(guint8 client_index);
void set_tgs_client_logged(guint8 client_index, gboolean new_value);
guint8 get_tgs_client_access_level_param(guint8 client_index);
void set_tgs_client_access_level_param(guint8 client_index, guint8 new_value);
void set_new_tgs_client_param( guint8 client_index, guint8 tgsi);
guint8 get_client_tank_info_count( guint8 client_index );
guint8 get_client_tank_status_is_changed( guint8 client_index, guint8 tank_index);
void set_client_tank_status_is_changed(guint8 client_index, guint8 tank_index, gboolean value);
guint32 get_client_tank_num(guint8 client_index, guint8 tank_index);
void destroy_tgs_client( guint8 client_index);
gboolean get_tgs_client_read_thread_is_active(guint8 index_client);
void set_tgs_client_read_thread_is_active(guint8 index_client, gboolean value);


#endif /* TGS_DEVICE_CLIENT_DATA_H_ */
