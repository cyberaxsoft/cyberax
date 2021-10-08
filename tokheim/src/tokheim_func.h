#ifndef TOKHEIM_FUNC_H_
#define TOKHEIM_FUNC_H_

guint64 get_date_time(void);
guint32 packed_bcd_to_bin(guint8* buff, guint8 size);
void get_error_description(guint8 error_code, guchar* buffer);
const gchar* ds_to_str(DispencerState state);

guint8 set_reply_length(guint8 disp_index, TokheimCommand command);
guint8 set_reply_aux_length(guint8 disp_index, TokheimAuxCommand command);

guint32 mult_price_litres(guint32 price, guint32 litres, guint8 litres_decimal_point);
guint32 div_amount_price(guint32 price, guint32 amount, guint8 litres_decimal_point);

guint8 add_byte_to_buffer(guint8* buffer, guint8 value);
guint8 add_value_to_buffer(guint8* buffer, guint32 value, guint8 size);
guint8 add_reverse_value_to_buffer(guint8* buffer, guint32 value, guint8 size);

#endif /* TOKHEIM_FUNC_H_ */
