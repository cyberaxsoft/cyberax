#ifndef SOCKET_FUNC_H_
#define SOCKET_FUNC_H_

typedef struct _SockClientInfo
{
	gint32 					socket;
	GMutex*					socket_mutex;

	gchar* 					device_name;
	LogParams*				log_params;
	gchar* 					ip_address;
	gboolean 				log_trace;
	gboolean 				log_frames;
	gboolean 				log_parsing;
}SockClientInfo;

void sock_mutex_init();

gint32 create_serv_sock(gint32 old_sock, gint16 port);
void socket_send(SockClientInfo* client_info, guchar* buffer, guint32 size);
void send_short_message(SockClientInfo* client_info, MessageType message_type, ExchangeError exchange_error);

#endif
