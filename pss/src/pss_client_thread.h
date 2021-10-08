#ifndef PSS_CLIENT_THREAD_H_
#define PSS_CLIENT_THREAD_H_

typedef struct _PSSClientThreadFuncParam
{
	guint8 client_index;
	LogParams* log_params;
	gboolean log_trace;
	gboolean log_enable;
}PSSClientThreadFuncParam;

typedef struct _PSSClientDeviceParam
{
	PSSClientThreadFuncParam*	client_params;
	guint8						device_index;
	guint8						device_id;
}PSSClientDeviceParam;

gpointer client_thread_func(gpointer data);


#endif /* PSS_CLIENT_THREAD_H_ */
