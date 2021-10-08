#ifndef TGS_DEVICE_DRIVER_FUNC_H_
#define TGS_DEVICE_DRIVER_FUNC_H_

void set_tank_config_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi, LogParams* log_params, gboolean log_trace);
void set_tgs_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi);
void set_tank_state_from_driver(void* handle_lib, gchar* device_name, guint8 tgsi, guint8  index_tank, LogParams* log_params, gboolean log_trace);


#endif /* TGS_DEVICE_DRIVER_FUNC_H_ */
