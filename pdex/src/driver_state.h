

#ifndef DRIVER_STATE_H_
#define DRIVER_STATE_H_

void init_driver_mutex();


void safe_set_driver_state_from_settings(LibConfig* configuration);

guint8 safe_set_reply_length(guint8 new_value);
guint8 safe_get_reply_length();

void safe_set_active_disp_index(guint8 new_value);
guint8 safe_get_active_disp_index();
void safe_increment_active_disp_index();

void safe_set_command_sended(guint8 disp_index, gboolean new_value);
gboolean safe_get_command_sended(guint8 disp_index);

void safe_set_start_filling_filter(guint8 disp_index, guint8 new_value);
guint8 safe_get_start_filling_filter(guint8 disp_index);
void safe_dec_start_filling_filter(guint8 disp_index);

void safe_set_disp_counters_enable(guint8 disp_index, gboolean new_value);
gboolean safe_get_disp_counters_enable(guint8 disp_index);

void safe_set_exchange_state(guint8 disp_index, ExchangeState new_value);
ExchangeState safe_get_exchange_state(guint8 disp_index);

void safe_set_char_ready(guint8 disp_index, PdexCtrlChar new_value);
PdexCtrlChar safe_get_char_ready(guint8 disp_index);

void safe_set_auth_code(guint8 disp_index, guint8* new_value);
void safe_get_auth_code(guint8 disp_index, guint8* buffer);

void safe_set_fault_reply_counter(guint8 disp_index, guint8 new_value);
guint8 safe_get_fault_reply_counter(guint8 disp_index);
void safe_inc_fault_reply_counter(guint8 disp_index);

void safe_get_preset(guint8 disp_index, gint8* nozzle_index, guint32* price, guint32* volume, guint32* amount);

guint8 safe_get_nozzle_num(guint8 disp_index, guint8 nozzle_index);

guint8 safe_get_nozzle_count(guint8 disp_index);

guint32 safe_get_nozzle_price(guint8 disp_index, guint8 nozzle_index);

void safe_set_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32 value);

DriverError safe_get_disp_addr(guint8 disp_index, guint8* disp_addr);

DispencerState safe_get_dispencer_state(guint8 disp_index);
void safe_set_dispencer_state(guint8 disp_index, DispencerState new_value);

gboolean safe_get_emergency_stop(guint8 disp_index);
void safe_set_emergency_stop(guint8 disp_index, gboolean new_value);

gboolean safe_get_suspend(guint8 disp_index);
void safe_set_suspend(guint8 disp_index, gboolean new_value);

gboolean safe_get_resume(guint8 disp_index);
void safe_set_resume(guint8 disp_index, gboolean new_value);

gboolean safe_get_reset(guint8 disp_index);
void safe_set_reset(guint8 disp_index, gboolean new_value);

void safe_set_prices(guint8 disp_index, guint32 price1, guint32 price2, guint32 price3, guint32 price4, guint32 price5, guint32 price6, guint32 price7, guint32 price8);

gboolean safe_get_send_prices(guint8 disp_index);
void safe_set_send_prices(guint8 disp_index, gboolean new_value);

gboolean safe_get_is_pay(guint8 disp_index);
void safe_set_is_pay(guint8 disp_index, gboolean new_value);

guint32 safe_get_current_volume(guint8 disp_index);
void safe_set_current_volume(guint8 disp_index, guint32 new_value);

guint32 safe_get_current_amount(guint8 disp_index);
void safe_set_current_amount(guint8 disp_index, guint32 new_value);

guint32 safe_get_current_price(guint8 disp_index);
void safe_set_current_price(guint8 disp_index, guint32 new_value);

guint8 safe_get_current_counter_index(guint8 disp_index);
void safe_set_current_counter_index(guint8 disp_index, guint8 new_value);
void safe_increment_current_counter_index(guint8 disp_index);

gboolean safe_compare_disp_values(guint8 disp_index);

guint8 safe_get_error(guint8 disp_index);
void safe_set_error(guint8 disp_index, guint8 new_value);
void safe_set_active_nozzle_index(guint8 disp_index, gint8 new_value);

guint8 safe_get_preset_nozzle_num(guint8 disp_index);
guint8 safe_get_active_nozzle_num(guint8 disp_index);

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


#endif /* DRIVER_STATE_H_ */

