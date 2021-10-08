#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

#include "logger.h"
#include "configuration.h"
#include "tlv.h"
#include "system_func.h"
#include "socket_func.h"
#include "dc_device.h"
#include "dc_device_data.h"
#include "dc_device_client_data.h"
#include "dc_device_driver_func.h"
#include "dc_device_message_func.h"

void close_dc_sock(guint8 index)
{
	if (get_dc_sock(index) >= 0)
	{

		gint32 sock = get_dc_sock(index);

		shutdown(sock, SHUT_RDWR);

		set_dc_sock_status(index, ss_Disconnected);

		set_dc_clients_status_is_destroying();

		while(TRUE)
		{
			gboolean exit_ready = TRUE;

			for (guint8 i = 0; i < MAX_DEVICE_CLIENT_COUNT; i++)
			{
				if ( get_client_dci(i) == index && get_dc_client_state(i) != cls_Free)
				{
					exit_ready = FALSE;
				}
			}

			if (exit_ready)
			{
				break;
			}
		}
	}
}

ExchangeError ParseNozzlePricePack(guchar* buffer, guint16 buffer_length, guint8* nozzle_num, guint32* price, DCClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d 	Nozzle pack: ", client_info->socket);


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tppt_Id:
					if (unit->length == sizeof(guint8))
					{
						*nozzle_num = (guint8)(unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 		Tag %04x (length = %d) Nozzle num = %0d", client_info->socket, unit->tag, unit->length, *nozzle_num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 		Parse Error!", client_info->socket);
						return ee_Parse;
					}

					break;

				case tppt_Price:
					if (unit->length == sizeof(guint32))
					{
						*price = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 		Tag %04x (length = %d) Price = %0d", client_info->socket, unit->tag, unit->length, *price);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 		Parse Error!", client_info->socket);
						return ee_Parse;
					}

					break;


			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}


ExchangeError ParseDCRequest(guchar* buffer, guint16 buffer_length, guchar* guid, guint32* message_id, MessageType* message_type, HardwareServerCommand* command,
			guint32* disp_num, guint8* nozzle_num, guint32* price, guint32* volume, guint32* amount, guint8* ext_func_index, DCPricePacks* price_packs,
			DCClientInfo* client_info)
{
	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
			"%d Parsing frame (result = %d): ", client_info->socket, result);

	price_packs->nozzle_price_pack_count = 0;


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tst_GuidClient:
					if (unit->length > 0 && unit->length < GUID_LENGTH)
					{
						memcpy(guid, unit->value, unit->length);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Guid = %s", client_info->socket, unit->tag, unit->length, guid);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageId:
					if (unit->length == sizeof(guint32))
					{
						*message_id = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) MessageId = %d", client_info->socket, unit->tag, unit->length, *message_id);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_CommandCode:
					if (unit->length == sizeof(guint32))
					{
						*command = (HardwareServerCommand)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) CommandCode = %08X (%s)", client_info->socket, unit->tag, unit->length, *command, server_command_to_str(*command) );
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_MessageType:
					if (unit->length == sizeof(guint8))
					{
						*message_type = (MessageType)unit->value[0];
						gchar* message_type_description = return_message_type_name((MessageType)unit->value[0]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Message type = %s", client_info->socket, unit->tag, unit->length, message_type_description);
						if (message_type_description != NULL)
						{
							g_free(message_type_description);
						}
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_DeviceNumber:
					if (unit->length == sizeof(guint32))
					{
						*disp_num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Device number = %0d", client_info->socket, unit->tag, unit->length, *disp_num);
					}
					break;

				case tst_NozzleNumber:
					if (unit->length == sizeof(guint8))
					{
						*nozzle_num = unit->value[0];
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Nozzle number = %0d", client_info->socket, unit->tag, unit->length, *nozzle_num);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_ExtendedFuncIndex:
					if (unit->length == sizeof(guint8))
					{
						*ext_func_index = unit->value[0];
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Extended func index = %0d", client_info->socket, unit->tag, unit->length, *ext_func_index);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_Price:
					if (unit->length == sizeof(guint32))
					{
						*price = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Price = %d\n", client_info->socket, unit->tag, unit->length,*price);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_Quantity:
					if (unit->length == sizeof(guint32))
					{
						*volume = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Volume = %d\n",client_info->socket, unit->tag, unit->length, *volume);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_Amount:
					if (unit->length == sizeof(guint32))
					{
						*amount = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d 	Tag %04x (length = %d) Amount = %d\n",client_info->socket, unit->tag, unit->length, *amount);
					}
					else
					{
						add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing,
								"%d Parse Error!", client_info->socket);
						return ee_Parse;
					}
					break;

				case tst_PricePack:
					{
						guint8 nozzle_num = 0;
						guint32 price = 0;

						result = ParseNozzlePricePack(unit->value, unit->length, &nozzle_num, &price, client_info);

						if (result != ee_None)
						{
							return result;
						}
						else
						{
							DCNozzlePricePack* price_pack = &price_packs->nozzle_price_packs[price_packs->nozzle_price_pack_count];
							price_pack->nozzle_num = nozzle_num;
							price_pack->price = price;
							price_packs->nozzle_price_pack_count++;
						}
					}
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	return result;

}

void get_dc_client_configuration(ProfilesConf* configuration, guchar* guid, gboolean* is_present, guint8* access_level )
{
	if (configuration->enable == FALSE)
	{
		*is_present = TRUE;
		*access_level = cal_Unlim;

	}
	else
	{
		*is_present = FALSE;
		*access_level = cal_Disable;
		if ( configuration->profiles_count > 0 )
		{
			for (guint8 i = 0; i < configuration->profiles_count; i++)
			{
	        	if (strcmp((gchar*)guid,configuration->profiles[i].guid) == 0)
	        	{
	        		*is_present = TRUE;
	        		*access_level = configuration->profiles[i].access_level;
	        		break;
	        	}
			}
		}
	}
}

gpointer dc_device_client_read_thread_func(gpointer data)
{
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

	DCClientInfo client_info = *(DCClientInfo*)data;

	SockClientInfo socket_client_info = {.device_name = g_strdup(client_info.device_name),
										.ip_address = g_strdup(client_info.ip_address),
										.log_params = client_info.log_params,
										.log_frames = client_info.log_frames,
										.log_parsing = client_info.log_parsing,
										.log_trace = client_info.log_trace,
										.socket = client_info.socket,
										.socket_mutex = &client_info.socket_mutex};

	ProfilesConf profiles = {0x00};

	get_profiles_conf(&profiles);

	ExchangeState es = es_WaitStartMessage;

	gchar* log_prefix = g_strdup_printf("%s_connect", client_info.device_name);

	while(TRUE)
	{

		int bytes_read = recv(client_info.socket, bufr, 8192, 0);
		if(bytes_read <= 0)
		{
			break;
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
								if (client_info.log_frames)
								{
									add_log(client_info.log_params, TRUE, FALSE, client_info.log_trace, client_info.log_frames, "%d << ", client_info.socket);

									for (guint16 i = 0; i < pos_bufrd; i++)
									{
										add_log(client_info.log_params, FALSE, FALSE, client_info.log_trace, client_info.log_frames, " %02X", bufrd[i]);

									}
									add_log(client_info.log_params, FALSE, TRUE, client_info.log_trace, client_info.log_frames, "");
								}

								guchar guid[GUID_LENGTH] = {0x00};
								guint32 message_id = 0;
								MessageType message_type = mt_Undefined;
								HardwareServerCommand command = hsc_None;
								guint32 disp_num = 0;
								guint8 nozzle_num = 0;
								guint32 price = 0;
								guint32 volume = 0;
								guint32 amount = 0;
								guint8 ext_func_index;
								DCPricePacks price_packs = {0x00};

								ExchangeError parse_result = ParseDCRequest(bufrd, pos_bufrd, guid, &message_id, &message_type, &command, &disp_num, &nozzle_num, &price, &volume, &amount,
										&ext_func_index, &price_packs, &client_info);

								add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, client_info.log_parsing, "%d Parse request result: %d", client_info.socket, parse_result);


								if (parse_result == ee_None  && message_type == mt_Request)
								{

									gboolean client_is_present = FALSE;

									guint8 client_access_level = get_dc_client_access_level_param(client_info.client_index);

									if (strlen((gchar*)guid) > 0)
									{
										get_dc_client_configuration(&profiles, guid, &client_is_present, &client_access_level);

										set_dc_client_access_level_param(client_info.client_index, client_access_level);

										if (client_is_present)
										{
											set_dc_client_logged(client_info.client_index, TRUE);
										}
									}
									else
									{
										if (get_dc_client_logged(client_info.client_index))
										{
											client_is_present = TRUE;
										}
									}


									if (client_is_present)
									{
										set_dc_client_logged(client_info.client_index, TRUE);

										switch(command)
										{
											case hsc_GetDeviceStatus:
												send_dc_device_status_message(client_info.client_index, client_info.dispencer_controller_index,&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_GetConfiguration:
												send_dc_device_configuration_message(client_info.client_index, client_info.dispencer_controller_index,&socket_client_info, message_id, mt_Reply);
												break;

											case hsc_DCGetData:
												{
													guint8 disp_index = 0;
													if (disp_num == 0 || dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
													{
														send_dc_device_data_message(client_info.client_index, client_info.dispencer_controller_index,&socket_client_info, message_id, mt_Reply, disp_num, disp_index);
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												break;

											case hsc_DCGetCounters:
												{

													guint8 disp_index = 0;
													if (disp_num == 0 || dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
													{
														guint8 nozzle_index = 0;
														if (dc_nozzle_num_is_present(client_info.dispencer_controller_index, disp_num, nozzle_num, &disp_index, &nozzle_index))
														{
															send_dc_device_counters_message(client_info.client_index, client_info.dispencer_controller_index,&socket_client_info, message_id, mt_Reply, disp_num, nozzle_num, disp_index, nozzle_index);
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												break;

											case hsc_DCSetVolumeDose:
												if (client_access_level )
												{
													if (price > 0 && disp_num > 0 && nozzle_num > 0 && volume > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id, nozzle_num, price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, client_info.log_parsing, "%d set dc command result: %d", client_info.socket, set_result);


																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCSetSumDose:
												if (client_access_level > cal_ReadOnly)
												{
													if (price > 0 && disp_num > 0 && nozzle_num > 0 && amount > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id, nozzle_num,price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCSetUnlimDose:
												if (client_access_level > cal_ReadOnly)
												{
													if (price > 0 && disp_num > 0 && nozzle_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCStart:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price, volume , amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCStop:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														ExchangeError set_result = set_dc_client_command(client_info.client_index, 0, command,
																message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

														if (set_result!=ee_None)
														{
															send_short_message(&socket_client_info, mt_Nak, set_result);
														}
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCSuspend:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														ExchangeError set_result = set_dc_client_command(client_info.client_index, 0, command,
																message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

														if (set_result!=ee_None)
														{
															send_short_message(&socket_client_info, mt_Nak, set_result);
														}
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;


											case hsc_DCResume:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														ExchangeError set_result = set_dc_client_command(client_info.client_index, 0, command,
																message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

														if (set_result!=ee_None)
														{
															send_short_message(&socket_client_info, mt_Nak, set_result);
														}
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;


											case hsc_DCPayment:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price , volume , amount , ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCReset:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{

															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCPriceUpdate:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}
												break;

											case hsc_DCExecuteExtendedFunction:
												if (client_access_level > cal_ReadOnly)
												{
													if (disp_num > 0)
													{
														guint8 disp_index = 0;
														if (dc_dispencer_num_is_present(client_info.dispencer_controller_index, disp_num, &disp_index))
														{
															ExchangeError set_result = set_dc_client_command(client_info.client_index, disp_index, command,
																	message_id,nozzle_num, price, volume, amount, ext_func_index, price_packs);

															if (set_result!=ee_None)
															{
																send_short_message(&socket_client_info, mt_Nak, set_result);
															}
														}
														else
														{
															send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
														}
													}
													else
													{
														send_short_message(&socket_client_info, mt_Nak, ee_FaultParam);
													}
												}
												else
												{
													send_short_message(&socket_client_info, mt_Nak, ee_AccessDenied);
												}

												break;

											default:
												send_short_message(&socket_client_info, mt_Nak, ee_UndefineCommandCode);
												break;

										}
									}
								}
								else
								{
									send_short_message(&socket_client_info, mt_Nak, parse_result);
								}
								es = es_WaitStartMessage;
							}
							else
							{
								send_short_message(&socket_client_info, mt_Ack, ee_None);
							}
						}
						else
						{
							pos_bufrd_tmp = pos_bufrd;
							bad_frame_count++;

							if (bad_frame_count >= MAX_BAD_FRAME)
							{
								send_short_message(&socket_client_info, mt_Eot, ee_LimitBadFrame);
								es = es_WaitStartMessage;
							}
							else
							{
								if (bad_frame)
								{
									send_short_message(&socket_client_info, mt_Nak, ee_Format);
								}
								else
								{
									send_short_message(&socket_client_info, mt_Nak, ee_Crc);
								}
								es = es_WaitSTX;
							}
						}
						break;
				}
			}
		}
	}

	set_dc_client_read_thread_is_active(client_info.client_index, FALSE);

	g_free(log_prefix);

	return NULL;
}


gpointer dc_device_client_thread_func(gpointer data)
{
	DCClientInfo client_info = *(DCClientInfo*)data;

	g_mutex_init(&client_info.socket_mutex);

	SockClientInfo socket_client_info = {.device_name = g_strdup(client_info.device_name),
										.ip_address = g_strdup(client_info.ip_address),
										.log_params = client_info.log_params,
										.log_frames = client_info.log_frames,
										.log_parsing = client_info.log_parsing,
										.log_trace = client_info.log_trace,
										.socket = client_info.socket,
										.socket_mutex = &client_info.socket_mutex};

	gchar* log_prefix = g_strdup_printf("%s_connect",client_info.device_name);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d starting main client thread", client_info.socket );

	GThread* client_read_thread = g_thread_new("dc_client_read_thread", dc_device_client_read_thread_func, &client_info);

	if (client_read_thread == NULL)
	{
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d error starting client read thread", client_info.socket);
	}
	else
	{
		client_read_thread->priority = G_THREAD_PRIORITY_LOW;
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d set read thread priority is low", client_info.socket );
		set_dc_client_read_thread_is_active(client_info.client_index, TRUE);
		add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d set read thread is active", client_info.socket );

	}

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE,  "%d main loop", client_info.socket );

	while(get_dc_client_read_thread_is_active(client_info.client_index))
	{
		if (get_dc_device_status_is_changed_for_client(client_info.client_index) && get_dc_client_logged(client_info.client_index))
		{
			send_dc_device_status_message(client_info.client_index,client_info.dispencer_controller_index,&socket_client_info, 0, mt_Event);

			set_dc_device_status_is_changed_for_client(client_info.client_index, FALSE);
		}

		guint8 disp_count = get_client_dispencer_info_count( client_info.client_index );

		if (disp_count > 0)
		{
			for (guint i = 0; i < disp_count; i++)
			{
				guint32 disp_num = get_client_dispencer_num( client_info.client_index, i);

				if(get_client_dispencer_command_state( client_info.client_index, i) == cs_Complete)
				{
					guint32 message_id = 0;
					DcDeviceError device_error = dce_Undefined;
					HardwareServerCommand command = hsc_None;

					get_dc_client_command_result(client_info.client_index, i, &message_id, &command, &device_error);
					if (clear_dc_client_command(client_info.client_index, i))
					{
						add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d dispencer %d comand complete. Cleared.", client_info.socket, disp_num);

					}
					send_dc_device_command_result_message(client_info.client_index,client_info.dispencer_controller_index,&socket_client_info, message_id, command, device_error, device_error == dce_NoError ?  ee_None : ee_DeviceDriverError, disp_num);
				}

				if (get_client_dispencer_status_is_changed( client_info.client_index, i) && get_dc_client_logged(client_info.client_index))
				{
					send_dc_device_data_message(client_info.client_index,client_info.dispencer_controller_index,&socket_client_info, 0, mt_Event, disp_num, i);
					set_client_dispencer_status_is_changed( client_info.client_index, i, FALSE);
				}

			}
		}
	}

	g_thread_join(client_read_thread);

	destroy_dc_client(client_info.client_index);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d destroy dc client", client_info.socket);

	close(client_info.socket);

	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d close socket", client_info.socket);

	set_dc_client_sock(client_info.client_index, -1);


	add_log(client_info.log_params, TRUE, TRUE, client_info.log_trace, TRUE, "%d thread ended", client_info.socket);

	g_free(log_prefix);

	return NULL;
}

void connect_dc_socket(guint8 index, LogParams* log_params, gboolean log_trace, guint32 port)
{
    gint32 old_sock = get_dc_sock(index);

	gint32 sock = create_serv_sock(old_sock, port);

	set_dc_sock(index, sock);

    if(sock < 0)
    {
    	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "DC socket create error");
    	set_dc_sock_status(index, ss_ConnectReady);

    }
    else
    {
    	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "DC socket create");
    	set_dc_sock_status(index, ss_Connected);
    }

}

gpointer dc_main_socket_thread_func(gpointer data)
{

    guint8 dci = *(guint8*)data;

	gchar* device_name = get_dc_device_name(dci);
	guint32 device_port = get_dc_device_port(dci);

	LogParams log_params = {0x00};

	CommonConf common_conf = {0x00};
	get_common_conf(&common_conf);

	gchar* log_prefix = g_strdup_printf("%s_connect", device_name);

	create_log_params(&log_params, common_conf.conn_log_enable, common_conf.conn_log_dir, log_prefix, common_conf.file_size, common_conf.save_days);

	if (common_conf.conn_log_enable)
	{
		g_printf("%s : start connect logger\n",log_prefix);


		if (!init_log_dir(&log_params))
		{
			g_printf("%s : initialization log error\n",log_prefix);
		}

		g_printf("%s : system log path: %s\n",log_prefix, common_conf.conn_log_dir);

		create_log(&log_params);

		if (log_params.log == NULL)
		{
			g_printf("%s : log create error\n",log_prefix);
		}
	}

	connect_dc_socket(dci, &log_params, common_conf.conn_log_trace, device_port);
	guint64 last_connect_time = get_date_time();

	set_dc_main_sock_thread_status(dci, ts_Active);

	gboolean accept_error_sended = FALSE;

	while(get_dc_sock_status(dci)!= ss_Disconnected)
	{
   		if (get_dc_sock_status(dci) == ss_ConnectReady && get_date_time() > last_connect_time + SOCKET_RECONNECT_TIMEOUT && device_port > 0)
   		{
   			connect_dc_socket(dci, &log_params, common_conf.conn_log_trace, device_port);
        	last_connect_time = get_date_time();
   			accept_error_sended = FALSE;
        }

        if (get_dc_sock_status(dci) == ss_Connected )
        {
        	struct sockaddr client_addr;
        	socklen_t client_addr_len = 0;

        	gint32 dc_sock = get_dc_sock(dci);

       		gint32 client_socket = accept(dc_sock, &client_addr, &client_addr_len);

       		if(client_socket < 0)
       		{
       			if (!accept_error_sended)
       			{
       				add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client socket accepting error");
       				accept_error_sended = TRUE;
       			}
       		}
       		else
       		{
               	gboolean pe = TRUE;
               	get_profiles_enable(&pe);

               	guint8 client_index = find_new_dc_client_index(pe);

               	if (client_index != NO_CLIENT)
               	{
               		set_new_dc_client_param(client_index, dci);

               		set_dc_client_sock(client_index, client_socket);

        			struct sockaddr_in addr;
        			socklen_t addr_size = sizeof(struct sockaddr_in);
        			getpeername(client_socket, (struct sockaddr *)&addr, &addr_size);

        			add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "accepted client socket %d (%s)", client_socket, inet_ntoa(addr.sin_addr));

    				DCClientInfo info = { .client_index = client_index,
    															.dispencer_controller_index = dci,
    															.socket = client_socket,
    															.device_name = device_name,
    															.ip_address = g_strdup( inet_ntoa(addr.sin_addr)),
																.log_params = &log_params,
																.log_frames = common_conf.conn_log_frames,
																.log_parsing = common_conf.conn_log_parsing,
																.log_trace = common_conf.conn_log_trace
    															};

    				GThread* client_thread = g_thread_new("dc_client_thread", dc_device_client_thread_func, &info);

    				if (client_thread == NULL)
    				{
    					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "%d error starting client thread socket (%s)", client_socket, inet_ntoa(addr.sin_addr));
    				}
    				else
    				{
    					client_thread->priority = G_THREAD_PRIORITY_LOW;
    				}
        		}
               	else
               	{
					add_log(&log_params, TRUE, TRUE, common_conf.conn_log_trace, TRUE,  "client list is full. Disconnecting client");
               		close(client_socket);
               	}
        	}
        }
	}

    g_free(device_name);
    g_free(log_prefix);

    close(get_dc_sock(dci));

    set_dc_sock(dci, -1);

    return NULL;
}

gpointer dc_device_thread_func(gpointer data)
{
	guchar (*init_lib_func)(DCLibConfig config);
	guchar (*close_lib_func)();

	guint8 device_index = *(guint8*)data;


	unlock_threads_mutex();

	DispencerControllerConf device = {0x00};
	get_dc_conf(device_index, &device);

	guint8 dci = 0;

	if (!get_new_dispencer_controller_index(&dci))
	{
		g_printf("%s : error getting new dispencer controller index\n", device.name);
		set_dc_thread_status(device_index, ts_Finished);
		g_printf("%s : set thread status is finished\n", device.name);

		return NULL;
	}
	else
	{
		g_printf("%s : dci = %d, %d\n", device.name, dci, device.file_size);
	}

	set_dispencer_controller_index(device_index, dci);

	if (!init_dc_log_settings(dci, device.log_dir, device.name, device.log_enable, device.log_trace, device.file_size, device.save_days))
	{
		g_printf("%s : error init log settings\n", device.name);
		set_dispencer_controller_index(device_index, 0);
		set_dc_thread_status(device_index, ts_Finished);
		return NULL;

	}

	LogParams* log_params = NULL;
	gboolean log_trace = FALSE;

	get_dc_log_settings(dci, &log_params, &log_trace);

	g_printf("%d, %d, %p, %s, %s, %d\n", log_params->enable, log_params->file_size, log_params->log, log_params->path, log_params->prefix, log_params->save_days);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "dispencer controller index = %d", dci);

	set_dc_device_status(dci, dcs_NotInitializeDriver);
	set_dc_device_name(dci, device.name);
	set_dc_device_port(dci, device.port);

	set_dc_thread_status(device_index, ts_Active);
	add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "set thread status is active");

	GThread* main_sock_thread = g_thread_new("dc_main_sock_thread", dc_main_socket_thread_func, &dci);

	if (main_sock_thread == NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error starting main socket thread");
	}
	else
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "main socket thread is started");
		main_sock_thread->priority = G_THREAD_PRIORITY_LOW;
	}

	set_dc_device_is_working(dci, TRUE);

	long init_timer = get_date_time();

	while(get_dc_device_is_working(dci))
	{
		if (get_date_time() > init_timer + INIT_DEVICE_TIMEOUT)
		{
			init_timer = get_date_time();

			void* handle_lib = dlopen((gchar*)device.module_name, RTLD_LAZY );

			if (!handle_lib)
			{
				add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "error load module %s", device.module_name);
				set_dispencer_controller_index(device_index, 0);
				set_dc_thread_status(device_index, ts_Finished);
				break;
			}
			else
			{
				add_log(log_params, TRUE, TRUE,log_trace, TRUE, "successfully load module %s", device.module_name);

				init_lib_func = dlsym( handle_lib, "init_lib" );
				gchar* error = dlerror();

				if (error != NULL)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "init function not found (%s)", error);
					g_free(error);
				}
				else
				{
					g_free(error);
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "initialization");

					guchar device_error = (*init_lib_func)(device.module_config);

					set_dc_device_last_error(dci, device_error);

					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "initialization complete with error = %d", device_error);


					if (device_error == dce_NoError)
					{
						set_dc_dp_from_driver(handle_lib, device.name, dci, log_params, log_trace);

						set_ext_funcs_from_driver(handle_lib, device.name, dci, log_params, log_trace);

						set_dispencers_config_from_driver(handle_lib, device.name, dci, log_params, log_trace);


						add_log(log_params, TRUE, TRUE, log_trace, TRUE, "set dispencer controller device working is true");

						guint64 timer = get_date_time();

						while(get_dc_device_is_working(dci))
						{
							if (get_date_time() > timer + device.interval)
							{
								timer = get_date_time();

								set_dc_device_status_from_driver(handle_lib, device.name, dci);

								DcDeviceStatus device_status = get_dc_device_status(dci);

								if (device_status == dcs_NoError)
								{
									guint8 dispencer_count = get_dc_disp_count(dci);

									if (dispencer_count > 0)
									{
										for (guint8 i = 0; i < dispencer_count; i++)
										{

											HardwareServerCommand command = hsc_None;
											guint32 disp_num = 0;
											guint8 nozzle_num = 0;
											guint32 price = 0;
											guint32 volume = 0;
											guint32 amount = 0;
											guint8 index_ext_func = 0;
											DCPricePacks price_packs = {0x00};

											guint8 index_client = find_dc_client_command(dci, i, &command, &disp_num, &nozzle_num, &price, &volume, &amount, &index_ext_func, &price_packs);

											if (index_client != NO_CLIENT)
											{
												add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Finded client index %d command = %d (%s)", index_client, command,server_command_to_str(command));

												switch(command)
												{

													case hsc_DCSetVolumeDose:
														{
															DcDeviceError result = send_volume_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price, volume);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set volume dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
																	disp_num, nozzle_num, price, volume, result);
														}
														break;

													case hsc_DCSetSumDose:
														{
															DcDeviceError result = send_sum_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price, amount);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set amount dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
																	disp_num, nozzle_num, price, volume, result);
														}
														break;

													case hsc_DCSetUnlimDose:
														{
															DcDeviceError result = send_full_tank_dose_to_driver(handle_lib, device.name, dci, disp_num, nozzle_num, price);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set unlim dose for disp %d, nozzle %d, price = %d, volume = %d (result = %d)",
																	disp_num, nozzle_num, price, volume, result);
														}
														break;

													case hsc_DCStart:
														{
															DcDeviceError result = send_start_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set start for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCStop:
														{
															DcDeviceError result = send_stop_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client,  i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCSuspend:
														{
															DcDeviceError result = send_suspend_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client,  i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCResume:
														{
															DcDeviceError result = send_resume_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client,  i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set stop for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCPayment:
														{
															DcDeviceError result = send_payment_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set payment for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCReset:
														{
															DcDeviceError result = send_reset_to_driver(handle_lib, device.name, dci, disp_num);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set reset for disp %d (result = %d)",
																	disp_num, result);
														}
														break;

													case hsc_DCPriceUpdate:
														{
															DcDeviceError result = send_update_prices_to_driver(handle_lib, device.name, dci, disp_num, &price_packs);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set update prices for disp %d (result = %d)",
																	disp_num, result);
														}
														break;


													case hsc_DCExecuteExtendedFunction:
														{
															DcDeviceError result = send_extended_func_to_driver(handle_lib, device.name, dci, disp_num, index_ext_func);
															set_dc_client_command_result(index_client, i, result);
															add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set extended command for disp %d, command index %d (result = %d)", disp_num, index_ext_func, result);
														}
														break;

													default:
														set_dc_client_command_result(index_client, i, dce_WrongCommand);
														add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Wrond command for disp index %d (command = %d)",i, command);
														break;
												}
											}

											set_dispencer_state_from_driver(handle_lib, device.name, dci, i, log_params, log_trace);

											if (get_dc_dispencer_data_is_change(dci, i))
											{
												add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Update dispencer state for all clients for disp index %d",i);
												set_dc_disp_status_is_changed_for_all_clients(dci, i, TRUE);
												set_dc_dispencer_data_is_change(dci, i, FALSE);
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
									add_log(log_params, TRUE, TRUE, log_trace, TRUE, "DEVICE STATUS ERROR");
								}

								if (get_dc_device_status_is_changed(dci))
								{
									add_log(log_params, TRUE, TRUE, log_trace, TRUE, "Set device status is changed for all clients");
									set_dc_device_status_is_changed_for_all_clients(dci, TRUE, device_status);
									set_dc_device_status_is_changed(dci, FALSE);
								}
							}
						}

						add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "Send close lib");

						close_lib_func = dlsym( handle_lib, "close_lib" );
						error = dlerror();

						if (error != NULL)
						{
							add_log(log_params, TRUE, TRUE, log_trace, TRUE,  "driver close func not found (%s)", error);
							g_free(error);
						}
						else
						{
							g_free(error);

							guchar device_error = (*close_lib_func)();
							add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "driver close func return %d", device_error);
						}
					}
				}

				dlclose(handle_lib);

			}
		}
	}
	if (main_sock_thread !=NULL)
	{
		set_dc_main_sock_thread_status(dci, ts_Finished);

		//--------------------------------------- stopping all client threads ------------------------------------------
		if (get_dc_sock(dci) >= 0)
		{
			close_dc_sock(dci);
		}

		g_thread_join(main_sock_thread);
		//--------------------------------------------------------------------------------------------------------------
	}

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finish device thread");

	set_dispencer_controller_index(device_index, 0);

	set_dc_thread_status(device_index, ts_Finished);

	add_log(log_params, TRUE, TRUE, log_trace, TRUE,   "finished");

	close_log(log_params);

	return NULL;
}
