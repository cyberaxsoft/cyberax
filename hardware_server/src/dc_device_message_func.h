#ifndef DC_DEVICE_MESSAGE_FUNC_H_
#define DC_DEVICE_MESSAGE_FUNC_H_

void send_dc_device_status_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id,
										MessageType message_type);
void send_dc_device_command_result_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id,
										HardwareServerCommand command, guint8 device_reply_code, ExchangeError exchange_error, guint32 device_num);
void send_dc_device_configuration_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id,
									MessageType message_type);
void send_dc_device_data_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type,
								guint32 disp_num, guint8 disp_index);
void send_dc_device_counters_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id,
								MessageType message_type, guint32 disp_num, guint8 nozzle_num, guint8 disp_index, guint8 nozzle_index);


#endif /* DC_DEVICE_MESSAGE_FUNC_H_ */
