#include <glib.h>
#include <stdio.h>
#include <sys/time.h>
#include <math.h>

#include "driver.h"
#include "logger.h"
#include "vds.h"
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

	for (guint8 i = 1; i < length - 1; i++)
	{
		result ^= buffer[i];
	}

	return result;
}



guint32 vds_unpacked_bcd_to_bin(guint8* string, guint8 length)
{
	guint32 value = 0;

	while (length--)
	{
		value = (value << 1) + (value << 3);
		value += *(string++) & 0x0F;
	}

	return value;
}

guint8 vds_bin_to_unpacked_bcd(guint8* buffer, guint32 value, guint8 length)
{
//	length *= 2;

	guint8 result = length;

	while (length)
	{
		guint8 byte = (value % 10) | 0x30;
		buffer[--length] = byte;
		value /= 10;
	}

	return result;
}


const gchar* pps_to_str(PricePoleState state)
{
	switch(state)
	{
		case pps_NotInitialize: return "NotInitialize";
		case pps_Offline: return "Offline";
		case pps_Online: return "Online";
		default: return "Undefined";
	}
}

guint8 prepare_get_price_command(guint8 index, guint8* buffer)
{
	guint8 pos = 0;

	guint8 num = safe_get_price_pole_num(index);
	guint8 symbol_count = safe_get_price_pole_symbol_count(index);

	buffer[pos++] = ctrl_STX;

	pos += vds_bin_to_unpacked_bcd(&buffer[pos], DEF_LAYER, LAYER_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], num, POLE_NUM_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], vcc_GetPrice, COMMAND_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], symbol_count, SYMBOL_COUNT_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], 0, symbol_count);
	buffer[pos++] = ctrl_ETX;

	guint8 crc = calc_crc(buffer, pos + 1);
	buffer[pos++] = crc;

	return pos;
}

void set_point(guint8 symbol_count, guint8* buffer)
{

	guint8 price_dp = 0;
	safe_get_decimal_point_positions(&price_dp);

	buffer[PRICE_OFFSET + symbol_count - price_dp - 1] |= 0x80;
}

guint8 prepare_set_price_command(guint8 index, guint8* buffer)
{
	guint8 pos = 0;

	guint8 num = safe_get_price_pole_num(index);
	guint8 symbol_count = safe_get_price_pole_symbol_count(index);
	guint32 price = safe_get_price(index);

	buffer[pos++] = ctrl_STX;

	pos += vds_bin_to_unpacked_bcd(&buffer[pos], DEF_LAYER, LAYER_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], num, POLE_NUM_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], vcc_SetPrice, COMMAND_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], symbol_count, SYMBOL_COUNT_LENGTH);
	pos += vds_bin_to_unpacked_bcd(&buffer[pos], price, symbol_count);
	buffer[pos++] = ctrl_ETX;

	set_point(symbol_count, buffer);

	guint8 crc = calc_crc(buffer, pos + 1);
	buffer[pos++] = crc;

	return pos;
}

