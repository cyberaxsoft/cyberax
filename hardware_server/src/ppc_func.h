#ifndef PPC_FUNC_H_
#define PPC_FUNC_H_

const gchar* ppc_status_to_str(PpcDeviceStatus value);
const gchar* ppc_error_to_str(PpcDeviceError error);
gchar* ppc_status_description(PpcDeviceStatus  status, guint32* size);
gchar* ppc_error_description(PpcDeviceError  error, guint32* size);

const gchar* price_pole_state_to_str(PricePoleState value);
#endif /* PPC_FUNC_H_ */
