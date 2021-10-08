#ifndef TOKHEIM_FRAMES_H_
#define TOKHEIM_FRAMES_H_

guint8 prepare_get_fuelling_point_id_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_get_aux_fuelling_point_id_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_get_fuelling_point_display_data_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_get_fuelling_point_status_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_suspend_fuelling_point_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_resume_fuelling_point_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_authorize_fuelling_point_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume);
guint8 prepare_authorize_fuelling_point2_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume);
guint8 prepare_send_data_for_fuelling_point_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume);
guint8 prepare_reset_fuelling_point_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_fuelling_point_totals_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr);
guint8 prepare_set_fuelling_point_display_control_frame(guint8* buffer, guint8 disp_addr, gboolean light_on);
guint8 prepare_request_activated_hose_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_request_deactivated_hose_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_send_cash_prices_frame(guint8* buffer, guint8 disp_addr, guint32 price1, guint32 price2, guint32 price3);
guint8 prepare_send_aux_cash_prices_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr);


//guint8 prepare_get_counters_frame(guint8* buffer, guint8 disp_addr, guint8 nozzle_count);


#endif /* TOKHEIM_FRAMES_H_ */
