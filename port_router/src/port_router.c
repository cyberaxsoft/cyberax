/*
 ============================================================================
 Name        : port_router.c
 Author      : Konstantin Mazalov
 Version     :
 Copyright   : (C) Copyright 2019 All rights reserved
 Description : port router in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include <hidapi.h>

#include "logger.h"
#include "port_router.h"
#include "port_conf.h"
#include "port_func.h"
#include "port_router_data.h"

GMutex	   threads_mutex;

void lock_threads_mutex()
{
	g_mutex_lock(&threads_mutex);
}

void unlock_threads_mutex()
{
	g_mutex_unlock(&threads_mutex);
}

void wait_threads_mutex()
{
	lock_threads_mutex();
	unlock_threads_mutex();
}

static guint8 extract_escape(guint8* buffer, guint8* index)
{
	uint8_t symbol = buffer[*index];	(*index)++;

	if(symbol == IDC_ESCAPE_SYMBOL)
	{
		switch(buffer[*index])
		{
			case IDC_ESCAPED_START_MARKER:		symbol = IDC_START_MARKER;		break;
			case IDC_ESCAPED_END_MARKER:		symbol = IDC_END_MARKER;		break;
			case IDC_ESCAPED_ESCAPE:			symbol = IDC_ESCAPE_SYMBOL;		break;
		}
		(*index)++;
	}
	return symbol;
}

static guint8 check_frame(guint8* buffer, guint16 length)
{
	uint8_t escaped_pos = 1;
	uint8_t pos = 0;

	while( escaped_pos < (length - 1) )
	{
		buffer[pos++] = extract_escape( buffer, &escaped_pos );
	}

	uint8_t size = pos - 3;

	if(size != buffer[size])
	{
		return 0;
	}

	uint16_t crc16_value = CRC16_INIT;

	for(pos = 0; pos < size; pos++)
	{
		crc16_value = crc16_next(crc16_value, buffer[pos]);
	}

	uint16_t received_crc = buffer[size + 2];

	received_crc <<= 8;
	received_crc |= buffer[size + 1];

	if(crc16_value != received_crc)
	{
		return 0;
	}

	return size;
}

gboolean compare_sensor_settings(guint8 device_index, guint8* buffer)
{
	gboolean result = TRUE;

	guint8 pos = 0;

	if (buffer[pos++] != 0x01) result = FALSE;
	if (buffer[pos++] != 0x02) result = FALSE;
	if (buffer[pos++] != 0x00) result = FALSE;
	if (buffer[pos++] != 0x01) result = FALSE;
	if (buffer[pos++] != 0x01) result = FALSE;

	if (buffer[pos++] != 0xCD) result = FALSE;
	if (buffer[pos++] != 0xCC) result = FALSE;
	if (buffer[pos++] != 0xCC) result = FALSE;
	if (buffer[pos++] != 0x3D) result = FALSE;

	return result;
}


gboolean compare_port_settings(guint8 device_index, guint8 port_index, guint8* buffer)
{
	gboolean result = TRUE;

	guint8 pos = 0;

	if (port_index >= 0)
	{
		guint32 uart_baudrate = (buffer[pos + 3] << 24) | (buffer[pos + 2] << 16 ) | (buffer[pos + 1] << 8) | buffer[pos]; pos+=sizeof(guint32);
		if (uart_baudrate != safe_get_baudrate(device_index, port_index)) result = FALSE;

		guint8 uart_byte_size = buffer[pos++];
		if (uart_byte_size != safe_get_byte_size(device_index, port_index)) result = FALSE;

		guint8 uart_flags = buffer[pos++];
		if ((uart_flags & 0x03) != safe_get_parity(device_index, port_index)) result = FALSE;
		if (((uart_flags & 0x04) >> 2) !=  safe_get_duplex(device_index, port_index)) result = FALSE;
		if (((uart_flags & 0x08) >> 3) !=  (safe_get_stop_bits(device_index, port_index) - 1)) result = FALSE;
		if (((uart_flags & 0x10) >> 4) !=  safe_get_debug(device_index, port_index)) result = FALSE;
		if (((uart_flags & 0x20) >> 5) !=  safe_get_passtrough(device_index, port_index)) result = FALSE;
		if (((uart_flags & 0x40) >> 6) !=  safe_get_invert_rx(device_index, port_index)) result = FALSE;
		if (((uart_flags & 0x80) >> 7) !=  safe_get_invert_tx(device_index, port_index)) result = FALSE;

		guint8 uart_invert_tx = buffer[pos++];
		if (uart_invert_tx != safe_get_invert_tx(device_index, port_index)) result = FALSE;

		guint8 uart_invert_rx = buffer[pos++];
		if (uart_invert_rx != safe_get_invert_rx(device_index, port_index)) result = FALSE;

		guint8 uart_req_timeout = buffer[pos++];
		if (uart_req_timeout != safe_get_req_timeout(device_index, port_index)) result = FALSE;

		guint8 uart_application = buffer[pos++];
		if (uart_application != safe_get_application(device_index, port_index)) result = FALSE;

		guint8 uart_recv_timeout = buffer[pos++];
		if (uart_recv_timeout != safe_get_recv_timeout(device_index, port_index)) result = FALSE;
	}
	return result;
}

void interpret_idc_frame(guint8 device_index, gchar* serial_number, guint8* buffer, guint8 length, LogOptions log_options)
{
//	add_log(True, False, log_options.trace, log_options.system, "	Device %s buffer recv (length %d)", serial_number, length);
//
//	for (guint16 i = 0; i < length; i++)
//	{
//		add_log(False, False, log_options.trace, log_options.system, "%02X ", buffer[i]);
//	}
//	add_log(False, True, log_options.trace, log_options.system,  "");

	switch(buffer[IDC_INTERFACE_ID_OFFSET])
	{
		case PORT_TRANSFER_ID:
			{
				guint8 port = buffer[IDC_PORT_OFFSET];
				guint8 data_len  = buffer[IDC_DATA_LENGTH_OFFSET];

				add_log(True, False, log_options.trace, log_options.system,  "Device %s transfer port %d << ", serial_number, port);
				for (guint16 i = IDC_DATA_OFFSET; i < IDC_DATA_OFFSET + data_len; i++)
				{
					add_log(False, False, log_options.trace, log_options.system, "%02X ", buffer[i]);
				}
				add_log(False, True, log_options.trace, log_options.system,  "");

				if (check_device_port_is_mult(serial_number, port))
				{
					guint8 mult_device_count = safe_get_mult_device_count();

					if (mult_device_count > 0)
					{
						for (guint8 i = 0; i < mult_device_count; i++)
						{
							if (device_present_in_mult_device(i, serial_number, port))
							{
								MultDevice* mult_device = get_mult_device(i);

								g_mutex_lock(&mult_device->mutex);

								for (guint8 j = 0; j < MAX_CLIENT_COUNT; j++)
								{
									if (mult_device->client_socks[j] > cs_Free)
									{
										send(mult_device->client_socks[j], &buffer[IDC_DATA_OFFSET], data_len, 0);
									}
								}

								g_mutex_unlock(&mult_device->mutex);
							}
						}
					}
				}
				else
				{
					guint8 port_count = safe_get_port_count(device_index);

					if (port_count > 0)
					{
						for (guint8 i = 0; i < port_count; i++)
						{
							if (safe_get_port_num(device_index,i) == port && safe_get_interface_id(device_index,i) == PORT_TRANSFER_ID)
							{
								UsbDevicePort* port = get_port(device_index, i);

								g_mutex_lock(&port->mutex);

								for (guint8 j = 0; j < MAX_CLIENT_COUNT; j++)
								{
									if (port->client_socks[j] > cs_Free)
									{
										send(port->client_socks[j], &buffer[IDC_DATA_OFFSET], data_len, 0);
									}
								}

								g_mutex_unlock(&port->mutex);

								break;
							}
						}
					}
				}
			}
			break;

		case DM_ID:
			{
				if (buffer[IDC_COMMAND_OFFSET] == IDC_CMD_UART_CFG)
				{
					guint8 uart_num = buffer[IDC_COMMAND_OFFSET+1];
					gint8 port_index = safe_get_port_index_by_num(device_index, uart_num);

					if (safe_get_port_stage(device_index, port_index) == udps_WaitConfiguration)
					{
						safe_set_port_stage(device_index, port_index, udps_WaitReset);
					}
					else
					{
						if (port_index >= 0)
						{
							if (compare_port_settings(device_index, port_index, &buffer[IDC_COMMAND_OFFSET + 2]))
							{
								add_log(True, True, log_options.trace, log_options.system, "	Device %s port %d uart configuration compared", serial_number, uart_num);
								safe_set_port_stage(device_index, port_index, udps_Idle);
							}
							else
							{
								add_log(True, True, log_options.trace, log_options.system, "	Device %s port %d uart configuration not compared", serial_number, uart_num);
								safe_set_port_stage(device_index, port_index, udps_WaitConfiguration);
							}
						}
					}
					safe_set_usb_message_sended(device_index, FALSE);
				}
				else if (buffer[IDC_COMMAND_OFFSET] == IDC_CMD_RESET_REPLY)
				{
					safe_set_device_stage(device_index, uds_Reset);
				}
			}
			break;
		case CFG_ID:
			{
//				add_log(True, False, log_options.trace, log_options.system,  "Device %s sensor config << ", serial_number);
//
//				for (guint16 i = IDC_COMMAND_OFFSET; i < length; i++)
//				{
//					add_log(False, False, log_options.trace, log_options.system, "%02X ", buffer[i]);
//				}
//				add_log(False, True, log_options.trace, log_options.system,  "");

				guint8 port_num = buffer[IDC_COMMAND_OFFSET+1];
				gint8 port_index = safe_get_port_index_by_num(device_index, port_num);

				if (safe_get_port_stage(device_index, port_index) == udps_WaitConfiguration)
				{
					add_log(True, True, log_options.trace, log_options.system, "	Device %s  WAIT RESET", serial_number);
					safe_set_port_stage(device_index, port_index, udps_WaitReset);
				}
				else
				{
					if (port_index >= 0)
					{
						if (compare_sensor_settings(device_index, &buffer[IDC_COMMAND_OFFSET + 2]))
						{
							add_log(True, True, log_options.trace, log_options.system, "	Device %s sensor configuration compared", serial_number);
							safe_set_port_stage(device_index, port_index, udps_Idle);
						}
						else
						{
							add_log(True, True, log_options.trace, log_options.system, "	Device %s sensor configuration not compared", serial_number);
							safe_set_port_stage(device_index, port_index, udps_WaitConfiguration);
						}
					}
				}
				safe_set_usb_message_sended(device_index, FALSE);
			}
			break;

		case SENSORS_ID:
			{
				add_log(True, False, log_options.trace, log_options.system,  "Device %s sensors << ", serial_number);

				for (guint16 i = IDC_COMMAND_OFFSET; i < length; i++)
				{
					add_log(False, False, log_options.trace, log_options.system, "%02X ", buffer[i]);
				}
				add_log(False, True, log_options.trace, log_options.system,  "");

				guint8 port_count = safe_get_port_count(device_index);

				if (port_count > 0)
				{
					for (guint8 i = 0; i < port_count; i++)
					{
						if (safe_get_interface_id(device_index,i) == SENSORS_ID)
						{
							UsbDevicePort* port = get_port(device_index, i);

							g_mutex_lock(&port->mutex);

							for (guint8 j = 0; j < MAX_CLIENT_COUNT; j++)
							{
								if (port->client_socks[j] > -1)
								{
									guint8 tmp[SOCKET_BUFFER_SIZE] = {0x00};
									guint16 len = 0;

									tmp[len++] = 0x01;
									tmp[len++] = length - IDC_COMMAND_OFFSET;
									memcpy(&tmp[len], &buffer[IDC_COMMAND_OFFSET], length - IDC_COMMAND_OFFSET); len+=length - IDC_COMMAND_OFFSET;

									send(port->client_socks[j], tmp, len, 0);

									add_log(True, True, log_options.trace, log_options.system,  "sended client %d (%d bytes)", j,  len);

								}
							}

							g_mutex_unlock(&port->mutex);

							break;
						}
					}
				}
			}
			break;
	}
}

void destroy_device_sockets(guint8 device_index, LogOptions log_options)
{
	add_log(True, True, log_options.trace, log_options.system,  "Destroy device sockets");

	UsbDevice* device = get_device(device_index);

	for (guint8 i = 0; i < MAX_PORT_COUNT; i++)
	{
		UsbDevicePort* port = &device->ports[i];

		g_mutex_lock(&port->mutex);

		if (port->sock >= 0)
		{
			add_log(True, True, log_options.trace, log_options.system,  "Stoped port num %d", port->num);

			for (guint8 j = 0; j < MAX_CLIENT_COUNT; j++)
			{
				if (port->client_socks[j] > cs_Free)
				{
					shutdown(port->client_socks[j], 2);

					close(port->client_socks[j]);
					port->client_socks[j] = cs_Free;
				}
			}

			shutdown(port->sock, 2);

			close(port->sock);
			port->sock = -1;
		}
		g_mutex_unlock(&port->mutex);
	}
	add_log(True, True, log_options.trace, log_options.system,  "Device socket destroyed");
}

guint8 add_byte(guchar* buffer, guint8 byte, guint8 pos)
{
	guint8 result = 1;

	switch(byte)
	{
		case IDC_START_MARKER:
			buffer[pos] = IDC_ESCAPE_SYMBOL;
			buffer[pos+1] = IDC_ESCAPED_START_MARKER;
			result++;
			break;

		case IDC_END_MARKER:
			buffer[pos] = IDC_ESCAPE_SYMBOL;
			buffer[pos+1] = IDC_ESCAPED_END_MARKER;
			result++;
			break;

		case IDC_ESCAPE_SYMBOL:
			buffer[pos] = IDC_ESCAPE_SYMBOL;
			buffer[pos+1] = IDC_ESCAPED_ESCAPE;
			result++;
			break;

		default:
			buffer[pos] = byte;
			break;
	}
	return result;
}

void hid_send(guint8 device_index, gchar* serial_number, guint8* buffer, guint8 length, LogOptions log_options)
{
	g_mutex_lock(get_device_hid_mutex(device_index));

	guint res = hid_write(get_hid_device(device_index), buffer, length);

	if (res == -1)
	{
		add_log(True, True, log_options.trace, log_options.system,  "Device %s data send error", serial_number);

		add_log(True, False, log_options.trace, log_options.system,  "Device %s close HID...",serial_number);
		safe_close_hid(device_index);
		add_log(False, True, log_options.trace, log_options.system,  "Ok");
		destroy_device_sockets(device_index, log_options);
		safe_set_stoping(device_index,True);

	}

	g_mutex_unlock(get_device_hid_mutex(device_index));
}


void send_get_uart_conf_frame(guint8 idc_id, LogOptions log_options, guint8 device_index, gchar* serial_number, guint8 port_num)
{
	add_log(True, True, log_options.trace, log_options.system, "	Device %s get uart configuraion for port %d", serial_number, port_num);

	guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
	guint8 pos = 0;
	guint8 length = 0;
	guint16 crc = CRC16_INIT;

	buffer[pos++] = IDC_START_MARKER;
	pos+=add_byte(buffer, DM_ID, pos);  							crc = crc16_next(crc, DM_ID);      			length++;
	pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   	length++;
	pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);           	length++;

	pos+=add_byte(buffer, IDC_CMD_GET_UART_CFG, pos); 			   	crc = crc16_next(crc, IDC_CMD_GET_UART_CFG);length++;
	pos+=add_byte(buffer, port_num, pos); 			   				crc = crc16_next(crc, port_num);			length++;


	pos+=add_byte(buffer, length, pos);
	pos+=add_byte(buffer, crc & 0xFF, pos);
	pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

	buffer[pos++] = IDC_END_MARKER;

	hid_send(device_index, serial_number, buffer, IDC_FRAME_LENGTH, log_options);
}

void send_get_sensor_conf_frame(guint8 idc_id, LogOptions log_options, guint8 device_index, gchar* serial_number, guint8 port_num)
{
	add_log(True, True, log_options.trace, log_options.system, "	Device %s get sensor configuraion", serial_number);

	guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
	guint8 pos = 0;
	guint8 length = 0;
	guint16 crc = CRC16_INIT;

	buffer[pos++] = IDC_START_MARKER;
	pos+=add_byte(buffer, CFG_ID, pos);  							crc = crc16_next(crc, CFG_ID);      			length++;
	pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   		length++;
	pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);           		length++;

	pos+=add_byte(buffer, IDC_CMD_GET_SENSOR_CFG, pos); 			crc = crc16_next(crc, IDC_CMD_GET_SENSOR_CFG);	length++;
	pos+=add_byte(buffer, port_num, pos); 			   				crc = crc16_next(crc, port_num);				length++;


	pos+=add_byte(buffer, length, pos);
	pos+=add_byte(buffer, crc & 0xFF, pos);
	pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

	buffer[pos++] = IDC_END_MARKER;

	hid_send(device_index, serial_number, buffer, IDC_FRAME_LENGTH, log_options);
}

void send_set_sensor_conf_frame(guint8 idc_id, LogOptions log_options, guint8 device_index, gchar* serial_number, guint8 port_num, guint8 port_index)
{
	add_log(True, True, log_options.trace, log_options.system, "	Device %s set sensor configuraion", serial_number);

	guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
	guint8 pos = 0;
	guint8 length = 0;
	guint16 crc = CRC16_INIT;

	buffer[pos++] = IDC_START_MARKER;
	pos+=add_byte(buffer, CFG_ID, pos);  							crc = crc16_next(crc, CFG_ID);      		length++;
	pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   	length++;
	pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);           	length++;

	pos+=add_byte(buffer, IDC_CMD_SET_SENSOR_CFG, pos); 			crc = crc16_next(crc, IDC_CMD_SET_SENSOR_CFG);length++;
	pos+=add_byte(buffer, port_num, pos); 			   				crc = crc16_next(crc, port_num);			length++;  //index

	pos+=add_byte(buffer, 0x01, pos); 			   					crc = crc16_next(crc, 0x01);				length++;	//номер датчика
	pos+=add_byte(buffer, 0x02, pos); 			   					crc = crc16_next(crc, 0x02);				length++;	//тип
	pos+=add_byte(buffer, 0x00, pos); 			   					crc = crc16_next(crc, 0x00);				length++;	//порт
	pos+=add_byte(buffer, 0x01, pos); 			   					crc = crc16_next(crc, 0x01);				length++;
	pos+=add_byte(buffer, 0x01, pos); 			   					crc = crc16_next(crc, 0x01);				length++;
	pos+=add_byte(buffer, 0xCD, pos); 			   					crc = crc16_next(crc, 0xCD);				length++;
	pos+=add_byte(buffer, 0xCC, pos); 			   					crc = crc16_next(crc, 0xCC);				length++;
	pos+=add_byte(buffer, 0xCC, pos); 			   					crc = crc16_next(crc, 0xCC);				length++;
	pos+=add_byte(buffer, 0x3D, pos); 			   					crc = crc16_next(crc, 0x3D);				length++;

	pos+=add_byte(buffer, length, pos);
	pos+=add_byte(buffer, crc & 0xFF, pos);
	pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

	buffer[pos++] = IDC_END_MARKER;


	add_log(True, False, log_options.trace, log_options.system, "	Device %s buffer send (length %d)", serial_number, pos);

	for (guint16 i = 0; i < pos; i++)
	{
		add_log(False, False, log_options.trace, log_options.system, "%02X ", buffer[i]);
	}
	add_log(False, True, log_options.trace, log_options.system,  "");


	hid_send(device_index, serial_number, buffer, IDC_FRAME_LENGTH, log_options);


}

void send_set_uart_conf_frame(guint8 idc_id, LogOptions log_options, guint8 device_index, gchar* serial_number, guint8 port_num, guint8 port_index)
{
	add_log(True, True, log_options.trace, log_options.system, "	Device %s set uart configuraion for port %d", serial_number, port_num);

	guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
	guint8 pos = 0;
	guint8 length = 0;
	guint16 crc = CRC16_INIT;

	buffer[pos++] = IDC_START_MARKER;
	pos+=add_byte(buffer, DM_ID, pos);  							crc = crc16_next(crc, DM_ID);      			length++;
	pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   	length++;
	pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);           	length++;

	pos+=add_byte(buffer, IDC_CMD_SET_UART_CFG, pos); 			   	crc = crc16_next(crc, IDC_CMD_SET_UART_CFG);length++;
	pos+=add_byte(buffer, port_num, pos); 			   				crc = crc16_next(crc, port_num);			length++;

	guint32 uart_baudrate = safe_get_baudrate(device_index, port_index);

	pos+=add_byte(buffer, uart_baudrate & 0xFF, pos); 			   	crc = crc16_next(crc, uart_baudrate & 0xFF);length++;
	pos+=add_byte(buffer, (uart_baudrate >> 8) & 0xFF, pos); 		crc = crc16_next(crc, (uart_baudrate >> 8) & 0xFF);length++;
	pos+=add_byte(buffer, (uart_baudrate >> 16) & 0xFF, pos); 		crc = crc16_next(crc, (uart_baudrate >> 16) & 0xFF);length++;
	pos+=add_byte(buffer, (uart_baudrate >> 24) & 0xFF, pos); 		crc = crc16_next(crc, (uart_baudrate >> 24) & 0xFF);length++;

	guint8 uart_byte_size = safe_get_byte_size(device_index, port_index);
	pos+=add_byte(buffer, uart_byte_size, pos); 			   		crc = crc16_next(crc, uart_byte_size);length++;

	guint8 uart_flags = (safe_get_invert_tx(device_index, port_index) << 7) |
						(safe_get_invert_rx(device_index, port_index) << 6) |
						(safe_get_passtrough(device_index, port_index) << 5) |
						(safe_get_debug(device_index, port_index) << 4) |
						((safe_get_stop_bits(device_index, port_index) - 1) << 3) |
						(safe_get_duplex(device_index, port_index) << 2) |
						safe_get_parity(device_index, port_index);
	pos+=add_byte(buffer, uart_flags, pos); 			   			crc = crc16_next(crc, uart_flags);length++;

	guint8 uart_invert_tx = safe_get_invert_tx(device_index, port_index);
	pos+=add_byte(buffer, uart_invert_tx, pos); 			   		crc = crc16_next(crc, uart_invert_tx);length++;

	guint8 uart_invert_rx = safe_get_invert_rx(device_index, port_index);
	pos+=add_byte(buffer, uart_invert_rx, pos); 			   		crc = crc16_next(crc, uart_invert_rx);length++;

	guint8 uart_req_timeout = safe_get_req_timeout(device_index, port_index);
	pos+=add_byte(buffer, uart_req_timeout, pos); 			   		crc = crc16_next(crc, uart_req_timeout);length++;

	guint8 uart_application = safe_get_application(device_index, port_index);
	pos+=add_byte(buffer, uart_application, pos); 			   		crc = crc16_next(crc, uart_application);length++;

	guint8 uart_recv_timeout = safe_get_recv_timeout(device_index, port_index);
	pos+=add_byte(buffer, uart_recv_timeout, pos); 			   		crc = crc16_next(crc, uart_recv_timeout);length++;

	pos+=add_byte(buffer, length, pos);
	pos+=add_byte(buffer, crc & 0xFF, pos);
	pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

	buffer[pos++] = IDC_END_MARKER;

	hid_send(device_index, serial_number, buffer, IDC_FRAME_LENGTH, log_options);
}

void send_reset_frame(guint8 idc_id, LogOptions log_options, guint8 device_index, gchar* serial_number)
{
	add_log(True, True, log_options.trace, log_options.system, "	Device %s send reset", serial_number);

	guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
	guint8 pos = 0;
	guint8 length = 0;
	guint16 crc = CRC16_INIT;

	buffer[pos++] = IDC_START_MARKER;
	pos+=add_byte(buffer, DM_ID, pos);  							crc = crc16_next(crc, DM_ID);      			length++;
	pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   	length++;
	pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);           	length++;

	pos+=add_byte(buffer, IDC_CMD_RESET, pos); 			   			crc = crc16_next(crc, IDC_CMD_RESET);		length++;

	pos+=add_byte(buffer, length, pos);
	pos+=add_byte(buffer, crc & 0xFF, pos);
	pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

	buffer[pos++] = IDC_END_MARKER;

	hid_send(device_index, serial_number, buffer, IDC_FRAME_LENGTH, log_options);
}

gpointer device_conf_thread_func(gpointer data)
{
	UsbThreadParam param = *(UsbThreadParam*)data;

	unlock_threads_mutex();

	safe_set_device_stage(param.device_index, uds_Configuration);
	safe_set_usb_message_sended(param.device_index, FALSE);

	guint8 port_count = safe_get_port_count(param.device_index);

	if (port_count > 0)
	{
		for (guint8 i = 0; i < port_count; i++)
		{
			safe_set_port_stage(param.device_index, i, udps_Undefined);
		}

		while(safe_get_device_stage(param.device_index) == uds_Configuration)
		{
			if (!safe_get_usb_message_sended(param.device_index))
			{
				gboolean reset_ready = FALSE;
				gboolean idle_ready = TRUE;

				for (guint8 i = 0; i < port_count; i++)
				{
					switch (safe_get_port_stage(param.device_index, i))
					{
						case udps_Undefined:
							if ( safe_get_interface_id(param.device_index, i ) == PORT_TRANSFER_ID)
							{
								safe_set_usb_message_sended(param.device_index, TRUE);
								send_get_uart_conf_frame(safe_get_idc_id(param.device_index), param.log_options, param.device_index, param.serial_number, safe_get_port_num(param.device_index, i));
								idle_ready = FALSE;
							}
							else if ( safe_get_interface_id(param.device_index, i ) == SENSORS_ID)
							{
								safe_set_usb_message_sended(param.device_index, TRUE);
								send_get_sensor_conf_frame(safe_get_idc_id(param.device_index), param.log_options, param.device_index, param.serial_number, safe_get_port_num(param.device_index, i));
								idle_ready = FALSE;
							}
							break;

						case udps_WaitConfiguration:
							if ( safe_get_interface_id(param.device_index, i ) == PORT_TRANSFER_ID)
							{
								safe_set_usb_message_sended(param.device_index, TRUE);
								send_set_uart_conf_frame(safe_get_idc_id(param.device_index), param.log_options, param.device_index, param.serial_number, safe_get_port_num(param.device_index, i), i);
								idle_ready = FALSE;
							}
							else if ( safe_get_interface_id(param.device_index, i ) == SENSORS_ID)
							{
								safe_set_usb_message_sended(param.device_index, TRUE);
								send_set_sensor_conf_frame(safe_get_idc_id(param.device_index), param.log_options, param.device_index, param.serial_number, safe_get_port_num(param.device_index, i), i);
								idle_ready = FALSE;
							}
							break;

						case udps_WaitReset:
							reset_ready = TRUE;
							break;

						case udps_Idle:
							break;

					}
					if (safe_get_usb_message_sended(param.device_index)) break;
				}
				if (idle_ready)
				{
					if (reset_ready)
					{
						add_log(True, True, param.log_options.trace, param.log_options.system, "	Device %s RESET READY", param.serial_number);
						send_reset_frame(safe_get_idc_id(param.device_index), param.log_options, param.device_index, param.serial_number);
						safe_set_device_stage(param.device_index, uds_Reset);
					}
					else
					{
						add_log(True, True, param.log_options.trace, param.log_options.system, "	Device %s IDLE READY", param.serial_number);
						safe_set_device_stage(param.device_index, uds_Idle);
					}
					break;
				}
			}
		}
	}

	return NULL;
}


gpointer device_read_thread_func(gpointer data)
{
	UsbThreadParam param = *(UsbThreadParam*)data;

	unlock_threads_mutex();

	guint8 buff[SOCKET_BUFFER_SIZE] = {0x00};

	guint8 buffer[SOCKET_BUFFER_SIZE] = {0x00};
	guint16 pos = 0;

	IdcExchangeState ies = ies_WaitStart;

	UsbDevice* device = get_device(param.device_index);


	while(!safe_get_stoping(param.device_index))
	{
		guint res = hid_read(device->handle, buff, SOCKET_BUFFER_SIZE);

		if (res == -1)
		{

			add_log(True, True, param.log_options.trace, param.log_options.system,  "Error HID read");
			add_log(True, False, param.log_options.trace, param.log_options.system,  "Device %s close HID...",param.serial_number);
			safe_close_hid(param.device_index);
			add_log(False, True, param.log_options.trace, param.log_options.system,  "Ok");
			destroy_device_sockets(param.device_index, param.log_options);
			safe_set_stoping(param.device_index,True);
			break;
		}
		else
		{
			if (res > 0)
			{
				for (guint8 i = 0; i < res; i++)
				{
				//	add_log(True, True, param.log_options.trace, param.log_options.system,  " << %02X", buff[i]);

					switch(ies)
					{
						case ies_WaitStart:
							if (buff[i] == IDC_START_MARKER)
							{
								pos = 0;
								buffer[pos++] = buff[i];
								ies = ies_WaitEnd;
							}
							break;

						case ies_WaitEnd:
							buffer[pos++] = buff[i];

							if (buff[i] == IDC_END_MARKER)
							{
								guint8 size = check_frame(buffer, pos);

								if(size)
								{
									interpret_idc_frame(param.device_index, param.serial_number, buffer, size, param.log_options);
									ies = ies_WaitStart;
								}
								else
								{
									ies = ies_WaitStart;
								}
							}
							break;
					}
				}
			}
		}
	}

	safe_set_is_connected(param.device_index, False);

	safe_set_stoping(param.device_index, False);

	add_log(True, True, param.log_options.trace, param.log_options.system,  "Device index %d connected false", param.device_index);

	return NULL;
}

gpointer mult_client_thread_func(gpointer data)
{
	MultClientThreadParam param = *(MultClientThreadParam*)data;

	unlock_threads_mutex();

	guchar bufr[SOCKET_BUFFER_SIZE];

	while(True)
	{
		guint8 length = 0;

		guint16 bytes_read = recv(param.sock, bufr, 1024, 0);

		if(bytes_read <= 0)
		{
			break;
		}
		else
		{
			if (param.units.unit_count > 0)
			{
				for (guint8 i = 0; i < param.units.unit_count; i++)
				{
					guint16 total_pos = 0;

					while (total_pos < bytes_read)
					{
						guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
						guint8 pos = 0;
						guint16 crc = CRC16_INIT;

						buffer[pos++] = IDC_START_MARKER;
						pos+=add_byte(buffer, PORT_TRANSFER_ID, pos);  					crc = crc16_next(crc, PORT_TRANSFER_ID); 			length++;
						pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);  			length++;
						pos+=add_byte(buffer, param.units.units[i].idc_id, pos); 		crc = crc16_next(crc, param.units.units[i].idc_id); length++;

						pos+=add_byte(buffer, IDC_CMD_SEND, pos);						crc = crc16_next(crc, IDC_CMD_SEND);      			length++;
						pos+=add_byte(buffer, param.units.units[i].num, pos); 			crc = crc16_next(crc, param.units.units[i].num);	length++;
						pos+=add_byte(buffer, bytes_read, pos);							crc = crc16_next(crc, bytes_read);        			length++;

						add_log(True, False, param.log_options.trace, param.log_options.system, "Socket %d device %s port %d >> ", param.sock, param.units.units[i].serial_number, param.units.units[i].num);

						while (pos <  DATA_MAX_FRAME_LENGTH && total_pos < bytes_read)
						{
							add_log(False, False, param.log_options.trace, param.log_options.system, "%02X ", bufr[total_pos]);
							pos+=add_byte(buffer, bufr[total_pos], pos);   				crc = crc16_next(crc, bufr[total_pos++]);   		length++;
						}

						add_log(False, True, param.log_options.trace, param.log_options.system, "");

						pos+=add_byte(buffer, length, pos);
						pos+=add_byte(buffer, crc & 0xFF, pos);
						pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

						buffer[pos++] = IDC_END_MARKER;

						hid_send(param.units.units[i].device_index, param.units.units[i].serial_number, buffer, IDC_FRAME_LENGTH, param.log_options);
					}
				}
			}
		}
	}

	close(param.sock);

	safe_set_mult_client_sock(param.device_index, param.client_index, cs_Free);

	return NULL;
}


gpointer socket_client_thread_func(gpointer data)
{
	ClientThreadParam param = *(ClientThreadParam*)data;

	unlock_threads_mutex();

	guchar bufr[SOCKET_BUFFER_SIZE];

	guint8 idc_id = safe_get_idc_id(param.device_index);

	gint32 sock = safe_get_client_sock(param.device_index, param.port_index, param.client_index);

	gint8 port_num = safe_get_port_num(param.device_index, param.port_index);
	gint8 interface_id = safe_get_interface_id(param.device_index, param.port_index);

	while(!safe_get_stoping(param.device_index))
	{
		guint8 length = 0;

		guint16 bytes_read = recv(sock, bufr, 1024, 0);
		if(bytes_read <= 0)
		{
			break;
		}
		else
		{
			guint16 total_pos = 0;

			while (total_pos < bytes_read)
			{
				guint8 buffer[IDC_FRAME_LENGTH] = {0x00};
				guint8 pos = 0;
				guint16 crc = CRC16_INIT;

				buffer[pos++] = IDC_START_MARKER;
				pos+=add_byte(buffer, interface_id, pos);  						crc = crc16_next(crc, interface_id);      length++;
				pos+=add_byte(buffer, IDC_USB_ADDRESS, pos);					crc = crc16_next(crc, IDC_USB_ADDRESS);   length++;
				pos+=add_byte(buffer, idc_id, pos); 				   			crc = crc16_next(crc, idc_id);            length++;

				if (interface_id == PORT_TRANSFER_ID)
				{
					pos+=add_byte(buffer, IDC_CMD_SEND, pos);					crc = crc16_next(crc, IDC_CMD_SEND);      length++;
					pos+=add_byte(buffer, port_num, pos); 						crc = crc16_next(crc, port_num);          length++;
					pos+=add_byte(buffer, bytes_read, pos);						crc = crc16_next(crc, bytes_read);        length++;
				}

				add_log(True, False, param.log_options.trace, param.log_options.system, "Device %s port %d >> ", param.serial_number, port_num);

				while (pos <  DATA_MAX_FRAME_LENGTH && total_pos < bytes_read)
				{
					add_log(False, False, param.log_options.trace, param.log_options.system, "%02X ", bufr[total_pos]);
					pos+=add_byte(buffer, bufr[total_pos], pos);   													crc = crc16_next(crc, bufr[total_pos++]);   length++;
				}

				add_log(False, True, param.log_options.trace, param.log_options.system, "");

				pos+=add_byte(buffer, length, pos);
				pos+=add_byte(buffer, crc & 0xFF, pos);
				pos+=add_byte(buffer, (crc >> 8) & 0xFF, pos);

				buffer[pos++] = IDC_END_MARKER;

				hid_send(param.device_index, param.serial_number, buffer, IDC_FRAME_LENGTH, param.log_options);
			}
		}
	}

	close(sock);

	safe_set_client_sock(param.device_index, param.port_index, param.client_index, cs_Free);

	return NULL;
}

gpointer mult_port_serv_thread_func(gpointer data)
{

	MultServThreadParam param = *(MultServThreadParam*)data;

	unlock_threads_mutex();

	add_log(True, True, param.log_options.trace, param.log_options.system, "Listen socket %d",param.sock);
	listen(param.sock, 1);
	add_log(True, True, param.log_options.trace, param.log_options.system, "Listen socket %d complete",param.sock);

	while(True)
	{
		struct sockaddr client_addr;
		socklen_t client_addr_len = 0;

		gint8 client_index = find_next_mult_client_index(param.device_index, param.max_client_count);

		if (client_index >= 0)
		{
			add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d wait client index %d", param.sock, client_index);

			gint32 client_sock = accept(param.sock, &client_addr, &client_addr_len);

			if(client_sock < 0)
			{
				add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d client index %d accepting error", param.sock, client_index);
				break;
			}
			else
			{
				add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d client index %d accepted socket %d",  param.sock, client_index, client_sock);

				lock_threads_mutex();

				safe_set_mult_client_sock(param.device_index, client_index, client_sock);

				MultClientThreadParam client_param =  {.device_index = param.device_index,
													.client_index = client_index,
													.sock = client_sock,
													.units = param.units,
													.log_options = param.log_options};
				GThread* client_thread = g_thread_new("mult_client_thread", mult_client_thread_func, &client_param);

				if (client_thread == NULL)
				{
					add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d client index %d error starting client thread", param.sock, client_index);
					unlock_threads_mutex();

				}
				else
				{
					wait_threads_mutex();

					add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d client index %d client thread started", param.sock, client_index);
					client_thread->priority = G_THREAD_PRIORITY_LOW;
					add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d client index %d set client thread priority", param.sock, client_index);

					client_index = find_next_mult_client_index(param.device_index, param.max_client_count);
					if (client_index >= 0)
						add_log(True, True, param.log_options.trace, param.log_options.system, "Socket %d wait client index %d", param.sock, client_index);
				}
			}
		}
	}

	return NULL;
}

gpointer port_serv_thread_func(gpointer data)
{
	ServThreadParam param = *(ServThreadParam*)data;

	unlock_threads_mutex();

	gint32 port_sock = safe_get_port_sock(param.device_index, param.port_index);

	add_log(True, True, param.log_options.trace, param.log_options.system, "Listen device %s (device_index = %d)", param.serial_number, param.device_index);
	listen(port_sock, 1);
	add_log(True, True, param.log_options.trace, param.log_options.system, "Listen device %s complete", param.serial_number);

	while(!safe_get_stoping(param.device_index))
	{
		struct sockaddr client_addr;
		socklen_t client_addr_len = 0;

		gint8 client_index = find_next_client_index(param.device_index, param.port_index, param.max_client_count);

		if (client_index >= 0)
		{
			add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s wait client index %d", param.serial_number, client_index);

			gint32 client_sock = accept(port_sock, &client_addr, &client_addr_len);

			if(client_sock < 0)
			{
				add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s client index %d accepting error", param.serial_number, client_index);
				break;
			}
			else
			{
				if (!safe_get_stoping(param.device_index))
				{
					add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s client index %d accepted socket %d", param.serial_number, client_index, client_sock);

					lock_threads_mutex();

					safe_set_client_sock(param.device_index, param.port_index, client_index, client_sock);

					ClientThreadParam client_param =  {.device_index = param.device_index,
													.port_index = param.port_index,
													.client_index = client_index,
													.serial_number = g_strdup(param.serial_number),
													.log_options = param.log_options};
					GThread* client_thread = g_thread_new("dc_client_thread", socket_client_thread_func, &client_param);

					if (client_thread == NULL)
					{
						add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s client index %d error starting client thread", param.serial_number, client_index);
						unlock_threads_mutex();

					}
					else
					{
						wait_threads_mutex();

						add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s client index %d client thread started", param.serial_number, client_index);
						client_thread->priority = G_THREAD_PRIORITY_LOW;
						add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s client index %d set client thread priority", param.serial_number, client_index);

						client_index = find_next_client_index(param.device_index, param.port_index, param.max_client_count);
						if (client_index >= 0)
							add_log(True, True, param.log_options.trace, param.log_options.system, "Device %s wait client index %d", param.serial_number, client_index);
					}
				}
				else
				{
					close(client_sock);
					safe_set_client_sock(param.device_index, param.port_index, client_index, -1);
				}
			}
		}
	}

	return NULL;
}

gboolean start_ports_sockets(guint8 device_index, gchar* serial_number, LogOptions log_options)
{
	gboolean result = True;

	guint8 port_count = safe_get_port_count(device_index);

	UsbDevice* device = get_device(device_index);

	if (port_count > 0)
	{
		for (guint8 i = 0; i < port_count; i++)
		{
			UsbDevicePort* port = &device->ports[i];

			add_log(True, False, log_options.trace, log_options.system, "		Create mutex for port %d...", port->num);

			if (port->mutex.p == NULL)
			{
				g_mutex_init(&port->mutex);
				add_log(False, True, log_options.trace, log_options.system, "Ok");
			}
			else
			{
				add_log(False, True, log_options.trace, log_options.system, "skipped");
			}

			if (!check_device_port_is_mult(serial_number, port->num))
			{

				add_log(True, False, log_options.trace, log_options.system, "		Prepare socket array for port %d...",port->num);

				g_mutex_lock(&port->mutex);

				for (guint8 j = 0; j < port->max_client_count; j++)
				{
					if (port->client_socks[j] !=-1)
					{
						close(port->client_socks[j]);
					}

					port->client_socks[j] = -1;
				}

				UsbDevicePort tmp_port = *port;

				g_mutex_unlock(&port->mutex);

				add_log(False, True, log_options.trace, log_options.system, "Ok");

				struct sockaddr_in addr;

				add_log(True, False, log_options.trace, log_options.system, "		Create server socket for port %d (ip_port %d)...", tmp_port.num, tmp_port.ip_port);

				tmp_port.sock = socket(AF_INET, SOCK_STREAM, 0);

				if(tmp_port.sock < 0)
				{
					add_log(False, True, log_options.trace, log_options.system, "Socket create error");
					safe_set_stoping(i, True);
					safe_close_hid(i);
					g_thread_join(device->usb_device_thread);

					destroy_device_sockets(device_index, log_options);

					result = False;

					break;
				}
				else
				{
					addr.sin_family = AF_INET;
					addr.sin_port = htons(tmp_port.ip_port);
					addr.sin_addr.s_addr = htonl(INADDR_ANY);
					if(bind(tmp_port.sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
					{
						add_log(False, True, log_options.trace, log_options.system, "Socket bind error");
						safe_set_stoping(device_index, True);
						safe_close_hid(device_index);
						g_thread_join(device->usb_device_thread);

						destroy_device_sockets(device_index, log_options);

						result = False;

						break;
					}
					else
					{
						add_log(False, True, log_options.trace, log_options.system, "Ok");

						safe_set_port_sock(device_index, i, tmp_port.sock);

						lock_threads_mutex();

						ServThreadParam param = {.device_index = device_index, .port_index = i, .serial_number = NULL, .max_client_count = tmp_port.max_client_count, .log_options = log_options};

						safe_get_serial_number(device_index, &param.serial_number);

						add_log(True, False, log_options.trace, log_options.system, "		Start server socket thread for port %d...", tmp_port.num);

						g_mutex_lock(&port->mutex);

						port->serv_thread = g_thread_new("port_serv_thread", port_serv_thread_func, &param);

						if (port->serv_thread == NULL)
						{
							add_log(False, True, log_options.trace, log_options.system, "Error");
							unlock_threads_mutex();
						}
						else
						{
							add_log(False, True, log_options.trace, log_options.system, "Ok");

							wait_threads_mutex();

							add_log(True, False, log_options.trace, log_options.system, "		Set server socket thread priority for port %d...", tmp_port.num);
							port->serv_thread->priority = G_THREAD_PRIORITY_LOW;
							add_log(False, True, log_options.trace, log_options.system, "Ok");
						}
						g_mutex_unlock(&port->mutex);
					}
				}
			}
		}
	}
	return result;
}

void start_mult_device_serv_thread(MultDevice* device, guint8 device_index, LogOptions log_options)
{

	if (device->sock != -1)
	{
		close(device->sock);

	}

	device->sock = -1;

	for (guint8 i = 0; i < device->max_client_count; i++)
	{
		if (device->client_socks[i] !=-1)
		{
			close(device->client_socks[i]);
		}

		device->client_socks[i] = -1;
	}

	add_log(True, True, log_options.trace, log_options.system, "Create server socket for mult ip_port %d", device->ip_port);

	struct sockaddr_in addr;

	device->sock = socket(AF_INET, SOCK_STREAM, 0);

	if(device->sock < 0)
	{
		add_log(True, True, log_options.trace, log_options.system, "Socket port %d create error", device->ip_port);
	}
	else
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(device->ip_port);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if(bind(device->sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			add_log(True, True, log_options.trace, log_options.system, "Socket port %d bind error", device->ip_port);
		}
		else
		{
			lock_threads_mutex();

			MultServThreadParam param = {.device_index = device_index,.sock =device->sock, .units = device->units,.max_client_count = device->max_client_count , .log_options = log_options};

			add_log(True, False, log_options.trace, log_options.system, "Start server socket thread for port %d...", device->ip_port);

			device->serv_thread = g_thread_new("mult_port_serv_thread", mult_port_serv_thread_func, &param);

			if (device->serv_thread == NULL)
			{
				add_log(False, True, log_options.trace, log_options.system, "Error");
				unlock_threads_mutex();
			}
			else
			{
				add_log(False, True, log_options.trace, log_options.system, "Ok");

				wait_threads_mutex();

				add_log(True, False, log_options.trace, log_options.system, "Set server socket thread priority for port %d...", device->ip_port);
				device->serv_thread->priority = G_THREAD_PRIORITY_LOW;
				add_log(False, True, log_options.trace, log_options.system, "Ok");
			}


		}


	}

}

gboolean find_and_start_device(gchar* serial_number, guint8 dev_index, LogOptions log_options)
{
	gboolean result = False;

	struct hid_device_info *devs, *cur_dev;

	devs = hid_enumerate(0x00, 0x00);

	cur_dev = devs;

	while (cur_dev)
	{
		gchar* ser_num = g_strdup_printf("%ls", cur_dev->serial_number);

		if (cur_dev->vendor_id == safe_get_vendor_id(dev_index) &&
				cur_dev->product_id == safe_get_product_id(dev_index) &&
				strcmp(serial_number, ser_num) == 0)
		{
			add_log(True, True, log_options.trace, log_options.system, "Device %s found. Initialization....", ser_num);
			add_log(True, False, log_options.trace, log_options.system, "	Create HID mutex for %s...", ser_num);

			UsbDevice* device = get_device(dev_index);

			if (device->hid_mutex.p == NULL)
			{
				g_mutex_init(&device->hid_mutex);
			}

			add_log(False, True, log_options.trace, log_options.system, "Ok");

			add_log(True, False, log_options.trace, log_options.system, "	Device %ls hid opening (vendor_id = 0x%04X product_id = 0x%04X)...", cur_dev->serial_number, cur_dev->vendor_id, cur_dev->product_id);

			device->handle = hid_open(cur_dev->vendor_id, cur_dev->product_id, cur_dev->serial_number);

			safe_set_device_stage(dev_index, uds_Undefined);

			if (device->handle == NULL)
			{
				add_log(False, True, log_options.trace, log_options.system, "Error");
			}
			else
			{
				add_log(False, True, log_options.trace, log_options.system, "Ok");

				add_log(True, False, log_options.trace, log_options.system, "	Device %s set hid unblocking mode...", ser_num);
				hid_set_nonblocking(device->handle, 0);
				add_log(False, True, log_options.trace, log_options.system, "Ok");

				add_log(True, False, log_options.trace, log_options.system, "	Device %s start usb thread...", ser_num);

				lock_threads_mutex();

				UsbThreadParam r_param = {.device_index = dev_index, .serial_number = g_strdup(ser_num), .log_options = log_options};

				device->usb_device_thread = g_thread_new("usb_device_thread", device_read_thread_func, &r_param);

				if (device->usb_device_thread == NULL)
				{
					add_log(False, True, log_options.trace, log_options.system, "Error");
					unlock_threads_mutex();
				}
				else
				{
					add_log(False, True, log_options.trace, log_options.system, "Ok");

					wait_threads_mutex();

					add_log(True, False, log_options.trace, log_options.system, "	Device %s set usb thread priority...",ser_num);
					device->usb_device_thread->priority = G_THREAD_PRIORITY_LOW;
					add_log(False, True, log_options.trace, log_options.system, "Ok");
				}

				add_log(True, False, log_options.trace, log_options.system, "	Device %s start configuration thread...", ser_num);

				lock_threads_mutex();

				UsbThreadParam c_param = {.device_index = dev_index, .serial_number = g_strdup(ser_num), .log_options = log_options};

				device->configuration_thread = g_thread_new("usb_device_conf_thread", device_conf_thread_func, &c_param);

				if (device->configuration_thread == NULL)
				{
					add_log(False, True, log_options.trace, log_options.system, "Error");
					unlock_threads_mutex();
				}
				else
				{
					add_log(False, True, log_options.trace, log_options.system, "Ok");

					wait_threads_mutex();

					device->configuration_thread->priority = G_THREAD_PRIORITY_LOW;
//					add_log(True, True, log_options.trace, log_options.system, "	Device %s set configuration thread priority...Ok",ser_num);
				}

				while (safe_get_device_stage(dev_index) < uds_Reset);

				add_log(True, True, log_options.trace, log_options.system,  "	Device %s device stage %d", ser_num, safe_get_device_stage(dev_index));


				if (safe_get_device_stage(dev_index) == uds_Reset)
				{
					safe_set_stoping(dev_index, True);

					g_thread_join(device->usb_device_thread);

					add_log(True, True, log_options.trace, log_options.system, "	Device %s Usb device thread is joined", ser_num);
				}
				else
				{
					result = start_ports_sockets(dev_index, ser_num, log_options);

					if (result == False)
					{
						safe_set_stoping(dev_index, True);

						g_thread_join(device->usb_device_thread);

						add_log(True, True, log_options.trace, log_options.system,  "	Device %s Usb device thread is joined", ser_num);

					}
				}
			}
		}
		g_free(ser_num);
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

	return result;
}

int main(int argc, char *argv[])
{
	guint res = hid_init();

	g_mutex_init(&threads_mutex);

	if (res == -1)
	{
		g_printf("HID init error\n");
		return EXIT_FAILURE;
	}

	if (!g_thread_supported())
	{
		g_printf("GThread not supported\n");
		return EXIT_FAILURE;
	}

	init_conf_mutex();

	PortRouterConfig* configuration =  get_configuration();

	if (!read_settings(argv[1], configuration))
	{
		g_printf("Settings error\n");
		return EXIT_FAILURE;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	log_options.frames = False;
	log_options.parsing = False;
	log_options.requests = False;
	log_options.system = True;


	if (configuration->log_enable)
	{
		if (!init_log_dir(configuration->log_dir, LOG_PREFIX))
		{
			g_printf("%s : log init error\n", LOG_PREFIX);
			return EXIT_FAILURE;
		}

		if (!init_log(configuration->log_dir, log_options.file_size, log_options.save_days))
		{
			g_printf("%s : log create error\n", LOG_PREFIX);
			return EXIT_FAILURE;
		}
	}

	delete_old_logs(configuration->log_dir);


	guint8 device_count = configuration->device_count;

	if ( device_count == 0 )
	{
		add_log(True, True, log_options.trace, log_options.system, "Devices not found");
		close_log();
		return EXIT_FAILURE;
	}

	guint8 mult_device_count = configuration->mult_device_count;

	if (mult_device_count > 0)
	{
		set_device_indexes();

		for (guint8 i = 0; i < mult_device_count; i++)
		{
			start_mult_device_serv_thread(&configuration->mult_devices[i], i, log_options);
		}
	}

	add_log(True, True, log_options.trace, log_options.system, "Starting router...");

	guint64 last_search = get_date_time();

	while(True)
	{
		if (get_date_time() > last_search + SEARCH_TIMEOUT)
		{
			last_search = get_date_time();

			for (guint8 i = 0; i < device_count; i++)
			{
				gchar* serial_number = NULL;
				safe_get_serial_number(i, &serial_number);

				gboolean is_connected = safe_get_is_connected(i);

				if (serial_number != NULL && !is_connected)
				{
					safe_set_stoping(i, False);

					if (find_and_start_device(serial_number, i, log_options))
					{
						safe_set_is_connected(i, True);
						add_log(True, True, log_options.trace, log_options.system, "Device %s initialization complete", serial_number);
					}
				}
				if (serial_number!=NULL)
				{
					g_free(serial_number);
				}
			}
		}
	}

	close_log();

	return EXIT_SUCCESS;
}
