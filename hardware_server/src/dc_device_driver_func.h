#ifndef DC_DEVICE_DRIVER_FUNC_H_
#define DC_DEVICE_DRIVER_FUNC_H_

void set_dc_dp_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace );
void set_ext_funcs_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace);
void set_dispencers_config_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace);
void set_dc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 dci);
DcDeviceError get_counter_from_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32* counter );
DcDeviceError send_reset_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_update_prices_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, DCPricePacks* price_packs);
DcDeviceError send_start_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_stop_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_suspend_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_resume_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_payment_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num);
DcDeviceError send_volume_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 volume);
DcDeviceError send_sum_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 amount);
DcDeviceError send_full_tank_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price);
DcDeviceError send_extended_func_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 func_index);
void set_dispencer_state_from_driver(void* handle_lib, gchar* device_name, guint8 dci, guint8  index_disp, LogParams* log_params, gboolean log_trace);


#endif /* DC_DEVICE_DRIVER_FUNC_H_ */
