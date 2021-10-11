#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "strunaplus.h"
#include "config.h"
#include "driver_state.h"
#include "strunaplus_func.h"

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

void interpret_select_channel_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	guint8 active_tank_index = safe_get_active_tank_index();

	guint8 tank_channel = 0;

	if (safe_get_tank_channel(active_tank_index, &tank_channel) == de_NoError)
	{
		if(safe_get_active_tank_index() == buffer[CHANNEL_OFFSET])
		{
			safe_set_exchange_state(es_RequestChannelConfiguration);
		}
	}
}

void interpret_configuration_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	guint8 active_tank_index = safe_get_active_tank_index();
	guint8 tank_channel = 0;

	if (safe_get_tank_channel(active_tank_index, &tank_channel) == de_NoError)
	{
		if(buffer[CHANNEL_TYPE_OFFSET] == 0 && buffer[CHANNEL_OFFSET_1] == tank_channel)
		{
//			guint8 config_mask = buffer[CONFIG_MASK_OFFSET];
//
//			gboolean level_conf = ((config_mask & LEVEL_MASK) == 0) ? FALSE : TRUE;
//			gboolean temp_conf = ((config_mask & TEMPERATURE_MASK) == 0) ? FALSE : TRUE;
//			gboolean volume_conf = ((config_mask & VOLUME_MASK) == 0) ? FALSE : TRUE;
//			gboolean water_conf = ((config_mask & WATER_MASK) == 0) ? FALSE : TRUE;
//			gboolean density_conf = ((config_mask & DENSITY_MASK) == 0) ? FALSE : TRUE;

			safe_set_exchange_state(es_RequestParameters);
		}
	}
}

void interpret_parametres_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	guint8 active_tank_index = safe_get_active_tank_index();
	guint8 offset = DATA_OFFSET;

//----------------------------------------------------------------------------	level

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_volume(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;

	//----------------------------------------------------------------------------	skip weight

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_weight(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;
	//----------------------------------------------------------------------------	volume

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_volume(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;

	//----------------------------------------------------------------------------	density

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_density(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;

	//----------------------------------------------------------------------------	temperature

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_temperature(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;

	//----------------------------------------------------------------------------	water

	if(buffer[offset + DATA_ERROR_OFFSET] == 0)
	{
		safe_set_tank_water(active_tank_index, parse_float_value(&buffer[offset]));
	}

	offset += DATA_VALUE_SIZE;

	//----------------------------------------------------------------------------

	safe_increment_active_tank_index();

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

	guint8 command = buffer[COMMAND_OFFSET];

	if (command & STRUNA_EXCEPTION_MASK)
	{
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Error: %d", buffer[ERROR_OFFSET]);
	}
	else
	{

	}
	switch (safe_get_exchange_state())
	{
		case es_Idle:
			break;

		case es_SelectChannel:
			interpret_select_channel_reply(buffer, length, log_options);
			break;

		case es_RequestChannelConfiguration:
			break;

		case es_WaitChannelConfiguration:
			interpret_configuration_reply(buffer, length, log_options);
			break;


		case es_RequestParameters:
			break;

		case es_WaitParameters:
			interpret_parametres_reply(buffer, length, log_options);
			break;
	}
}

void struna_process_command_byte(guint8 value, guint8* length)
{
	guint8 command = value & STRUNA_COMMAND_MASK;

	if (value & STRUNA_EXCEPTION_MASK)
	{
		*length = 1;
		safe_set_rx_state(rs_ReadData);
		return;
	}

	switch (safe_get_exchange_state())
	{
		case es_Idle:
			safe_set_rx_state(rs_None);
			break;

		case es_SelectChannel:
			if(command == sc_PresetSingleRegister)
			{
				*length = 4;
				safe_set_rx_state(rs_ReadData);
			}
			else
			{
				safe_set_rx_state(rs_None);
			}
			break;

		case es_RequestChannelConfiguration:
		case es_WaitChannelConfiguration:
		case es_RequestParameters:
		case es_WaitParameters:
			if(command == sc_ReadInputRegisters)
			{
				safe_set_rx_state(rs_WaitLength);
			}
			else
			{
				safe_set_rx_state(rs_None);
			}
			break;
	}
}

guint16 store_byte(guint8* buffer, guint16 crc, guint8 byte)
{
	buffer[0] = byte;
	return struna_crc_next(crc, byte);
}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[BUFFER_SIZE] = {0x00};
	guint8 buff[BUFFER_SIZE] = {0x00};
	guint8 length = 0;
	guint8 data_length = 0;
	guint16 crc = STRUNA_CRC_INIT;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch(safe_get_rx_state())
				{
					case rs_None:
						break;

					case rs_WaitAddress:
						if (buff[i] == STRUNA_ADDRESS)
						{
							safe_set_rx_state(rs_WaitCommand);
							length = 0;
							data_length = 0;
							crc = STRUNA_CRC_INIT;
							crc = store_byte(&buffer[length++], crc, buff[i]);

						}
						break;

					case rs_WaitCommand:
						crc = store_byte(&buffer[length++], crc, buff[i]);
						struna_process_command_byte(buff[i], &data_length);
						break;

					case rs_WaitLength:
						if (buff[i] < BUFFER_SIZE - 2)
						{
							crc = store_byte(&buffer[length++], crc, buff[i]);
							data_length = buff[i];
							safe_set_rx_state(rs_ReadData);
						}
						else
						{
							safe_set_rx_state(rs_None);
						}
						break;

					case rs_ReadData:
						crc = store_byte(&buffer[length++], crc, buff[i]);
						data_length--;
						if (data_length == 0)
						{
							safe_set_rx_state(rs_WaitCrcLow);
						}
						break;

					case rs_WaitCrcLow:
						if((crc & 0xFF) == buff[i])
						{
							safe_set_rx_state(rs_WaitCrcHigh);
						}
						else
						{
							safe_set_rx_state(rs_None);
						}
						break;

					case rs_WaitCrcHigh:


						if(((crc >> 8) & 0xFF) == buff[i])
						{
							interpret_reply(buffer, length, log_options);
						}

						safe_set_rx_state(rs_None);
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
