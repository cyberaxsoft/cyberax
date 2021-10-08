#include <glib.h>
#include <glib/gstdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_func.h"

gpointer main_thread_func(gpointer data)
{
	PSSThreadParam param = *(PSSThreadParam*)data;

    gint32 sock;
    struct sockaddr_in addr;

	LogParams log_params = {0x00};

	create_log_params(&log_params, param.log_enable, param.log_dir, (gchar*)port_to_str(param.port), param.file_size, param.save_days);

	delete_old_logs(&log_params);

	if (log_params.enable && log_params.log==NULL)
	{
		if (!init_log_dir(&log_params))
		{
			g_printf("PSS : initialization log error\n");
			return NULL;
		}

		g_printf("PSS : system log path: %s\n", param.log_dir);

		create_log(&log_params);

		if (log_params.log == NULL)
		{
			g_printf("System : log create error\n");
			return NULL;
		}
	}
	add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "starting thread...");


    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
    	add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "main socket create error");
    }
    else
    {
    	add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "main socket create");

        addr.sin_family = AF_INET;
        addr.sin_port = htons(param.port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        gboolean socket_bind = FALSE;
    	guint64 last_bind_time = get_date_time();

        while (!socket_bind)
        {
        	if (get_date_time() > last_bind_time + SOCKET_BIND_TIMEOUT)
        	{
        		last_bind_time = get_date_time();

                if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
                {
                	add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "main socket bind error!");
                }
                else
                {
                	socket_bind = TRUE;
                }
        	}
        }

       	add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "main socket bind");
       	listen(sock, 1);

		while(TRUE)
		{
			struct sockaddr client_addr;
			socklen_t client_addr_len = 0;

			guint8 new_client_index = find_next_client_index();

			add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "wait new client index %d", new_client_index);

			if (new_client_index < PSS_MAX_CLIENT_COUNT)
			{
				guint32 client_sock = accept(sock, &client_addr, &client_addr_len);

				set_client_sock(new_client_index, client_sock);

				if(client_sock < 0)
				{
					add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "client %d socket accepting error", new_client_index);
					set_client_port(new_client_index, 0);
					set_client_state(new_client_index,cs_Free);

				}
				else
				{
					struct sockaddr_in addr;
					socklen_t addr_size = sizeof(struct sockaddr_in);
					getpeername(client_sock, (struct sockaddr *)&addr, &addr_size);

					set_client_ip_address(new_client_index, addr.sin_addr.s_addr);

					set_client_exchange_status(new_client_index, ces_Undefined);
					set_client_port(new_client_index, param.port);
					set_client_state(new_client_index,cs_Active);

					add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "client %d socket accepted (%s)", new_client_index, inet_ntoa(addr.sin_addr));

					PSSClientThreadFuncParam param_client_thread = {.client_index = new_client_index, .log_params = &log_params, .log_enable = log_params.enable, .log_trace =  param.log_trace};

					GThread* client_thread = g_thread_new("pss_client_thread", client_thread_func, &param_client_thread);

					if (client_thread == NULL)
					{
						add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "client %d error starting client thread", new_client_index);
					}
					else
					{
						client_thread->priority = G_THREAD_PRIORITY_LOW;
					}
				}
			}
			else
			{
				add_log(&log_params, TRUE, TRUE, param.log_trace, TRUE, "Client list is full!");
			}
		}
		close(sock);
    }
    return NULL;
}
