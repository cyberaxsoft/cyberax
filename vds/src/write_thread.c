#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "vds.h"
#include "config.h"
#include "vds_func.h"
#include "driver_state.h"

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
	guint8 price_pole_count = safe_get_price_pole_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[WRITE_BUFFER_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && price_pole_count > 0)
		{
			guint8 cpi = safe_get_current_price_pole_index();

			if (safe_get_command_sent(cpi) == TRUE)
			{
				safe_inc_uart_error(cpi);

				if (safe_get_uart_error(cpi) > MAX_UART_ERROR)
				{
					safe_set_exchange_state(cpi, ves_SetPrice);
					safe_set_uart_error(cpi, 0);
					safe_set_state(cpi, pps_Offline);
					safe_set_command_sent(cpi, FALSE);

					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Set price pole index %d offline. Send price.", cpi);
				}
			}
			switch(safe_get_exchange_state(cpi))
			{
				case ves_GetPrice:
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send get data from price pole index %d", cpi);
					buffer_length = prepare_get_price_command(cpi, buffer);

					if (buffer_length > 0)
					{
						if (send_func(buffer, buffer_length, log_options))
						{
							safe_set_command_sent(cpi, TRUE);
						}
					}
					break;

				case ves_SetPrice:
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Send set price for price pole index %d", cpi);

					buffer_length = prepare_set_price_command(cpi, buffer);

					if (buffer_length > 0)
					{
						if (send_func(buffer, buffer_length, log_options))
						{
							safe_set_command_sent(cpi, FALSE);
							safe_set_exchange_state(cpi, ves_GetPrice);
						}
					}
					break;
			}


			safe_inc_current_price_pole_index();
			exec_time = get_date_time();

		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}


