#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tlv.h"
#include "socket_func.h"
#include "dc_device.h"
#include "dc_device_data.h"
#include "dc_func.h"

void send_dc_device_status_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%d Send device status message", client_info->socket);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Device status message:", client_info->socket);

	DcDeviceStatus  status = get_dc_device_status(dispencer_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = dc_status_description(status, &status_description_length);

	DcDeviceError  device_error = get_dc_device_last_erorr(dispencer_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = dc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;


	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetDeviceStatus, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetDeviceStatus, server_command_to_str(hsc_GetDeviceStatus));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, dc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, dc_error_to_str(device_error));
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

void send_dc_device_command_result_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, HardwareServerCommand command, guint8 device_reply_code, ExchangeError exchange_error, guint32 device_num)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device command result message:", client_info->socket);

	DcDeviceStatus  status = get_dc_device_status(dispencer_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = dc_status_description(status, &status_description_length);

	DcDeviceError  device_error = get_dc_device_last_erorr(dispencer_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = dc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 			message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, 		mt_Reply, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, mt_Reply, return_message_type_name(mt_Reply));
	add_tlv_unit_32(&units, tst_CommandCode, 		command, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, command, server_command_to_str(command));
	add_tlv_unit_32(&units, tst_DeviceNumber,		device_num, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device num: %d", client_info->socket, device_num);
	add_tlv_unit_8( &units, tst_Error,				exchange_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, exchange_error);
	add_tlv_unit_8( &units, tst_DeviceReplyCode,	device_reply_code,		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device reply code: %d", client_info->socket, device_reply_code);
	add_tlv_unit_8( &units, tst_DeviceStatus,		status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, dc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, dc_error_to_str(device_error));
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

void send_dc_device_configuration_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device configurtion message:", client_info->socket);

	DcDeviceStatus  status = get_dc_device_status(dispencer_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = dc_status_description(status, &status_description_length);

	DcDeviceError  device_error = get_dc_device_last_erorr(dispencer_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = dc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 		message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, 	message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, 	hsc_GetConfiguration, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetConfiguration, server_command_to_str(hsc_GetConfiguration));
	add_tlv_unit_8( &units, tst_Error, 			0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8( &units, tst_DeviceStatus,	status,					client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, dc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,		device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, dc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* conf_units = NULL;

	guint8 disp_count = get_dc_disp_count(dispencer_controller_index);

	add_tlv_unit_8(&conf_units, tdcct_DispencerCount, disp_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 	Dispencer count: %d", client_info->socket, disp_count);

	if (disp_count  > 0)
	{
		for (guint8 i = 0; i < disp_count; i++)
		{
			guint32 disp_num = 0;
			guint8  disp_addr = 0;
			guint8 disp_nozzle_count = 0;

			get_dc_disp_info(dispencer_controller_index, i, &disp_num, &disp_addr, &disp_nozzle_count);

			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d		Dispencer:", client_info->socket);

			TlvUnit* disp_conf_units = NULL;

			add_tlv_unit_32(&disp_conf_units, tdcdct_Number, disp_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d			Num: %d", client_info->socket, disp_num);
			add_tlv_unit_8( &disp_conf_units, tdcdct_Address, disp_addr, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Address: %d", client_info->socket, disp_addr);
			add_tlv_unit_8( &disp_conf_units, tdcdct_NozzleCount, disp_nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Nozzle count: %d", client_info->socket, disp_nozzle_count);

			if (disp_nozzle_count > 0)
			{
				for (guint8 j = 0; j < disp_nozzle_count; j++)
				{
					TlvUnit* nozzle_conf_units = NULL;

					guint8 nozzle_num = 0;
					guint8 nozzle_grade = 0;

					get_dc_nozzle_info(dispencer_controller_index, i, j, &nozzle_num, &nozzle_grade);

					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d			Nozzle:", client_info->socket);

					add_tlv_unit_8( &nozzle_conf_units, tdcnct_Number, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d				Number: %d", client_info->socket, nozzle_num);
					add_tlv_unit_8( &nozzle_conf_units, tdcnct_Grade, nozzle_grade, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d				Grade: %d", client_info->socket, nozzle_grade);

					guint32 nozzle_conf_size = 0;
					guchar* nozzle_conf_frame = tlv_serialize_units(nozzle_conf_units, &nozzle_conf_size);

					if (nozzle_conf_frame !=NULL && nozzle_conf_size > 0)
					{
						tlv_add_unit(&disp_conf_units,tlv_create_unit(tdcdct_Nozzle , nozzle_conf_frame, 0, nozzle_conf_size));
						g_free(nozzle_conf_frame);
					}

					tlv_delete_units(&nozzle_conf_units);

				}
			}

			guint32 disp_conf_size = 0;
			guchar* disp_conf_frame = tlv_serialize_units(disp_conf_units, &disp_conf_size);

			if (disp_conf_frame !=NULL && disp_conf_size > 0)
			{
				tlv_add_unit(&conf_units,tlv_create_unit(tdcct_DispencerConfiguration , disp_conf_frame, 0, disp_conf_size));
				g_free(disp_conf_frame);
			}

			tlv_delete_units(&disp_conf_units);

		}
	}

	guint8 ext_func_count = get_dc_ext_func_count(dispencer_controller_index);

	add_tlv_unit_8(&conf_units, tdcct_ExtendedFuncCount, ext_func_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d		Extended func count: %d", client_info->socket, ext_func_count);

	if (ext_func_count > 0)
	{
		for (guint8 i = 0; i < ext_func_count; i++)
		{
			gchar* func_name = get_dc_ext_func_name(dispencer_controller_index, i);

			if (func_name != NULL)
			{
				gsize length = 0;
				gchar* unicode_name = g_convert_with_fallback(func_name, -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);

				if (unicode_name!=NULL)
				{
					TlvUnit* ext_func_units = NULL;

					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d		Extended func:", client_info->socket);

					add_tlv_unit_8(&ext_func_units, tdceft_Index, i, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d			Index: %d", client_info->socket, i);
					add_tlv_unit_str(&ext_func_units, tdceft_Name, unicode_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d			Name: %s", client_info->socket, unicode_name);

					guint32 ext_func_size = 0;
					guchar* ext_func_frame = tlv_serialize_units(ext_func_units, &ext_func_size);

					if (ext_func_frame !=NULL && ext_func_size > 0)
					{
						tlv_add_unit(&conf_units,tlv_create_unit(tdcct_ExtendedFunction , ext_func_frame, 0, ext_func_size));
						g_free(ext_func_frame);
					}

					tlv_delete_units(&ext_func_units);

					g_free(unicode_name);
				}

				g_free(func_name);
			}
		}
	}

	guint8 price_dp = 0;
	guint8 volume_dp = 0;
	guint8 amount_dp = 0;

	get_dc_decimal_pointers(dispencer_controller_index, &price_dp, &volume_dp, &amount_dp);

	add_tlv_unit_8(&conf_units, tdcct_PriceDP, price_dp, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Price decimal points: %d", client_info->socket, price_dp);
	add_tlv_unit_8(&conf_units, tdcct_VolumeDP, volume_dp, client_info->log_params, client_info->log_trace, client_info->log_parsing,"%d 		Volime decimal points: %d", client_info->socket, volume_dp);
	add_tlv_unit_8(&conf_units, tdcct_AmountDP, amount_dp, client_info->log_params, client_info->log_trace, client_info->log_parsing,"%d 		Amount decimal points: %d", client_info->socket, amount_dp);

	guint32 configuration_size = 0;
	guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

	if (configuration_frame !=NULL && configuration_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_DCDeviceConfiguration , configuration_frame, 0, configuration_size));
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

void send_dc_device_data_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 disp_num, guint8 disp_index, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device data message:", client_info->socket);

	DcDeviceStatus  status = get_dc_device_status(dispencer_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = dc_status_description(status, &status_description_length);

	DcDeviceError  device_error = get_dc_device_last_erorr(dispencer_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = dc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, 	tst_MessageId, 		message_id, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8(&units, 	tst_MessageType,	message_type, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, 	tst_CommandCode, 	hsc_DCGetData, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_DCGetData, server_command_to_str(hsc_DCGetData));
	add_tlv_unit_8(&units, 	tst_Error,			0,			 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8(&units, 	tst_DeviceStatus,	status,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, dc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, dc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* data_units = NULL;

	guint8 disp_count = get_dc_disp_count(dispencer_controller_index);

	if (disp_count > 0)
	{
		if (disp_num == 0)
		{
			//all disp data
			add_tlv_unit_8(&data_units, tdcdt_Count,	disp_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Dispencer count: %d", client_info->socket, disp_count);

			for (guint8 i = 0; i < disp_count; i++)
			{
				guint32 num = 0;
				DispencerState state = ds_NotInitialize;
				OrderType order_type = ot_Free;
				OrderType preset_order_type = ot_Free;
				gboolean is_pay = FALSE;
				guint8 preset_nozzle_num = 0;
				guint8 active_nozzle_num = 0;
				guint32 preset_price = 0;
				guint32 preset_volume = 0;
				guint32 preset_amount = 0;
				guint32 current_price = 0;
				guint32 current_volume = 0;
				guint32 current_amount = 0;
				guchar error = 0;
				gchar* error_description = NULL;
				guint8 error_description_length = 0;

				if (get_dc_dispencer_data(dispencer_controller_index, i, &num, &state, &order_type, &preset_order_type, &is_pay, &preset_nozzle_num, &active_nozzle_num,
						&preset_price, &preset_volume, &preset_amount, &current_price, &current_volume, &current_amount, &error, error_description, &error_description_length))
				{
					TlvUnit* disp_data_units = NULL;

					add_tlv_unit_32(&disp_data_units, tdcddt_Number,		num, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Dispencer num: %d", client_info->socket, num);
					add_tlv_unit_8(&disp_data_units,  tdcddt_State,		state, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				State: %d (%s)", client_info->socket, state, dispencer_state_to_str(state));
					add_tlv_unit_8(&disp_data_units,  tdcddt_OrderType,	order_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Order type: %d (%s)", client_info->socket, order_type, order_type_to_str(order_type));
					add_tlv_unit_8(&disp_data_units,  tdcddt_PresetOrderType,	preset_order_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset order type: %d (%s)", client_info->socket, preset_order_type, order_type_to_str(preset_order_type));
					add_tlv_unit_8(&disp_data_units,  tdcddt_IsPay,	tlv_bool_to_byte(is_pay), client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Is pay: %d", client_info->socket, is_pay);
					add_tlv_unit_8(&disp_data_units,  tdcddt_PresetNozzleNum,	preset_nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset nozzle num: %d", client_info->socket, preset_nozzle_num);
					add_tlv_unit_8(&disp_data_units,  tdcddt_ActiveNozzleNum,	active_nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Active nozzle num: %d", client_info->socket, active_nozzle_num);
					add_tlv_unit_32(&disp_data_units, tdcddt_PresetPrice, preset_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset price: %d", client_info->socket, preset_price);
					add_tlv_unit_32(&disp_data_units, tdcddt_PresetVolume, preset_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset volume: %d", client_info->socket, preset_volume);
					add_tlv_unit_32(&disp_data_units, tdcddt_PresetAmount, preset_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset amount: %d", client_info->socket, preset_amount);
					add_tlv_unit_32(&disp_data_units, tdcddt_CurrentPrice, current_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current price: %d", client_info->socket, current_price);
					add_tlv_unit_32(&disp_data_units, tdcddt_CurrentVolume, current_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current volume: %d", client_info->socket, current_volume);
					add_tlv_unit_32(&disp_data_units, tdcddt_CurrentAmount, current_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current amount: %d", client_info->socket, current_amount);
					add_tlv_unit_8(&disp_data_units,  tdcddt_ErrorCode,	error, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Error: %d", client_info->socket, error);

					if (error_description!=NULL)
					{
						add_tlv_unit_str(&disp_data_units,  tdcddt_ErrorDescription,	error_description, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Error description: %s", client_info->socket, error_description);
					}

					//---------------------------------  counters -------------------------------------------------------------------------------------------------

					guint8 nozzle_count = 0;
					guint8 addr = 0;
					get_dc_disp_info(dispencer_controller_index, i, &num, &addr, &nozzle_count);

					TlvUnit* disp_counters_units = NULL;

					add_tlv_unit_8(&disp_counters_units, tdcdcdt_NozzleCount, nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Nozzle count: %d", client_info->socket, nozzle_count);

					if (nozzle_count > 0)
					{
						for (guint8 j = 0; j < nozzle_count; j++)
						{
							TlvUnit* nozzle_data_units = NULL;

							guint8 nozzle_num = 0;
							guint32 nozzle_counter = 0;
							get_dc_nozzle_counter(dispencer_controller_index, disp_index, j, &nozzle_num, &nozzle_counter);

							add_tlv_unit_8(&nozzle_data_units, tdcncdt_Number, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 						Nozzle num: %d", client_info->socket, nozzle_num);
							add_tlv_unit_32(&nozzle_data_units, tdcncdt_Counter, nozzle_counter, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 							Counter: %d", client_info->socket, nozzle_counter);

							guint32 nozzle_data_size = 0;
							guchar* nozzle_data_frame = tlv_serialize_units(nozzle_data_units, &nozzle_data_size);

							if (nozzle_data_frame !=NULL && nozzle_data_size > 0)
							{
								tlv_add_unit(&disp_counters_units,tlv_create_unit(tdcdcdt_NozzleCounterData , nozzle_data_frame, 0, nozzle_data_size));
								g_free(nozzle_data_frame);
							}

							tlv_delete_units(&nozzle_data_units);
						}
					}

					guint32 disp_counters_data_size = 0;
					guchar* disp_counters_data_frame = tlv_serialize_units(disp_counters_units, &disp_counters_data_size);

					if (disp_counters_data_frame !=NULL && disp_counters_data_size > 0)
					{
						tlv_add_unit(&disp_data_units,tlv_create_unit(tdcddt_Counters , disp_counters_data_frame, 0, disp_counters_data_size));
						g_free(disp_counters_data_frame);
					}

					tlv_delete_units(&disp_counters_units);

					//---------------------------------------------------------------------------------------------------------------------------------------------


					guint32 disp_data_size = 0;
					guchar* disp_data_frame = tlv_serialize_units(disp_data_units, &disp_data_size);

					if (disp_data_frame !=NULL && disp_data_size > 0)
					{
						tlv_add_unit(&data_units,tlv_create_unit(tdcdt_DispencerData , disp_data_frame, 0, disp_data_size));
						g_free(disp_data_frame);
					}

					tlv_delete_units(&disp_data_units);

				}
			}
		}
		else
		{
			add_tlv_unit_8(&data_units, tdcdt_Count,	1, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Dispencer count: %d", client_info->socket, 1);

			guint32 num = 0;
			DispencerState state = ds_NotInitialize;
			OrderType order_type = ot_Free;
			OrderType preset_order_type = ot_Free;
			gboolean is_pay = FALSE;
			guint8 preset_nozzle_num = 0;
			guint8 active_nozzle_num = 0;
			guint32 preset_price = 0;
			guint32 preset_volume = 0;
			guint32 preset_amount = 0;
			guint32 current_price = 0;
			guint32 current_volume = 0;
			guint32 current_amount = 0;
			guchar error = 0;
			gchar* error_description = NULL;
			guint8 error_description_length = 0;

			if (get_dc_dispencer_data(dispencer_controller_index, disp_index, &num, &state, &order_type, &preset_order_type,
					&is_pay, &preset_nozzle_num, &active_nozzle_num, &preset_price, &preset_volume, &preset_amount,
					&current_price, &current_volume, &current_amount, &error, error_description, &error_description_length))
			{
				TlvUnit* disp_data_units = NULL;

				add_tlv_unit_32(&disp_data_units, tdcddt_Number,		num, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Dispencer num: %d", client_info->socket, num);
				add_tlv_unit_8(&disp_data_units,  tdcddt_State,		state, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				State: %d (%s)", client_info->socket, state, dispencer_state_to_str(state));
				add_tlv_unit_8(&disp_data_units,  tdcddt_OrderType,	order_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Order type: %d (%s)", client_info->socket, order_type, order_type_to_str(order_type));
				add_tlv_unit_8(&disp_data_units,  tdcddt_PresetOrderType,	preset_order_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset order type: %d (%s)", client_info->socket, preset_order_type, order_type_to_str(preset_order_type));
				add_tlv_unit_8(&disp_data_units,  tdcddt_IsPay,	tlv_bool_to_byte(is_pay), client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Is pay: %d", client_info->socket, is_pay);
				add_tlv_unit_8(&disp_data_units,  tdcddt_PresetNozzleNum,	preset_nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset nozzle num: %d", client_info->socket, preset_nozzle_num);
				add_tlv_unit_8(&disp_data_units,  tdcddt_ActiveNozzleNum,	active_nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Active nozzle num: %d", client_info->socket, active_nozzle_num);
				add_tlv_unit_32(&disp_data_units, tdcddt_PresetPrice, preset_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset price: %d", client_info->socket, preset_price);
				add_tlv_unit_32(&disp_data_units, tdcddt_PresetVolume, preset_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset volume: %d", client_info->socket, preset_volume);
				add_tlv_unit_32(&disp_data_units, tdcddt_PresetAmount, preset_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Preset amount: %d", client_info->socket, preset_amount);
				add_tlv_unit_32(&disp_data_units, tdcddt_CurrentPrice, current_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current price: %d", client_info->socket, current_price);
				add_tlv_unit_32(&disp_data_units, tdcddt_CurrentVolume, current_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current volume: %d", client_info->socket, current_volume);
				add_tlv_unit_32(&disp_data_units, tdcddt_CurrentAmount, current_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Current amount: %d", client_info->socket, current_amount);
				add_tlv_unit_8(&disp_data_units,  tdcddt_ErrorCode,	error, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Error: %d", client_info->socket, error);

				if (error_description!=NULL)
				{
					add_tlv_unit_str(&disp_data_units,  tdcddt_ErrorDescription,	error_description, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Error description: %s", client_info->socket, error_description);
				}

				//---------------------------------  counters -------------------------------------------------------------------------------------------------

				guint8 nozzle_count = 0;
				guint8 addr = 0;
				get_dc_disp_info(dispencer_controller_index, disp_index, &num, &addr, &nozzle_count);

				TlvUnit* disp_counters_units = NULL;

				add_tlv_unit_8(&disp_counters_units, tdcdcdt_NozzleCount, nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Nozzle count: %d", client_info->socket, nozzle_count);

				if (nozzle_count > 0)
				{
					for (guint8 j = 0; j < nozzle_count; j++)
					{
						TlvUnit* nozzle_data_units = NULL;

						guint8 nozzle_num = 0;
						guint32 nozzle_counter = 0;
						get_dc_nozzle_counter(dispencer_controller_index, disp_index, j, &nozzle_num, &nozzle_counter);

						add_tlv_unit_8(&nozzle_data_units, tdcncdt_Number, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 						Nozzle num: %d", client_info->socket, nozzle_num);
						add_tlv_unit_32(&nozzle_data_units, tdcncdt_Counter, nozzle_counter, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 							Counter: %d", client_info->socket, nozzle_counter);

						guint32 nozzle_data_size = 0;
						guchar* nozzle_data_frame = tlv_serialize_units(nozzle_data_units, &nozzle_data_size);

						if (nozzle_data_frame !=NULL && nozzle_data_size > 0)
						{
							tlv_add_unit(&disp_counters_units,tlv_create_unit(tdcdcdt_NozzleCounterData , nozzle_data_frame, 0, nozzle_data_size));
							g_free(nozzle_data_frame);
						}

						tlv_delete_units(&nozzle_data_units);
					}
				}

				guint32 disp_counters_data_size = 0;
				guchar* disp_counters_data_frame = tlv_serialize_units(disp_counters_units, &disp_counters_data_size);

				if (disp_counters_data_frame !=NULL && disp_counters_data_size > 0)
				{
					tlv_add_unit(&disp_data_units,tlv_create_unit(tdcddt_Counters , disp_counters_data_frame, 0, disp_counters_data_size));
					g_free(disp_counters_data_frame);
				}

				tlv_delete_units(&disp_counters_units);

				//---------------------------------------------------------------------------------------------------------------------------------------------


				guint32 disp_data_size = 0;
				guchar* disp_data_frame = tlv_serialize_units(disp_data_units, &disp_data_size);

				if (disp_data_frame !=NULL && disp_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(tdcdt_DispencerData , disp_data_frame, 0, disp_data_size));
					g_free(disp_data_frame);
				}

				tlv_delete_units(&disp_data_units);

			}
		}
	}

	guint32 data_size = 0;
	guchar* data_frame = tlv_serialize_units(data_units, &data_size);

	if (data_frame !=NULL && data_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_DCData , data_frame, 0, data_size));
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

void send_dc_device_counters_message(guint8 client_index, guint8 dispencer_controller_index, SockClientInfo* client_info, guint32 message_id, MessageType message_type, guint32 disp_num, guint8 nozzle_num, guint8 disp_index, guint8 nozzle_index, gchar* log_prefix)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d Send device counters message:", client_info->socket);

	DcDeviceStatus  status = get_dc_device_status(dispencer_controller_index);

	guint32 status_description_length = 0;
	gchar* status_desc = dc_status_description(status, &status_description_length);

	DcDeviceError  device_error = get_dc_device_last_erorr(dispencer_controller_index);

	guint32 error_description_length = 0;
	gchar* error_desc = dc_error_description(device_error, &error_description_length);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, 	tst_MessageId, 		message_id, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8(&units, 	tst_MessageType,	message_type, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, 	tst_CommandCode, 	hsc_DCGetCounters, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_DCGetCounters, server_command_to_str(hsc_DCGetCounters));
	add_tlv_unit_8(&units, 	tst_Error,			0,			 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);
	add_tlv_unit_8(&units, 	tst_DeviceStatus,	status,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status: %d", client_info->socket, status);

	if (status_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceStatusDescription, status_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Status description: %s", client_info->socket, dc_status_to_str(status));
		g_free(status_desc);
	}

	add_tlv_unit_8( &units, tst_DeviceError,device_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error: %d", client_info->socket, device_error);

	if (error_desc != NULL)
	{
		add_tlv_unit_str(&units, tst_DeviceErrorDescription, error_desc, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Device error description: %s", client_info->socket, dc_error_to_str(device_error));
		g_free(error_desc);
	}

	TlvUnit* data_units = NULL;

	guint8 disp_count = get_dc_disp_count(dispencer_controller_index);

	if (disp_count > 0)
	{
		if (disp_num == 0)
		{
			add_tlv_unit_8(&data_units, tdcdt_Count,	disp_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Dispencer count: %d", client_info->socket, disp_count);

			for (guint8 i = 0; i < disp_count; i++)
			{
				guint32 num = 0;
				guint8	addr = 0;
				guint8 nozzle_count = 0;
				get_dc_disp_info(dispencer_controller_index, i, &num, &addr, &nozzle_count);

				TlvUnit* disp_data_units = NULL;

				add_tlv_unit_32(&disp_data_units, tdcdcdt_DispencerNumber, num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Dispencer num: %d", client_info->socket, num);
				add_tlv_unit_8(&disp_data_units, tdcdcdt_NozzleCount, nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Nozzle count: %d", client_info->socket, nozzle_count);

				if (nozzle_count > 0)
				{
					for (guint8 j = 0; j < nozzle_count; j++)
					{
						TlvUnit* nozzle_data_units = NULL;

						guint8 nozzle_num = 0;
						guint32 nozzle_counter = 0;
						get_dc_nozzle_counter(dispencer_controller_index, i, j, &nozzle_num, &nozzle_counter);

						add_tlv_unit_8(&nozzle_data_units, tdcncdt_Number, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Nozzle num: %d", client_info->socket, nozzle_num);
						add_tlv_unit_32(&nozzle_data_units, tdcncdt_Counter, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Counter: %d", client_info->socket, nozzle_counter);

						guint32 nozzle_data_size = 0;
						guchar* nozzle_data_frame = tlv_serialize_units(nozzle_data_units, &nozzle_data_size);

						if (nozzle_data_frame !=NULL && nozzle_data_size > 0)
						{
							tlv_add_unit(&disp_data_units,tlv_create_unit(tdcdcdt_NozzleCounterData , nozzle_data_frame, 0, nozzle_data_size));
							g_free(nozzle_data_frame);
						}

						tlv_delete_units(&nozzle_data_units);
					}
				}

				guint32 disp_data_size = 0;
				guchar* disp_data_frame = tlv_serialize_units(disp_data_units, &disp_data_size);

				if (disp_data_frame !=NULL && disp_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(tdccnt_DispencerCountersData, disp_data_frame, 0, disp_data_size));
					g_free(disp_data_frame);
				}

				tlv_delete_units(&disp_data_units);

			}
		}
		else
		{
			if (nozzle_num == 0)
			{
				add_tlv_unit_8(&data_units, tdcdt_Count,	1, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Dispencer count: %d", client_info->socket, 1);

				guint32 num = 0;
				guint8	addr = 0;
				guint8 nozzle_count = 0;
				get_dc_disp_info(dispencer_controller_index, disp_index, &num, &addr, &nozzle_count);

				TlvUnit* disp_data_units = NULL;

				add_tlv_unit_32(&disp_data_units, tdcdcdt_DispencerNumber, num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 			Dispencer num: %d", client_info->socket, num);
				add_tlv_unit_8(&disp_data_units, tdcdcdt_NozzleCount, nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Nozzle count: %d", client_info->socket, nozzle_count);

				if (nozzle_count > 0)
				{
					for (guint8 j = 0; j < nozzle_count; j++)
					{
						TlvUnit* nozzle_data_units = NULL;

						guint8 nozzle_num = 0;
						guint32 nozzle_counter = 0;
						get_dc_nozzle_counter(dispencer_controller_index, disp_index, j, &nozzle_num, &nozzle_counter);

						add_tlv_unit_8(&nozzle_data_units, tdcncdt_Number, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 				Nozzle num: %d", client_info->socket, nozzle_num);
						add_tlv_unit_32(&nozzle_data_units, tdcncdt_Counter, nozzle_num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 					Counter: %d", client_info->socket, nozzle_counter);

						guint32 nozzle_data_size = 0;
						guchar* nozzle_data_frame = tlv_serialize_units(nozzle_data_units, &nozzle_data_size);

						if (nozzle_data_frame !=NULL && nozzle_data_size > 0)
						{
							tlv_add_unit(&disp_data_units,tlv_create_unit(tdcdcdt_NozzleCounterData , nozzle_data_frame, 0, nozzle_data_size));
							g_free(nozzle_data_frame);
						}

						tlv_delete_units(&nozzle_data_units);
					}
				}

				guint32 disp_data_size = 0;
				guchar* disp_data_frame = tlv_serialize_units(disp_data_units, &disp_data_size);

				if (disp_data_frame !=NULL && disp_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(tdccnt_DispencerCountersData , disp_data_frame, 0, disp_data_size));
					g_free(disp_data_frame);
				}

				tlv_delete_units(&disp_data_units);

			}
			else
			{
				guchar disp_count_buffer[] = { 1 };
				tlv_add_unit(&data_units,tlv_create_unit(tdccnt_Count , disp_count_buffer, 0, sizeof(disp_count_buffer)));
				add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%d 		Dispencer count: %d", client_info->socket, disp_count);

				guint32 num = 0;
				guint8	addr = 0;
				guint8 nozzle_count = 0;
				get_dc_disp_info(dispencer_controller_index, disp_index, &num, &addr, &nozzle_count);

				TlvUnit* disp_data_units = NULL;

				guchar disp_num_buffer[] = {(num >> 24) & 0xFF,(num >> 16) & 0xFF,(num >> 8) & 0xFF,num & 0xFF};
				tlv_add_unit(&disp_data_units,tlv_create_unit(tdcdcdt_DispencerNumber , disp_num_buffer, 0, sizeof(disp_num_buffer)));
				add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
						"%d 			Dispencer num: %d", client_info->socket, num);

				guchar disp_nozzle_count_buffer[] = { 1 };
				tlv_add_unit(&disp_data_units,tlv_create_unit(tdcdcdt_NozzleCount , disp_nozzle_count_buffer, 0, sizeof(disp_nozzle_count_buffer)));
				add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
						"%d 				Nozzle count: %d", client_info->socket, 1);

				if (nozzle_index < nozzle_count)
				{
					TlvUnit* nozzle_data_units = NULL;

					guint8 nozzle_num = 0;
					guint32 nozzle_counter = 0;
					get_dc_nozzle_counter(dispencer_controller_index, disp_index, nozzle_index, &nozzle_num, &nozzle_counter);

					guchar nozzle_num_buffer[] = {nozzle_num};
					tlv_add_unit(&nozzle_data_units,tlv_create_unit(tdcncdt_Number , nozzle_num_buffer, 0, sizeof(nozzle_num_buffer)));

					guchar nozzle_counter_buffer[] = {(nozzle_counter >> 24) & 0xFF,(nozzle_counter >> 16) & 0xFF,(nozzle_counter >> 8) & 0xFF,nozzle_counter & 0xFF};
					tlv_add_unit(&nozzle_data_units,tlv_create_unit(tdcncdt_Counter , nozzle_counter_buffer, 0, sizeof(nozzle_counter_buffer)));

					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
							"%d 					Nozzle %d: %d", client_info->socket, nozzle_num, nozzle_counter);

					guint32 nozzle_data_size = 0;
					guchar* nozzle_data_frame = tlv_serialize_units(nozzle_data_units, &nozzle_data_size);

					if (nozzle_data_frame !=NULL && nozzle_data_size > 0)
					{
						tlv_add_unit(&disp_data_units,tlv_create_unit(tdcdcdt_NozzleCounterData , nozzle_data_frame, 0, nozzle_data_size));
						g_free(nozzle_data_frame);
					}

					tlv_delete_units(&nozzle_data_units);
				}

				guint32 disp_data_size = 0;
				guchar* disp_data_frame = tlv_serialize_units(disp_data_units, &disp_data_size);

				if (disp_data_frame !=NULL && disp_data_size > 0)
				{
					tlv_add_unit(&data_units,tlv_create_unit(tdccnt_DispencerCountersData , disp_data_frame, 0, disp_data_size));
					g_free(disp_data_frame);
				}

				tlv_delete_units(&disp_data_units);

			}
		}
	}

	guint32 data_size = 0;
	guchar* data_frame = tlv_serialize_units(data_units, &data_size);

	if (data_frame !=NULL && data_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_DCCounters , data_frame, 0, data_size));
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
