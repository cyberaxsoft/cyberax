#ifndef PSS_PARSE_H_
#define PSS_PARSE_H_

void send_rejected_reply(PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 code, guint8 subcode);
void send_pin_status(PSSClientThreadFuncParam* params, gboolean status);
void interpret_service_mode_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_fuelling_mode_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_grade_texts_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_global_fuelling_limits_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_fuelling_mode_group_param_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_change_fc_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_general_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_change_service_mode_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_change_fuelling_mode_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_change_global_fuelling_limits_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void interpret_change_fuelling_mode_group_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void fc_gen_par_set_reply_ex(guint8* buffer, guint8 length, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_open_fp(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_close_fp(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_authorize_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_authorize_fp_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id);
void parse_install_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_clr_install_data(guint8 subcode,guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_load_fc_price_set(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode);
void parse_fc_price_set_req(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode);
void send_fc_install_status_req(PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_fc_price_set_status_req( PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fc_logon_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_fc_status_reply(PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_fc_date_time_reply(PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_load_fp_operation_mode_set(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fp_install_data_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_fc_set_date_time(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fp_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params,  gboolean ex_mode);
void send_fp_status_req_1_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode);
void send_fp_status_req_2_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode);
void send_fp_status_req_3_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode);
void parse_fp_sup_trans_buffer_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fp_info_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fp_fuelling_data_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_fp_sup_trans_buffer_status_req_1_mult(PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port);
void send_fp_sup_trans_buffer_status_req_3_mult(PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port);
void send_fp_status_req_3(guint8 fp_id, PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port);
void send_fc_operation_mode_status(PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_set_fc_operation_mode_no(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode);
void send_pos_connection_status(PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_pss_peripheral_status(PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_fp_grade_totals_req_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_pump_grade_totals_req_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);

void parse_cancel_estop_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_tgs_tg_parametars_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_tgs_status_req_1_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode);

void send_tg_status_req_1_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode);

void parse_install_price_pole(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_price_pole_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_load_pp_operation_mode_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_open_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_close_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_reset_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);

void parse_install_tank_gauge(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_install_tg_ext_sub_dev(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_change_tgs_tg_parametars(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_tg_status(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_tg_data_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_tg_status_req_1(guint8 tg_id, PSSClientThreadFuncParam* params,  gboolean ex_mode);

void send_prices_message(PSSClientThreadFuncParam* params, guint8 device_index, PricePacks* price_packs);

void send_clr_fp_sup_trans_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id);

void interpret_change_grade_texts(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
#endif
