#ifndef VDS_FUNC_H_
#define VDS_FUNC_H_

guint64 get_date_time(void);
guint8 calc_crc(guint8* buffer, guint8 length);

const gchar* pps_to_str(PricePoleState state);

guint8 prepare_get_price_command(guint8 index, guint8* buffer);
guint8 prepare_set_price_command(guint8 index, guint8* buffer);

guint32 vds_unpacked_bcd_to_bin(guint8* string, guint8 length);

#endif /* VDS_FUNC_H_ */
