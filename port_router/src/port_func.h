#ifndef PORT_FUNC_H_
#define PORT_FUNC_H_

const gchar* bool_to_str(gboolean value);
guint64 get_date_time(void);

guint16 crc16_calc(guint8 *buffer, gsize length);
guint16 crc16_next(guint16 crc16_value, guint8 symbol);


#endif /* PORT_FUNC_H_ */
