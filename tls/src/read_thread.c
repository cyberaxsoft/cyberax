#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "tls.h"
#include "config.h"
#include "driver_state.h"
#include "tls_func.h"

gboolean 		read_thread_terminating = FALSE;
gboolean 		read_thread_terminated = FALSE;
GMutex			read_thread_mutex;

void init_read_thread_mutex()
{
	if (read_thread_mutex.p == NULL)
	{
		g_mutex_init(&read_thread_mutex);
	}
}

gboolean safe_get_read_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminating;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}


gboolean safe_get_read_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminated;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}

void safe_set_read_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminating = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void safe_set_read_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminated = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}

	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");


	add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Parse:");

	guint8 tank_index = 0;

	if (safe_get_tank_index_by_channel(ascii_to_int(&buffer[REPLY_CHANNEL_OFFSET], REPLY_CHANNEL_LENGTH), &tank_index) == de_NoError)
	{

		safe_set_tank_online(tank_index, TRUE);

		guint8 param_index = 0;

		for (guint i = REPLY_DATA_OFFSET; i < length; i+=REPLY_DATA_FIELD_LENGTH)
		{
			if (buffer[i] == ctrl_Tilda && buffer[i + 1] == ctrl_Tilda)
			{
				break;
			}
			else
			{
				switch (param_index)
				{
					case 0: //volume
						safe_set_tank_volume(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Volume = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 1:
						safe_set_tank_weight(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Weight = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 2:
						safe_set_tank_density(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Density = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 3:
						safe_set_tank_height(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Height = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 4:
						safe_set_tank_water(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	WaterLevel = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 5:
						safe_set_tank_temperature(tank_index, parse_float_hex_string(&buffer[i]));
						add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Temperature = %f", parse_float_hex_string(&buffer[i]));
						break;
					case 6:
						//TC density
						break;
					case 7:
						//TC volume
						break;
					case 8:
						//ullage
						break;
					case 9:
						//water volume
						break;
				}
			}


			param_index++;


		}
	}

	guint8 active_tank_index = safe_get_active_tank_index();
	safe_set_exchange_state(active_tank_index, tes_Free);

}


gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	TlsExchangeState es = es_WaitSTX;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch(es)
				{
					case es_WaitSTX:
						if (buff[i] == ctrl_STX)
						{
							length = 0;
							buffer[length++] = buff[i];
							es = es_CommPefix;
						}
						break;

					case es_CommPefix:
						buffer[length++] = buff[i];

						if (buff[i] == ctrl_ErrorPrefix)
						{
							es = es_WaitEtx;
						}
						else
						{
							es = es_WaitTilda1;
						}
						break;

					case es_WaitTilda1:
						buffer[length++] = buff[i];
						if (buff[i] == ctrl_Tilda)
						{
							es = es_WaitTilda2;
						}
						break;

					case es_WaitTilda2:
						buffer[length++] = buff[i];
						if (buff[i] == ctrl_Tilda)
						{
							es = es_WaitCrc1;
						}
						else
						{
							es = es_WaitTilda1;
						}
						break;

					case es_WaitCrc1:
						buffer[length++] = buff[i];
						es = es_WaitCrc2;
						break;

					case es_WaitCrc2:
						buffer[length++] = buff[i];
						es = es_WaitCrc3;
						break;

					case es_WaitCrc3:
						buffer[length++] = buff[i];
						es = es_WaitCrc4;
						break;

					case es_WaitCrc4:
						buffer[length++] = buff[i];
						es = es_WaitEtx;
						break;

					case es_WaitEtx:
						buffer[length++] = buff[i];
						if (buff[i] == ctrl_ETX)
						{
							interpret_reply(buffer, length, log_options);
							es = es_WaitSTX;
						}
						break;
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
		else
		{
		//	add_log(&log_file, TRUE, TRUE, "Read timeout");
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}
