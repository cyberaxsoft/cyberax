#ifndef FR_DEVICE_CLIENT_DATA_H_
#define FR_DEVICE_CLIENT_DATA_H_

void fr_client_init();
guint8 get_client_fri(guint8 index);
ClientState get_fr_client_state(guint8 index);

void set_fr_clients_status_is_destroying();
void set_fr_device_status_is_changed_for_all_clients(guint8 fri, gboolean value, FrDeviceStatus status );


#endif /* FR_DEVICE_CLIENT_DATA_H_ */
