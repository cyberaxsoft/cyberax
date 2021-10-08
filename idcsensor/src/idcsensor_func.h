#ifndef IDCSENSOR_FUNC_H_
#define IDCSENSOR_FUNC_H_

guint64 get_date_time(void);
guint8 ascii_to_int(guint8* buffer, guint8 length);
gfloat parse_hex_to_float(guint8* buffer);


#endif /* IDCSENSOR_FUNC_H_ */
