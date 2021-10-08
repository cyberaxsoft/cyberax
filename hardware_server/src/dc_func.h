
#ifndef DC_FUNC_H_
#define DC_FUNC_H_

const gchar* dispencer_state_to_str(DispencerState value);
const gchar* order_type_to_str(OrderType value);
const gchar* dc_status_to_str(DcDeviceStatus value);
const gchar* dc_error_to_str(DcDeviceError error);

gchar* dc_status_description(DcDeviceStatus  status, guint32* size);
gchar* dc_error_description(DcDeviceError  error, guint32* size);





#endif /* DC_FUNC_H_ */
