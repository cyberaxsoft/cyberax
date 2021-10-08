#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tlv.h"
#include "socket_func.h"
#include "tgs_device.h"
#include "tgs_device_data.h"
#include "tgs_func.h"

void send_tgs_device_status_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%d Send TGS device status message", client_info->socket);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d TGS Device status message:", client_info->socket);

	TgsDeviceStatus  status = get_tgs_device_status(tgs_index);

	guint32 status_description_length = 0;
	gchar* status_desc = tgs_status_description(status, &status_description_length);

	TgsDeviceError  device_error = get_tgs_device_last_erorr(tgs_index);

	guint32 error_description_length = 0;
	gchar* error_desc = tgs_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetDeviceStatus, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetDeviceStatus, server_command_to_str(hsc_GetDeviceStatus));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, tgs_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, tgs_error_to_str(device_error));
		g_free(error_desc);
	}

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}
	tlv_delete_units(&units);

}

void send_tgs_device_configuration_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send TGS device configurtion message:", client_info->socket);

	TgsDeviceStatus  status = get_tgs_device_status(tgs_index);

	guint32 status_description_length = 0;
	gchar* status_desc = tgs_status_description(status, &status_description_length);

	TgsDeviceError  device_error = get_tgs_device_last_erorr(tgs_index);

	guint32 error_description_length = 0;
	gchar* error_desc = tgs_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetConfiguration, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetConfiguration, server_command_to_str(hsc_GetConfiguration));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, tgs_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, tgs_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* conf_units = NULL;

	guint8 tank_count = get_tgs_tank_count(tgs_index);

	add_tlv_unit_8( &conf_units, ttgsct_TankCount,tank_count,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Tank count: %d", client_info->socket, tank_count);

	if (tank_count  > 0)
	{
		for (guint8 i = 0; i < tank_count; i++)
		{
			guint32 tank_num = 0;
			guint8  tank_channel = 0;

			get_tgs_tank_info(tgs_index, i, &tank_num, &tank_channel);

			TlvUnit* tank_conf_units = NULL;

			add_tlv_unit_32(&tank_conf_units, ttgstct_Number, 	tank_num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Tank num: %d", client_info->socket, tank_num);
			add_tlv_unit_8(&tank_conf_units, ttgstct_Channel, 	tank_channel, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Tank channel: %d", client_info->socket, tank_channel);

			guint32 tank_conf_size = 0;
			guchar* tank_conf_frame = tlv_serialize_units(tank_conf_units, &tank_conf_size);

			if (tank_conf_frame !=NULL && tank_conf_size > 0)
			{
				tlv_add_unit(&conf_units,tlv_create_unit(ttgsct_TankConfiguration , tank_conf_frame, 0, tank_conf_size));
				g_free(tank_conf_frame);
			}

			tlv_delete_units(&tank_conf_units);

		}
	}

	guint32 configuration_size = 0;
	guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

	if (configuration_frame !=NULL && configuration_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_TGSDeviceConfiguration , configuration_frame, 0, configuration_size));
		g_free(configuration_frame);
	}

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}

	tlv_delete_units(&conf_units);
	tlv_delete_units(&units);
}

void send_tgs_device_data_message(guint8 client_index, guint8 tgs_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 tank_num, guint8 tank_index, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send TGS device data message:", client_info->socket);

	TgsDeviceStatus  status = get_tgs_device_status(tgs_index);

	guint32 status_description_length = 0;
	gchar* status_desc = tgs_status_description(status, &status_description_length);

	TgsDeviceError  device_error = get_tgs_device_last_erorr(tgs_index);

	guint32 error_description_length = 0;
	gchar* error_desc = tgs_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_TGSGetTankData, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_TGSGetTankData, server_command_to_str(hsc_TGSGetTankData));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, tgs_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, tgs_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* data_units = NULL;

	guint8 tank_count = get_tgs_tank_count(tgs_index);

	if (tank_count > 0)
	{
		if (tank_num == 0)
		{
			//all disp data
			add_tlv_unit_8( &data_units, ttgsdt_Count,tank_count,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Tank count: %d", client_info->socket, tank_count);

			for (guint8 i = 0; i < tank_count; i++)
			{
				guint32 num = 0;

				gfloat	height = 0;
				gfloat	volume = 0;
				gfloat	weight = 0;
				gfloat	density = 0;
				gfloat	temperature = 0;
				gfloat	water_level = 0;
				gboolean online = FALSE;

				if (get_tgs_tank_data(tgs_index, i, &num, &height, &volume, &weight, &density, &temperature, &water_level, &online))
				{
					TlvUnit* tank_data_units = NULL;

					add_tlv_unit_32(&tank_data_units, 	ttgstdt_Number, 		num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Tank num: %d", client_info->socket, num);

					add_tlv_unit_float(&tank_data_units, ttgstdt_Height, 		height, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Height: %f", client_info->socket, height);
					add_tlv_unit_float(&tank_data_units, ttgstdt_Volume, 		volume, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Volume: %f", client_info->socket, volume);
					add_tlv_unit_float(&tank_data_units, ttgstdt_Weight, 		weight, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Weight: %f", client_info->socket, weight);
					add_tlv_unit_float(&tank_data_units, ttgstdt_Density, 		density, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Density: %f", client_info->socket, density);
					add_tlv_unit_float(&tank_data_units, ttgstdt_Temperature,	temperature,	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Temperature: %f", client_info->socket, temperature);
					add_tlv_unit_float(&tank_data_units, ttgstdt_WaterLevel,		water_level,	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Water level: %f", client_info->socket, water_level);

					add_tlv_unit_8(&tank_data_units, 	ttgstdt_Online, 		tlv_bool_to_byte(online), 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Online: %d", client_info->socket, online);

					guint32 tank_data_size = 0;
					guchar* tank_data_frame = tlv_serialize_units(tank_data_units, &tank_data_size);

					if (tank_data_frame !=NULL && tank_data_size > 0)
					{
						tlv_add_unit(&data_units,tlv_create_unit(ttgsdt_TankData , tank_data_frame, 0, tank_data_size));
						g_free(tank_data_frame);
					}

					tlv_delete_units(&tank_data_units);

				}
			}
		}
		else
		{
			add_tlv_unit_8( &data_units, ttgsdt_Count,1,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Tank count: %d", client_info->socket, 1);

			guint32 num = 0;
			gfloat	height = 0;
			gfloat	volume = 0;
			gfloat	weight = 0;
			gfloat	density = 0;
			gfloat	temperature = 0;
			gfloat	water_level = 0;
			gboolean online = FALSE;

			if (get_tgs_tank_data(tgs_index, tank_index, &num, &height, &volume, &weight, &density, &temperature, &water_level, &online))
			{

				TlvUnit* tank_data_units = NULL;

				add_tlv_unit_32(&tank_data_units, 	ttgstdt_Number, 		num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Tank num: %d", client_info->socket, num);

				add_tlv_unit_float(&tank_data_units, ttgstdt_Height, 		height, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Height: %f", client_info->socket, height);
				add_tlv_unit_float(&tank_data_units, ttgstdt_Volume, 		volume, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Volume: %f", client_info->socket, volume);
				add_tlv_unit_float(&tank_data_units, ttgstdt_Weight, 		weight, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Weight: %f", client_info->socket, weight);
				add_tlv_unit_float(&tank_data_units, ttgstdt_Density, 		density, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Density: %f", client_info->socket, density);
				add_tlv_unit_float(&tank_data_units, ttgstdt_Temperature,	temperature,	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Temperature: %f", client_info->socket, temperature);
				add_tlv_unit_float(&tank_data_units, ttgstdt_WaterLevel,		water_level,	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Water level: %f", client_info->socket, water_level);

				add_tlv_unit_8(&tank_data_units, 	ttgstdt_Online, 		tlv_bool_to_byte(online), 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Online: %d", client_info->socket, online);

				guint32 tank_data_size = 0;
				guchar* tank_data_frame = tlv_serialize_units(tank_data_units, &tank_data_size);

				if (tank_data_frame !=NULL && tank_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(ttgsdt_TankData , tank_data_frame, 0, tank_data_size));
					g_free(tank_data_frame);
				}

				tlv_delete_units(&tank_data_units);

			}
		}
	}

	guint32 data_size = 0;
	guchar* data_frame = tlv_serialize_units(data_units, &data_size);

	if (data_frame !=NULL && data_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_TGSTankData , data_frame, 0, data_size));
		g_free(data_frame);
	}
	tlv_delete_units(&data_units);

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}

	tlv_delete_units(&units);

}
