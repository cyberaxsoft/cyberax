#ifndef TLS_FUNC_H_
#define TLS_FUNC_H_

guint64 get_date_time(void);
guint16 struna_crc_next(guint16 crc_value, guint8 symbol);
gfloat parse_float_value(guint8* buffer);

#endif /* TLS_FUNC_H_ */
