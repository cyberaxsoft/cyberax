#include <glib.h>
#include <sys/time.h>
#include <hidapi.h>

#include "logger.h"
#include "port_router.h"


guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
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

guint16 crc16_next(guint16 crc16_value, guint8 symbol)
{
	guint8 Counter;

	crc16_value ^= (guint16) symbol << 8;

	for(Counter = 8; Counter--; )
	{
		if(crc16_value & 0x8000) { crc16_value <<= 1; crc16_value ^= CRC16_POLY;}
		else                       crc16_value <<= 1;
	}

	return crc16_value;
}

guint16 crc16_next_word(guint16 crc, guint16 value)
{
	crc = crc16_next(crc, (guint8) (value & 0xFF)); value >>= 8;
	crc = crc16_next(crc, (guint8) (value & 0xFF));

	return crc;
}

guint16 crc16_next_dword(guint16 crc, guint32 value)
{
	crc = crc16_next(crc, (guint8) (value & 0xFF)); value >>= 8;
	crc = crc16_next(crc, (guint8) (value & 0xFF)); value >>= 8;
	crc = crc16_next(crc, (guint8) (value & 0xFF)); value >>= 8;
	crc = crc16_next(crc, (guint8) (value & 0xFF));

	return crc;
}

guint16 crc16_calc(guint8 *buffer, gsize length)
{
	guint16 crc = CRC16_INIT;

	while(length--)
	{
		crc = crc16_next(crc, *(buffer++));
	}

	return crc;
}
