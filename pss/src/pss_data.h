#ifndef PSS_DATA_H_
#define PSS_DATA_H_

void set_fc_log_configuration(gchar* log_dir, gboolean log_enable, gboolean log_trace);

void set_door_settings(guint32 new_sensor_num, guint8 new_param_num);
void get_door_settings(guint32* p_sensor_num, guint8* p_param_num);
void init_sc_mutex();


void master_reset();
gboolean get_server_conf_is_load();
void set_server_conf_is_load(gboolean new_value);
void clear_service_devices();
void clear_service_device(PSSServerDevice* device );
void add_server_device(PSSServerDevice* device );
void init_pss_data_mutex();
gboolean load_pss_data();
gboolean save_pss_data();
void get_fc_status(PSSFcStatus* status);
void update_fc_master_reset_date_and_time(time_t new_value);
void update_fc_reset_date_and_time(time_t new_value);
void update_fc_price_set_id(guint8 new_value);
guint8 get_fc_operation_mode_no();
void update_fc_operation_mode_no(guint8 new_value);
guint8 get_device_groups_count();
guint8 get_fuelling_point_count();
guint8 get_price_pole_count();
guint8 get_tank_gauge_count();
guint8 get_tank_gauge_sub_device_count();
guint8 get_server_device_count();
guint8 get_price_pole_id_by_index(guint8 index );
guint8 get_fuelling_point_id_by_index(guint8 index );
guint8 get_tank_gauge_id_by_index(guint8 index );
guint8 get_tank_gauge_sub_device_id_by_index(guint8 index );
void update_fc_single_price(guint8 price_group_id, guint8 grade_id, guint32 new_value);
void close_fp(guint8 fp_id );
gboolean clear_install_data(guint8 install_msg_code, guint8 fc_device_id);
gboolean clear_ext_install_data(guint8 install_msg_code_0, guint8 install_msg_code_1,guint8 install_msg_code_2, guint8 fc_device_id);
void get_general_params(PSSGeneralParams* params);
void get_fc_general_functions(PSSGeneralFunctions* gf);
void set_general_params(PSSGeneralParams params);
void get_service_mode_by_id(guint8 id, PSSServiceMode* sm);
void set_service_mode_by_id(guint8 id, PSSServiceMode sm);
void get_fuelling_mode_by_id(guint8 id, PSSFuellingMode* fm);
void set_fuelling_mode_by_id(guint8 id, PSSFuellingMode fm);
void get_global_fuelling_limits(PSSDispenceLimits* dl);
void set_global_fuelling_limits(PSSDispenceLimits dl);
void get_fuelling_mode_group_by_id(guint8 id, PSSFuellingModeGroup* fmg);
void get_server_device_by_index(guint8 index, PSSServerDevice* device);
void get_server_device_guid_by_index(guint8 index, gchar** guid);
void get_server_devices(PSSServerDevices* devices);
void set_fuelling_mode_group_by_id(guint8 id, PSSFuellingModeGroup fmg);
gint8 get_new_grade_option_index(PSSFuellingPoint* fp, guint8 id);
void install_new_fp(PSSFuellingPoint new_fp);
void get_fuelling_point_by_id(guint8 id, PSSFuellingPoint* fp);
void get_fuelling_point_by_index(guint8 index, PSSFuellingPoint* fp);
void set_fuelling_point_by_id(guint8 id, PSSFuellingPoint fp);
void update_fp_operation_mode(PSSFuellingPoint* fp, PSSOperationMode om);
void update_fp(PSSFuellingPoint fp);
void update_price_group(PSSPriceGroup price_group);
guint8 get_fuelling_mode_group_count();
void get_fuelling_mode_group_by_index(guint8 index, PSSFuellingModeGroup* fmg);
void get_fc_log_configuration(gchar** log_dir, gboolean* log_enable, gboolean* log_trace);
guint8 get_price_group_id_by_index(guint index_price_group);
void get_price_group_by_id(guint8 id, PSSPriceGroup* pg);
void set_fps_main_state(PSSServerDeviceUnits* ids, PSSFpMainState new_value );
void set_fps_sub_state(PSSServerDeviceUnits* ids, guint8 new_value );
void set_fp_main_state_by_id(guint8 fp_id, PSSFpMainState new_value);
PSSFpMainState get_fp_main_state_by_id(guint8 fp_id);
void get_sup_transaction(guint8 fp_id, guint32 trans_seq_no, PSSTransaction* transaction);
void delete_fp_sup_trans(guint8 fp_id, guint32 trans_seq_no);

void install_new_pp(PSSPricePole new_pp);

void install_new_tg(PSSTankGauge new_tg);
void install_new_tgsd(PSSTankGaugeSubDevice new_tgsd);
void update_tg_tank_height(guint8 tg_id, guint32 tank_height);
void update_tg_point(guint8 tg_id, guint8 index_point, guint32 volume);
void get_tg_by_id(guint8 id, PSSTankGauge* tg);
void get_tgsd_by_id(guint8 id, PSSTankGaugeSubDevice* tgsd);

void get_tank_by_id(guint8 id, PSSTank* tank);
void set_tank_by_id(guint8 id, PSSTank tank);

guint8 get_tgs_count();
guint8 get_tgsds_count();
guint8 get_tgs_id_by_index(guint8 index );
void get_tgs_by_id(guint8 id, PSSTankGauge* tg);
void set_tgs_by_id(guint8 id, PSSTankGauge tg);
void set_tgsds_by_id(guint8 id, PSSTankGaugeSubDevice tank_gauge_sub_device);

void get_pp_by_id(guint8 id, PSSPricePole* pp);
guint8 get_pp_count();
void get_pp_by_index(guint8 index, PSSPricePole* pp);
void set_pp_by_id(guint8 id, PSSPricePole pp);


void fp_price_update(PSSFuellingPoint fp, guint8 op_mode_no, PSSClientThreadFuncParam* params);
void price_update(PSSPriceGroup* price_group, PSSClientThreadFuncParam* params);
void reset_fp(guint8  fp_id, PSSClientThreadFuncParam* params, PSSCommand pss_command, guint8 subcode, gboolean ex_mode);
void stop_fp(guint8  fp_id, PSSClientThreadFuncParam* params);
void suspend_fp(guint8  fp_id, PSSClientThreadFuncParam* params);
void resume_fp(guint8  fp_id, PSSClientThreadFuncParam* params);
void authorize_fp(PSSFuellingPoint fp, PSSClientThreadFuncParam* params, PSSCommand pss_command, guint8 subcode, gboolean ex_mode);
void parse_fp_sup_trans_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_clr_fp_sup_trans(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void parse_reset_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);
void send_reset_fp_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id);
void parse_estop_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode);

gboolean get_door_switch();
void set_door_switch(gboolean new_value);

#endif /* PSS_DATA_H_ */
