#ifndef SC_DEVICE_MESSAGE_FUNC_H_
#define SC_DEVICE_MESSAGE_FUNC_H_

void send_sc_device_status_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix);
void send_sc_device_configuration_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix);
void send_sc_device_data_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 sensor_num, guint8 sensor_index, gchar* log_prefix);

#endif /* SC_DEVICE_MESSAGE_FUNC_H_ */
