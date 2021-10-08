#include <glib.h>
#include <glib/gstdio.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "pss.h"
#include "pss_tlv.h"
#include "pss_client_thread.h"
#include "pss_data.h"
#include "pss_client_data.h"
#include "pss_parse.h"
#include "pss_func.h"
#include "pss_dc_device.h"

void send_dc_get_status_message(PSSClientThreadFuncParam* params, guint8 device_index)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d prepare get status message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_GetDeviceStatus >> 24) & 0xFF, (hsc_GetDeviceStatus >> 16) & 0xFF, (hsc_GetDeviceStatus >> 8) & 0xFF, hsc_GetDeviceStatus & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_GetDeviceStatus, server_command_to_str(hsc_GetDeviceStatus));

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}


void send_stop_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d prepare stop message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCStop >> 24) & 0xFF, (hsc_DCStop >> 16) & 0xFF, (hsc_DCStop >> 8) & 0xFF, hsc_DCStop & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCStop, server_command_to_str(hsc_DCStop));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}

void send_suspend_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare stop message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCSuspend >> 24) & 0xFF, (hsc_DCSuspend >> 16) & 0xFF, (hsc_DCSuspend >> 8) & 0xFF, hsc_DCSuspend & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCSuspend, server_command_to_str(hsc_DCSuspend));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}

void send_resume_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare stop message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCResume >> 24) & 0xFF, (hsc_DCResume >> 16) & 0xFF, (hsc_DCResume >> 8) & 0xFF, hsc_DCResume & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCResume, server_command_to_str(hsc_DCResume));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}

void send_reset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare reset message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCReset >> 24) & 0xFF, (hsc_DCReset >> 16) & 0xFF, (hsc_DCReset >> 8) & 0xFF, hsc_DCReset & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCReset, server_command_to_str(hsc_DCReset));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}


void send_start_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare start message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCStart >> 24) & 0xFF, (hsc_DCStart >> 16) & 0xFF, (hsc_DCStart >> 8) & 0xFF, hsc_DCStart & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCStart, server_command_to_str(hsc_DCStart));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}


void send_volume_preset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, guint8 nozzle_num, guint32 price, guint32 volume)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare volume preset message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCSetVolumeDose >> 24) & 0xFF, (hsc_DCSetVolumeDose >> 16) & 0xFF, (hsc_DCSetVolumeDose >> 8) & 0xFF, hsc_DCSetVolumeDose & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCSetVolumeDose, server_command_to_str(hsc_DCSetVolumeDose));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guchar nozzle_num_buffer[] = {nozzle_num};
		tlv_add_unit(&units,tlv_create_unit(tst_NozzleNumber , nozzle_num_buffer, 0, sizeof(nozzle_num_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add NozzleNum: %d",
				params->client_index, device_index, nozzle_num);

		guchar price_buffer[] = {(price >> 24) & 0xFF, (price >> 16) & 0xFF, (price >> 8) & 0xFF, price & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_Price , price_buffer, 0, sizeof(price_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add Price: %d",
				params->client_index, device_index, price);

		guchar volume_buffer[] = {(volume >> 24) & 0xFF, (volume >> 16) & 0xFF, (volume >> 8) & 0xFF, volume & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_Quantity , volume_buffer, 0, sizeof(volume_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add Volume: %d",
				params->client_index, device_index, volume);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}

}

void send_amount_preset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, guint8 nozzle_num, guint32 price, guint32 amount)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare amount preset message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCSetSumDose >> 24) & 0xFF, (hsc_DCSetSumDose >> 16) & 0xFF, (hsc_DCSetSumDose >> 8) & 0xFF, hsc_DCSetSumDose & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCSetSumDose, server_command_to_str(hsc_DCSetSumDose));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		guchar nozzle_num_buffer[] = {nozzle_num};
		tlv_add_unit(&units,tlv_create_unit(tst_NozzleNumber , nozzle_num_buffer, 0, sizeof(nozzle_num_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add NozzleNum: %d",
				params->client_index, device_index, nozzle_num);

		guchar price_buffer[] = {(price >> 24) & 0xFF, (price >> 16) & 0xFF, (price >> 8) & 0xFF, price & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_Price , price_buffer, 0, sizeof(price_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add Price: %d",
				params->client_index, device_index, price);

		guchar amount_buffer[] = {(amount >> 24) & 0xFF, (amount >> 16) & 0xFF, (amount >> 8) & 0xFF, amount & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_Amount , amount_buffer, 0, sizeof(amount_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add Amount: %d",
				params->client_index, device_index, amount);

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}
}


void send_update_price_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, PricePacks* price_packs)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare update price message",
			params->client_index, device_index);


	gchar* guid = NULL;
	get_server_device_guid_by_index(device_index, &guid);

	if (guid!=NULL)
	{
		TlvUnit* units = NULL;

		tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)guid, 0, strlen(guid)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add GUID: %s",
				params->client_index, device_index, guid);

		guint32 message_id = get_client_device_message_id(params->client_index, device_index);

		guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageId: %d",
				params->client_index, device_index, message_id);

		guchar message_type_buffer[] = {mt_Request};
		tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add MessageType: %s",
				params->client_index, device_index, return_message_type_name(mt_Request));

		guchar command_code_buffer[] = {(hsc_DCUpdatePrices >> 24) & 0xFF, (hsc_DCUpdatePrices >> 16) & 0xFF, (hsc_DCUpdatePrices >> 8) & 0xFF, hsc_DCUpdatePrices & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_DCUpdatePrices, server_command_to_str(hsc_DCUpdatePrices));

		guchar device_number_buffer[] = {(dispencer_num >> 24) & 0xFF, (dispencer_num >> 16) & 0xFF, (dispencer_num >> 8) & 0xFF, dispencer_num & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_DeviceNumber , device_number_buffer, 0, sizeof(device_number_buffer)));
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add DeviceNumber: %d",
				params->client_index, device_index, dispencer_num);

		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add Nozzles data",
				params->client_index, device_index);

		if (price_packs->unit_count > 0)
		{
			for (guint8 i = 0; i < price_packs->unit_count; i++)
			{
				TlvUnit* price_pack_units = NULL;

				guchar nozzle_num_buffer[] = {price_packs->units[i].id};
				tlv_add_unit(&price_pack_units,tlv_create_unit(tppt_Id , nozzle_num_buffer, 0, sizeof(nozzle_num_buffer)));

				guchar price_buffer[] = {(price_packs->units[i].price >> 24) & 0xFF,
											(price_packs->units[i].price >> 16) & 0xFF,
											(price_packs->units[i].price >> 8) & 0xFF,
											price_packs->units[i].price & 0xFF};
				tlv_add_unit(&price_pack_units,tlv_create_unit(tppt_Price , price_buffer, 0, sizeof(price_buffer)));


				add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 		Nozzle %d price %d",
						params->client_index, device_index, price_packs->units[i].id, price_packs->units[i].price);

				guint32 price_pack_size = 0;
				guchar* price_pack_frame = tlv_serialize_units(price_pack_units, &price_pack_size);

				if (price_pack_frame !=NULL && price_pack_size > 0)
				{
					tlv_add_unit(&units,tlv_create_unit(tst_PricePack , price_pack_frame, 0, price_pack_size));
					g_free(price_pack_frame);
				}

				tlv_delete_units(&price_pack_units);

			}
		}

		guint32 size = 0;
		guchar* frame =  tlv_create_transport_frame(units, &size);

		if (frame!=NULL && size > 0)
		{
			socket_client_device_send_with_mutex(params, device_index, frame, size );
			free(frame);
		}

		tlv_delete_units(&units);

		g_free(guid);
	}
	else
	{
		add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}
}

void send_short_dc_message(PSSClientThreadFuncParam* params, guint8 device_index, MessageType message_type, ExchangeError exchange_error)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d prepare %s short message",
			params->client_index, device_index, return_message_type_name(message_type));

	TlvUnit* units = NULL;

	guchar message_type_buffer[] = {message_type};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add message type: %s",
			params->client_index, device_index, return_message_type_name(message_type));

	guchar error_buffer[] = {exchange_error};
	tlv_add_unit(&units,tlv_create_unit(tst_Error , error_buffer, 0, sizeof(error_buffer)));

	add_log(log_params, TRUE, TRUE,params->log_trace, params->log_enable, "client %d device %d 	add exchange error: %d",
			params->client_index, device_index, exchange_error);

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_client_device_send_with_mutex(params, device_index, frame, size );
		free(frame);
	}

	tlv_delete_units(&units);

}
ExchangeError ParseNozzleConfigurationFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 			Nozzle configuration:",	client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcnct_Number:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Number: %d",	client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Number parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcnct_Grade:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Grade: %d",	client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Grade parse error",	client_params->client_index, params->device_index);
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 				Undefined tag (%04X)",	client_params->client_index, params->device_index, unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}


ExchangeError ParseDispencerConfigurationFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 		Dispencer configuration:",	client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcdct_Number:
					if (unit->length == sizeof(guint32))
					{
						guint32 number = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Number: %d",	client_params->client_index, params->device_index, number);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Number parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcdct_Address:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Address: %d",	client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Address parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcdct_NozzleCount:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Nozzle count: %d",	client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Nozzle count parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcdct_Nozzle:
					result = ParseNozzleConfigurationFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						return result;
					}

					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 			Undefined tag (%04X)",	client_params->client_index, params->device_index, unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}

ExchangeError ParseDispencerExtendedFunctionFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 		Extended function configuration:",	client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdceft_Index:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Index: %d",	client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Index parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdceft_Name:
					if (unit->length > 0)
					{
						gchar* name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
									"client %d device %d 			Name: %s",	client_params->client_index, params->device_index, name);
							g_free(name);
						}
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 			Undefined tag (%04X)",	client_params->client_index, params->device_index,unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}

ExchangeError ParseDCConfigurationFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 	Configuration:",	client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcct_DispencerCount:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Dispencer count: %d",	client_params->client_index, params->device_index,	unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Dispencer count parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcct_DispencerConfiguration:
					result = ParseDispencerConfigurationFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcct_ExtendedFuncCount:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Extended function count: %d",	client_params->client_index, params->device_index,	unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Extended function count parse error",	client_params->client_index, params->device_index);
					}
					break;

				case tdcct_ExtendedFunction:
					result = ParseDispencerExtendedFunctionFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 		Undefined tag (%04X)",	client_params->client_index, params->device_index,unit->tag);
					break;

			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}
ExchangeError ParseNozzleDataFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port, Nozzle* nozzle)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 			Nozzle data:", client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);



	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcncdt_Number:
					if (unit->length == sizeof(guint8))
					{
						nozzle->num = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Nozzle number: %d", client_params->client_index, params->device_index, nozzle->num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Nozzle number parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcncdt_Counter:
					if (unit->length == sizeof(guint32))
					{
						nozzle->counter = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Counter: %d", client_params->client_index, params->device_index, nozzle->counter);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 				Counter parse error", client_params->client_index, params->device_index);
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 				Undefined tag (%04X)", client_params->client_index, params->device_index,unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}

ExchangeError ParseDispencerCountersFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port, Dispencer* dispencer)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 		Dispencer counters data:", client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcdcdt_DispencerNumber:
					if (unit->length == sizeof(guint32))
					{
						dispencer->num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Dispencer number: %d", client_params->client_index, params->device_index, dispencer->num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Dispencer number parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcdcdt_NozzleCount:
					if (unit->length == sizeof(guint8))
					{
						dispencer->nozzle_count = 0;
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Nozzle count: %d", client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Nozzle count parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcdcdt_NozzleCounterData:
					result = ParseNozzleDataFrame(unit->value, unit->length, params, port, &dispencer->nozzles[dispencer->nozzle_count]);

					if (result != ee_None)
					{
						return result;
					}
					else
					{
						dispencer->nozzle_count++;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 			Undefined tag (%04X)", client_params->client_index, params->device_index, unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}

ExchangeError ParseDispencerDataFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 		Dispencer data:", client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	Dispencer dispencer = {0x00};

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcddt_Number:
					if (unit->length == sizeof(guint32))
					{
						dispencer.num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Number: %d", client_params->client_index, params->device_index, dispencer.num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Number parse error", params->device_index, client_params->client_index);
					}
					break;

				case tdcddt_State:
					if (unit->length == sizeof(guint8))
					{
						dispencer.state = (DispencerState)unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			State: %d (%s)", client_params->client_index, params->device_index, dispencer.state, dispencer_state_to_str(dispencer.state) );
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			State parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_OrderType:
					if (unit->length == sizeof(guint8))
					{
						dispencer.order_type = (OrderType)unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Order type: %d (%s)", client_params->client_index, params->device_index, dispencer.order_type, order_type_to_str(dispencer.order_type) );
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Order type parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_PresetOrderType:
					if (unit->length == sizeof(guint8))
					{
						dispencer.preset_order_type = (OrderType)unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset order type: %d (%s)", client_params->client_index, params->device_index, dispencer.preset_order_type, order_type_to_str(dispencer.preset_order_type) );
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset order type parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_IsPay:
					if (unit->length == sizeof(guint8))
					{
						dispencer.is_pay = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Is pay: %d", client_params->client_index, params->device_index, dispencer.is_pay);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Is pay parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_PresetNozzleNum:
					if (unit->length == sizeof(guint8))
					{
						dispencer.preset_nozzle_num = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset nozzle num: %d", client_params->client_index, params->device_index, dispencer.preset_nozzle_num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset nozzle num parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_ActiveNozzleNum:
					if (unit->length == sizeof(guint8))
					{
						dispencer.active_nozzle_num = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Active nozzle num: %d", client_params->client_index, params->device_index, dispencer.active_nozzle_num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Active nozzle num parse error", client_params->client_index, params->device_index );
					}
					break;

				case tdcddt_PresetPrice:
					if (unit->length == sizeof(guint32))
					{
						dispencer.preset_price = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset price: %d", client_params->client_index, params->device_index, dispencer.preset_price);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset price parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_PresetVolume:
					if (unit->length == sizeof(guint32))
					{
						dispencer.preset_volume = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset volume: %d", client_params->client_index, params->device_index, dispencer.preset_volume);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset volume parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_PresetAmount:
					if (unit->length == sizeof(guint32))
					{
						dispencer.preset_amount = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset amount: %d", client_params->client_index, params->device_index, dispencer.preset_amount);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Preset amount parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_CurrentPrice:
					if (unit->length == sizeof(guint32))
					{
						dispencer.current_price = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current price: %d", client_params->client_index, params->device_index, dispencer.current_price);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current price parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_CurrentVolume:
					if (unit->length == sizeof(guint32))
					{
						dispencer.current_volume = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current volume: %d", client_params->client_index, params->device_index, dispencer.current_volume);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current volume parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_CurrentAmount:
					if (unit->length == sizeof(guint32))
					{
						dispencer.current_amount = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current amount: %d", client_params->client_index, params->device_index, dispencer.current_amount);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Current amount parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_ErrorCode:
					if (unit->length == sizeof(guint8))
					{
						dispencer.error =  unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Error: %d", client_params->client_index, params->device_index, dispencer.error);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 			Error parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdcddt_ErrorDescription:
					if (unit->length > 0)
					{
						gchar* error_description = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (error_description !=NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
									"client %d device %d 			Error description: %s", client_params->client_index, params->device_index, error_description);
							g_free(error_description);
						}
					}
					break;

				case tdcddt_Counters:
					result = ParseDispencerCountersFrame(unit->value, unit->length, params, port, &dispencer);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 			Undefined tag (%04X)", client_params->client_index, params->device_index, unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}

	if (result == ee_None)
	{
		//TODO update fuelling point and send APC2
		PSSFuellingPoint fuelling_point = {0x00};

		get_fuelling_point_by_id(dispencer.num, &fuelling_point);

		//update counters
		if (dispencer.nozzle_count > 0)
		{
			for (guint8 i = 0; i < dispencer.nozzle_count; i++)
			{
				if (dispencer.nozzles[i].num > 0 && fuelling_point.grade_option_count > 0)
				{
					for (guint8 j = 0; j < fuelling_point.grade_option_count; j++)
					{
						if (fuelling_point.grade_options[j].id == dispencer.nozzles[i].num)
						{
							fuelling_point.grade_options[j].volume_total = dispencer.nozzles[i].counter;
						}
					}
				}
			}
		}

		fuelling_point.volume_total = 0;

		for (guint8 j = 0; j < fuelling_point.grade_option_count; j++)
		{
				fuelling_point.volume_total+= fuelling_point.grade_options[j].volume_total;
		}

		switch (dispencer.state)
		{
			case ds_NotInitialize:
				fuelling_point.sub_state &= ~FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 &= ~FP_SS2_PUMP_TOTALS_READY;
				fuelling_point.sub_state &= ~FP_SS_IS_E_STOPPED;
				fuelling_point.main_state =fpms_Unconfigured;

				break;

			case ds_Busy:

				break;

			case ds_Free:
				if (dispencer.preset_order_type == ot_Free)
				{
					fuelling_point.main_state = fpms_Idle;
					fuelling_point.sub_state = FP_SS_IS_ONLINE;
					fuelling_point.fuelling_volume = 0;
					fuelling_point.fuelling_money = 0;
				}
				fuelling_point.sub_state |= FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 |= FP_SS2_PUMP_TOTALS_READY;
				fuelling_point.sub_state &= ~FP_SS_IS_E_STOPPED;

				break;

			case ds_NozzleOut:

				g_printf("\n111111111111 %d  1111111111111111111111\n", fuelling_point.main_state);


				if (fuelling_point.main_state == fpms_Calling && ((fuelling_point.sub_state & FP_SS_IS_PRESET) > 0))
				{
					fuelling_point.reset = FALSE;
					authorize_fp(fuelling_point, params->client_params, pssc_None, 0, FALSE);
					fuelling_point.main_state = fpms_PreAuthorized;
				}
				else if (fuelling_point.main_state < fpms_PreAuthorized)
				{
					g_printf("\n22222222222222222222222222222222222\n");

					fuelling_point.main_state = fpms_Calling;
				}
				fuelling_point.sub_state |= FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 |= FP_SS2_PUMP_TOTALS_READY;
				fuelling_point.sub_state &= ~FP_SS_IS_E_STOPPED;

				break;

			case ds_Filling:
				if ( fuelling_point.main_state != fpms_Fuelling && fuelling_point.main_state != fpms_FuellingPaused)
				{
					fuelling_point.main_state = fpms_Fuelling;
				}
				fuelling_point.sub_state |= FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 |= FP_SS2_PUMP_TOTALS_READY;
				fuelling_point.fuelling_volume = dispencer.current_volume;
				fuelling_point.fuelling_money = dispencer.current_amount;
				break;


			case ds_Stopped:
				fuelling_point.sub_state |= FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 |= FP_SS2_PUMP_TOTALS_READY;
				fuelling_point.fuelling_volume = dispencer.current_volume;
				fuelling_point.fuelling_money = dispencer.current_amount;
				fuelling_point.main_state = fpms_FuellingPaused;
				break;

			case ds_Finish:
				fuelling_point.fuelling_volume = dispencer.current_volume;
				fuelling_point.fuelling_money = dispencer.current_amount;
				if (fuelling_point.main_state == fpms_Fuelling)
				{
					PSSTransaction transaction = {0x00};

					transaction.flags = TBF_STORED_TRANS | TBF_TRAN_MIN_LIMIT | TBF_PREPAY_MODE_USED | TBF_NOT_USED_1 | TBF_NOT_USED_2;
					transaction.grade_id = fuelling_point.preset_data.grade_id;
					transaction.money = fuelling_point.fuelling_money;
					transaction.sm_id = fuelling_point.sm_id;
					transaction.trans_lock_id = 0;
					transaction.volume = fuelling_point.fuelling_volume;

					fuelling_point.sup_transactions.last_trans_seq_no++;
					if (fuelling_point.sup_transactions.last_trans_seq_no >= 10000) fuelling_point.sup_transactions.last_trans_seq_no = 0;
					transaction.trans_seq_no = fuelling_point.sup_transactions.last_trans_seq_no;
					fuelling_point.sup_transactions.units[fuelling_point.sup_transactions.count] = transaction;
					fuelling_point.sup_transactions.count++;

					fuelling_point.main_state = fpms_Unavailable;
				}
				if (fuelling_point.main_state == fpms_FuellingPaused || fuelling_point.main_state == fpms_Unconfigured || fuelling_point.main_state == fpms_Unavailable)
				{
					reset_fp(fuelling_point.id, params->client_params, pssc_None, 0, FALSE);
				}

				fuelling_point.sub_state |= FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 |= FP_SS2_PUMP_TOTALS_READY;
				break;

			case ds_ConnectionError:
				fuelling_point.sub_state &= ~FP_SS_IS_ONLINE;
				fuelling_point.sub_state2 &= ~FP_SS2_PUMP_TOTALS_READY;
				break;

		}

		guint8 grade_id = 0;

		for (guint8 i = 0; i < fuelling_point.grade_option_count; i++)
		{
				if (fuelling_point.grade_options[i].id == dispencer.active_nozzle_num)
				{
					grade_id = fuelling_point.grade_options[i].grade_id;
				}
		}

		fuelling_point.grade_id = grade_id;

		set_fuelling_point_by_id(dispencer.num, fuelling_point);


		if (port == SUPERVISED_MESSAGES_PORT)
		{
			send_fp_status_req_3(dispencer.num, params->client_params, FALSE, port);
		}

		save_pss_data();

	}

	return result;
}

ExchangeError ParseDCDataFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 	Dispencers data:", client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcdt_Count:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Count: %d", client_params->client_index, params->device_index, unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Count parse error", client_params->client_index, params->device_index, unit->value[0]);
					}
					break;

				case tdcdt_DispencerData:
					{
						result = ParseDispencerDataFrame(unit->value, unit->length, params, port);

						if (result != ee_None)
						{
							return result;
						}
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 		Undefined tag (%04X)", client_params->client_index, params->device_index,unit->tag);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}



ExchangeError ParseDCCountersFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
			"client %d device %d 	Counters:", client_params->client_index, params->device_index);

	TlvUnit* units = NULL;
	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdccnt_Count:
					if (unit->length == sizeof(guint8))
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Dispencer count: %d", client_params->client_index, params->device_index,unit->value[0]);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 		Dispencer count parse error", client_params->client_index, params->device_index);
					}
					break;

				case tdccnt_DispencerCountersData:
					{
						Dispencer dispencer = {0x00};

						result = ParseDispencerCountersFrame(unit->value, unit->length, params, port, &dispencer);

						if (result != ee_None)
						{
							return result;
						}
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 		Undefined tag (%04X)", client_params->client_index, params->device_index);
					break;
			}
			unit = unit->next;
		} while(unit != NULL);
	}
	return result;
}


ExchangeError ParseDCFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
{
	PSSClientThreadFuncParam* client_params = params->client_params;
	LogParams* log_params = client_params->log_params;

	add_log(log_params, TRUE, FALSE, client_params->log_trace, client_params->log_enable, "client %d device %d <<", client_params->client_index, params->device_index);

	for (guint16 i = 0; i < buffer_length; i++)
	{
		add_log(log_params, FALSE, FALSE, client_params->log_trace, client_params->log_enable, " %02X", buffer[i]);
	}
	add_log(log_params, FALSE, TRUE, client_params->log_trace, client_params->log_enable, "");



	add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable, "client %d device %d parse frame:",
			client_params->client_index, params->device_index);

	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	guint32 message_id = 0;
	MessageType message_type = mt_Undefined;
	guint8 error = 0;
	guint8 device_status = 0;
	gchar* device_status_description = NULL;

	guint8 device_error = 0;
	gchar* device_error_description = NULL;
	guint8 device_reply_code = 0;
	guint32 command = 0;
	guint32 device_num = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tst_MessageId:
					if (unit->length == sizeof(guint32))
					{
						message_id = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	MessageId: %d",	client_params->client_index, params->device_index, message_id);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	MessageId parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceNumber:
					if (unit->length == sizeof(guint32))
					{
						device_num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device number: %d",	client_params->client_index, params->device_index, device_num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device number parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}

					break;

				case tst_MessageType:
					if (unit->length == sizeof(guint8))
					{
						message_type = (MessageType)unit->value[0];
						gchar* message_type_description = return_message_type_name(message_type);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Message type: %s",	client_params->client_index, params->device_index, message_type_description);

						if (message_type_description != NULL)
						{
							g_free(message_type_description);
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Message type parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_Error:
					if (unit->length == sizeof(guint8))
					{
						error = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Error: %d",	client_params->client_index, params->device_index, error);
					}
					else
					{
						add_log(log_params, FALSE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Error parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceStatus:
					if (unit->length == sizeof(guint8))
					{
						device_status =  unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device status: %d",	client_params->client_index, params->device_index, device_status);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device status parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceStatusDescription:
					if (unit->length > 0)
					{
						if (device_status_description !=NULL)
						{
							g_free(device_status_description);
							device_status_description = NULL;
						}
						device_status_description = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (device_status_description !=NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
									"client %d device %d 	Device status description: %s",	client_params->client_index, params->device_index, device_status_description);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device status description parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceError:
					if (unit->length == sizeof(guint8))
					{
						device_error = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device error: %d",	client_params->client_index, params->device_index, device_error);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device error parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceErrorDescription:
					if (unit->length > 0)
					{
						if (device_error_description !=NULL)
						{
							g_free(device_error_description);
							device_error_description = NULL;
						}
						device_error_description = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-16", " ", NULL, NULL, NULL);
						if (device_error_description !=NULL)
						{
							add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
									"client %d device %d 	Device error description: %s",	client_params->client_index, params->device_index, device_error_description);
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device error description parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DeviceReplyCode:
					if (unit->length == sizeof(guint8))
					{
						device_reply_code = unit->value[0];
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device reply code: %d",	client_params->client_index, params->device_index, device_reply_code);
					}
					else
					{
						add_log(log_params, FALSE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Device reply code parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;


				case tst_CommandCode:
					if (unit->length == sizeof(guint32))
					{
						command = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Command code: %08X (%s)",	client_params->client_index, params->device_index, command, server_command_to_str(command));
					}
					else
					{
						add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
								"client %d device %d 	Command code parse error",	client_params->client_index, params->device_index);
						result = ee_Parse;
					}
					break;

				case tst_DCDeviceConfiguration:
					result = ParseDCConfigurationFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						if (device_status_description !=NULL)
						{
							g_free(device_status_description);
							device_status_description = NULL;
						}
						if (device_error_description !=NULL)
						{
							g_free(device_error_description);
							device_error_description = NULL;
						}


						return result;
					}

					break;

				case tst_DCData:
					result = ParseDCDataFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						if (device_status_description !=NULL)
						{
							g_free(device_status_description);
							device_status_description = NULL;
						}
						if (device_error_description !=NULL)
						{
							g_free(device_error_description);
							device_error_description = NULL;
						}


						return result;
					}

					break;

				case tst_DCCounters:
					result = ParseDCCountersFrame(unit->value, unit->length, params, port);

					if (result != ee_None)
					{
						if (device_status_description !=NULL)
						{
							g_free(device_status_description);
							device_status_description = NULL;
						}
						if (device_error_description !=NULL)
						{
							g_free(device_error_description);
							device_error_description = NULL;
						}

						return result;
					}

					break;

				default:
					add_log(log_params, TRUE, TRUE, client_params->log_trace, client_params->log_enable,
							"client %d device %d 	Undefined tag (%04X)!",	client_params->client_index, params->device_index,unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (message_type == mt_Reply || message_type == mt_Nak || message_type == mt_Ack || message_type == mt_Eot)
	{
		increment_client_device_message_id(client_params->client_index, params->device_index);
	}
	if (message_type == mt_Reply)
	{
		if (command == get_current_command(client_params->client_index, params->device_index))
		{
			PSSCommand pss_command = get_pss_command(client_params->client_index, params->device_index);
			guint8 subcode = get_pss_command_subcode(client_params->client_index, params->device_index);
			gboolean ex_mode = get_pss_ex_mode(client_params->client_index, params->device_index);

			if (device_reply_code == dce_NoError)
			{
				switch (command)
				{
					case hsc_DCGetData:
					case hsc_DCGetCounters:
						break;
					case hsc_DCSetVolumeDose:
					case hsc_DCSetSumDose:
					case hsc_DCSetUnlimDose:
						if (pss_command == pssc_authorize_Fp)
						{
							send_authorize_fp_reply(subcode, client_params, ex_mode, device_num);
						}
						break;

					case hsc_DCStart:
					case hsc_DCStop:
					case hsc_DCPayment:

						break;

					case hsc_DCReset:
						if (pss_command == pssc_clr_FpSupTrans)
						{
							send_clr_fp_sup_trans_reply(subcode, client_params, ex_mode, device_num);
						}
						if (pss_command == pssc_reset_Fp)
						{
							send_reset_fp_reply(subcode, client_params, ex_mode, device_num);
						}

						break;

					case hsc_DCExecuteExtendedFunction:
					case hsc_DCUpdatePrices:
					case hsc_DCSuspend:
					case hsc_DCResume:

						break;
				}
			}
			else
			{
				send_rejected_reply(client_params, ex_mode, pss_command, subcode);
			}
			set_current_command(client_params->client_index, params->device_index, hsc_None);
			set_pss_command(client_params->client_index, params->device_index, pssc_None);
			set_pss_command_subcode(client_params->client_index, params->device_index, 0);
			set_current_command_sent_time(client_params->client_index, params->device_index, 0);
			set_pss_ex_mode(client_params->client_index, params->device_index, FALSE);

		}


		if (((command == hsc_DCSetVolumeDose) || (command == hsc_DCSetSumDose)) && (device_reply_code == dce_NoError))
		{
			if (device_num > 0)
			{
				if (get_fp_main_state_by_id(device_num) == fpms_PreAuthorized)
				{
					set_fp_main_state_by_id(device_num, fpms_Starting);

					send_start_message(params->client_params, params->device_index, device_num);

				}
			}
		}


		//TODO

	}




	if (device_status_description !=NULL)
	{
		g_free(device_status_description);
		device_status_description = NULL;
	}
	if (device_error_description !=NULL)
	{
		g_free(device_error_description);
		device_error_description = NULL;
	}

	return result;

}


gpointer dc_device_func(gpointer data)
{
	PSSClientDeviceParam params = *(PSSClientDeviceParam*)data;

	unlock_threads_mutex();

	guint32 port = get_client_port(params.client_params->client_index);

	guchar bufrd[EXCHANGE_BUFFER_SIZE];
	guint16 pos_bufrd_tmp = 0;
	guint16 pos_bufrd = 0;


	guchar bufr[SOCKET_BUFFER_SIZE];

	gboolean bad_frame = FALSE;
	guint8 bad_frame_count = 0;
	gboolean finish_message = FALSE;
	guint16 crc = 0;
	guint16 calc_crc = 0;
	guint32 data_len = 0;

	g_printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

	g_printf("device_index = %d\n", params.device_index);

	g_printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

	ExchangeState es = es_WaitStartMessage;

	gint32 sock = get_client_device_sock(params.client_params->client_index, params.device_index);

	guint64 connect_time = get_date_time();

	//TODO
	if (sock > 0)
	{
		send_dc_get_status_message(params.client_params,  params.device_index);
	}

	while(get_client_device_thread_status(params.client_params->client_index, params.device_index) < ts_Destroing)
	{
		if (get_current_command(params.client_params->client_index, params.device_index) != hsc_None)
		{
			if (get_date_time() > get_current_command_sent_time(params.client_params->client_index, params.device_index) + EXEC_COMMAND_TIMEOUT)
			{
				send_rejected_reply(params.client_params,
									get_pss_ex_mode(params.client_params->client_index, params.device_index),
									get_pss_command(params.client_params->client_index, params.device_index),
									get_pss_command_subcode(params.client_params->client_index, params.device_index));

				set_current_command(params.client_params->client_index, params.device_index, hsc_None);
				set_pss_command(params.client_params->client_index, params.device_index, pssc_None);
				set_pss_command_subcode(params.client_params->client_index, params.device_index, 0);
				set_current_command_sent_time(params.client_params->client_index, params.device_index, 0);
				set_pss_ex_mode(params.client_params->client_index, params.device_index, FALSE);

			}
		}

		int bytes_read = recv(sock, bufr, 1024, 0);

		if(bytes_read <= 0)
		{
			if (get_client_state(params.client_params->client_index) == cs_Active)
			{
				if (get_date_time() > connect_time + RECONNECT_TIMEOUT)
				{

					sock = reconnect_to_server_device(params.client_params, params.device_index);
					connect_time = get_date_time();
					if (sock > 0)
					{
						send_dc_get_status_message(params.client_params,  params.device_index);
					}
				}
			}
			else
			{
				break;
			}
		}
		else
		{
			for (guint32 i = 0; i < bytes_read; i++)
			{
				switch(es)
				{
					case es_None:
						break;

					case es_WaitStartMessage:
						if (bufr[i] == ctrl_STX)
						{
							memset(bufrd, 0x00, EXCHANGE_BUFFER_SIZE);
							pos_bufrd_tmp = 0;
							pos_bufrd = 0;
							bad_frame = FALSE;
							bad_frame_count = 0;
							finish_message = FALSE;
							crc = 0;
							calc_crc = 0;
							es = es_ReadLength1;
						}
						break;

					case es_WaitSTX:
						if (bufr[i] == ctrl_STX)
						{

							bad_frame = FALSE;
							crc = 0;
							calc_crc = 0;
							es = es_ReadLength1;
						}
						break;

					case es_ReadLength1:
						data_len = bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength2;
						break;

					case es_ReadLength2:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength3;
						break;

					case es_ReadLength3:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						es = es_ReadLength4;
						break;

					case es_ReadLength4:
						data_len = (data_len << 8) | bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);

						if (data_len > 0)
						{
							es = es_ReadData;
						}
						else
						{
							es = es_WaitEndMessage;
						}
						break;

					case es_ReadData:
						bufrd[pos_bufrd_tmp++] = bufr[i];
						calc_crc = calc_crc_next(bufr[i], calc_crc);
						data_len--;
						if (data_len == 0)
						{
							es = es_WaitEndMessage;
						}
						break;

					case es_WaitEndMessage:
						calc_crc = calc_crc_next(bufr[i], calc_crc);

						switch (bufr[i])
						{
							case ctrl_ETX:
								finish_message = TRUE;
								break;

							case ctrl_ETB:
								finish_message = FALSE;
								break;

							default:
								bad_frame = TRUE;

						}
						es = es_WaitCRC1;
						break;

					case es_WaitCRC1:
						crc = bufr[i];
						es = es_WaitCRC2;
						break;

					case es_WaitCRC2:
						crc = (crc << 8) | bufr[i];

						if (crc == calc_crc && !bad_frame)
						{
							pos_bufrd = pos_bufrd_tmp;

							if (finish_message)
							{
								ExchangeError parse_result = ParseDCFrame(bufrd, pos_bufrd, &params, port);

								if (parse_result != ee_None)
								{
									send_short_dc_message(params.client_params, params.device_index, mt_Nak, parse_result);
								}
								es = es_WaitStartMessage;
							}
							else
							{
								send_short_dc_message(params.client_params, params.device_index, mt_Ack, ee_None);
							}
						}
						else
						{
							pos_bufrd_tmp = pos_bufrd;
							bad_frame_count++;

							if (bad_frame_count >= MAX_BAD_FRAME)
							{
								send_short_dc_message(params.client_params, params.device_index, mt_Eot, ee_LimitBadFrame);
								es = es_WaitStartMessage;
							}
							else
							{
								if (bad_frame)
								{
									send_short_dc_message(params.client_params, params.device_index, mt_Nak, ee_Format);
								}
								else
								{
									send_short_dc_message(params.client_params, params.device_index, mt_Nak, ee_Crc);
								}
								es = es_WaitSTX;
							}
						}
						break;
				}
			}
		}
	}

	set_client_device_thread_status(params.client_params->client_index, params.device_index, ts_Destroyed);

	return NULL;
}
