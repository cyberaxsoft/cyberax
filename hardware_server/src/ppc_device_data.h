#ifndef PPC_DEVICE_DATA_H_
#define PPC_DEVICE_DATA_H_

#define	MAX_PRICE_POLE_CONTROL_COUNT	16

void ppc_init();
ThreadStatus get_ppc_main_sock_thread_status(guint8 index);
void set_ppc_main_sock_thread_status(guint8 index, ThreadStatus status);
gint32 get_ppc_sock(guint8 index);
void set_ppc_sock_status(guint8 index, SocketStatus sock_status);
SocketStatus get_ppc_sock_status(guint8 index);
gboolean get_new_ppc_index(guint8* index);
void set_ppc_device_is_working(guint8 index, gboolean value);
gboolean get_ppc_device_is_working(guint8 index);
void set_ppc_device_status(guint8 index, PpcDeviceStatus status);
PpcDeviceStatus get_ppc_device_status(guint8 index);
void set_ppc_device_last_error(guint8 index, PpcDeviceError error);
PpcDeviceError get_ppc_device_last_erorr(guint8 index);
void set_ppc_device_status_is_changed(guint8 index, gboolean value);
gboolean get_ppc_device_status_is_changed(guint8 index);
gboolean get_ppc_price_pole_data(guint8 index, guint8 price_pole_index, guint8* num, guint8* grade, guint32* price, PricePoleState* state );
void set_ppc_price_pole_data(guint8 index, guint8 price_pole_index, guint32 price, PricePoleState state );
void set_ppc_device_name(guint8 index, gchar* name);
gchar* get_ppc_device_name(guint8 index);
void set_ppc_sock(guint8 index, gint32 sock);
gboolean init_ppc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days);
void get_ppc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace);
guint8 get_ppc_price_pole_num(guint8 index, guint8 index_price_pole);
gboolean ppc_price_pole_num_is_present(guint8 index, guint8 price_pole_num, guint8* index_price_pole);
gboolean get_ppc_price_pole_data_is_change(guint8 index);
void set_ppc_price_pole_data_is_change(guint8 index, gboolean value);
void set_ppc_device_port(guint8 index, guint32 port);
gint32 get_ppc_device_port(guint8 index);
void set_ppc_price_pole_count(guint8 index, guint8 count);
guint8 get_ppc_price_pole_count(guint8 index);
void set_ppc_price_pole_info(guint8 index, guint8 index_price_pole, guint8 num, guint8 grade, guint8 symbol_count);
void get_ppc_price_pole_info(guint8 index, guint8 index_price_pole, guint8* num, guint8* grade, guint8* symbol_count);

void set_ppc_decimal_pointers(guint8 index, guint8 price_dp);
void get_ppc_decimal_pointers(guint8 index, guint8* price_dp);

#endif /* PPC_DEVICE_DATA_H_ */
