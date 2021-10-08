#ifndef TOPAZ_FUNC_H_
#define TOPAZ_FUNC_H_

guint64 get_date_time(void);
guint8 calc_crc(guint8* buffer, guint8 length);
guint8 add_byte(guint8* destination, guint8 value, guint8* crc);
guint8 get_stx(guint8 channel);
guint8 get_addr(guint8 channel);
guint32 azt_unpacked_bcd_to_bin(guint8* string, guint8 length);
guint8 azt_bin_to_unpacked_bcd(guint8* buffer, guint32 value, guint8* crc, guint8 length);
void prepare_command_request(guint8* destination, guint8 channel, guint8* length, AztCommandCode command_code);
void prepare_command_param_request(guint8* destination, guint8 channel, guint8* length, AztCommandCode command_code, guint32 value, guint8 size);
const gchar* bool_to_str(gboolean value);
const gchar* ds_to_str(DispencerState state);
void get_error_description(guint8 error_code, guchar* buffer);

#endif /* TOPAZ_FUNC_H_ */
