

#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();
void safe_set_start_driver_state(LibConfig* configuration);

void safe_set_active_tank_index(guint8 new_value);
guint8 safe_get_active_tank_index();
void safe_increment_active_tank_index();

void safe_set_last_sended_tank_index(guint8 tank_index);
guint8 safe_get_last_sended_tank_index();

guint8 safe_get_tank_count();
DriverError safe_get_tank_channel(guint8 tank_index, guint8* tank_channel);
gboolean safe_get_tank_online(guint8 tank_index);
void safe_set_tank_online(guint8 tank_index, gboolean new_value);

guint8 safe_get_uart_error_counter(guint8 tank_index);
void safe_set_uart_error_counter(guint8 tank_index, guint8 new_value);
void safe_increment_uart_error_counter(guint8 tank_index);

TankExchangeState safe_get_exchange_state(guint8 tank_index);
void safe_set_exchange_state(guint8 tank_index, TankExchangeState new_value);

void safe_set_tank_volume(guint8 tank_index, gfloat new_value);
void safe_set_tank_weight(guint8 tank_index, gfloat new_value);
void safe_set_tank_density(guint8 tank_index, gfloat new_value);
void safe_set_tank_height(guint8 tank_index, gfloat new_value);
void safe_set_tank_water(guint8 tank_index, gfloat new_value);
void safe_set_tank_temperature(guint8 tank_index, gfloat new_value);

guint8 safe_get_sending_error_counter(guint8 tank_index);
void safe_set_sending_error_counter(guint8 tank_index, guint8 new_value);
void safe_increment_sending_error_counter(guint8 tank_index);

DriverError safe_get_tank_data(guint8 tank_index, gfloat* height, gfloat* volume, gfloat* density, gfloat* weight, gfloat* temperature, gfloat* water_level, gboolean* online);

#endif /* DRIVER_STATE_H_ */
