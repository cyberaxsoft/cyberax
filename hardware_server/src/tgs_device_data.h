#ifndef TGS_DEVICE_DATA_H_
#define TGS_DEVICE_DATA_H_

void tgs_init();

ThreadStatus get_tgs_main_sock_thread_status(guint8 index);
void set_tgs_main_sock_thread_status(guint8 index, ThreadStatus status);
gint32 get_tgs_sock(guint8 index);
void set_tgs_sock_status(guint8 index, SocketStatus sock_status);
SocketStatus get_tgs_sock_status(guint8 index);

gboolean get_new_tgs_index(guint8* index);
void set_tgs_device_is_working(guint8 index, gboolean value);
gboolean get_tgs_device_is_working(guint8 index);
void set_tgs_device_status(guint8 index, TgsDeviceStatus status);
TgsDeviceStatus get_tgs_device_status(guint8 index);
void set_tgs_device_last_error(guint8 index, TgsDeviceError error);
TgsDeviceError get_tgs_device_last_erorr(guint8 index);
void set_tgs_device_status_is_changed(guint8 index, gboolean value);
gboolean get_tgs_device_status_is_changed(guint8 index);
gboolean get_tgs_tank_data(guint8 index, guint8 tank_index, guint32* num, gfloat* height, gfloat* volume, gfloat* weight, gfloat* density, gfloat* temperature, gfloat* water_level, gboolean* online );
void set_tgs_tank_data(guint8 index, guint8 tank_index, gfloat height, gfloat volume, gfloat weight, gfloat density, gfloat temperature, gfloat water_level, gboolean online );
void set_tgs_device_name(guint8 index, gchar* name);
gchar* get_tgs_device_name(guint8 index);
void set_tgs_sock(guint8 index, gint32 sock);

gboolean init_tgs_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days);
void get_tgs_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace);
guint32 get_tgs_tank_num(guint8 index, guint8 index_tank);
gboolean tgs_tank_num_is_present(guint8 index, guint32 tank_num, guint8* index_tank);
gboolean get_tgs_tank_data_is_change(guint8 index, guint8 index_tank);
void set_tgs_tank_data_is_change(guint8 index, guint8 index_tank, gboolean value);
void set_tgs_device_port(guint8 index, guint32 port);
gint32 get_tgs_device_port(guint8 index);
void set_tgs_tank_count(guint8 index, guint8 count);
guint8 get_tgs_tank_count(guint8 index);
void set_tgs_tank_info(guint8 index, guint8 index_tank, guint32 num, guint8 channel );
void get_tgs_tank_info(guint8 index, guint8 index_tank, guint32* num, guint8* channel );


#endif /* TGS_DEVICE_DATA_H_ */
