#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();

void safe_set_driver_state_from_settings(LibConfig* configuration);

guint8 safe_get_current_price_pole_index();
void safe_set_current_price_pole_index(guint8 new_value);
void safe_inc_current_price_pole_index();

guint32 safe_get_price(guint8 price_pole_index);
void safe_set_price(guint8 price_pole_index, guint32 new_value);
void safe_set_full_price(guint8 grade, guint32 new_value);

PricePoleState safe_get_state(guint8 price_pole_index);
void safe_set_state(guint8 price_pole_index, PricePoleState new_value);

VdsExchangeState safe_get_exchange_state(guint8 price_pole_index);
void safe_set_exchange_state(guint8 price_pole_index, VdsExchangeState new_value);

guint8 safe_get_uart_error(guint8 price_pole_index);
void safe_set_uart_error(guint8 price_pole_index, guint8 new_value);
void safe_inc_uart_error(guint8 price_pole_index);

gboolean safe_get_command_sent(guint8 price_pole_index);
void safe_set_command_sent(guint8 price_pole_index, gboolean new_value);

#endif /* DRIVER_STATE_H_ */
