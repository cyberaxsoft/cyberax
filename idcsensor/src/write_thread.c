#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "idcsensor.h"
#include "idcsensor_func.h"
#include "driver_state.h"
#include "idcsensor_frame.h"

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
	guint8 sensor_count = safe_get_sensor_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[BUFFER_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && sensor_count > 0)
		{
			if (safe_get_command_sended())
			{
				safe_set_all_sensor_online(FALSE);
				safe_set_command_sended(FALSE);
			}

			buffer_length = prepare_get_data_frame(buffer);

			if (send_func(buffer, buffer_length, log_options))
			{
				safe_set_command_sended(TRUE);
			}
			else
			{
				safe_set_all_sensor_online(FALSE);
			}
			exec_time = get_date_time();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}

