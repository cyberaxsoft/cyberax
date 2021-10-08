#ifndef PPC_DEVICE_DRIVER_FUNC_H_
#define PPC_DEVICE_DRIVER_FUNC_H_

void set_ppc_dp_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace );
void set_price_pole_config_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace);
void set_ppc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace);
void set_price_pole_state_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, guint8  index_price_pole, LogParams* log_params, gboolean log_trace);
PpcDeviceError set_price_to_driver(void* handle_lib, gchar* device_name, guint8 ppci, PPCPricePacks price_packs, LogParams* log_params, gboolean log_trace);

#endif
