
#ifndef FR_DEVICE_DATA_H_
#define FR_DEVICE_DATA_H_

void fr_init();

gboolean get_new_fiscal_register_index(guint8* index);
gboolean init_fr_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days);
void get_fr_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace);
void set_fr_device_status(guint8 index, FrDeviceStatus status);
void set_fr_device_name(guint8 index, gchar* name);
void set_fr_device_port(guint8 index, guint32 port);

void set_fr_device_is_working(guint8 index, gboolean value);
gboolean get_fr_device_is_working(guint8 index);

void set_fr_device_last_error(guint8 index, FrDeviceError error);
FrDeviceError get_fr_device_last_erorr(guint8 index);

FrDeviceStatus get_fr_device_status(guint8 index);

ThreadStatus get_fr_main_sock_thread_status(guint8 index);
void set_fr_main_sock_thread_status(guint8 index, ThreadStatus status);
gint32 get_fr_sock(guint8 index);
void set_fr_sock_status(guint8 index, SocketStatus sock_status);
SocketStatus get_fr_sock_status(guint8 index);
gboolean get_fr_device_status_is_changed(guint8 index);
void set_fr_device_status_is_changed(guint8 index, gboolean value);


#endif /* FR_DEVICE_DATA_H_ */
