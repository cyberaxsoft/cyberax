

#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();

void safe_set_driver_state_from_settings(LibConfig* configuration);

void safe_get_current_nozzle(guint8* disp_index, guint8* nozzle_index);

gboolean safe_get_command_send(guint8 disp_index, guint8 nozzle_index);
void safe_set_command_send(guint8 disp_index, guint8 nozzle_index, gboolean new_value);

void safe_set_channel_exchange_state(guint8 disp_index, guint8 nozzle_index, ChannelExchangeState new_value);
ChannelExchangeState safe_get_channel_exchange_state(guint8 disp_index, guint8 nozzle_index);


void safe_set_current_disp_index(guint8 new_value);
guint8 safe_get_current_disp_index();
void safe_increment_current_disp_index();

void safe_get_preset(guint8 disp_index, gint8* nozzle_index, guint32* price, guint32* volume, guint32* amount);
guint8 safe_get_nozzle_num(guint8 disp_index, guint8 nozzle_index);
guint8 safe_get_nozzle_count(guint8 disp_index);
guint32 safe_get_nozzle_price(guint8 disp_index, guint8 nozzle_index);
void safe_set_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32 value);
DriverError safe_get_disp_addr(guint8 disp_index, guint8* disp_addr);

DriverCommand safe_get_current_command(guint8 disp_index);
void safe_set_current_command(guint8 disp_index, DriverCommand new_value);

DispencerState safe_get_dispencer_state(guint8 disp_index);
void safe_set_dispencer_state(guint8 disp_index, DispencerState new_value);

gboolean safe_get_get_counters(guint8 disp_index);
void safe_set_get_counters(guint8 disp_index, gboolean new_value);

gboolean safe_get_emergency_stop(guint8 disp_index);
void safe_set_emergency_stop(guint8 disp_index, gboolean new_value);

gboolean safe_get_suspend(guint8 disp_index);
void safe_set_suspend(guint8 disp_index, gboolean new_value);

gboolean safe_get_resume(guint8 disp_index);
void safe_set_resume(guint8 disp_index, gboolean new_value);

gboolean safe_get_reset(guint8 disp_index);
void safe_set_reset(guint8 disp_index, gboolean new_value);

void safe_set_prices(guint8 disp_index, guint32 price1, guint32 price2, guint32 price3, guint32 price4, guint32 price5, guint32 price6, guint32 price7, guint32 price8);

gboolean safe_get_is_pay(guint8 disp_index);
void safe_set_is_pay(guint8 disp_index, gboolean new_value);

guint32 safe_get_current_volume(guint8 disp_index);
void safe_set_current_volume(guint8 disp_index, guint32 new_value);

guint32 safe_get_current_amount(guint8 disp_index);
void safe_set_current_amount(guint8 disp_index, guint32 new_value);

guint32 safe_get_current_price(guint8 disp_index);
void safe_set_current_price(guint8 disp_index, guint32 new_value);

gboolean safe_compare_disp_values(guint8 disp_index);

guint8 safe_get_error(guint8 disp_index);
void safe_set_error(guint8 disp_index, guint8 new_value);
void safe_set_active_nozzle_index(guint8 disp_index, gint8 new_value);

guint8 safe_get_preset_nozzle_num(guint8 disp_index);
guint8 safe_get_active_nozzle_num(guint8 disp_index);

gboolean safe_get_nozzle_was_on(guint8 disp_index);
void safe_set_nozzle_was_on(guint8 disp_index, gboolean new_value);

DriverError safe_get_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32* counter);
DriverError safe_get_nozzle_counter_by_nums(guint32 disp_num, guint8 nozzle_num, guint32* counter);
OrderType safe_get_preset_order_type(guint8 disp_index);
void safe_set_nozzle_price(guint8 disp_index, guint8 nozzle_index, guint32 value);
void safe_set_preset(guint8 disp_index, gint8 nozzle_index, guint32 price, guint32 volume, guint32 amount, OrderType order_type);
DriverError safe_set_disp_preset(guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 volume, guint32 amount, OrderType order_type);
gint8 safe_get_preset_nozzle_index(guint8 disp_index);

gboolean safe_get_disp_start(guint8 disp_index);
void safe_set_disp_start(guint8 disp_index, gboolean new_value);

gint8 safe_get_active_nozzle_index(guint8 disp_index);

void safe_disp_clear(guint8 disp_index);
void safe_disp_reset(guint8 disp_index);

guchar safe_get_disp_state(guint8 disp_index, guint8* disp_state, guint8* preset_order_type, guint8* preset_nozzle_num,
		guint32* preset_price, guint32* preset_volume, guint32* preset_amount, guint8* order_type, guint8* active_nozzle_num,
		guint32* current_price, guint32* current_volume, guint32* current_amount,
		guint8* is_pay, guint8* error, guchar* error_description);

void safe_copy_disp_order_type(guint8 disp_index);
OrderType safe_get_order_type(guint8 disp_index);

void safe_store_last_indexes(guint8 disp_index, guint8 nozzle_index);
void safe_restore_last_indexes(guint8* disp_index, guint8* nozzle_index);

void safe_change_channel();


void safe_set_nozzle_param(guint8 disp_index, guint8 nozzle_index, guint8 volume_length, guint8 price_length, guint8 amount_length);
void safe_get_nozzle_param(guint8 disp_index, guint8 nozzle_index, guint8* volume_length, guint8* price_length, guint8* amount_length);

void safe_set_nozzle_version(guint8 disp_index, guint8 nozzle_index, guint32 new_value);
guint32 safe_get_nozzle_version(guint8 disp_index, guint8 nozzle_index);

#endif /* DRIVER_STATE_H_ */
