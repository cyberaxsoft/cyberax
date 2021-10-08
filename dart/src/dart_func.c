#include <glib.h>
#include <stdio.h>
#include <sys/time.h>

#include "driver.h"
#include "logger.h"
#include "dart.h"
#include "config.h"
#include "driver_state.h"

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

void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size)
{
	size*=2;

	while(size--)
	{
		if (size & 0x01) *(buff) = val % 10;
		else *(buff--) |= (val % 10) << 4;
		val /= 10;
	}
}

guint32 packed_bcd_to_bin(guint8* buff, guint8 size)
{
	guint32 result = 0;

	size*=2;

	while(size--)
	{
		result *= 10;
		result+= (size & 0x01) ? *(buff) >> 4 : *(buff++) & 0x0F;
	}

	return result;
}

guint16 calc_crc(guint8* buffer, guint8 length)
{
	guint8 crc_table[] =
	{
			0x00, 0xCC, 0xD8, 0x14,
			0xF0, 0x3C, 0x28, 0xE4,
			0xA0, 0x6C, 0x78, 0xB4,
			0x50, 0x9C, 0x88, 0x44
	};

	guint8 crc_low = 0;
	guint8 crc_high = 0;
	guint8 symbol = 0, acc = 0;

	while(length--)
	{
		symbol = *buffer++ ^ crc_low;
		crc_low = symbol;
		symbol = crc_table[symbol & 0x0F];

		if (symbol & 0x80) crc_low ^= 0x10;

		acc = crc_low;
		crc_low = (symbol << 4) ^ crc_high;
		acc = crc_table[acc >> 4];

		if (acc & 0x80) crc_low ^=0x01;
		crc_high = acc ^ (symbol >> 4);
	}

	return (crc_high << 8) | crc_low;
}

guint8 prepare_pool_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Poll;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_status_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number)
{
	guint8 result = 0;

	guint16	crc = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dc_CommandToPump;
	buffer[result++] = 0x01;
	buffer[result++] = dcp_ReturnStatus;

	buffer[result++] = dc_CommandToPump;
	buffer[result++] = 0x01;
	buffer[result++] = dcp_ReturnFillingInfo;

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;



	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_command_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number)
{
	guint8 result = 0;

	guint16	crc = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dc_CommandToPump;
	buffer[result++] = 0x01;
	buffer[result++] = command;

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;


	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_suspend_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number, guint8 disp_index)
{
	guint8 result = 0;

	guint16	crc = 0;

//	gint8	nozzle_index = 0;
//	guint32 price = 0;
//	guint32 volume = 0;
//	guint32 amount = 0;

//	safe_get_preset(disp_index, &nozzle_index, &price, &volume, &amount);
//	guint8 nozzle = safe_get_nozzle_num(disp_index, nozzle_index);

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dcp_Suspend;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;


	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_resume_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number, guint8 disp_index)
{
	guint8 result = 0;

	guint16	crc = 0;

//	gint8	nozzle_index = 0;
//	guint32 price = 0;
//	guint32 volume = 0;
//	guint32 amount = 0;

//	safe_get_preset(disp_index, &nozzle_index, &price, &volume, &amount);
//	guint8 nozzle = safe_get_nozzle_num(disp_index, nozzle_index);

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dcp_Resume;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;


	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_ack_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number)
{
	guint8 result = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Ack | block_sequence_number;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_nak_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number)
{
	guint8 result = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Nak | block_sequence_number;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_get_counter_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number, guint8 nozzle_index)
{
	guint8 result = 0;

	guint16	crc = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dc_TotalCounters;
	buffer[result++] = 0x01;
	buffer[result++] = nozzle_index + 1;

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;


	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_ack_pool_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number)
{
	guint8 result = 0;

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_AckPoll | block_sequence_number;
	buffer[result++] = CTRL_SF;

	return result;
}
guint8 prepare_set_prices_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, LogOptions log_options)
{
	guint8 result = 0;

	guint16	crc = 0;

	guint8 nozzle_count = safe_get_nozzle_count(disp_index);

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dc_PriceUpdate;
	buffer[result++] = nozzle_count * 3;

	for (guint8 i = 0; i < nozzle_count; i++)
	{
		guint32 nozzle_price = safe_get_nozzle_price(disp_index, i);

		buffer[result++] = 0x00;
		buffer[result++] = 0x00;
		buffer[result++] = 0x00;

		bin_to_packet_bcd(nozzle_price, &buffer[result-1], DART_PRICE_LENGTH);

		add_log(TRUE, TRUE, log_options.trace, log_options.system, "	Price %d: %d", i+1, nozzle_price);
	}

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;

	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;
}

guint8 prepare_preset_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, DartCommand command, LogOptions log_options)
{
	guint8 result = 0;

	guint16	crc = 0;

	gint8	nozzle_index = 0;
	guint32 price = 0;
	guint32 volume = 0;
	guint32 amount = 0;

	safe_get_preset(disp_index, &nozzle_index, &price, &volume, &amount);

	guint8 nozzle = safe_get_nozzle_num(disp_index, nozzle_index);
	guint8 nozzle_count = safe_get_nozzle_count(disp_index);

	buffer[result++] = (disp_addr - 1) | DART_ADDRESS_OFFSET;
	buffer[result++] = dct_Data | block_sequence_number;
	buffer[result++] = dc_PriceUpdate;
	buffer[result++] = nozzle_count * 3;

	for (guint8 i = 0; i < nozzle_count; i++)
	{
		guint32 nozzle_price = safe_get_nozzle_price(disp_index, i);

		buffer[result++] = 0x00;
		buffer[result++] = 0x00;
		buffer[result++] = 0x00;

		bin_to_packet_bcd(nozzle_price, &buffer[result-1], DART_PRICE_LENGTH);

		add_log(TRUE, TRUE, log_options.trace, log_options.system, "	Price %d: %d", i+1, nozzle_price);
	}

	buffer[result++] = dc_AllowedNozzle;
	buffer[result++] = 0x01;
	buffer[result++] = nozzle;

	buffer[result++] = command;
	buffer[result++] = 0x04;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;

	if(command == dc_PresetVolume)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "	Volume: %d", volume);
		bin_to_packet_bcd(volume, &buffer[result-1], DART_VOLUME_LENGTH);
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.system, "	Amount: %d", amount);
		bin_to_packet_bcd(amount, &buffer[result-1], DART_VOLUME_LENGTH);
	}

	crc = calc_crc(buffer, result);

	if ((crc & 0xFF) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc & 0xFF;
	if ((crc >> 8) == CTRL_SF)
	{
		buffer[result++] = CTRL_DLE;
	}
	buffer[result++] = crc >> 8;

	buffer[result++] = CTRL_ETX;
	buffer[result++] = CTRL_SF;

	return result;

}

void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{
		case 0x00: memcpy(buffer, "No error", strlen("No error")); break;
		case 0x03: memcpy(buffer, "Ram error", strlen("Ram error")); break;
		case 0x04: memcpy(buffer, "PROM checksum error", strlen("PROM checksum error")); break;
		case 0x06: memcpy(buffer, "Pulser error", strlen("Pulser error")); break;
		case 0x07: memcpy(buffer, "Pulser current error", strlen("Pulser current error")); break;
		case 0x09: memcpy(buffer, "Emergency stop", strlen("Emergency stop")); break;
		case 0x0A: memcpy(buffer, "Power failure", strlen("Power failure")); break;
		case 0x0B: memcpy(buffer, "Pressure lost", strlen("Pressure lost")); break;
		case 0x0C: memcpy(buffer, "Blend ratio error", strlen("Blend ratio error")); break;
		case 0x0D: memcpy(buffer, "Low leak error", strlen("Low leak error")); break;
		case 0x0E: memcpy(buffer, "High leak error", strlen("High leak error")); break;
		case 0x0F: memcpy(buffer, "Hose leak error", strlen("Hose leak error")); break;
		case 0x10: memcpy(buffer, "VR monitor, error reset", strlen("VR monitor, error reset")); break;
		case 0x11: memcpy(buffer, "VR monitor, 10 consecutive error", strlen("VR monitor, 10 consecutive error")); break;
		case 0x12: memcpy(buffer, "VR monitor, shut down pump", strlen("VR monitor, shut down pump")); break;
		case 0x13: memcpy(buffer, "VR monitor, internal error", strlen("VR monitor, internal error")); break;

		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
	}
}
//dds_NotProgrammed		= 0x00,
//dds_Reset				= 0x01,
//dds_Authorized			= 0x02,
//dds_Filling				= 0x04,
//dds_FillingComplete		= 0x05,
//dds_MaxVolReached		= 0x06,
//dds_SwitchedOff			= 0x07,

void get_original_pump_status_description(guint8 status, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (status)
	{
		case dds_NotProgrammed: 	memcpy(buffer, "Not programmed", strlen("Not programmed")); break;
		case dds_Reset: 			memcpy(buffer, "Reset", strlen("Reset")); break;
		case dds_Authorized: 		memcpy(buffer, "Authorized", strlen("Authorized")); break;
		case dds_Filling: 			memcpy(buffer, "Filling", strlen("Filling")); break;
		case dds_FillingComplete: 	memcpy(buffer, "Filling Complete", strlen("Filling Complete")); break;
		case dds_MaxVolReached: 	memcpy(buffer, "Max Vol Reached", strlen("Max Vol Reached")); break;
		case dds_SwitchedOff: 		memcpy(buffer, "Switched Off", strlen("Switched Off")); break;

		default: memcpy(buffer, "Undefined", strlen("Undefined")); break;
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



