#ifndef TGS_FUNC_H_
#define TGS_FUNC_H_

const gchar* tgs_status_to_str(TgsDeviceStatus value);
const gchar* tgs_error_to_str(TgsDeviceError error);
gchar* tgs_status_description(TgsDeviceStatus  status, guint32* size);
gchar* tgs_error_description(TgsDeviceError  error, guint32* size);


#endif /* TGS_FUNC_H_ */
