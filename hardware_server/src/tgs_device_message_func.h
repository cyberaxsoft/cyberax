#ifndef TGS_DEVICE_MESSAGE_FUNC_H_
#define TGS_DEVICE_MESSAGE_FUNC_H_

void send_tgs_device_status_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix);
void send_tgs_device_configuration_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix);
void send_tgs_device_data_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 tank_num, guint8 tank_index, gchar* log_prefix);


#endif /* TGS_DEVICE_MESSAGE_FUNC_H_ */
