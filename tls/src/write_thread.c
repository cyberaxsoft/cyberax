#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "tls.h"
#include "tls_func.h"
#include "config.h"
#include "driver_state.h"
#include "tls_frames.h"

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

gpointer write_thread_func(gpointer data)
{
	safe_set_write_thread_terminated(FALSE);

	guint64 exec_time = get_date_time();
	guint32 timeout_write = safe_get_timeout_write();
	guint8 tank_count = safe_get_tank_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[UART_BUFFER_WRITE_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && tank_count > 0)
		{
			guint8 tank_channel = 0;
			guint8 active_tank_index = safe_get_active_tank_index();

			if (safe_get_tank_channel(active_tank_index, &tank_channel) == de_NoError)
			{
				if (safe_get_exchange_state(active_tank_index) == tes_SendCommand)
				{
					safe_increment_uart_error_counter(active_tank_index);
					if (safe_get_uart_error_counter(active_tank_index) > MAX_UART_ERROR)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Timeout error tank index: %d", active_tank_index);

						safe_set_exchange_state(active_tank_index, tes_Free);
						safe_set_tank_online(active_tank_index, FALSE);
						safe_set_uart_error_counter(active_tank_index, 0);
					}
				}
				else
				{
					buffer_length = prepare_inventory_frame(buffer, tank_channel);

					if (send_func(buffer, buffer_length, log_options))
					{
						safe_set_exchange_state(active_tank_index,tes_SendCommand);

						exec_time = get_date_time();
						safe_set_uart_error_counter(active_tank_index, 0);
						safe_set_last_sended_tank_index(active_tank_index);
					}
				}
			}
			safe_increment_active_tank_index();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}

