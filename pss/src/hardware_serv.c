#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include <dlfcn.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_data.h"
#include "pss_tlv.h"
#include "hardware_serv.h"
#include "pss_func.h"

HwServerExchangeState hs_state = hses_GetDcConf;
gboolean command_sended = FALSE;
gboolean connected = FALSE;
guint32	 message_id = 1;
guint8	server_device_id = 1;

GMutex hs_mutex;

void init_hs_mutex()
{
	g_mutex_init(&hs_mutex);
}

void set_hs_state(HwServerExchangeState new_value)
{
	g_mutex_lock(&hs_mutex);

	hs_state = new_value;

	g_mutex_unlock(&hs_mutex);
}

HwServerExchangeState get_hs_state()
{
	HwServerExchangeState result = hses_GetDcConf;

	g_mutex_lock(&hs_mutex);

	result = hs_state;

	g_mutex_unlock(&hs_mutex);

	return result;
}

void set_command_sended(gboolean new_value)
{
	g_mutex_lock(&hs_mutex);

	command_sended = new_value;

	g_mutex_unlock(&hs_mutex);
}

gboolean get_command_sended()
{
	gboolean result = FALSE;

	g_mutex_lock(&hs_mutex);

	result = command_sended;

	g_mutex_unlock(&hs_mutex);

	return result;
}
void set_connected(gboolean new_value)
{
	g_mutex_lock(&hs_mutex);

	connected = new_value;

	g_mutex_unlock(&hs_mutex);
}

gboolean get_connected()
{
	gboolean result = FALSE;

	g_mutex_lock(&hs_mutex);

	result = connected;

	g_mutex_unlock(&hs_mutex);

	return result;
}

void set_message_id(guint32 new_value)
{
	g_mutex_lock(&hs_mutex);

	message_id = new_value;

	g_mutex_unlock(&hs_mutex);
}

guint32 get_message_id()
{
	guint32 result = 0;

	g_mutex_lock(&hs_mutex);

	result = message_id;

	g_mutex_unlock(&hs_mutex);

	return result;
}

void inc_message_id()
{
	g_mutex_lock(&hs_mutex);

	message_id++;

	g_mutex_unlock(&hs_mutex);
}

void set_server_device_id(guint8 new_value)
{
	g_mutex_lock(&hs_mutex);

	server_device_id = new_value;

	g_mutex_unlock(&hs_mutex);
}

guint8 get_server_device_id()
{
	guint32 result = 0;

	g_mutex_lock(&hs_mutex);

	result = server_device_id;

	g_mutex_unlock(&hs_mutex);

	return result;
}

void inc_server_device_id()
{
	g_mutex_lock(&hs_mutex);

	server_device_id++;

	g_mutex_unlock(&hs_mutex);
}

void send_buffer(gint32 sock, LogParams* log_params, gboolean log_trace, gboolean log_enable, guint8* buffer ,guint32 size)
{
	send(sock, buffer, size, 0);

	add_log(log_params, TRUE, FALSE,log_trace, log_enable, " >>");

	for (guint16 i = 0; i < size; i++)
	{
		add_log(log_params, FALSE, FALSE, log_trace, log_enable, " %02X", buffer[i]);
	}
	add_log(log_params, FALSE, TRUE, log_trace, log_enable, "");

}


void send_config_request_message(gint32 sock, PSSServConfThreadParam* param, guint32 message_id, HardwareServerCommand command)
{
	TlvUnit* units = NULL;

	tlv_add_unit(&units,tlv_create_unit(tst_GuidClient , (guchar*)param->guid, 0, strlen(param->guid)));

	guchar message_id_buffer[] = {(message_id >> 24) & 0xFF, (message_id >> 16) & 0xFF, (message_id >> 8) & 0xFF, message_id & 0xFF};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageId , message_id_buffer, 0, sizeof(message_id_buffer)));

	guchar message_type_buffer[] = {mt_Request};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));

	guchar command_code_buffer[] = {(command >> 24) & 0xFF, (command >> 16) & 0xFF, (command >> 8) & 0xFF, command & 0xFF};
	tlv_add_unit(&units,tlv_create_unit(tst_CommandCode , command_code_buffer, 0, sizeof(command_code_buffer)));

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		send_buffer(sock, param->log_params, param->log_trace, param->log_enable, frame, size);


		free(frame);
	}

	tlv_delete_units(&units);
}

void send_short_serv_message(PSSServConfReadThreadParam* param, MessageType message_type, ExchangeError exchange_error)
{

	TlvUnit* units = NULL;

	guchar message_type_buffer[] = {message_type};
	tlv_add_unit(&units,tlv_create_unit(tst_MessageType , message_type_buffer, 0, sizeof(message_type_buffer)));
//	add_log(&params.log, params.plog_mutex, port_to_str(port), TRUE, TRUE, params.log_trace, params.log_enable, "client %d device %d 	add message type: %s",
//			params.client_index, device_index, return_message_type_name(message_type));

	guchar error_buffer[] = {exchange_error};
	tlv_add_unit(&units,tlv_create_unit(tst_Error , error_buffer, 0, sizeof(error_buffer)));

//	add_log(&params.log, params.plog_mutex, port_to_str(port), TRUE, TRUE, params.log_trace, params.log_enable, "client %d device %d 	add exchange error: %d",
//			params.client_index, device_index, exchange_error);

	guint32 size = 0;
	guchar* frame =  tlv_create_transport_frame(units, &size);

	if (frame!=NULL && size > 0)
	{
		send_buffer(param->sock, param->log_params, param->log_trace, param->log_enable, frame, size);


		free(frame);
	}

	tlv_delete_units(&units);

}

ExchangeError ParseModuleSettingsLogSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Log settings:");

	gboolean enable = FALSE;
	gchar* directory = NULL;
	gboolean trace = FALSE;
	gboolean system = FALSE;
	gboolean request = FALSE;
	gboolean frames = FALSE;
	gboolean parsing = FALSE;
	guint32 file_size = 0;
	guint32 save_days = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmlst_Enable:
					if (unit->length == sizeof(guint8))
					{
						enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Enable: %d", enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Enable parse error");
					}
					break;

				case tmlst_Directory:
					if (unit->length > 0)
					{
						if (directory !=NULL)
						{
							g_free(directory);
							directory = NULL;
						}
						directory = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (directory !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		 		Directory: %s",	directory);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Directory parse error");
						result = ee_Parse;
					}
					break;

				case tmlst_Trace:
					if (unit->length == sizeof(guint8))
					{
						trace = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Trace: %d", trace);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Trace parse error");
					}
					break;

				case tmlst_System:
					if (unit->length == sizeof(guint8))
					{
						system = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				System: %d", system);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				System parse error");
					}
					break;

				case tmlst_Requests:
					if (unit->length == sizeof(guint8))
					{
						request = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Request: %d", request);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Request parse error");
					}
					break;

				case tmlst_Frames:
					if (unit->length == sizeof(guint8))
					{
						frames = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Frames: %d", frames);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Frames parse error");
					}
					break;

				case tmlst_Parsing:
					if (unit->length == sizeof(guint8))
					{
						parsing = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Parsing: %d", parsing);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Parsing parse error");
					}
					break;

				case tmlst_FileSize:
					if (unit->length == sizeof(guint32))
					{
						file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				File size: %d", file_size);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				File size parse error");
					}
					break;

				case tmlst_SaveDays:
					if (unit->length == sizeof(guint32))
					{
						save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Save days: %d", save_days);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Save days parse error");
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (directory !=NULL)
	{
		g_free(directory);
		directory = NULL;
	}
	return result;
}

ExchangeError ParseModuleSettingsConnSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Log settings:");

	guint8 type = 0;
	gchar* port = NULL;
	gchar* ip_address = NULL;
	guint16 ip_port = 0;
	guint32 uart_baudrate = 0;
	guint8 uart_byte_size = 0;
	gchar* uart_parity = NULL;
	guint8 uart_stop_bits = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmcst_Type:
					if (unit->length == sizeof(guint8))
					{
						type = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Type: %d",	 type);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Type parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_Port:
					if (unit->length > 0)
					{
						if (port !=NULL)
						{
							g_free(port);
							port = NULL;
						}
						port = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (port !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 			Port: %s",	port);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Port parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_IPAddress:
					if (unit->length > 0)
					{
						if (ip_address !=NULL)
						{
							g_free(ip_address);
							ip_address = NULL;
						}
						ip_address = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (ip_address !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 			IP address: %s",	ip_address);
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			IP address parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_IPPort:
					if (unit->length == sizeof(guint16))
					{
						ip_port = (unit->value[0] << 8 ) |  unit->value[1];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			IP Port: %d",	 ip_port);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			IP Port parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_UartBaudrate:
					if (unit->length == sizeof(guint32))
					{
						uart_baudrate = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart baudrate: %d",	 uart_baudrate);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart speed parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_UartByteSize:
					if (unit->length == sizeof(guint8))
					{
						uart_byte_size = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart byte size: %d",	 type);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart byte size parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_UartParity:
					if (unit->length > 0)
					{
						if (uart_parity !=NULL)
						{
							g_free(uart_parity);
							uart_parity = NULL;
						}
						uart_parity = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (uart_parity !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 			Uart parity: %s",	uart_parity);
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart parity parse error");
						result = ee_Parse;
					}
					break;

				case tmcst_UartStopBits:
					if (unit->length == sizeof(guint8))
					{
						uart_stop_bits = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart stop bits: %d",	 type);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Uart stop bits parse error");
						result = ee_Parse;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (port !=NULL)
	{
		g_free(port);
		port = NULL;
	}
	if (ip_address !=NULL)
	{
		g_free(ip_address);
		ip_address = NULL;
	}

	return result;
}

ExchangeError ParseModuleSettingsTimeoutSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Timeout settings:");

	guint32 timeout_read = 0;
	guint32 timeout_write = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmtst_Read:
					if (unit->length == sizeof(guint32))
					{
						timeout_read = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Timeout read: %d", timeout_read);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Timeout read parse error");
					}
					break;

				case tmtst_Write:
					if (unit->length == sizeof(guint32))
					{
						timeout_write = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Timeout write : %d", timeout_write);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Timeout write parse error");
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseDispencerControllerModuleSettingsDpSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Decimal point settings:");

	guint8 price_dp = 0;
	guint8 volume_dp = 0;
	guint8 amount_dp = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcmdpst_Price:
					if (unit->length == sizeof(guint8))
					{
						price_dp = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Price decimal point: %d", price_dp);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Price decimal point parse error");
					}
					break;

				case tdcmdpst_Volume:
					if (unit->length == sizeof(guint8))
					{
						volume_dp = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Volume decimal point: %d", volume_dp);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Volume decimal point parse error");
					}
					break;

				case tdcmdpst_Amount:
					if (unit->length == sizeof(guint8))
					{
						amount_dp = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Amount decimal point: %d", amount_dp);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Amount decimal point parse error");
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParsePricePoleControllerModuleSettingsDpSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Decimal point settings:");

	guint8 price_dp = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmppcdpst_Price:
					if (unit->length == sizeof(guint8))
					{
						price_dp = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Price decimal point: %d", price_dp);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 			Price decimal point parse error");
					}
					break;



				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseDispencerControllerModuleSettingsNozzleFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"					Nozzle:");

	guint8 num = 0;
	guint8 grade = 0;


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmnct_Number:
					if (unit->length == sizeof(guint8))
					{
						num = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Number: %d", num);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Number parse error");
					}
					break;


				case tmnct_Grade:
					if (unit->length == sizeof(guint8))
					{
						grade = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Grade: %d", grade);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Grade parse error");
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseDispencerControllerModuleSettingsDispencerFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"				Dispencer:");

	guint32 num = 0;
	guint8 address = 0;
	guint8 nozzle_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmdct_Number:
					if (unit->length == sizeof(guint32))
					{
						num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number: %d", num);

						if (server_device->units.count < MAX_SERVER_DEVICE_UNIT_COUNT)
						{
							server_device->units.units[server_device->units.count] = num;
							server_device->units.count++;
						}

					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number parse error");
					}
					break;

				case tmdct_Address:
					if (unit->length == sizeof(guint8))
					{
						address = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Address: %d", address);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Address parse error");
					}
					break;

				case tmdct_NozzleCount:
					if (unit->length == sizeof(guint8))
					{
						nozzle_count = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Nozzle count: %d", nozzle_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Nozzle count parse error");
					}
					break;

				case tmdct_Nozzle:
					result = ParseDispencerControllerModuleSettingsNozzleFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseTgsModuleSettingsTankFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params , PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"				Tank:");

	guint32 num = 0;
	guint8 channel = 0;


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tmtct_Number:
					if (unit->length == sizeof(guint32))
					{
						num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number: %d", num);

						if (server_device->units.count < MAX_SERVER_DEVICE_UNIT_COUNT)
						{
							server_device->units.units[server_device->units.count] = num;
							server_device->units.count++;
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number parse error");
					}
					break;

				case tmtct_Channel:
					if (unit->length == sizeof(guint8))
					{
						channel = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Channel: %d", channel);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Channel parse error");
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParsePpcModuleSettingsPricePoleFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"				Price pole:");

	guint8 num = 0;
	guint8 grade = 0;
	guint8 symbol_count = 0;


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tppmct_Number:
					if (unit->length == sizeof(guint8))
					{
						num = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number: %d", num);

						if (server_device->units.count < MAX_SERVER_DEVICE_UNIT_COUNT)
						{
							server_device->units.units[server_device->units.count] = num;
							server_device->units.count++;
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number parse error");
					}
					break;

				case tppmct_Grade:
					if (unit->length == sizeof(guint8))
					{
						grade = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Grade: %d", grade);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Grade parse error");
					}
					break;

				case tppmct_SymbolCount:
					if (unit->length == sizeof(guint8))
					{
						symbol_count = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Symbol count: %d", symbol_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Symbol count parse error");
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseSensorControllerModuleSettingsParamFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"					Param:");


	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tspmct_Number:
					if (unit->length == sizeof(guint8))
					{
						guint8 num = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Number: %d", num);


					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 					Number parse error");
					}
					break;

				case tspmct_Type:
					if (unit->length == sizeof(guint8))
					{
						guint8 type = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Type: %d", type);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Type parse error");
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseScModuleSettingsSensorFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"				Sensor:");

	guint32 num = 0;
	guint8 addr = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tsmct_Number:
					if (unit->length == sizeof(guint32))
					{
						num = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number: %d", num);

						if (server_device->units.count < MAX_SERVER_DEVICE_UNIT_COUNT)
						{
							server_device->units.units[server_device->units.count] = num;
							server_device->units.count++;
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Number parse error");
					}
					break;

				case tsmct_Addr:
					if (unit->length == sizeof(guint8))
					{
						addr = unit->value[0];

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Addr: %d", addr);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Addr parse error");
					}
					break;

				case tsmct_Param:
					result = ParseSensorControllerModuleSettingsParamFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 				Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}



ExchangeError ParseDispencerControllerModuleSettingsMappingFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Mapping:");

	guint8 dispencer_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdmmt_DispencerCount:
					if (unit->length == sizeof(guint8))
					{
						dispencer_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Dispencer count: %d", dispencer_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Dispencer count parse error");
					}
					break;

				case tdmmt_DispencerConfiguration:
					result = ParseDispencerControllerModuleSettingsDispencerFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseTgsModuleSettingsMappingFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Mapping:");

	guint8 tank_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case ttmmt_TankCount:
					if (unit->length == sizeof(guint8))
					{
						tank_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Tank count: %d", tank_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Tank count parse error");
					}
					break;

				case ttmmt_TankConfiguration:
					result = ParseTgsModuleSettingsTankFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParsePpcModuleSettingsMappingFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Mapping:");

	guint8 price_pole_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tppcmmt_PricePoleCount:
					if (unit->length == sizeof(guint8))
					{
						price_pole_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Price pole count: %d", price_pole_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Price pole count parse error");
					}
					break;

				case tppcmmt_PricePoleConfiguration:
					result = ParsePpcModuleSettingsPricePoleFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseScModuleSettingsMappingFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"			Mapping:");

	guint8 sensor_count = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tscmmt_SensorCount:
					if (unit->length == sizeof(guint8))
					{
						sensor_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Sensor count: %d", sensor_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"				Sensor count parse error");
					}
					break;

				case tscmmt_SensorConfiguration:
					result = ParseScModuleSettingsSensorFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;


				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"	 			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}



ExchangeError ParseDispencerControllerModuleSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"		Module settings:");

	gboolean counters_enable = FALSE;
	gboolean auto_start = FALSE;
	gboolean auto_payment = FALSE;
	guint32 full_tank_volume = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdcmst_LogSettings:
					result = ParseModuleSettingsLogSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_ConnSettings:
					result = ParseModuleSettingsConnSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_TimeoutSettings:
					result = ParseModuleSettingsTimeoutSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_DPSettings:
					result = ParseDispencerControllerModuleSettingsDpSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tdcmst_CountersEnable:
					if (unit->length == sizeof(guint8))
					{
						counters_enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Counters enable: %d", counters_enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Counters enable parse error");
					}
					break;

				case tdcmst_AutoStart:
					if (unit->length == sizeof(guint8))
					{
						auto_start = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Auto start: %d", auto_start);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Auto start parse error");
					}
					break;

				case tdcmst_AutoPayment:
					if (unit->length == sizeof(guint8))
					{
						auto_payment = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Auto payment: %d", auto_payment);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Auto payment parse error");
					}
					break;

				case tdcmst_FullTankVolume:
					if (unit->length == sizeof(guint32))
					{
						full_tank_volume = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 		Full tank volume: %d", full_tank_volume);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"	 		Full tank volume parse error");
					}
					break;

				case tdcmst_ModuleMapping:
					result = ParseDispencerControllerModuleSettingsMappingFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseTgsModuleSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"		Module settings:");

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case ttgsmst_LogSettings:
					result = ParseModuleSettingsLogSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case ttgsmst_ConnSettings:
					result = ParseModuleSettingsConnSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case ttgsmst_TimeoutSettings:
					result = ParseModuleSettingsTimeoutSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case ttgsmst_ModuleMapping:
					result = ParseTgsModuleSettingsMappingFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParsePpcModuleSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"		Module settings:");

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tppcmst_LogSettings:
					result = ParseModuleSettingsLogSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_ConnSettings:
					result = ParseModuleSettingsConnSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_TimeoutSettings:
					result = ParseModuleSettingsTimeoutSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_DpSettings:
					result = ParsePricePoleControllerModuleSettingsDpSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tppcmst_ModuleMapping:
					result = ParsePpcModuleSettingsMappingFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}

ExchangeError ParseScModuleSettingsFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"		Module settings:");

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tscmst_LogSettings:
					result = ParseModuleSettingsLogSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_ConnSettings:
					result = ParseModuleSettingsConnSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_TimeoutSettings:
					result = ParseModuleSettingsTimeoutSettingsFrame(unit->value, unit->length, params);

					if (result != ee_None)
					{
						return result;
					}
					break;

				case tscmst_ModuleMapping:
					result = ParseScModuleSettingsMappingFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"			Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}


	return result;
}


ExchangeError ParseDispencerControllerConfigFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"	Dispencer controller:");

	guint8 id = 0;
	gchar* name = NULL;
	guint16 port = 0;
	gboolean enable = FALSE;
	guint32 command_timeout = 0;
	guint32 interval = 0;
	gchar* module = NULL;
	gchar* log_dir = NULL;
	gboolean log_enable = FALSE;
	gboolean log_trace = FALSE;
	guint32 log_file_size = 0;
	guint32 log_save_days = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tdst_Id:
					if (unit->length == sizeof(guint8))
					{
						id = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID: %d",	 id);

					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID parse error");
						result = ee_Parse;
					}
					break;

				case tdst_Name:
					if (unit->length > 0)
					{
						if (name !=NULL)
						{
							g_free(name);
							name = NULL;
						}
						name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Name: %s",	name);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Name parse error");
						result = ee_Parse;
					}
					break;

				case tdst_Port:
					if (unit->length == sizeof(guint16))
					{
						port = (unit->value[0] << 8 ) |  unit->value[1];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port: %d",	 port);

						server_device->port = port;
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port parse error");
						result = ee_Parse;
					}
					break;

				case tdst_Enable:
					if (unit->length == sizeof(guint8))
					{
						enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable: %d", enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable parse error");
					}
					break;

				case tdst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						command_timeout = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout: %d", command_timeout);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout parse error");
					}
					break;

				case tdst_Interval:
					if (unit->length == sizeof(guint32))
					{
						interval = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval: %d", interval);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval parse error");
					}
					break;

				case tdst_Module:
					if (unit->length > 0)
					{
						if (module !=NULL)
						{
							g_free(module);
							module = NULL;
						}
						module = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Module: %s",	module);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Module parse error");
						result = ee_Parse;
					}
					break;

				case tdst_LogDir:
					if (unit->length > 0)
					{
						if (log_dir !=NULL)
						{
							g_free(log_dir);
							log_dir = NULL;
						}
						log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Lod dir: %s",	log_dir);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Lod dir parse error");
						result = ee_Parse;
					}
					break;

				case tdst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						log_enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable: %d", log_enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable parse error");
					}
					break;

				case tdst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						log_trace = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace: %d", log_trace);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace parse error");
					}
					break;

				case tdst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						log_file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size: %d", log_file_size);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size parse error");
					}
					break;

				case tdst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						log_save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days: %d", log_save_days);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days parse error");
					}
					break;

				case tdst_ModuleSettings:
					result = ParseDispencerControllerModuleSettingsFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (name !=NULL)
	{
		g_free(name);
		name = NULL;
	}
	if (module !=NULL)
	{
		g_free(module);
		module = NULL;
	}
	if (log_dir !=NULL)
	{
		g_free(log_dir);
		log_dir = NULL;
	}
	return result;
}

ExchangeError ParseTgsConfigFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	TlvUnit* units = NULL;

	LogParams* log_params = params->log_params;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"	Tgs:");

	guint8 id = 0;
	gchar* name = NULL;
	guint16 port = 0;
	gboolean enable = FALSE;
	guint32 command_timeout = 0;
	guint32 interval = 0;
	gchar* module = NULL;
	gchar* log_dir = NULL;
	gboolean log_enable = FALSE;
	gboolean log_trace = FALSE;
	guint32 log_file_size = 0;
	guint32 log_save_days = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case ttst_Id:
					if (unit->length == sizeof(guint8))
					{
						id = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID: %d",	 id);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID parse error");
						result = ee_Parse;
					}
					break;

				case ttst_Name:
					if (unit->length > 0)
					{
						if (name !=NULL)
						{
							g_free(name);
							name = NULL;
						}
						name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Name: %s",	name);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Name parse error");
						result = ee_Parse;
					}
					break;

				case ttst_Port:
					if (unit->length == sizeof(guint16))
					{
						port = (unit->value[0] << 8 ) |  unit->value[1];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port: %d",	 port);

						server_device->port = port;
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port parse error");
						result = ee_Parse;
					}
					break;

				case ttst_Enable:
					if (unit->length == sizeof(guint8))
					{
						enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable: %d", enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable parse error");
					}
					break;

				case ttst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						command_timeout = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout: %d", command_timeout);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout parse error");
					}
					break;

				case ttst_Interval:
					if (unit->length == sizeof(guint32))
					{
						interval = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval: %d", interval);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval parse error");
					}
					break;

				case ttst_Module:
					if (unit->length > 0)
					{
						if (module !=NULL)
						{
							g_free(module);
							module = NULL;
						}
						module = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Module: %s",	module);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Module parse error");
						result = ee_Parse;
					}
					break;

				case ttst_LogDir:
					if (unit->length > 0)
					{
						if (log_dir !=NULL)
						{
							g_free(log_dir);
							log_dir = NULL;
						}
						log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Lod dir: %s",	log_dir);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Lod dir parse error");
						result = ee_Parse;
					}
					break;

				case ttst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						log_enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable: %d", log_enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable parse error");
					}
					break;

				case ttst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						log_trace = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace: %d", log_trace);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace parse error");
					}
					break;

				case ttst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						log_file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size: %d", log_file_size);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size parse error");
					}
					break;

				case ttst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						log_save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days: %d", log_save_days);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days parse error");
					}
					break;

				case ttst_ModuleSettings:
					result = ParseTgsModuleSettingsFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (name !=NULL)
	{
		g_free(name);
		name = NULL;
	}
	if (module !=NULL)
	{
		g_free(module);
		module = NULL;
	}
	if (log_dir !=NULL)
	{
		g_free(log_dir);
		log_dir = NULL;
	}
	return result;
}

ExchangeError ParsePpcConfigFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	LogParams* log_params = params->log_params;

	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"	Price pole controller:");

	guint8 id = 0;
	gchar* name = NULL;
	guint16 port = 0;
	gboolean enable = FALSE;
	guint32 command_timeout = 0;
	guint32 interval = 0;
	gchar* module = NULL;
	gchar* log_dir = NULL;
	gboolean log_enable = FALSE;
	gboolean log_trace = FALSE;
	guint32 log_file_size = 0;
	guint32 log_save_days = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tspt_Id:
					if (unit->length == sizeof(guint8))
					{
						id = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID: %d",	 id);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID parse error");
						result = ee_Parse;
					}
					break;

				case tspt_Name:
					if (unit->length > 0)
					{
						if (name !=NULL)
						{
							g_free(name);
							name = NULL;
						}
						name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Name: %s",	name);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Name parse error");
						result = ee_Parse;
					}
					break;

				case tspt_Port:
					if (unit->length == sizeof(guint16))
					{
						port = (unit->value[0] << 8 ) |  unit->value[1];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port: %d",	 port);

						server_device->port = port;
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port parse error");
						result = ee_Parse;
					}
					break;

				case tspt_Enable:
					if (unit->length == sizeof(guint8))
					{
						enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable: %d", enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable parse error");
					}
					break;

				case tspt_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						command_timeout = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout: %d", command_timeout);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout parse error");
					}
					break;

				case tspt_Interval:
					if (unit->length == sizeof(guint32))
					{
						interval = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval: %d", interval);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval parse error");
					}
					break;

				case tspt_Module:
					if (unit->length > 0)
					{
						if (module !=NULL)
						{
							g_free(module);
							module = NULL;
						}
						module = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Module: %s",	module);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Module parse error");
						result = ee_Parse;
					}
					break;

				case tspt_LogDir:
					if (unit->length > 0)
					{
						if (log_dir !=NULL)
						{
							g_free(log_dir);
							log_dir = NULL;
						}
						log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Lod dir: %s",	log_dir);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Lod dir parse error");
						result = ee_Parse;
					}
					break;

				case tspt_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						log_enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable: %d", log_enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable parse error");
					}
					break;

				case tspt_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						log_trace = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace: %d", log_trace);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace parse error");
					}
					break;

				case tspt_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						log_file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size: %d", log_file_size);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size parse error");
					}
					break;

				case tspt_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						log_save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days: %d", log_save_days);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days parse error");
					}
					break;

				case tspt_ModuleSettings:
					result = ParsePpcModuleSettingsFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (name !=NULL)
	{
		g_free(name);
		name = NULL;
	}
	if (module !=NULL)
	{
		g_free(module);
		module = NULL;
	}
	if (log_dir !=NULL)
	{
		g_free(log_dir);
		log_dir = NULL;
	}
	return result;
}

ExchangeError ParseScConfigFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params, PSSServerDevice* server_device)
{
	LogParams* log_params = params->log_params;

	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"	Sensor controller:");

	guint8 id = 0;
	gchar* name = NULL;
	guint16 port = 0;
	gboolean enable = FALSE;
	guint32 command_timeout = 0;
	guint32 interval = 0;
	gchar* module = NULL;
	gchar* log_dir = NULL;
	gboolean log_enable = FALSE;
	gboolean log_trace = FALSE;
	guint32 log_file_size = 0;
	guint32 log_save_days = 0;

	if (tlv_calc_list_count(units) > 0)
	{
		TlvUnit* unit = units;

		do
		{
			switch (unit->tag)
			{
				case tsst_Id:
					if (unit->length == sizeof(guint8))
					{
						id = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID: %d",	 id);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		ID parse error");
						result = ee_Parse;
					}
					break;

				case tsst_Name:
					if (unit->length > 0)
					{
						if (name !=NULL)
						{
							g_free(name);
							name = NULL;
						}
						name = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Name: %s",	name);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Name parse error");
						result = ee_Parse;
					}
					break;

				case tsst_Port:
					if (unit->length == sizeof(guint16))
					{
						port = (unit->value[0] << 8 ) |  unit->value[1];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port: %d",	 port);

						server_device->port = port;
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 		Port parse error");
						result = ee_Parse;
					}
					break;

				case tsst_Enable:
					if (unit->length == sizeof(guint8))
					{
						enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable: %d", enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Enable parse error");
					}
					break;

				case tsst_CommandTimeout:
					if (unit->length == sizeof(guint32))
					{
						command_timeout = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout: %d", command_timeout);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Command timeout parse error");
					}
					break;

				case tsst_Interval:
					if (unit->length == sizeof(guint32))
					{
						interval = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval: %d", interval);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Interval parse error");
					}
					break;

				case tsst_Module:
					if (unit->length > 0)
					{
						if (module !=NULL)
						{
							g_free(module);
							module = NULL;
						}
						module = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Module: %s",	module);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Module parse error");
						result = ee_Parse;
					}
					break;

				case tsst_LogDir:
					if (unit->length > 0)
					{
						if (log_dir !=NULL)
						{
							g_free(log_dir);
							log_dir = NULL;
						}
						log_dir = g_convert_with_fallback((gchar*)unit->value, unit->length, "UTF-8", "UTF-8", " ", NULL, NULL, NULL);
						if (name !=NULL)
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"	 	Lod dir: %s",	log_dir);

						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Lod dir parse error");
						result = ee_Parse;
					}
					break;

				case tsst_LogEnable:
					if (unit->length == sizeof(guint8))
					{
						log_enable = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable: %d", log_enable);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log enable parse error");
					}
					break;

				case tsst_LogTrace:
					if (unit->length == sizeof(guint8))
					{
						log_trace = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace: %d", log_trace);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log trace parse error");
					}
					break;

				case tsst_LogFileSize:
					if (unit->length == sizeof(guint32))
					{
						log_file_size = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size: %d", log_file_size);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log file size parse error");
					}
					break;

				case tsst_LogSaveDays:
					if (unit->length == sizeof(guint32))
					{
						log_save_days = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);

						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days: %d", log_save_days);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Log save days parse error");
					}
					break;

				case tsst_ModuleSettings:
					result = ParseScModuleSettingsFrame(unit->value, unit->length, params, server_device);

					if (result != ee_None)
					{
						return result;
					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"		Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	if (name !=NULL)
	{
		g_free(name);
		name = NULL;
	}
	if (module !=NULL)
	{
		g_free(module);
		module = NULL;
	}
	if (log_dir !=NULL)
	{
		g_free(log_dir);
		log_dir = NULL;
	}
	return result;
}

ExchangeError ParseConfigFrame(guchar* buffer, guint16 buffer_length, PSSServConfReadThreadParam* params)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, FALSE, params->log_trace, params->log_enable, " <<");

	for (guint16 i = 0; i < buffer_length; i++)
	{
		add_log(log_params, FALSE, FALSE, params->log_trace, params->log_enable, " %02X", buffer[i]);
	}
	add_log(log_params, FALSE, TRUE, params->log_trace, params->log_enable, "");

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "");

	TlvUnit* units = NULL;

	ExchangeError result = tlv_parse_frame(buffer, buffer_length, &units);

	guint32 message_id = 0;
	MessageType message_type = mt_Undefined;
	guint32 command = 0;
	guint8 error;
	guint8 dispencer_controller_count = 0;
	guint8 tgs_count = 0;
	guint8 price_pole_controller_count = 0;
	guint8 sensor_controller_count = 0;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,	"Parse frame:");

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
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	MessageId: %d", message_id);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	MessageId parse error");
						result = ee_Parse;
					}
					break;

				case tst_MessageType:
					if (unit->length == sizeof(guint8))
					{
						message_type = (MessageType)unit->value[0];
						gchar* message_type_description = return_message_type_name(message_type);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Message type: %s",	 message_type_description);

						if (message_type_description != NULL)
						{
							g_free(message_type_description);
						}
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Message type parse error");
						result = ee_Parse;
					}
					break;

				case tst_CommandCode:
					if (unit->length == sizeof(guint32))
					{
						command = (guint32)((unit->value[0] << 24) | (unit->value[1] << 16) | (unit->value[2] << 8) | unit->value[3]);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Command code: %08X (%s)",	command, server_command_to_str(command));
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Command code parse error");
						result = ee_Parse;
					}
					break;

				case tst_Error:
					if (unit->length == sizeof(guint8))
					{
						error = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error: %d",	error);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error parse error");
						result = ee_Parse;
					}
					break;

				case tst_DispencerControllerCount:
					if (unit->length == sizeof(guint8))
					{
						dispencer_controller_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Dispencer controller count: %d",	dispencer_controller_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error parse dispencer controller count");
						result = ee_Parse;
					}
					break;

				case tst_DispencerControllerConfig:
					{
						PSSServerDevice server_device = {0x00};

						server_device.device_type = psdt_DispencerController;
						server_device.units.count = 0;
						server_device.guid = g_strdup(params->guid);
						server_device.ip_address = g_strdup(params->ip_address);
						server_device.timeout = DEF_DC_TIMEOUT;
						server_device.id = get_server_device_id();

						result = ParseDispencerControllerConfigFrame(unit->value, unit->length, params, &server_device);

						if (result != ee_None)
						{
							return result;
						}
						else
						{
							add_server_device(&server_device);
							inc_server_device_id();

							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
															"Added server device ID=%d, Type = %s, GUID = %s, IP address = %s, Port = %d, Timeout = %d,  Units count = %d ", server_device.id,
															server_device_type_to_str(server_device.device_type), server_device.guid, server_device.ip_address,server_device.port, server_device.timeout, server_device.units.count);
						}
						clear_service_device(&server_device);
					}
					break;

				case tst_TgsCount:
					if (unit->length == sizeof(guint8))
					{
						tgs_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Tgs count: %d",	tgs_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error parse tgs count");
						result = ee_Parse;
					}
					break;

				case tst_TgsConfig:
					{
						PSSServerDevice server_device = { 0x00 };

						server_device.device_type = psdt_TankGaugeSystem;
						server_device.units.count = 0;
						server_device.guid = g_strdup(params->guid);
						server_device.ip_address = g_strdup(params->ip_address);
						server_device.timeout = DEF_DC_TIMEOUT;
						server_device.id = get_server_device_id();

						result = ParseTgsConfigFrame(unit->value, unit->length, params, &server_device);

						if (result != ee_None)
						{
							return result;
						}
						else
						{
							add_server_device(&server_device);
							inc_server_device_id();

							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
															"Added server device ID=%d, Type = %s, GUID = %s, IP address = %s, Port = %d, Timeout = %d,  Units count = %d ", server_device.id,
															server_device_type_to_str(server_device.device_type), server_device.guid, server_device.ip_address,server_device.port, server_device.timeout, server_device.units.count);
						}
						clear_service_device(&server_device);

					}
					break;

				case tst_PpcCount:
					if (unit->length == sizeof(guint8))
					{
						price_pole_controller_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Price pole controller count: %d",	price_pole_controller_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error parse price pole controller count");
						result = ee_Parse;
					}
					break;


				case tst_PpcConfig:
					{
						PSSServerDevice server_device = {0x00};

						server_device.device_type = psdt_PricePoleController;
						server_device.units.count = 0;
						server_device.guid = g_strdup(params->guid);
						server_device.ip_address = g_strdup(params->ip_address);
						server_device.timeout = DEF_DC_TIMEOUT;
						server_device.id = get_server_device_id();

						result = ParsePpcConfigFrame(unit->value, unit->length, params, &server_device);

						if (result != ee_None)
						{
							return result;
						}
						else
						{
							add_server_device(&server_device);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
															"Added server device ID=%d, Type = %s, GUID = %s, IP address = %s, Port = %d, Timeout = %d,  Units count = %d ", server_device.id,
															server_device_type_to_str(server_device.device_type), server_device.guid, server_device.ip_address,server_device.port, server_device.timeout, server_device.units.count);
							inc_server_device_id();
						}
						clear_service_device(&server_device);

					}
					break;

				case tst_ScCount:
					if (unit->length == sizeof(guint8))
					{
						sensor_controller_count = unit->value[0];
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Sensor controller count: %d",	sensor_controller_count);
					}
					else
					{
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Error parse sensor controller count");
						result = ee_Parse;
					}
					break;


				case tst_ScConfig:
					{
						PSSServerDevice server_device = {0x00};

						server_device.device_type = psdt_SensorController;
						server_device.units.count = 0;
						server_device.guid = g_strdup(params->guid);
						server_device.ip_address = g_strdup(params->ip_address);
						server_device.timeout = DEF_DC_TIMEOUT;
						server_device.id = get_server_device_id();

						result = ParseScConfigFrame(unit->value, unit->length, params, &server_device);

						if (result != ee_None)
						{
							return result;
						}
						else
						{
							add_server_device(&server_device);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
															"Added server device ID=%d, Type = %s, GUID = %s, IP address = %s, Port = %d, Timeout = %d,  Units count = %d ", server_device.id,
															server_device_type_to_str(server_device.device_type), server_device.guid, server_device.ip_address,server_device.port, server_device.timeout, server_device.units.count);
							inc_server_device_id();
						}
						clear_service_device(&server_device);

					}
					break;

				default:
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								" 	Undefined tag (%04X)!", unit->tag);
					break;
			}

			unit = unit->next;

		}while(unit != NULL);
	}

	return result;
}

gpointer serv_conf_read_thread_func(gpointer data)
{
	PSSServConfReadThreadParam param = *(PSSServConfReadThreadParam*)data;

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

	ExchangeState es = es_WaitStartMessage;

	while(TRUE)
	{

		int bytes_read = recv(param.sock, bufr, 1024, 0);

		if(bytes_read <= 0)
		{
			set_connected(FALSE);
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
								ExchangeError parse_result = ParseConfigFrame(bufrd, pos_bufrd, &param);

								if (parse_result != ee_None)
								{
									send_short_serv_message(&param, mt_Nak, parse_result);
								}
								else
								{
									switch(get_hs_state())
									{
										case hses_GetDcConf:
											set_hs_state(hses_GetTgsConf);
											break;

										case hses_GetTgsConf:
											set_hs_state(hses_GetPpConf);
											break;

										case hses_GetPpConf:
											set_hs_state(hses_GetScConf);
											break;

										case hses_GetScConf:
											set_hs_state(hses_Complete);
											return NULL;

										case hses_Complete:
											return NULL;
									}
								}
								es = es_WaitStartMessage;
							}
							else
							{
								send_short_serv_message(&param, mt_Ack, ee_None);
							}
						}
						else
						{
							pos_bufrd_tmp = pos_bufrd;
							bad_frame_count++;

							if (bad_frame_count >= MAX_BAD_FRAME)
							{
								send_short_serv_message(&param, mt_Eot, ee_LimitBadFrame);
								es = es_WaitStartMessage;
							}
							else
							{
								if (bad_frame)
								{
									send_short_serv_message(&param, mt_Nak, ee_Format);
								}
								else
								{
									send_short_serv_message(&param, mt_Nak, ee_Crc);
								}
								es = es_WaitSTX;
							}
						}
						break;
				}
			}
		}
	}

	return NULL;

}

gpointer serv_conf_thread_func(gpointer data)
{
	PSSServConfThreadParam param = *(PSSServConfThreadParam*)data;

	struct	sockaddr_in addr;

	LogParams* log_params = param.log_params;

	add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Connect to hardware server...");

	addr.sin_port = htons( param.port);
	addr.sin_addr.s_addr = inet_addr(param.ip_address);
	addr.sin_family = AF_INET;

	guint64 last_connect_time =  get_date_time();


	while(!connected)
	{
		if (get_date_time() > last_connect_time +  HW_SERVER_CONNECT_TIMEOUT)
		{
			last_connect_time = get_date_time();

			gint32 sock = socket(AF_INET, SOCK_STREAM, 0);

			if(sock < 0)
			{
				add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Hardware server socket create error");
			}
			else
			{
				add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Hardware server socket successfully create (port %d)", param.port);

				if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
				{
					add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Hardware server connection error");

				}
				else
				{
					add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Hardware server connected");

					clear_service_devices();
					set_server_device_id(1);

					PSSServConfReadThreadParam read_param = {.sock = sock, .log_params = log_params, .log_trace = param.log_trace, .log_enable = param.log_enable, .guid = g_strdup(param.guid), .ip_address = g_strdup(param.ip_address)};

					GThread* serv_conf_read_thread = g_thread_new("serv_conf_read_thread", serv_conf_read_thread_func, &read_param);

					if (serv_conf_read_thread == NULL)
					{
						add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Hardware server configuration read thread error");
					}
					else
					{
						serv_conf_read_thread->priority = G_THREAD_PRIORITY_LOW;
					}
					set_connected(TRUE);

					guint64 send_time =  get_date_time();

					while(get_connected())
					{
						if (get_date_time() > send_time + SERVER_REPLY_TIMEOUT)
						{
							if (get_command_sended())
							{
								set_command_sended(FALSE);
								set_hs_state(hses_GetDcConf);
							}

							switch(get_hs_state())
							{
								case hses_GetDcConf:
									add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Get dispencer controllers configuration");
									send_config_request_message(sock, &param, get_message_id(), hsc_GetDispencerControllerConfig);
									send_time = get_date_time();
									break;

								case hses_GetTgsConf:
									add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Get tgs controllers configuration");
									send_config_request_message(sock, &param, get_message_id(), hsc_GetTgsConfig);
									send_time = get_date_time();
									break;

								case hses_GetPpConf:
									add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Get price pole controllers configuration");
									send_config_request_message(sock, &param, get_message_id(), hsc_GetPricePoleControllerConfig);
									send_time = get_date_time();
									break;

								case hses_GetScConf:
									add_log(log_params, TRUE, TRUE, param.log_trace, param.log_enable, "Get sensor controllers configuration");
									send_config_request_message(sock, &param, get_message_id(), hsc_GetSensorControllerConfig);
									send_time = get_date_time();
									break;

								case hses_Complete:
									set_server_conf_is_load(TRUE);
									return NULL;
							}
						}
					}
					if (sock > 0)
					{
						close(sock);
						sock = -1;
					}
				}
			}
		}
	}

	return NULL;
}
