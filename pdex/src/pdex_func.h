#ifndef PDEX_FUNC_H_
#define PDEX_FUNC_H_

guint64 get_date_time(void);
void get_error_description(guint8 error_code, guchar* buffer);
const gchar* ds_to_str(DispencerState state);

void bin_to_unpacked_bcd(guint8* buffer, guint8 length, guint32 value);
void pdex_create_authorization_code(guint8* destination, guint8* source, guint8 addr);
guint16 pdex_crc(guint8* source, guint8 length);

#endif /* PDEX_FUNC_H_ */
