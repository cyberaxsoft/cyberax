#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "pdex.h"
#include "config.h"
#include "driver_state.h"
#include "pdex_frames.h"
#include "pdex_func.h"

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
	guint8 disp_count = safe_get_disp_count();

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 buffer[WRITE_BUFFER_SIZE] = {0x00};
	guint8 buffer_length = 0;

	while(!safe_get_write_thread_terminating())
	{
		if (get_date_time() > exec_time + timeout_write && disp_count > 0)
		{

			guint8 adi = safe_get_active_disp_index();

			if (safe_get_command_sended(adi))
			{
				safe_set_dispencer_state(adi, ds_NotInitialize);
				safe_set_exchange_state(adi,es_Undefined);
				safe_set_char_ready(adi, ctrl_ENQ);
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Reset exchange channel index %d", adi);

				safe_set_command_sended(adi,FALSE);
			}
			guint8 disp_addr = 0;

			if (safe_get_disp_addr(adi, &disp_addr) == de_NoError)
			{
				PdexCtrlChar char_ready = safe_get_char_ready(adi);

				if ( char_ready > ctrl_Undefined)
				{
					buffer_length = pdex_prepare_ctrl(buffer, disp_addr, char_ready);
					safe_set_char_ready(adi, ctrl_Undefined);

					if (send_func(buffer, buffer_length, log_options))
					{
						buffer_length = 0;
					}
				}

				switch(safe_get_exchange_state(adi))
				{
					case es_Undefined:
						break;

					case es_Initialize:
						{
							guint8 auth_code[TRK_AUTH_CODE_LENGTH] = {0x00};
							safe_get_auth_code(adi, auth_code);
							buffer_length = pdex_prepare_command(buffer, disp_addr, pcc_Initialization, auth_code, TRK_AUTH_CODE_LENGTH);

						}
						break;
				}

				if (buffer_length > 0)
				{
					if (send_func(buffer, buffer_length, log_options))
					{
						safe_set_command_sended(adi, TRUE);
					}
				}
			}

			safe_increment_active_disp_index();
			exec_time = get_date_time();
		}
	}

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}

