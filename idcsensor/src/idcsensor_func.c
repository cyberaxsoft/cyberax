#include <glib.h>
#include <stdio.h>
#include <time.h>


#include "driver.h"
#include "logger.h"
#include "idcsensor.h"

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

gfloat parse_hex_to_float(guint8* buffer)
{
	union
	{
		gfloat f;
		gchar b[4];
	}u;

	memset(u.b, 0x00, sizeof(u));

	memcpy(u.b, buffer, sizeof(gfloat));


	return u.f;
}

