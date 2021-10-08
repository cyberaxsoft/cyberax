#ifndef SC_DEVICE_DRIVER_FUNC_H_
#define SC_DEVICE_DRIVER_FUNC_H_

void set_sensor_config_from_driver(void* handle_lib, gchar* device_name, guint8 sci, LogParams* log_params, gboolean log_trace);
void set_sc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 sci);
void set_sensor_state_from_driver(void* handle_lib, gchar* device_name, guint8 sci, guint8  sensor_index, LogParams* log_params, gboolean log_trace);

#endif /* SC_DEVICE_DRIVER_FUNC_H_ */
