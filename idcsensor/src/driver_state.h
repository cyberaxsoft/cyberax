

#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();
void safe_set_start_driver_state(LibConfig* configuration);

guint8 safe_get_sensor_count();
DriverError safe_get_sensor_addr(guint8 sensor_index, guint8* sensor_addr);
gboolean safe_get_sensor_online(guint8 sensor_index);
void safe_set_sensor_online(guint8 sensor_index, gboolean new_value);
void safe_set_sensor_param(guint8 sensor_index, guint8 param_index, guint8* buffer);

DriverError safe_get_sensor_data(guint8 sensor_index, SensorParams* params, gboolean* online);

gboolean safe_get_command_sended();
void safe_set_command_sended(gboolean new_value);

void safe_set_all_sensor_online(gboolean new_value);

#endif /* DRIVER_STATE_H_ */
