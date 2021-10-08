#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"

#include "shtrih.h"
#include "write_thread.h"
#include "read_thread.h"
#include "driver_state.h"
#include "shtrih_func.h"
#include "uart.h"


gboolean 		driver_init;
GMutex			init_mutex;

DriverStatus status = drs_NotInitializeDriver;
GMutex	status_mutex;

gint uart_descriptor = -1;
gint32 sock = -1;

GThread* 		write_thread = NULL;
GThread* 		read_thread = NULL;

//-------------------------------------------------   setters and getters -----------------------------------------------------------

//--------------------------------------------------     driver_init      -----------------------------------------------------------

gboolean safe_get_driver_init()
{
	gboolean result = FALSE;

	g_mutex_lock(&init_mutex);

	result = driver_init;

	g_mutex_unlock(&init_mutex);

	return result;
}

void safe_set_driver_init(gboolean new_value)
{
	g_mutex_lock(&init_mutex);

	driver_init = new_value;

	g_mutex_unlock(&init_mutex);
}

//--------------------------------------------------       status      -----------------------------------------------------------

DriverStatus safe_get_status()
{
	DriverStatus result = drs_UndefinedError;

	g_mutex_lock(&status_mutex);

	result = status;

	g_mutex_unlock(&status_mutex);

	return result;
}

void safe_set_status(DriverStatus new_status)
{
	g_mutex_lock(&status_mutex);

	status = new_status;

	g_mutex_unlock(&status_mutex);
}

//--------------------------------------------------       read      -----------------------------------------------------------

ssize_t read_func(guint8* buffer)
{
	ssize_t result = 0;

	switch (safe_get_connection_type())
	{
		case ct_Uart:
			result = read(uart_descriptor,buffer, READ_BUFFER_SIZE);
			break;

		case ct_TcpIp:
			result = recv(sock, buffer, READ_BUFFER_SIZE, 0);
			break;

	}

	return result;
}

//--------------------------------------------------       send      -----------------------------------------------------------

gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options)
{
	switch (safe_get_connection_type())
	{
		case  ct_Uart:
			{
				guint32 n = write(uart_descriptor, buffer, size);

				if (n < 0)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Error! Send failed!");
					safe_set_status(drs_ErrorConnection);
					return FALSE;
				}
				else
				{
					add_tx_buffer_to_log(buffer, size , log_options);
					return TRUE;
				}
			}
			break;

		case ct_TcpIp:
			{
				guint32 n = send(sock, buffer, size, 0);

				if (n < 0)
				{
					add_log(TRUE, TRUE,  log_options.trace, log_options.system, "Error! Send failed!");
					safe_set_status(drs_ErrorConnection);
					return FALSE;
				}
				else
				{
					add_tx_buffer_to_log(buffer, size, log_options);

					return TRUE;
				}
			}
			break;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												init_lib
//-----------------------------------------------------------------------------------------------------------------------------------

guchar init_lib(LibConfig config)
{
	if (init_mutex.p == NULL)
	{
		g_mutex_init(&init_mutex);
	}

	if (status_mutex.p == NULL)
	{
		g_mutex_init(&status_mutex);
	}

	init_driver_mutex();
	init_conf_mutex();
	init_write_thread_mutex();
	init_read_thread_mutex();

	if (!safe_get_driver_init())
	{
		safe_set_configuration(config);

		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		if (log_options.dir != NULL )
		{
			if (log_options.enable)
			{
				if (!init_log_dir(log_options.dir, LOG_PREFIX))
				{
					g_printf("%s : log init error\n", LOG_PREFIX);
					safe_set_status(drs_ErrorLogging);
					return de_ErrorLogging;
				}

				if (!init_log(log_options.dir, log_options.file_size, log_options.save_days))
				{
					g_printf("%s : log create error\n", LOG_PREFIX);
					safe_set_status(drs_ErrorLogging);
					return de_ErrorLogging;
				}

			}

			delete_old_logs(log_options.dir);

			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Init lib ------------------------------------------------------------------------------------------------");
		}
		else
		{
			safe_set_status(drs_ErrorConfiguration);
			return de_ErrorConfiguration;
		}

		safe_set_driver_settings();

		switch (safe_get_connection_type())
		{
			case  ct_Uart:
				{
					gchar* port = safe_get_port();
					guint32 uart_baudrate = safe_get_uart_baudrate();
					guint8 uart_byte_size = safe_get_uart_byte_size();
					gchar* uart_parity = safe_get_uart_parity();
					guint8 uart_stop_bits = safe_get_uart_stop_bits();

					if (port!=NULL)
					{
						uart_descriptor = open_uart(port, log_options.trace, log_options.system);

						if (uart_descriptor !=-1)
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "Open port %s", port);

						    set_settings_uart(uart_descriptor, uart_baudrate, uart_byte_size, uart_parity,  uart_stop_bits,  safe_get_timeout_read(), log_options.trace, log_options.system);

						    read_thread = g_thread_new("shtrih_read_thread", read_thread_func, NULL);
						    if (read_thread == NULL)
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting read thread", LOG_PREFIX);
						    }
						    else
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s read thread started", LOG_PREFIX);
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }

						    write_thread = g_thread_new("shtrih_write_thread", write_thread_func, NULL);
						    if (write_thread == NULL)
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting write thread", LOG_PREFIX);
						    }
						    else
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s write thread started", LOG_PREFIX);
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }
						}
						else
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "Open port %s error", port);
							safe_set_status(drs_ErrorConnection);

							return de_ErrorConnection;
						}
					}
					else
					{
						safe_set_status(drs_ErrorConfiguration);
						return de_ErrorConfiguration;
					}
				}
				break;

			case  ct_TcpIp:
				{
					gchar* ip_address = safe_get_ip_address();
					guint32 ip_port = safe_get_ip_port();
					struct sockaddr_in addr;

					sock = socket(AF_INET, SOCK_STREAM, 0);

					if(sock < 0)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Socket create error (address: %s port: %d)", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Socket create (address: %s port: %d)", ip_address, ip_port);

					addr.sin_port = htons( ip_port );
					addr.sin_addr.s_addr = inet_addr(ip_address);
					addr.sin_family = AF_INET;

					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Connecting to %s %d...", ip_address, ip_port);

					if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Connection error %s %d...", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(TRUE, TRUE,log_options.trace, log_options.system, "Connected %s %d...", ip_address, ip_port);

				    read_thread = g_thread_new("dart_read_thread", read_thread_func, NULL);
				    if (read_thread == NULL)
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting read thread", LOG_PREFIX);
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;
				    }
				    else
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s read thread started", LOG_PREFIX);
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    }

				    write_thread = g_thread_new("dart_write_thread", write_thread_func, NULL);
				    if (write_thread == NULL)
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting write thread" LOG_PREFIX);
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;
				    }
				    else
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s write thread started" LOG_PREFIX);
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    }
				}
				break;
		}

		safe_set_driver_init(TRUE);
		safe_set_status(drs_NoError);

		safe_set_write_thread_terminating(FALSE);
		safe_set_read_thread_terminating(FALSE);

		return de_NoError;
	}
	else
	{
		return de_AlreadyInit;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												close_lib
//-----------------------------------------------------------------------------------------------------------------------------------

guchar close_lib()
{
	if (safe_get_driver_init())
	{
		safe_set_write_thread_terminating(TRUE);

		while (!safe_get_write_thread_terminated());

		safe_set_write_thread_terminated(FALSE);

		g_thread_join(write_thread);

		safe_set_read_thread_terminating(TRUE);

		while (!safe_get_read_thread_terminated());

		safe_set_read_thread_terminated(FALSE);

		g_thread_join(read_thread);

		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Close lib");

		switch (safe_get_connection_type())
		{
			case  ct_Uart:
				if (uart_descriptor!=-1)
				{
					close_uart(uart_descriptor);
				}
				break;

			case  ct_TcpIp:
				if (sock > 0)
				{
					close(sock);
					sock = -1;
				}
				break;
		}

		safe_set_status(drs_NotInitializeDriver);

		safe_set_driver_init(FALSE);

		return de_NoError;
	}
	else
	{
		return de_NotInitializeDriver;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_status
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_status()
{
	return safe_get_status();
}


