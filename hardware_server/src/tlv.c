#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "tlv.h"
#include "system_func.h"

const guint16 crc_table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
};

guint16 calc_crc(guchar* buffer, guint16 start_pos, guint16 length)
{
	guint16 crc = 0;

	for (guint32 i = start_pos; i < start_pos + length; ++i)
	{
		guint8 index = crc ^ buffer[i];
		crc = (guint16) ((crc >> 8 ) ^ crc_table[index]);
	}

	return crc;
}

guint16 calc_crc_next(guchar byte, guint16 crc)
{
	guint16 result = crc;

	guint8 index = crc ^ byte;
	result = (guint16) ((crc >> 8 ) ^ crc_table[index]);

	return result;
}

guint16 calc_new_crc(guchar* buffer, guint16 start_pos, guint16 length)
{
	guint16 crc = 0;

	for (guint32 i = start_pos; i < start_pos + length; i++)
	{
		crc ^= buffer[i];
		for (guint8 i = 8; i > 0; i-- )
		{
			if ((crc & 0x0001) != 0)
			{
				crc >>=1;
				crc ^= 0xa001;
			}
			else
			{
				crc >>=1;
			}
		}
	}
	return crc;
}

guint16 calc_new_crc_next(guchar byte, guint16 crc)
{
	guint16 result = crc;

	result ^= byte;

	for (guint8 i = 8; i > 0; i-- )
	{
		if ((result & 0x0001) != 0)
		{
			result >>=1;
			result ^= 0xa001;
		}
		else
		{
			result >>=1;
		}
	}
	return result;
}


void tlv_copy_unit(TlvUnit* destination, TlvUnit* source)
{
	destination->tag = source->tag;
	destination->length = source->length;
	destination->value = source->value;
}

guint32 tlv_calc_list_count(TlvUnit* first_unit)
{
	guint32 result = 0;

	if (first_unit!=NULL)
	{
		gpointer next = first_unit->next;
		result++;

		while(next!=NULL)
		{
			next = ((TlvUnit*)next)->next;
			result++;
		}

	}

	return result;
}

TlvUnit* tlv_find_last_unit(TlvUnit* first_unit)
{
	if (first_unit == NULL) return NULL;

	TlvUnit* result = first_unit;

	while(result->next!=NULL)
	{
		result = result->next;
	}

	return (TlvUnit*)result;
}

void tlv_add_unit(TlvUnit** first_unit, TlvUnit* unit)
{
	if (*first_unit == NULL)
	{
		*first_unit = unit;
	}
	else
	{
		TlvUnit* last = tlv_find_last_unit(*first_unit);
		last->next = (gpointer)unit;
	}
}

gchar* return_message_type_name(MessageType type)
{
	gchar* result = NULL;

	switch(type)
	{
		case mt_Undefined:	result = g_strdup("Undefined"); break;
		case mt_Request:	result = g_strdup("Request"); break;
		case mt_Reply:		result = g_strdup("Reply"); break;
		case mt_Ack:		result = g_strdup("Ack"); break;
		case mt_Nak:		result = g_strdup("Nak"); break;
		case mt_Event:		result = g_strdup("Event"); break;
		case mt_Eot:		result = g_strdup("Eot"); break;
		default:			result = g_strdup("Undefined"); break;
	}

	return result;
}

TlvUnit* tlv_create_unit(guint16 tag, guchar* buffer, guint32 start_pos, guint32 length)
{

	TlvUnit* result = (TlvUnit*)malloc(sizeof(TlvUnit));

	if (result!=NULL)
	{
		result->tag = tag;
		result->length = length;
		result->next = NULL;

		result->value = (guchar*)malloc(length);
		if (result->value != NULL)
		{
			memcpy(result->value, &buffer[start_pos], length);
		}
		else
		{
			free(result);
			result = NULL;
		}
	}

	return result;

}

void tlv_delete_units(TlvUnit** first_unit)
{
	if (*first_unit == NULL) return;

	TlvUnit* curr_unit = *first_unit;

	do
	{
		if (curr_unit->value != NULL)
		{
			g_free(curr_unit->value);
			curr_unit->value = NULL;
		}

		TlvUnit* tmp = curr_unit->next;
		g_free(curr_unit);
		curr_unit = tmp;

	}while(curr_unit != NULL);

}


ExchangeError tlv_parse_frame(guchar* buffer, guint16 buffer_length, TlvUnit** destination )
{
	ParseState ps = ps_ReadTag1;

	guint16 current_tag = 0;
	guint32 tag_length = 0;
	guint8 length_field_size = 0;

	if (buffer_length > 0)
	{
		for (guint32 i = 0; i < buffer_length; i++)
		{
			switch(ps)
			{
				case ps_ReadTag1:
					current_tag = buffer[i];
					ps = ps_ReadTag2;
					break;

				case ps_ReadTag2:
					current_tag = ( current_tag << 8 ) | buffer[i];
					ps = ps_ReadLen;
					break;

				case ps_ReadLen:
					if (buffer[i] < 0x80)
					{
						tag_length = buffer[i];
						if (tag_length > 0)
						{
							ps = ps_ReadData;
						}
						else
						{
							ps = ps_ReadTag1;
						}
					}
					else
					{
						length_field_size = buffer[i] & 0x0F;
						tag_length = 0;
						ps = ps_ReadLenEx;
					}
					break;

				case ps_ReadLenEx:
					tag_length = (tag_length << 8) | buffer[i];
					length_field_size--;
					if (length_field_size == 0)
					{
						ps = ps_ReadData;
					}
					break;

				case ps_ReadData:
					if (i + tag_length <= buffer_length)
					{
						TlvUnit* unit = tlv_create_unit(current_tag, &buffer[i], 0, tag_length);

						tlv_add_unit(destination, unit);

						i+=tag_length - 1;
						ps = ps_ReadTag1;
					}
					else
					{
						return ee_Parse;
					}
					break;
			}
		}
	}
	else
	{
		return ee_Parse;
	}



	return ee_None;

}

const int is_bigendian_val = 1;
#define is_bigendian() ( (*(gchar*)&is_bigendian_val) == 0 )

gfloat tlv_bin_to_float(guchar* buffer)
{
	union result
	{
		gfloat value;
		guchar buf[sizeof(gfloat)];
	}res;

	res.value = 0;

	if (is_bigendian())
	{
		memcpy(res.buf, buffer, sizeof(gfloat));
	}
	else
	{
		guchar tmp[] = {buffer[0], buffer[1], buffer[2], buffer[3]};
		memcpy(res.buf, tmp, sizeof(gfloat));
	}
	return res.value;
}

void tlv_float_to_bin(gfloat value, guchar* buffer)
{
	union result
	{
		gfloat value;
		guchar buf[sizeof(gfloat)];
	}res;

	res.value = value;

	if (is_bigendian())
	{
		memcpy(buffer, res.buf, sizeof(gfloat));
	}
	else
	{
		guchar tmp[] = {res.buf[0], res.buf[1], res.buf[2], res.buf[3]};
		memcpy(buffer, tmp, sizeof(gfloat));
	}
}


gint8 tlv_bool_to_byte(gboolean value)
{
	if (value)
	{
		return 0x01;
	}
	else
	{
		return 0x00;
	}
}


guint8 tlv_calc_len(guint32 length)
{
	guint8 result = 0;

	if (length < 0x80)
	{
		result = 1;
	}
	else if (length >= 0x80 && length <= 0xFF )
	{
		result = 2;
	}
	else if (length > 0xFF && length <= 0xFFFF)
	{
		result = 3;
	}
	else
	{
		result = 5;
	}
	return result;
}

guchar* tlv_serialize_unit(TlvUnit* unit, guint32* result_length)
{
	guchar* result = NULL;

	guint8 field_len_size = tlv_calc_len(unit->length);
	guint32 frame_length = unit->length + field_len_size + sizeof(guint16);

	result = malloc(frame_length);
	guint32 pos = 0;


	if (result != NULL)
	{

		result[pos++] = (unit->tag >> 8) & 0xFF;
		result[pos++] = unit->tag & 0xFF;

		switch(field_len_size)
		{
			case 1:
				result[pos++] = (guchar)(unit->length & 0xFF);
				break;

			case 2:
				result[pos++] = 0x81;
				result[pos++] = unit->length & 0xFF;
				break;

			case 3:
				result[pos++] = 0x82;
				result[pos++] = (unit->length >> 8 ) & 0xFF;
				result[pos++] = unit->length & 0xFF;
				break;

			case 5:
				result[pos++] = 0x84;
				result[pos++] = (unit->length >> 24 ) & 0xFF;
				result[pos++] = (unit->length >> 16 ) & 0xFF;
				result[pos++] = (unit->length >> 8 ) & 0xFF;
				result[pos++] = unit->length & 0xFF;
				break;
		}

		if (unit->length > 0)
		{
			memcpy(&result[pos], unit->value, unit->length);
			pos+=unit->length;
		}
	}

	*result_length = pos;

	return result;
}

guint32 tlv_calc_unit_size(TlvUnit* unit)
{
	return unit->length + tlv_calc_len(unit->length) + sizeof(guint16);
}


guint32 tlv_serialize_units_size(TlvUnit* first_unit)
{
	guint32 result = 0;

	if (first_unit!=NULL)
	{
		TlvUnit* next_unit = first_unit;
		result += tlv_calc_unit_size(next_unit);

		while(next_unit->next!=NULL)
		{
			next_unit = next_unit->next;
			result += tlv_calc_unit_size(next_unit);
		}

	}

	return result;
}

void tlv_add_serialize_unit(TlvUnit* unit, guchar* buffer, guint32* current_pos )
{
	guint32 size = 0;
	guchar* seriailize_unit_frame = tlv_serialize_unit(unit, &size);

	if (seriailize_unit_frame!=NULL)
	{
		memcpy(buffer, seriailize_unit_frame, size);
		current_pos += size;
		free(seriailize_unit_frame);
	}

}

guchar* tlv_serialize_units(TlvUnit* first_unit, guint32* size)
{
	guint32 len = tlv_serialize_units_size(first_unit);
	guchar* result = malloc(len);
	guint32 pos = 0;

	if (first_unit!=NULL)
	{
		TlvUnit* next_unit = first_unit;
		tlv_add_serialize_unit(next_unit, &result[pos], &pos);
		pos += tlv_calc_unit_size(next_unit);

		while(next_unit->next!=NULL)
		{
			next_unit = next_unit->next;
			tlv_add_serialize_unit(next_unit, &result[pos], &pos);

			pos += tlv_calc_unit_size(next_unit);
		}

	}

	*size = pos;
	return result;

}

guchar* tlv_create_transport_frame(TlvUnit* first_unit, guint32* size)
{
	guint32 tlv_frame_len = 0;
	guchar* tlv_frame = tlv_serialize_units(first_unit, &tlv_frame_len);

	guchar* result = malloc(tlv_frame_len + 8);
	guint32 pos = 0;

	result[pos++] = ctrl_STX;

	result[pos++] = ( tlv_frame_len >> 24 ) & 0xFF;
	result[pos++] = ( tlv_frame_len >> 16 ) & 0xFF;
	result[pos++] = ( tlv_frame_len >> 8 ) & 0xFF;
	result[pos++] =  tlv_frame_len & 0xFF;


	if ( tlv_frame != NULL && tlv_frame_len > 0)
	{
		memcpy(&result[pos], tlv_frame, tlv_frame_len);
		pos+=tlv_frame_len;

		g_free(tlv_frame);
	}

	result[pos++] = ctrl_ETX;

	guint16 crc =  calc_crc(result, 1, pos - 1);

	result[pos++] =  (crc >> 8) & 0xFF;
	result[pos++] =  crc & 0xFF;

	*size = pos;
	return result;
}


void tlv_test()
{
	TlvUnit* units = NULL;

	guchar buffer[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35};


	for (guint16 i = 0; i< 3; i++)
	{
		tlv_add_unit(&units, tlv_create_unit(i, buffer, 0, sizeof(buffer)));

		g_printf(" count = %d\n", tlv_calc_list_count(units));

	}


	TlvUnit* last = tlv_find_last_unit(units);

	g_printf(" last: tag = %04x length = %d value = %s \n", last->tag, last->length, last->value);

	g_printf(" All tags:\n");

	if (units != NULL)
	{
		TlvUnit* result = units;

		do
		{
			g_printf(" next: tag = %04x length = %d value = %s \n", result->tag, result->length, result->value);

			guint32 len = 0;
			guchar* buf = tlv_serialize_unit(result, &len );

			if (buf!=NULL)
			{
				if (len > 0)
				{
					g_printf(" serialize:");
					for (guint32 i = 0; i < len; i++)
					{
						g_printf(" %02x", buf[i]);
					}
					g_printf(" \n");
				}

				free(buf);
				buf = NULL;
			}

			result = result->next;

		}while(result!=NULL);

	}

	guint32 size = 0;
	guchar* finish_frame =  tlv_create_transport_frame(units, &size);

	if (finish_frame!=NULL && size>0)
	{
		g_printf(" transport:");
		for (guint32 i = 0; i < size; i++)
		{
			g_printf(" %02x", finish_frame[i]);
		}
		g_printf(" \n");

	}

	free(finish_frame);

	tlv_delete_units(&units);

	g_printf(" count = %d\n", tlv_calc_list_count(units));

}

void add_tlv_unit_32(TlvUnit** units, guint16 tag, guint32 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	va_list args;
	va_start(args, format);
	gchar* string = g_strdup_vprintf(format, args);
	va_end(args);

	guchar buffer[] = {(value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF };
	tlv_add_unit(units, tlv_create_unit(tag , buffer, 0, sizeof(buffer)));
	add_log(log_params, TRUE, TRUE, log_trace, active, string);

	g_free(string);
}

void add_tlv_unit_8(TlvUnit** units, guint16 tag, guint8 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	va_list args;
	va_start(args, format);
	gchar* string = g_strdup_vprintf(format, args);
	va_end(args);

	guchar buffer[] = { value };
	tlv_add_unit(units, tlv_create_unit(tag , buffer, 0, sizeof(buffer)));
	add_log(log_params, TRUE, TRUE, log_trace, active, string);

	g_free(string);
}

void add_tlv_unit_16(TlvUnit** units, guint16 tag, guint16 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	va_list args;
	va_start(args, format);
	gchar* string = g_strdup_vprintf(format, args);
	va_end(args);

	guchar buffer[] = { value >> 8, value & 0xFF };
	tlv_add_unit(units, tlv_create_unit(tag , buffer, 0, sizeof(buffer)));
	add_log(log_params, TRUE, TRUE, log_trace, active, string);

	g_free(string);
}


void add_tlv_unit_str(TlvUnit** units, guint16 tag, gchar* str, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	if (str !=NULL)
	{
		va_list args;
		va_start(args, format);
		gchar* string = g_strdup_vprintf(format, args);
		va_end(args);

		tlv_add_unit(units, tlv_create_unit(tag , (guchar*)str, 0, count_utf8_code_points(str)));
		add_log(log_params, TRUE, TRUE, log_trace, active, string);

		g_free(string);
	}

}

void add_tlv_unit_bytes(TlvUnit** units, guint16 tag, guint8* buffer, guint8 lenght, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	if (buffer !=NULL)
	{
		va_list args;
		va_start(args, format);
		gchar* string = g_strdup_vprintf(format, args);
		va_end(args);

		tlv_add_unit(units, tlv_create_unit(tag , (guchar*)buffer, 0, lenght));
		add_log(log_params, TRUE, TRUE, log_trace, active, string);

		g_free(string);
	}

}

void add_tlv_unit_float(TlvUnit** units, guint16 tag, gfloat value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...)
{
	va_list args;
	va_start(args, format);
	gchar* string = g_strdup_vprintf(format, args);
	va_end(args);

	guchar buffer[sizeof(gfloat)] = { 0x00 };
	tlv_float_to_bin(value, buffer);
	tlv_add_unit(units, tlv_create_unit(tag , buffer, 0, sizeof(buffer)));
	add_log(log_params, TRUE, TRUE, log_trace, active, string);

	g_free(string);
}


