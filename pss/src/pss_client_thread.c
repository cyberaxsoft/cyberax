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


void parse_fc_change_param_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	guint16 pos = 0;

	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse fc_change_param_req:", params->client_index);

	guint8 fc_par_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FcParGroupId %02X (%s)",
			params->client_index, fc_par_group_id, fc_par_group_id_to_str(fc_par_group_id));

	switch (fc_par_group_id)
	{
		case fcpgid_GeneralFcParmeters:
			interpret_change_fc_parameters(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_ServiceModes:
			interpret_change_service_mode_parameters(&buffer[pos], params,ex_mode);
			break;

		case fcpgid_FuellingModes:
			interpret_change_fuelling_mode_parameters(&buffer[pos], params,ex_mode);
			break;

		case fcpgid_GradeTexts:
			interpret_change_grade_texts(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_GlobalFuellingLimits:
			interpret_change_global_fuelling_limits_parameters(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_FuellingModeGroups:
			interpret_change_fuelling_mode_group_parameters(&buffer[pos], params, ex_mode);
			break;

	}

}

void parse_fc_param_set_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	guint16 pos = 0;

	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse fc_param_set_req:", params->client_index);

	guint8 fc_par_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FcParGroupId %d (%s)",
			params->client_index, fc_par_group_id, fc_par_group_id_to_str(fc_par_group_id));

	switch (fc_par_group_id)
	{
		case fcpgid_GeneralFcParmeters:
			interpret_general_fc_parameters_request(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_ServiceModes:
			interpret_service_mode_fc_parameters_request(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_FuellingModes:
			interpret_fuelling_mode_fc_parameters_request(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_GradeTexts:
			interpret_grade_texts_fc_parameters_request(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_GlobalFuellingLimits:
			interpret_global_fuelling_limits_request(&buffer[pos], params, ex_mode);
			break;

		case fcpgid_FuellingModeGroups:
			interpret_fuelling_mode_group_param_request(&buffer[pos], params, ex_mode);
			break;

	}

}

guint16 interpret_header(guint8* buffer, guint16 length, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	pos++;			//SOH

	guint8 header_version = parse_BIN8(&buffer[pos]); pos+=2;
	guint8 encoding_type = parse_BIN8(&buffer[pos]); pos+=2;
	guint8 checksum = parse_BIN8(&buffer[pos]); pos+=2;
	guint16 datalength = parse_BIN16(&buffer[pos]); pos+=4;


	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parsing header: version = %d, encoding_type = %d, checksum = 0x%02X, datalength %02d",
			params->client_index, port, header_version, encoding_type, checksum, datalength);

	pos++;  		//STX


	return pos;
}

void interpret_command(guint32 port, guint8 code, guint8 subcode, gboolean exmode, guint8* buffer, guint16 length, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;

	switch(code)
	{
		case pssc_FcLogon_req:
			switch (subcode)
			{
				case 0x06:
					parse_install_tg_ext_sub_dev(buffer, params, exmode);
					break;

				case 0x00:
					parse_fc_logon_req(buffer, params, exmode);
					break;
			}
			break;

		case pssc_DiffComm:
			switch (subcode)
			{
				case 0x06:
					parse_change_tgs_tg_parametars(buffer, params, exmode);
					save_pss_data();
					break;

				case 0x07:
					parse_fp_install_data_req(buffer, params, exmode);
					break;

				case 0x00:
					send_fc_set_date_time(buffer, params, exmode);
					break;
			}
			break;

		case pssc_FcDateAndTime_req:
			switch (subcode)
			{
				case 0x06:
					parse_tgs_tg_parametars_req(buffer, params, exmode);
					break;

				case 0x00:
					send_fc_date_time_reply(params, FALSE);
					break;
			}
			break;


		case pssc_FcStatus_req:
			send_fc_status_reply(params, FALSE);
			break;

		case pssc_change_FcParameters:
			parse_fc_change_param_req(buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_FcParameterSet_req:
			parse_fc_param_set_req(buffer, params, exmode);
			break;

		case pssc_FcInstallStatus_req:
			send_fc_install_status_req(params, exmode);
			break;

		case pssc_FcPriceSetStatus_req:
			send_fc_price_set_status_req(params, exmode);
			break;

		case pssc_FcOperationModeStatus_req:
			send_fc_operation_mode_status(params, exmode);
			break;

		case pssc_load_FcPriceSet:
			parse_load_fc_price_set(buffer, params, subcode, exmode);
			save_pss_data();
			break;

		case pssc_FcPriceSet_req:
			parse_fc_price_set_req(buffer, params, subcode, exmode);
			break;

		case pssc_set_FcOperationModeNo:
			parse_set_fc_operation_mode_no(buffer, params, subcode, exmode);
			break;

		case pssc_clr_install_data:
			parse_clr_install_data(subcode, buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_open_fp:
			parse_open_fp(buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_close_fp:
			parse_close_fp(buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_authorize_Fp:
			parse_authorize_fp(subcode, buffer, params, exmode);
			break;

		case pssc_install_Fp:

			if (subcode == 0x04)
			{
				send_pin_status(params, get_door_switch());
			}
			else
			{
				parse_install_fp(subcode, buffer, params, exmode);
				save_pss_data();
			}
			break;

		case pssc_FpStatus_req:
			parse_fp_status_req(subcode, buffer, params, exmode);
			break;

		case pssc_FpSupTransBufStatus_req:
			parse_fp_sup_trans_buffer_status_req(subcode, buffer, params, exmode);
			break;

		case pssc_FpInfo_req:
			parse_fp_info_req(subcode, buffer, params, exmode);
			break;

		case pssc_FpFuellingData_req:
			parse_fp_fuelling_data_req(subcode, buffer, params, exmode);
			break;

		case pssc_estop_Fp:
			parse_estop_fp(subcode, buffer, params, exmode);
			break;

		case pssc_cancel_estop_Fp:
			parse_cancel_estop_fp(subcode, buffer, params, exmode);
			break;

		case pssc_reset_Fp:
			parse_reset_fp(subcode, buffer, params, exmode);
			break;

		case pssc_FpSupTrans_req:
			parse_fp_sup_trans_req(subcode, buffer, params, exmode);
			break;

		case pssc_clr_FpSupTrans:
			parse_clr_fp_sup_trans(subcode, buffer, params, exmode);
			break;

		case pssc_load_FpOperationModeSet:
			parse_load_fp_operation_mode_set( buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_FpGradeTotals_req:
			parse_fp_grade_totals_req_set(subcode, buffer, params, exmode);
			break;

		case pssc_PumpGradeTotals_req:
			parse_pump_grade_totals_req_set(subcode, buffer, params, exmode);
			break;


		case pssc_PssPeripheralsStatus_req:
			send_pss_peripheral_status(params, exmode);
			break;

		case pssc_PosConnectionStatus_req:
			send_pos_connection_status(params, exmode);
			break;

		//------------------------------------- Price pole ------------------------------

		case pssc_install_Pp:
			parse_install_price_pole(subcode, buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_PpStatus_req:
			parse_price_pole_status_req(subcode, buffer, params, exmode);
			break;

		case pssc_load_PpOperationModeSet:
			parse_load_pp_operation_mode_set(subcode, buffer, params, exmode);
			break;

		case pssc_open_Pp:
			parse_open_pp(subcode, buffer, params, exmode);
			break;

		case pssc_close_Pp:
			parse_close_pp(subcode, buffer, params, exmode);
			break;

		case pssc_reset_Pp:
			parse_reset_pp(subcode, buffer, params, exmode);
			break;


		//------------------------------------- Tank Gauge ------------------------------

		case pssc_install_TankGauge:
			parse_install_tank_gauge(subcode,  buffer, params, exmode);
			save_pss_data();
			break;

		case pssc_TgData_Req:
			parse_tg_data_req(subcode, buffer, params, exmode);
			break;

		case pssc_TgStatus_Req:
			parse_tg_status(subcode, buffer, params, exmode);
			break;

		default:
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d UNDEFINED COMMAND code-subcode %02X-%02X",params->client_index, code, subcode);
			send_rejected_reply(params, FALSE, code, subcode);
			break;
	}

}


void interpret_client_request(guint8* buffer, guint16 length, PSSClientThreadFuncParam* params)
{
	guint32 port = get_client_port(params->client_index);

	guint16 pos = interpret_header(buffer, length, params, port);

	LogParams* log_params = params->log_params;

	guint8 apc_code = parse_BIN8(&buffer[pos]); 	pos+=2;
	guint8 code = parse_BIN8(&buffer[pos]); 		pos+=2;
	guint8 subcode = parse_BIN8(&buffer[pos]); 		pos+=2;
	guint8 extsubcode = 0;
	gboolean exmode = FALSE;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parsing data APC %02d:",params->client_index, apc_code);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d code-subcode %02X-%02X",params->client_index, code, subcode);

	if (code == pssc_ExtendedFunc)
	{
		code = subcode;
		subcode = parse_BIN8(&buffer[pos]); pos+=2;
		extsubcode = parse_BIN8(&buffer[pos]); pos+=2;
		exmode = TRUE;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	extsubcode %02X",params->client_index, extsubcode);
	}

	interpret_command(port, code, subcode, exmode, &buffer[pos], length, params);

}

gpointer client_read_thread_func(gpointer data)
{
	guint8 bufrd[EXCHANGE_BUFFER_SIZE];
	guint16 pos_bufrd = 0;

	guchar bufr[SOCKET_BUFFER_SIZE];

	PSSClientThreadFuncParam param = *(PSSClientThreadFuncParam*)data;

	LogParams* log_params = param.log_params;

	guint32 clietn_sock = get_client_sock(param.client_index);

	add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d read thread started", param.client_index);

	PSSExchangeState es = psses_WaitSOH;

	while(get_client_state(param.client_index) == cs_Active )
	{
		int bytes_read = recv(clietn_sock, bufr, 1024, 0);
		if(bytes_read <= 0)
		{
			break;
		}
		else
		{
			for (guint32 i = 0; i < bytes_read; i++)
			{
				switch (es)
				{
					case psses_WaitSOH:
						if (bufr[i] == ctrl_SOH)
						{
							pos_bufrd = 0;
							bufrd[pos_bufrd++] = bufr[i];
							es = psses_WaitSTX;
						}
						break;

					case psses_WaitSTX:
						if (bufr[i] == ctrl_SOH)
						{
							pos_bufrd = 0;
							bufrd[pos_bufrd++] = bufr[i];
							es = psses_WaitSTX;
						}
						else
						{
							if (pos_bufrd < EXCHANGE_BUFFER_SIZE)
							{
								bufrd[pos_bufrd++] = bufr[i];
								if (bufr[i] == ctrl_STX)
								{
									es = psses_WaitETX;
								}
							}
							else
							{
								es = psses_WaitSOH;
							}
						}
						break;

					case psses_WaitETX:
						if (pos_bufrd < EXCHANGE_BUFFER_SIZE)
						{
							bufrd[pos_bufrd++] = bufr[i];
							if (bufr[i] == ctrl_ETX)
							{
								if (pos_bufrd > PSS_DATALENGTH_OFFSET + 1)
								{

									add_log(log_params, TRUE, FALSE, param.log_trace, param.log_enable, "client %d <<", param.client_index);

									for (guint16 i = 0; i < pos_bufrd; i++)
									{
										add_log(log_params, FALSE, FALSE, param.log_trace, param.log_enable, " %02X", bufrd[i]);
									}
									add_log(log_params, FALSE, TRUE, param.log_trace, param.log_enable, "");

									if (parse_BIN16(&bufrd[PSS_DATALENGTH_OFFSET]) > 0)
									{
										interpret_client_request(bufrd, pos_bufrd, &param);
									}

									set_client_last_exchange_time( param.client_index, get_date_time());

								}
								es = psses_WaitSOH;
							}
						}
						else
						{
							es = psses_WaitSOH;
						}
						break;
				}
			}
		}
	}

	client_socket_close(param.client_index);
	set_client_state(param.client_index, cs_Free);

	add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d destroyed", param.client_index);

	return NULL;
}

gpointer client_thread_func(gpointer data)
{
	PSSClientThreadFuncParam param = *(PSSClientThreadFuncParam*)data;

	guint32 port = get_client_port(param.client_index);

	LogParams* log_params = param.log_params;

	add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d starting client thread", param.client_index);

	connect_to_server_devices(&param);

	GThread* client_read_thread = g_thread_new("client_read_thread", client_read_thread_func, &param);

	if (client_read_thread == NULL)
	{
		add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d error starting read thread", param.client_index);
	}
	else
	{
		client_read_thread->priority = G_THREAD_PRIORITY_LOW;
	}

	guint64 timer = 0;

	while( get_client_state(param.client_index) == cs_Active )
	{
		switch (get_client_exchange_status(param.client_index))
		{
			case ces_Undefined:
				break;

			case ces_Logged:
				send_fc_status_reply(&param, FALSE);
				set_client_exchange_status(param.client_index, ces_FcStatus);
				break;

			case ces_FcStatus:
				send_fc_install_status_req(&param, FALSE);

				if (get_client_type(param.client_index) == ct_FPControl || get_client_type(param.client_index) == ct_Undefined)
				{
					set_client_exchange_status(param.client_index, ces_FcInstallStatus);
				}
				else
				{
					set_client_exchange_status(param.client_index, ces_TgsStatus);
				}

				break;

			case ces_FcInstallStatus:
				if (get_fuelling_point_count() > 0)
				{
					switch(get_client_fp_status_mode(param.client_index))
					{
						case 1:
							send_fp_status_req_1_mult(&param, FALSE);
							break;

						case 2:
							send_fp_status_req_2_mult(&param, FALSE);
							break;

						case 3:
							send_fp_status_req_3_mult(&param, FALSE);
							break;
					}

				}
				set_client_exchange_status(param.client_index, ces_FpStatus);
				break;

			case ces_FpStatus:
				if (get_fuelling_point_count() > 0)
				{
					switch(get_client_tr_buf_status_mode(param.client_index))
					{
						case 1:
							send_fp_sup_trans_buffer_status_req_1_mult(&param, FALSE, port);
							break;

						case 2:
							send_fp_sup_trans_buffer_status_req_1_mult(&param, FALSE, port);
							break;

						case 3:
							send_fp_sup_trans_buffer_status_req_3_mult(&param, FALSE, port);
							break;
					}



				}
				set_client_exchange_status(param.client_index, ces_FpSupTransBufStatus);
				break;

			case ces_FpSupTransBufStatus:
				send_fc_price_set_status_req(&param, FALSE);

				if (get_client_type(param.client_index) == ct_Undefined)
				{
					set_client_exchange_status(param.client_index, ces_TgsStatus);
				}
				else
				{
					set_client_exchange_status(param.client_index, ces_FcPriceSetStatus);
				}
				break;

			case ces_TgsStatus:
				send_tg_status_req_1_mult(&param, TRUE);
				set_client_exchange_status(param.client_index, ces_FcPriceSetStatus);
				break;

			case ces_FcPriceSetStatus:
				send_fc_operation_mode_status(&param, FALSE);
				set_client_exchange_status(param.client_index, ces_FcOperationModeStatus);
				break;

			case ces_FcOperationModeStatus:
				send_pos_connection_status(&param, FALSE);
				set_client_exchange_status(param.client_index, ces_FcPosConnectionStatus);
				break;

			case ces_FcPosConnectionStatus:
				send_pss_peripheral_status(&param, FALSE);
				set_client_exchange_status(param.client_index, ces_DoorSwitch);
				break;

			case ces_DoorSwitch:
				send_pin_status(&param, FALSE);
				set_client_exchange_status(param.client_index, ces_Idle);
				break;

			case ces_Idle:
				if (get_date_time() > timer + PSS_HEARTBEAT_TIMEOUT )
				{
					add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d send heartbeat frame", param.client_index);
					socket_client_send_with_mutex(&param, NULL,0);
					timer = get_date_time();
				}

				if (get_client_last_exchange_time(param.client_index) + (PSS_HEARTBEAT_TIMEOUT * 3) < get_date_time())
				{
					set_client_state(param.client_index, cs_Destroying);
				}

				break;
		}
	}

	disconnect_server_devices(&param);

	g_thread_join(client_read_thread);

	add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "client %d disconnected socket", param.client_index);

	clear_client(param.client_index);

	return NULL;
}
