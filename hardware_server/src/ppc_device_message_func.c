#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tlv.h"
#include "socket_func.h"
#include "ppc_device.h"
#include "ppc_device_data.h"
#include "ppc_func.h"

void send_ppc_device_status_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%d Send device status message", client_info->socket);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Device status message:", client_info->socket);

	PpcDeviceStatus  status = get_ppc_device_status(price_pole_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = ppc_status_description(status, &status_description_length);

	PpcDeviceError  device_error = get_ppc_device_last_erorr(price_pole_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = ppc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;


	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetDeviceStatus, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetDeviceStatus, server_command_to_str(hsc_GetDeviceStatus));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, ppc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, ppc_error_to_str(device_error));
		g_free(error_desc);
	}

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame != NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}
	tlv_delete_units(&units);
}

void send_ppc_device_command_result_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, HardwareServerCommand command, guint8 device_reply_code, ExchangeError exchange_error)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device command result message:", client_info->socket);

	PpcDeviceStatus  status = get_ppc_device_status(price_pole_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = ppc_status_description(status, &status_description_length);

	PpcDeviceError  device_error = get_ppc_device_last_erorr(price_pole_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = ppc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 		message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, 	mt_Reply, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, mt_Reply, return_message_type_name(mt_Reply));
	add_tlv_unit_32(&units, tst_CommandCode, 	command, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, command, server_command_to_str(command));
	add_tlv_unit_8( &units, tst_Error,			exchange_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, exchange_error);
	add_tlv_unit_8( &units, tst_DeviceReplyCode,	device_reply_code,		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device reply code: %d", client_info->socket, device_reply_code);
	add_tlv_unit_8( &units, tst_DeviceStatus,	status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, ppc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, ppc_error_to_str(device_error));
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

void send_ppc_device_configuration_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device configurtion message:", client_info->socket);

	PpcDeviceStatus  status = get_ppc_device_status(price_pole_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = ppc_status_description(status, &status_description_length);

	PpcDeviceError  device_error = get_ppc_device_last_erorr(price_pole_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = ppc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 		message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, 	message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, 	hsc_GetConfiguration, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetConfiguration, server_command_to_str(hsc_GetConfiguration));
	add_tlv_unit_8( &units, tst_Error, 			0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,	status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, ppc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,		device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, ppc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* conf_units = NULL;

	guint8 price_pole_count = get_ppc_price_pole_count(price_pole_controller_index);

	add_tlv_unit_8(&conf_units, tppcct_PricePoleCount, price_pole_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 	Price pole count: %d", client_info->socket, price_pole_count);

	if (price_pole_count  > 0)
	{
		for (guint8 i = 0; i < price_pole_count; i++)
		{
			guint8 num = 0;
			guint8 grade = 0;
			guint8 symbol_count = 0;

			get_ppc_price_pole_info(price_pole_controller_index, i, &num, &grade, &symbol_count);

			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d		Price pole:", client_info->socket);

			TlvUnit* price_pole_conf_units = NULL;

			add_tlv_unit_8(&price_pole_conf_units, tppcppct_Number, num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d			Num: %d", client_info->socket, num);
			add_tlv_unit_8( &price_pole_conf_units, tppcppct_Grade, grade, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Grade: %d", client_info->socket, grade);
			add_tlv_unit_8( &price_pole_conf_units, tppcppct_SymbolCount, symbol_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Symbol count: %d", client_info->socket, symbol_count);

			guint32 price_pole_conf_size = 0;
			guchar* price_pole_conf_frame = tlv_serialize_units(price_pole_conf_units, &price_pole_conf_size);

			if (price_pole_conf_frame !=NULL && price_pole_conf_size > 0)
			{
				tlv_add_unit(&conf_units,tlv_create_unit(tppcct_PricePoleConfiguration , price_pole_conf_frame, 0, price_pole_conf_size));
				g_free(price_pole_conf_frame);
			}

			tlv_delete_units(&price_pole_conf_units);

		}
	}

	guint8 price_dp = 0;

	get_ppc_decimal_pointers(price_pole_controller_index, &price_dp);

	add_tlv_unit_8(&conf_units, tppcct_PriceDp, price_dp, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Price decimal points: %d", client_info->socket, price_dp);

	guint32 configuration_size = 0;
	guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

	if (configuration_frame !=NULL && configuration_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_PPCDeviceConfiguration , configuration_frame, 0, configuration_size));
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

void send_ppc_device_data_message(guint8 client_index, guint8 price_pole_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device data message:", client_info->socket);

	PpcDeviceStatus  status = get_ppc_device_status(price_pole_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = ppc_status_description(status, &status_description_length);

	PpcDeviceError  device_error = get_ppc_device_last_erorr(price_pole_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = ppc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, 	tst_MessageId, 		message_id, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8(&units, 	tst_MessageType,	message_type, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, 	tst_CommandCode, 	hsc_PPGetData, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_PPGetData, server_command_to_str(hsc_PPGetData));
	add_tlv_unit_8(&units, 	tst_Error,			0,			 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8(&units, 	tst_DeviceStatus,	status,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, ppc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, ppc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* data_units = NULL;

	guint8 price_pole_count = get_ppc_price_pole_count(price_pole_controller_index);

	if (price_pole_count > 0)
	{
			//all disp data
			add_tlv_unit_8(&data_units, tppcdt_Count,	price_pole_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Price pole count: %d", client_info->socket, price_pole_count);

			for (guint8 i = 0; i < price_pole_count; i++)
			{
				guint8 num = 0;
				guint8 grade = 0;
				guint32 price = 0;
				PricePoleState state = pps_NotInitialize;

				if (get_ppc_price_pole_data(price_pole_controller_index, i, &num, &grade, &price, &state))
				{
					TlvUnit* price_pole_data_units = NULL;

					add_tlv_unit_8(&price_pole_data_units, tppcppdt_Number,		num, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Number: %d", client_info->socket, num);
					add_tlv_unit_8(&price_pole_data_units, tppcppdt_Grade,		grade, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Grade: %d", client_info->socket, grade);
					add_tlv_unit_32(&price_pole_data_units, tppcppdt_Price,		price, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Price: %d", client_info->socket, price);
					add_tlv_unit_8(&price_pole_data_units, tppcppdt_State,		state, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			State: %d (%s)", client_info->socket, state, price_pole_state_to_str(state));

					guint32 price_pole_data_size = 0;
					guchar* price_pole_data_frame = tlv_serialize_units(price_pole_data_units, &price_pole_data_size);

					if (price_pole_data_frame !=NULL && price_pole_data_size > 0)
					{
						tlv_add_unit(&data_units,tlv_create_unit(tppcdt_PricePoleData , price_pole_data_frame, 0, price_pole_data_size));
						g_free(price_pole_data_frame);
					}

					tlv_delete_units(&price_pole_data_units);

				}
			}

	}

	guint32 data_size = 0;
	guchar* data_frame = tlv_serialize_units(data_units, &data_size);

	if (data_frame !=NULL && data_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_PPCPricePoleData , data_frame, 0, data_size));
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

