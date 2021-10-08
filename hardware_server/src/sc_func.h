
#ifndef SC_FUNC_H_
#define SC_FUNC_H_

const gchar* sc_status_to_str(ScDeviceStatus value);
const gchar* sc_error_to_str(ScDeviceError error);
const gchar* spt_to_str(SensorParamType value);
gchar* sc_status_description(ScDeviceStatus  status, guint32* size);
gchar* sc_error_description(ScDeviceError  error, guint32* size);
gchar* spt_description(SensorParamType  value, guint32* size);

#endif /* SC_FUNC_H_ */
