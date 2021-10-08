#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_func.h"

guint8 is_leaf(xmlNode * node)
{
	xmlNode * child = node->children;

	while(child)
	{
		if(child->type == XML_ELEMENT_NODE) return 0;
		child = child->next;
	}
	return 1;
}


void parse_int_tag(xmlNode* node, const gchar* nodename, guint32* value, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			*value =  atoi((gchar*)tmp_node->content);
			g_printf("%s %d\n", description, *value);
		}
	}
}

void parse_time_t_tag(xmlNode* node, const gchar* nodename, time_t* value, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			*value =  atoi((gchar*)tmp_node->content);

    		struct tm tm_mr = *localtime(value);
    		printf("%s: %d-%02d-%02d %02d:%02d:%02d\n", description,  tm_mr.tm_year + 1900, tm_mr.tm_mon + 1, tm_mr.tm_mday, tm_mr.tm_hour, tm_mr.tm_min, tm_mr.tm_sec);

		}
	}
}

void parse_ascii_tag(xmlNode* node, const gchar* nodename, gchar* value, guint8 max_length, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			gchar* tmp_str = g_strdup((gchar*)tmp_node->content);

			memset(value, 0x00, max_length);

			memcpy(value, tmp_str, MIN(strlen(tmp_str), max_length - 1));

			g_printf("%s %s\n", description, value);

			g_free(tmp_str);
		}
	}
}

void parse_binary_tag(xmlNode* node, const gchar* nodename, guint16* value, const gchar* description)
{
	guint32 val = 0;

	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			if (strlen((gchar*)tmp_node->content) > 0)
			{
				gchar* str = g_strdup((gchar*)tmp_node->content);
				guint8 size = strlen(str) - 1;

				for (guint8 i = 0; i < strlen(str); i++)
				{
					if (str[i] == '1')
					{
						val |= 0x01 << size;
					}
					size--;
				}

				g_free(str);
			}

			*value = val;

			g_printf("%s %d\n", description, *value);
		}
	}
}

void parse_int8_tag(xmlNode* node, const gchar* nodename, guint8* value, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			*value =  atoi((gchar*)tmp_node->content);
			g_printf("%s %d\n", description, *value);
		}
	}
}

void parse_int16_tag(xmlNode* node, const gchar* nodename, guint16* value, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			*value =  atoi((gchar*)tmp_node->content);
			g_printf("%s %d\n", description, *value);
		}
	}
}

void parse_bool_tag(xmlNode* node, const gchar* nodename, gboolean* value, const gchar* description)
{
	if (node->type == XML_ELEMENT_NODE && strcmp((gchar*)node->name,nodename) == 0)
	{
		xmlNode * tmp_node = node->children;

		if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
		{
			*value = strcmp((gchar*)tmp_node->content,"Yes") == 0 ? TRUE : FALSE;
			g_printf("%s %d\n", description, *value);
		}
	}
}

void parse_pss_grades(xmlNode * node, PSSGeneralFunctions* data)
{
	g_printf("			Grades:\n");

	data->grade_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Grade") == 0 && data->grade_count < MAX_GRADE_COUNT)
		{
			PSSGrade* grade = &data->grades[data->grade_count];
			grade->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			if (grade->name!=NULL) { g_free(grade->name); grade->name = NULL;}
			grade->name = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"Name"));
			g_printf("				Grade ID%d %s\n", grade->id, grade->name);
			data->grade_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_price_groups(xmlNode * node, PSSGeneralFunctions* data)
{
	g_printf("			Price groups:\n");

	data->price_group_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PriceGroup") == 0 && data->price_group_count < MAX_PRICE_GROUP_COUNT)
		{
			PSSPriceGroup* price_group = &data->price_groups[data->price_group_count];

			price_group->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("				Price group ID%d \n", price_group->id);

			xmlNode * price_group_node = nodes->children;

			price_group->grade_price_count  = 0;

			while(price_group_node)
			{
				if (price_group_node->type == XML_ELEMENT_NODE && strcmp((gchar*)price_group_node->name,"GradeID") == 0 && price_group->grade_price_count < MAX_GRADE_COUNT)
				{
					xmlNode * grade_id_node = price_group_node->children;

					if (grade_id_node !=NULL && grade_id_node->type == XML_TEXT_NODE)
					{
						price_group->grade_prices[price_group->grade_price_count].grade_id =  atoi((gchar*)grade_id_node->content);
					}
				}
				if (price_group_node->type == XML_ELEMENT_NODE && strcmp((gchar*)price_group_node->name,"Price") == 0 && price_group->grade_price_count < MAX_GRADE_COUNT)
				{
					xmlNode * price_node = price_group_node->children;

					if (price_node !=NULL && price_node->type == XML_TEXT_NODE)
					{
						price_group->grade_prices[price_group->grade_price_count].price =  atoi((gchar*)price_node->content);
					}
					g_printf("					Grade ID%d price = %d\n", price_group->grade_prices[price_group->grade_price_count].grade_id, price_group->grade_prices[price_group->grade_price_count].price);
					price_group->grade_price_count++;
				}
				price_group_node = price_group_node->next;
			}

			g_printf("					Item count = %d\n", price_group->grade_price_count);

			data->price_group_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_general_params(xmlNode * node, PSSGeneralParams* data)
{
	g_printf("		General params:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int_tag(nodes, "PriceIncreaseDelay", &data->price_increase_delay, "			Price increase delay:");
		parse_int_tag(nodes, "PriceDecreaseDelay", &data->price_decrease_delay, "			Price decrease delay:");
		parse_ascii_tag(nodes, "DefaultLangugeCode", data->default_language_code,MAX_LANG_CODE_LENGTH + 1 , "			Default language code:");
		parse_bool_tag(nodes, "DisableFpTotalsError", &data->disable_fp_totals_error, "			Disable fp totals error:");
		parse_bool_tag(nodes, "EnableDemoEncription", &data->enable_demo_encription, "			Enable demo encription:");
		parse_ascii_tag(nodes, "CurrencyCode", data->currency_code,MAX_CURRENCY_CODE_LENGTH + 1 , "			Currency code:");
		parse_int8_tag(nodes, "FcPumpTotalsHandlingMode", &data->fc_pump_totals_handling_mode, "			Fc pump totals handling mode:");
		parse_ascii_tag(nodes, "FcShiftNo", data->fc_shift_no,MAX_FC_SHIFT_NO_LENGTH + 1 , "			Fc shift No:");
		parse_int_tag(nodes, "VATRate", &data->vat_rate, "			VAT rate:");
		nodes = nodes->next;
	}
}

void parse_pss_general_functions(xmlNode * node, PSSGeneralFunctions* data)
{
	g_printf("		General functions:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Grades") == 0)
		{
			parse_pss_grades(nodes, data);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PriceGroups") == 0)
		{
			parse_pss_price_groups(nodes, data);
		}
		nodes = nodes->next;
	}
}


void parse_pss_service_modes(xmlNode * node, PSSDispenceControl* data)
{
	g_printf("			Service modes:\n");

	data->service_mode_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ServiceMode") == 0 && data->service_mode_count <  MAX_SERVICE_MODE_COUNT)
		{
			PSSServiceMode* service_mode = &data->service_modes[data->service_mode_count];

			service_mode->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("				Service mode ID%d \n", service_mode->id);

			xmlNode * service_mode_node = nodes->children;

			while(service_mode_node)
			{
				parse_int8_tag(service_mode_node, "AutoAuthorizeLimit", &service_mode->auto_autorize_limit, "					Auto authorize limit:");
				parse_int_tag(service_mode_node, "MaxPreAuthTime", &service_mode->max_pre_auth_time, "					Max pre auth time:");
				parse_int_tag(service_mode_node, "MaxNzLayDownTime", &service_mode->max_nz_lay_down_time, "					Max nz lay down time:");
				parse_bool_tag(service_mode_node, "ZeroTransToPos", &service_mode->zero_trans_to_pos, "					Zero trans to pos:");
				parse_int8_tag(service_mode_node, "MinTransVol", &service_mode->min_trans_vol, "					Min trans vol:");
				parse_int8_tag(service_mode_node, "MinTransMoney", &service_mode->min_trans_money, "					Min trans money:");
				parse_int8_tag(service_mode_node, "SupTransBufferSize", &service_mode->sup_trans_buffer_size, "					Sup trans buffer size:");
				parse_int8_tag(service_mode_node, "UnsupTransBufferSize", &service_mode->unsup_trans_buffer_size, "					Unsup trans buffer size:");
				parse_bool_tag(service_mode_node, "StoreAtPreAuthorize", &service_mode->store_at_pre_authorize, "					Store at pre authorize:");
				parse_bool_tag(service_mode_node, "VolInTransBufferStatus", &service_mode->vol_in_trans_buffer_status, "					Vol in trans buffer status:");
				parse_bool_tag(service_mode_node, "AuthorizeAtModeSelection", &service_mode->authorize_at_mode_selection, "					Authorize at mode selection:");
				parse_int8_tag(service_mode_node, "MnoConsecutiveZeroTrans", &service_mode->mno_consecutive_zero_trans, "					Mno consecutive zero trans:");
				parse_int_tag(service_mode_node, "AutoClearTransDelayTime", &service_mode->auto_clear_trans_delay_time, "					Auto clear trans delay time:");
				parse_int8_tag(service_mode_node, "PumpLightMode", &service_mode->pump_light_mode, "					Pump light mode:");
				parse_bool_tag(service_mode_node, "StopFpOnVehicleTag", &service_mode->stop_fp_on_vehicle_tag, "					Stop fp on vehicle tag:");
				parse_bool_tag(service_mode_node, "UseVehicleTagReadingButton", &service_mode->use_vehicle_tag_reading_button, "					Use vehicle tag reading button:");
				parse_int_tag(service_mode_node, "AutoUnlockTransDelayTime", &service_mode->auto_unlock_trans_delay_time, "					Auto unlock trans delay time:");

				service_mode_node = service_mode_node->next;
			}
			data->service_mode_count++;
		}
		nodes = nodes->next;
	}

}
void parse_pss_fuelling_modes(xmlNode * node, PSSDispenceControl* data)
{
	g_printf("			Fuelling modes:\n");

	data->fuelling_mode_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingMode") == 0 && data->fuelling_mode_count <  MAX_FUELLING_MODE_COUNT)
		{
			PSSFuellingMode* fuelling_mode = &data->fuelling_modes[data->fuelling_mode_count];

			fuelling_mode->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("				Fuelling mode ID%d \n", fuelling_mode->id);

			xmlNode * fuelling_mode_node = nodes->children;

			while(fuelling_mode_node)
			{
				parse_binary_tag(fuelling_mode_node, "FuellingType", &fuelling_mode->fuelling_type, "					Fuelling type:");
				parse_int_tag(fuelling_mode_node, "MaxTimeToReachMinLimit", &fuelling_mode->max_time_to_reach_min_limit, "					Max time to reach min limit:");
				parse_int_tag(fuelling_mode_node, "MaxTimeWithoutProgress", &fuelling_mode->max_time_without_progress, "					Max time without progress:");
				parse_int_tag(fuelling_mode_node, "MaxTransVolume", &fuelling_mode->max_trans_volume, "					Max trans volume:");
				parse_int_tag(fuelling_mode_node, "MaxTransMoney", &fuelling_mode->max_trans_money, "					Max trans money:");
				parse_int_tag(fuelling_mode_node, "MaxFuellingTime", &fuelling_mode->max_fuelling_time, "					Max fuelling time:");
				parse_int8_tag(fuelling_mode_node, "MaxPresetVolOverrunErrLimit", &fuelling_mode->max_preset_vol_overrun_err_limit, "					Max preset vol overrun err limit:");
				parse_int_tag(fuelling_mode_node, "ClrDisplayDelayTime", &fuelling_mode->clr_display_delay_time, "					Clr display delay time:");
				parse_bool_tag(fuelling_mode_node, "ClrDisplayWhenCurTrDisappear", &fuelling_mode->clr_display_when_cur_tr_disappear, "					Clear display when current transaction disappear:");
				parse_int8_tag(fuelling_mode_node, "MinSubPumpRuntimeBeforeStart", &fuelling_mode->min_sub_pump_runtime_before_start, "					Min sub pump runtime before start:");
				parse_int_tag(fuelling_mode_node, "MaxTransVolume_e", &fuelling_mode->max_trans_volume_e, "					Max trans volume extended:");
				parse_int_tag(fuelling_mode_node, "MaxTransMoney_e", &fuelling_mode->max_trans_money_e, "					Max trans money extended:");

				fuelling_mode_node = fuelling_mode_node->next;
			}
			data->fuelling_mode_count++;
		}
		nodes = nodes->next;
	}
}
void parse_pss_fuelling_mode_groups(xmlNode * node, PSSDispenceControl* data)
{
	g_printf("			Fuelling mode groups:\n");

	data->fuelling_mode_group_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingModeGroup") == 0 && data->fuelling_mode_group_count < MAX_FUELLING_MODE_GROUP_COUNT)
		{
			PSSFuellingModeGroup* fuelling_mode_group = &data->fuelling_mode_groups[data->fuelling_mode_group_count];

			fuelling_mode_group->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("				Fuelling mode group ID%d \n", fuelling_mode_group->id);

			fuelling_mode_group->fuelling_mode_group_item_count = 0;

			xmlNode * fuelling_mode_group_node = nodes->children;

			while(fuelling_mode_group_node)
			{
				if (fuelling_mode_group_node->type == XML_ELEMENT_NODE && strcmp((gchar*)fuelling_mode_group_node->name,"FuellingModeID") == 0 && fuelling_mode_group->fuelling_mode_group_item_count < MAX_GRADE_COUNT)
				{
					fuelling_mode_group->fuelling_mode_group_items[fuelling_mode_group->fuelling_mode_group_item_count].grade_id = atoi((gchar*)xmlGetProp(fuelling_mode_group_node,(guchar*)"GradeID"));

					xmlNode * fuelling_mode_id_node = fuelling_mode_group_node->children;

					if (fuelling_mode_id_node !=NULL && fuelling_mode_id_node->type == XML_TEXT_NODE)
					{
						fuelling_mode_group->fuelling_mode_group_items[fuelling_mode_group->fuelling_mode_group_item_count].fuelling_mode_id =  atoi((gchar*)fuelling_mode_id_node->content);
					}
					g_printf("					Fuelling mode ID%d Grade ID%d\n", fuelling_mode_group->fuelling_mode_group_items[fuelling_mode_group->fuelling_mode_group_item_count].fuelling_mode_id ,
							fuelling_mode_group->fuelling_mode_group_items[fuelling_mode_group->fuelling_mode_group_item_count].grade_id);

					fuelling_mode_group->fuelling_mode_group_item_count++;
				}
				fuelling_mode_group_node = fuelling_mode_group_node->next;
			}
			data->fuelling_mode_group_count++;
		}
		nodes = nodes->next;
	}
}
void parse_pss_dispense_limits(xmlNode * node, PSSDispenceLimits* data)
{
	g_printf("			Dispense limits:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int_tag(nodes, "GlobalMoneyLimit", &data->global_money_limit, "				Global money limit:");
		parse_int_tag(nodes, "GlobalVolumeLimit", &data->global_volume_limit, "				Global volume limit:");

		nodes = nodes->next;
	}
}

void parse_pss_dispence_control(xmlNode * node, PSSDispenceControl* data)
{
	g_printf("		Dispence control:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ServiceModes") == 0)
		{
			parse_pss_service_modes(nodes, data);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingModes") == 0)
		{
			parse_pss_fuelling_modes(nodes, data);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingModeGroups") == 0)
		{
			parse_pss_fuelling_mode_groups(nodes, data);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"DispenseLimits") == 0)
		{
			parse_pss_dispense_limits(nodes, &data->dispence_limits);
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipt_lines(xmlNode * node, PSSReceipts* data)
{
	g_printf("				Receipt lines:\n");

	data->receipt_line_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Line") == 0 && data->receipt_line_count < MAX_RECEIPT_LINE_COUNT)
		{
			PSSReceiptLine* receipt_line = &data->receipt_lines[data->receipt_line_count];
			receipt_line->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			receipt_line->definition = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"Definition"));

			g_printf("					Line ID%d : %s\n", receipt_line->id, receipt_line->definition);

			data->receipt_line_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipt_header(xmlNode * node, PSSReceiptHeader* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Line") == 0 && data->receipt_line_id_count < MAX_RECEIPT_LINE_COUNT)
		{
			data->receipt_line_ids[data->receipt_line_id_count] = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("						Line ID%d\n", data->receipt_line_ids[data->receipt_line_id_count]);

			data->receipt_line_id_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipt_headers(xmlNode * node, PSSReceipts* data)
{
	g_printf("				Receipt headers:\n");

	data->receipt_header_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ReceiptHeader") == 0 && data->receipt_header_count < MAX_RECEIPT_HEADER_COUNT)
		{
			PSSReceiptHeader* receipt_header = &data->receipt_headers[data->receipt_header_count];

			receipt_header->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("					Receipt header ID%d:\n", receipt_header->id);

			parse_pss_receipt_header(nodes, receipt_header);

			data->receipt_header_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipt_format(xmlNode * node, PSSReceiptFormat* data)
{
	xmlNode * nodes = node->children;

	data->receipt_line_id_count = 0;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Line") == 0 && data->receipt_line_id_count < MAX_RECEIPT_LINE_COUNT)
		{
			data->receipt_line_ids[data->receipt_line_id_count] = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("						Line ID%d\n", data->receipt_line_ids[data->receipt_line_id_count]);

			data->receipt_line_id_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipt_formats(xmlNode * node, PSSReceipts* data)
{
	g_printf("				Receipt formats:\n");

	data->receipt_format_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ReceiptFormat") == 0 && data->receipt_format_count < MAX_RECEIPT_FORMAT_COUNT)
		{
			PSSReceiptFormat* receipt_format = &data->receipt_formats[data->receipt_format_count];

			receipt_format->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("					Receipt format ID%d:\n", receipt_format->id);

			parse_pss_receipt_format(nodes, receipt_format);

			data->receipt_format_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_receipts(xmlNode * node, PSSReceipts* data)
{
	g_printf("			Receipts:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ReceiptLines") == 0)
		{
			parse_pss_receipt_lines(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ReceiptHeaders") == 0)
		{
			parse_pss_receipt_headers(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ReceiptFormats") == 0)
		{
			parse_pss_receipt_formats(nodes, data);
		}
		nodes = nodes->next;
	}
}

void parse_pss_payment_control(xmlNode * node, PSSPaymentControl* data)
{
	g_printf("		Payment control:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Receipts") == 0)
		{
			parse_pss_receipts(nodes, &data->receipts);
		}
		nodes = nodes->next;
	}
}

void parse_pss_general_configuration(xmlNode * node, PSSGeneralConfiguration* data)
{
	g_printf("	General configuration:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"GeneralParams") == 0)
		{
			parse_pss_general_params(nodes, &data->general_params);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"GeneralFunctions") == 0)
		{
			parse_pss_general_functions(nodes, &data->general_functions);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"DispenseControl") == 0)
		{
			parse_pss_dispence_control(nodes, &data->dispence_control);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PaymentControl") == 0)
		{
			parse_pss_payment_control(nodes, &data->payment_control);
		}
		nodes = nodes->next;
	}
}
void parse_pss_server_device_units(xmlNode * node, PSSServerDeviceUnits* data)
{
	g_printf("				Units:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"UnitID") == 0 && data->count < MAX_SERVER_DEVICE_UNIT_COUNT)
		{
			xmlNode* child_nodes = nodes->children;

			if (child_nodes !=NULL && child_nodes->type == XML_TEXT_NODE)
			{
				data->units[data->count] =  atoi((gchar*)child_nodes->content);
				g_printf("					UnitID%d\n", data->units[data->count]);
				data->count++;
			}
			child_nodes  = child_nodes->next;
		}
		nodes = nodes->next;
	}
}
//TODO
void parse_pss_server_devices(xmlNode * node, PSSServerDevices* data)
{
	g_printf("		Server devices:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ServerDevice") == 0 && data->count < MAX_SERVER_DEVICE_COUNT)
		{
			PSSServerDevice* server_device = &data->units[data->count];

			server_device->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			server_device->device_type = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Type"));

			if (server_device->ip_address!=NULL) { g_free(server_device->ip_address); server_device->ip_address = NULL; }
			server_device->ip_address = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"IpAddress"));

			server_device->port = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Port"));
			server_device->timeout = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Timeout"));

			if (server_device->guid!=NULL) { g_free(server_device->guid); server_device->guid = NULL; }
			server_device->guid = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"GUID"));

			xmlNode * c_node = nodes->children;

			g_printf("			Server device ID%d: Type = %d (%s), IP address %s, Port = %d, Timeout = %d, GUID = %s\n", server_device->id, server_device->device_type,
					server_device_type_to_str(server_device->device_type), server_device->ip_address, server_device->port, server_device->timeout, server_device->guid);

			while(c_node)
			{
				if (c_node->type == XML_ELEMENT_NODE && strcmp((gchar*)c_node->name,"Units") == 0)
				{
					server_device->units.count = 0;
					parse_pss_server_device_units(c_node, &server_device->units);
				}
				c_node = c_node->next;
			}


			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_tanks(xmlNode * node, PSSTanks* data)
{
	g_printf("		Tanks:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Tank") == 0 && data->count < MAX_TANK_COUNT)
		{
			PSSTank* tank = &data->units[data->count];

			tank->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			tank->height = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Height"));
			tank->volume = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Volume"));
			tank->weight = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Weight"));
			tank->density = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Density"));
			tank->temperature = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Temperature"));
			tank->waterlevel = atoi((gchar*)xmlGetProp(nodes,(guchar*)"WaterLevel"));
			tank->online = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Online"));
			g_printf("			Tank ID%d:\n", tank->id);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_fp_tank_connections(xmlNode * node, PSSGradeOption* data)
{
	g_printf("						Tank connections:\n");

	data->tank_connection_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Part") == 0 && data->tank_connection_count< MAX_TANK_COUNT)
		{
			PSSTankConnection* tank_connection = &data->tank_connections[data->tank_connection_count];

			tank_connection->tank_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"TankID"));

			xmlNode* child_nodes = nodes->children;

			if (child_nodes !=NULL && child_nodes->type == XML_TEXT_NODE)
			{
				tank_connection->part =  atoi((gchar*)child_nodes->content);
				g_printf("							Tank ID%d part: %d\n", tank_connection->tank_id, tank_connection->part);
			}

			data->tank_connection_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_fp_transactions(xmlNode * node, PSSTransactions* data)
{

	g_printf("				Fp sup transactions:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int_tag(nodes, "LastTransSeqNo", &data->last_trans_seq_no, "					Last trans seq no:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Transaction") == 0 && data->count < MAX_TRANSACTION_SEQ_COUNT)
		{
			PSSTransaction* transaction = &data->units[data->count];

			xmlNode* child_nodes = nodes->children;

			g_printf("					Transaction:\n");

			while(child_nodes)
			{
				parse_int16_tag(child_nodes, "TransSeqNo", &transaction->trans_seq_no, "						Trans seq no:");
				parse_int8_tag(child_nodes, "ServiceModeID", &transaction->sm_id, "						Service mode ID:");
				parse_int8_tag(child_nodes, "TransLockID", &transaction->trans_lock_id, "						Trans lock ID:");
				parse_int8_tag(child_nodes, "TransInfoFlags", &transaction->flags, "						Trans info flags:");
				parse_int_tag(child_nodes, "Money", &transaction->money, "						Money:");
				parse_int_tag(child_nodes, "Volume", &transaction->volume, "						Volume:");
				parse_int8_tag(child_nodes, "GradeID", &transaction->grade_id, "						Grade ID:");

				child_nodes = child_nodes->next;
			}

			data->count++;
		}
		nodes = nodes->next;
	}

}

void parse_pss_fp_preset_valid_grades(xmlNode * node, PSSPresetData* data)
{
	g_printf("					Valid grades:\n");

	xmlNode * nodes = node->children;

	data->valid_grades_count = 0;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"GradeId") == 0 && data->valid_grades_count < MAX_GRADE_COUNT)
		{
			xmlNode * tmp_node = nodes->children;

			if (tmp_node !=NULL && tmp_node->type == XML_TEXT_NODE)
			{
				data->valid_grades[data->valid_grades_count] =  atoi((gchar*)tmp_node->content);
				data->valid_grades_count++;
				g_printf("						GradeID: %d\n",data->valid_grades[data->valid_grades_count]);

			}

			parse_pss_fp_preset_valid_grades(nodes, data);
		}
		nodes = nodes->next;
	}

}

void parse_pss_fp_preset_data(xmlNode * node, PSSPresetData* data)
{
	g_printf("				Fp preset data:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "PosID", &data->pos_id, "					PosId:");
		parse_int8_tag(nodes, "FmgID", &data->fmg_id, "					FmgId:");
		parse_int8_tag(nodes, "PgID", &data->pg_id, "					PgId:");
		parse_int8_tag(nodes, "PresetType", (guint8*)&data->preset_type, "					PresetType:");
		parse_int_tag(nodes, "PresetValue", &data->preset_value, "					PresetValue:");
		parse_int8_tag(nodes, "SmID", &data->sm_id, "					SmId:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ValidGrades") == 0)
		{
			parse_pss_fp_preset_valid_grades(nodes, data);
		}
		nodes = nodes->next;
	}

}
void parse_pss_fp_grade_options(xmlNode * node, PSSFuellingPoint* data)
{
	g_printf("				Fp grade options:\n");

	data->grade_option_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"GradeOption") == 0 && data->grade_option_count < MAX_NOZZLE_COUNT)
		{
			PSSGradeOption* grade_option = &data->grade_options[data->grade_option_count];

			grade_option->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("					Grade option ID%d:\n", grade_option->id);

			xmlNode* child_nodes = nodes->children;

			while(child_nodes)
			{
				parse_int8_tag(child_nodes, "GradeID", &grade_option->grade_id, "						Grade ID:");
				parse_ascii_tag(child_nodes, "NozzleID", grade_option->nozzle_id, MAX_NOZZLE_ID_LENGTH,"						Nozzle ID:");
				parse_int8_tag(child_nodes, "NozzleTagReaderID", &grade_option->nozzle_tag_reader_id, "						Nozzle tag reader ID:");
				parse_int_tag(child_nodes, "VolumeTotal", &grade_option->volume_total, "				Volume total:");
				parse_int_tag(child_nodes, "MoneyTotal", &grade_option->money_total, "				Money total:");

				if (child_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)child_nodes->name,"TankConnections") == 0)
				{
					parse_pss_fp_tank_connections(child_nodes, grade_option);
				}



				child_nodes = child_nodes->next;
			}

			data->grade_option_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_pump_interface_decimal_positions(xmlNode * node, PSSFuellingPoint* data)
{
	g_printf("				Pump interface decimal positions:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "InPrice", &data->pump_interface_decimal_positions.in_price, "					In price:");
		parse_int8_tag(nodes, "InVolume", &data->pump_interface_decimal_positions.in_volume, "					In volume:");
		parse_int8_tag(nodes, "InMoney", &data->pump_interface_decimal_positions.in_money, "					In money:");
		parse_int8_tag(nodes, "InVolumeTotal", &data->pump_interface_decimal_positions.in_volume_total, "					In volume total:");
		parse_int8_tag(nodes, "InMoneyTotal", &data->pump_interface_decimal_positions.in_money_total, "					In money total:");

		nodes = nodes->next;
	}
}

void parse_pss_fp_operation_modes(xmlNode * node, PSSFuellingPoint* data)
{
	g_printf("				Fp operation modes:\n");

	data->operation_mode_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"OperationMode") == 0 && data->operation_mode_count < MAX_OPERATION_MODE_COUNT)
		{
			PSSOperationMode* operation_mode = &data->operation_modes[data->operation_mode_count];

			operation_mode->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("					Operation mode ID%d:\n", operation_mode->id);

			xmlNode* fp_service_mode = nodes->children;

			while(fp_service_mode)
			{
				if (fp_service_mode->type == XML_ELEMENT_NODE && strcmp((gchar*)fp_service_mode->name,"FPServiceMode") == 0 && operation_mode->service_mode_count < MAX_SERVICE_MODE_COUNT)
				{
					g_printf("						Service mode:\n");

					xmlNode* sm_detal = fp_service_mode->children;

					PSSOperationModeServiceMode* tmp = &operation_mode->service_modes[operation_mode->service_mode_count];

					while(sm_detal)
					{
						parse_int8_tag(sm_detal, "ServiceModeID", &tmp->service_mode_id, "							Service mode ID:");
						parse_int8_tag(sm_detal, "FuellingModeGroupID", &tmp->fuelling_mode_group_id, "							Fuelling mode group ID:");
						parse_int8_tag(sm_detal, "PriceGroupID", &tmp->price_group_id, "							price group ID:");

						sm_detal = sm_detal->next;
					}

					operation_mode->service_mode_count++;
				}

				fp_service_mode = fp_service_mode->next;
			}
			data->operation_mode_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_fp_limits(xmlNode * node, PSSFuellingPoint* data)
{
	g_printf("				Fp limits:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int_tag(nodes, "FloorLimitMargin", &data->limits.floor_limit_margin, "					Floor limit margin:");

		nodes = nodes->next;
	}
}

void parse_pss_fuelling_point(xmlNode * node, PSSFuellingPoint* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "PSSPortNo", &data->pss_port_no, "				PSS Port no:");
		parse_int8_tag(nodes, "ServerDeviceID", &data->server_device_id, "				Server device ID:");
		parse_binary_tag(nodes, "InterfaceTypeGeneral", &data->interface_type_general, "				Interface type general:");
		parse_binary_tag(nodes, "InterfaceTypeProtocol", &data->interface_type_protocol, "				Interface type protocol:");
		parse_int16_tag(nodes, "PssExtProtocolId", &data->pss_ext_protocol_id, "				PSS ext protocol ID:");
		parse_int_tag(nodes, "PhysicalAddress", &data->phisical_address, "				Phisical address:");
		parse_int8_tag(nodes, "PhysicalAddressType", &data->phisical_address_type, "				Physical address type:");
		parse_int16_tag(nodes, "PhysicalAddressPort", &data->phisical_address_port, "				Physical address port:");
		parse_int8_tag(nodes, "DeviceSubAddress", &data->device_sub_address, "				Device sub address:");
		parse_bool_tag(nodes, "IsClose", &data->is_close, "				Is close:");

		parse_int8_tag(nodes, "SmID", &data->sm_id, "				Service Mode ID:");
		parse_int8_tag(nodes, "MainState", (guint8*)&data->main_state, "				Main State:");
		parse_int8_tag(nodes, "SubState", &data->sub_state, "				Sub State:");
		parse_int8_tag(nodes, "SubState2", &data->sub_state2, "				Sub State 2:");
		parse_int8_tag(nodes, "LockedID", &data->locked_id, "				Locked ID:");
		parse_int8_tag(nodes, "GradeID", &data->grade_id, "				Grade ID:");
		parse_int_tag(nodes, "FuellingVolume", &data->fuelling_volume, "				Fuelling Volume:");
		parse_int_tag(nodes, "FuellingMoney", &data->fuelling_money, "				Fuelling Money:");
		parse_ascii_tag(nodes, "AttendantAccauntID", data->attendant_accaunt_id, MAX_ATTENDANT_ACCAUNT_ID_LENGTH, "				Attendant Accaunt ID:");

		parse_ascii_tag(nodes, "PumpInterfaceConfigString", data->config_string, MAX_CONFIG_STRING_LENGTH, "				Config string:");
		parse_int16_tag(nodes, "NominalNormalSpeed", &data->nominal_normal_speed, "				Nominal normal speed:");
		parse_int16_tag(nodes, "NominalHighSpeed", &data->nominal_high_speed, "				Nominal high speed:");
		parse_int16_tag(nodes, "HighSpeedTriggerLevel", &data->high_speed_trigger_level, "				High speed trigger level:");

		parse_int8_tag(nodes, "OperationModeNo", &data->operation_mode_no, "				Operation mode no:");

		parse_int_tag(nodes, "VolumeTotal", &data->volume_total, "				Volume total:");
		parse_int_tag(nodes, "MoneyTotal", &data->money_total, "				Money total:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PresetData") == 0)
		{
			parse_pss_fp_preset_data(nodes, &data->preset_data);
		}

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"GradeOptions") == 0)
		{
			parse_pss_fp_grade_options(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PumpInterfaceDecimalPositions") == 0)
		{
			parse_pss_pump_interface_decimal_positions(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"OperationModes") == 0)
		{
			parse_pss_fp_operation_modes(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Limits") == 0)
		{
			parse_pss_fp_limits(nodes, data);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"SupTransactions") == 0)
		{
			parse_pss_fp_transactions(nodes, &data->sup_transactions);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"UnsupTransactions") == 0)
		{
			parse_pss_fp_transactions(nodes, &data->unsup_transactions);
		}

		nodes = nodes->next;
	}
}

void parse_pss_fuelling_points(xmlNode * node, PSSFuellingPoints* data)
{
	g_printf("		Fuelling points:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingPoint") == 0 && data->count < MAX_FUELLING_POINT_COUNT)
		{
			PSSFuellingPoint* fuelling_point = &data->units[data->count];

			fuelling_point->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("			Fuelling point ID%d:\n", fuelling_point->id);

			parse_pss_fuelling_point(nodes, fuelling_point);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_tg_alarms(xmlNode * node, PSSAlarms* data)
{
	g_printf("					Alarms:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"HeightAlarms") == 0)
		{
			data->height_alarm_count = 0;
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"VolumeAlarms") == 0)
		{
			data->volume_alarm_count = 0;
		}
		nodes = nodes->next;
	}
}

void parse_pss_tg_extra_install_data(xmlNode * node, PSSExtraInstallData* data)
{
	g_printf("				Extra install data:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Alarms") == 0)
		{
			parse_pss_tg_alarms(nodes, &data->alarms);
		}
		nodes = nodes->next;
	}
}


void parse_pss_tank_gauge(xmlNode * node, PSSTankGauge* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "PSSPortNo", &data->pss_port_no, "				PSS Port no:");
		parse_int8_tag(nodes, "ServerDeviceID", &data->server_device_id, "				Server device ID:");
		parse_binary_tag(nodes, "InterfaceType", &data->interface_type, "				Interface type:");
		parse_int8_tag(nodes, "PhysicalAddress", &data->phisical_address, "				Physical address:");
		parse_int_tag(nodes, "TankHeight", &data->tank_height, "				TankHeight:");
		parse_int8_tag(nodes, "MainState", (guint8*)&data->main_state, "				Main state:");
		parse_int8_tag(nodes, "SubState", &data->sub_state, "				SubState:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PhysicalSubAddress") == 0)
		{
			data->phisical_sub_address = atoi((gchar*)xmlGetProp(nodes,(guchar*)"Value"));
			g_printf("				Physical sub address: %d\n",data->phisical_sub_address);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Tank") == 0)
		{
			data->tank_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("				Tank ID: %d\n",data->tank_id);
		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ExtraInstallData") == 0)
		{
			parse_pss_tg_extra_install_data(nodes, &data->extra_install_data);
		}
		nodes = nodes->next;
	}
}

void parse_pss_tank_gauge_sub_device(xmlNode * node, PSSTankGaugeSubDevice* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_binary_tag(nodes, "InterfaceType", &data->interface_type, "				Interface type:");
		parse_int8_tag(nodes, "PSSChannelNo", &data->pss_channel_no, "				PSS Channel no:");

		guint8 physical_address = 0;
		parse_int8_tag(nodes, "PhysicalAddress", &physical_address, "				Physical address:");

		data->main_address = physical_address & 0x3F;
		data->sub_address = (physical_address & 0xC0) >> 6;

		parse_int8_tag(nodes, "PhysicalSubAddress", &data->phisical_sub_address, "				Physical sub address:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"SubDeviceIds") == 0)
		{
			xmlNode* sub_device_id = nodes->children;

			while(sub_device_id)
			{
				if (sub_device_id->type == XML_ELEMENT_NODE && strcmp((gchar*)sub_device_id->name,"SubDeviceID") == 0 && data->sub_dev_ids_count < MAX_TGS_SUB_DEV_ID_COUNT)
				{
					data->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

					g_printf("						SubDeviceId: %d\n", data->id);

					data->sub_dev_ids_count++;
				}

				sub_device_id = sub_device_id->next;
			}
		}
		nodes = nodes->next;
	}
}

void parse_pss_tank_gauges(xmlNode * node, PSSTankGauges* data)
{
	g_printf("		Tank gauges:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"TankGauge") == 0 && data->count< MAX_TANK_COUNT)
		{
			PSSTankGauge* tank_gauge = &data->units[data->count];

			tank_gauge->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("			Tank gauge ID%d:\n", tank_gauge->id);

			parse_pss_tank_gauge(nodes, tank_gauge);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_tank_gauge_sub_devices(xmlNode * node, PSSTankGaugeSubDevices* data)
{
	g_printf("		Tank gauge sub devices:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"TankGaugeSubDevice") == 0 && data->count< MAX_TANK_COUNT)
		{
			PSSTankGaugeSubDevice* tank_gauge_sub_device = &data->units[data->count];

			tank_gauge_sub_device->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));

			g_printf("			Tank gauge sub device ID%d:\n", tank_gauge_sub_device->id);

			parse_pss_tank_gauge_sub_device(nodes, tank_gauge_sub_device);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_price_position_options(xmlNode * node, PSSPricePole* data)
{
	g_printf("		Price position option:\n");

	data->option_count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PricePositionOption") == 0 && data->option_count< MAX_GRADE_COUNT)
		{
			PSSPricePositionOption* price_position_option = &data->options[data->option_count];

			price_position_option->option_no = atoi((gchar*)xmlGetProp(nodes,(guchar*)"No"));
			price_position_option->fc_price_group_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"FcPriceGroupId"));
			price_position_option->fc_grade_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"FcPriceGroupId"));

			g_printf("			Option No%d, FcPriceGroupId %d, fcGradeId %d\n", price_position_option->option_no, price_position_option->fc_price_group_id, price_position_option->fc_grade_id);

			data->option_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_price_pole(xmlNode * node, PSSPricePole* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "PSSPortNo", &data->pss_port_no, "				PSS Port no:");
		parse_int8_tag(nodes, "ServerDeviceID", &data->server_device_id, "				Server device ID:");
		parse_binary_tag(nodes, "InterfaceType", &data->interface_type, "				Interface type:");
		parse_int8_tag(nodes, "PhysicalAddress", &data->phisical_address, "				Physical address:");
		parse_int8_tag(nodes, "MainState",(guint8*) &data->main_state, "				MainState:");
		parse_int8_tag(nodes, "SubState", &data->sub_state, "				SubState:");
		parse_int8_tag(nodes, "NoPricePositionOption", &data->option_count, "				OptionCount:");

		//TODO
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PricePositionOptions") == 0)
		{
			parse_pss_price_position_options(nodes, data);
		}

		nodes = nodes->next;
	}
}

void parse_pss_price_poles(xmlNode * node, PSSPricePoles* data)
{
	g_printf("		Price poles:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PricePole") == 0 && data->count< MAX_PRICE_POLE_COUNT)
		{
			PSSPricePole* price_pole = &data->units[data->count];

			price_pole->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("			Price pole ID%d:\n", price_pole->id);

			parse_pss_price_pole(nodes, price_pole);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_pt_select_options(xmlNode * node, PSSPaymentTerminal* data)
{
	g_printf("				Select options:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"SelectOption") == 0 )
		{
			data->select_options[data->select_option_count].id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("					ID%d\n",data->select_options[data->select_option_count].id);

			data->select_option_count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_payment_terminal(xmlNode * node, PSSPaymentTerminal* data)
{
	xmlNode * nodes = node->children;

	while(nodes)
	{
		parse_int8_tag(nodes, "PSSPortNo", &data->pss_port_no, "				PSS Port no:");
		parse_int8_tag(nodes, "ServerDeviceID", &data->server_device_id, "				Server device ID:");
		parse_binary_tag(nodes, "InterfaceType", &data->interface_type, "				Interface type:");
		parse_int8_tag(nodes, "PhysicalAddress", &data->phisical_address, "				Physical address:");

		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"SelectOptions") == 0)
		{
			parse_pss_pt_select_options(nodes, data);
		}

		nodes = nodes->next;
	}
}

void parse_pss_payment_terminals(xmlNode * node, PSSPaymentTerminals* data)
{
	g_printf("		Payment terminals:\n");

	data->count = 0;

	xmlNode * nodes = node->children;

	while(nodes)
	{
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PaymentTerminal") == 0 && data->count< MAX_PAYMENT_TERMINAL_COUNT)
		{
			PSSPaymentTerminal* payment_terminal = &data->units[data->count];

			payment_terminal->id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ID"));
			g_printf("			Payment terminal ID%d:\n", payment_terminal->id);

			parse_pss_payment_terminal(nodes, payment_terminal);

			data->count++;
		}
		nodes = nodes->next;
	}
}

void parse_pss_devices(xmlNode * node, PSSDevices* data)
{
	g_printf("	Devices:\n");

	xmlNode * nodes = node->children;

	while(nodes)
	{
//		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"ServerDevices") == 0)
//		{
//			parse_pss_server_devices(nodes, &data->server_devices);
//		}
		if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"Tanks") == 0)
		{
			parse_pss_tanks(nodes, &data->tanks);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"FuellingPoints") == 0)
		{
			parse_pss_fuelling_points(nodes, &data->fuelling_points);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"TankGauges") == 0)
		{
			parse_pss_tank_gauges(nodes, &data->tank_gauges);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"TankGaugeSubDevices") == 0)
		{
			parse_pss_tank_gauge_sub_devices(nodes, &data->tank_gauge_sub_devices);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PricePoles") == 0)
		{
			parse_pss_price_poles(nodes, &data->price_poles);
		}
		else if (nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nodes->name,"PaymentTerminals") == 0)
		{
			parse_pss_payment_terminals(nodes, &data->payment_terminals);
		}
		nodes = nodes->next;
	}
}

void parse_pss_common(xmlNode * node, PSSCommon* data)
{
	printf("Common:\n");

	xmlNode * nodes = node->children;

    while(nodes)
    {
        if(nodes->type == XML_ELEMENT_NODE)
        {
        	if (strcmp((gchar*)nodes->name,"FcStatus") == 0)
        	{
        		printf("	FcStatus:\n");
        		PSSFcStatus* fc_status = &data->fc_status;

        		xmlNode * status_nodes = nodes->children;

        	    while(status_nodes)
        	    {
        	    	parse_int8_tag(status_nodes, "FcStatus1Flags", &fc_status->fc_status_1_flags, "		FcStatus1Flag:");
        	    	parse_int8_tag(status_nodes, "FcStatus2Flags", &fc_status->fc_status_2_flags, "		FcStatus2Flag:");
        	    	parse_int8_tag(status_nodes, "FcServiceMsgSeqNo", &fc_status->fc_service_msg_seq_no, "		FcServiceMsgSeqNo:");
        	    	parse_time_t_tag(status_nodes, "FcMasterResetDateAndTime", &fc_status->fc_master_reset_date_and_time, "		FcMasterResetDateAndTime:");
        	    	parse_int8_tag(status_nodes, "FcMasterResetCode", &fc_status->fc_master_reset_code, "		FcMasterResetCode:");
        	    	parse_time_t_tag(status_nodes, "FcResetDateAndTime", &fc_status->fc_reset_date_and_time, "		FcResetDateAndTime:");
        	    	parse_int8_tag(status_nodes, "FcResetCode", &fc_status->fc_reset_code, "		FcResetCode:");
        	    	parse_int8_tag(status_nodes, "FcPriceSetId", &fc_status->fc_price_set_id, "		FcPriceSetId:");
        	    	parse_time_t_tag(status_nodes, "FcPriceSetDateAndTime", &fc_status->fc_price_set_date_and_time, "		FcPriceSetDateAndTime:");
        	    	parse_int8_tag(status_nodes, "FcOperationModeNo", &fc_status->fc_operation_mode_no, "		FcOperationModeNo:");
        	    	parse_time_t_tag(status_nodes, "FcOperationModeDateAndTime", &fc_status->fc_operation_mode_date_and_time, "		FcOperationModeDateAndTime:");

        	    	status_nodes = status_nodes->next;
        	    }
        	}
        }
        nodes = nodes->next;
    }
}

void parse_pss_config(xmlNode * node, PSSData* data)
{
	xmlNode * nodes = node->children;

    while(nodes)
    {
        if(nodes->type == XML_ELEMENT_NODE)
        {
        	if (strcmp((gchar*)nodes->name,"Common") == 0)
        	{
        		parse_pss_common(nodes, &data->common);
        	}
        	else if (strcmp((gchar*)nodes->name,"GeneralConfiguration") == 0)
        	{
        		parse_pss_general_configuration(nodes, &data->general_configuration);
        	}
        	else if (strcmp((gchar*)nodes->name,"Devices") == 0)
        	{
        		parse_pss_devices(nodes, &data->devices);
        	}
        }
        nodes = nodes->next;
    }
}

gboolean load_pss_xml_data(const gchar* filename, PSSData* data)
{

	fprintf(stderr,"Load PSS configuration: %s\n", filename);

	xmlDoc *doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		fprintf(stderr,"Could not parse %s\n", filename);
		return FALSE;
	}

	xmlNode *root_element = xmlDocGetRootElement(doc);

	parse_pss_config(root_element, data);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	return TRUE;
}

void add_grades(xmlNodePtr root_node, PSSGeneralFunctions* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "Grades");

	if (data->grade_count > 0)
	{
		for (guint8 i = 0; i < data->grade_count; i++)
		{
			PSSGrade* grade = &data->grades[i];

			xmlNodePtr grade_node = xmlNewNode(NULL, BAD_CAST "Grade");

			xmlSetProp(grade_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d", grade->id));
			xmlSetProp(grade_node, (guchar*)"Name", (guchar*)g_strdup_printf("%s", grade->name));

			xmlAddChild(node,grade_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_uint32_node(xmlNodePtr parent_node, const gchar* node_name, guint32 value )
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
	xmlNodePtr node_val = xmlNewText((guchar*)g_strdup_printf("%d", value));

	xmlAddChild(node, node_val);
	xmlAddChild(parent_node,node);
}

void add_ascii_node(xmlNodePtr parent_node, const gchar* node_name, gchar*  value )
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
	xmlNodePtr node_val = xmlNewText((guchar*)value);

	xmlAddChild(node, node_val);
	xmlAddChild(parent_node,node);
}

void add_boolean_node(xmlNodePtr parent_node, const gchar* node_name, gboolean value )
{
	const gchar* text = value ? g_strdup("Yes") : g_strdup("No");

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
	xmlNodePtr node_val = xmlNewText((guchar*)g_strdup_printf("%s", text));

	xmlAddChild(node, node_val);
	xmlAddChild(parent_node,node);
}

void add_binary_node(xmlNodePtr parent_node, const gchar* node_name, guint32 value, guint8 size )
{
	gchar* buffer = malloc(size+1);
	memset(buffer, 0, size+1);

	guint8 pos = 0;

	for (guint8 i = size; i > 0; i-- )
	{
		buffer[pos++] = (value & (1 << (i - 1))) ? 0x31 : 0x30;
	}

	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST node_name);
	xmlNodePtr node_val = xmlNewText((guchar*)g_strdup_printf("%s", buffer));
	xmlAddChild(node, node_val);
	xmlAddChild(parent_node,node);

	g_free(buffer);
}

void add_price_groups(xmlNodePtr root_node, PSSGeneralFunctions* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "PriceGroups");

	if (data->price_group_count > 0)
	{
		for (guint8 i = 0; i < data->price_group_count; i++)
		{
			xmlNodePtr price_group_node = xmlNewNode(NULL, BAD_CAST "PriceGroup");

			PSSPriceGroup* price_group = &data->price_groups[i];

			xmlSetProp(price_group_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d", price_group->id));

			if (price_group->grade_price_count > 0)
			{
				for (guint8 j = 0; j < price_group->grade_price_count; j++)
				{
					add_uint32_node(price_group_node, "GradeID",  price_group->grade_prices[j].grade_id);
					add_uint32_node(price_group_node, "Price",  price_group->grade_prices[j].price);
				}
			}
			xmlAddChild(node,price_group_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_general_params(xmlNodePtr root_node, PSSGeneralParams* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "GeneralParams");

	add_uint32_node(node, "PriceIncreaseDelay", data->price_increase_delay);
	add_uint32_node(node, "PriceDecreaseDelay", data->price_decrease_delay);
	add_ascii_node(node, "DefaultLangugeCode", data->default_language_code);
	add_boolean_node(node, "DisableFpTotalsError", data->disable_fp_totals_error);
	add_boolean_node(node, "EnableDemoEncription", data->enable_demo_encription);
	add_ascii_node(node, "CurrencyCode", data->currency_code);
	add_uint32_node(node, "FcPumpTotalsHandlingMode", data->fc_pump_totals_handling_mode);
	add_ascii_node(node, "FcShiftNo", data->fc_shift_no);
	add_uint32_node(node, "VATRate", data->vat_rate);

	xmlAddChild(root_node,node);
}

void add_general_functions(xmlNodePtr root_node, PSSGeneralFunctions* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "GeneralFunctions");

	add_grades(node, data);
	add_price_groups(node, data);

	xmlAddChild(root_node,node);
}

void add_service_modes(xmlNodePtr root_node, PSSDispenceControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "ServiceModes");

	if (data->service_mode_count > 0)
	{
		for (guint8 i = 0; i < data->service_mode_count; i++)
		{
			xmlNodePtr service_mode_node = xmlNewNode(NULL, BAD_CAST "ServiceMode");

			PSSServiceMode* service_mode = &data->service_modes[i];

			xmlSetProp(service_mode_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d", service_mode->id));

			add_uint32_node(service_mode_node, "AutoAuthorizeLimit",  service_mode->auto_autorize_limit);
			add_uint32_node(service_mode_node, "MaxPreAuthTime",  service_mode->max_pre_auth_time);
			add_uint32_node(service_mode_node, "MaxNzLayDownTime",  service_mode->max_nz_lay_down_time);
			add_boolean_node(service_mode_node, "ZeroTransToPos",  service_mode->zero_trans_to_pos);
			add_boolean_node(service_mode_node, "MoneyDueInTransBufferStatus",  service_mode->money_due_in_trans_buffer_status);
			add_uint32_node(service_mode_node, "MinTransVol",  service_mode->min_trans_vol);
			add_uint32_node(service_mode_node, "MinTransMoney",  service_mode->min_trans_money);
			add_uint32_node(service_mode_node, "SupTransBufferSize",  service_mode->sup_trans_buffer_size);
			add_uint32_node(service_mode_node, "UnsupTransBufferSize",  service_mode->unsup_trans_buffer_size);
			add_boolean_node(service_mode_node, "StoreAtPreAuthorize",  service_mode->store_at_pre_authorize);
			add_boolean_node(service_mode_node, "VolInTransBufferStatus",  service_mode->vol_in_trans_buffer_status);
			add_boolean_node(service_mode_node, "AuthorizeAtModeSelection",  service_mode->authorize_at_mode_selection);
			add_uint32_node(service_mode_node, "MnoConsecutiveZeroTrans",  service_mode->mno_consecutive_zero_trans);
			add_uint32_node(service_mode_node, "AutoClearTransDelayTime",  service_mode->auto_clear_trans_delay_time);
			add_uint32_node(service_mode_node, "PumpLightMode",  service_mode->pump_light_mode);
			add_boolean_node(service_mode_node, "StopFpOnVehicleTag",  service_mode->stop_fp_on_vehicle_tag);
			add_boolean_node(service_mode_node, "UseVehicleTagReadingButton",  service_mode->use_vehicle_tag_reading_button);
			add_uint32_node(service_mode_node, "AutoUnlockTransDelayTime",  service_mode->auto_unlock_trans_delay_time);

			xmlAddChild(node,service_mode_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_fuelling_modes(xmlNodePtr root_node, PSSDispenceControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "FuellingModes");

	if (data->fuelling_mode_count > 0)
	{
		for (guint8 i = 0; i < data->fuelling_mode_count; i++)
		{
			xmlNodePtr fuelling_mode_node = xmlNewNode(NULL, BAD_CAST "FuellingMode");

			PSSFuellingMode* fuelling_mode = &data->fuelling_modes[i];

			xmlSetProp(fuelling_mode_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d", fuelling_mode->id));

			add_binary_node(fuelling_mode_node, "FuellingType", fuelling_mode->fuelling_type, 16);
			add_uint32_node(fuelling_mode_node, "MaxTimeToReachMinLimit",  fuelling_mode->max_time_to_reach_min_limit);
			add_uint32_node(fuelling_mode_node, "MaxTimeWithoutProgress",  fuelling_mode->max_time_without_progress);
			add_uint32_node(fuelling_mode_node, "MaxTransVolume",  fuelling_mode->max_trans_volume);
			add_uint32_node(fuelling_mode_node, "MaxTransMoney",  fuelling_mode->max_trans_money);
			add_uint32_node(fuelling_mode_node, "MaxFuellingTime",  fuelling_mode->max_fuelling_time);
			add_uint32_node(fuelling_mode_node, "MaxPresetVolOverrunErrLimit",  fuelling_mode->max_preset_vol_overrun_err_limit);
			add_uint32_node(fuelling_mode_node, "ClrDisplayDelayTime",  fuelling_mode->clr_display_delay_time);
			add_boolean_node(fuelling_mode_node, "ClrDisplayWhenCurTrDisappear",  fuelling_mode->clr_display_when_cur_tr_disappear);
			add_uint32_node(fuelling_mode_node, "MinSubPumpRuntimeBeforeStart",  fuelling_mode->min_sub_pump_runtime_before_start);
			add_uint32_node(fuelling_mode_node, "MaxTransVolume_e",  fuelling_mode->max_trans_volume_e);
			add_uint32_node(fuelling_mode_node, "MaxTransMoney_e",  fuelling_mode->max_trans_money_e);

			xmlAddChild(node,fuelling_mode_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_fuelling_mode_groups(xmlNodePtr root_node, PSSDispenceControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "FuellingModeGroups");

	if (data->fuelling_mode_group_count > 0)
	{
		for (guint8 i = 0; i < data->fuelling_mode_group_count; i++)
		{
			xmlNodePtr fuelling_mode_group_node = xmlNewNode(NULL, BAD_CAST "FuellingModeGroup");

			PSSFuellingModeGroup* fuelling_mode_group = &data->fuelling_mode_groups[i];

			xmlSetProp(fuelling_mode_group_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d", fuelling_mode_group->id));

			if (fuelling_mode_group->fuelling_mode_group_item_count > 0)
			{
				for (guint8 j = 0; j < fuelling_mode_group->fuelling_mode_group_item_count; j++)
				{
					xmlNodePtr fuelling_mode_id_node = xmlNewNode(NULL, BAD_CAST "FuellingModeID");

					xmlSetProp(fuelling_mode_id_node, (guchar*)"GradeID", (guchar*)g_strdup_printf("%d",fuelling_mode_group->fuelling_mode_group_items[j].grade_id));

					xmlNodePtr node_val = xmlNewText((guchar*)g_strdup_printf("%d", fuelling_mode_group->fuelling_mode_group_items[j].fuelling_mode_id));
					xmlAddChild(fuelling_mode_id_node, node_val);

					xmlAddChild(fuelling_mode_group_node,fuelling_mode_id_node);
				}
			}
			xmlAddChild(node,fuelling_mode_group_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_dispence_limits(xmlNodePtr root_node, PSSDispenceControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "DispenseLimits");

	add_uint32_node(node, "GlobalMoneyLimit", data->dispence_limits.global_money_limit);
	add_uint32_node(node, "GlobalVolumeLimit", data->dispence_limits.global_volume_limit);

	xmlAddChild(root_node,node);
}

void add_dispence_control(xmlNodePtr root_node, PSSDispenceControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "DispenseControl");

	add_service_modes(node, data);
	add_fuelling_modes(node, data);
	add_fuelling_mode_groups(node, data);
	add_dispence_limits(node, data);

	xmlAddChild(root_node,node);
}

void add_receipt_lines(xmlNodePtr root_node, PSSReceipts* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "ReceiptLines");

	if (data->receipt_line_count > 0)
	{
		for (guint8 i = 0; i < data->receipt_line_count; i++)
		{
			PSSReceiptLine* receipt_line = &data->receipt_lines[i];

			xmlNodePtr line_node = xmlNewNode(NULL, BAD_CAST "Line");
			xmlSetProp(line_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",receipt_line->id));
			xmlSetProp(line_node, (guchar*)"Definition", (guchar*)g_strdup_printf("%s",receipt_line->definition));

			xmlAddChild(node,line_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_receipt_headers(xmlNodePtr root_node, PSSReceipts* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "ReceiptHeaders");

	if (data->receipt_header_count > 0)
	{
		for (guint8 i = 0; i < data->receipt_header_count; i++)
		{
			PSSReceiptHeader* receipt_header = &data->receipt_headers[i];

			xmlNodePtr receipt_header_node = xmlNewNode(NULL, BAD_CAST "ReceiptHeader");
			xmlSetProp(receipt_header_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",receipt_header->id));

			if (receipt_header->receipt_line_id_count > 0)
			{
				for (guint8 j = 0; j < receipt_header->receipt_line_id_count; j++)
				{
					xmlNodePtr header_line_node = xmlNewNode(NULL, BAD_CAST "Line");
					xmlSetProp(header_line_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",receipt_header->receipt_line_ids[j]));
					xmlAddChild(receipt_header_node,header_line_node);
				}
			}
			xmlAddChild(node,receipt_header_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_receipt_formats(xmlNodePtr root_node, PSSReceipts* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "ReceiptFormats");

	if (data->receipt_format_count > 0)
	{
		for (guint8 i = 0; i < data->receipt_format_count; i++)
		{
			PSSReceiptFormat* receipt_format = &data->receipt_formats[i];

			xmlNodePtr receipt_format_node = xmlNewNode(NULL, BAD_CAST "ReceiptFormat");
			xmlSetProp(receipt_format_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",receipt_format->id));

			if (receipt_format->receipt_line_id_count > 0)
			{
				for (guint8 j = 0; j < receipt_format->receipt_line_id_count; j++)
				{
					xmlNodePtr header_line_node = xmlNewNode(NULL, BAD_CAST "Line");
					xmlSetProp(header_line_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",receipt_format->receipt_line_ids[j]));
					xmlAddChild(receipt_format_node,header_line_node);
				}
			}
			xmlAddChild(node,receipt_format_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_receipts(xmlNodePtr root_node, PSSReceipts* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "Receipts");

	add_receipt_lines(node, data);
	add_receipt_headers(node, data);
	add_receipt_formats(node, data);

	xmlAddChild(root_node,node);
}

void add_payment_control(xmlNodePtr root_node, PSSPaymentControl* data)
{
	xmlNodePtr node = xmlNewNode(NULL, BAD_CAST "PaymentControl");

	add_receipts(node, &data->receipts);

	xmlAddChild(root_node,node);
}

void add_general_configuration(xmlNodePtr root_node, PSSGeneralConfiguration* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"GeneralConfiguration");

    add_general_params(node, &data->general_params);
    add_general_functions(node, &data->general_functions);
    add_dispence_control(node, &data->dispence_control);
    add_payment_control(node, &data->payment_control);

    xmlAddChild(root_node,node);
}

void add_server_device_unts(xmlNodePtr root_node, PSSServerDeviceUnits* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"Units");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		add_uint32_node(node, "UnitID",  data->units[i]);
    	}
    }

    xmlAddChild(root_node,node);
}

void add_devices_server_devices(xmlNodePtr root_node, PSSServerDevices* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"ServerDevices");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSServerDevice* server_device = &data->units[i];
    		xmlNodePtr server_device_node = xmlNewNode(NULL, BAD_CAST "ServerDevice");

    		xmlSetProp(server_device_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",server_device->id));
    		xmlSetProp(server_device_node, (guchar*)"Type", (guchar*)g_strdup_printf("%d",server_device->device_type));
    		xmlSetProp(server_device_node, (guchar*)"IpAddress", (guchar*)g_strdup_printf("%s",server_device->ip_address));
    		xmlSetProp(server_device_node, (guchar*)"Port", (guchar*)g_strdup_printf("%d",server_device->port));
    		xmlSetProp(server_device_node, (guchar*)"Timeout", (guchar*)g_strdup_printf("%d",server_device->timeout));
    		xmlSetProp(server_device_node, (guchar*)"GUID", (guchar*)g_strdup_printf("%s",server_device->guid));

    		add_server_device_unts(server_device_node, &server_device->units);

    		xmlAddChild(node,server_device_node);

    	}
    }
    xmlAddChild(root_node,node);

}

void add_devices_tanks(xmlNodePtr root_node, PSSTanks* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"Tanks");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSTank* tank = &data->units[i];
    		xmlNodePtr tank_node = xmlNewNode(NULL, BAD_CAST "Tank");
    		xmlSetProp(tank_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",tank->id));
    		xmlSetProp(tank_node, (guchar*)"Height", (guchar*)g_strdup_printf("%d",tank->height));
    		xmlSetProp(tank_node, (guchar*)"Volume", (guchar*)g_strdup_printf("%d",tank->volume));
    		xmlSetProp(tank_node, (guchar*)"Weight", (guchar*)g_strdup_printf("%d",tank->weight));
    		xmlSetProp(tank_node, (guchar*)"Density", (guchar*)g_strdup_printf("%d",tank->density));
    		xmlSetProp(tank_node, (guchar*)"Temperature", (guchar*)g_strdup_printf("%d",tank->temperature));
    		xmlSetProp(tank_node, (guchar*)"WaterLevel", (guchar*)g_strdup_printf("%d",tank->waterlevel));
    		xmlSetProp(tank_node, (guchar*)"Online", (guchar*)g_strdup_printf("%d",tank->online));
   			xmlAddChild(node,tank_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices_fuelling_point_grade_option_tank_connections(xmlNodePtr root_node, PSSGradeOption* grade_option)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"TankConnections");

	if (grade_option->tank_connection_count > 0)
	{
		for (guint8 i = 0; i < grade_option->tank_connection_count; i++)
		{
			PSSTankConnection* tank_connection = &grade_option->tank_connections[i];

			xmlNodePtr tank_conneciton_node = xmlNewNode(NULL, BAD_CAST "Part");
			xmlSetProp(tank_conneciton_node, (guchar*)"TankID", (guchar*)g_strdup_printf("%d",tank_connection->tank_id));

			xmlNodePtr part_val = xmlNewText((guchar*)g_strdup_printf("%d", tank_connection->part));

			xmlAddChild(tank_conneciton_node, part_val);

			xmlAddChild(node,tank_conneciton_node);
		}
	}
	xmlAddChild(root_node,node);

}

void add_devices_fuelling_point_valid_grades(xmlNodePtr root_node, PSSPresetData* data)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"ValidGrades");

	if (data->valid_grades_count > 0)
	{
		for (guint8 i = 0; i < data->valid_grades_count; i++)
		{
			add_uint32_node(node, "GradeID", data->valid_grades[i]);

		}
	}

	xmlAddChild(root_node,node);

}


void add_devices_fuelling_point_preset_data(xmlNodePtr root_node, PSSPresetData* data)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"PresetData");

	add_uint32_node(node, "FmgID", data->fmg_id);
	add_uint32_node(node, "PgID", data->pg_id);
	add_uint32_node(node, "PosID", data->pos_id);
	add_uint32_node(node, "PresetType", data->preset_type);
	add_uint32_node(node, "GradeID", data->grade_id);
	add_uint32_node(node, "PresetValue", data->preset_value);
	add_uint32_node(node, "SmID", data->sm_id);

	add_devices_fuelling_point_valid_grades(node, data);

	xmlAddChild(root_node,node);

}

void add_devices_fuelling_point_grade_options(xmlNodePtr root_node, PSSFuellingPoint* fp)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"GradeOptions");

	if (fp->grade_option_count > 0)
	{
		for (guint8 i = 0; i < fp->grade_option_count; i++)
		{
			PSSGradeOption* grade_option = &fp->grade_options[i];
			xmlNodePtr grade_option_node = xmlNewNode(NULL, BAD_CAST "GradeOption");
			xmlSetProp(grade_option_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",grade_option->id));

			add_uint32_node(grade_option_node, "GradeID", grade_option->grade_id);
    		add_uint32_node(grade_option_node, "VolumeTotal", grade_option->volume_total);
    		add_uint32_node(grade_option_node, "MoneyTotal", grade_option->money_total);

			add_ascii_node(grade_option_node, "NozzleID", grade_option->nozzle_id);
			add_uint32_node(grade_option_node, "NozzleTagReaderID", grade_option->nozzle_tag_reader_id);
			add_devices_fuelling_point_grade_option_tank_connections(grade_option_node, grade_option);

			xmlAddChild(node,grade_option_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_devices_fuelling_point_transactions(xmlNodePtr root_node, PSSTransactions* trs, const gchar* name)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST name);

	add_uint32_node(node, "LastTransSeqNo", trs->last_trans_seq_no);

	if (trs->count > 0)
	{
		for (guint8 i = 0; i < trs->count; i++)
		{
			PSSTransaction* transaction = &trs->units[i];
			xmlNodePtr transaction_node = xmlNewNode(NULL, BAD_CAST "Transaction");

			add_uint32_node(transaction_node, "TransSeqNo", transaction->trans_seq_no);
			add_uint32_node(transaction_node, "ServiceModeID", transaction->sm_id);
			add_uint32_node(transaction_node, "TransLockID", transaction->trans_lock_id);
			add_uint32_node(transaction_node, "TransInfoFlags", transaction->flags);
			add_uint32_node(transaction_node, "Money", transaction->money);
			add_uint32_node(transaction_node, "Volume", transaction->volume);
			add_uint32_node(transaction_node, "GradeID", transaction->grade_id);

			xmlAddChild(node,transaction_node);
		}
	}
	xmlAddChild(root_node,node);


}

void add_devices_fuelling_point_pump_interface_decimal_positions(xmlNodePtr root_node, PSSFuellingPoint* fp)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"PumpInterfaceDecimalPositions");

	add_uint32_node(node, "InPrice", fp->pump_interface_decimal_positions.in_price);
	add_uint32_node(node, "InVolume", fp->pump_interface_decimal_positions.in_volume);
	add_uint32_node(node, "InMoney", fp->pump_interface_decimal_positions.in_money);
	add_uint32_node(node, "InVolumeTotal", fp->pump_interface_decimal_positions.in_volume_total);
	add_uint32_node(node, "InMoneyTotal", fp->pump_interface_decimal_positions.in_money_total);

	xmlAddChild(root_node,node);
}

void add_devices_fuelling_point_operation_modes(xmlNodePtr root_node, PSSFuellingPoint* fp)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"OperationModes");

	if (fp->operation_mode_count > 0)
	{
		for (guint8 i = 0; i < fp->operation_mode_count; i++)
		{
			PSSOperationMode* operation_mode = &fp->operation_modes[i];
			xmlNodePtr operation_mode_node = xmlNewNode(NULL, BAD_CAST "OperationMode");
			xmlSetProp(operation_mode_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",operation_mode->id));

			if (operation_mode->service_mode_count > 0)
			{
				for (guint8 j = 0; j < operation_mode->service_mode_count; j++)
				{
					xmlNodePtr service_mode_node = xmlNewNode(NULL, BAD_CAST "FPServiceMode");

					add_uint32_node(service_mode_node, "ServiceModeID", operation_mode->service_modes[j].service_mode_id);
					add_uint32_node(service_mode_node, "FuellingModeGroupID", operation_mode->service_modes[j].fuelling_mode_group_id);
					add_uint32_node(service_mode_node, "PriceGroupID", operation_mode->service_modes[j].price_group_id);

					xmlAddChild(operation_mode_node,service_mode_node);
				}
			}
			xmlAddChild(node,operation_mode_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_devices_fuelling_point_limits(xmlNodePtr root_node, PSSFuellingPoint* fp)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"Limits");

	add_uint32_node(node, "FloorLimitMargin", fp->limits.floor_limit_margin);

	xmlAddChild(root_node,node);
}

void add_devices_fuelling_points(xmlNodePtr root_node, PSSFuellingPoints* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"FuellingPoints");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSFuellingPoint* fuelling_point = &data->units[i];
    		xmlNodePtr fp_node = xmlNewNode(NULL, BAD_CAST "FuellingPoint");
    		xmlSetProp(fp_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",fuelling_point->id));

    		add_uint32_node(fp_node, "PSSPortNo", fuelling_point->pss_port_no);
    		add_uint32_node(fp_node, "ServerDeviceID", fuelling_point->server_device_id);
    		add_binary_node(fp_node, "InterfaceTypeGeneral", fuelling_point->interface_type_general, 16);
    		add_binary_node(fp_node, "InterfaceTypeProtocol", fuelling_point->interface_type_protocol, 16);
    		add_uint32_node(fp_node, "PssExtProtocolId", fuelling_point->pss_ext_protocol_id);
    		add_uint32_node(fp_node, "PhysicalAddress", fuelling_point->phisical_address);
    		add_uint32_node(fp_node, "PhysicalAddressType", fuelling_point->phisical_address_type);
    		add_uint32_node(fp_node, "PhysicalAddressPort", fuelling_point->phisical_address_port);
    		add_uint32_node(fp_node, "DeviceSubAddress", fuelling_point->device_sub_address);
    		add_boolean_node(fp_node, "IsClose", fuelling_point->is_close);

    		add_uint32_node(fp_node, "SmID", fuelling_point->sm_id);
    		add_uint32_node(fp_node, "MainState", fuelling_point->main_state);
    		add_uint32_node(fp_node, "SubState", fuelling_point->sub_state);
    		add_uint32_node(fp_node, "SubState2", fuelling_point->sub_state2);
    		add_uint32_node(fp_node, "LockedID", fuelling_point->locked_id);
    		add_uint32_node(fp_node, "GradeID", fuelling_point->grade_id);
    		add_uint32_node(fp_node, "FuellingVolume", fuelling_point->fuelling_volume);
    		add_uint32_node(fp_node, "FuellingMoney", fuelling_point->fuelling_money);
    		add_ascii_node(fp_node, "AttendantAccauntID", fuelling_point->attendant_accaunt_id);


    		add_uint32_node(fp_node, "OperationModeNo", fuelling_point->operation_mode_no);

    		add_uint32_node(fp_node, "VolumeTotal", fuelling_point->volume_total);
    		add_uint32_node(fp_node, "MoneyTotal", fuelling_point->money_total);


    		add_devices_fuelling_point_grade_options(fp_node, fuelling_point);
    		add_devices_fuelling_point_pump_interface_decimal_positions(fp_node, fuelling_point);
    		add_devices_fuelling_point_operation_modes(fp_node, fuelling_point);
    		add_devices_fuelling_point_limits(fp_node, fuelling_point);
    		add_devices_fuelling_point_transactions(fp_node, &fuelling_point->sup_transactions, "SupTransactions");
    		add_devices_fuelling_point_transactions(fp_node, &fuelling_point->unsup_transactions, "UnsupTransactions");

    		add_devices_fuelling_point_preset_data(fp_node, &fuelling_point->preset_data);

    		add_ascii_node(fp_node, "PumpInterfaceConfigString", fuelling_point->config_string);
    		add_uint32_node(fp_node, "NominalNormalSpeed", fuelling_point->nominal_normal_speed);
    		add_uint32_node(fp_node, "NominalHighSpeed", fuelling_point->nominal_high_speed);
    		add_uint32_node(fp_node, "HighSpeedTriggerLevel", fuelling_point->high_speed_trigger_level);

   			xmlAddChild(node,fp_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices_tank_gauges(xmlNodePtr root_node, PSSTankGauges* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"TankGauges");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSTankGauge* tank_gauge = &data->units[i];

    		xmlNodePtr tank_gauge_node = xmlNewNode(NULL, BAD_CAST "TankGauge");
    		xmlSetProp(tank_gauge_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",tank_gauge->id));

    		add_uint32_node(tank_gauge_node, "PSSPortNo", tank_gauge->pss_port_no);
    		add_uint32_node(tank_gauge_node, "ServerDeviceID", tank_gauge->server_device_id);

    		add_binary_node(tank_gauge_node, "InterfaceType", tank_gauge->interface_type, 16);
    		add_uint32_node(tank_gauge_node, "PhysicalAddress", tank_gauge->phisical_address);

    		xmlNodePtr physical_sub_address_node = xmlNewNode(NULL, BAD_CAST "PhysicalSubAddress");
    		xmlSetProp(physical_sub_address_node, (guchar*)"Value", (guchar*)g_strdup_printf("%d",tank_gauge->phisical_sub_address));
    		xmlAddChild(tank_gauge_node, physical_sub_address_node);

    		xmlNodePtr tank_node = xmlNewNode(NULL, BAD_CAST "Tank");
    		xmlSetProp(tank_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",tank_gauge->tank_id));
    		xmlAddChild(tank_gauge_node, tank_node);

    		add_uint32_node(tank_gauge_node, "TankHeight", tank_gauge->tank_height);

    		add_uint32_node(tank_gauge_node, "MainState", tank_gauge->main_state);
    		add_uint32_node(tank_gauge_node, "SubState", tank_gauge->sub_state);


    		xmlNodePtr extra_install_data_node = xmlNewNode(NULL, BAD_CAST "ExtraInstallData");
    		xmlNodePtr alarms_node = xmlNewNode(NULL, BAD_CAST "Alarms");

    		xmlNodePtr height_alarms_node = xmlNewNode(NULL, BAD_CAST "HeightAlarms");
    		xmlNodePtr volume_alarms_node = xmlNewNode(NULL, BAD_CAST "VolumeAlarms");

    		xmlAddChild(alarms_node,height_alarms_node);
    		xmlAddChild(alarms_node,volume_alarms_node);

    		xmlAddChild(extra_install_data_node, alarms_node);
    		xmlAddChild(tank_gauge_node, extra_install_data_node);

   			xmlAddChild(node,tank_gauge_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices_tank_gauge_sub_devices(xmlNodePtr root_node, PSSTankGaugeSubDevices* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"TankGaugeSubDevices");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSTankGaugeSubDevice* tank_gauge_sub_device = &data->units[i];

    		xmlNodePtr tank_gauge_sub_device_node = xmlNewNode(NULL, BAD_CAST "TankGaugeSubDevice");
    		xmlSetProp(tank_gauge_sub_device_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",tank_gauge_sub_device->id));

    		add_binary_node(tank_gauge_sub_device_node, "InterfaceType", tank_gauge_sub_device->interface_type, 16);
    		add_uint32_node(tank_gauge_sub_device_node, "PSSChannelNo", tank_gauge_sub_device->pss_channel_no);
    		add_uint32_node(tank_gauge_sub_device_node, "PhysicalAddress", ( tank_gauge_sub_device->sub_address << 6 ) | tank_gauge_sub_device->main_address);
    		add_uint32_node(tank_gauge_sub_device_node, "PhysicalSubAddress", tank_gauge_sub_device->phisical_sub_address);

    		xmlNodePtr tank_gauge_sub_device_ids_node = xmlNewNode(NULL, BAD_CAST "SubDeviceIds");

			if (tank_gauge_sub_device->sub_dev_ids_count > 0)
			{
				for (guint8 j = 0; j < tank_gauge_sub_device->sub_dev_ids_count; j++)
				{
					xmlNodePtr sub_device_id_node = xmlNewNode(NULL, BAD_CAST "SubDeviceId");

					xmlSetProp(sub_device_id_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",tank_gauge_sub_device->sub_dev_ids[j]));

					xmlAddChild(tank_gauge_sub_device_ids_node,sub_device_id_node);
				}
			}
			xmlAddChild(tank_gauge_sub_device_node,tank_gauge_sub_device_ids_node);

   			xmlAddChild(node,tank_gauge_sub_device_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices_price_pole_price_position_options(xmlNodePtr root_node, PSSPricePole* pp)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"PricePositionOptions");

	if (pp->option_count > 0)
	{
		for (guint8 i = 0; i < pp->option_count; i++)
		{
			PSSPricePositionOption* price_position_option = &pp->options[i];
			xmlNodePtr price_position_option_node = xmlNewNode(NULL, BAD_CAST "PricePositionOption");
			xmlSetProp(price_position_option_node, (guchar*)"No", (guchar*)g_strdup_printf("%d",price_position_option->option_no));
			xmlSetProp(price_position_option_node, (guchar*)"FcPriceGroupId", (guchar*)g_strdup_printf("%d",price_position_option->fc_price_group_id));
			xmlSetProp(price_position_option_node, (guchar*)"FcGradeId", (guchar*)g_strdup_printf("%d",price_position_option->fc_grade_id));

			xmlAddChild(node,price_position_option_node);
		}
	}
	xmlAddChild(root_node,node);
}

void add_devices_price_poles(xmlNodePtr root_node, PSSPricePoles* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"PricePoles");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSPricePole* pp = &data->units[i];

    		xmlNodePtr pp_node = xmlNewNode(NULL, BAD_CAST "PricePole");
    		xmlSetProp(pp_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",pp->id));

    		add_uint32_node(pp_node, "PSSPortNo", pp->pss_port_no);
    		add_uint32_node(pp_node, "ServerDeviceID", pp->server_device_id);
    		add_binary_node(pp_node, "InterfaceType", pp->interface_type, 16);
    		add_uint32_node(pp_node, "PhysicalAddress", pp->phisical_address);
    		add_uint32_node(pp_node, "MainState", pp->main_state);
    		add_uint32_node(pp_node, "SubState", pp->sub_state);
    		add_uint32_node(pp_node, "NoPricePositionOption", pp->option_count);

    		add_devices_price_pole_price_position_options(pp_node, pp);

   			xmlAddChild(node,pp_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices_payment_terminals(xmlNodePtr root_node, PSSPaymentTerminals* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"PaymentTerminals");

    if (data->count > 0)
    {
    	for (guint8 i = 0; i < data->count; i++)
    	{
    		PSSPaymentTerminal* pt = &data->units[i];

    		xmlNodePtr pt_node = xmlNewNode(NULL, BAD_CAST "PaymentTerminal");
    		xmlSetProp(pt_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",pt->id));

    		add_uint32_node(pt_node, "PSSPortNo", pt->pss_port_no);
    		add_uint32_node(pt_node, "ServerDeviceID", pt->server_device_id);
    		add_binary_node(pt_node, "InterfaceType", pt->interface_type, 16);
    		add_uint32_node(pt_node, "PhysicalAddress", pt->phisical_address);

    		xmlNodePtr select_options_node = xmlNewNode(NULL, BAD_CAST "SelectOptions");

    		if (pt->select_option_count > 0)
    		{
    			for (guint8 j = 0; j < pt->select_option_count; j++)
    			{
    				xmlNodePtr select_option_node = xmlNewNode(NULL, BAD_CAST "SelectOption");
    				xmlSetProp(select_option_node, (guchar*)"ID", (guchar*)g_strdup_printf("%d",pt->select_options[j].id));

    				xmlAddChild(select_options_node, select_option_node);
    			}
    		}

    		xmlAddChild(pt_node,select_options_node);
   			xmlAddChild(node,pt_node);
    	}
    }
    xmlAddChild(root_node,node);
}

void add_devices(xmlNodePtr root_node, PSSDevices* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"Devices");

  //  add_devices_server_devices(node, &data->server_devices);
    add_devices_tanks(node, &data->tanks);
    add_devices_fuelling_points(node, &data->fuelling_points);
    add_devices_tank_gauges(node, &data->tank_gauges);
    add_devices_tank_gauge_sub_devices(node, &data->tank_gauge_sub_devices);
    add_devices_price_poles(node, &data->price_poles);
    add_devices_payment_terminals(node, &data->payment_terminals);

    xmlAddChild(root_node,node);
}

void add_fc_status(xmlNodePtr root_node, PSSFcStatus* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"FcStatus");

    add_uint32_node(node, "FcStatus1Flags", data->fc_status_1_flags);
    add_uint32_node(node, "FcStatus2Flags", data->fc_status_2_flags);
    add_uint32_node(node, "FcServiceMsgSeqNo", data->fc_service_msg_seq_no);
    add_uint32_node(node, "FcMasterResetDateAndTime", data->fc_master_reset_date_and_time);
    add_uint32_node(node, "FcMasterResetCode", data->fc_master_reset_code);
    add_uint32_node(node, "FcResetDateAndTime", data->fc_reset_date_and_time);
    add_uint32_node(node, "FcResetCode", data->fc_reset_code);
    add_uint32_node(node, "FcPriceSetId", data->fc_price_set_id);
    add_uint32_node(node, "FcPriceSetDateAndTime", data->fc_price_set_date_and_time);
    add_uint32_node(node, "FcOperationModeNo", data->fc_operation_mode_no);
    add_uint32_node(node, "FcOperationModeDateAndTime", data->fc_operation_mode_date_and_time);

    xmlAddChild(root_node,node);
}

void add_common(xmlNodePtr root_node, PSSCommon* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"Common");

//	xmlSetProp(node, (guchar*)"LogDir", (guchar*)g_strdup_printf("%s",data->log_dir));
//	xmlSetProp(node, (guchar*)"LogEnable", (guchar*)g_strdup_printf("%d",data->log_enable));
//	xmlSetProp(node, (guchar*)"LogTrace", (guchar*)g_strdup_printf("%d",data->log_trace));

    add_fc_status(node, &data->fc_status);

    xmlAddChild(root_node,node);
}

gboolean save_pss_xml_data(const gchar* filename, PSSData* data)
{
	fprintf(stderr,"Save PSS configuration: %s\n", filename);

    xmlDocPtr doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
    if (doc == NULL)
    {
        printf("Error creating the xml document\n");
        return FALSE;
    }



    xmlNodePtr root_node = xmlNewDocNode(doc, NULL, BAD_CAST "PSS_Config", NULL);
    if (root_node == NULL)
    {
        printf("Error creating the xml root node\n");
        return FALSE;
    }

    xmlDocSetRootElement(doc, root_node);

    add_common(root_node, &data->common);
    add_general_configuration(root_node, &data->general_configuration);
    add_devices(root_node, &data->devices);

    //xmlSaveFile(filename,doc);
    xmlSaveFormatFile(filename, doc, 1);

    xmlFreeDoc(doc);
	return TRUE;
}
