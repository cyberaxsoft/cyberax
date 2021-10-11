#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "strunaplus.h"
#include "strunaplus_func.h"

guint16 prepare_byte(guint8* buffer, guint16 crc, guint8 byte)
{
	buffer[0] = byte;
	return struna_crc_next(crc, byte);
}

guint8 prepare_select_channel_frame(guint8* buffer, guint8 active_channel)
{
	guint8 result = 0;

	guint16 crc = STRUNA_CRC_INIT;

	crc = prepare_byte(&buffer[result++], crc, STRUNA_ADDRESS);
	crc = prepare_byte(&buffer[result++], crc, sc_PresetSingleRegister);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, active_channel);

	buffer[result++] = crc & 0xFF;
	buffer[result++] = (crc >> 8 ) & 0xFF;

	return result;
}

guint8 prepare_channel_configuration_frame(guint8* buffer)
{
	guint8 result = 0;

	guint16 crc = STRUNA_CRC_INIT;

	crc = prepare_byte(&buffer[result++], crc, STRUNA_ADDRESS);
	crc = prepare_byte(&buffer[result++], crc, sc_ReadInputRegisters);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x03);

	buffer[result++] = crc & 0xFF;
	buffer[result++] = (crc >> 8 ) & 0xFF;

	return result;
}

guint8 prepare_request_parameters_frame(guint8* buffer)
{
	guint8 result = 0;

	guint16 crc = STRUNA_CRC_INIT;

	crc = prepare_byte(&buffer[result++], crc, STRUNA_ADDRESS);
	crc = prepare_byte(&buffer[result++], crc, sc_ReadInputRegisters);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x03);
	crc = prepare_byte(&buffer[result++], crc, 0x00);
	crc = prepare_byte(&buffer[result++], crc, 0x12);

	buffer[result++] = crc & 0xFF;
	buffer[result++] = (crc >> 8 ) & 0xFF;

	return result;
}

