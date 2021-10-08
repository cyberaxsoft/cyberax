#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "driver.h"
#include "pdex.h"
#include "config.h"
#include "driver_state.h"
#include "pdex_func.h"

guint8 pdex_prepare_ctrl(guint8* buffer, guint8 channel, guint8 symbol)
{
	guint8 result = 0;

	buffer[result++] = ctrl_SYN;
	buffer[result++] = ctrl_SYN;
	buffer[result++] = PDEX_START_ADDRESS + channel;
	buffer[result++] = symbol;

	return result;
}

guint8 pdex_prepare_command(guint8* buffer, guint8 channel, guint8 command, guint8* data, guint8 data_size)
{
	guint8 result = 0;

	guint16 crc = ctrl_STX;

	buffer[result++] = ctrl_SYN;
	buffer[result++] = ctrl_SYN;
	buffer[result++] = PDEX_START_ADDRESS + channel;

	buffer[result++] = ctrl_STX;
	buffer[result++] = command; crc += command;

	while(data_size--)
	{
		guint8 byte = *(data++);
		buffer[result++] = byte; crc += byte;
	}

	buffer[result++] = ctrl_ETX; crc += ctrl_ETX;


	crc = (((crc >> 8) & 0x7F) << 8) | ((crc & 0xFF) & 0x7F);

	buffer[result++] =  crc >> 8;
	buffer[result++] = crc & 0xFF;

	return result;

}
