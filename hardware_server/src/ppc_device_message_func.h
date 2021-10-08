#ifndef PPC_DEVICE_MESSAGE_FUNC_H_
#define PPC_DEVICE_MESSAGE_FUNC_H_

void send_ppc_device_status_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_ppc_device_command_result_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, HardwareServerCommand command, guint8 device_reply_code, ExchangeError exchange_error);
void send_ppc_device_configuration_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_ppc_device_data_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type);

#endif /* PPC_DEVICE_MESSAGE_FUNC_H_ */
