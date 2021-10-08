#include <glib.h>
#include <glib/gstdio.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_data.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_parse.h"
#include "pss_func.h"

void send_ppc_get_status_message(PSSClientThreadFuncParam* params, guint8 device_index)
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

void send_prices_message(PSSClientThreadFuncParam* params, guint8 device_index, PricePacks* price_packs)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d prepare update price message",
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

		guchar command_code_buffer[] = {(hsc_PPSetPrices >> 24) & 0xFF, (hsc_PPSetPrices >> 16) & 0xFF, (hsc_PPSetPrices >> 8) & 0xFF, hsc_PPSetPrices & 0xFF};
		tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add CommandCode: %08X (%s)",
				params->client_index, device_index, hsc_PPSetPrices, server_command_to_str(hsc_PPSetPrices));

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add Prices data",
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


				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 		Grade %d price %d",
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
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d error get GUID device",
				params->client_index, device_index);
	}
}

void send_short_ppc_message(PSSClientThreadFuncParam* params, guint8 device_index, MessageType message_type, ExchangeError exchange_error)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d prepare %s short message",
			params->client_index, device_index, return_message_type_name(message_type));

	TlvUnit* units = NULL;

	guchar message_type_buffer[] = {message_type};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add message type: %s",
			params->client_index, device_index, return_message_type_name(message_type));

	guchar error_buffer[] = {exchange_error};
	tlv_add_unit(&units,tlv_create_unit(tst_Error , error_buffer, 0, sizeof(error_buffer)));

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d device %d 	add exchange error: %d",
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

ExchangeError ParsePPCFrame(guchar* buffer, guint16 buffer_length, PSSClientDeviceParam* params, guint32 port)
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


gpointer ppc_device_func(gpointer data)
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
		send_ppc_get_status_message(params.client_params,  params.device_index);
	}

	while(get_client_device_thread_status(params.client_params->client_index, params.device_index) < ts_Destroing)
	{

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
						send_ppc_get_status_message(params.client_params,  params.device_index);
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
								ExchangeError parse_result = ParsePPCFrame(bufrd, pos_bufrd, &params, port);

								if (parse_result != ee_None)
								{
									send_short_ppc_message(params.client_params, params.device_index, mt_Nak, parse_result);
								}
								es = es_WaitStartMessage;
							}
							else
							{
								send_short_ppc_message(params.client_params, params.device_index, mt_Ack, ee_None);
							}
						}
						else
						{
							pos_bufrd_tmp = pos_bufrd;
							bad_frame_count++;

							if (bad_frame_count >= MAX_BAD_FRAME)
							{
								send_short_ppc_message(params.client_params, params.device_index, mt_Eot, ee_LimitBadFrame);
								es = es_WaitStartMessage;
							}
							else
							{
								if (bad_frame)
								{
									send_short_ppc_message(params.client_params, params.device_index, mt_Nak, ee_Format);
								}
								else
								{
									send_short_ppc_message(params.client_params, params.device_index, mt_Nak, ee_Crc);
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

