#include <glib.h>
#include <stdio.h>
#include <time.h>


#include "driver.h"
#include "logger.h"
#include "strunaplus.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

guint16 struna_crc_next(guint16 crc_value, guint8 symbol)
{
	guint8 counter = 8;

	crc_value ^= symbol;

	while(counter > 0)
	{
		if(crc_value & 0x0001) crc_value = (crc_value >> 1) ^ STRUNA_CRC_POLY;
		else crc_value >>= 1;

		counter--;
	}

	return crc_value;
}

gfloat parse_float_value(guint8* buffer)
{
	union
	{
	  gfloat f;
	  guint8 b[4];
	}
	u;

	u.b[0] = buffer[1];
	u.b[1] = buffer[0];
	u.b[2] = buffer[3];
	u.b[3] = buffer[2];

	return u.f;
}

