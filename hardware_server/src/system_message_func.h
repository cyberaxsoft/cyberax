#ifndef SYSTEM_MESSAGE_FUNC_H_
#define SYSTEM_MESSAGE_FUNC_H_

void send_system_common_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_system_dispencer_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_system_tgs_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_system_client_profile_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_system_command_result_message(SockClientInfo* client_info, guint32 message_id, HardwareServerCommand command, ExchangeError exchange_error );
void send_system_price_pole_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);
void send_system_sensor_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type);



#endif /* SYSTEM_MESSAGE_FUNC_H_ */
