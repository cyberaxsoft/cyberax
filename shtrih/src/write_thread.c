#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "shtrih.h"
#include "driver_state.h"
#include "shtrih_func.h"

gboolean 		write_thread_terminating = FALSE;
gboolean 		write_thread_terminated = FALSE;
GMutex			write_thread_mutex;

void init_write_thread_mutex()
{
	if (write_thread_mutex.p == NULL)
	{
		g_mutex_init(&write_thread_mutex);
	}
}

gboolean safe_get_write_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminating;

	g_mutex_unlock(&write_thread_mutex);

	return result;

}

gboolean safe_get_write_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminated;

	g_mutex_unlock(&write_thread_mutex);

	return result;

}

void safe_set_write_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminating = new_value;

	g_mutex_unlock(&write_thread_mutex);

}

void safe_set_write_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminated = new_value;

	g_mutex_unlock(&write_thread_mutex);

}

guint8 prepare_next_frame(guint8* buffer)
{
	guint8 pos = 0;

	pos = prepare_get_state_frame(buffer);

	return pos;
}

gpointer write_thread_func(gpointer data)
{
	safe_set_write_thread_terminated(FALSE);

	guint64 exec_time = get_date_time();
	guint32 timeout_write = safe_get_timeout_write();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[WRITE_BUFFER_SIZE] = {0x00};
	guint8 buffer_length = 0;

	guint8 uart_error_counter = 0;


	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write )
		{
			buffer_length = 0;

			switch (safe_get_exchange_state())
			{
				case es_Ready:
					buffer_length = prepare_enq_frame(buffer);
					safe_set_exchange_state(es_EnqSent);
					break;

				case es_ReadyAck:
					buffer_length = prepare_ack_frame(buffer);
					safe_set_exchange_state(es_Ready);
					break;

				case es_ReadyNak:
					buffer_length = prepare_nak_frame(buffer);
					safe_set_exchange_state(es_Ready);
					break;

				case es_CommandReady:
					buffer_length = prepare_next_frame(buffer);
					safe_set_exchange_state(es_WaitSTX);
					break;

				case es_EnqSent:
				case es_DataSent:
				case es_WaitSTX:
				case es_WaitLength:
				case es_WaitLrc:
				case es_WaitData:
					uart_error_counter++;

					if (uart_error_counter >= MAX_UART_ERROR)
					{
						uart_error_counter = 0;
						safe_set_status(drs_ErrorConnection);
						safe_set_exchange_state(es_Ready);
					}
					break;
			}

			if (buffer_length > 0)
			{
				if (send_func(buffer, buffer_length, log_options))
				{
					uart_error_counter = 0;
				}
			}
			exec_time = get_date_time();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}


