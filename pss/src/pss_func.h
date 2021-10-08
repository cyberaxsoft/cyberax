#ifndef PSS_FUNC_H_
#define PSS_FUNC_H_

const gchar* fc_par_group_id_to_str(FcParGroupId id);
const gchar* fc_gen_par_id_to_str(FcGenParId id);
const gchar* fc_serv_mode_par_id_to_str(FcServModeParId id);
const gchar* fc_fuelling_mode_par_id_to_str(FcFuellingModeParId id);
const gchar* fc_global_fuelling_limits_par_id_to_str(FcDispenceLimitsParId id);
const gchar* fp_install_par_id_to_str(FpInstallParId id);
const gchar* server_device_type_to_str(PSSServerDeviceType device_type);
const gchar* port_to_str(guint32 port);
const gchar* fp_main_state_to_str(PSSFpMainState fp_state);
const gchar* dispencer_state_to_str(DispencerState value);
const gchar* order_type_to_str(OrderType value);

gint prepare_heartbeat_frame(guint8* buffer);

guint16 parse_BIN16(guint8* buffer);
guint16 parse_BIN16_LSB(guint8* buffer);
guint32 parse_BIN32_LSB(guint8* buffer);
guint8 parse_BIN8(guint8* buffer);
guint32 parse_BCD(guint8* buffer, guint8 length);
guint8 parse_ASCII_byte(guint8* buffer);
guint16 parse_pss_ascii(guint8* dest, guint8* src, guint8 length);
gint32 prepare_dpl_frame(guint8* dest, guint8* src, guint16 src_length);
void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size);
PSSFpMainState decode_dispencer_state(DispencerState value, OrderType order_type);

guint64 get_date_time(void);
const gchar* bool_to_str(gboolean value);
void socket_send(gint32 sock, guchar* buffer, guint32 size);
guint8 port_to_apc_code(guint32 port);
guint8 return_pos_device_type(guint32 port);

guint8 prepare_pss_reply_header(guint8* buffer, guint32 port, gboolean ex_mode, PSSCommand command_code, PSSReplyCode reply_code, guint8 subcode);
guint8 prepare_pss_mult_reply_header(guint8* buffer, guint32 port, gboolean ex_mode, PSSCommand command_code, PSSReplyCode reply_code, guint8 subcode);
guint8 add_bcd_field(guint8* buffer, guint32 value, guint8 length);
guint8 add_date_time_field(guint8* buffer, struct tm* timeinfo);

#endif /* PSS_FUNC_H_ */
