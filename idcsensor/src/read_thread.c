#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "driver_state.h"
#include "idcsensor.h"
#include "idcsensor_func.h"

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


	safe_set_command_sended(FALSE);


	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Parse:");

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Sensor ID: %02X",  buffer[SENSOR_ID_OFFSET]);
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Port: %02X",  buffer[PORT_OFFSET]);
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Sensor addr: %02X",  buffer[SENSOR_ADDR_OFFSET]);
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Tank: %02X",  buffer[TANK_OFFSET]);
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Type: %02X",  buffer[TYPE_OFFSET]);
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Offline: %02X",  buffer[OFFLINE_OFFSET]);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Params:");

	for (guint8 pos = PARAM_DATA_OFFSET; pos < length; pos+=PARAM_DATA_LENGTH)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Param %d: value = %f code = %02X status %02X",
					buffer[pos + PARAM_ID_OFFSET],
					parse_hex_to_float(&buffer[pos+PARAM_VALUE_OFFSET]),
					buffer[pos + PARAM_CODE_OFFSET],
					buffer[pos + PARAM_STATUS_OFFSET]);
	}

	guint8 sensor_index = 0;

	if (safe_get_sensor_index_by_addr(buffer[SENSOR_ID_OFFSET], &sensor_index) == de_NoError)
	{
		safe_set_sensor_online(sensor_index, TRUE);

		for (guint8 pos = PARAM_DATA_OFFSET; pos < length; pos+=PARAM_DATA_LENGTH)
		{
			guint8 param_index = 0;

			if (safe_get_param_index_by_num(sensor_index, buffer[pos], &param_index) == de_NoError)
			{
				safe_set_sensor_param(sensor_index, param_index, &buffer[pos+PARAM_VALUE_OFFSET]);
			}
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "Error: sensor not found!");
	}

}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[BUFFER_SIZE] = {0x00};
	guint8 buff[BUFFER_SIZE] = {0x00};
	guint8 length = 0;
	guint8 frame_len = 0;

	IdcSensorFrameStage fs = isfs_WaitSOH;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch(fs)
				{
					case isfs_WaitSOH:
						if (buff[i] == CTRL_SOH)
						{
							length = 0;
							fs = isfs_WaitLength;
						}
						break;

					case isfs_WaitLength:
						frame_len = buff[i];
						fs = isfs_ReadData;
						break;

					case isfs_ReadData:
						buffer[length++] = buff[i];

						if (length == frame_len)
						{
							interpret_reply(buffer, length, log_options);
							fs = isfs_WaitSOH;
						}
						break;


				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(ds_ErrorConnection);
		}
		else
		{
		//	add_log(&log_file, TRUE, TRUE, "Read timeout");
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}
