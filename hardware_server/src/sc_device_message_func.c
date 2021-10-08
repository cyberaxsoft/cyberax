#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tlv.h"
#include "socket_func.h"
#include "sc_device.h"
#include "sc_device_data.h"
#include "sc_func.h"

void send_sc_device_status_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%d Send SC device status message", client_info->socket);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d SC Device status message:", client_info->socket);

	ScDeviceStatus  status = get_sc_device_status(sc_index);

	guint32 status_description_length = 0;
	gchar* status_desc = sc_status_description(status, &status_description_length);

	ScDeviceError  device_error = get_sc_device_last_erorr(sc_index);

	guint32 error_description_length = 0;
	gchar* error_desc = sc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetDeviceStatus, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetDeviceStatus, server_command_to_str(hsc_GetDeviceStatus));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, sc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, sc_error_to_str(device_error));
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

void send_sc_device_configuration_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send SC device configurtion message:", client_info->socket);

	ScDeviceStatus  status = get_sc_device_status(sc_index);

	guint32 status_description_length = 0;
	gchar* status_desc = sc_status_description(status, &status_description_length);

	ScDeviceError  device_error = get_sc_device_last_erorr(sc_index);

	guint32 error_description_length = 0;
	gchar* error_desc = sc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetConfiguration, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetConfiguration, server_command_to_str(hsc_GetConfiguration));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, sc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, sc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* conf_units = NULL;

	guint8 sensor_count = get_sc_sensor_count(sc_index);

	add_tlv_unit_8( &conf_units, tscct_SensorCount,sensor_count,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Sensor count: %d", client_info->socket, sensor_count);

	if (sensor_count  > 0)
	{
		for (guint8 i = 0; i < sensor_count; i++)
		{
			guint32 sensor_num = 0;
			guint8  sensor_addr = 0;
			SensorParams sensor_params = {0x00};

			get_sc_sensor_info(sc_index, i, &sensor_num, &sensor_addr, &sensor_params);

			TlvUnit* sensor_conf_units = NULL;

			add_tlv_unit_32(&sensor_conf_units, tscsct_Number, 	sensor_num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Sensor num: %d", client_info->socket, sensor_num);
			add_tlv_unit_8(&sensor_conf_units, tscsct_Addr, 	sensor_addr, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Sensor addr: %d", client_info->socket, sensor_addr);

			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 			Params:", client_info->socket);

			if (sensor_params.param_count > 0)
			{
				for (guint8 j = 0; j < sensor_params.param_count; j++)
				{
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 				Param:", client_info->socket);

					TlvUnit* sensor_param_units = NULL;

					add_tlv_unit_8(&sensor_param_units, tscspct_Number, 	sensor_params.params[j].num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Num: %d", client_info->socket, sensor_params.params[j].num);
					add_tlv_unit_8(&sensor_param_units, tscspct_Type, 		sensor_params.params[j].type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Type: %d (%s)", client_info->socket, sensor_params.params[j].type, spt_to_str(sensor_params.params[j].type));

					guint32 sensor_param_conf_size = 0;
					guchar* sensor_param_conf_frame = tlv_serialize_units(sensor_param_units, &sensor_param_conf_size);

					if (sensor_param_conf_frame !=NULL && sensor_param_conf_size > 0)
					{
						tlv_add_unit(&sensor_conf_units,tlv_create_unit(tscsct_Param , sensor_param_conf_frame, 0, sensor_param_conf_size));
						g_free(sensor_param_conf_frame);
					}

					tlv_delete_units(&sensor_param_units);
				}

			}

			guint32 sensor_conf_size = 0;
			guchar* sensor_conf_frame = tlv_serialize_units(sensor_conf_units, &sensor_conf_size);

			if (sensor_conf_frame !=NULL && sensor_conf_size > 0)
			{
				tlv_add_unit(&conf_units,tlv_create_unit(tscct_SensorConfiguration , sensor_conf_frame, 0, sensor_conf_size));
				g_free(sensor_conf_frame);
			}

			tlv_delete_units(&sensor_conf_units);

		}
	}

	guint32 configuration_size = 0;
	guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

	if (configuration_frame !=NULL && configuration_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_SCDeviceConfiguration , configuration_frame, 0, configuration_size));
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

void send_sc_device_data_message(guint8 client_index, guint8 sc_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 sensor_num, guint8 sensor_index, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send SC device data message:", client_info->socket);

	ScDeviceStatus  status = get_sc_device_status(sc_index);

	guint32 status_description_length = 0;
	gchar* status_desc = sc_status_description(status, &status_description_length);

	ScDeviceError  device_error = get_sc_device_last_erorr(sc_index);

	guint32 error_description_length = 0;
	gchar* error_desc = sc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_SCGetSensorData, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_SCGetSensorData, server_command_to_str(hsc_SCGetSensorData));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, sc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, sc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* data_units = NULL;

	guint8 sensor_count = get_sc_sensor_count(sc_index);

	if (sensor_count > 0)
	{
		if (sensor_num == 0)
		{
			//all sensor data
			add_tlv_unit_8( &data_units, tscdt_Count,sensor_count,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Sensor count: %d", client_info->socket, sensor_count);

			for (guint8 i = 0; i < sensor_count; i++)
			{
				guint32 num = 0;
				SensorParams params = {0x00};
				gboolean online = FALSE;

				if (get_sc_sensor_data(sc_index, i, &num, &params, &online))
				{
					TlvUnit* sensor_data_units = NULL;

					add_tlv_unit_32(&sensor_data_units, 	tscsdt_Number, 		num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Sensor num: %d", client_info->socket, num);

					if (params.param_count > 0)
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 			Params:", client_info->socket);

						for (guint8 j = 0; j < params.param_count; j++)
						{
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 				Param:", client_info->socket);

							TlvUnit* sensor_param_units = NULL;

							add_tlv_unit_8(&sensor_param_units, tscspdt_Number, 	params.params[j].num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Num: %d", client_info->socket, params.params[j].num);
							add_tlv_unit_8(&sensor_param_units, tscspdt_Type, 		params.params[j].type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Type: %d (%s)", client_info->socket, params.params[j].type, spt_to_str(params.params[j].type));
							add_tlv_unit_bytes(&sensor_param_units, tscspdt_Data, 	params.params[j].value, SENSOR_PARAM_VALUE_SIZE, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Value: %02X%02X%02X%02X%02X%02X%02X%02X", client_info->socket,
									params.params[j].value[0], params.params[j].value[1], params.params[j].value[2], params.params[j].value[3], params.params[j].value[4], params.params[j].value[5], params.params[j].value[6], params.params[j].value[7]);

							guint32 sensor_param_data_size = 0;
							guchar* sensor_param_data_frame = tlv_serialize_units(sensor_param_units, &sensor_param_data_size);

							if (sensor_param_data_frame !=NULL && sensor_param_data_size > 0)
							{
								tlv_add_unit(&sensor_data_units,tlv_create_unit(tscsdt_Param , sensor_param_data_frame, 0, sensor_param_data_size));
								g_free(sensor_param_data_frame);
							}

							tlv_delete_units(&sensor_param_units);
						}
					}


					add_tlv_unit_8(&sensor_data_units, 	tscsdt_Online, 		tlv_bool_to_byte(online), 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Online: %d", client_info->socket, online);

					guint32 sensor_data_size = 0;
					guchar* sensor_data_frame = tlv_serialize_units(sensor_data_units, &sensor_data_size);

					if (sensor_data_frame !=NULL && sensor_data_size > 0)
					{
						tlv_add_unit(&data_units,tlv_create_unit(tscdt_SensorData , sensor_data_frame, 0, sensor_data_size));
						g_free(sensor_data_frame);
					}

					tlv_delete_units(&sensor_data_units);

				}
			}
		}
		else
		{
			add_tlv_unit_8( &data_units, tscdt_Count,1,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Sensor count: %d", client_info->socket, 1);

			guint32 num = 0;
			SensorParams params = {0x00};
			gboolean online = FALSE;

			if (get_sc_sensor_data(sc_index, sensor_index, &num, &params, &online))
			{
				TlvUnit* sensor_data_units = NULL;

				add_tlv_unit_32(&sensor_data_units, 	tscsdt_Number, 		num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Sensor num: %d", client_info->socket, num);

				if (params.param_count > 0)
				{
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 			Params:", client_info->socket);

					for (guint8 j = 0; j < params.param_count; j++)
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 				Param:", client_info->socket);

						TlvUnit* sensor_param_units = NULL;

						add_tlv_unit_8(&sensor_param_units, tscspdt_Number, 	params.params[j].num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Num: %d", client_info->socket, params.params[j].num);
						add_tlv_unit_8(&sensor_param_units, tscspdt_Type, 		params.params[j].type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Type: %d (%s)", client_info->socket, params.params[j].type, spt_to_str(params.params[j].type));
						add_tlv_unit_bytes(&sensor_param_units, tscspdt_Data, 	params.params[j].value, SENSOR_PARAM_VALUE_SIZE, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Value: %02X%02X%02X%02X%02X%02X%02X%02X", client_info->socket,
								params.params[j].value[0], params.params[j].value[1], params.params[j].value[2], params.params[j].value[3], params.params[j].value[4], params.params[j].value[5], params.params[j].value[6], params.params[j].value[7]);

						guint32 sensor_param_data_size = 0;
						guchar* sensor_param_data_frame = tlv_serialize_units(sensor_param_units, &sensor_param_data_size);

						if (sensor_param_data_frame !=NULL && sensor_param_data_size > 0)
						{
							tlv_add_unit(&sensor_data_units,tlv_create_unit(tscsdt_Param , sensor_param_data_frame, 0, sensor_param_data_size));
							g_free(sensor_param_data_frame);
						}

						tlv_delete_units(&sensor_param_units);
					}
				}

				add_tlv_unit_8(&sensor_data_units, 	tscsdt_Online, 		tlv_bool_to_byte(online), 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Online: %d", client_info->socket, online);

				guint32 sensor_data_size = 0;
				guchar* sensor_data_frame = tlv_serialize_units(sensor_data_units, &sensor_data_size);

				if (sensor_data_frame !=NULL && sensor_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(tscdt_SensorData , sensor_data_frame, 0, sensor_data_size));
					g_free(sensor_data_frame);
				}

				tlv_delete_units(&sensor_data_units);

			}
		}
	}

	guint32 data_size = 0;
	guchar* data_frame = tlv_serialize_units(data_units, &data_size);

	if (data_frame !=NULL && data_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_SCData , data_frame, 0, data_size));
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

