#ifndef DC_DEVICE_DATA_H_
#define DC_DEVICE_DATA_H_

void dc_init();
ThreadStatus get_dc_main_sock_thread_status(guint8 index);
void set_dc_main_sock_thread_status(guint8 index, ThreadStatus status);
gint32 get_dc_sock(guint8 index);
void set_dc_sock_status(guint8 index, SocketStatus sock_status);
SocketStatus get_dc_sock_status(guint8 index);
gboolean get_new_dispencer_controller_index(guint8* index);
void set_dc_device_is_working(guint8 index, gboolean value);
gboolean get_dc_device_is_working(guint8 index);
void set_dc_device_status(guint8 index, DcDeviceStatus status);
DcDeviceStatus get_dc_device_status(guint8 index);
void set_dc_device_last_error(guint8 index, DcDeviceError error);
DcDeviceError get_dc_device_last_erorr(guint8 index);
void set_dc_device_status_is_changed(guint8 index, gboolean value);
gboolean get_dc_device_status_is_changed(guint8 index);
gboolean get_dc_dispencer_data(guint8 index, guint8 disp_index, guint32* num, DispencerState* state, OrderType*	order_type, OrderType* preset_order_type,
		gboolean* is_pay, guint8* preset_nozzle_num, guint8* active_nozzle_num, guint32* preset_price, guint32* preset_volume, guint32* preset_amount,
		guint32* current_price, guint32* current_volume, guint32* current_amount, guchar* error, gchar* error_description, guint8* error_description_length);
void set_dc_device_name(guint8 index, gchar* name);
gchar* get_dc_device_name(guint8 index);
void set_dc_sock(guint8 index, gint32 sock);
gboolean init_dc_log_settings(guint8 index, gchar* log_dir, gchar* device_name, gboolean log_enable, gboolean log_trace, guint32 file_size, guint32 save_days);
void get_dc_log_settings(guint8 index, LogParams** log_params, gboolean* log_trace);
gchar* get_dc_log_dir(guint8 index);
guint32 get_dc_dispencer_num(guint8 index, guint8 index_disp);
guint8 get_dc_dispencer_nozzle_count(guint8 index, guint8 index_disp);
gboolean dc_dispencer_num_is_present(guint8 index, guint32 disp_num, guint8* index_disp);
gboolean dc_nozzle_num_is_present(guint8 index, guint32 disp_num, guint8 nozzle_num, guint8* disp_index, guint8* nozzle_index);
gboolean get_dc_dispencer_data_is_change(guint8 index, guint8 index_disp);
void set_dc_dispencer_data_is_change(guint8 index, guint8 index_disp, gboolean value);
void set_dc_device_port(guint8 index, guint32 port);
gint32 get_dc_device_port(guint8 index);
void set_dc_decimal_pointers(guint8 index, guint8 price_dp, guint8 volume_dp, guint8 amount_dp);
void get_dc_decimal_pointers(guint8 index, guint8* price_dp, guint8* volume_dp, guint8* amount_dp);
void set_dc_ext_func_count(guint8 index, guint8 count);
guint8 get_dc_ext_func_count(guint8 index);
void set_dc_ext_func_name(guint8 index, guint8 func_index, gchar* name);
gchar* get_dc_ext_func_name(guint8 index, guint8 func_index);
void set_dc_disp_count(guint8 index, guint8 count);
guint8 get_dc_disp_count(guint8 index);
void set_dc_disp_info(guint8 index, guint8 index_disp, guint32 num, guint8 addr, guint8 nozzle_count );
void get_dc_disp_info(guint8 index, guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count );
void set_dc_nozzle_info(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8 num, guint8 grade );
void get_dc_nozzle_info(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8* num, guint8* grade );
void get_dc_nozzle_counter(guint8 index, guint8 index_disp,guint8 index_nozzle, guint8* nozzle_num, guint32* counter );
DispencerState get_dc_dispencer_state(guint8 index, guint8 disp_index);

void set_dc_dispencer_data(guint8 dci, guint8 disp_index, gchar* device_name, guint32 disp_num, DcDeviceError last_device_error, DispencerState disp_state,
							OrderType preset_order_type, guint8 preset_nozzle_num, guint32 preset_price, guint32 preset_volume, guint32 preset_amount,
							OrderType	order_type,  guint8 active_nozzle_num, guint32 current_price, guint32 current_volume, guint32 current_amount,
							gboolean is_pay, guchar error, guchar* error_description, gboolean counters_ready, guint8 nozzle_count, guint32 counters[MAX_NOZZLE_COUNT], LogParams* log_params, gboolean log_trace );



#endif /* DC_DEVICE_DATA_H_ */
