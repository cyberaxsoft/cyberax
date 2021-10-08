#include <glib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include "driver.h"
#include "logger.h"
#include "topaz.h"
#include "config.h"
#include "driver_state.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

guint8 calc_crc(guint8* buffer, guint8 length)
{
	guint8 result = 0;

	if (length > 2)
	{
		for (guint8 i = 2; i < length; i++)
		{
			result ^= buffer[i];
		}
	}

	return result | 0x40;
}

guint8 add_byte(guint8* destination, guint8 value, guint8* crc)
{
	guint8 result = 0;

	destination[result++] = value;
	*crc ^= value;
	destination[result++] = value ^ 0x7F;

	return result;
}

guint8 get_stx(guint8 channel)
{
	if (channel < 16)
	{
		return ctrl_STX;
	}
	else
	{
		return ctrl_ACK + (guint8)(channel / 15);
	}
}


guint8 get_addr(guint8 channel)
{
	if (channel < 16)
	{
		return 0x20 | channel;
	}
	else
	{
		return 0x20 | (channel % 15);
	}
}

guint32 azt_unpacked_bcd_to_bin(guint8* string, guint8 length)
{
	guint32 value = 0;

	while (length--)
	{
		value = (value << 1) + (value << 3);
		value += *(string++) & 0x0F;
	}

	return value;
}

guint8 azt_bin_to_unpacked_bcd(guint8* buffer, guint32 value, guint8* crc, guint8 length)
{
	length *= 2;

	guint8 result = length;

	while (length)
	{

		guint8 byte = (value % 10) | 0x30;
		buffer[--length] = byte ^ 0x7F;
		buffer[--length] = byte;
		*crc ^= byte;
		value /= 10;
	}

	return result;
}


void prepare_command_request(guint8* destination, guint8 channel, guint8* length, AztCommandCode command_code)
{
	guint8 pos = 0;
	guint8 crc = 0;

	guint8 stx = get_stx(channel);
	guint8 addr = get_addr(channel);


	destination[pos++] = ctrl_DEL;
	destination[pos++] = stx;

	pos += add_byte(&destination[pos], addr, &crc);
	pos += add_byte(&destination[pos], command_code, &crc);

	destination[pos++] = ctrl_ETX; crc ^= ctrl_ETX;
	destination[pos++] = ctrl_ETX;

	destination[pos++] = crc | 0x40;

	*length = pos;
}

void prepare_command_param_request(guint8* destination, guint8 channel, guint8* length, AztCommandCode command_code, guint32 value, guint8 size)
{
	guint8 pos = 0;
	guint8 crc = 0;

	guint8 stx = get_stx(channel);
	guint8 addr = get_addr(channel);

	destination[pos++] = ctrl_DEL;
	destination[pos++] = stx;

	pos += add_byte(&destination[pos], addr, &crc);
	pos += add_byte(&destination[pos], command_code, &crc);

	guint32 max_value = (guint32)pow(10, size) - 1;
	if (value > max_value) value = max_value;

	pos += azt_bin_to_unpacked_bcd(&destination[pos], value, &crc, size);

	destination[pos++] = ctrl_ETX; crc ^= ctrl_ETX;
	destination[pos++] = ctrl_ETX;

	destination[pos++] = crc | 0x40;

	*length = pos;
}



const gchar* bool_to_str(gboolean value)
{
	if (value)
	{
		return "True";
	}
	else
	{
		return "False";
	}
}


const gchar* ds_to_str(DispencerState state)
{
	switch(state)
	{
		case ds_NotInitialize: return "NotInitialize";
		case ds_Busy: return "Busy";
		case ds_Free: return "Free";
		case ds_NozzleOut: return "NozzleOut";
		case ds_Filling: return "Filling";
		case ds_Stopped: return "Stopped";
		case ds_Finish: return "Finish";
		case ds_ConnectionError: return "ConnectionError";
		default: return "Undefined";
	}
}

void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{
		case 0x00: memcpy(buffer, "No error", strlen("No error")); break;

		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
	}
}


