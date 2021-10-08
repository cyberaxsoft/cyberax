

#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();

ExchangeState safe_get_exchange_state();
void safe_set_exchange_state(ExchangeState new_value);

#endif /* DRIVER_STATE_H_ */
