#include <glib.h>
#include <glib/gstdio.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_data.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_func.h"

void send_rejected_reply(PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 code, guint8 subcode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d rejected prepare data:", params->client_index);

	guint8 buffer[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = 0;

	buffer[pos++] = port_to_apc_code(port);
	buffer[pos++] = pssc_Rejected;
	buffer[pos++] = 0x00;

	buffer[pos++] = code;
	buffer[pos++] = subcode;

	buffer[pos++] = 0x00;
	buffer[pos++] = 0x00;

	socket_client_send_with_mutex(params, buffer,pos);
}

void send_pin_status(PSSClientThreadFuncParam* params, gboolean status)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d pin status prepare data:", params->client_index);

	guint8 buffer[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = 0;

	buffer[pos++] = port_to_apc_code(port);
	buffer[pos++] = 0x00;
	buffer[pos++] = 0x10;
	buffer[pos++] = 0x84;

	buffer[pos++] = 0x00;

	pos+=add_bcd_field(&buffer[pos], 1, 1);

	buffer[pos++] = 0x01;

	pos+=add_bcd_field(&buffer[pos], 1, 1);

	buffer[pos++] = 0x01;

	if (status)
	{
		buffer[pos++] = 0x01;
	}
	else
	{
		buffer[pos++] = 0x00;
	}

	socket_client_send_with_mutex(params, buffer,pos);
}

void send_fc_date_time_reply(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d fc_status_reply prepare data:", params->client_index);

	guint8 buffer[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(buffer, port, ex_mode, pssc_FcDateAndTime_req, pssc_FcDateAndTime, 0x00);

	struct tm* timeinfo;
	struct timeval tv;
	gettimeofday (&tv, NULL);

	time_t rawtime;
	time (&rawtime);
	timeinfo = localtime(&rawtime);

	pos+=add_date_time_field(&buffer[pos], timeinfo);
	pos+=add_date_time_field(&buffer[pos], timeinfo);

	socket_client_send_with_mutex(params, buffer,pos);
}

void send_fc_status_reply(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d fc_status_reply prepare data:", params->client_index);

	guint8 buffer[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(buffer, port, ex_mode, pssc_FcStatus_req, pssc_FcStatus, 0x00);

	PSSFcStatus status = {0x00};
	get_fc_status(&status);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_status_1_flags = 0x%02X", params->client_index, status.fc_status_1_flags);
	buffer[pos++] = status.fc_status_1_flags;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_status_2_flags = 0x%02X", params->client_index, status.fc_status_2_flags);
	buffer[pos++] = status.fc_status_2_flags;

	buffer[pos++] = status.fc_service_msg_seq_no;

	struct tm tm_master_reset = *localtime(&status.fc_master_reset_date_and_time);

	pos+=add_date_time_field(&buffer[pos], &tm_master_reset);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_master_reset_date_and_time: %04d.%02d.%02d %02d:%02d:%02d",
			params->client_index,tm_master_reset.tm_year + 1900, tm_master_reset.tm_mon + 1, tm_master_reset.tm_mday, tm_master_reset.tm_hour,
			tm_master_reset.tm_min, tm_master_reset.tm_sec);

	buffer[pos++] = status.fc_master_reset_code;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_master_reset_code: 0x%02X", params->client_index, status.fc_master_reset_code);

	struct tm tm_reset = *localtime(&status.fc_reset_date_and_time);

	pos+=add_date_time_field(&buffer[pos], &tm_reset);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_reset_date_and_time: %04d.%02d.%02d %02d:%02d:%02d",
			params->client_index,tm_reset.tm_year + 1900, tm_reset.tm_mon + 1, tm_reset.tm_mday, tm_reset.tm_hour,tm_reset.tm_min, tm_reset.tm_sec);

	buffer[pos++] = status.fc_reset_code;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_reset_code: 0x%02X", params->client_index, status.fc_reset_code);

	socket_client_send_with_mutex(params, buffer,pos);
}

void parse_fc_logon_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse fc_logon_req:", params->client_index);

	guint8 access_code[MAX_STRING_LENGTH] = {0x00};
	guint8 pos_version_id[MAX_STRING_LENGTH] = {0x00};

	guint16 pos = 0;

	pos+=parse_pss_ascii(access_code, &buffer[pos], MAX_STRING_LENGTH);

	guint16 PosCountryCode = parse_BIN16(&buffer[pos]); pos+=4;

	//guint8 NoPosVersionIdBytes = parse_BIN8(&buffer[pos]); pos+=2;
	pos+=parse_pss_ascii(pos_version_id, &buffer[pos], MAX_STRING_LENGTH);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	logon data: access_code = %s, PosCountryCode = %02d,  pos_version_id = %s",
			params->client_index, access_code, PosCountryCode, pos_version_id);

	if (strcmp((gchar*)access_code, "UNSO_TGSTA_1") == 0)
	{
		set_client_tgs_status_mode(params->client_index, 1);
	}

	if (strcmp((gchar*)access_code, "UNSO_FPSTA_1") == 0)
	{
		set_client_fp_status_mode(params->client_index, 1);
	}

	if (strcmp((gchar*)access_code, "UNSO_TRBUFSTA_1") == 0)
	{
		set_client_tr_buf_status_mode(params->client_index, 1);
	}

	if (strcmp((gchar*)access_code, "MASTER-RESET") == 0)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d MASTER-RESET", params->client_index);

		master_reset();
		update_fc_master_reset_date_and_time(time(NULL));

		send_fc_status_reply(params, ex_mode);
	}
	else if (strcmp((gchar*)access_code, "POS") > 0 || strcmp((gchar*)access_code, "NamosPos") > 0)
	{
		if ( strcmp((gchar*)access_code, "UNSO_FPSTA") > 0)
		{
			set_client_type(params->client_index, ct_FPControl);

			if (strcmp((gchar*)access_code, "UNSO_TGSTA") > 0)
			{
				set_client_type(params->client_index, ct_Undefined);
			}
		}
		else if (strcmp((gchar*)access_code, "UNSO_TGSTA") > 0)
		{
			set_client_type(params->client_index, ct_TGSControl);

			if ( strcmp((gchar*)access_code, "UNSO_FPSTA") > 0)
			{
				set_client_type(params->client_index, ct_Undefined);
			}
		}

		switch (port)
		{
			case SUPERVISED_PORT:
			case UNSUPERVISED_PORT:
			case UNSUPERVISED_MESSAGES_PORT:
			case FALLBACK_CONSOLE:
			case MSG_PAYMENT_SERVER:
			case CONTROL_PAYMENT_SERVER:
			case PIN_PAD_INTERFACE:
			case REMOTE_LOG_INTERFACE:
				set_client_exchange_status(params->client_index, ces_Idle);
				break;

			case SUPERVISED_MESSAGES_PORT:
				set_client_exchange_status(params->client_index, ces_Logged);
				break;

		}

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FcLogon_req frame:", params->client_index);

		guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
		guint16 pos_repl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcLogon_req, pssc_FcLogon_ack, 0x00);

		reply[pos_repl++] = PSS_COUNTRY_CODE >> 8;
		reply[pos_repl++] = PSS_COUNTRY_CODE & 0xFF;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PssCountryCode: %d:", params->client_index,PSS_COUNTRY_CODE);

		reply[pos_repl++] = FC_HW_TYPE >> 8;
		reply[pos_repl++] = FC_HW_TYPE & 0xFF;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		HwType: %d:", params->client_index,FC_HW_TYPE);

		reply[pos_repl++] = FC_HW_VERSION_NUMBER;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		HwVersionNumber: %d:", params->client_index,FC_HW_VERSION_NUMBER);

		reply[pos_repl++] = FC_SW_TYPE >> 8;
		reply[pos_repl++] = FC_SW_TYPE & 0xFF;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SwType: %d:", params->client_index,FC_SW_TYPE);

		reply[pos_repl++] = 0x00;
		reply[pos_repl++] = 0x00;
		reply[pos_repl++] = 0x00;

		reply[pos_repl++] = FC_SW_VERSION_NUMBER >> 24;
		reply[pos_repl++] = (FC_SW_VERSION_NUMBER >> 16) & 0xFF;
		reply[pos_repl++] = (FC_SW_VERSION_NUMBER >> 8) & 0xFF;
		reply[pos_repl++] = FC_SW_VERSION_NUMBER & 0xFF;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SwVersionNumber: %d:", params->client_index,FC_SW_VERSION_NUMBER);

		reply[pos_repl++] = FC_SW_DATETIME >> 24;
		reply[pos_repl++] = (FC_SW_DATETIME >> 16) & 0xFF;
		reply[pos_repl++] = (FC_SW_DATETIME >> 8) & 0xFF;;
		reply[pos_repl++] = FC_SW_DATETIME & 0xFF;;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SwDateTime: %d:", params->client_index,FC_SW_DATETIME);

		reply[pos_repl++] = 0x01;
		reply[pos_repl++] = 0x02;
		reply[pos_repl++] = 0x02;
		reply[pos_repl++] = 0x05;
		reply[pos_repl++] = 0x53;
		reply[pos_repl++] = 0x2a;
		reply[pos_repl++] = 0xb9;

		socket_client_send_with_mutex(params, reply,pos_repl);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d LOGON", params->client_index);

	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d FAULT CODE", params->client_index);
	}
}

void send_pss_peripheral_status(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d pss_peripheral_status prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_PssPeripheralsStatus_req, pssc_PssPeripheralStatus, 0x00);

	reply[pos++] = 0x00;

	socket_client_send_with_mutex(params, reply,pos);
}

void send_pos_connection_status(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d pos_connection_status prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_PosConnectionStatus_req, pssc_PosConnectionStatus, 0x00);

	guint8 no_connection_pos = pos;
	guint8 no_connection = 0;

	reply[pos++] = 0x00;

	for (guint8 i = 0; i < PSS_MAX_CLIENT_COUNT; i++)
	{
		if (get_client_state(i) == cs_Active)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		client %d:", params->client_index, i);

			guint32 client_port = get_client_port(i);

			reply[pos++] = return_pos_device_type(client_port);
			reply[pos++] = 0x02; //connection_type tcp/ip
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			port: %d (TCP/IP)", params->client_index, client_port);

			//ip address
			guint32 ip_address = get_client_ip_address(i);

			reply[pos++] = ip_address & 0xFF;
			reply[pos++] = (ip_address >> 8 ) & 0xFF;
			reply[pos++] = (ip_address >> 16 ) & 0xFF;
			reply[pos++] = (ip_address >> 24 ) & 0xFF;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			address: %d.%d.%d.%d",
					params->client_index,ip_address & 0xFF, (ip_address >> 8 ) & 0xFF, (ip_address >> 16 ) & 0xFF ,(ip_address >> 24 ) & 0xFF);

			//port
			reply[pos++] = client_port & 0xFF;
			reply[pos++] = (client_port >> 8 ) & 0xFF;

			reply[pos++] = 0x04; //online
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Online", params->client_index);

			no_connection++;
		}
	}
	reply[no_connection_pos] = no_connection;

	socket_client_send_with_mutex(params, reply,pos);
}

void send_fc_price_set_status_req(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d fc_price_set_status prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcPriceSetStatus_req, pssc_FcPriceSetStatus, 0x00);

	PSSFcStatus status = {0x00};
	get_fc_status(&status);

	pos+=add_bcd_field(&reply[pos], status.fc_price_set_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FcPriceSetID: %d", params->client_index,status.fc_price_set_id);

	struct tm tm_sp = *localtime(&status.fc_price_set_date_and_time);

	pos+=add_date_time_field(&reply[pos], &tm_sp);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_price_set_date_and_time: %04d.%02d.%02d %02d:%02d:%02d",
			params->client_index,tm_sp.tm_year + 1900, tm_sp.tm_mon + 1, tm_sp.tm_mday,tm_sp.tm_hour,tm_sp.tm_min, tm_sp.tm_sec);

	socket_client_send_with_mutex(params, reply,pos);

}

void send_fc_operation_mode_status(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d fc_operation_mode_status prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcOperationModeStatus_req, pssc_FcOperationModeStatus, 0x00);

	PSSFcStatus status = {0x00};
	get_fc_status(&status);

	reply[pos++] = status.fc_operation_mode_no;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		fc_operation_mode_no: %d", params->client_index, status.fc_operation_mode_no);

	struct tm tm_sp = *localtime(&status.fc_operation_mode_date_and_time);

	pos+=add_date_time_field(&reply[pos], &tm_sp);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		fc_operation_mode_date_and_time: %04d.%02d.%02d %02d:%02d:%02d",
			params->client_index, tm_sp.tm_year + 1900, tm_sp.tm_mon + 1, tm_sp.tm_mday, tm_sp.tm_hour,tm_sp.tm_min, tm_sp.tm_sec);

	socket_client_send_with_mutex(params,reply,pos);
}


void send_fc_install_status_req(PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d FcInstallStatus prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcInstallStatus_req, pssc_FcInstallStatus, 0x00);

	guint8 device_group_count = get_device_groups_count();

	reply[pos++] = device_group_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	device group count: %d:", params->client_index, device_group_count);

	guint8 fuelling_point_count = get_fuelling_point_count();

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fuelling point count: %d:", params->client_index, fuelling_point_count);

	if (fuelling_point_count > 0)
	{
		reply[pos++] = FUELLING_POINT_INSTALL_MSG_CODE;

		reply[pos++] = fuelling_point_count;

		for(guint8 i = 0; i < fuelling_point_count; i++)
		{
			pos+=add_bcd_field(&reply[pos], get_fuelling_point_id_by_index(i), 1);
		}

	}

	guint8 price_pole_count = get_price_pole_count();

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	price pole count: %d:", params->client_index, price_pole_count);

	if (price_pole_count > 0)
	{
		reply[pos++] = PRICE_POLE_INSTALL_MSG_CODE;

		reply[pos++] = price_pole_count;

		for(guint8 i = 0; i < price_pole_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],get_price_pole_id_by_index(i), 1);
		}

	}

	guint8 tank_gauge_count = get_tank_gauge_count();

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	tank gauge count: %d:", params->client_index, tank_gauge_count);

	if (tank_gauge_count > 0)
	{
		reply[pos++] = TANK_GAUGE_INSTALL_MSG_CODE;

		reply[pos++] = tank_gauge_count;

		for(guint8 i = 0; i < tank_gauge_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],get_tank_gauge_id_by_index(i), 1);
		}

	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_grade_totals_set(guint8 comm,guint8 reply_comm,guint8 subcode, guint8 fp_id, PSSClientThreadFuncParam* params, guint8 ex_mode)
{
	LogParams* log_params = params->log_params;
	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpGradeTotals data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, comm, reply_comm, subcode);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos], fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index,fp_id);

	pos+=add_bcd_field(&reply[pos], fp.volume_total, 6);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Volume total: %d", params->client_index,fp.volume_total);

	pos+=add_bcd_field(&reply[pos], fp.money_total, 6);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Money total: %d", params->client_index,fp.money_total);

	reply[pos++] = fp.grade_option_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grade_option_count: %d", params->client_index, fp.grade_option_count);

	if (fp.grade_option_count > 0)
	{
		for (guint8 i = 0; i < fp.grade_option_count; i++)
		{
			reply[pos++] = fp.grade_options[i].id;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grade_option_id: %d", params->client_index, fp.grade_options[i].id);

			pos+=add_bcd_field(&reply[pos], fp.grade_options[i].grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		grade_id: %d", params->client_index, fp.grade_options[i].grade_id);

			pos+=add_bcd_field(&reply[pos], fp.grade_options[i].volume_total, 6);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		VolumeTotal: %d", params->client_index, fp.grade_options[i].volume_total);

			if (subcode > 0)
			{
				pos+=add_bcd_field(&reply[pos], fp.grade_options[i].money_total, 6);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MoneyTotal: %d", params->client_index, fp.grade_options[i].money_total);
			}
		}
	}
	socket_client_send_with_mutex(params,reply,pos);

}

void send_fp_sup_trans_buffer_status_req_0(guint8 fp_id, PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus0 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x00);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos], fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	reply[pos++] = fp.sup_transactions.count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransactionCount: %d", params->client_index, fp.sup_transactions.count);

	if (fp.sup_transactions.count > 0)
	{
		for (guint8 i = 0; i < fp.sup_transactions.count; i++)
		{
			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_seq_no, 2);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		trans_seq_no: %d", params->client_index, fp.sup_transactions.units[i].trans_seq_no);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			sm_id: %d",	params->client_index, fp.sup_transactions.units[i].sm_id);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_lock_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			trans_lock_id: %d", params->client_index, fp.sup_transactions.units[i].trans_lock_id);

			reply[pos++] = fp.sup_transactions.units[i].flags;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			flags: %d",	params->client_index, fp.sup_transactions.units[i].flags);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].money, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			money: %d", params->client_index, fp.sup_transactions.units[i].money);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].volume, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			volume: %d", params->client_index, fp.sup_transactions.units[i].volume);
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_sup_trans_buffer_status_req_1(guint8 fp_id, PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus1 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x01);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos], fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	reply[pos++] = fp.sup_transactions.count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransactionCount: %d", params->client_index, fp.sup_transactions.count);

	if (fp.sup_transactions.count > 0)
	{
		for (guint8 i = 0; i < fp.sup_transactions.count; i++)
		{
			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_seq_no, 2);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		trans_seq_no: %d", params->client_index, fp.sup_transactions.units[i].trans_seq_no);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			sm_id: %d", params->client_index, fp.sup_transactions.units[i].sm_id);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_lock_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			trans_lock_id: %d", params->client_index, fp.sup_transactions.units[i].trans_lock_id);

			reply[pos++] = fp.sup_transactions.units[i].flags;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			flags: %d", params->client_index, fp.sup_transactions.units[i].flags);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].money, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			money: %d", params->client_index, fp.sup_transactions.units[i].money);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].volume, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			volume: %d", params->client_index, fp.sup_transactions.units[i].volume);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			GradeId: %d", params->client_index, fp.sup_transactions.units[i].grade_id);
		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_sup_trans_buffer_status_req_3(guint8 fp_id, PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus3 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x03);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos], fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	reply[pos++] = fp.sup_transactions.count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransactionCount: %d", params->client_index, fp.sup_transactions.count);

	if (fp.sup_transactions.count > 0)
	{
		for (guint8 i = 0; i < fp.sup_transactions.count; i++)
		{
			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_seq_no, 2);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		trans_seq_no: %d", params->client_index, fp.sup_transactions.units[i].trans_seq_no);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			sm_id: %d", params->client_index, fp.sup_transactions.units[i].sm_id);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].trans_lock_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			trans_lock_id: %d", params->client_index, fp.sup_transactions.units[i].trans_lock_id);

			reply[pos++] = fp.sup_transactions.units[i].flags;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			flags: %d", params->client_index, fp.sup_transactions.units[i].flags);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].money, 5);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			money: %d", params->client_index, fp.sup_transactions.units[i].money);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].volume, 5);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			volume: %d", params->client_index, fp.sup_transactions.units[i].volume);

			pos+=add_bcd_field(&reply[pos], fp.sup_transactions.units[i].grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			GradeId: %d", params->client_index, fp.sup_transactions.units[i].grade_id);
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_sup_trans_buffer_status_req_0_mult( PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus0 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x00);

	guint8 fp_count = get_fuelling_point_count();

	reply[pos++] = fp_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_count: %d", params->client_index, fp_count);

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos], fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

			fp_frame[fppos++] = fp.sup_transactions.count;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransactionCount: %d", params->client_index, fp.sup_transactions.count);

			if (fp.sup_transactions.count > 0)
			{
				for (guint8 j = 0; j < fp.sup_transactions.count; j++)
				{
					fppos+=add_bcd_field(&fp_frame[fppos], fp.sup_transactions.units[j].trans_seq_no, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		trans_seq_no: %d", params->client_index, fp.sup_transactions.units[i].trans_seq_no);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].sm_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			sm_id: %d", params->client_index, fp.sup_transactions.units[i].sm_id);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].trans_lock_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			trans_lock_id: %d", params->client_index, fp.sup_transactions.units[i].trans_lock_id);

					fp_frame[fppos++] = fp.sup_transactions.units[j].flags;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			flags: %d", params->client_index, fp.sup_transactions.units[i].flags);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].money, 3);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			money: %d", params->client_index, fp.sup_transactions.units[i].money);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].volume, 3);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			volume: %d", params->client_index, fp.sup_transactions.units[i].volume);
				}
			}

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;

		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_sup_trans_buffer_status_req_1_mult(PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus1 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x01);

	guint8 fp_count = get_fuelling_point_count();

	reply[pos++] = fp_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_count: %d", params->client_index, fp_count);

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos], fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

			fp_frame[fppos++] = fp.sup_transactions.count;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransactionCount: %d", params->client_index, fp.sup_transactions.count);

			if (fp.sup_transactions.count > 0)
			{
				for (guint8 j = 0; j < fp.sup_transactions.count; j++)
				{
					fppos+=add_bcd_field(&fp_frame[fppos], fp.sup_transactions.units[j].trans_seq_no, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		trans_seq_no: %d", params->client_index, fp.sup_transactions.units[i].trans_seq_no);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].sm_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			sm_id: %d", params->client_index, fp.sup_transactions.units[i].sm_id);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].trans_lock_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			trans_lock_id: %d", params->client_index, fp.sup_transactions.units[i].trans_lock_id);

					fp_frame[fppos++] = fp.sup_transactions.units[j].flags;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			flags: %d", params->client_index, fp.sup_transactions.units[i].flags);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].money, 3);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			money: %d", params->client_index, fp.sup_transactions.units[i].money);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].volume, 3);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			volume: %d", params->client_index, fp.sup_transactions.units[i].volume);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].grade_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			GradeId: %d", params->client_index, fp.sup_transactions.units[i].grade_id);

				}
			}

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;

		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_sup_trans_buffer_status_req_3_mult(PSSClientThreadFuncParam* params, guint8 ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FpSupTransBufStatus3 data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpSupTransBufStatus_req, pssc_FpSupTransBufStatus, 0x03);

	guint8 fp_count = get_fuelling_point_count();

	reply[pos++] = fp_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp count: %d", params->client_index, fp_count);

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos], fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpID: %d", params->client_index, fp_id);

			fp_frame[fppos++] = fp.sup_transactions.count;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Transaction count: %d", params->client_index, fp.sup_transactions.count);

			if (fp.sup_transactions.count > 0)
			{
				for (guint8 j = 0; j < fp.sup_transactions.count; j++)
				{
					fppos+=add_bcd_field(&fp_frame[fppos], fp.sup_transactions.units[j].trans_seq_no, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				TransSeqNo: %d", params->client_index, fp.sup_transactions.units[j].trans_seq_no);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].sm_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					SmID: %d", params->client_index, fp.sup_transactions.units[j].sm_id);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].trans_lock_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					TransLockID: %d", params->client_index, fp.sup_transactions.units[j].trans_lock_id);

					fp_frame[fppos++] = fp.sup_transactions.units[j].flags;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Flags: %d", params->client_index, fp.sup_transactions.units[j].flags);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].money, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Money: %d", params->client_index, fp.sup_transactions.units[i].money);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].volume, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Volume: %d", params->client_index, fp.sup_transactions.units[i].volume);

					fppos+=add_bcd_field(&fp_frame[fppos],fp.sup_transactions.units[j].grade_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					GradeID: %d", params->client_index, fp.sup_transactions.units[j].grade_id);
				}
			}

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;

		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fp_grade_totals_req_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse fp_grade_totals_req:", params->client_index);

	guint8 fp_id = parse_BCD(buffer, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	send_fp_grade_totals_set(pssc_FpGradeTotals_req, pssc_FpGradeTotals, subcode, fp_id, params, ex_mode);

}

void parse_pump_grade_totals_req_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	guint8 fp_id = parse_BCD(buffer, 1);

	send_fp_grade_totals_set(pssc_PumpGradeTotals_req, pssc_PumpGradeTotals, subcode, fp_id, params, ex_mode);

}

void parse_clr_fp_sup_trans(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse clr_fp_sup_trans:", params->client_index);

	guint16 pos = 0;

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 pos_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PosId: %d", params->client_index, pos_id);

	guint8 trans_seq_no = parse_BCD(&buffer[pos], 2); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransSeqNo: %d", params->client_index, trans_seq_no);

	guint32 vol_e = parse_BCD(&buffer[pos], 5); pos+=10;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Vol_E: %d", params->client_index, vol_e);

	guint32 money_e = parse_BCD(&buffer[pos], 5); pos+=10;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Vol_E: %d", params->client_index, money_e);

	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Param count: %d", params->client_index, param_count);

	delete_fp_sup_trans(fp_id, trans_seq_no);

	reset_fp(fp_id, params, pssc_clr_FpSupTrans, subcode, ex_mode);
}

void send_clr_fp_sup_trans_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare clr_fp_sup_trans_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_clr_FpSupTrans, pssc_clr_FpSupTrans_ack, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);

}

void parse_fp_sup_trans_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse fp_sup_trans_req:", params->client_index);

	guint16 pos = 0;

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 trans_seq_no = parse_BCD(&buffer[pos], 2); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TransSeqNo: %d", params->client_index, trans_seq_no);

	guint8 pos_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PosId: %d", params->client_index, pos_id);

	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Param count: %d", params->client_index, param_count);

	guint8 param_ids[PSS_MAX_TRANSACTION_PARAM_COUNT] = {0x00};

	if (param_count > 0)
	{
		for (guint8 i = 0; i < MIN(param_count,PSS_MAX_TRANSACTION_PARAM_COUNT) ; i++)
		{
			param_ids[i] = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamId: %d", params->client_index, param_ids[i]);
		}
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_sup_trans:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpSupTrans_req, pssc_FpSupTrans, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	pos+=add_bcd_field(&reply[pos],trans_seq_no, 2);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	trans_seq_no: %d",	params->client_index, trans_seq_no);

	reply[pos++] = param_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	param_count: %d",	params->client_index, param_count);

	PSSTransaction transaction = {0x00};

	get_sup_transaction(fp_id, trans_seq_no, &transaction);

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],param_ids[i], 1);

			switch(param_ids[i])
			{
				case tpi_Vol_e:
					pos+=add_bcd_field(&reply[pos],transaction.volume, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamId %d (Volume): %d",	params->client_index, param_ids[i], transaction.volume);
					break;

				case tpi_Money_e:
					pos+=add_bcd_field(&reply[pos],transaction.money, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamId %d (Money): %d",	params->client_index, param_ids[i], transaction.money);
					break;
			}
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fp_fuelling_data_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse FpFuellingData_req:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_fuelling_data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpFuellingData_req, pssc_FpFuellingData, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	PSSFuellingPoint fp = {0x00};

	get_fuelling_point_by_id(fp_id, &fp);

	if (fp.id == fp_id)
	{
		if (subcode == 0)
		{
			pos+=add_bcd_field(&reply[pos],fp.fuelling_volume, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fuelling_volume: %d",	params->client_index, fp.fuelling_volume);

			pos+=add_bcd_field(&reply[pos],fp.fuelling_money, 3);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fuelling_money: %d",	params->client_index, fp.fuelling_money);
		}
		else
		{
			pos+=add_bcd_field(&reply[pos],fp.fuelling_volume, 5);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fuelling_volume: %d",	params->client_index, fp.fuelling_volume);

			pos+=add_bcd_field(&reply[pos],fp.fuelling_money, 5);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fuelling_money: %d",	params->client_index, fp.fuelling_money);
		}

	}
	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fp_info_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse FpInfoReq:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	NoFpInofItems: %d", params->client_index, param_count);

	guint8 param_ids[PSS_MAX_TRANSACTION_PARAM_COUNT] = {0x00};

	if (param_count > 0)
	{
		for (guint8 i = 0; i < MIN(param_count,PSS_MAX_TRANSACTION_PARAM_COUNT) ; i++)
		{
			param_ids[i] = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamId: %d", params->client_index, param_ids[i]);
		}
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_info:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpInfo_req, pssc_FpInfo, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	reply[pos++] = param_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	param_count: %d",	params->client_index, param_count);

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],param_ids[i], 1);

			switch(param_ids[i])
			{
				case 1:
					if (subcode == 0)
					{
						reply[pos++] = 0x00;
					}
					else
					{
						reply[pos++] = 0x01;
						reply[pos++] = 0x00;
						reply[pos++] = 0x00;
					}
					break;

				case 2:
					{

						PSSFuellingPoint fp = {0x00};

						get_fuelling_point_by_id(fp_id, &fp);

						if (fp.id == fp_id)
						{
							if (subcode > 0)
							{
								guint16 len = 1 + (fp.grade_option_count * 3);

								reply[pos++] = len & 0xFF;
								reply[pos++] = len >> 8;
							}

							reply[pos++] = fp.grade_option_count;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grade_count: %d",	params->client_index, fp.grade_option_count);

							if (fp.grade_option_count > 0)
							{
								PSSPriceGroup pg = {0x00};

								if (fp.operation_mode_count > 0 && fp.operation_modes[0].service_mode_count>0)
								{
									get_price_group_by_id(fp.operation_modes[0].service_modes[0].price_group_id, &pg);

									for (guint8 j = 0; j < fp.grade_option_count; j++)
									{
										pos+=add_bcd_field(&reply[pos],fp.grade_options[j].grade_id, 1);

										if (pg.grade_price_count > 0)
										{
											for( guint8 k = 0; k < pg.grade_price_count; k++)
											{
												if (pg.grade_prices[k].grade_id == fp.grade_options[j].grade_id)
												{
													pos+=add_bcd_field(&reply[pos],pg.grade_prices[k].price, 2);
												}
											}
										}
										else
										{
											reply[pos++] = 0x00;
											reply[pos++] = 0x00;
										}
									}
								}
							}
						}
					}

					break;

				case 3:
					if (subcode == 0)
					{
						reply[pos++] = 0x00;
					}
					else
					{
						reply[pos++] = 0x01;
						reply[pos++] = 0x00;
						reply[pos++] = 0x00;

					}
					break;

			}
		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fp_sup_trans_buffer_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse FcSupTransBufferStatus:", params->client_index);

	guint8 fp_id = parse_BCD(buffer, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	switch (subcode)
	{
		case 0x00:
			if (fp_id > 0)
			{
				send_fp_sup_trans_buffer_status_req_0(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_sup_trans_buffer_status_req_0_mult(params, ex_mode, port);
			}
			break;

		case 0x01:
			if (fp_id > 0)
			{
				send_fp_sup_trans_buffer_status_req_1(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_sup_trans_buffer_status_req_1_mult(params, ex_mode, port);
			}
			break;

		case 0x03:
			if (fp_id > 0)
			{
				send_fp_sup_trans_buffer_status_req_3(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_sup_trans_buffer_status_req_3_mult(params, ex_mode, port);
			}
			break;
	}
}

void all_fp_price_update(guint8 op_mode_no, PSSClientThreadFuncParam* params)
{
	guint8 fp_count = get_fuelling_point_count();

	if (fp_count > 0)
	{

		for (guint i = 0; i < fp_count; i++)
		{
			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_index(i, &fp);

			fp_price_update(fp, op_mode_no, params);
		}
	}
}

void parse_set_fc_operation_mode_no(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_set_FcOperationModeNo, pssc_set_FcOperationModeNo_ack, subcode);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse set_FcOperationModeNo:", params->client_index);

	guint8 op_mode_no = parse_BIN8(buffer);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	OperationModeNo: %d", params->client_index, op_mode_no);

	update_fc_operation_mode_no(op_mode_no);
	all_fp_price_update(op_mode_no,	params );

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare set_FcOperationModeNo_ack:", params->client_index);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fc_price_set_req(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcPriceSet_req, pssc_FcPriceSet, subcode);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FcPriceSet:", params->client_index);

	PSSFcStatus fc_status = {0x00};
	get_fc_status(&fc_status);

	PSSGeneralFunctions general_functions = {0x00};
	get_fc_general_functions(&general_functions);

	if (subcode == 3)
	{
		reply[pos++] = 0x00;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	user_id: %d", params->client_index, 0);
	}

	pos+=add_bcd_field(&reply[pos],fc_status.fc_price_set_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_price_set_id: %d", params->client_index, fc_status.fc_price_set_id);

	reply[pos++] = general_functions.price_group_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	price_group_count: %d", params->client_index, general_functions.price_group_count);

	if (general_functions.price_group_count > 0)
	{
		reply[pos++] = general_functions.price_groups[0].grade_price_count;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grade_count: %d", params->client_index, general_functions.price_groups[0].grade_price_count);
	}
	else
	{
		reply[pos++] = general_functions.grade_count;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grade_count: %d", params->client_index, general_functions.grade_count);
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	price_groups:",	params->client_index);

	if (general_functions.price_group_count > 0)
	{
		for (guint8 i = 0; i < general_functions.price_group_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],general_functions.price_groups[i].id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PriceGroupId: %d", params->client_index, general_functions.price_groups[i].id);
		}
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	grades:", params->client_index);

	if (general_functions.price_group_count > 0)
	{
		if (general_functions.price_groups[0].grade_price_count > 0)
		{
			for (guint8 i = 0; i < general_functions.price_groups[0].grade_price_count; i++)
			{
				pos+=add_bcd_field(&reply[pos],general_functions.price_groups[0].grade_prices[i].grade_id, 1);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		GradeId: %d", params->client_index,general_functions.price_groups[0].grade_prices[i].grade_id);
			}

		}
	}
	else
	{
		if (general_functions.grade_count > 0)
		{
			for (guint8 i = 0; i < general_functions.grade_count; i++)
			{
				pos+=add_bcd_field(&reply[pos],general_functions.grades[i].id, 1);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		GradeId: %d", params->client_index, general_functions.grades[i].id);
			}
		}
	}
	if (general_functions.price_group_count > 0)
	{
		for (guint8 i = 0; i < general_functions.price_group_count; i++)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PriceGroupId: %d", params->client_index, general_functions.price_groups[i].id);
			if (general_functions.price_groups[i].grade_price_count > 0)
			{
				for (guint8 k = 0; k < general_functions.price_groups[i].grade_price_count; k++)
				{
					switch(subcode)
					{
						case 0x00:
							pos+=add_bcd_field(&reply[pos],general_functions.price_groups[i].grade_prices[k].price, 2);
							break;
						case 0x02:
						case 0x03:
							pos+=add_bcd_field(&reply[pos],general_functions.price_groups[i].grade_prices[k].price, 3);
							break;

					}
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Grade ID%d: price %d",
							params->client_index, general_functions.price_groups[i].grade_prices[k].grade_id, general_functions.price_groups[i].grade_prices[k].price);
				}
			}
		}
	}

	if (subcode > 0)
	{
		struct tm tm_sp = *localtime(&fc_status.fc_price_set_date_and_time);

		pos+=add_date_time_field(&reply[pos], &tm_sp);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_price_set_date_and_time: %04d.%02d.%02d %02d:%02d:%02d",
				params->client_index, tm_sp.tm_year + 1900, tm_sp.tm_mon + 1, tm_sp.tm_mday, tm_sp.tm_hour,tm_sp.tm_min, tm_sp.tm_sec);
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_load_fc_price_set(guint8* buffer, PSSClientThreadFuncParam* params, guint8 subcode, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse load_fc_price:", params->client_index);

	guint16 pos = 0;

	guint8 fc_price_set_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_price_set_id: %d", params->client_index, fc_price_set_id);

	guint8 no_fc_price_group = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	no_fc_price_group: %d", params->client_index, no_fc_price_group);

	guint8 no_fc_grades = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  no_fc_grades: %d", params->client_index, no_fc_grades);

	update_fc_price_set_id(fc_price_set_id);

	guint8 price_group_ids[MAX_PRICE_GROUP_COUNT] = {0x00};
	guint8 grade_ids[MAX_GRADE_COUNT] = {0x00};

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  fc_price_groups:", params->client_index);

	for (guint8 i = 0; i < no_fc_price_group; i++)
	{
		if (i < MAX_PRICE_GROUP_COUNT)
		{
			price_group_ids[i] = parse_BCD(&buffer[pos], 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  	fc_price_droup_id = %d", params->client_index, price_group_ids[i]);
		}
		pos+=2;
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  fc_grades:",params->client_index);

	for (guint8 i = 0; i < no_fc_grades; i++)
	{
		if (i < MAX_GRADE_COUNT)
		{
			grade_ids[i] = parse_BCD(&buffer[pos], 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  	fc_grade_id = %d", params->client_index, grade_ids[i]);
		}
		pos+=2;
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  prices:",params->client_index);

	for (guint8 i = 0; i < no_fc_price_group; i++)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d  	fc_price_group_id = %d", params->client_index, price_group_ids[i]);

		PSSPriceGroup price_group = {0x00};
		price_group.id = price_group_ids[i];

		for (guint8 j = 0; j < no_fc_grades; j++)
		{
			guint32 price = parse_BCD(&buffer[pos], 3); pos+=6;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			grade id %d price %d", params->client_index, grade_ids[j], price);

			if (price_group.grade_price_count < MAX_GRADE_COUNT)
			{
				price_group.grade_prices[price_group.grade_price_count].grade_id = grade_ids[j];
				price_group.grade_prices[price_group.grade_price_count].price = price;
				price_group.grade_price_count++;
			}
		}

		update_price_group(price_group);

		price_update(&price_group, params);
	}

	all_fp_price_update(get_fc_operation_mode_no(), params);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare load_FcPriceSetAck:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_load_FcPriceSet, pssc_load_FcPriceSetAck, subcode);

	pos+=add_bcd_field(&reply[pos],fc_price_set_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_price_set_id: %d", params->client_index, fc_price_set_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_clr_install_data(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse clr_install_data:", params->client_index);

	guint16 pos = 0;

	if (subcode == 0)
	{
		guint8 install_msg_code = parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	install_msg_code: %d", params->client_index, install_msg_code);

		guint8 fc_device_id = parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_device_id: %d", params->client_index, fc_device_id);

		clear_install_data(install_msg_code, fc_device_id);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d cleared devices install_msg_code: %d fc_device_id: %d",
				params->client_index, install_msg_code, fc_device_id);
	}
	else
	{
		guint8 ext_install_msg_code_0 = parse_BIN8(&buffer[pos]); pos+=2;
		guint8 ext_install_msg_code_1 = parse_BIN8(&buffer[pos]); pos+=2;
		guint8 ext_install_msg_code_2 = parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	ext_install_msg_code: %02X, %02X, %02X", params->client_index,
				ext_install_msg_code_0, ext_install_msg_code_1, ext_install_msg_code_2);

		guint8 fc_device_id = parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fc_device_id: %d", params->client_index, fc_device_id);

		clear_ext_install_data(ext_install_msg_code_0, ext_install_msg_code_1, ext_install_msg_code_2, fc_device_id);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d cleared devices install_msg_code: %02X, %02X, %02X fc_device_id: %d",
				params->client_index, ext_install_msg_code_0, ext_install_msg_code_1, ext_install_msg_code_2, fc_device_id);

	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare clr_install_data_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_clr_install_data, pssc_clr_install_data_ack, subcode);

	socket_client_send_with_mutex(params,reply,pos);
}

void send_tg_status_req_0(guint8 tg_id, PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_TgStatus_Req, pssc_TgStatus, 0x00);

	PSSTankGauge tg = {0x00};
	get_tg_by_id(tg_id, &tg);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare tg_status0:", params->client_index);

	pos+=add_bcd_field(&reply[pos],tg_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TgId: %d", params->client_index, tg_id);

	reply[pos++] = tg.main_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d",
			params->client_index, tg.main_state);


	reply[pos++] = tg.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d",
			params->client_index, tg.sub_state);

	reply[pos++] = 0x00;
	reply[pos++] = 0x00;

	socket_client_send_with_mutex(params,reply,pos);
}

void send_tg_status_req_0_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare TgStatus1_req:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_TgStatus_Req, pssc_TgStatus, 0x00);  //исправлен subcode 1

	guint8 tgs_count = get_tgs_count();

	reply[pos++] = tgs_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	tgs_count: %d", params->client_index, tgs_count);

	if (tgs_count > 0)
	{
		for (guint8 i = 0; i < tgs_count; i++)
		{
			guint8 tg_id = get_tank_gauge_id_by_index(i);

			PSSTankGauge tg = {0x00};
			get_tg_by_id(tg_id, &tg);

			guint8 tg_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 tgpos = 0;

			tgpos+=add_bcd_field(&tg_frame[tgpos],tg_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgId: %d", params->client_index, tg_id);

			tg_frame[tgpos++] = tg.main_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d",
					params->client_index, tg.main_state);


			tg_frame[tgpos++] = tg.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SubState: %d",
					params->client_index, tg.sub_state);

			reply[pos++] = 0x00;
			reply[pos++] = 0x00;

			reply[pos++] = tgpos;
			memcpy(&reply[pos], tg_frame, tgpos);
			pos+=tgpos;
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_tg_status_req_1(guint8 tg_id, PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_TgStatus_Req, pssc_TgStatus, 0x01);

	PSSTankGauge tg = {0x00};
	get_tg_by_id(tg_id, &tg);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare tg_status0:", params->client_index);

	pos+=add_bcd_field(&reply[pos],tg_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TgId: %d", params->client_index, tg_id);

	reply[pos++] = tg.main_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d",	params->client_index, tg.main_state);

	reply[pos++] = tg.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d", params->client_index, tg.sub_state);

	reply[pos++] = 0x00;
	reply[pos++] = 0x01;

	reply[pos++] = 0x00;

	socket_client_send_with_mutex(params,reply,pos);
}

void send_tg_status_req_1_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;
	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare TgStatus1_req:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_TgStatus_Req, pssc_TgStatus, 0x01);

	guint8 tgs_count = get_tgs_count();

	reply[pos++] = tgs_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	tgs_count: %d", params->client_index, tgs_count);

	if (tgs_count > 0)
	{
		for (guint8 i = 0; i < tgs_count; i++)
		{
			guint8 tg_id = get_tank_gauge_id_by_index(i);
			PSSTankGauge tg = {0x00};
			get_tg_by_id(tg_id, &tg);

			guint8 tg_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint8 tgpos = 0;

			tgpos+=add_bcd_field(&tg_frame[tgpos],tg_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgId: %d", params->client_index, tg_id);

			tg_frame[tgpos++] = 0x02;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d",	params->client_index, tg.main_state);


			tg_frame[tgpos++] = tg.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SubState: %d", params->client_index, tg.sub_state);

			tg_frame[tgpos++] = 0x00;
			tg_frame[tgpos++] = 0x00;

			tg_frame[tgpos++] = 0x00;

			reply[pos++] = tgpos;
			memcpy(&reply[pos], tg_frame, tgpos);
			pos+=tgpos;
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_tg_data_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;
	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse TgData_req:", params->client_index);

	guint16 pos = 0;

	guint8 tg_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TgId: %d", params->client_index, tg_id);

	PSSTankGauge tg = {0x00};
	get_tg_by_id(tg_id, &tg);

	guint8 no_tank_data_items = parse_BIN8(&buffer[pos]); pos+=2;

	guint8 tank_data_ids[PSS_MAX_TANK_UNITS_COUNT] = {0x00};

	if (no_tank_data_items > 0)
	{
		for (guint8 i = 0; i < no_tank_data_items && i < PSS_MAX_TANK_UNITS_COUNT; i++)
		{
			tank_data_ids[i] = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamID: %d", params->client_index, tank_data_ids[i]);
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_TgData_Req, pssc_TgData, subcode);

	PSSTank pss_tank = {0x00};
	get_tank_by_id(tg_id, &pss_tank);

	pos+=add_bcd_field(&reply[pos],tg_id, 1);

	reply[pos++] = tg.sub_state;

	reply[pos++] = no_tank_data_items;

	if (no_tank_data_items > 0)
	{
		for (guint8 i = 0; i < no_tank_data_items && i < PSS_MAX_TANK_UNITS_COUNT; i++)
		{
			pos+=add_bcd_field(&reply[pos],tank_data_ids[i], 1);

			switch (tank_data_ids[i])
			{
				case ptdii_TankProductLevel:
					reply[pos++] = 0x03;
					pos+=add_bcd_field(&reply[pos],pss_tank.height, 3);
					break;

				case ptdii_TankWaterLevel:
					reply[pos++] = 0x03;
					pos+=add_bcd_field(&reply[pos],pss_tank.waterlevel, 3);
					break;

				case ptdii_TankTotalObservedVol:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.volume, 6);
					break;

				case ptdii_TankWaterVol:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],0 , 6);
					break;

				case ptdii_TankGrossObservedVol:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.volume, 6);
					break;

				case ptdii_TankGrossStdVol:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.volume, 6);
					break;

				case ptdii_TankAvailableRoom:
					reply[pos++] = 0x00;   //6
					break;

				case ptdii_TankAverageTemp:
					reply[pos++] = 0x03;
					pos+=add_bcd_field(&reply[pos],pss_tank.temperature, 3);
					break;

				case ptdii_TankDataLastUpdateDateTime:
					reply[pos++] = 0x00;   //7
					break;

				case ptdii_TankMaxSafeFillCapacity:
					reply[pos++] = 0x00;	//7
					break;

				case ptdii_TankShellCapacity:
					reply[pos++] = 0x00;   //6
					break;

				case ptdii_TankProductMass:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.weight, 6);
					break;

				case ptdii_TankProductDensity:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.density, 6);
					break;

				case ptdii_TankProductTcDensity:
					reply[pos++] = 0x06;
					pos+=add_bcd_field(&reply[pos],pss_tank.density, 6);
					break;

				case ptdii_TankDensityProbeTemp:
					reply[pos++] = 0x00;   //3
					break;

				case ptdii_TankTempSensor1:
					reply[pos++] = 0x00;//3
					break;

				case ptdii_TankTempSensor2:
					reply[pos++] = 0x00;//3
					break;

				case ptdii_TankTempSensor3:
					reply[pos++] = 0x00;//3
					break;

			}
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_tg_status(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse TgStatus_req:", params->client_index);

	guint8 tg_id = parse_BCD(buffer, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TgId: %d", params->client_index, tg_id);

	switch (subcode)
	{
		case 0x00:
			if (tg_id > 0)
			{
				send_tg_status_req_0(tg_id, params, ex_mode);
			}
			else
			{
				send_tg_status_req_0_mult(params, ex_mode);
			}
			break;

		case 0x01:
			if (tg_id > 0)
			{
				send_tg_status_req_1(tg_id, params, ex_mode);
			}
			else
			{
				send_tg_status_req_1_mult(params, ex_mode);
			}
			break;
	}
}

void parse_install_tank_gauge(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse install_TankGauge:", params->client_index);

	PSSTankGauge new_tank_gauge = {0x00};

	new_tank_gauge.id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TgId: %d", params->client_index, new_tank_gauge.id);

	new_tank_gauge.interface_type = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TankGaugeType: %d", params->client_index,new_tank_gauge.interface_type);

	new_tank_gauge.pss_port_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PssChannelNo: %d", params->client_index,new_tank_gauge.pss_port_no);

	guint8 tmp = parse_BIN8(&buffer[pos]);  pos+=2;

	new_tank_gauge.phisical_address = tmp & 0x3F;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PhysicalAddress: %d", params->client_index,new_tank_gauge.phisical_address);

	new_tank_gauge.phisical_sub_address = (tmp & 0xC0) >> 6;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PhysicalSubAddress: %d", params->client_index, new_tank_gauge.phisical_sub_address);

	if (subcode > 0)
	{
		new_tank_gauge.tank_id = parse_BCD(&buffer[pos], 1); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	TankId: %d", params->client_index, new_tank_gauge.tank_id);
	}

	//TODO

	install_new_tg(new_tank_gauge);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d installed tank gauge id: %d", params->client_index, new_tank_gauge.id);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare install_TankGauge_asc:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_install_TankGauge, pssc_install_TankGauge_ack, subcode);

	pos+=add_bcd_field(&reply[pos],new_tank_gauge.id, 1);

	socket_client_send_with_mutex(params,reply,pos);

}

void parse_install_price_pole(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse install_Pp:", params->client_index);

	PSSPricePole new_price_pole = {0x00};

	new_price_pole.id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, new_price_pole.id);

	new_price_pole.interface_type = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpInterfaceType: %d", params->client_index, new_price_pole.interface_type);

	new_price_pole.pss_port_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PssChannelNo: %d", params->client_index, new_price_pole.pss_port_no);

	new_price_pole.phisical_address = parse_BIN8(&buffer[pos]);  pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PhysicalAddress: %d", params->client_index, new_price_pole.phisical_address);

	new_price_pole.option_count = parse_BIN8(&buffer[pos]);  pos+=2;

	if (new_price_pole.option_count > MAX_GRADE_COUNT)
	{
		new_price_pole.option_count = MAX_GRADE_COUNT;
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Options count: %d", params->client_index, new_price_pole.option_count );

	if (new_price_pole.option_count  > 0)
	{
		for (guint8 i = 0; i < new_price_pole.option_count ; i++)
		{
			new_price_pole.options[i].option_no = parse_BIN8(&buffer[pos]);  pos+=2;
			new_price_pole.options[i].fc_price_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
			new_price_pole.options[i].fc_grade_id = parse_BCD(&buffer[pos], 1); pos+=2;

			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		No: %d, fc_price_group_id = %d, fc_grade_id = %d",
					params->client_index, new_price_pole.options[i].option_no, new_price_pole.options[i].fc_price_group_id, new_price_pole.options[i].fc_grade_id);
		}
	}

	new_price_pole.main_state = pppms_Closed;
	new_price_pole.sub_state = PP_SS_ONLINE;

	install_new_pp(new_price_pole);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d installed price pole id: %d", params->client_index, new_price_pole.id);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare install_Pp_asc:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_install_Pp, pssc_install_Pp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],new_price_pole.id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_price_pole_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse PpStatus_req:", params->client_index);

	guint8 pp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};

	if (pp_id > 0)
	{
		pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_PpStatus_req, pssc_PpStatus_ack, subcode);

		PSSPricePole pp = {0x00};
		get_pp_by_id(pp_id, &pp);

		pos+=add_bcd_field(&reply[pos],pp_id, 1);

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

		reply[pos++] = pp.main_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d", params->client_index, pp.main_state);

		reply[pos++] = pp.sub_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d", params->client_index, pp.sub_state);

	}
	else
	{
		pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_PpStatus_req, pssc_PpStatus_ack, subcode);

		guint8 pp_count = get_pp_count();

		reply[pos++] = pp_count;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpCount: %d", params->client_index, pp_count);

		if (pp_count > 0)
		{
			for (guint8 i = 0; i < pp_count; i++)
			{
				reply[pos++] = 3;

				PSSPricePole pp = {0x00};
				get_pp_by_index(i, &pp);

				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Price pole:", params->client_index);

				pos+=add_bcd_field(&reply[pos],pp.id, 1);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PpId: %d", params->client_index, pp_id);

				reply[pos++] = pp.main_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d", params->client_index, pp.main_state);

				reply[pos++] = pp.sub_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d", params->client_index, pp.sub_state);
			}
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_load_pp_operation_mode_set(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse load_PpOperationModeSet:", params->client_index);

	guint pp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

	guint8 no_pp_operation_modes = parse_BIN8(&buffer[pos]);  pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	NoPpOperationModes: %d", params->client_index, no_pp_operation_modes);

	if (no_pp_operation_modes > 0)
	{
		for (guint8 i = 0; i < no_pp_operation_modes; i++)
		{
			guint8 pp_operation_mode_no = parse_BIN8(&buffer[pos]);  pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PpOperationModeNo: %d", params->client_index, pp_operation_mode_no);

			guint8 pp_operation_type = parse_BIN8(&buffer[pos]);  pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PpOperationType: %d", params->client_index, pp_operation_type);

			guint8 no_pp_service_modes = parse_BIN8(&buffer[pos]);  pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		NoPpServiceModes: %d", params->client_index, no_pp_service_modes);

			if (no_pp_service_modes > 0)
			{
				for (guint8 j = 0; j < no_pp_service_modes; j++)
				{
					guint pp_sm_id = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpSmId: %d", params->client_index, pp_sm_id);
				}
			}
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_load_PpOperationModeSet, pssc_load_PpOperationModeSet_ack, subcode);

	pos+=add_bcd_field(&reply[pos],pp_id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_open_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse open_Pp:", params->client_index);

	guint pp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

	if (pp_id > 0)
	{
		PSSPricePole pp = {0x00};

		get_pp_by_id(pp_id, &pp);

		if (pp.id == pp_id)
		{
			pp.main_state = pppms_Idle;
			set_pp_by_id(pp_id, pp);
		}
	}
	else
	{
		guint8 pp_count = get_pp_count();

		if (pp_count > 0)
		{
			for (guint8 i = 0; i < pp_count; i++)
			{
				PSSPricePole pp = {0x00};

				get_pp_by_index(i, &pp);

				if (pp.id == pp_id)
				{
					pp.main_state = pppms_Idle;
					set_pp_by_id(pp_id, pp);
				}
			}
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_open_Pp, pssc_open_Pp_ack, subcode);

	pos += add_bcd_field(&reply[pos],pp_id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_close_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse close_Pp:", params->client_index);

	guint pp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

	if (pp_id > 0)
	{
		PSSPricePole pp = {0x00};

		get_pp_by_id(pp_id, &pp);

		if (pp.id == pp_id)
		{
			pp.main_state = pppms_Closed;
			set_pp_by_id(pp_id, pp);
		}
	}
	else
	{
		guint8 pp_count = get_pp_count();

		if (pp_count > 0)
		{
			for (guint8 i = 0; i < pp_count; i++)
			{
				PSSPricePole pp = {0x00};

				get_pp_by_index(i, &pp);

				if (pp.id == pp_id)
				{
					pp.main_state = pppms_Closed;
					set_pp_by_id(pp_id, pp);
				}
			}
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_close_Pp, pssc_close_Pp_ack, subcode);

	pos += add_bcd_field(&reply[pos],pp_id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_reset_pp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse reset_Pp:", params->client_index);

	guint pp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PpId: %d", params->client_index, pp_id);

	PSSPricePole pp = {0x00};

	get_pp_by_id(pp_id, &pp);

	if (pp.id == pp_id)
	{
		pp.main_state = pppms_Unconfigured;
		set_pp_by_id(pp_id, pp);
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_reset_Pp, pssc_reset_Pp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],pp_id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_install_fp_v0(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	fp->interface_type_general = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		interface_type_general: %d", params->client_index,fp->interface_type_general);

	fp->pss_port_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		pss_port_no: %d", params->client_index,fp->pss_port_no);

	fp->phisical_address_type = 0x00;

	guint8 tmp = parse_BIN8(&buffer[pos]);  pos+=2;

	fp->phisical_address = tmp & 0x3F;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		phisical_address: %d", params->client_index,fp->phisical_address);

	fp->device_sub_address = (tmp & 0xC0) >> 6;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		device_sub_address: %d", params->client_index,fp->device_sub_address);

	guint8 grade_options_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		grade_options_count: %d", params->client_index,grade_options_count);

	if (grade_options_count > 0)
	{
		for (guint8 i = 0; i < grade_options_count; i++)
		{
			guint8 grade_option_id = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			grade_options_id: %d", params->client_index,grade_option_id);
			guint8 grade_id = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				grade_id: %d", params->client_index,grade_id);

			if (fp->grade_option_count < MAX_NOZZLE_COUNT)
			{
				fp->grade_options[fp->grade_option_count].grade_id = grade_id;
				fp->grade_options[fp->grade_option_count].id = grade_option_id;
				fp->grade_option_count++;
			}
		}
	}
}

void parse_install_fp_v1(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	fp->interface_type_general = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		interface_type_general: %d", params->client_index, fp->interface_type_general);

	fp->pss_port_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		pss_port_no: %d", params->client_index, fp->pss_port_no);

	fp->phisical_address_type = 0x00;

	guint8 tmp = parse_BIN8(&buffer[pos]);  pos+=2;

	fp->phisical_address = tmp & 0x3F;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		phisical_address: %d", params->client_index, fp->phisical_address);

	fp->device_sub_address = (tmp & 0xC0) >> 6;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		device_sub_address: %d", params->client_index, fp->device_sub_address);

	guint8 grade_options_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		grade_options_count: %d", params->client_index, grade_options_count);

	if (grade_options_count > 0)
	{
		for (guint8 i = 0; i < grade_options_count; i++)
		{
			guint8 grade_option_id = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			grade_options_id: %d", params->client_index, grade_option_id);

			guint8 grade_id = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				grade_id: %d", params->client_index, grade_id);

			guint8 tank_id = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				tank_id: %d", params->client_index,tank_id);

			if (fp->grade_option_count < MAX_NOZZLE_COUNT)
			{
				fp->grade_options[fp->grade_option_count].grade_id = grade_id;
				fp->grade_options[fp->grade_option_count].id = grade_option_id;
				fp->grade_options[fp->grade_option_count].tank_connection_count = 1;
				fp->grade_options[fp->grade_option_count].tank_connections[0].part = 100;
				fp->grade_options[fp->grade_option_count].tank_connections[0].tank_id = tank_id;

				fp->grade_option_count++;
			}
		}
	}
}

void parse_install_fp_v2(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	fp->interface_type_general = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		interface_type_general: %d", params->client_index, fp->interface_type_general);

	fp->pss_port_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		pss_port_no: %d", params->client_index, fp->pss_port_no);

	fp->phisical_address_type = 0x00;

	guint8 tmp = parse_BIN8(&buffer[pos]);  pos+=2;

	fp->phisical_address = tmp & 0x3F;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		phisical_address: %d", params->client_index, fp->phisical_address);

	fp->device_sub_address = (tmp & 0xC0) >> 6;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		device_sub_address: %d", params->client_index, fp->device_sub_address);

	guint8 grade_options_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		grade_options_count: %d", params->client_index, grade_options_count);

	if (grade_options_count > 0)
	{
		for (guint8 i = 0; i < grade_options_count; i++)
		{
			guint8 grade_option_id = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			grade_options_id: %d", params->client_index, grade_option_id);

			guint8 grade_id = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				grade_id: %d", params->client_index, grade_id);

			guint8 tank_count = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				tank_count: %d", params->client_index, tank_count);

			if (fp->grade_option_count < MAX_NOZZLE_COUNT)
			{
				fp->grade_options[fp->grade_option_count].grade_id = grade_id;
				fp->grade_options[fp->grade_option_count].id = grade_option_id;
				fp->grade_options[fp->grade_option_count].tank_connection_count = 0;

				if (tank_count > 0)
				{
					for (guint j = 0; j < tank_count; j++)
					{
						guint8 tank_id = parse_BCD(&buffer[pos], 1); pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					tank_id: %d", params->client_index,tank_id);

						guint8 part = parse_BCD(&buffer[pos], 1); pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					part: %d", params->client_index,part);

						if (fp->grade_options[fp->grade_option_count].tank_connection_count < MAX_TANK_COUNT)
						{
							fp->grade_options[fp->grade_option_count].tank_connections[fp->grade_options[fp->grade_option_count].tank_connection_count].tank_id = tank_id;
							fp->grade_options[fp->grade_option_count].tank_connections[fp->grade_options[fp->grade_option_count].tank_connection_count].part = part;
							fp->grade_options[fp->grade_option_count].tank_connection_count++;
						}
					}
				}
				fp->grade_option_count++;
			}
		}
	}
}

void parse_fuelling_point_address_v3(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	fp->pss_ext_protocol_id = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Extended protocol ID: %d", params->client_index, fp->pss_ext_protocol_id);

	fp->phisical_address_type = parse_BIN8(&buffer[pos]);  pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Physical address type: %s",
			params->client_index,fp->phisical_address_type == 0x01 ? "local" : "Ip address");

	if (fp->phisical_address_type == 0x01)
	{
		fp->phisical_address = parse_BIN8(&buffer[pos]);  pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Physical address: %d", params->client_index,fp->phisical_address);

		fp->device_sub_address = parse_BIN8(&buffer[pos]);  pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Device sub address: %d", params->client_index,fp->device_sub_address);
	}
	else
	{
		fp->phisical_address = parse_BIN32_LSB(&buffer[pos]); pos+=8;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			IP address: %03d.%03d.%03d.%03d",
				params->client_index, (fp->phisical_address >> 24) & 0xFF , (fp->phisical_address >> 16) & 0xFF ,(fp->phisical_address >> 8) & 0xFF ,fp->phisical_address & 0xFF);

		fp->phisical_address_port = parse_BIN16_LSB(&buffer[pos]); pos+=4;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Phisical address port: %d", params->client_index,fp->phisical_address_port);

		fp->device_sub_address = parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Device sub address: %d", params->client_index,fp->device_sub_address);
	}
}

guint8 parse_fuelling_point_tank_connections(PSSGradeOption* grade_option, guint8* buffer, guint16 len, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	guint8 tank_connection_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Tank connecitons (count = %d):", params->client_index,tank_connection_count);

	if (tank_connection_count > 0)
	{
		for (guint8 i = 0; i < tank_connection_count; i++)
		{
			guint8 tank_id = parse_BCD(&buffer[pos], 1); pos+=2;
			guint8 part = parse_BCD(&buffer[pos], 1); pos+=2;

			if (grade_option->tank_connection_count < MAX_TANK_COUNT)
			{
				grade_option->tank_connections[grade_option->tank_connection_count].tank_id = tank_id;
				grade_option->tank_connections[grade_option->tank_connection_count].part = part;
				grade_option->tank_connection_count++;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Tank ID %d: %d", params->client_index, tank_id, part);
			}
		}
	}
	return pos;
}


void parse_fuelling_point_grade_options(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	guint8 options_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		fp_grade_options_count: %d", params->client_index, options_count);

	if (options_count > 0)
	{
		for (guint8 i = 0; i < options_count; i++)
		{
			guint8 fp_grade_option_id = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			fp_grade_options_id: %d", params->client_index, fp_grade_option_id);

			gint8 fp_grade_option_index = get_new_grade_option_index(fp, fp_grade_option_id);

			if (fp_grade_option_index >= 0)
			{
				PSSGradeOption* grade_option = &fp->grade_options[fp_grade_option_index];

				grade_option->grade_id = parse_BCD(&buffer[pos], 1); pos+=2;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				grade_id: %d", params->client_index,grade_option->grade_id);

				pos+=parse_fuelling_point_tank_connections(grade_option, &buffer[pos], 0, params, port);

				if (fp->grade_option_count < MAX_NOZZLE_COUNT - 1)
				{
					fp->grade_option_count++;
				}
			}
		}
	}
}
guint8  parse_fuelling_point_nozzles(PSSGradeOption* grade_option, guint8* buffer,guint16 len, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	pos += parse_pss_ascii((guchar*)grade_option->nozzle_id, &buffer[pos], MAX_NOZZLE_ID_LENGTH);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Nozzle ID %s", params->client_index,grade_option->nozzle_id);

	return pos;
}

void parse_fuelling_point_grade_option_pars(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	guint8 options_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Grade options count: %d", params->client_index,options_count);

	if (options_count > 0)
	{
		for (guint8 i = 0; i < options_count; i++)
		{
			guint8 fp_grade_option_id = parse_BIN8(&buffer[pos]); pos+=2;

			gint8 fp_grade_option_index = get_new_grade_option_index(fp, fp_grade_option_id);

			if (fp_grade_option_index >= 0)
			{
				PSSGradeOption* grade_option = &fp->grade_options[fp_grade_option_index];

				grade_option->id = fp_grade_option_id;
				grade_option->grade_id = parse_BCD(&buffer[pos], 1); pos+=2;

				guint8 option_par_count = parse_BIN8(&buffer[pos]); pos+=2;

				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				Grade option ID%d: grade ID%d (param count = %d)",
										params->client_index, grade_option->id, grade_option->grade_id,  option_par_count);

				if (option_par_count > 0)
				{
					for (guint8 j = 0; j < option_par_count; j++)
					{
						guint8 option_par_id = parse_BCD(&buffer[pos], 1); pos+=2;
						guint16 option_par_len = parse_BIN16_LSB(&buffer[pos]); pos+=4;

						switch (option_par_id)
						{
							case 1: //Tank Connections
								pos += parse_fuelling_point_tank_connections(grade_option, &buffer[pos], option_par_len, params, port);
								break;

							case 2: // Nozzles
								pos += parse_fuelling_point_nozzles(grade_option, &buffer[pos], option_par_len, params, port);
								break;

							case 3: // NozzleTagReaderId
								grade_option->nozzle_tag_reader_id = parse_BCD(&buffer[pos], 1); pos+=2;
								add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Nozzle tag reader ID: %d",
														params->client_index, grade_option->nozzle_tag_reader_id);
								break;
						}
					}
				}

				if (fp->grade_option_count < MAX_NOZZLE_COUNT - 1)
				{
					fp->grade_option_count++;
				}
			}
		}
	}
}


void parse_install_fp_v3(PSSFuellingPoint* fp, guint8* buffer, PSSClientThreadFuncParam* params, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;
	guint8 params_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		Params (count = %d):", params->client_index,params_count);

	if (params_count > 0)
	{
		for (guint8 i = 0; i < params_count; i++)
		{
			guint8 param_id = parse_BCD(&buffer[pos], 1); pos+=2;
			guint16 param_length  = parse_BIN16_LSB(&buffer[pos]); pos+=4;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		Param id %d (length = %d):", params->client_index, param_id, param_length);

			if (param_length > 0)
			{
				switch(param_id)
				{
					case fpipi_PumpInterfaceType:
						fp->interface_type_general = parse_BIN16_LSB(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Interface type general: %d", params->client_index,fp->interface_type_general);
						break;

					case fpipi_PssChannelNo:
						fp->pss_port_no = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			PSS Channel No: %d", params->client_index,fp->pss_port_no);
						break;

					case fpipi_DeviceCommunicationAddress:
						parse_fuelling_point_address_v3(fp, &buffer[pos], params, port);
						break;

					case fpipi_FpGradeOptions:
						parse_fuelling_point_grade_options(fp, &buffer[pos], params, port);
						break;

					case fpipi_FpGradeOptionsPars:
						parse_fuelling_point_grade_option_pars(fp, &buffer[pos], params, port);
						break;

					case fpipi_PumpInterfaceTypeGeneral:
						fp->interface_type_general = parse_BIN16_LSB(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Interface type general: %d",
								params->client_index, fp->interface_type_general);
						break;

					case fpipi_PumpInterfaceTypeProtocol:
						fp->interface_type_protocol = parse_BIN16_LSB(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Interface type protocol: %d",
								params->client_index,fp->interface_type_protocol);
						break;

					case fpipi_PumpDecimalPositionInMoney:
						fp->pump_interface_decimal_positions.in_money = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in money: %d",
								params->client_index, fp->pump_interface_decimal_positions.in_money);
						break;

					case fpipi_PumpDecimalPositionInVolume:
						fp->pump_interface_decimal_positions.in_volume = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in volume: %d",
								params->client_index, fp->pump_interface_decimal_positions.in_volume);
						break;

					case fpipi_PumpDecimalPositionInPrice:
						fp->pump_interface_decimal_positions.in_price = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in price: %d",
								params->client_index, fp->pump_interface_decimal_positions.in_price);
						break;

					case fpipi_PumpDecimalPositionInMoneyTot:
						fp->pump_interface_decimal_positions.in_money_total = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in money total: %d",
								params->client_index, fp->pump_interface_decimal_positions.in_money_total);
						break;

					case fpipi_PumpDecimalPositionInVolumeTot:
						fp->pump_interface_decimal_positions.in_volume_total = parse_BIN8(&buffer[pos]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in volume total: %d",
								params->client_index, fp->pump_interface_decimal_positions.in_volume_total);
						break;

					case fpipi_PumpInterfaceConfigString:
						parse_pss_ascii((guchar*)fp->config_string, &buffer[pos], MAX_CONFIG_STRING_LENGTH);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Config string: %s",
								params->client_index, fp->config_string);
						break;

					case fpipi_NomimalNormalSpeed:
						fp->nominal_normal_speed = parse_BCD(&buffer[pos], 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Nominal normal speed: %d",
								params->client_index, fp->nominal_normal_speed);
						break;

					case fpipi_NominalHighSpeed:
						fp->nominal_high_speed = parse_BCD(&buffer[pos], 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Nominal high speed: %d",
								params->client_index, fp->nominal_high_speed);
						break;

					case fpipi_HighSpeedTriggerLevel:
						fp->high_speed_trigger_level = parse_BCD(&buffer[pos], 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			High speed trigger level: %d",
								params->client_index, fp->high_speed_trigger_level);
						break;

					default:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Undefined param: %d", params->client_index,param_id);
						break;
				}
			}
			pos += param_length * 2;
		}
	}
}

void parse_install_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse install_Fp:", params->client_index);

	PSSFuellingPoint new_fp = {0x00};

	new_fp.id = parse_BCD(&buffer[pos], 1); pos+=2;

	new_fp.operation_mode_no = get_fc_operation_mode_no();

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, new_fp.id);

	switch(subcode)
	{
		case 0x00:
			parse_install_fp_v0(&new_fp, &buffer[pos], params, port);
			break;

		case 0x01:
			parse_install_fp_v1(&new_fp, &buffer[pos], params, port);
			break;

		case 0x02:
			parse_install_fp_v2(&new_fp, &buffer[pos], params, port);
			break;

		case 0x03:
			parse_install_fp_v3(&new_fp, &buffer[pos], params, port);
			break;
	}

	install_new_fp(new_fp);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d installed fuelling point id: %d", params->client_index, new_fp.id);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare install_Fp_asc:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_install_Fp, pssc_install_Fp_asc, subcode);

	pos+=add_bcd_field(&reply[pos],new_fp.id, 1);

	socket_client_send_with_mutex(params,reply,pos);
}
void send_fp_status_req_0(guint8 fp_id, PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x00);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status0:", params->client_index);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	pos+=add_bcd_field(&reply[pos],fp.sm_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SmId: %d", params->client_index, fp.sm_id);

	if (fp.is_close)
	{
		reply[pos++] = fpms_Closed;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));
	}
	else
	{
		reply[pos++] = fp.main_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
	}


	reply[pos++] = fp.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d",
			params->client_index, fp.sub_state);

	pos+=add_bcd_field(&reply[pos],fp.locked_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	LockedId: %d",
			params->client_index, fp.locked_id);

	socket_client_send_with_mutex(params,reply,pos);
}


void send_fp_status_req_0_mult( PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status0:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x00);

	guint8 fp_count = get_fuelling_point_count();

	reply[pos++] = fp_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_count: %d", params->client_index, fp_count);

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos],fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpId: %d", params->client_index, fp_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SmId: %d", params->client_index, fp.sm_id);

			if (fp.is_close)
			{
				fp_frame[fppos++] = fpms_Closed;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));

			}
			else
			{
				fp_frame[fppos++] = fp.main_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
			}

			fp_frame[fppos++] = fp.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SubState: %d",
					params->client_index, fp.sub_state);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.locked_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		LockedId: %d",
					params->client_index, fp.locked_id);

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_status_req_1(guint8 fp_id, PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status1:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x01);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	pos+=add_bcd_field(&reply[pos],fp.sm_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SmId: %d", params->client_index, fp.sm_id);

	if (fp.is_close)
	{
		reply[pos++] = fpms_Closed;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)", params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));
	}
	else
	{
		reply[pos++] = fp.main_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
	}

	reply[pos++] = fp.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d", params->client_index, fp.sub_state);

	pos+=add_bcd_field(&reply[pos],fp.locked_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	LockedId: %d", 	params->client_index, fp.locked_id);

	pos+=add_bcd_field(&reply[pos],fp.grade_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	GradeId: %d", params->client_index, fp.grade_id);

	socket_client_send_with_mutex(params,reply,pos);
}
void send_fp_status_req_1_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status1:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x01);

	guint8 fp_count = get_fuelling_point_count();

	reply[pos++] = fp_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_count: %d", params->client_index, fp_count);

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos],fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpId: %d", params->client_index, fp_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SmId: %d", params->client_index, fp.sm_id);

			if (fp.is_close)
			{
				fp_frame[fppos++] = fpms_Closed;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));
			}
			else
			{
				fp_frame[fppos++] = fp.main_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
			}

			fp_frame[fppos++] = fp.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SubState: %d", params->client_index, fp.sub_state);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.locked_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		LockedId: %d", params->client_index, fp.locked_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		GradeId: %d", params->client_index, fp.grade_id);

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_status_req_2(guint8 fp_id, PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status2:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x02);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	pos+=add_bcd_field(&reply[pos],fp.sm_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SmId: %d", params->client_index, fp.sm_id);

	if (fp.is_close)
	{
		reply[pos++] = fpms_Closed;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));
	}
	else
	{
		reply[pos++] = fp.main_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
	}

	reply[pos++] = fp.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d", params->client_index, fp.sub_state);

	pos+=add_bcd_field(&reply[pos], fp.locked_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	LockedId: %d", params->client_index, fp.locked_id);

	reply[pos++] = 0x00;

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_status_req_2_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status2:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x02);

	guint8 fp_count = get_fuelling_point_count();
	reply[pos++] = fp_count;

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos],fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpId: %d", params->client_index, fp_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SmId: %d", params->client_index, fp.sm_id);

			if (fp.is_close)
			{
				fp_frame[fppos++] = fpms_Closed;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));

			}
			else
			{
				fp_frame[fppos++] = fp.main_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
			}

			fp_frame[fppos++] = fp.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SubState: %d",
					params->client_index, fp.sub_state);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.locked_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		LockedId: %d",
					params->client_index, fp.locked_id);

			fp_frame[fppos++] = 0x00;

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;
		}
	}

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fp_status_req_3(guint8 fp_id, PSSClientThreadFuncParam* params,  gboolean ex_mode, guint32 port)
{
	LogParams* log_params = params->log_params;

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x03);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fp_status3:", params->client_index);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	pos+=add_bcd_field(&reply[pos],fp.sm_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SmId: %d", params->client_index, fp.sm_id);

	if (fp.is_close)
	{
		reply[pos++] = fpms_Closed;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));
	}
	else
	{
		reply[pos++] = fp.main_state;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	MainState: %d (%s)",
				params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
	}

	reply[pos++] = fp.sub_state;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState: %d",
			params->client_index, fp.sub_state);

	pos+=add_bcd_field(&reply[pos],fp.locked_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	LockedId: %d", params->client_index, fp.locked_id);

	pos+=add_bcd_field(&reply[pos],fp.grade_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	GradeId: %d", params->client_index, fp.grade_id);

	guint price_group_id = 0;
	guint8 operation_mode_no = 0;
	guint8 smids[MAX_SERVICE_MODE_COUNT] = {0x00};
	guint8 smids_count = 0;

	if (fp.operation_mode_count > 0)
	{
		for (guint8 i = 0; i < fp.operation_mode_count; i++)
		{
			operation_mode_no = fp.operation_modes[i].id;

			if (fp.operation_modes[i].service_mode_count > 0)
			{
				for (guint8 j = 0; j < fp.operation_modes[i].service_mode_count; j++)
				{
					for (guint k = 0; k < MAX_SERVICE_MODE_COUNT; k++)
					{
						price_group_id = fp.operation_modes[i].service_modes[j].price_group_id;

						if(smids[k] == fp.operation_modes[i].service_modes[j].service_mode_id)
						{
							break;
						}
						else if (smids[k] == 0)
						{
							smids[k] = fp.operation_modes[i].service_modes[j].service_mode_id;
							smids_count++;
							break;
						}
					}
				}
			}
		}
	}

	guint8 param_count = 4 + ((fp.fuelling_volume > 0) ? 1 : 0) + ((fp.fuelling_money > 0) ? 1 : 0) + ((strlen(fp.attendant_accaunt_id) > 0) ? 1 : 0) +
			((price_group_id > 0) ? 1 : 0) + ((fp.grade_id > 0) ? 1 : 0);

	reply[pos++] = param_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	param_count: %d", params->client_index, param_count);

	pos+=add_bcd_field(&reply[pos],fpspid_SubState2, 1);
	reply[pos++] = 0x01;
	reply[pos++] = fp.sub_state2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	SubState2: %d", params->client_index, 1);

	pos+=add_bcd_field(&reply[pos],fpspid_AvailableSms, 1);
	reply[pos++] = smids_count + 1;
	reply[pos++] = smids_count;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	AvailableSms:", params->client_index);

	if (smids_count > 0)
	{
		for (guint8 i = 0; i < smids_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],smids[i], 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SmId: %d", params->client_index, smids[i]);

		}
	}

	pos+=add_bcd_field(&reply[pos],fpspid_AvailableGrades, 1);
	reply[pos++] = fp.grade_option_count + 1;
	reply[pos++] = fp.grade_option_count;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	AvailableGrades:", params->client_index);

	if (fp.grade_option_count > 0)
	{
		for (guint8 i = 0; i < fp.grade_option_count; i++)
		{
			pos+=add_bcd_field(&reply[pos],fp.grade_options[i].grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		GradeId: %d", params->client_index, fp.grade_options[i].grade_id);
		}
	}

	if (fp.grade_id > 0)
	{
		if (fp.grade_option_count > 0)
		{
			for (guint8 i = 0; i < fp.grade_option_count; i++)
			{
				if (fp.grade_options[i].grade_id == fp.grade_id)
				{
					pos+=add_bcd_field(&reply[pos],fpspid_GradeOptionNo, 1);
					reply[pos++] = 0x01;
					reply[pos++] = fp.grade_options[i].id;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	GradeOptionNo: %d", params->client_index, fp.grade_options[i].id);
					break;
				}
			}
		}
	}

	if (fp.fuelling_volume > 0)
	{
		pos+=add_bcd_field(&reply[pos],fpspid_FuellingDataVolE, 1);
		reply[pos++] = 0x05;
		pos+=add_bcd_field(&reply[pos],fp.fuelling_volume, 5);
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FuellingDataVolE: %d", params->client_index, fp.fuelling_volume);
	}
	if (fp.fuelling_money > 0)
	{
		pos+=add_bcd_field(&reply[pos],fpspid_FuellingDataMonE, 1);
		reply[pos++] = 0x05;
		pos+=add_bcd_field(&reply[pos],fp.fuelling_money, 5);
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FuellingDataMonE: %d", params->client_index, fp.fuelling_money);
	}

	if (strlen(fp.attendant_accaunt_id) > 0)
	{
		pos+=add_bcd_field(&reply[pos],fpspid_AttendantAccountId, 1);
		reply[pos++] = strlen(fp.attendant_accaunt_id) + 1;
		reply[pos++] = strlen(fp.attendant_accaunt_id);
		memcpy(&reply[pos],fp.attendant_accaunt_id, strlen(fp.attendant_accaunt_id) );
		pos+=strlen(fp.attendant_accaunt_id);
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	AttendantAccountId: %s", params->client_index, fp.attendant_accaunt_id);
	}

//	guint8 operation_mode_no = get_fc_operation_mode_no();

	pos+=add_bcd_field(&reply[pos],fpspid_OperationModeNo, 1);
	reply[pos++] = 0x01;
	reply[pos++] = operation_mode_no;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	OperationModeNo: %d", params->client_index, operation_mode_no);

	if (price_group_id > 0)
	{
		pos+=add_bcd_field(&reply[pos],fpspid_PriceGroupID, 1);
		reply[pos++] = 0x01;
		pos+=add_bcd_field(&reply[pos],price_group_id, 1);
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PriceGroupID: %d", params->client_index, price_group_id);
	}

	if (fp.grade_id > 0)
	{
		for (guint8 i = 0; i < fp.grade_option_count; i++)
		{
			if (fp.grade_options[i].grade_id == fp.grade_id)
			{
				if (strlen(fp.grade_options[i].nozzle_id) > 0)
				{
					pos+=add_bcd_field(&reply[pos],fpspid_NozzleID, 1);
					reply[pos++] = strlen(fp.grade_options[i].nozzle_id) + 1;
					reply[pos++] = strlen(fp.grade_options[i].nozzle_id);
					memcpy(&reply[pos],fp.grade_options[i].nozzle_id, strlen(fp.grade_options[i].nozzle_id) );
					pos+=strlen(fp.grade_options[i].nozzle_id);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	NozzleID: %s", params->client_index, fp.grade_options[i].nozzle_id);
				}
				if (fp.grade_options[i].nozzle_tag_reader_id > 0)
				{
					pos+=add_bcd_field(&reply[pos],fpspid_NozzleTagReaderId, 1);
					reply[pos++] = 0x01;
					reply[pos++] = fp.grade_options[i].nozzle_tag_reader_id;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	NozzleTagReaderId: %d", params->client_index, fp.grade_options[i].nozzle_tag_reader_id);
				}
			}
		}
	}
	socket_client_send_with_mutex(params, reply, pos);
}

void send_fp_status_req_3_mult(PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d fp_status_req_3_mult prepare data:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_mult_reply_header(reply, port, ex_mode, pssc_FpStatus_req, pssc_FpStatus, 0x03);

	guint8 fp_count = get_fuelling_point_count();
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d	fuelling point count: %d", params->client_index, fp_count);

	reply[pos++] = fp_count;

	if (fp_count > 0)
	{
		for (guint8 i = 0; i < fp_count; i++)
		{
			guint8 fp_id = get_fuelling_point_id_by_index(i);

			PSSFuellingPoint fp = {0x00};
			get_fuelling_point_by_id(fp_id, &fp);

			guint8 fp_frame[MAX_MULTIMESSAGE_FRAME_LENGTH] = {0x00};
			guint16 fppos = 0;

			fppos+=add_bcd_field(&fp_frame[fppos],fp_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		FpID: %d", params->client_index, fp_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.sm_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		SmID: %d", params->client_index, fp.sm_id);

			if (fp.is_close)
			{
				fp_frame[fppos++] = fpms_Closed;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fpms_Closed, fp_main_state_to_str(fpms_Closed));

			}
			else
			{
				fp_frame[fppos++] = fp.main_state;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		MainState: %d (%s)",
						params->client_index, fp.main_state, fp_main_state_to_str(fp.main_state));
			}

			fp_frame[fppos++] = fp.sub_state;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		FpSubState: %d", params->client_index, fp.sub_state);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.locked_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		LockedID: %d", params->client_index, fp.locked_id);

			fppos+=add_bcd_field(&fp_frame[fppos],fp.grade_id, 1);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		GradeID: %d", params->client_index, fp.grade_id);

			guint price_group_id = 0;
			guint8 operation_mode_no = 0;
			guint8 smids[MAX_SERVICE_MODE_COUNT] = {0x00};
			guint8 smids_count = 0;

			if (fp.operation_mode_count > 0)
			{
				for (guint8 i = 0; i < fp.operation_mode_count; i++)
				{
					operation_mode_no = fp.operation_modes[i].id;

					if (fp.operation_modes[i].service_mode_count > 0)
					{
						for (guint8 j = 0; j < fp.operation_modes[i].service_mode_count; j++)
						{
							for (guint k = 0; k < MAX_SERVICE_MODE_COUNT; k++)
							{
								price_group_id = fp.operation_modes[i].service_modes[j].price_group_id;

								if(smids[k] == fp.operation_modes[i].service_modes[j].service_mode_id)
								{
									break;
								}
								else if (smids[k] == 0)
								{
									smids[k] = fp.operation_modes[i].service_modes[j].service_mode_id;
									smids_count++;
									break;
								}
							}
						}
					}
				}
			}

			//param count
			guint8 param_count =  4 + ((fp.fuelling_volume > 0) ? 1 : 0) + ((fp.fuelling_money > 0) ? 1 : 0) + ((strlen(fp.attendant_accaunt_id) > 0) ? 1 : 0) +
					((price_group_id > 0) ? 1 : 0) + ((fp.grade_id > 0) ? 1 : 0);

			fp_frame[fppos++] = param_count;

			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		Param count: %d", params->client_index, param_count);

			fppos+=add_bcd_field(&fp_frame[fppos],fpspid_SubState2, 1);
			fp_frame[fppos++] = 0x01;
			fp_frame[fppos++] = fp.sub_state2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		SubState2: %d", params->client_index, fp.sub_state2);

			fppos+=add_bcd_field(&fp_frame[fppos],fpspid_AvailableSms, 1);
			fp_frame[fppos++] = smids_count + 1;
			fp_frame[fppos++] = smids_count;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		AvailableSms count: %d", params->client_index, smids_count);

			if (smids_count > 0)
			{
				for (guint8 i = 0; i < smids_count; i++)
				{
					fppos+=add_bcd_field(&fp_frame[fppos],smids[i], 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d			SmID: %d", params->client_index, smids[i]);
				}
			}

			fppos+=add_bcd_field(&fp_frame[fppos],fpspid_AvailableGrades, 1);
			fp_frame[fppos++] = fp.grade_option_count + 1;
			fp_frame[fppos++] = fp.grade_option_count;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		AvailableGrades count: %d", params->client_index, fp.grade_option_count);

			if (fp.grade_option_count > 0)
			{
				for (guint8 i = 0; i < fp.grade_option_count; i++)
				{
					fppos+=add_bcd_field(&fp_frame[fppos],fp.grade_options[i].grade_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d			GradeID: %d", params->client_index, fp.grade_options[i].grade_id);
				}
			}

			if (fp.grade_id > 0)
			{
				if (fp.grade_option_count > 0)
				{
					for (guint8 i = 0; i < fp.grade_option_count; i++)
					{
						if (fp.grade_options[i].grade_id == fp.grade_id)
						{
							fppos+=add_bcd_field(&fp_frame[fppos],fpspid_GradeOptionNo, 1);
							fp_frame[fppos++] = 0x01;
							fp_frame[fppos++] = fp.grade_options[i].id;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		GradeOptionNo: %d", params->client_index, fp.grade_options[i].id);
							break;
						}
					}
				}
			}


			if (fp.fuelling_volume > 0)
			{
				fppos+=add_bcd_field(&fp_frame[fppos],fpspid_FuellingDataVolE, 1);
				fp_frame[fppos++] = 0x05;
				fppos+=add_bcd_field(&fp_frame[fppos],fp.fuelling_volume, 5);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		FuellingVolume: %d", params->client_index, fp.fuelling_volume);
			}
			if (fp.fuelling_money > 0)
			{
				fppos+=add_bcd_field(&fp_frame[fppos],fpspid_FuellingDataMonE, 1);
				fp_frame[fppos++] = 0x05;
				fppos+=add_bcd_field(&fp_frame[fppos],fp.fuelling_money, 5);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		FuellingMoney: %d", params->client_index, fp.fuelling_money);
			}

			if (strlen(fp.attendant_accaunt_id) > 0)
			{
				fppos+=add_bcd_field(&fp_frame[fppos],fpspid_AttendantAccountId, 1);
				fp_frame[fppos++] = strlen(fp.attendant_accaunt_id) + 1;
				fp_frame[fppos++] = strlen(fp.attendant_accaunt_id);
				memcpy(&fp_frame[fppos],fp.attendant_accaunt_id, strlen(fp.attendant_accaunt_id) );
				fppos+=strlen(fp.attendant_accaunt_id);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		AttendantAccountId: %s", params->client_index, fp.attendant_accaunt_id);
			}

			fppos+=add_bcd_field(&fp_frame[fppos],fpspid_OperationModeNo, 1);
			fp_frame[fppos++] = 0x01;
			fp_frame[fppos++] = operation_mode_no;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		OperationModeNo: %d", params->client_index, operation_mode_no);

			if (price_group_id > 0)
			{
				fppos+=add_bcd_field(&fp_frame[fppos],fpspid_PriceGroupID, 1);
				fp_frame[fppos++] = 0x01;
				fppos+=add_bcd_field(&fp_frame[fppos],price_group_id, 1);
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		PriceGroupID: %d", params->client_index, price_group_id);
			}

			if (fp.grade_id > 0)
			{
				for (guint8 i = 0; i < fp.grade_option_count; i++)
				{
					if (fp.grade_options[i].grade_id == fp.grade_id)
					{
						if (strlen(fp.grade_options[i].nozzle_id) > 0)
						{
							fppos+=add_bcd_field(&fp_frame[fppos],fpspid_NozzleID, 1);
							fp_frame[fppos++] = strlen(fp.grade_options[i].nozzle_id) + 1;
							fp_frame[fppos++] = strlen(fp.grade_options[i].nozzle_id);
							memcpy(&fp_frame[fppos],fp.grade_options[i].nozzle_id, strlen(fp.grade_options[i].nozzle_id) );
							fppos+=strlen(fp.grade_options[i].nozzle_id);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		NozzleID: %s", params->client_index, fp.grade_options[i].nozzle_id);

						}
						if (fp.grade_options[i].nozzle_tag_reader_id > 0)
						{
							fppos+=add_bcd_field(&fp_frame[fppos],fpspid_NozzleTagReaderId, 1);
							fp_frame[fppos++] = 0x01;
							fp_frame[fppos++] = fp.grade_options[i].nozzle_tag_reader_id;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d		NozzleTagReaderId: %d", params->client_index, fp.grade_options[i].nozzle_tag_reader_id);
						}
					}
				}
			}

			reply[pos++] = fppos;
			memcpy(&reply[pos], fp_frame, fppos);
			pos+=fppos;
		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void parse_fp_status_req(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params,  gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse fp_status:", params->client_index);

	guint8 fp_id = parse_BCD(buffer, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fpId: %d", params->client_index, fp_id);

	switch (subcode)
	{
		case 0x00:
			if (fp_id > 0)
			{
				send_fp_status_req_0(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_status_req_0_mult(params, ex_mode, port);
			}
			break;

		case 0x01:
			if (fp_id > 0)
			{
				send_fp_status_req_1(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_status_req_1_mult(params, ex_mode, port);
			}
			break;

		case 0x02:
			if (fp_id > 0)
			{
				send_fp_status_req_2(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_status_req_2_mult(params, ex_mode, port);
			}
			break;

		case 0x03:
			if (fp_id > 0)
			{
				send_fp_status_req_3(fp_id, params, ex_mode, port);
			}
			else
			{
				send_fp_status_req_3_mult(params, ex_mode);
			}
			break;
	}
}

void parse_open_fp(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse open_fp:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 pos_id = parse_BCD(&buffer[pos], 1);pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PosId: %d", params->client_index, pos_id);

	guint8 fp_operation_mode_no = parse_BIN8(&buffer[pos]);pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpOperationModeNo: %d", params->client_index, fp_operation_mode_no);

	PSSFuellingPoint fp = {0x00};

	get_fuelling_point_by_id(fp_id, &fp);

	if (fp.id > 0)
	{
		fp.locked_id = pos_id;
		if (fp_operation_mode_no > 0)
		{
			fp.operation_mode_no = fp_operation_mode_no;
		}
		else
		{
			fp.operation_mode_no = get_fc_operation_mode_no();
		}
		fp.is_close = FALSE;

		update_fp(fp);
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d closed fuelling point(s) id: %d", params->client_index, fp_id);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare close_fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_open_fp, pssc_open_fp_ack, 0x00);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_close_fp(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse close_fp:", params->client_index);

	guint8 fp_id = parse_BCD(buffer, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	close_fp(fp_id);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d closed fuelling point(s) id: %d", params->client_index, fp_id);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare close_fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_close_fp, pssc_close_fp_ack, 0x00);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare close_fp_ack:", params->client_index);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_cancel_estop_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse cancel_estop_Fp:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 pos_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PosId: %d", params->client_index, pos_id);

	resume_fp(fp_id, params);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare cancel_estop_Fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_cancel_estop_Fp, pssc_cancel_estop_Fp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpId: %d", params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);

}

void parse_estop_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse estop_Fp:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	suspend_fp(fp_id, params);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare estop_Fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_estop_Fp, pssc_estop_Fp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpId: %d", params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_reset_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse reset_Fp:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	reset_fp(fp_id, params, pssc_reset_Fp, subcode, ex_mode);
}

void send_reset_fp_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare reset_Fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_reset_Fp, pssc_reset_Fp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void parse_authorize_fp(guint8 subcode, guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse authorize_Fp:", params->client_index);

	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	FpId: %d", params->client_index, fp_id);

	guint8 pos_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PosId: %d", params->client_index, pos_id);

	PSSPresetData pdata = {0x00};

	guint8 param_count = 0;
	PSSAuthParId param_id = 0;

	switch (subcode)
	{
		case 0x00:
			break;

		case 0x01:
			pdata.preset_type = (PSSPresetType)parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Preset type: %d", params->client_index, pdata.preset_type);

			pdata.preset_value = parse_BCD(&buffer[pos], 3); pos+=6;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Preset value: %d", params->client_index, pdata.preset_value);
			break;

		case 0x02:
			param_count = parse_BIN8(&buffer[pos]); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Param count: %d", params->client_index, param_count);
			if (param_count > 0)
			{
				for (guint8 i = 0; i < param_count; i++)
				{
					param_id = parse_BCD(&buffer[pos], 1); pos+=2;

					switch(param_id)
					{
						case papi_SmId:
							pdata.sm_id = parse_BCD(&buffer[pos], 1); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		SmId: %d", params->client_index, pdata.sm_id);
							break;

						case papi_FmgId:
							pdata.fmg_id = parse_BCD(&buffer[pos], 1); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FmgId: %d", params->client_index, pdata.fmg_id);
							break;

						case papi_PgId:
							pdata.pg_id = parse_BCD(&buffer[pos], 1); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PgId: %d", params->client_index, pdata.pg_id);
							break;

						case papi_ValidGrades:
							pdata.valid_grades_count = parse_BIN8(&buffer[pos]); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Valid grades count: %d", params->client_index, pdata.valid_grades_count);
							if (pdata.valid_grades_count > 0)
							{
								for (guint8 j = 0; j < pdata.valid_grades_count; j++)
								{
									pdata.valid_grades[j] = parse_BCD(&buffer[pos], 1); pos+=2;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Grade ID%d", params->client_index, pdata.valid_grades[j] );
								}
							}
							break;

						case papi_StartLimit:
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Start limit:", params->client_index);

							pdata.preset_type = (PSSPresetType)parse_BIN8(&buffer[pos]); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Preset type: %d", params->client_index, pdata.preset_type);
							pdata.preset_value = parse_BCD(&buffer[pos], 3); pos+=6;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Preset value: %d", params->client_index, pdata.preset_value);
							break;

						case papi_FpTransReturnData:
							{//TODO доделать
								guint8 tmp_count = parse_BIN8(&buffer[pos]); pos+=2;
								pos+=tmp_count;
							}
							break;

						case papi_LogData:
							{//TODO доделать
								guint8 log_param_count = parse_BIN8(&buffer[pos]); pos+=2;

								if (log_param_count > 0)
								{
									for (guint8 j = 0; j < log_param_count; j++)
									{
										guint8 log_par_id = parse_BCD(&buffer[pos], 1); pos+=2;
										switch (log_par_id)
										{
											case 1:
												pos+=40;
												break;

											case 2:
												{
													guint8 card_auth_code_len = parse_BIN8(&buffer[pos]); pos+=2;
													pos+=card_auth_code_len * 2;
												}
												break;

											case 3:
												pos+=12;
												break;
										}
									}
								}
							}
							break;

						case papi_AutoLockId:
							pos+=2;//TODO доделать
							break;

						case papi_FpGradePriceDiscount:
							{ //TODO доделать
								guint8 grade_disc_count = parse_BIN8(&buffer[pos]); pos+=2;

								if (grade_disc_count > 0)
								{
									for (guint8 j = 0; j < grade_disc_count; j++)
									{
										//guint8 gr_id = parse_BCD(&buffer[pos], 1);
										pos+=2;
										//guint32 money_disc = parse_BCD(&buffer[pos], 4);
										 pos+=8;
									}
								}
							}
							break;

						case papi_LockFpPrices:
							pos+=2; //здесь всегда 0
							break;


						case papi_StartLimit_e:

							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Start limit E:", params->client_index);

							pdata.preset_type = (PSSPresetType)parse_BIN8(&buffer[pos]); pos+=2;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Preset type: %d", params->client_index, pdata.preset_type);
							pdata.preset_value = parse_BCD(&buffer[pos], 5); pos+=10;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Preset value: %d", params->client_index, pdata.preset_value);
							break;

						case papi_FpGradePriceDiscounts_e:
							{ //TODO доделать
								guint8 grade_disc_count = parse_BIN8(&buffer[pos]); pos+=2;

								if (grade_disc_count > 0)
								{
									for (guint8 j = 0; j < grade_disc_count; j++)
									{
										//guint8 gr_id = parse_BCD(&buffer[pos], 1);
										pos+=2;
										//guint32 money_disc = parse_BCD(&buffer[pos], 6);
										pos+=12;
									}
								}
							}
							break;

						case papi_FpTransReturnData2:
							{//TODO доделать
								guint8 tmp_count = parse_BIN8(&buffer[pos]); pos+=2;
								pos+=tmp_count;
							}
							break;
					}
				}
			}
			break;
	}

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	if ((fp.sub_state & FP_SS_IS_PRESET)  == 0)
	{
		fp.preset_data = pdata;
		fp.sub_state |= FP_SS_IS_PRESET;
		fp.sub_state &= ~FP_SS_IS_E_STOPPED;
		fp.locked_id = pos_id;

		authorize_fp(fp, params, pssc_authorize_Fp, subcode, ex_mode);
	}
	else
	{
		send_rejected_reply(params, ex_mode, pssc_authorize_Fp, subcode);
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare authorize_fp:", params->client_index);
}

void send_authorize_fp_reply(guint8 subcode, PSSClientThreadFuncParam* params, gboolean ex_mode, guint8 fp_id)
{
	LogParams* log_params = params->log_params;
	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare authorize_fp_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_authorize_Fp, pssc_authorize_Fp_ack, subcode);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	fp_id: %d",	params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void send_fc_set_date_time(guint8* buffer,PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare fc_set_date_time:", params->client_index);

	//TODO  Set Date and Time ???

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_DiffComm, pssc_set_FcDateAndTime, 0x00);

	socket_client_send_with_mutex(params,reply,pos);

}
void parse_install_tg_ext_sub_dev(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse install_TgExtSubDev :", params->client_index);

	guint16	pos = 0;

	PSSTankGaugeSubDevice new_tgsd = {0x00};

	new_tgsd.id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgExtSubDevID: %d", params->client_index, new_tgsd.id);

	new_tgsd.interface_type = parse_BIN16_LSB(&buffer[pos]); pos+=4;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	InterfaceType: %d", params->client_index,new_tgsd.interface_type);

	new_tgsd.pss_channel_no = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PssChannelNo: %d", params->client_index,new_tgsd.pss_channel_no);

	guint8 tmp = parse_BIN8(&buffer[pos]);  pos+=2;

	new_tgsd.main_address = tmp & 0x3F;
	new_tgsd.sub_address = (tmp & 0xC0) >> 6;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PhysicalAddress: %d", params->client_index,tmp);

	new_tgsd.phisical_sub_address = parse_BIN8(&buffer[pos]);  pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	PhysicalSubAddress: %d", params->client_index, new_tgsd.phisical_sub_address);

	guint8 param_count = parse_BIN8(&buffer[pos]);  pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Param count: %d", params->client_index,param_count);

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 par_id = parse_BCD(&buffer[pos], 1); pos+=2;

			switch (par_id)
			{
				case 1:
					{
						guint8 ids_count = parse_BIN8(&buffer[pos]);  pos+=2;
						if (ids_count > 0)
						{
							for (guint8 j = 0 ; j < ids_count; j++)
							{
								if (new_tgsd.sub_dev_ids_count < MAX_TGS_SUB_DEV_ID_COUNT)
								{
									new_tgsd.sub_dev_ids[new_tgsd.sub_dev_ids_count] = parse_BCD(&buffer[pos], 1); pos+=2;
									new_tgsd.sub_dev_ids_count++;
								}
							}
						}
					}
					break;
			}
		}
	}

	install_new_tgsd(new_tgsd);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare install_TgExtSubDev_ack:", params->client_index);

	reply[posrepl++] = port_to_apc_code(port);

	if (ex_mode)
	{
		reply[posrepl++] = 0x00;
		reply[posrepl++] = 0x01;
		reply[posrepl++] = 0x86;  //????
	}
	else
	{
		reply[posrepl++] = 0x01;
		reply[posrepl++] = 0x86;
	}
	reply[posrepl++] = 0x00;

	posrepl+=add_bcd_field(&reply[posrepl],new_tgsd.id, 1);

	socket_client_send_with_mutex(params,reply,posrepl);
}

void parse_tgs_tg_parametars_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse TgsTgParameters_req:", params->client_index);

	guint8 pos = 0;

	guint8 subcode = parse_BIN8(&buffer[pos]); pos+=2;

	guint8 tg_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgId: %d", params->client_index, tg_id);

	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		NoTgsTgPars: %d", params->client_index, param_count);

	guint8 ids[MAX_TGS_PARAM_COUNT] = {0x00};

	if (param_count > 0)
	{
		for (guint8 i = 0 ; i < param_count; i++)
		{
			if(i < MAX_TGS_PARAM_COUNT)
			{
				ids[i] = parse_BCD(&buffer[pos], 1); pos+=2;
			}
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare TgsTgParameters:", params->client_index);

	reply[posrepl++] = port_to_apc_code(port);

	if (ex_mode)
	{
		reply[posrepl++] = 0x00;
		reply[posrepl++] = 0x03;
		reply[posrepl++] = 0x86;  //????
	}
	else
	{
		reply[posrepl++] = 0x03;
		reply[posrepl++] = 0x86;
	}
	reply[posrepl++] = subcode;

	posrepl+=add_bcd_field(&reply[posrepl],tg_id, 1);

	reply[posrepl++] = param_count;

	PSSTankGauge tank_gauge = {0x00};
	get_tg_by_id(tg_id, &tank_gauge);

	if (param_count > 0 && tg_id > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			posrepl+=add_bcd_field(&reply[posrepl],ids[i], 1);

			switch(ids[i])
			{
				case pttpi_VR20Point:

					reply[posrepl++] = 0x78;
					reply[posrepl++] = 0x00;

					for (guint8 j = 0; j < 20; j++)
					{
						posrepl+=add_bcd_field(&reply[posrepl],tank_gauge.points[j], 6);
					}
					break;

				case pttpi_TankHeight:
					{
						reply[posrepl++] = 0x03;
						reply[posrepl++] = 0x00;

						posrepl+=add_bcd_field(&reply[posrepl],tank_gauge.tank_height, 3);
					}
					break;
			}
		}
	}

	socket_client_send_with_mutex(params,reply,posrepl);

}

void parse_change_tgs_tg_parametars(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse change_TgsTgParameters:", params->client_index);

	guint16	pos = 0;

	guint8 subcode = parse_BIN8(&buffer[pos]); pos+=2;

	guint8 tg_id = parse_BCD(&buffer[pos], 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgId: %d", params->client_index, tg_id);
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		NoTgsTgPars: %d", params->client_index, param_count);

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 tgs_tg_par_id = parse_BCD(&buffer[pos], 1); pos+=2;
			pos+=4; //length

			switch(tgs_tg_par_id)
			{
				case pttpi_VR20Point:
					for (guint8 j = 0; j < 20; j++)
					{
						guint32 tank_vol =  parse_BCD(&buffer[pos], 6); pos+=12;
						update_tg_point(tg_id, i ,tank_vol);
					}
					break;

				case pttpi_TankHeight:
					{
						guint32 tank_height =  parse_BCD(&buffer[pos], 3); pos+=6;
						update_tg_tank_height(tg_id, tank_height);
					}

					break;
			}
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		TgId: %d", params->client_index, tg_id);
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_TgsTgParameters_ack:", params->client_index);

	reply[posrepl++] = port_to_apc_code(port);

	if (ex_mode)
	{
		reply[posrepl++] = 0x00;
		reply[posrepl++] = 0x02;
		reply[posrepl++] = 0x86;  //????
	}
	else
	{
		reply[posrepl++] = 0x02;
		reply[posrepl++] = 0x86;
	}
	reply[posrepl++] = subcode;

	posrepl+=add_bcd_field(&reply[posrepl],tg_id, 1);

	socket_client_send_with_mutex(params,reply,posrepl);

}

void parse_fp_install_data_req(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	parse fp_install_data_req:", params->client_index);

	guint16	pos = 0;
	guint8 fp_id = parse_BCD(&buffer[pos], 1); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FuellingPointID: %d", params->client_index, fp_id);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare fp_install_data:", params->client_index);

	reply[posrepl++] = port_to_apc_code(port);

	if (ex_mode)
	{
		reply[posrepl++] = 0x00;
		reply[posrepl++] = pssc_DiffComm;
		reply[posrepl++] = 0x87;
	}
	else
	{
		reply[posrepl++] = pssc_DiffComm;
		reply[posrepl++] = 0x87;
	}
	reply[posrepl++] = 0x00;

	posrepl+=add_bcd_field(&reply[posrepl],fp_id, 1);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		FpID: %d", params->client_index, fp_id);

	if(fp.id > 0)
	{
		guint8 param_count =  parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		param count: %d", params->client_index, param_count);

		reply[posrepl++] = param_count;

		if (param_count > 0)
		{
			for (guint8 i = 0; i < param_count; i++)
			{
				guint8 param_id = parse_BCD(&buffer[pos], 1); pos+=2;

				posrepl+=add_bcd_field(&reply[posrepl],param_id, 1);

				switch(param_id)
				{
					case fpipi_PumpInterfaceType:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.interface_type_general & 0xFF;
						reply[posrepl++] = (fp.interface_type_general >> 8) & 0xFF;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Interface type general: %d", params->client_index, fp.interface_type_general);

						break;

					case fpipi_PssChannelNo:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PSS Channel No: %d", params->client_index, fp.pss_port_no);
						reply[posrepl++] = fp.pss_port_no;
						break;

					case fpipi_DeviceCommunicationAddress:
						if (fp.phisical_address_type == 1)
						{
							reply[posrepl++] = 0x05;
							reply[posrepl++] = 0x00;
						}
						else
						{
							reply[posrepl++] = 0x0A;
							reply[posrepl++] = 0x00;
						}
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Device communication address:", params->client_index);

						reply[posrepl++] = fp.pss_ext_protocol_id & 0xFF;    //00
						reply[posrepl++] = (fp.pss_ext_protocol_id >> 8) & 0xFF;   //00
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		PSS Ext protocol ID: %d", params->client_index, fp.pss_ext_protocol_id);

						reply[posrepl++] = fp.phisical_address_type;       //01
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Address type: %d", params->client_index, fp.phisical_address_type);

						if (fp.phisical_address_type == 1)
						{
//							guint8 tmp =  (fp.device_sub_address << 6) | (fp.phisical_address & 0x3F);
							reply[posrepl++] = fp.phisical_address & 0x3F;
							reply[posrepl++] = fp.phisical_address >> 6; //fp.device_sub_address;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Device address: %d, %d", params->client_index, fp.phisical_address, 0);
						}
						else
						{
							reply[posrepl++] = fp.phisical_address & 0xFF;
							reply[posrepl++] = (fp.phisical_address >> 8) & 0xFF;
							reply[posrepl++] = (fp.phisical_address >> 16) & 0xFF;
							reply[posrepl++] = (fp.phisical_address >> 24) & 0xFF;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		IP address: %03d.%03d.%03d.%03d",
									params->client_index,  (fp.phisical_address >> 24) & 0xFF ,(fp.phisical_address >> 16) & 0xFF ,(fp.phisical_address >> 8) & 0xFF ,fp.phisical_address & 0xFF);

							reply[posrepl++] = fp.phisical_address_port & 0xFF;
							reply[posrepl++] = (fp.phisical_address_port >> 8) & 0xFF;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		IP port: %d", params->client_index,  fp.phisical_address_port);

							reply[posrepl++] = fp.device_sub_address;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Sub address: %d", params->client_index,  0);
						}
						break;

					case fpipi_FpGradeOptions:
						{
							guint16 len = 1;

							if (fp.grade_option_count > 0)
							{
								for (guint8 j = 0; j < fp.grade_option_count; j++ )
								{
									len+=3;

									if (fp.grade_options[j].tank_connection_count > 0)
									{
										for (guint8 k = 0; k < fp.grade_options[j].tank_connection_count; k++)
										{
											len+=2;
										}
									}
								}
							}
							reply[posrepl++] = len & 0xFF;
							reply[posrepl++] = (len >> 8) & 0xFF;


							reply[posrepl++] = fp.grade_option_count;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Grade option count: %d", params->client_index,  fp.grade_option_count);

							if (fp.grade_option_count > 0)
							{
								for (guint8 j = 0; j < fp.grade_option_count; j++ )
								{
									reply[posrepl++] = fp.grade_options[j].id;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Grade option ID%d:", params->client_index, fp.grade_options[j].id);

									posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].grade_id, 1);
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Grade ID%d:", params->client_index, fp.grade_options[j].grade_id);

									reply[posrepl++] = fp.grade_options[j].tank_connection_count;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Tanks count: %d:", params->client_index, fp.grade_options[j].tank_connection_count);

									if (fp.grade_options[j].tank_connection_count > 0)
									{
										for (guint8 k = 0; k < fp.grade_options[j].tank_connection_count; k++)
										{
											posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].tank_connections[k].tank_id, 1);
											posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].tank_connections[k].part, 1);
											add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Tank ID: %d part: %d",
													params->client_index, fp.grade_options[j].tank_connections[k].tank_id, fp.grade_options[j].tank_connections[k].part);
										}
									}
								}
							}
						}
						break;

					case fpipi_FpGradeOptionsPars:
						{
							guint16 length = 1;

							if (fp.grade_option_count > 0)
							{
								for (guint8 j = 0; j < fp.grade_option_count; j++ )
								{
									length+=3;
									guint8 NoFpGradeOptionPar = (fp.grade_options[j].tank_connection_count > 0) ? 1 : 0  +
															(strlen(fp.grade_options[j].nozzle_id) > 0) ? 1 : 0 +
															(fp.grade_options[j].nozzle_tag_reader_id > 0) ? 1 : 0;
									if (NoFpGradeOptionPar > 0)
									{
										if (fp.grade_options[j].tank_connection_count > 0)
										{
											length +=4;
											for (guint8 k = 0; k < fp.grade_options[j].tank_connection_count; k++)
											{
												length+=2;
											}
										}
										if (strlen(fp.grade_options[j].nozzle_id) > 0)
										{
											length += 1 + 2 + 1 + strlen(fp.grade_options[j].nozzle_id);

										}
										if (fp.grade_options[j].nozzle_tag_reader_id > 0)
										{
											length +=4;
										}
									}
								}
							}

							reply[posrepl++] = length & 0xFF;
							reply[posrepl++] = (length >> 8) & 0xFF;

							reply[posrepl++] = fp.grade_option_count;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Grade option count: %d", params->client_index, fp.grade_option_count);

							if (fp.grade_option_count > 0)
							{
								for (guint8 j = 0; j < fp.grade_option_count; j++ )
								{
									reply[posrepl++] = fp.grade_options[j].id;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				Grade option ID%d:", params->client_index, fp.grade_options[j].id);

									posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].grade_id, 1);
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Grade ID%d:", params->client_index, fp.grade_options[j].grade_id);

									guint8 NoFpGradeOptionPar = (fp.grade_options[j].tank_connection_count > 0) ? 1 : 0  +
															(fp.grade_options[j].nozzle_id > 0) ? 1 : 0 +
															(fp.grade_options[j].nozzle_tag_reader_id > 0) ? 1 : 0;

									reply[posrepl++] = NoFpGradeOptionPar;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 					Grade option par count: %d", params->client_index, NoFpGradeOptionPar);

									if (NoFpGradeOptionPar > 0)
									{
										if (fp.grade_options[j].tank_connection_count > 0)
										{
											posrepl+=add_bcd_field(&reply[posrepl],1, 1);

											guint16 len = 1 + (2 * fp.grade_options[j].tank_connection_count);

											reply[posrepl++] = len & 0xFF;
											reply[posrepl++] = (len >> 8) & 0xFF;

											reply[posrepl++] = fp.grade_options[j].tank_connection_count;
											add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Tank connections (count %d, len %d):",
													params->client_index, fp.grade_options[j].tank_connection_count, len);

											for (guint8 k = 0; k < fp.grade_options[j].tank_connection_count; k++)
											{
												posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].tank_connections[k].tank_id, 1);
												posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].tank_connections[k].part, 1);

												add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 							Tank ID: %d part %d:",
														params->client_index, fp.grade_options[j].tank_connections[k].tank_id, fp.grade_options[j].tank_connections[k].part);

											}
										}
										if (strlen(fp.grade_options[j].nozzle_id) > 0)
										{
											posrepl+=add_bcd_field(&reply[posrepl],2, 1);

											guint16 l = 1+ strlen(fp.grade_options[j].nozzle_id);

											reply[posrepl++] = l & 0xFF;
											reply[posrepl++] = ( l >> 8 ) & 0xFF;

											reply[posrepl++] = strlen(fp.grade_options[j].nozzle_id);

											memcpy(&reply[posrepl], fp.grade_options[j].nozzle_id, strlen(fp.grade_options[j].nozzle_id));

											posrepl+=strlen(fp.grade_options[j].nozzle_id);

											add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Nozzle ID: %s",
													params->client_index, fp.grade_options[j].nozzle_id);

										}
										if (fp.grade_options[j].nozzle_tag_reader_id > 0)
										{
											posrepl+=add_bcd_field(&reply[posrepl],3, 1);

											reply[posrepl++] = 1;
											reply[posrepl++] = 0;

											posrepl+=add_bcd_field(&reply[posrepl],fp.grade_options[j].nozzle_tag_reader_id, 1);

											add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 						Nozzle tag reader ID: %d",
													params->client_index, fp.grade_options[j].nozzle_tag_reader_id);
										}
									}
								}
							}
						}
						break;

					case fpipi_PumpInterfaceTypeGeneral:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.interface_type_general & 0xFF;
						reply[posrepl++] = (fp.interface_type_general >> 8) & 0xFF;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Interface type general: %d",
								params->client_index, fp.interface_type_general);
						break;

					case fpipi_PumpInterfaceTypeProtocol:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.interface_type_protocol & 0xFF;
						reply[posrepl++] = (fp.interface_type_protocol >> 8) & 0xFF;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Interface type protocol: %d",
								params->client_index, fp.interface_type_protocol);
						break;

					case fpipi_PumpDecimalPositionInMoney:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.pump_interface_decimal_positions.in_money;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in money: %d",
								params->client_index, fp.pump_interface_decimal_positions.in_money);
						break;

					case fpipi_PumpDecimalPositionInVolume:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.pump_interface_decimal_positions.in_volume;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in volume: %d",
								params->client_index, fp.pump_interface_decimal_positions.in_volume);
						break;

					case fpipi_PumpDecimalPositionInPrice:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.pump_interface_decimal_positions.in_price;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in price: %d",
								params->client_index, fp.pump_interface_decimal_positions.in_price);
						break;

					case fpipi_PumpDecimalPositionInMoneyTot:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.pump_interface_decimal_positions.in_money_total;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in money total: %d",
								params->client_index, fp.pump_interface_decimal_positions.in_money_total);
						break;

					case fpipi_PumpDecimalPositionInVolumeTot:
						reply[posrepl++] = 0x01;
						reply[posrepl++] = 0x00;
						reply[posrepl++] = fp.pump_interface_decimal_positions.in_volume_total;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Pump decimal position in volume total: %d",
								params->client_index, fp.pump_interface_decimal_positions.in_volume_total);
						break;

					case fpipi_PumpInterfaceConfigString:
						{
							guint len = 1 + strlen(fp.config_string);

							reply[posrepl++] = len & 0xFF;
							reply[posrepl++] = (len >> 8) & 0xFF;

							reply[posrepl++] = strlen(fp.config_string);
							if (strlen(fp.config_string) > 0)
							{
								memcpy(&reply[posrepl], fp.config_string,  strlen(fp.config_string));
								posrepl+=strlen(fp.config_string);
							}
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Config string: %s",
									params->client_index, fp.config_string);

						}
						break;

					case fpipi_NomimalNormalSpeed:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;

						posrepl+=add_bcd_field(&reply[posrepl],fp.nominal_normal_speed, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Nominal normal speed: %d",
								params->client_index, fp.nominal_normal_speed);
						break;

					case fpipi_NominalHighSpeed:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;
						posrepl+=add_bcd_field(&reply[posrepl],fp.nominal_high_speed, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Nominal high speed: %d",
								params->client_index, fp.nominal_high_speed);
						break;

					case fpipi_HighSpeedTriggerLevel:
						reply[posrepl++] = 0x02;
						reply[posrepl++] = 0x00;
						posrepl+=add_bcd_field(&reply[posrepl],fp.high_speed_trigger_level, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			High speed trigger level: %d",
								params->client_index, fp.high_speed_trigger_level);
						break;

					default:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Undefined param: %d",
								params->client_index, param_id);
						break;
				}

			}

		}
	}
	else
	{
		reply[posrepl++] = 0;
	}
	socket_client_send_with_mutex(params,reply,posrepl);
}

void parse_load_fp_operation_mode_set(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16	pos = 0;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d parse load_fp_operation_mode:", params->client_index);

	guint8 fp_id = parse_BCD(buffer, 1); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d FpId: %d", params->client_index, fp_id);

	PSSFuellingPoint fp = {0x00};
	get_fuelling_point_by_id(fp_id, &fp);

	if(fp.id > 0)
	{
		guint8 operation_mode_count =  parse_BIN8(&buffer[pos]); pos+=2;
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		Operation mode count %d", params->client_index, operation_mode_count);

		if (operation_mode_count > 0)
		{
			for (guint8 i = 0; i < operation_mode_count; i++)
			{
				PSSOperationMode om = {0x00};

				om.id  = parse_BIN8(&buffer[pos]); pos+=2;
				om.type  = parse_BIN8(&buffer[pos]); pos+=2;
				guint8 service_mode_count  = parse_BIN8(&buffer[pos]); pos+=2;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Operation mode ID%d (type %d, service mode count %d)",
						params->client_index, om.id, om.type, service_mode_count);

				if (service_mode_count > 0)
				{
					for (guint8 j = 0; j < service_mode_count; j++)
					{
						guint8 service_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;
						guint8 fuelling_mode_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
						guint8 price_group_id = parse_BCD(&buffer[pos], 1); pos+=2;

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				Service mode ID%d, fuelling mode group ID %d, price group ID %d",
								params->client_index, service_mode_id, fuelling_mode_group_id, price_group_id);

						if (om.service_mode_count < MAX_SERVICE_MODE_COUNT)
						{
							om.service_modes[om.service_mode_count].service_mode_id = service_mode_id;
							om.service_modes[om.service_mode_count].fuelling_mode_group_id = fuelling_mode_group_id;
							om.service_modes[om.service_mode_count].price_group_id = price_group_id;
							om.service_mode_count++;
						}
					}
				}

				update_fp_operation_mode(&fp, om);

				if (om.id == get_fc_operation_mode_no())
				{
					fp_price_update(fp, om.id ,params);
				}
			}
		}
		update_fp(fp);
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare load_FpOperationModeSet_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_load_FpOperationModeSet, pssc_load_FpOperationModeSet_ack, 0x00);

	pos+=add_bcd_field(&reply[pos],fp_id, 1);
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d FpId: %d", params->client_index, fp_id);

	socket_client_send_with_mutex(params,reply,pos);
}

void fc_gen_par_set_reply_ex(guint8* buffer, guint8 length, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare fc_gen_par_set_reply:", params->client_index);

	PSSGeneralParams general_params = {0x00};
	get_general_params(&general_params);

	pos+=add_bcd_field(&reply[pos],fcpgid_GeneralFcParmeters, 1);
	pos+=add_bcd_field(&reply[pos],1, 1);

	reply[pos++] = length;

	if (length > 0)
	{
		for (guint8 i = 0; i < length; i++)
		{
			pos+=add_bcd_field(&reply[pos],buffer[i], 1);

			switch(buffer[i])
			{
				case fcgpi_PriceIncreaseDelay:
					pos+=add_bcd_field(&reply[pos],general_params.price_increase_delay, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.price_increase_delay);
					break;

				case fcgpi_PriceDecreaseDelay:
					pos+=add_bcd_field(&reply[pos],general_params.price_decrease_delay, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.price_decrease_delay);
					break;

				case fcgpi_DefaultLanguageCode:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.default_language_code);
					memcpy(&reply[pos], general_params.default_language_code, MAX_LANG_CODE_LENGTH);
					pos+=MAX_LANG_CODE_LENGTH;
					break;

				case fcgpi_DisableFpTotalsError:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.disable_fp_totals_error);
					reply[pos++] = general_params.disable_fp_totals_error ? 0x01 : 0x00;
					break;

				case fcgpi_EnableDemoEncryption:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.enable_demo_encription);
					reply[pos++] = general_params.enable_demo_encription ? 0x01 : 0x00;
					break;
				case fcgpi_CurrencyCode:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.currency_code);
					memcpy(&reply[pos], general_params.currency_code, MAX_CURRENCY_CODE_LENGTH);
					pos+=MAX_CURRENCY_CODE_LENGTH;
					break;
				case fcgpi_FcPumpTotalsHandlingMode:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.fc_pump_totals_handling_mode);
					reply[pos++] = general_params.fc_pump_totals_handling_mode;
					break;
				case fcgpi_FcShiftNo:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.fc_shift_no);
					memcpy(&reply[pos], general_params.fc_shift_no, MAX_FC_SHIFT_NO_LENGTH);
					pos+=MAX_FC_SHIFT_NO_LENGTH;
					break;
				case fcgpi_SpecificProject:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], 0);
					reply[pos++] = 0x00;
					break;
				case fcgpi_VatRate:
					pos+=add_bcd_field(&reply[pos],general_params.vat_rate, 2);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s (%d): %d",
							params->client_index, fc_gen_par_id_to_str(buffer[i]), buffer[i], general_params.vat_rate);
					break;

			}
		}
	}
	socket_client_send_with_mutex(params,reply,pos);
}

void interpret_change_fuelling_mode_group_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	guint8 fuelling_mode_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			FuellingModeGroupID = %d param_count %d:",
			params->client_index, fuelling_mode_group_id, param_count);

	PSSFuellingModeGroup fuelling_mode_group = {0x00};
	get_fuelling_mode_group_by_id(fuelling_mode_group_id, &fuelling_mode_group);

	for (guint8 i = 0; i < param_count; i++)
	{
		guint8 grade_id = parse_BIN8(&buffer[pos]); pos+=2;

		guint8 fuelling_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;

		for (guint8 j = 0; j < fuelling_mode_group.fuelling_mode_group_item_count; j++)
		{
			if (fuelling_mode_group.fuelling_mode_group_items[j].grade_id == grade_id)
			{
				fuelling_mode_group.fuelling_mode_group_items[j].fuelling_mode_id = fuelling_mode_id;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				Grade ID %d: %d",
						params->client_index, grade_id, fuelling_mode_id);
			}
		}

	}
	set_fuelling_mode_group_by_id(fuelling_mode_group_id, fuelling_mode_group);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	socket_client_send_with_mutex(params,reply,pos);
}

void interpret_change_grade_texts(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	socket_client_send_with_mutex(params,reply,pos);


}

void interpret_change_global_fuelling_limits_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	guint8 fixed_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				FixedID = %d param_count %d:",
			params->client_index, fixed_id, param_count);

	PSSDispenceLimits limits = {0x00};
	get_global_fuelling_limits(&limits);

	for (guint8 i = 0; i < param_count; i++)
	{
		guint8 par_id = parse_BCD(&buffer[pos], 1); pos+=2;

		switch(par_id)
		{
			case fcdlpi_GlobalMoneyLimit:
				limits.global_money_limit = parse_BCD(&buffer[pos], 5); pos+=10;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	%s: %d",
						params->client_index, fc_global_fuelling_limits_par_id_to_str(fcdlpi_GlobalMoneyLimit), limits.global_money_limit);
				break;

			case fcdlpi_GlobalVolumeLimit:
				limits.global_volume_limit = parse_BCD(&buffer[pos], 5); pos+=10;
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	%s: %d",
						params->client_index, fc_global_fuelling_limits_par_id_to_str(fcdlpi_GlobalVolumeLimit), limits.global_volume_limit);
				break;
		}
	}
	set_global_fuelling_limits(limits);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	socket_client_send_with_mutex(params,reply,pos);
}

void interpret_change_fuelling_mode_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	guint8 fuelling_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			FuellingModeID = %d param_count %d:",
			params->client_index, fuelling_mode_id, param_count);

	PSSFuellingMode fuelling_mode = {0x00};
	get_fuelling_mode_by_id(fuelling_mode_id, &fuelling_mode);

	if (fuelling_mode.id != 0 && param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 par_id = parse_BCD(&buffer[pos], 1); pos+=2;

			switch(par_id)
			{
				case fcfmpi_FuellingType:
				//	fuelling_mode.fuelling_type = parse_BIN16_LSB(&buffer[pos]); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d Don't changed!",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_FuellingType), fuelling_mode.fuelling_type);
					break;

				case fcfmpi_MaxTimeToReachMinLimit:
					fuelling_mode.max_time_to_reach_min_limit = parse_BCD(&buffer[pos], 2); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTimeToReachMinLimit), fuelling_mode.max_time_to_reach_min_limit);
					break;

				case fcfmpi_MaxTimeWithoutProgress:
					fuelling_mode.max_time_without_progress = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTimeWithoutProgress), fuelling_mode.max_time_without_progress);
					break;

				case fcfmpi_MaxTransVolume:
					fuelling_mode.max_trans_volume = parse_BCD(&buffer[pos], 3); pos+=6;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransVolume), fuelling_mode.max_trans_volume);
					break;

				case fcfmpi_MaxTransMoney:
					fuelling_mode.max_trans_money = parse_BCD(&buffer[pos], 3); pos+=6;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransMoney), fuelling_mode.max_trans_money);
					break;

				case fcfmpi_MaxFuellingTime:
					fuelling_mode.max_fuelling_time = parse_BCD(&buffer[pos], 2); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxFuellingTime), fuelling_mode.max_fuelling_time);
					break;

				case fcfmpi_MaxPresetVolOverrunErrLimit:
					fuelling_mode.max_preset_vol_overrun_err_limit = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxPresetVolOverrunErrLimit), fuelling_mode.max_preset_vol_overrun_err_limit);
					break;

				case fcfmpi_ClrDisplayDelayTime:
					fuelling_mode.clr_display_delay_time = parse_BCD(&buffer[pos], 2); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_ClrDisplayDelayTime), fuelling_mode.clr_display_delay_time);
					break;

				case fcfmpi_ClrDisplayWhenCurTrDisappear:
					fuelling_mode.clr_display_when_cur_tr_disappear = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_ClrDisplayWhenCurTrDisappear), fuelling_mode.clr_display_when_cur_tr_disappear);
					break;

				case fcfmpi_MinSubPumpRuntimeBeforeStart:
					fuelling_mode.min_sub_pump_runtime_before_start = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MinSubPumpRuntimeBeforeStart), fuelling_mode.min_sub_pump_runtime_before_start);
					break;

				case fcfmpi_MaxTransVolumeE:
					fuelling_mode.max_trans_volume_e = parse_BCD(&buffer[pos], 5); pos+=10;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransVolumeE), fuelling_mode.max_trans_volume_e);
					break;

				case fcfmpi_MaxTransMoneyE:
					fuelling_mode.max_trans_money_e = parse_BCD(&buffer[pos], 5); pos+=10;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransMoneyE), fuelling_mode.max_trans_money_e);
					break;
			}
		}
		set_fuelling_mode_by_id(fuelling_mode_id, fuelling_mode);
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	socket_client_send_with_mutex(params,reply,pos);

}


void interpret_change_service_mode_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	guint8 service_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			ServiceModeID = %d param_count %d:", params->client_index, service_mode_id, param_count);

	PSSServiceMode service_mode = {0x00};
	get_service_mode_by_id(service_mode_id, &service_mode);

	if (service_mode.id != 0 && param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 par_id = parse_BCD(&buffer[pos], 1); pos+=2;

			switch(par_id)
			{
				case fcsmpi_AutoAuthorizeLimit:
					service_mode.auto_autorize_limit = parse_BIN8(&buffer[pos]); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AutoAuthorizeLimit), service_mode.auto_autorize_limit);
					break;

				case fcsmpi_MaxPreAuthorizeTime:
					service_mode.max_pre_auth_time = parse_BCD(&buffer[pos], 2); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MaxPreAuthorizeTime), service_mode.max_pre_auth_time);
					break;

				case fcsmpi_MaxNzLaydownTime:
					service_mode.max_nz_lay_down_time = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MaxNzLaydownTime), service_mode.max_nz_lay_down_time);
					break;

				case fcsmpi_ZeroTransToPos:
					service_mode.zero_trans_to_pos = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_ZeroTransToPos), service_mode.zero_trans_to_pos);
					break;

				case fcsmpi_MoneyDueInTransBufStatus:
					service_mode.money_due_in_trans_buffer_status = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MoneyDueInTransBufStatus), service_mode.money_due_in_trans_buffer_status);
					break;

				case fcsmpi_MinTransVol:
					service_mode.min_trans_vol = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MinTransVol), service_mode.min_trans_vol);
					break;

				case fcsmpi_MinTransMoney:
					service_mode.min_trans_money = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MinTransMoney), service_mode.min_trans_money);
					break;

				case fcsmpi_SupTransBufferSize:
					service_mode.sup_trans_buffer_size = parse_BIN8(&buffer[pos]); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_SupTransBufferSize), service_mode.sup_trans_buffer_size);
					break;

				case fcsmpi_UnsupTransBufferSize:
					service_mode.unsup_trans_buffer_size = parse_BIN8(&buffer[pos]); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_UnsupTransBufferSize), service_mode.unsup_trans_buffer_size);
					break;

				case fcsmpi_StoreAtPreAuthorize:
					service_mode.store_at_pre_authorize = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_StoreAtPreAuthorize), service_mode.store_at_pre_authorize);
					break;

				case fcsmpi_VolInTransBufStatus:
					service_mode.vol_in_trans_buffer_status = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_VolInTransBufStatus), service_mode.vol_in_trans_buffer_status);
					break;

				case fcsmpi_AuthorizeAtModeSelection:
					service_mode.authorize_at_mode_selection = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AuthorizeAtModeSelection), service_mode.authorize_at_mode_selection);
					break;

				case fcsmpi_MnoConsecutiveZeroTrans:
					service_mode.mno_consecutive_zero_trans = parse_BCD(&buffer[pos], 1); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MnoConsecutiveZeroTrans), service_mode.mno_consecutive_zero_trans);
					break;

				case fcsmpi_AutoClearTransDelayTime:
					service_mode.auto_clear_trans_delay_time = parse_BCD(&buffer[pos], 2); pos+=4;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AutoClearTransDelayTime), service_mode.auto_clear_trans_delay_time);
					break;

				case fcsmpi_PumpLightMode:
					service_mode.pump_light_mode = parse_BIN8(&buffer[pos]); pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_PumpLightMode), service_mode.pump_light_mode);
					break;

				case fcsmpi_StopFpOnVehicleTag:
					service_mode.stop_fp_on_vehicle_tag = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_StopFpOnVehicleTag), service_mode.stop_fp_on_vehicle_tag);
					break;

				case fcsmpi_UseVehicleTagReadingButton:
					service_mode.use_vehicle_tag_reading_button = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
					pos+=2;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
							params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_UseVehicleTagReadingButton), service_mode.use_vehicle_tag_reading_button);
					break;

			}
		}
		set_service_mode_by_id(service_mode_id, service_mode);
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	socket_client_send_with_mutex(params,reply,pos);
}


void interpret_change_fc_parameters(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;
	guint8 fc_par_set_id = parse_BCD(&buffer[pos], 1); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		fc_par_set_id %d", params->client_index, fc_par_set_id);

	guint8 no_fc_pars = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		no_fc_pars %d:", params->client_index, no_fc_pars);

	if (fc_par_set_id == 0x01) // general params
	{
		PSSGeneralParams general_params = {0x00};
		get_general_params(&general_params);

		if (no_fc_pars > 0)
		{
			for (guint8 i = 0; i < no_fc_pars; i++)
			{
				guint8 fc_par_id = parse_BCD(&buffer[pos], 1); pos+=2;

				switch(fc_par_id)
				{
					case fcgpi_PriceIncreaseDelay:
						general_params.price_increase_delay = parse_BCD(&buffer[pos], 2); pos+=4;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_PriceIncreaseDelay), general_params.price_increase_delay);
						break;

					case fcgpi_PriceDecreaseDelay:
						general_params.price_decrease_delay = parse_BCD(&buffer[pos], 2); pos+=4;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_PriceDecreaseDelay), general_params.price_decrease_delay);
						break;

					case fcgpi_DefaultLanguageCode:
						pos+=parse_pss_ascii((guchar*)general_params.default_language_code, &buffer[pos], MAX_LANG_CODE_LENGTH);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_DefaultLanguageCode), general_params.default_language_code);
						break;

					case fcgpi_DisableFpTotalsError:
						general_params.disable_fp_totals_error = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
						pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_DisableFpTotalsError), general_params.disable_fp_totals_error);
						break;

					case fcgpi_EnableDemoEncryption:
						general_params.enable_demo_encription = parse_BIN8(&buffer[pos]) == 0 ? FALSE : TRUE;
						pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_EnableDemoEncryption), general_params.enable_demo_encription);
						break;

					case fcgpi_CurrencyCode:
						pos+=parse_pss_ascii((guchar*)general_params.currency_code, &buffer[pos], MAX_CURRENCY_CODE_LENGTH);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_CurrencyCode), general_params.currency_code);
						break;

					case fcgpi_FcPumpTotalsHandlingMode:
						general_params.fc_pump_totals_handling_mode = parse_BIN8(&buffer[pos]); pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_FcPumpTotalsHandlingMode), general_params.fc_pump_totals_handling_mode);
						break;

					case fcgpi_FcShiftNo:
						pos+=parse_pss_ascii((guchar*)general_params.fc_shift_no, &buffer[pos], MAX_FC_SHIFT_NO_LENGTH);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_gen_par_id_to_str(fcgpi_FcShiftNo), general_params.fc_shift_no);
						break;

					case fcgpi_SpecificProject:
						pos+=2;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index,  fc_gen_par_id_to_str(fcgpi_SpecificProject), 0);
						break;

					case fcgpi_VatRate:
						general_params.vat_rate = parse_BCD(&buffer[pos], 2); pos+=4;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index,  fc_gen_par_id_to_str(fcgpi_VatRate), general_params.vat_rate);
						break;

				}
			}
			set_general_params(general_params);
		}
	}

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	prepare change_FcParameter_ack:", params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	pos = prepare_pss_reply_header(reply, port, ex_mode, pssc_change_FcParameters, pssc_change_FcParameter_ack, 0x00);

	socket_client_send_with_mutex(params,reply,pos);
}


void interpret_fuelling_mode_group_param_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;
	guint8 fuelling_mode_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Fuel mode group ID %d (param count = %d)",
				params->client_index,  fuelling_mode_group_id, param_count);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare pssc_FcParameterSet frame:",
				params->client_index);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

	posrepl+=add_bcd_field(&reply[posrepl],fcpgid_FuellingModeGroups, 1);
	posrepl+=add_bcd_field(&reply[posrepl],fuelling_mode_group_id, 1);

	PSSFuellingModeGroup fuelling_mode_group = {0x00};
	get_fuelling_mode_group_by_id(fuelling_mode_group_id, &fuelling_mode_group);

	if (param_count > 0)
	{
		reply[posrepl++] = param_count;

		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 grade_id = parse_BCD(&buffer[pos], 1); pos+=2;

			posrepl += add_bcd_field(&reply[posrepl], grade_id, 1);

			gboolean flag_found = FALSE;
			for (guint8 j = 0; j < fuelling_mode_group.fuelling_mode_group_item_count; j++)
			{
				if (fuelling_mode_group.fuelling_mode_group_items[j].grade_id == grade_id)
				{
					posrepl += add_bcd_field(&reply[posrepl], fuelling_mode_group.fuelling_mode_group_items[j].fuelling_mode_id, 1);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Grade %d: %d",
								params->client_index, grade_id, fuelling_mode_group.fuelling_mode_group_items[j].fuelling_mode_id);
					flag_found = TRUE;
				}
			}
			if (!flag_found)
			{
				posrepl += add_bcd_field(&reply[posrepl], 0, 1);
			}
		}
	}
	else
	{
		reply[posrepl++] = fuelling_mode_group.fuelling_mode_group_item_count;

		for (guint8 i = 0; i < fuelling_mode_group.fuelling_mode_group_item_count; i++)
		{
			posrepl += add_bcd_field(&reply[posrepl], fuelling_mode_group.fuelling_mode_group_items[i].grade_id, 1);
			posrepl += add_bcd_field(&reply[posrepl], fuelling_mode_group.fuelling_mode_group_items[i].fuelling_mode_id, 1);

			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Grade %d: %d",
						params->client_index, fuelling_mode_group.fuelling_mode_group_items[i].grade_id, fuelling_mode_group.fuelling_mode_group_items[i].fuelling_mode_id);
		}
	}
	socket_client_send_with_mutex(params,reply,posrepl);
}

void interpret_global_fuelling_limits_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;
	guint8 fixed_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare pssc_FcParameterSet frame:",
				params->client_index);

	PSSDispenceLimits limits = {0x00};
	get_global_fuelling_limits(&limits);

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

	posrepl += add_bcd_field(&reply[posrepl], fcpgid_GlobalFuellingLimits, 1);
	posrepl += add_bcd_field(&reply[posrepl], fixed_id, 1);

	reply[posrepl++] = param_count;

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count; i++)
		{
			guint8 param_id = parse_BCD(&buffer[pos], 1); pos+=2;

			posrepl += add_bcd_field(&reply[posrepl], param_id, 1);

			switch(param_id)
			{
				case fcdlpi_GlobalMoneyLimit:
					posrepl += add_bcd_field(&reply[posrepl], limits.global_money_limit, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index, fc_global_fuelling_limits_par_id_to_str(fcdlpi_GlobalMoneyLimit), limits.global_money_limit);
					break;

				case fcdlpi_GlobalVolumeLimit:
					posrepl += add_bcd_field(&reply[posrepl], limits.global_volume_limit, 5);
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 				%s: %d",
								params->client_index,fc_global_fuelling_limits_par_id_to_str(fcdlpi_GlobalVolumeLimit), limits.global_volume_limit);
					break;

			}
		}
	}
	socket_client_send_with_mutex(params,reply,posrepl);
}

void interpret_grade_texts_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;

	guint8 grade_text_group_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	guint8 par_ids[MAX_NOZZLE_COUNT] = {0x00};

	if (param_count > 0)
	{
		for (guint8 i = 0; i < param_count && i < MAX_NOZZLE_COUNT; i++)
		{
			par_ids[i] = parse_BCD(&buffer[pos], 1); pos+=2;
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		ParamID: %d", params->client_index, par_ids[i]);
		}
	}

	guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
	guint16 posrepl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

	posrepl += add_bcd_field(&reply[posrepl], fcpgid_GradeTexts, 1);
	posrepl += add_bcd_field(&reply[posrepl], grade_text_group_id, 1);

	guint8 field_length = 0;
	switch(grade_text_group_id)
	{
		case 1:
			field_length = 12;
			break;

		case 2:
			field_length = 40;
			break;

		case 3:
			field_length = 3;
			break;

		case 4:
			field_length = 0;
			break;

		case 5:
			field_length = 8;
			break;

		default:
			field_length = 0;
			break;

	}

	PSSGeneralFunctions gf = {0x00};

	get_fc_general_functions(&gf);


	if (param_count == 0)
	{
		reply[posrepl++] = gf.grade_count;

		for (guint8 i = 0; i < gf.grade_count; i++)
		{
			posrepl += add_bcd_field(&reply[posrepl], gf.grades[i].id, 1);

			if (field_length > 0)
			{
				memset(&reply[posrepl], 0x00, field_length);
				if (gf.grades[i].name!=NULL)
				{
					memcpy(&reply[posrepl], gf.grades[i].name, MIN(strlen(gf.grades[i].name), field_length - 1));
				}
				posrepl+=field_length;
			}
			else
			{
				reply[posrepl++] = 0x30;
				reply[posrepl++] = 0x30;
				reply[posrepl++] = 0x30;
				reply[posrepl++] = 0x30;
				reply[posrepl++] = 0x30;
				reply[posrepl++] = 0x30;
			}
		}
	}
	else
	{
		reply[posrepl++] = param_count;

		for (guint8 i = 0; i < param_count && i < MAX_NOZZLE_COUNT; i++)
		{
			posrepl += add_bcd_field(&reply[posrepl], par_ids[i], 1);

			for (guint8 j = 0; j < gf.grade_count; j++)
			{
				if (gf.grades[j].id == par_ids[i])
				{
					if (field_length > 0)
					{
						memset(&reply[posrepl], 0x00, field_length);
						if (gf.grades[j].name!=NULL)
						{
							memcpy(&reply[posrepl], gf.grades[j].name, MIN(strlen(gf.grades[j].name), field_length - 1));
						}
						posrepl+=field_length;
					}
					else
					{
						reply[posrepl++] = 0x30;
						reply[posrepl++] = 0x30;
						reply[posrepl++] = 0x30;
						reply[posrepl++] = 0x30;
						reply[posrepl++] = 0x30;
						reply[posrepl++] = 0x30;
					}

				}
			}

		}
	}
	socket_client_send_with_mutex(params,reply,posrepl);
}

void interpret_fuelling_mode_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;
	guint8 fuelling_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;
	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			Fuelling mode ID %d (param count %d)",
			params->client_index, fuelling_mode_id, param_count);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FcParameterSet:",
			params->client_index);


	PSSFuellingMode fuelling_mode = {0x00};
	get_fuelling_mode_by_id(fuelling_mode_id, &fuelling_mode);

	if (fuelling_mode.id != 0)
	{
		guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
		guint16 posrepl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

		posrepl += add_bcd_field(&reply[posrepl], fcpgid_FuellingModes, 1);
		posrepl += add_bcd_field(&reply[posrepl], fuelling_mode_id, 1);

		reply[posrepl++] = param_count;

		if (param_count > 0)
		{
			for (guint8 i = 0; i < param_count; i++)
			{
				guint8 param_id = parse_BCD(&buffer[pos], 1); pos+=2;

				posrepl += add_bcd_field(&reply[posrepl], param_id, 1);

				switch(param_id)
				{
					case fcfmpi_FuellingType:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_FuellingType), fuelling_mode.fuelling_type);
						reply[posrepl++] = (fuelling_mode.fuelling_type >> 8) & 0xFF;
						reply[posrepl++] = fuelling_mode.fuelling_type & 0xFF;;
						break;

					case fcfmpi_MaxTimeToReachMinLimit:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_time_to_reach_min_limit, 2);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTimeToReachMinLimit), fuelling_mode.max_time_to_reach_min_limit);
						break;

					case fcfmpi_MaxTimeWithoutProgress:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_time_without_progress, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTimeWithoutProgress), fuelling_mode.max_time_without_progress);
						break;

					case fcfmpi_MaxTransVolume:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_trans_volume, 3);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransVolume), fuelling_mode.max_trans_volume);
						break;

					case fcfmpi_MaxTransMoney:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_trans_money, 3);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransMoney), fuelling_mode.max_trans_money);
						break;

					case fcfmpi_MaxFuellingTime:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_fuelling_time, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxFuellingTime), fuelling_mode.max_fuelling_time);
						break;

					case fcfmpi_MaxPresetVolOverrunErrLimit:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_preset_vol_overrun_err_limit, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxPresetVolOverrunErrLimit), fuelling_mode.max_preset_vol_overrun_err_limit);
						break;

					case fcfmpi_ClrDisplayDelayTime:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.clr_display_delay_time, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_ClrDisplayDelayTime), fuelling_mode.clr_display_delay_time);
						break;

					case fcfmpi_ClrDisplayWhenCurTrDisappear:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_ClrDisplayWhenCurTrDisappear), fuelling_mode.clr_display_when_cur_tr_disappear);
						reply[posrepl++] = fuelling_mode.clr_display_when_cur_tr_disappear ? 0x01 : 0x00;
						break;

					case fcfmpi_MinSubPumpRuntimeBeforeStart:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.min_sub_pump_runtime_before_start, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MinSubPumpRuntimeBeforeStart), fuelling_mode.min_sub_pump_runtime_before_start);
						break;

					case fcfmpi_MaxTransVolumeE:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_trans_volume_e, 5);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransVolumeE), fuelling_mode.max_trans_volume_e);
						break;

					case fcfmpi_MaxTransMoneyE:
						posrepl += add_bcd_field(&reply[posrepl], fuelling_mode.max_trans_money_e, 5);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_fuelling_mode_par_id_to_str(fcfmpi_MaxTransMoneyE), fuelling_mode.max_trans_money_e);
						break;

				}
			}
		}
		socket_client_send_with_mutex(params,reply,posrepl);
	}
	else
	{
		send_rejected_reply(params, ex_mode, pssc_FcParameterSet, 0x00);
	}
}


void interpret_service_mode_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint32 port = get_client_port(params->client_index);
	guint16 pos = 0;
	guint8 service_mode_id = parse_BCD(&buffer[pos], 1); pos+=2;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			SmId: %d",
			params->client_index, service_mode_id);

	guint8 param_count = parse_BIN8(&buffer[pos]); pos+=2;
	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			ParamCount: %d",
			params->client_index, param_count);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d prepare FcParameterSet:",
			params->client_index);

	PSSServiceMode service_mode = {0x00};
	get_service_mode_by_id(service_mode_id, &service_mode);

	if (service_mode.id != 0)
	{
		guint8 reply[EXCHANGE_BUFFER_SIZE] = {0x00};
		guint16 posrepl = prepare_pss_reply_header(reply, port, ex_mode, pssc_FcParameterSet_req, pssc_FcParameterSet, 0x00);

		posrepl += add_bcd_field(&reply[posrepl], fcpgid_ServiceModes, 1);
		posrepl += add_bcd_field(&reply[posrepl], service_mode.id, 1);

		reply[posrepl++] = param_count;

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 	Service mode ID%d:",
				params->client_index, service_mode.id);

		if (param_count > 0)
		{
			for (guint8 i = 0; i < param_count; i++)
			{
				guint8 sm_par_id = parse_BCD(&buffer[pos], 1); pos+=2;

				posrepl += add_bcd_field(&reply[posrepl], sm_par_id, 1);

				switch(sm_par_id)
				{
					case fcsmpi_AutoAuthorizeLimit:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AutoAuthorizeLimit), service_mode.auto_autorize_limit);
						reply[posrepl++] = service_mode.auto_autorize_limit;
						break;

					case fcsmpi_MaxPreAuthorizeTime:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.max_pre_auth_time, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MaxPreAuthorizeTime), service_mode.max_pre_auth_time);
						break;

					case fcsmpi_MaxNzLaydownTime:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.max_nz_lay_down_time, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MaxNzLaydownTime), service_mode.max_nz_lay_down_time);
						break;

					case fcsmpi_ZeroTransToPos:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_ZeroTransToPos), service_mode.zero_trans_to_pos);
						reply[posrepl++] = service_mode.zero_trans_to_pos ? 0x01 : 0x00;
						break;

					case fcsmpi_MoneyDueInTransBufStatus:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MoneyDueInTransBufStatus), service_mode.money_due_in_trans_buffer_status);
						reply[posrepl++] = service_mode.money_due_in_trans_buffer_status ? 0x01 : 0x00;
						break;

					case fcsmpi_MinTransVol:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.min_trans_vol, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MinTransVol), service_mode.min_trans_vol);
						break;

					case fcsmpi_MinTransMoney:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.min_trans_money, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MinTransMoney), service_mode.min_trans_money);
						break;

					case fcsmpi_SupTransBufferSize:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_SupTransBufferSize), service_mode.sup_trans_buffer_size);
						reply[posrepl++] = service_mode.sup_trans_buffer_size;
						break;

					case fcsmpi_UnsupTransBufferSize:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_UnsupTransBufferSize), service_mode.unsup_trans_buffer_size);
						reply[posrepl++] = service_mode.unsup_trans_buffer_size;
						break;

					case fcsmpi_StoreAtPreAuthorize:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_StoreAtPreAuthorize), service_mode.store_at_pre_authorize);
						reply[posrepl++] = service_mode.store_at_pre_authorize ? 0x01 : 0x00;
						break;

					case fcsmpi_VolInTransBufStatus:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_VolInTransBufStatus), service_mode.vol_in_trans_buffer_status);
						reply[posrepl++] = service_mode.vol_in_trans_buffer_status ? 0x01 : 0x00;
						break;

					case fcsmpi_AuthorizeAtModeSelection:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AuthorizeAtModeSelection), service_mode.authorize_at_mode_selection);
						reply[posrepl++] = service_mode.authorize_at_mode_selection ? 0x01 : 0x00;
						break;

					case fcsmpi_MnoConsecutiveZeroTrans:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.mno_consecutive_zero_trans, 1);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_MnoConsecutiveZeroTrans), service_mode.mno_consecutive_zero_trans);
						break;

					case fcsmpi_AutoClearTransDelayTime:
						posrepl += add_bcd_field(&reply[posrepl], service_mode.auto_clear_trans_delay_time, 2);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_AutoClearTransDelayTime), service_mode.auto_clear_trans_delay_time);
						break;

					case fcsmpi_PumpLightMode:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_PumpLightMode), service_mode.pump_light_mode);
						reply[posrepl++] = service_mode.pump_light_mode;
						break;

					case fcsmpi_StopFpOnVehicleTag:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_StopFpOnVehicleTag), service_mode.stop_fp_on_vehicle_tag);
						reply[posrepl++] = service_mode.stop_fp_on_vehicle_tag ? 0x01 : 0x00;
						break;

					case fcsmpi_UseVehicleTagReadingButton:
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 		%s: %d",
								params->client_index, fc_serv_mode_par_id_to_str(fcsmpi_UseVehicleTagReadingButton), service_mode.use_vehicle_tag_reading_button);
						reply[posrepl++] = service_mode.use_vehicle_tag_reading_button ? 0x01 : 0x00;
						break;
				}
			}
		}
		socket_client_send_with_mutex(params,reply,posrepl);
	}
	else
	{
		send_rejected_reply(params, ex_mode, pssc_FcParameterSet, 0x00);
	}

}

void interpret_general_fc_parameters_request(guint8* buffer, PSSClientThreadFuncParam* params, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	guint16 pos = 0;

	guint8 fc_par_set_id = parse_BCD(&buffer[pos], 1); pos+=2;

	guint8 no_fc_pars = parse_BIN8(&buffer[pos]); pos+=2;

	guint8 param_ids[MAX_PARAM_COUNT] = {0x00};

	if (no_fc_pars > 0)
	{
		for (guint8 i = 0; i < no_fc_pars; i++)
		{
			if (i < MAX_PARAM_COUNT)
			{
				param_ids[i] = parse_BCD(&buffer[pos], 1);
			}
			pos+=2;
		}
	}

	switch (fc_par_set_id)
	{
		case 1: //FcGenParSet
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			FcGenParSet (code 1, count %d):",
					params->client_index, no_fc_pars);
			fc_gen_par_set_reply_ex(param_ids, no_fc_pars, params, ex_mode);
			break;

		case 2: //PSSData
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d 			PSSData (code 2):",
					params->client_index);
			break;

		case 3: //DialerConfig
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "%d 			DialerConfig (code 3):",
					params->client_index);
			break;

		case 4: //PBS_TT_Conf
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "%d 			PBS_TT_Conf (code 4):",
					params->client_index);
			break;

		case 5: //SiteReportConfig
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "%d 			SiteReportConfig (code 5):",
					params->client_index);
			break;

		case 6: //FcOperationModeName
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "%d 			FcOperationModeName (code 6):",
					params->client_index);
			break;
	}

}

