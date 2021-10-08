
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "logger.h"
#include "tlv.h"
#include "socket_func.h"

GMutex create_sock_mutex;

void sock_mutex_init()
{
	g_mutex_init(&create_sock_mutex);
}


//------------------create socket function------------------
gint32 create_serv_sock(gint32 old_sock, gint16 port)
{

	gint32 result = -1;

	g_mutex_lock(&create_sock_mutex);

	struct sockaddr_in addr;

	if (old_sock > 0) close(old_sock);

	result = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(result, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
    	close(result);
    	result = -1;
    }
    else
    {
    	listen(result, 1);
    }

	g_mutex_unlock(&create_sock_mutex);

	return result;

}

void socket_send(SockClientInfo* client_info, guchar* buffer, guint32 size)
{

	add_log(client_info->log_params, TRUE, FALSE, client_info->log_trace, client_info->log_frames, "%s >> ", client_info->device_name);

	for (guint16 i = 0; i < size; i++)
	{
		add_log(client_info->log_params, FALSE, FALSE, client_info->log_trace, client_info->log_frames, " %02X", buffer[i]);

	}
	add_log(client_info->log_params, FALSE, TRUE, client_info->log_trace, client_info->log_frames, "");



	g_mutex_lock(client_info->socket_mutex);
	send(client_info->socket, buffer, size, 0);
	g_mutex_unlock(client_info->socket_mutex);
}

void send_short_message(SockClientInfo* client_info, MessageType message_type, ExchangeError exchange_error)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send short request", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s Short request:", client_info->device_name);

	TlvUnit* units = NULL;

	guchar message_type_buffer[] = {message_type};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Message type: %d (%s)", client_info->device_name, message_type, return_message_type_name(message_type));

	guchar error_buffer[] = {exchange_error};
	tlv_add_unit(&units,tlv_create_unit(tst_Error , error_buffer, 0, sizeof(error_buffer)));
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Error: 0x%02X", client_info->device_name, exchange_error);

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}

	tlv_delete_units(&units);

}
