#ifndef SC_DEVICE_CLIENT_DATA_H_
#define SC_DEVICE_CLIENT_DATA_H_

void sc_client_init();

guint8 get_client_sci(guint8 index);
ClientState get_sc_client_state(guint8 index);
void set_sc_client_sock(guint8 index, gint32 sock);
void set_sc_clients_status_is_destroying();

void set_sc_device_status_is_changed_for_all_clients(guint8 sci, gboolean value, ScDeviceStatus status );
gboolean get_sc_device_status_is_changed_for_client(guint8 client_index);
void set_sc_device_status_is_changed_for_client(guint8 client_index, gboolean value);
gint8 find_new_sc_client_index(gboolean clients_filtering);
gboolean get_sc_client_logged(guint8 client_index);
void set_sc_client_logged(guint8 client_index, gboolean new_value);
guint8 get_sc_client_access_level_param(guint8 client_index);
void set_sc_client_access_level_param(guint8 client_index, guint8 new_value);
void set_new_sc_client_param( guint8 client_index, guint8 sci);
guint8 get_client_sensor_info_count( guint8 client_index );
guint8 get_client_sensor_status_is_changed( guint8 client_index, guint8 sensor_index);
void set_client_sensor_status_is_changed(guint8 client_index, guint8 sensor_index, gboolean value);
void set_sc_sensor_status_is_changed_for_all_clients(guint8 sci, guint8 index_sensor, gboolean value);
guint32 get_client_sensor_num(guint8 client_index, guint8 sensor_index);
void destroy_sc_client( guint8 client_index);
gboolean get_sc_client_read_thread_is_active(guint8 index_client);
void set_sc_client_read_thread_is_active(guint8 index_client, gboolean value);



#endif
