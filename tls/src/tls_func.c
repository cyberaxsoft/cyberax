#include <glib.h>
#include <stdio.h>
#include <time.h>


#include "driver.h"
#include "logger.h"
#include "tls.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

guint8 ascii_to_int(guint8* buffer, guint8 length)
{
	guint8 result = 0;

	for (guint8 i = 0; i < length; i++)
	{
		result = (result * 10) + (buffer[i] - 0x30);
	}

	return result;
}

gfloat parse_float_hex_string(guint8* buffer)
{
	union
	{
		gfloat f;
		gchar b[4];
	}u;

	guint8 length = REPLY_DATA_FIELD_LENGTH;

	memset(u.b, 0x00, sizeof(u));

	while (length--)
	{
		guchar ascii = *buffer++;
		guchar value = 0;

		if (ascii >= 0x30 && ascii <= 0x39)
		{
			value = ascii - 0x30;
		}
		else
		{
			value = ascii - 0x41 + 10;
		}

		guchar index = length >> 1;

		if ((length & 0x01) != 0)
		{
			u.b[index] = value << 4;
		}
		else
		{
			u.b[index] |= value;
		}

	}

	return u.f;
}

