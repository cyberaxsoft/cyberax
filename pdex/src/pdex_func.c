#include <glib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "driver.h"
#include "logger.h"
#include "pdex.h"
#include "config.h"
#include "driver_state.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}


void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{
		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
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

void bin_to_unpacked_bcd(guint8* buffer, guint8 length, guint32 value)
{
	while(length--)
	{
		*(buffer--) = (value % 10) | 0x30;
		value /= 10;
	}
}

void pdex_create_authorization_code(guint8* destination, guint8* source, guint8 addr)
{
	#define high(x) *((guint8*)&x+1)
	#define low(x) *((guint8*)&x)

	guint16 c = addr + 1;

	for (guint8 i = 0; i < PDEX_AUTHORIZATION_CODE_LENGTH; i++)
	{
		c = c * source[i];
		low(c) = low(c) & 0x7F;
		if ( low(c) < 0x20 ) c += 0x20;
		destination[i] = low(c);
		c = (c >> 8) + addr;
	}
}

guint16 pdex_crc(guint8* source, guint8 length)
{
	guint16 crc = 0;

	for (guint8 i = 0; i < length; i++)
	{
		crc += source[i];
	}
	return (((crc >> 8) & 0x7F) << 8) | ((crc & 0xFF) & 0x7F);
}

guint32 pdex_ascii_to_uint32(guint8* source, guint8 length)
{
	guint32 result = 0;

	for (guint8 i = 0; i < length; i++)
	{
		result = (result * 10) + (source[i] & 0x0F);
	}

	return result;
}
