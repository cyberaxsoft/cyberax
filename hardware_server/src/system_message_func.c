#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "tlv.h"
#include "socket_func.h"

void send_system_common_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system common configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s Common configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetCommonServerConfig, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetCommonServerConfig, server_command_to_str(hsc_GetCommonServerConfig));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	TlvUnit* conf_units = NULL;

	CommonConf common_conf = {0x00};
	get_common_conf(&common_conf);

	gboolean profiles_enable = FALSE;
	get_profiles_enable(&profiles_enable);

	add_tlv_unit_8( &conf_units, tsct_ProfilesEnable, profiles_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ProfilesEnable: %d", client_info->device_name, profiles_enable);
	add_tlv_unit_str( &conf_units, tsct_ServerName, common_conf.server_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ServerName: %s", client_info->device_name, common_conf.server_name);
	add_tlv_unit_16( &conf_units, tsct_ConfigurationPort, common_conf.port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConfigurationPort: %d", client_info->device_name, common_conf.port);
	add_tlv_unit_str( &conf_units, tsct_LogDir, common_conf.log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogDir: %s", client_info->device_name, common_conf.log_dir);
	add_tlv_unit_8( &conf_units, tsct_LogEnable, common_conf.log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogEnable: %d", client_info->device_name, common_conf.log_enable);
	add_tlv_unit_8( &conf_units, tsct_LogTrace, common_conf.log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogTrace: %d", client_info->device_name, common_conf.log_trace);
	add_tlv_unit_32( &conf_units, tsct_LogFileSize, common_conf.file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogFileSize: %d", client_info->device_name,common_conf.file_size);
	add_tlv_unit_32( &conf_units, tsct_LogSaveDays, common_conf.save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogSaveDays: %d", client_info->device_name,common_conf.save_days);
	add_tlv_unit_str( &conf_units, tsct_ConnLogDir, common_conf.conn_log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogDir: %s", client_info->device_name, common_conf.conn_log_dir);
	add_tlv_unit_8( &conf_units, tsct_ConnLogEnable, common_conf.conn_log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogEnable: %d", client_info->device_name, common_conf.conn_log_enable);
	add_tlv_unit_8( &conf_units, tsct_ConnLogTrace, common_conf.conn_log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogTrace: %d", client_info->device_name, common_conf.conn_log_trace);
	add_tlv_unit_8( &conf_units, tsct_ConnLogFrames, common_conf.conn_log_frames, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogFrames: %d", client_info->device_name, common_conf.conn_log_frames);
	add_tlv_unit_8( &conf_units, tsct_ConnLogParsing, common_conf.conn_log_parsing, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogParsing: %d", client_info->device_name, common_conf.conn_log_parsing);
	add_tlv_unit_32( &conf_units, tsct_ConnLogFileSize, common_conf.conn_log_file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogFileSize: %d", client_info->device_name,common_conf.conn_log_file_size);
	add_tlv_unit_32( &conf_units, tsct_ConnLogSaveDays, common_conf.conn_log_save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		ConnLogSaveDays: %d", client_info->device_name,common_conf.conn_log_save_days);

	guint32 configuration_size = 0;
	guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

	if (configuration_frame !=NULL && configuration_size > 0)
	{
		tlv_add_unit(&units,tlv_create_unit(tst_ServerCommonSettings , configuration_frame, 0, configuration_size));
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

void send_system_dispencer_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system dispencer controllers configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s System dispencer controllers configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetDispencerControllerConfig, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetDispencerControllerConfig, server_command_to_str(hsc_GetDispencerControllerConfig));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	DeviceConfs device_confs = {0x00};
	get_device_confs(&device_confs);

	add_tlv_unit_8( &units, tst_DispencerControllerCount, device_confs.dispencer_controller_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Dispencer controller count: %d ", client_info->device_name, device_confs.dispencer_controller_count);

	if (device_confs.dispencer_controller_count > 0)
	{
		for (guint8 i = 0; i < device_confs.dispencer_controller_count; i++)
		{
			TlvUnit* dc_units = NULL;
			DispencerControllerConf* dc_conf = &device_confs.dispencer_controllers[i];
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Dispencer controller: ", client_info->device_name);

			add_tlv_unit_8( &dc_units, tdst_Id, dc_conf->id, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Id: %d", client_info->device_name, dc_conf->id);
			add_tlv_unit_str( &dc_units, tdst_Name, dc_conf->name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Name: %s", client_info->device_name, dc_conf->name);
			add_tlv_unit_16(&dc_units, tdst_Port, dc_conf->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Port: %d", client_info->device_name, dc_conf->port);
			add_tlv_unit_8( &dc_units, tdst_Enable, dc_conf->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Enable: %d", client_info->device_name, dc_conf->enable);
			add_tlv_unit_32(&dc_units, tdst_CommandTimeout, dc_conf->command_timeout, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Command timeout: %d ", client_info->device_name, dc_conf->command_timeout);
			add_tlv_unit_32(&dc_units, tdst_Interval, dc_conf->interval, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Interval: %d ", client_info->device_name, dc_conf->interval);
			add_tlv_unit_str( &dc_units, tdst_Module, dc_conf->module_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Module: %s", client_info->device_name, dc_conf->module_name);
			add_tlv_unit_str( &dc_units, tdst_LogDir, dc_conf->log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogDir: %s", client_info->device_name, dc_conf->log_dir);
			add_tlv_unit_8( &dc_units, tdst_LogEnable, dc_conf->log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogEnable: %d", client_info->device_name, dc_conf->log_enable);
			add_tlv_unit_8( &dc_units, tdst_LogTrace, dc_conf->log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogTrace: %d", client_info->device_name, dc_conf->log_trace);
			add_tlv_unit_32(&dc_units, tdst_LogFileSize, dc_conf->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogFileSize: %d", client_info->device_name,dc_conf->file_size);
			add_tlv_unit_32(&dc_units, tdst_LogSaveDays, dc_conf->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogSaveDays: %d", client_info->device_name,dc_conf->save_days);

			//module settings
			TlvUnit* ms_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 			Module settings: ", client_info->device_name);

			DCLibConfig* module_config = &dc_conf->module_config;

			//module log settings
			TlvUnit* ms_log_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Log settings: ", client_info->device_name);

			LibLogOptions* log_options = &module_config->log_options;

			add_tlv_unit_8( &ms_log_units, tmlst_Enable, log_options->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Enable: %d", client_info->device_name, log_options->enable);
			add_tlv_unit_str( &ms_log_units, tmlst_Directory,  log_options->dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Dir: %s", client_info->device_name, log_options->dir);
			add_tlv_unit_8( &ms_log_units, tmlst_Trace, log_options->trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Trace: %d", client_info->device_name, log_options->trace);
			add_tlv_unit_8( &ms_log_units, tmlst_System, log_options->system, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					System: %d", client_info->device_name, log_options->system);
			add_tlv_unit_8( &ms_log_units, tmlst_Requests, log_options->requests, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Requests: %d", client_info->device_name, log_options->requests);
			add_tlv_unit_8( &ms_log_units, tmlst_Frames, log_options->frames, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Frames: %d", client_info->device_name, log_options->frames);
			add_tlv_unit_8( &ms_log_units, tmlst_Parsing, log_options->parsing, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Parsing: %d", client_info->device_name, log_options->parsing);
			add_tlv_unit_32( &ms_log_units, tmlst_FileSize, log_options->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					File size: %d", client_info->device_name, log_options->file_size);
			add_tlv_unit_32( &ms_log_units, tmlst_SaveDays, log_options->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Save days: %d", client_info->device_name, log_options->save_days);

			guint32 ms_log_size = 0;
			guchar* ms_log_frame = tlv_serialize_units(ms_log_units, &ms_log_size);

			if (ms_log_frame !=NULL && ms_log_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tdcmst_LogSettings , ms_log_frame, 0, ms_log_size));
				g_free(ms_log_frame);
			}
			tlv_delete_units(&ms_log_units);

			//module conn settings
			TlvUnit* ms_conn_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Connection settings: ", client_info->device_name);

			ConnOptions* conn_options = &module_config->conn_options;

			add_tlv_unit_8( &ms_conn_units, tmcst_Type, conn_options->connection_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->connection_type);
			add_tlv_unit_str( &ms_conn_units, tmcst_Port,  conn_options->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Port: %s", client_info->device_name, conn_options->port);
			add_tlv_unit_str( &ms_conn_units, tmcst_IPAddress,  conn_options->ip_address, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					IP Address: %s", client_info->device_name, conn_options->ip_address);
			add_tlv_unit_16( &ms_conn_units, tmcst_IPPort, conn_options->ip_port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->ip_port);
			add_tlv_unit_32( &ms_conn_units, tmcst_UartBaudrate, conn_options->uart_baudrate, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart baudrate: %d", client_info->device_name, conn_options->uart_baudrate);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartByteSize, conn_options->uart_byte_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart byte size: %d", client_info->device_name, conn_options->uart_byte_size);
			add_tlv_unit_str( &ms_conn_units, tmcst_UartParity,  conn_options->uart_parity, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart parity: %s", client_info->device_name, conn_options->uart_parity);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartStopBits, conn_options->uart_stop_bits, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart stop bits: %d", client_info->device_name, conn_options->uart_stop_bits);

			guint32 ms_conn_size = 0;
			guchar* ms_conn_frame = tlv_serialize_units(ms_conn_units, &ms_conn_size);

			if (ms_conn_frame !=NULL && ms_conn_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tdcmst_ConnSettings , ms_conn_frame, 0, ms_conn_size));
				g_free(ms_conn_frame);
			}
			tlv_delete_units(&ms_conn_units);

			//module timeouts settings
			TlvUnit* ms_timeout_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Timeout settings: ", client_info->device_name);

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			add_tlv_unit_32( &ms_timeout_units, tmtst_Read, timeout_options->t_read, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Read: %d", client_info->device_name, timeout_options->t_read);
			add_tlv_unit_32( &ms_timeout_units, tmtst_Write, timeout_options->t_write, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Write: %d", client_info->device_name, timeout_options->t_write);

			guint32 ms_timeout_size = 0;
			guchar* ms_timeout_frame = tlv_serialize_units(ms_timeout_units, &ms_timeout_size);

			if (ms_timeout_frame !=NULL && ms_timeout_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tdcmst_TimeoutSettings , ms_timeout_frame, 0, ms_timeout_size));
				g_free(ms_timeout_frame);
			}
			tlv_delete_units(&ms_timeout_units);

			//module dp settings
			TlvUnit* ms_dp_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Decimal point settings: ", client_info->device_name);

			DCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

			add_tlv_unit_8( &ms_dp_units, tdcmdpst_Price, dp_options->dp_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Price: %d", client_info->device_name, dp_options->dp_price);
			add_tlv_unit_8( &ms_dp_units, tdcmdpst_Volume, dp_options->dp_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Volume: %d", client_info->device_name, dp_options->dp_volume);
			add_tlv_unit_8( &ms_dp_units, tdcmdpst_Amount, dp_options->dp_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Amount: %d", client_info->device_name, dp_options->dp_amount);

			guint32 ms_dp_size = 0;
			guchar* ms_dp_frame = tlv_serialize_units(ms_dp_units, &ms_dp_size);

			if (ms_dp_frame !=NULL && ms_dp_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tdcmst_DPSettings , ms_dp_frame, 0, ms_dp_size));
				g_free(ms_dp_frame);
			}
			tlv_delete_units(&ms_dp_units);

//			add_tlv_unit_8( &ms_units, tdcmst_CountersEnable, dp_options->dp_amount, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Amount: %d", client_info->device_name, dp_options->dp_amount);

			add_tlv_unit_8( &ms_units, tdcmst_CountersEnable, module_config->counters_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 				CountersEnable: %d", client_info->device_name, module_config->counters_enable);
			add_tlv_unit_8( &ms_units, tdcmst_AutoStart, module_config->auto_start, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 				AutoStart: %d", client_info->device_name, module_config->auto_start);
			add_tlv_unit_8( &ms_units, tdcmst_AutoPayment, module_config->auto_payment, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 				AutoPayment: %d", client_info->device_name, module_config->auto_payment);
			add_tlv_unit_32(&ms_units, tdcmst_FullTankVolume, module_config->full_tank_volume, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 				FullTankVolume: %d", client_info->device_name, module_config->full_tank_volume);

			//module mapping
			TlvUnit* ms_mapping_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Mapping: ", client_info->device_name);

			add_tlv_unit_8( &ms_mapping_units, tdmmt_DispencerCount, module_config->dispencer_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Dispencer count: %d", client_info->device_name, module_config->dispencer_count);

			if (dc_conf->module_config.dispencer_count > 0)
			{
				for (guint8 j = 0; j < dc_conf->module_config.dispencer_count; j++)
				{
					DispencerConf* disp = &module_config->dispencers[j];

					TlvUnit* disp_units = NULL;
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 						Dispencer:", client_info->device_name);

					add_tlv_unit_32(&disp_units, tmdct_Number, disp->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Number: %d", client_info->device_name, disp->num);
					add_tlv_unit_8(&disp_units, tmdct_Address, disp->addr, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Address: %d", client_info->device_name, disp->addr);
					add_tlv_unit_8(&disp_units, tmdct_NozzleCount, disp->nozzle_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Nozzle count: %d", client_info->device_name, disp->nozzle_count);

					if (disp->nozzle_count > 0)
					{
						for (guint8 k = 0; k < disp->nozzle_count; k++)
						{
							NozzleConf* nozzle = &disp->nozzles[k];

							TlvUnit* nozzle_units = NULL;
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 							Nozzles:", client_info->device_name);

							add_tlv_unit_8(&nozzle_units, tmnct_Number, nozzle->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 								Num: %d", client_info->device_name, nozzle->num);
							add_tlv_unit_8(&nozzle_units, tmnct_Grade, nozzle->grade, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 								Grade: %d", client_info->device_name, nozzle->grade);

							guint32 nozzle_size = 0;
							guchar* nozzle_frame = tlv_serialize_units(nozzle_units, &nozzle_size);

							if (nozzle_frame !=NULL && nozzle_size > 0)
							{
								tlv_add_unit(&disp_units,tlv_create_unit(tmdct_Nozzle , nozzle_frame, 0, nozzle_size));
								g_free(nozzle_frame);
							}
							tlv_delete_units(&nozzle_units);

						}
					}

					guint32 disp_size = 0;
					guchar* disp_frame = tlv_serialize_units(disp_units, &disp_size);

					if (disp_frame !=NULL && disp_size > 0)
					{
						tlv_add_unit(&ms_mapping_units,tlv_create_unit(tdmmt_DispencerConfiguration , disp_frame, 0, disp_size));
						g_free(disp_frame);
					}
					tlv_delete_units(&disp_units);

				}
			}

			guint32 ms_mapping_size = 0;
			guchar* ms_mapping_frame = tlv_serialize_units(ms_mapping_units, &ms_mapping_size);

			if (ms_mapping_frame !=NULL && ms_mapping_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tdcmst_ModuleMapping , ms_mapping_frame, 0, ms_mapping_size));
				g_free(ms_mapping_frame);
			}
			tlv_delete_units(&ms_mapping_units);

			guint32 ms_size = 0;
			guchar* ms_frame = tlv_serialize_units(ms_units, &ms_size);

			if (ms_frame !=NULL && ms_size > 0)
			{
				tlv_add_unit(&dc_units,tlv_create_unit(tdst_ModuleSettings , ms_frame, 0, ms_size));
				g_free(ms_frame);
			}

			tlv_delete_units(&ms_units);

			guint32 dc_size = 0;
			guchar* dc_frame = tlv_serialize_units(dc_units, &dc_size);

			if (dc_frame !=NULL && dc_size > 0)
			{
				tlv_add_unit(&units,tlv_create_unit(tst_DispencerControllerConfig , dc_frame, 0, dc_size));
				g_free(dc_frame);
			}

			tlv_delete_units(&dc_units);
		}
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

void send_system_tgs_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system tgs configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s System tgs configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetTgsConfig, 		client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetTgsConfig, server_command_to_str(hsc_GetTgsConfig));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	DeviceConfs device_confs = {0x00};
	get_device_confs(&device_confs);

	add_tlv_unit_8( &units, tst_TgsCount, 	device_confs.tgs_count,	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Tgs count: %d ", client_info->device_name, device_confs.tgs_count);

	if (device_confs.tgs_count > 0)
	{
		for (guint8 i = 0; i < device_confs.tgs_count; i++)
		{
			TlvUnit* tgs_units = NULL;
			TgsConf* tgs_conf = &device_confs.tgs[i];
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Tgs: ", client_info->device_name);


			add_tlv_unit_8( &tgs_units, ttst_Id, tgs_conf->id, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Id: %d", client_info->device_name, tgs_conf->id);
			add_tlv_unit_str( &tgs_units, ttst_Name, tgs_conf->name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Name: %s", client_info->device_name, tgs_conf->name);
			add_tlv_unit_16(&tgs_units, ttst_Port, tgs_conf->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Port: %d", client_info->device_name, tgs_conf->port);
			add_tlv_unit_8( &tgs_units, ttst_Enable, tgs_conf->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Enable: %d", client_info->device_name, tgs_conf->enable);
			add_tlv_unit_32(&tgs_units, ttst_CommandTimeout, tgs_conf->command_timeout, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Command timeout: %d ", client_info->device_name, tgs_conf->command_timeout);
			add_tlv_unit_32(&tgs_units, ttst_Interval, tgs_conf->interval, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Interval: %d ", client_info->device_name, tgs_conf->interval);
			add_tlv_unit_str( &tgs_units, ttst_Module, tgs_conf->module_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Module: %s", client_info->device_name, tgs_conf->module_name);
			add_tlv_unit_str( &tgs_units, ttst_LogDir, tgs_conf->log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogDir: %s", client_info->device_name, tgs_conf->log_dir);
			add_tlv_unit_8( &tgs_units, ttst_LogEnable, tgs_conf->log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogEnable: %d", client_info->device_name, tgs_conf->log_enable);
			add_tlv_unit_8( &tgs_units, ttst_LogTrace, tgs_conf->log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogTrace: %d", client_info->device_name, tgs_conf->log_trace);
			add_tlv_unit_32(&tgs_units, ttst_LogFileSize, tgs_conf->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogFileSize: %d", client_info->device_name,tgs_conf->file_size);
			add_tlv_unit_32(&tgs_units, ttst_LogSaveDays, tgs_conf->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogSaveDays: %d", client_info->device_name,tgs_conf->save_days);

			//module settings
			TlvUnit* ms_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 			Module settings: ", client_info->device_name);

			TGSLibConfig* module_config = &tgs_conf->module_config;

			//module log settings
			TlvUnit* ms_log_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Log settings: ", client_info->device_name);

			LibLogOptions* log_options = &module_config->log_options;

			add_tlv_unit_8( &ms_log_units, tmlst_Enable, log_options->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Enable: %d", client_info->device_name, log_options->enable);
			add_tlv_unit_str( &ms_log_units, tmlst_Directory,  log_options->dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Dir: %s", client_info->device_name, log_options->dir);
			add_tlv_unit_8( &ms_log_units, tmlst_Trace, log_options->trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Trace: %d", client_info->device_name, log_options->trace);
			add_tlv_unit_8( &ms_log_units, tmlst_System, log_options->system, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					System: %d", client_info->device_name, log_options->system);
			add_tlv_unit_8( &ms_log_units, tmlst_Requests, log_options->requests, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Requests: %d", client_info->device_name, log_options->requests);
			add_tlv_unit_8( &ms_log_units, tmlst_Frames, log_options->frames, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Frames: %d", client_info->device_name, log_options->frames);
			add_tlv_unit_8( &ms_log_units, tmlst_Parsing, log_options->parsing, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Parsing: %d", client_info->device_name, log_options->parsing);
			add_tlv_unit_32( &ms_log_units, tmlst_FileSize, log_options->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					File size: %d", client_info->device_name, log_options->file_size);
			add_tlv_unit_32( &ms_log_units, tmlst_SaveDays, log_options->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Save days: %d", client_info->device_name, log_options->save_days);

			guint32 ms_log_size = 0;
			guchar* ms_log_frame = tlv_serialize_units(ms_log_units, &ms_log_size);

			if (ms_log_frame !=NULL && ms_log_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(ttgsmst_LogSettings , ms_log_frame, 0, ms_log_size));
				g_free(ms_log_frame);
			}
			tlv_delete_units(&ms_log_units);

			//module conn settings
			TlvUnit* ms_conn_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Connection settings: ", client_info->device_name);

			ConnOptions* conn_options = &module_config->conn_options;

			add_tlv_unit_8( &ms_conn_units, tmcst_Type, conn_options->connection_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->connection_type);
			add_tlv_unit_str( &ms_conn_units, tmcst_Port,  conn_options->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Port: %s", client_info->device_name, conn_options->port);
			add_tlv_unit_str( &ms_conn_units, tmcst_IPAddress,  conn_options->ip_address, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					IP Address: %s", client_info->device_name, conn_options->ip_address);
			add_tlv_unit_16( &ms_conn_units, tmcst_IPPort, conn_options->ip_port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->ip_port);
			add_tlv_unit_32( &ms_conn_units, tmcst_UartBaudrate, conn_options->uart_baudrate, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart baudrate: %d", client_info->device_name, conn_options->uart_baudrate);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartByteSize, conn_options->uart_byte_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart byte size: %d", client_info->device_name, conn_options->uart_byte_size);
			add_tlv_unit_str( &ms_conn_units, tmcst_UartParity,  conn_options->uart_parity, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart parity: %s", client_info->device_name, conn_options->uart_parity);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartStopBits, conn_options->uart_stop_bits, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart stop bits: %d", client_info->device_name, conn_options->uart_stop_bits);

			guint32 ms_conn_size = 0;
			guchar* ms_conn_frame = tlv_serialize_units(ms_conn_units, &ms_conn_size);

			if (ms_conn_frame !=NULL && ms_conn_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(ttgsmst_ConnSettings , ms_conn_frame, 0, ms_conn_size));
				g_free(ms_conn_frame);
			}
			tlv_delete_units(&ms_conn_units);


			//module timeouts settings
			TlvUnit* ms_timeout_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Timeout settings: ", client_info->device_name);

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			add_tlv_unit_32( &ms_timeout_units, tmtst_Read, timeout_options->t_read, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Read: %d", client_info->device_name, timeout_options->t_read);
			add_tlv_unit_32( &ms_timeout_units, tmtst_Write, timeout_options->t_write, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Write: %d", client_info->device_name, timeout_options->t_write);

			guint32 ms_timeout_size = 0;
			guchar* ms_timeout_frame = tlv_serialize_units(ms_timeout_units, &ms_timeout_size);

			if (ms_timeout_frame !=NULL && ms_timeout_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(ttgsmst_TimeoutSettings , ms_timeout_frame, 0, ms_timeout_size));
				g_free(ms_timeout_frame);
			}
			tlv_delete_units(&ms_timeout_units);

			//module mapping
			TlvUnit* ms_mapping_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Mapping: ", client_info->device_name);

			add_tlv_unit_8( &ms_mapping_units, ttmmt_TankCount, module_config->tank_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Tank count: %d", client_info->device_name, module_config->tank_count);

			if (tgs_conf->module_config.tank_count > 0)
			{
				for (guint8 j = 0; j < tgs_conf->module_config.tank_count; j++)
				{
					TankConf* tank = &tgs_conf->module_config.tanks[j];

					TlvUnit* tank_units = NULL;
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 						Tank:", client_info->device_name);

					add_tlv_unit_32( &tank_units, tmtct_Number, tank->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Number: %d", client_info->device_name, tank->num);
					add_tlv_unit_8( &tank_units, tmtct_Channel, tank->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Channel: %d", client_info->device_name, tank->channel);

					guint32 tank_size = 0;
					guchar* tank_frame = tlv_serialize_units(tank_units, &tank_size);

					if (tank_frame !=NULL && tank_size > 0)
					{
						tlv_add_unit(&ms_mapping_units,tlv_create_unit(ttmmt_TankConfiguration , tank_frame, 0, tank_size));
						g_free(tank_frame);
					}
					tlv_delete_units(&tank_units);

				}
			}

			guint32 ms_mapping_size = 0;
			guchar* ms_mapping_frame = tlv_serialize_units(ms_mapping_units, &ms_mapping_size);

			if (ms_mapping_frame !=NULL && ms_mapping_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(ttgsmst_ModuleMapping , ms_mapping_frame, 0, ms_mapping_size));
				g_free(ms_mapping_frame);
			}
			tlv_delete_units(&ms_mapping_units);

			guint32 ms_size = 0;
			guchar* ms_frame = tlv_serialize_units(ms_units, &ms_size);

			if (ms_frame !=NULL && ms_size > 0)
			{
				tlv_add_unit(&tgs_units,tlv_create_unit(ttst_ModuleSettings , ms_frame, 0, ms_size));
				g_free(ms_frame);
			}

			tlv_delete_units(&ms_units);
			//end module settings

			guint32 tgs_size = 0;
			guchar* tgs_frame = tlv_serialize_units(tgs_units, &tgs_size);

			if (tgs_frame !=NULL && tgs_size > 0)
			{
				tlv_add_unit(&units,tlv_create_unit(tst_TgsConfig , tgs_frame, 0, tgs_size));
				g_free(tgs_frame);
			}

			tlv_delete_units(&tgs_units);
		}
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

void send_system_client_profile_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system client profile configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s System client profile configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetClientProfiles, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetClientProfiles, server_command_to_str(hsc_GetClientProfiles));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	ProfilesConf profiles_conf = {0x00};
	get_profiles_conf(&profiles_conf);

	add_tlv_unit_8( &units, tst_ClientProfileCount, profiles_conf.profiles_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Profiles count: %d ", client_info->device_name, profiles_conf.profiles_count);

	if (profiles_conf.profiles_count > 0)
	{
		for (guint8 i = 0; i < profiles_conf.profiles_count; i++ )
		{
			TlvUnit* conf_units = NULL;
			Profile* profile = &profiles_conf.profiles[i];
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Profile:", client_info->device_name);

			add_tlv_unit_8(&conf_units, tspct_Id, profile->id, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Id: %d", client_info->device_name, profile->id);
			add_tlv_unit_str(&conf_units, tspct_Name, profile->name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Name: %s", client_info->device_name, profile->name);
			add_tlv_unit_8(&conf_units, tspct_Enable, profile->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Enable: %d", client_info->device_name, profile->enable);
			add_tlv_unit_str(&conf_units, tspct_Guid, profile->guid, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Guid: %s", client_info->device_name, profile->guid);
			add_tlv_unit_8(&conf_units, tspct_AccessLevel, profile->access_level, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Access level: %d", client_info->device_name, profile->access_level);

			guint32 configuration_size = 0;
			guchar* configuration_frame = tlv_serialize_units(conf_units, &configuration_size);

			if (configuration_frame !=NULL && configuration_size > 0)
			{
				tlv_add_unit(&units,tlv_create_unit(tst_ClientProfile , configuration_frame, 0, configuration_size));
				g_free(configuration_frame);
			}

			tlv_delete_units(&conf_units);
		}
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

void send_system_command_result_message(SockClientInfo* client_info, guint32 message_id, HardwareServerCommand command, ExchangeError exchange_error )
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send device command result message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s Device command result message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, mt_Reply, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, mt_Reply, return_message_type_name(mt_Reply));
	add_tlv_unit_32(&units, tst_CommandCode, command, 				client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, command, server_command_to_str(command));
	add_tlv_unit_8( &units, tst_Error, 		exchange_error,			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, exchange_error);

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		socket_send(client_info, frame, size );
		free(frame);
	}

	tlv_delete_units(&units);
}

void send_system_price_pole_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system price pole controllers configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s System price pole controllers configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetPricePoleControllerConfig, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetPricePoleControllerConfig, server_command_to_str(hsc_GetPricePoleControllerConfig));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	DeviceConfs device_confs = {0x00};
	get_device_confs(&device_confs);

	add_tlv_unit_8( &units, tst_PpcCount, device_confs.price_pole_controller_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Price pole controller count: %d ", client_info->device_name, device_confs.price_pole_controller_count);

	if (device_confs.price_pole_controller_count > 0)
	{
		for (guint8 i = 0; i < device_confs.price_pole_controller_count; i++)
		{
			TlvUnit* ppc_units = NULL;
			PpcConf* ppc_conf = &device_confs.price_pole_controllers[i];
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Price pole controller: ", client_info->device_name);

			add_tlv_unit_8( &ppc_units, tspt_Id, ppc_conf->id, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Id: %d", client_info->device_name, ppc_conf->id);
			add_tlv_unit_str( &ppc_units, tspt_Name, ppc_conf->name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Name: %s", client_info->device_name, ppc_conf->name);
			add_tlv_unit_16(&ppc_units, tspt_Port, ppc_conf->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Port: %d", client_info->device_name, ppc_conf->port);
			add_tlv_unit_8( &ppc_units, tspt_Enable, ppc_conf->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Enable: %d", client_info->device_name, ppc_conf->enable);
			add_tlv_unit_32(&ppc_units, tspt_CommandTimeout, ppc_conf->command_timeout, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Command timeout: %d ", client_info->device_name, ppc_conf->command_timeout);
			add_tlv_unit_32(&ppc_units, tspt_Interval, ppc_conf->interval, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Interval: %d ", client_info->device_name, ppc_conf->interval);
			add_tlv_unit_str( &ppc_units, tspt_Module, ppc_conf->module_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Module: %s", client_info->device_name, ppc_conf->module_name);
			add_tlv_unit_str( &ppc_units, tspt_LogDir, ppc_conf->log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogDir: %s", client_info->device_name, ppc_conf->log_dir);
			add_tlv_unit_8( &ppc_units, tspt_LogEnable, ppc_conf->log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogEnable: %d", client_info->device_name, ppc_conf->log_enable);
			add_tlv_unit_8( &ppc_units, tspt_LogTrace, ppc_conf->log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogTrace: %d", client_info->device_name, ppc_conf->log_trace);
			add_tlv_unit_32(&ppc_units, tspt_LogFileSize, ppc_conf->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogFileSize: %d", client_info->device_name,ppc_conf->file_size);
			add_tlv_unit_32(&ppc_units, tspt_LogSaveDays, ppc_conf->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogSaveDays: %d", client_info->device_name,ppc_conf->save_days);

			//module settings
			TlvUnit* ms_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 			Module settings: ", client_info->device_name);

			PPCLibConfig* module_config = &ppc_conf->module_config;

			//module log settings
			TlvUnit* ms_log_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Log settings: ", client_info->device_name);

			LibLogOptions* log_options = &module_config->log_options;

			add_tlv_unit_8( &ms_log_units, tmlst_Enable, log_options->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Enable: %d", client_info->device_name, log_options->enable);
			add_tlv_unit_str( &ms_log_units, tmlst_Directory,  log_options->dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Dir: %s", client_info->device_name, log_options->dir);
			add_tlv_unit_8( &ms_log_units, tmlst_Trace, log_options->trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Trace: %d", client_info->device_name, log_options->trace);
			add_tlv_unit_8( &ms_log_units, tmlst_System, log_options->system, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					System: %d", client_info->device_name, log_options->system);
			add_tlv_unit_8( &ms_log_units, tmlst_Requests, log_options->requests, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Requests: %d", client_info->device_name, log_options->requests);
			add_tlv_unit_8( &ms_log_units, tmlst_Frames, log_options->frames, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Frames: %d", client_info->device_name, log_options->frames);
			add_tlv_unit_8( &ms_log_units, tmlst_Parsing, log_options->parsing, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Parsing: %d", client_info->device_name, log_options->parsing);
			add_tlv_unit_32( &ms_log_units, tmlst_FileSize, log_options->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					File size: %d", client_info->device_name, log_options->file_size);
			add_tlv_unit_32( &ms_log_units, tmlst_SaveDays, log_options->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Save days: %d", client_info->device_name, log_options->save_days);

			guint32 ms_log_size = 0;
			guchar* ms_log_frame = tlv_serialize_units(ms_log_units, &ms_log_size);

			if (ms_log_frame !=NULL && ms_log_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tppcmst_LogSettings , ms_log_frame, 0, ms_log_size));
				g_free(ms_log_frame);
			}
			tlv_delete_units(&ms_log_units);

			//module conn settings
			TlvUnit* ms_conn_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Connection settings: ", client_info->device_name);

			ConnOptions* conn_options = &module_config->conn_options;

			add_tlv_unit_8( &ms_conn_units, tmcst_Type, conn_options->connection_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->connection_type);
			add_tlv_unit_str( &ms_conn_units, tmcst_Port,  conn_options->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Port: %s", client_info->device_name, conn_options->port);
			add_tlv_unit_str( &ms_conn_units, tmcst_IPAddress,  conn_options->ip_address, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					IP Address: %s", client_info->device_name, conn_options->ip_address);
			add_tlv_unit_16( &ms_conn_units, tmcst_IPPort, conn_options->ip_port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->ip_port);
			add_tlv_unit_32( &ms_conn_units, tmcst_UartBaudrate, conn_options->uart_baudrate, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart baudrate: %d", client_info->device_name, conn_options->uart_baudrate);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartByteSize, conn_options->uart_byte_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart byte size: %d", client_info->device_name, conn_options->uart_byte_size);
			add_tlv_unit_str( &ms_conn_units, tmcst_UartParity,  conn_options->uart_parity, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart parity: %s", client_info->device_name, conn_options->uart_parity);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartStopBits, conn_options->uart_stop_bits, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart stop bits: %d", client_info->device_name, conn_options->uart_stop_bits);

			guint32 ms_conn_size = 0;
			guchar* ms_conn_frame = tlv_serialize_units(ms_conn_units, &ms_conn_size);

			if (ms_conn_frame !=NULL && ms_conn_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tppcmst_ConnSettings , ms_conn_frame, 0, ms_conn_size));
				g_free(ms_conn_frame);
			}
			tlv_delete_units(&ms_conn_units);

			//module timeouts settings
			TlvUnit* ms_timeout_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Timeout settings: ", client_info->device_name);

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			add_tlv_unit_32( &ms_timeout_units, tmtst_Read, timeout_options->t_read, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Read: %d", client_info->device_name, timeout_options->t_read);
			add_tlv_unit_32( &ms_timeout_units, tmtst_Write, timeout_options->t_write, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Write: %d", client_info->device_name, timeout_options->t_write);

			guint32 ms_timeout_size = 0;
			guchar* ms_timeout_frame = tlv_serialize_units(ms_timeout_units, &ms_timeout_size);

			if (ms_timeout_frame !=NULL && ms_timeout_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tppcmst_TimeoutSettings , ms_timeout_frame, 0, ms_timeout_size));
				g_free(ms_timeout_frame);
			}
			tlv_delete_units(&ms_timeout_units);

			//module dp settings
			TlvUnit* ms_dp_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Decimal point settings: ", client_info->device_name);

			PPCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

			add_tlv_unit_8( &ms_dp_units, tmppcdpst_Price, dp_options->dp_price, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Price: %d", client_info->device_name, dp_options->dp_price);

			guint32 ms_dp_size = 0;
			guchar* ms_dp_frame = tlv_serialize_units(ms_dp_units, &ms_dp_size);

			if (ms_dp_frame !=NULL && ms_dp_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tppcmst_DpSettings , ms_dp_frame, 0, ms_dp_size));
				g_free(ms_dp_frame);
			}
			tlv_delete_units(&ms_dp_units);

			//module mapping
			TlvUnit* ms_mapping_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Mapping: ", client_info->device_name);

			add_tlv_unit_8( &ms_mapping_units, tppcmmt_PricePoleCount, module_config->price_pole_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Price pole count: %d", client_info->device_name, module_config->price_pole_count);

			if (ppc_conf->module_config.price_pole_count > 0)
			{
				for (guint8 j = 0; j < ppc_conf->module_config.price_pole_count; j++)
				{
					PricePoleConf* pricepole = &module_config->price_poles[j];

					TlvUnit* price_pole_units = NULL;
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 						Price pole:", client_info->device_name);

					add_tlv_unit_8(&price_pole_units, tppmct_Number, pricepole->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Number: %d", client_info->device_name, pricepole->num);
					add_tlv_unit_8(&price_pole_units, tppmct_Grade, pricepole->grade, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Grade: %d", client_info->device_name, pricepole->grade);
					add_tlv_unit_8(&price_pole_units, tppmct_SymbolCount, pricepole->symbol_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Symbol count: %d", client_info->device_name, pricepole->symbol_count);

					guint32 price_pole_size = 0;
					guchar* price_pole_frame = tlv_serialize_units(price_pole_units, &price_pole_size);

					if (price_pole_frame !=NULL && price_pole_size > 0)
					{
						tlv_add_unit(&ms_mapping_units,tlv_create_unit(tppcmmt_PricePoleConfiguration , price_pole_frame, 0, price_pole_size));
						g_free(price_pole_frame);
					}
					tlv_delete_units(&price_pole_units);

				}
			}

			guint32 ms_mapping_size = 0;
			guchar* ms_mapping_frame = tlv_serialize_units(ms_mapping_units, &ms_mapping_size);

			if (ms_mapping_frame !=NULL && ms_mapping_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tppcmst_ModuleMapping , ms_mapping_frame, 0, ms_mapping_size));
				g_free(ms_mapping_frame);
			}
			tlv_delete_units(&ms_mapping_units);

			guint32 ms_size = 0;
			guchar* ms_frame = tlv_serialize_units(ms_units, &ms_size);

			if (ms_frame !=NULL && ms_size > 0)
			{
				tlv_add_unit(&ppc_units,tlv_create_unit(tspt_ModuleSettings , ms_frame, 0, ms_size));
				g_free(ms_frame);
			}

			tlv_delete_units(&ms_units);

			guint32 dc_size = 0;
			guchar* dc_frame = tlv_serialize_units(ppc_units, &dc_size);

			if (dc_frame !=NULL && dc_size > 0)
			{
				tlv_add_unit(&units,tlv_create_unit(tst_PpcConfig , dc_frame, 0, dc_size));
				g_free(dc_frame);
			}

			tlv_delete_units(&ppc_units);
		}
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

void send_system_sensor_controller_configuration_message(SockClientInfo* client_info, guint32 message_id, MessageType message_type)
{
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, TRUE, "%s Send system sensor controllers configuration message", client_info->device_name);
	add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s System sensor controllers configuration message:", client_info->device_name);

	TlvUnit* units = NULL;

	add_tlv_unit_32(&units, tst_MessageId, 	message_id, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		MessageId: %d", client_info->socket, message_id);
	add_tlv_unit_8( &units, tst_MessageType, message_type, 			client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Message type: %d (%s)", client_info->socket, message_type, return_message_type_name(message_type));
	add_tlv_unit_32(&units, tst_CommandCode, hsc_GetSensorControllerConfig, 	client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Command code: %d (%s)", client_info->socket, hsc_GetSensorControllerConfig, server_command_to_str(hsc_GetSensorControllerConfig));
	add_tlv_unit_8( &units, tst_Error, 		0, 						client_info->log_params, client_info->log_trace, client_info->log_parsing, "%d 		Error: 0x%02X", client_info->socket, 0);

	DeviceConfs device_confs = {0x00};
	get_device_confs(&device_confs);

	add_tlv_unit_8( &units, tst_ScCount, device_confs.sensor_controller_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Sensor controller count: %d ", client_info->device_name, device_confs.sensor_controller_count);

	if (device_confs.sensor_controller_count > 0)
	{
		for (guint8 i = 0; i < device_confs.sensor_controller_count; i++)
		{
			TlvUnit* sc_units = NULL;
			ScConf* sc_conf = &device_confs.sensor_controllers[i];
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 		Sensor controller: ", client_info->device_name);

			add_tlv_unit_8( &sc_units, tscst_Id, sc_conf->id, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Id: %d", client_info->device_name, sc_conf->id);
			add_tlv_unit_str(&sc_units, tscst_Name, sc_conf->name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Name: %s", client_info->device_name, sc_conf->name);
			add_tlv_unit_16(&sc_units, tscst_Port, sc_conf->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Port: %d", client_info->device_name, sc_conf->port);
			add_tlv_unit_8( &sc_units, tscst_Enable, sc_conf->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Enable: %d", client_info->device_name, sc_conf->enable);
			add_tlv_unit_32(&sc_units, tscst_CommandTimeout, sc_conf->command_timeout, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Command timeout: %d ", client_info->device_name, sc_conf->command_timeout);
			add_tlv_unit_32(&sc_units, tscst_Interval, sc_conf->interval, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		Interval: %d ", client_info->device_name, sc_conf->interval);
			add_tlv_unit_str( &sc_units, tscst_Module, sc_conf->module_name, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			Module: %s", client_info->device_name, sc_conf->module_name);
			add_tlv_unit_str( &sc_units, tscst_LogDir, sc_conf->log_dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogDir: %s", client_info->device_name, sc_conf->log_dir);
			add_tlv_unit_8( &sc_units, tscst_LogEnable, sc_conf->log_enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogEnable: %d", client_info->device_name, sc_conf->log_enable);
			add_tlv_unit_8( &sc_units, tscst_LogTrace, sc_conf->log_trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogTrace: %d", client_info->device_name, sc_conf->log_trace);
			add_tlv_unit_32(&sc_units, tscst_LogFileSize, sc_conf->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 			LogFileSize: %d", client_info->device_name,sc_conf->file_size);
			add_tlv_unit_32(&sc_units, tscst_LogSaveDays, sc_conf->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 		LogSaveDays: %d", client_info->device_name,sc_conf->save_days);

			//module settings
			TlvUnit* ms_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 			Module settings: ", client_info->device_name);

			SCLibConfig* module_config = &sc_conf->module_config;

			//module log settings
			TlvUnit* ms_log_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Log settings: ", client_info->device_name);

			LibLogOptions* log_options = &module_config->log_options;

			add_tlv_unit_8( &ms_log_units, tmlst_Enable, log_options->enable, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Enable: %d", client_info->device_name, log_options->enable);
			add_tlv_unit_str( &ms_log_units, tmlst_Directory,  log_options->dir, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Dir: %s", client_info->device_name, log_options->dir);
			add_tlv_unit_8( &ms_log_units, tmlst_Trace, log_options->trace, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Trace: %d", client_info->device_name, log_options->trace);
			add_tlv_unit_8( &ms_log_units, tmlst_System, log_options->system, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					System: %d", client_info->device_name, log_options->system);
			add_tlv_unit_8( &ms_log_units, tmlst_Requests, log_options->requests, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Requests: %d", client_info->device_name, log_options->requests);
			add_tlv_unit_8( &ms_log_units, tmlst_Frames, log_options->frames, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Frames: %d", client_info->device_name, log_options->frames);
			add_tlv_unit_8( &ms_log_units, tmlst_Parsing, log_options->parsing, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Parsing: %d", client_info->device_name, log_options->parsing);
			add_tlv_unit_32( &ms_log_units, tmlst_FileSize, log_options->file_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					File size: %d", client_info->device_name, log_options->file_size);
			add_tlv_unit_32( &ms_log_units, tmlst_SaveDays, log_options->save_days, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Save days: %d", client_info->device_name, log_options->save_days);

			guint32 ms_log_size = 0;
			guchar* ms_log_frame = tlv_serialize_units(ms_log_units, &ms_log_size);

			if (ms_log_frame !=NULL && ms_log_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tscmst_LogSettings , ms_log_frame, 0, ms_log_size));
				g_free(ms_log_frame);
			}
			tlv_delete_units(&ms_log_units);

			//module conn settings
			TlvUnit* ms_conn_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Connection settings: ", client_info->device_name);

			ConnOptions* conn_options = &module_config->conn_options;

			add_tlv_unit_8( &ms_conn_units, tmcst_Type, conn_options->connection_type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->connection_type);
			add_tlv_unit_str( &ms_conn_units, tmcst_Port,  conn_options->port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Port: %s", client_info->device_name, conn_options->port);
			add_tlv_unit_str( &ms_conn_units, tmcst_IPAddress,  conn_options->ip_address, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					IP Address: %s", client_info->device_name, conn_options->ip_address);
			add_tlv_unit_16( &ms_conn_units, tmcst_IPPort, conn_options->ip_port, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Type: %d", client_info->device_name, conn_options->ip_port);
			add_tlv_unit_32( &ms_conn_units, tmcst_UartBaudrate, conn_options->uart_baudrate, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart baudrate: %d", client_info->device_name, conn_options->uart_baudrate);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartByteSize, conn_options->uart_byte_size, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart byte size: %d", client_info->device_name, conn_options->uart_byte_size);
			add_tlv_unit_str( &ms_conn_units, tmcst_UartParity,  conn_options->uart_parity, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart parity: %s", client_info->device_name, conn_options->uart_parity);
			add_tlv_unit_8( &ms_conn_units, tmcst_UartStopBits, conn_options->uart_stop_bits, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Uart stop bits: %d", client_info->device_name, conn_options->uart_stop_bits);

			guint32 ms_conn_size = 0;
			guchar* ms_conn_frame = tlv_serialize_units(ms_conn_units, &ms_conn_size);

			if (ms_conn_frame !=NULL && ms_conn_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tscmst_ConnSettings , ms_conn_frame, 0, ms_conn_size));
				g_free(ms_conn_frame);
			}
			tlv_delete_units(&ms_conn_units);

			//module timeouts settings
			TlvUnit* ms_timeout_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Timeout settings: ", client_info->device_name);

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			add_tlv_unit_32( &ms_timeout_units, tmtst_Read, timeout_options->t_read, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Read: %d", client_info->device_name, timeout_options->t_read);
			add_tlv_unit_32( &ms_timeout_units, tmtst_Write, timeout_options->t_write, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Write: %d", client_info->device_name, timeout_options->t_write);

			guint32 ms_timeout_size = 0;
			guchar* ms_timeout_frame = tlv_serialize_units(ms_timeout_units, &ms_timeout_size);

			if (ms_timeout_frame !=NULL && ms_timeout_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tscmst_TimeoutSettings , ms_timeout_frame, 0, ms_timeout_size));
				g_free(ms_timeout_frame);
			}
			tlv_delete_units(&ms_timeout_units);

			//module mapping
			TlvUnit* ms_mapping_units = NULL;
			add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 				Mapping: ", client_info->device_name);

			add_tlv_unit_8( &ms_mapping_units, tscmmt_SensorCount, module_config->sensor_count, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 					Sensor count: %d", client_info->device_name, module_config->sensor_count);

			if (sc_conf->module_config.sensor_count > 0)
			{
				for (guint8 j = 0; j < sc_conf->module_config.sensor_count; j++)
				{
					SensorConf* sensor = &module_config->sensors[j];

					TlvUnit* sensor_units = NULL;
					add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 						Sensor:", client_info->device_name);

					add_tlv_unit_32(&sensor_units, tmsct_Number, sensor->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Number: %d", client_info->device_name, sensor->num);
					add_tlv_unit_8(&sensor_units, tmsct_Addr, sensor->addr, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 							Addr: %d", client_info->device_name, sensor->addr);

					if (sensor->param_count > 0)
					{
						for (guint8 k = 0; k < sensor->param_count; k++)
						{
							SensorParamConf* param = &sensor->params[k];

							TlvUnit* param_units = NULL;
							add_log(client_info->log_params, TRUE, TRUE, client_info->log_trace, client_info->log_parsing, "%s 							Param:", client_info->device_name);

							add_tlv_unit_8(&param_units, tmspct_Number, param->num, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 								Number: %d", client_info->device_name, param->num);
							add_tlv_unit_8(&param_units, tmspct_Type, param->type, client_info->log_params, client_info->log_trace, client_info->log_parsing, "%s 								Type: %d", client_info->device_name, param->type);


							guint32 param_size = 0;
							guchar* param_frame = tlv_serialize_units(param_units, &param_size);

							if (param_frame !=NULL && param_size > 0)
							{
								tlv_add_unit(&sensor_units,tlv_create_unit(tmsct_ParamConfiguration , param_frame, 0, param_size));
								g_free(param_frame);
							}
							tlv_delete_units(&param_units);

						}
					}

					guint32 sensor_size = 0;
					guchar* sensor_frame = tlv_serialize_units(sensor_units, &sensor_size);

					if (sensor_frame !=NULL && sensor_size > 0)
					{
						tlv_add_unit(&ms_mapping_units,tlv_create_unit(tscmmt_SensorConfiguration , sensor_frame, 0, sensor_size));
						g_free(sensor_frame);
					}
					tlv_delete_units(&sensor_units);

				}
			}

			guint32 ms_mapping_size = 0;
			guchar* ms_mapping_frame = tlv_serialize_units(ms_mapping_units, &ms_mapping_size);

			if (ms_mapping_frame !=NULL && ms_mapping_size > 0)
			{
				tlv_add_unit(&ms_units,tlv_create_unit(tscmst_ModuleMapping , ms_mapping_frame, 0, ms_mapping_size));
				g_free(ms_mapping_frame);
			}
			tlv_delete_units(&ms_mapping_units);

			guint32 ms_size = 0;
			guchar* ms_frame = tlv_serialize_units(ms_units, &ms_size);

			if (ms_frame !=NULL && ms_size > 0)
			{
				tlv_add_unit(&sc_units,tlv_create_unit(tspt_ModuleSettings , ms_frame, 0, ms_size));
				g_free(ms_frame);
			}

			tlv_delete_units(&ms_units);

			guint32 dc_size = 0;
			guchar* dc_frame = tlv_serialize_units(sc_units, &dc_size);

			if (dc_frame !=NULL && dc_size > 0)
			{
				tlv_add_unit(&units,tlv_create_unit(tst_ScConfig , dc_frame, 0, dc_size));
				g_free(dc_frame);
			}

			tlv_delete_units(&sc_units);
		}
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
