#ifndef SC_DEVICE_DATA_H_
#define SC_DEVICE_DATA_H_

void sc_init();

ThreadStatus get_sc_main_sock_thread_status(guint8 index);
void set_sc_main_sock_thread_status(guint8 index, ThreadStatus status);
gint32 get_sc_sock(guint8 index);
void set_sc_sock_status(guint8 index, SocketStatus sock_status);
SocketStatus get_sc_sock_status(guint8 index);
gboolean get_new_sc_index(guint8* index);
void set_sc_device_is_working(guint8 index, gboolean value);
gboolean get_sc_device_is_working(guint8 index);
void set_sc_device_status(guint8 index, ScDeviceStatus status);
ScDeviceStatus get_sc_device_status(guint8 index);
void set_sc_device_last_error(guint8 index, ScDeviceError error);
ScDeviceError get_sc_device_last_erorr(guint8 index);
void set_sc_device_status_is_changed(guint8 index, gboolean value);
gboolean get_sc_device_status_is_changed(guint8 index);
gboolean get_sc_sensor_data(guint8 index, guint8 sensor_index, guint32* num, SensorParams* params, gboolean* online);
void set_sc_sensor_data(guint8 index, guint8 sensor_index, SensorParams* params, gboolean online );
void set_sc_device_name(guint8 index, gchar* name);
gchar* get_sc_device_name(guint8 index);
void set_sc_sock(guint8 index, gint32 sock);
gboolean init_sc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days);
void get_sc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace);
guint32 get_sc_sensor_num(guint8 index, guint8 sensor_index);
gboolean sc_sensor_num_is_present(guint8 index, guint32 sensor_num, guint8* sensor_index);
gboolean get_sc_sensor_data_is_change(guint8 index, guint8 sensor_index);
void set_sc_sensor_data_is_change(guint8 index, guint8 sensor_index, gboolean value);
void set_sc_device_port(guint8 index, guint32 port);
gint32 get_sc_device_port(guint8 index);
void set_sc_sensor_count(guint8 index, guint8 count);
guint8 get_sc_sensor_count(guint8 index);
void set_sc_sensor_info(guint8 index, guint8 sensor_index, guint32 num, guint8 addr);
void get_sc_sensor_info(guint8 index, guint8 sensor_index, guint32* num, guint8* addr, SensorParams* params );


#endif /* SC_DEVICE_DATA_H_ */
