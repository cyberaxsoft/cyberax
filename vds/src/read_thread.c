#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "vds.h"
#include "config.h"
#include "vds_func.h"
#include "driver_state.h"

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
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Interpret reply:");

	guint8 layer_num = vds_unpacked_bcd_to_bin(&buffer[LAYER_OFFSET], LAYER_LENGTH);
	guint8 field_num = vds_unpacked_bcd_to_bin(&buffer[POLE_NUM_OFFSET], POLE_NUM_LENGTH);
	guint8 status = vds_unpacked_bcd_to_bin(&buffer[COMMAND_OFFSET], COMMAND_LENGTH);
	guint8 symbol_count = vds_unpacked_bcd_to_bin(&buffer[SYMBOL_COUNT_OFFSET], SYMBOL_COUNT_LENGTH);
	guint32 price = vds_unpacked_bcd_to_bin(&buffer[PRICE_OFFSET], symbol_count);

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Layer num = %d, field num = %d, status = %d, symbol count = %d, price = %d", layer_num, field_num, status, symbol_count, price);

	guint8 price_pole_index = 0;

	if (layer_num == DEF_LAYER)
	{
		if (safe_get_price_pole_index_by_num(field_num, &price_pole_index) == de_NoError)
		{
			safe_set_state(price_pole_index, pps_Online);
			safe_set_command_sent(price_pole_index, FALSE);
			safe_set_uart_error(price_pole_index, 0);

			if (status == 0)
			{
				//check symbol count
				if (symbol_count != safe_get_price_pole_symbol_count(price_pole_index))
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Symbol count configuration price pole index %d error!!! Set new value %d", price_pole_index, symbol_count);
					safe_set_price_pole_symbol_count(price_pole_index, symbol_count);
				}

				//check price
				if (safe_get_price(price_pole_index) == 0)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Update price in driver for price_pole_index %d! Set new value %d", price_pole_index, price);
					safe_set_price(price_pole_index, price);
				}
				else
				{
					if (safe_get_price(price_pole_index) != price)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Update price in price_pole_index %d! Set new value %d", price_pole_index, safe_get_price(price_pole_index));
						safe_set_exchange_state(price_pole_index, ves_SetPrice);
					}
					else
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Price in price_pole_index %d not changed", price_pole_index);
					}
				}
			}
			else
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Error! Status price pole index %d!", price_pole_index);
			}
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Error! Undefined field number!");
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Error! Layer number error!");
	}

}

void save_log(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");
}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	VdsFrameStage vfs = vfs_WaitSTX;

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch (vfs)
				{
					case vfs_WaitSTX:
						if (buff[i] == ctrl_STX)
						{
							length = 0;
							buffer[length++] = buff[i];
							vfs = vfs_WaitETX;
						}
						break;

					case vfs_WaitETX:
						if (length < READ_BUFFER_SIZE - 1)
						{
							buffer[length++] = buff[i];
							if (buff[i] == ctrl_ETX)
							{
								vfs = vfs_WaitLRC;
							}
						}
						else
						{
							vfs = vfs_WaitSTX;
						}
						break;

					case vfs_WaitLRC:
						buffer[length++] = buff[i];
						save_log(buffer, length, log_options);

						if (buff[i] == calc_crc(buffer, length))
						{
							interpret_reply(buffer, length, log_options);
						}
						else
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "CRC error!");
						}

						vfs = vfs_WaitSTX;
						break;
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}
