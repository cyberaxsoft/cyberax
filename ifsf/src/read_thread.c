#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "ifsf.h"
#include "ifsf_func.h"
#include "config.h"
#include "driver_state.h"

gboolean 		read_thread_terminating = FALSE;
gboolean 		read_thread_terminated = FALSE;
GMutex			read_thread_mutex;

void init_read_thread_mutex()
{
	if (read_thread_mutex.p == NULL)
	{
		g_mutex_init(&read_thread_mutex);
	}
}

gboolean safe_get_read_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminating;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}

gboolean safe_get_read_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&read_thread_mutex);

	result = read_thread_terminated;

	g_mutex_unlock(&read_thread_mutex);

	return result;

}

void safe_set_read_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminating = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void safe_set_read_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&read_thread_mutex);

	read_thread_terminated = new_value;

	g_mutex_unlock(&read_thread_mutex);
}

void read_db_address(guint8* buffer, guint8 length, IFSFDbAddress* database_address)
{
	if (length > 0)
	{
		database_address->main_address = buffer[0];

		database_address->error_identifier = 0;
		database_address->fuelling_mode_identifier = 0;
		database_address->fuelling_point_detal = 0;
		database_address->product_number = 0;
		database_address->transaction_sequence_number = 0;
		database_address->logial_nozzle_identifier = 0;

		switch(length)
		{
			case 1:
				break;

			case 2:
				if (buffer[1] >= 0x11 &&  buffer[1] <=  0x18 && database_address->main_address >= ifsfdbid_FuellingPointId1 && database_address->main_address <= ifsfdbid_FuellingPointId4)
				{
					database_address->fuelling_point_detal = buffer[1];
					database_address->logial_nozzle_identifier = buffer[1] & 0x0F;
				}
				break;

			case 3:
				if (buffer[1] == 0x41)
				{
					database_address->fuelling_point_detal = buffer[1];
					database_address->error_identifier = buffer[2];
				}
				break;

			case 4:
				if (buffer[1] == 0x21)
				{
					database_address->fuelling_point_detal = buffer[1];
					database_address->transaction_sequence_number = (guint16)packed_bcd_to_bin(&buffer[2], 2);
				}
				break;

			case 6:
				if (database_address->main_address == ifsfdbid_ProductData)
				{
					database_address->product_number = packed_bcd_to_bin(&buffer[1], 4);
					database_address->fuelling_mode_identifier = buffer[5] & 0x0F;
				}
				break;

		}
	}
}

gboolean parse_ifsf_header(guint8* buffer, guint8 length, IFSFNodeData* node_data, guint8* data_pos)
{
	gboolean result = FALSE;

	IFSFParseIfsfStage ps = ifsfpis_ReadTransportMC;

	guint8 pos = 0;

	for (guint8 i = 0; i < length; i++)
	{
		switch(ps)
		{
			case ifsfpis_ReadTransportMC:
				pos++;
				ps = ifsfpis_ReadDestSubnet;
				break;

			case ifsfpis_ReadDestSubnet:
				node_data->dest_subnet = buffer[i];
				pos++;
				ps = ifsfpis_ReadDestNode;
				break;

			case ifsfpis_ReadDestNode:
				node_data->dest_node = buffer[i];
				pos++;
				ps = ifsfpis_ReadSrcSubnet;
				break;

			case ifsfpis_ReadSrcSubnet:
				node_data->src_subnet = buffer[i];
				pos++;
				ps = ifsfpis_ReadSrcNode;
				break;

			case ifsfpis_ReadSrcNode:
				node_data->src_node = buffer[i];
				pos++;
				ps = ifsfpis_ReadMC;
				break;

			case ifsfpis_ReadMC:
				node_data->message_code = buffer[i];
				pos++;
				ps = ifsfpis_ReadBL;
				break;

			case ifsfpis_ReadBL:
				node_data->bl = buffer[i];
				pos++;
				if ((buffer[i] & 0x1F) > 0)
				{
					*data_pos = pos;
					return result;
				}
				else
				{
					ps = ifsfpis_ReadStatus;
				}
				break;

			case ifsfpis_ReadStatus:
				node_data->message_type = (IFSFMessageType)((buffer[i] & 0xe0) >> 5);
				node_data->token = buffer[i] & 0x1F;
				pos++;
				ps = ifsfpis_ReadLen1;
				break;

			case ifsfpis_ReadLen1:
				node_data->message_length = buffer[i];
				pos++;
				ps = ifsfpis_ReadLen2;
				break;

			case ifsfpis_ReadLen2:
				node_data->message_length = (node_data->message_length << 8 ) |  buffer[i];
				pos++;
				ps = ifsfpis_ReadLenDb;
				break;

			case ifsfpis_ReadLenDb:
				node_data->address_length = buffer[i];
				pos++;
				ps = ifsfpis_ReadDb;
				break;

			case ifsfpis_ReadDb:
				read_db_address(&buffer[i], node_data->address_length, &node_data->database_address);
				pos+=node_data->address_length;
				*data_pos = pos;
				result = TRUE;
				return result;
		}
	}
	return result;
}

void add_field_to_log(IFSFDbAddress database_address, guint8 id, guint16 length, guint8* data, LogOptions log_options)
{
	switch(ifsf_return_data_type(database_address, id))
	{
		case ifsfdt_Undefined:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s: undefined type", ifsf_data_id_to_str(database_address, id));
			break;

		case ifsfdt_Binary:
			switch(length)
			{
				case 1:
					add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s: 0x%02x", ifsf_data_id_to_str(database_address, id), data[0]);
					break;

				case 2:
					add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s: 0x%02x 0x%02x ", ifsf_data_id_to_str(database_address, id), data[0], data[1]);
					break;

				case 3:
					add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s: 0x%02x 0x%02x 0x%02x ", ifsf_data_id_to_str(database_address, id), data[0], data[1], data[2]);
					break;

				case 4:
					add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s: 0x%02x 0x%02x 0x%02x 0x%02x ", ifsf_data_id_to_str(database_address, id), data[0], data[1], data[2], data[3]);
					break;

			}
			break;

		case ifsfdt_Bcd:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[0], length));
			break;


		case ifsfdt_AddrTable:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s:", ifsf_data_id_to_str(database_address, id));
			for (guint8 i = 0; i < 128; i+=2)
			{
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "					%02x %02x ", data[i], data[i+1]);
			}
			break;

		case ifsfdt_Ascii:
			{
				gchar* str = malloc(length + 1);
				memset(str, 0x00, length + 1);
				memcpy(str, data, length);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %s ", ifsf_data_id_to_str(database_address, id),length, str);
				g_free(str);

			}
			break;

		case ifsfdt_Volume:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[1], length - 1));

			break;

		case ifsfdt_Amount:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[1], length - 1));
			break;

		case ifsfdt_UnitPrice:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[1], length - 1));
			break;

		case ifsfdt_Temp:

			break;

		case ifsfdt_Date:

			break;

		case ifsfdt_LongVolume:

			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[1], length - 1));
			break;

		case ifsfdt_LongAmount:
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				%s (%d): %d ", ifsf_data_id_to_str(database_address, id),length, packed_bcd_to_bin(&data[1], length - 1));
			break;

		case ifsfdt_LongNumber:

			break;

	}


}

void parse_ifsf_read_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{
	if (data_length > 0)
	{
		for (guint i = 0; i < data_length; i++)
		{
			add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				ID: %02x", data[i]);
		}
	}

	guint buffer_length = 0;
	guint8 buffer[UART_BUFFER_WRITE_SIZE] = {0x00};

	switch (database_address.main_address)
	{
		case ifsfdbid_CommunicationService:
			buffer_length = prepare_communication_service_reply_message(buffer, data, data_length, src_subnet, src_node);
			break;
	}

	if (buffer_length > 0)
	{
		if (send_func(buffer, buffer_length, log_options))
		{
			//TODO
		}
	}
}

void interpret_fp_state(guint8 disp_index, guint8 state)
{
	safe_set_original_state(disp_index, (IFSF_FPState)state);

}

void interpret_original_fp_state(guint8 disp_index)
{
	switch(safe_get_original_state(disp_index))
	{
		case ifsffps_Unknown:
		case ifsffps_Inoperative:
			safe_set_dispencer_state(disp_index, ds_NotInitialize);
			break;

		case ifsffps_Closed:
			safe_set_dispencer_state(disp_index, ds_Stopped);
			break;

		case ifsffps_Idle:
			if (safe_get_transaction_num(disp_index) > 0 && safe_get_preset_order_type(disp_index) > ot_Free)
			{
				safe_set_dispencer_state(disp_index, ds_Finish);
			}
			else
			{
				safe_set_dispencer_state(disp_index, ds_Free);
			}
			break;

		case ifsffps_Calling:
			safe_set_dispencer_state(disp_index, ds_NozzleOut);
			break;

		case ifsffps_Authorized:
		case ifsffps_Started:
		case ifsffps_Fuelling:
			safe_set_dispencer_state(disp_index, ds_Filling);
			break;

		case ifsffps_SuspendedStarted:
		case ifsffps_SuspendedFuelling:
			safe_set_dispencer_state(disp_index, ds_Stopped);
			break;

	}
}

void interpret_nozzle_state(guint8 disp_index, guint8 data)
{
	if (data == 0)
	{
		safe_set_active_nozzle_index(disp_index, -1);
	}
	else
	{
		if ((data & 0x01) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 0);
		}
		else if ((data & 0x02) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 1);
		}
		else if ((data & 0x04) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 2);
		}
		else if ((data & 0x08) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 3);
		}
		else if ((data & 0x10) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 4);
		}
		else if ((data & 0x20) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 5);
		}
		else if ((data & 0x40) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 6);
		}
		else if ((data & 0x80) > 0)
		{
			safe_set_active_nozzle_index(disp_index, 7);
		}

	}
}



void interpret_data_field(guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address, guint8 id, guint16 length, guint8* data,
		guint8 token, LogOptions log_options)
{
	add_field_to_log(database_address, id, length, data, log_options);

	if (database_address.main_address >= ifsfdbid_FuellingPointId1 && database_address.main_address <= ifsfdbid_FuellingPointId4)
	{

		guint disp_addr =  (src_node << 4) | (database_address.main_address & 0x0F);

		guint8 disp_index = 0;
		if (safe_get_disp_index_by_addr(disp_addr, &disp_index) == de_NoError)
		{
			if (database_address.logial_nozzle_identifier == 0 && database_address.error_identifier == 0 && database_address.transaction_sequence_number == 0)
			{
				switch(id)
				{
					case ifsffpid_NbLogicalNozzle:
						if (data[0] != safe_get_nozzle_count(disp_index))
						{
							add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "FATAL ERROR DISPENCER INDEX %d (config nozzle_count = %d, real nozzle count = %d) ",
										disp_index, safe_get_nozzle_count(disp_index), data[0]);
							safe_set_node_stage(src_node - 1, ifsfns_Offline);
						}

						break;

					case ifsffpid_FpState:
						interpret_fp_state(disp_index, data[0]);
						break;

					case ifsffpid_LogNozState:
						interpret_nozzle_state(disp_index, data[0]);
						break;

					case ifsffpid_CurrentAmount:
						safe_set_current_amount(disp_index, packed_bcd_to_bin(&data[1], length - 1));
						break;

					case ifsffpid_CurrentVolume:
						safe_set_current_volume(disp_index, packed_bcd_to_bin(&data[1], length - 1));
						break;

					case ifsffpid_CurrentUnitPrice:
						safe_set_current_price(disp_index, packed_bcd_to_bin(&data[1], length - 1));
						break;
				}
			}
			else if (database_address.logial_nozzle_identifier != 0 && database_address.fuelling_point_detal>= 0x11 && database_address.fuelling_point_detal<=0x18 )
			{
				switch(id)
				{
					case ifsflnid_PRId:
						safe_set_nozzle_product_id(disp_index, (database_address.fuelling_point_detal & 0x0F) - 1, data[0]);
						break;

					case ifsflnid_LogNozVolTotal:  //
						safe_set_nozzle_counter(disp_index, (database_address.fuelling_point_detal & 0x0F) - 1, packed_bcd_to_bin(&data[1], length - 1));
						break;
				}
			}
			else if (database_address.transaction_sequence_number != 0 && database_address.fuelling_point_detal== 0x21)
			{
				switch(id)
				{
					case ifsfftid_TrSeqNb:
						safe_set_transaction_num(disp_index, packed_bcd_to_bin(&data[0], length));
						break;

					case ifsfftid_TrAmount:
						break;

					case ifsfftid_TrVolume:
						break;

					case ifsfftid_TrUnitPrice:
						break;

					case ifsfftid_TrLogNoz:
						break;

				}
			}
			else if (database_address.error_identifier != 0 && database_address.fuelling_point_detal== 0x41)
			{
				//TODO
			}
		}
	}
	else if (database_address.main_address >= ifsfdbid_Product1 && database_address.main_address <= ifsfdbid_Product8)
	{
		switch(id)
		{
			case ifsfpid_ProdNb:
				break;
		}

	}
	else if (database_address.main_address == ifsfdbid_Calculator)
	{
		switch(id)
		{
			case ifsfcid_NbProducts:
				safe_set_product_count(src_node - 1, data[0]);
				break;

			case ifsfcid_NbFuellingModes:
				safe_set_fuelling_mode_count(src_node - 1, 0x02);
				break;

			case ifsfcid_NbFp:
				safe_set_fuelling_point_count(src_node - 1, data[0]);
				break;

		}
	}


}

void interpret_write_field(guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address, guint8 id, guint16 length, guint8* data,
		guint8 token, LogOptions log_options)
{
	add_field_to_log(database_address, id, length, data, log_options);
}


void parse_ifsf_answer_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{
	if (data_length > 0)
	{
		guint8 pos = 0;

		IFSFParseMessageStage pms = ifsfpms_ReadId;

		guint8 id = 0;
		guint16 len = 0;

		while (pos < data_length)
		{
			switch(pms)
			{
				case ifsfpms_ReadId:
					id = data[pos++];
					pms = ifsfpms_ReadLen;
					break;

				case ifsfpms_ReadLen:
					len = data[pos++];
					if (len < 0xFF)
					{
						pms = ifsfpms_ReadData;
					}
					else
					{
						pms = ifsfpms_ReadLenEx1;
					}
					break;

				case ifsfpms_ReadLenEx1:
					len = data[pos++];
					pms = ifsfpms_ReadLenEx2;
					break;

				case ifsfpms_ReadLenEx2:
					len = len << 8 | data[pos++];
					pms = ifsfpms_ReadData;
					break;

				case ifsfpms_ReadData:
					interpret_data_field(dest_subnet, dest_node, src_subnet, src_node, database_address, id, len, &data[pos], token, log_options);
					pos+=len;
					pms = ifsfpms_ReadId;
					break;
			}
		}
	}
}

void parse_ifsf_write_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{
	if (data_length > 0)
	{
		guint8 pos = 0;

		IFSFParseMessageStage pms = ifsfpms_ReadId;

		guint8 id = 0;
		guint16 len = 0;

		while (pos < data_length)
		{
			switch(pms)
			{
				case ifsfpms_ReadId:
					id = data[pos++];
					pms = ifsfpms_ReadLen;
					break;

				case ifsfpms_ReadLen:
					len = data[pos++];
					if (len < 0xFF)
					{
						pms = ifsfpms_ReadData;
					}
					else
					{
						pms = ifsfpms_ReadLenEx1;
					}
					break;

				case ifsfpms_ReadLenEx1:
					len = data[pos++];
					pms = ifsfpms_ReadLenEx2;
					break;

				case ifsfpms_ReadLenEx2:
					len = len << 8 | data[pos++];
					pms = ifsfpms_ReadData;
					break;

				case ifsfpms_ReadData:
					interpret_write_field(dest_subnet, dest_node, src_subnet, src_node, database_address, id, len, &data[pos], token, log_options);
					pos+=len;
					pms = ifsfpms_ReadId;
					break;
			}
		}
	}
}

void parse_ifsf_unsolicited_with_ack_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{
	if (data_length > 0)
	{
		guint8 pos = 0;

		IFSFParseMessageStage pms = ifsfpms_ReadId;

		guint8 id = 0;
		guint16 len = 0;

		while (pos < data_length)
		{
			switch(pms)
			{
				case ifsfpms_ReadId:
					id = data[pos++];
					pms = ifsfpms_ReadLen;
					break;

				case ifsfpms_ReadLen:
					len = data[pos++];
					if (len < 0xFF)
					{
						pms = ifsfpms_ReadData;
					}
					else
					{
						pms = ifsfpms_ReadLenEx1;
					}
					break;

				case ifsfpms_ReadLenEx1:
					len = data[pos++];
					pms = ifsfpms_ReadLenEx2;
					break;

				case ifsfpms_ReadLenEx2:
					len = len << 8 | data[pos++];
					pms = ifsfpms_ReadData;
					break;

				case ifsfpms_ReadData:
					interpret_data_field(dest_subnet, dest_node, src_subnet, src_node, database_address, id, len, &data[pos], token, log_options);
					pos+=len;
					pms = ifsfpms_ReadId;
					break;
			}
		}
	}
}

void parse_ifsf_unsolicited_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{
	if (data_length > 0)
	{
		guint8 pos = 0;

		IFSFParseMessageStage pms = ifsfpms_ReadId;

		guint8 id = 0;
		guint16 len = 0;

		while (pos < data_length)
		{
			switch(pms)
			{
				case ifsfpms_ReadId:
					id = data[pos++];
					pms = ifsfpms_ReadLen;
					break;

				case ifsfpms_ReadLen:
					len = data[pos++];
					if (len < 0xFF)
					{
						pms = ifsfpms_ReadData;
					}
					else
					{
						pms = ifsfpms_ReadLenEx1;
					}
					break;

				case ifsfpms_ReadLenEx1:
					len = data[pos++];
					pms = ifsfpms_ReadLenEx2;
					break;

				case ifsfpms_ReadLenEx2:
					len = len << 8 | data[pos++];
					pms = ifsfpms_ReadData;
					break;

				case ifsfpms_ReadData:
					interpret_data_field(dest_subnet, dest_node, src_subnet, src_node, database_address, id, len, &data[pos], token, log_options);
					pos+=len;
					pms = ifsfpms_ReadId;
					break;
			}
		}
	}
}

void parse_ifsf_ack_message(guint8* data, guint8 data_length, guint8 dest_subnet, guint8 dest_node, guint8 src_subnet, guint8 src_node, IFSFDbAddress database_address,
									guint8 token, LogOptions log_options)
{

	if (data_length > 0)
	{
		IFSFMessageAcknowledgeStatus status = (IFSFMessageAcknowledgeStatus)data[0];

		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Message status: %02x (%s)", status, ifsf_message_status_to_str(status));

		if (status != ifsfmas_DataNotAcceptable  && data_length > 1)
		{
			for (guint8 i = 1; i < data_length; i+=2)
			{
				guint8 id = data[i];
				IFSFDataAcknowledgeStatus status = (IFSFDataAcknowledgeStatus)data[i+1];

				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "					Id %02x: %02x (%s)",
							id, status, ifsf_data_status_to_str(status));

			}
		}

	}


}

void parse_ifsf_message(IFSFNodeData* node_data, LogOptions log_options)
{

	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " IFSF PACK { ");

	for (guint16 i = 0; i < node_data->buffer_length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", node_data->buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, " }");


	guint8* data =  &node_data->buffer[IFSF_DATA_OFFSET + node_data->address_length];
	guint8 data_length = node_data->buffer_length - (IFSF_DATA_OFFSET + node_data->address_length);

	add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			IFSF data (%d):", data_length);

	switch (node_data->message_type)
	{
		case ifsfmt_Read:
			parse_ifsf_read_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

		case ifsfmt_Answer:
			parse_ifsf_answer_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

		case ifsfmt_Write:
			parse_ifsf_write_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

		case ifsfmt_UnsWAck:
			parse_ifsf_unsolicited_with_ack_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

		case ifsfmt_Uns:
			parse_ifsf_unsolicited_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

		case ifsfmt_Ack:
			parse_ifsf_ack_message(data, data_length, node_data->dest_subnet, node_data->dest_node, node_data->src_subnet, node_data->src_node, node_data->database_address, node_data->token, log_options);
			break;

	}
}


gboolean  parse_ifsf_frame(guint8* buffer, IFSFNodeData* node_data, guint8 length, LogOptions log_options)
{
	gboolean result = FALSE;

	guint8			data_pos = 0;

	add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			Frame length : %d", length);

	if( parse_ifsf_header(buffer, length, node_data, &data_pos))
	{
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			IFSF header:");
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest subnet: %02x", node_data->dest_subnet);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest node: %02x", node_data->dest_node);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source subnet: %02x", node_data->src_subnet);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source node: %02x", node_data->src_node);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Message code: %02x", node_data->message_code);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				BL: %02x", node_data->bl);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Message type: %d (%s)", (guint8)node_data->message_type, ifsf_message_type_to_str(node_data->message_type) );
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Token: %d", node_data->token);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Message length: %d", node_data->message_length);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Database address=%02x, detal=%02X, nozzle=%d, tr_seq=%d, error=%d, prod_num=%d, fmode=%d", node_data->database_address.main_address,
				node_data->database_address.fuelling_point_detal,	node_data->database_address.logial_nozzle_identifier, node_data->database_address.transaction_sequence_number, node_data->database_address.error_identifier,
				node_data->database_address.product_number, node_data->database_address.fuelling_mode_identifier);

		memcpy(node_data->buffer, &buffer[1], length - 1);
		node_data->buffer_length = length - 1;

	}
	else
	{
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			IFSF header:");
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest subnet: %02x", node_data->dest_subnet);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest node: %02x", node_data->dest_node);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source subnet: %02x", node_data->src_subnet);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source node: %02x", node_data->src_node);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Message code: %02x", node_data->message_code);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				BL: %02x", node_data->bl);

		memcpy(&node_data->buffer[node_data->buffer_length], &buffer[data_pos], length - data_pos);
		node_data->buffer_length += length - data_pos;

	}

	if ((node_data->bl & 0x80) > 0)
	{
		result = TRUE;
	}

	return result;

}

void parse_transport_frame(guint8* buffer, guint8 length, LogOptions log_options, guint8* flags, guint8* format, guint8* src_subnet, guint8* src_node, guint8* dest_subnet, guint8* dest_node, guint8* token)
{

	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");

	LonFrame* lon_frame = (LonFrame*)buffer;

	if ( (length - 4) >= (sizeof(LonTransportFrame) - sizeof(guint8*)) )
	{
		LonTransportFrame* transport_frame = (LonTransportFrame*)&buffer[2];

		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "Parsing command %02x (length = %d):", lon_frame->command, lon_frame->length);

		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Flags: %02x", transport_frame->flags);
		add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "	Format: %02x", transport_frame->format);

		*flags = transport_frame->flags;
		*format = transport_frame->format;

		switch (transport_frame->format)
		{
			case IFSF_DATA_FORMAT:
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "		Data frame:");
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			Transport header:");
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source subnet: %02x", transport_frame->src_subnet);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Source node: %02x", transport_frame->src_node);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest subnet: %02x", transport_frame->dest_subnet);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Dest node: %02x", transport_frame->dest_node);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Group: %02x", transport_frame->group);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "				Token: %02x", transport_frame->token);

				*src_subnet = transport_frame->src_subnet;
				*src_node = transport_frame->src_node;
				*dest_subnet = transport_frame->dest_subnet;
				*dest_node = transport_frame->dest_node;
				*token = transport_frame->token;

				break;

			case IFSF_HEARTBEAT_FORMAT:
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "		Heartbeat frame:");
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			Source subnet: %02x", transport_frame->src_subnet);
				add_log(TRUE, TRUE, log_options.parsing, log_options.frames, "			Source node: %02x", transport_frame->src_node);
				*src_subnet = transport_frame->src_subnet;
				*src_node = transport_frame->src_node;
				break;
		}
	}
}

void adjust_node_stage(guint8 node_index, LogOptions log_options)
{
	switch (safe_get_node_stage(node_index))
	{
		case ifsfns_Offline:
			break;

		case ifsfns_ConnectReady:
			safe_set_node_stage(node_index, ifsfns_TimeoutReady);
			break;

		case ifsfns_TimeoutReady:
			safe_set_node_stage(node_index, ifsfns_CalculatorConfReady);
			break;

		case ifsfns_CalculatorConfReady:
			safe_set_node_stage(node_index, ifsfns_ConfigReady);
			break;

		case ifsfns_ConfigReady:
			safe_set_config_product_index(node_index,0);

			if (safe_get_product_count(node_index) > 0)
			{
				safe_set_node_stage(node_index, ifsfns_LoadProduct);
			}
			else
			{
				add_log(TRUE, TRUE,  log_options.trace, log_options.system, "Error! Dispencer product not found!");
				safe_set_node_stage(node_index, ifsfns_Offline);
			}
			break;

		case ifsfns_LoadProduct:
			safe_increment_config_product_index(node_index);

			if (safe_get_config_product_index(node_index) >= safe_get_product_count(node_index))
			{
				safe_set_config_product_index(node_index,0);
				safe_set_config_fuelling_mode_index(node_index,0);
				safe_set_node_stage(node_index, ifsfns_SetProductParams);
			}

			break;

		case ifsfns_SetProductParams:
			safe_increment_config_product_index(node_index);

			if (safe_get_config_product_index(node_index) >= safe_get_product_count(node_index))
			{
				safe_set_config_product_index(node_index,0);
				safe_increment_config_fuelling_mode_index(node_index);

				if (safe_get_config_fuelling_mode_index(node_index) >= safe_get_fuelling_mode_count(node_index))
				{
					safe_set_config_fuelling_mode_index(node_index,0);
					safe_set_config_product_index(node_index,0);
					safe_set_send_price_disp_index(node_index, -1);
					safe_set_node_stage(node_index, ifsfns_Online);
				}
			}

			break;

		case ifsfns_Online:

			break;

	}

}

void adjust_disp_stage(guint8 disp_index, LogOptions log_options)
{
	switch (safe_get_exchange_stage(disp_index))
	{

		case esg_SendZeroFunc:
			safe_set_current_nozzle_index(disp_index, 0);
			safe_set_exchange_stage(disp_index, esg_SetNozzleConf);
			break;

		case esg_SetNozzleConf:
			safe_increment_current_nozzle_index(disp_index);

			if (safe_get_current_nozzle_index(disp_index) == safe_get_nozzle_count(disp_index))
			{
				safe_set_current_nozzle_index(disp_index, 0);
				safe_set_exchange_stage(disp_index, esg_SetFpParam);
			}
			break;

		case esg_SetFpParam:

			if (safe_get_counters_enable())
			{
				safe_set_exchange_stage(disp_index, esg_GetCounters);
			}
			else
			{
				safe_set_exchange_stage(disp_index, esg_Work);
			}
			break;

		case esg_GetCounters:
			safe_increment_current_nozzle_index(disp_index);

			if (safe_get_current_nozzle_index(disp_index) == safe_get_nozzle_count(disp_index))
			{
				safe_set_current_nozzle_index(disp_index, 0);
				safe_set_exchange_stage(disp_index, esg_Work);
			}
			break;

		case esg_Work:

			if (safe_get_original_state(disp_index) == ifsffps_Closed)
			{
				safe_set_exchange_stage(disp_index, esg_OpenFP);
			}
			else if (safe_get_original_state(disp_index) == ifsffps_Idle)
			{
				safe_set_exchange_stage(disp_index, esg_Idle);
			}
			else if (safe_get_original_state(disp_index) == ifsffps_Inoperative && safe_get_transaction_num(disp_index) > 0)
			{
				safe_set_reset(disp_index, TRUE);
				safe_set_exchange_stage(disp_index,esg_GetTransaction);

			}
			break;

		case esg_OpenFP:
			safe_set_exchange_stage(disp_index, esg_WaitOpen);
			break;

		case esg_WaitOpen:
			if (safe_get_original_state(disp_index) == ifsffps_Idle)
			{
				safe_set_exchange_stage(disp_index, esg_AllowedNozzle);
			}
			if (safe_get_original_state(disp_index) == ifsffps_Closed)
			{
				safe_set_exchange_stage(disp_index, esg_OpenFP);
			}
			break;

		case esg_AllowedNozzle:
			safe_set_exchange_stage(disp_index, esg_Idle);
			break;

		case esg_Idle:
			if (safe_get_send_prices(disp_index))
			{
				safe_set_send_prices(disp_index, FALSE);
				safe_set_current_nozzle_index(disp_index, 0);
				safe_set_exchange_stage(disp_index, esg_SetPrices);
			}
			else if (safe_get_original_state(disp_index) == ifsffps_Calling)
			{
#ifdef DEBUG_MODE
				safe_set_preset(disp_index, safe_get_active_nozzle_index(disp_index), (safe_get_active_nozzle_index(disp_index) + 1) * 100, 200, 0, ot_Volume);
				safe_set_is_pay(disp_index, TRUE);
#endif
				if (safe_get_preset_nozzle_index(disp_index) == safe_get_active_nozzle_index(disp_index) &&
					safe_get_preset_order_type(disp_index) != ot_Free)
				{
					safe_set_transaction_num(disp_index, 0);
					safe_set_exchange_stage(disp_index, esg_SetPrice);
					safe_copy_disp_order_type(disp_index);

					if (safe_get_auto_start())
					{
						safe_set_disp_start(disp_index, TRUE);
					}
				}
			}
			else if ((safe_get_original_state(disp_index) == ifsffps_SuspendedStarted) || (safe_get_original_state(disp_index) == ifsffps_SuspendedFuelling))
			{
				safe_set_exchange_stage(disp_index, esg_Idle);
			}
			break;

		case esg_SetPrice:
			safe_set_send_price_disp_index(safe_get_node_index(disp_index), -1);
			safe_set_exchange_stage(disp_index, esg_AuthorizeNozzle);
			break;

		case esg_AuthorizeNozzle:
			safe_set_exchange_stage(disp_index, esg_Preset);
			break;

		case esg_Preset:
			safe_set_exchange_stage(disp_index, esg_WaitStart);
			break;

		case esg_WaitStart:
			safe_set_exchange_stage(disp_index, esg_Filling);
			break;

		case esg_Filling:
			if (safe_get_transaction_num(disp_index) > 0 && safe_get_original_state(disp_index) == ifsffps_Idle)
			{
				safe_set_exchange_stage(disp_index, esg_GetTransaction);
			}

			else if (safe_get_suspend(disp_index))
			{
				safe_set_suspend(disp_index, FALSE);
				safe_set_exchange_stage(disp_index, esg_Suspend);
			}

			else if (safe_get_resume(disp_index))
			{
				safe_set_resume(disp_index, FALSE);
				safe_set_exchange_stage(disp_index, esg_Resume);
			}
			else if (safe_get_original_state(disp_index) == ifsffps_Closed)
			{
				safe_set_exchange_stage(disp_index, esg_OpenFP);
			}

			break;

		case esg_GetTransaction:

			if (safe_compare_disp_values(disp_index))
			{
				if ( safe_get_is_pay(disp_index))
				{
					safe_set_exchange_stage(disp_index, esg_LockTransaction);
				}
			}
			else if (safe_get_reset(disp_index))
			{
				safe_set_exchange_stage(disp_index, esg_LockTransaction);
			}
			else
			{
				if (!safe_get_auto_payment())
				{
					safe_set_is_pay(disp_index, FALSE);
					safe_set_exchange_stage(disp_index, esg_WaitPaying);
				}
				else
				{
					safe_set_exchange_stage(disp_index, esg_LockTransaction);
				}
			}
			break;

		case esg_WaitPaying:
			if ( safe_get_is_pay(disp_index))
			{
				safe_set_exchange_stage(disp_index, esg_LockTransaction);
			}
			break;

		case esg_LockTransaction:
			safe_set_current_nozzle_index(disp_index, 0);

			if (safe_get_counters_enable())
			{
				safe_set_exchange_stage(disp_index, esg_GetFinishCounters);
			}
			else
			{
				safe_set_exchange_stage(disp_index, esg_ClearTransaction);
			}
			break;

		case esg_GetFinishCounters:
			safe_increment_current_nozzle_index(disp_index);

			if (safe_get_current_nozzle_index(disp_index) == safe_get_nozzle_count(disp_index))
			{
				safe_set_current_nozzle_index(disp_index, 0);
				safe_set_exchange_stage(disp_index, esg_ClearTransaction);
			}
			break;

		case esg_ClearTransaction:
			safe_disp_clear(disp_index);
			safe_set_exchange_stage(disp_index, esg_Work);
			break;

		case esg_Suspend:
			safe_set_exchange_stage(disp_index, esg_Filling);
			break;

		case esg_Resume:
			safe_set_exchange_stage(disp_index, esg_Filling);
			break;

		case esg_SetPrices:
			safe_increment_current_nozzle_index(disp_index);
			safe_set_send_price_disp_index(safe_get_node_index(disp_index), -1);

			if (safe_get_current_nozzle_index(disp_index) == safe_get_nozzle_count(disp_index))
			{
				safe_set_current_nozzle_index(disp_index, 0);
				safe_set_exchange_stage(disp_index, esg_Idle);
			}
			break;

	}

}
void parse_frame(guint8* buffer, guint8 length, IFSFNodesData* nodes_data, LogOptions log_options)
{
	guint8 flags = 0;
	guint8 format = 0;
	guint8 src_subnet = 0;
	guint8 src_node = 0;
	guint8 dest_subnet = 0;
	guint8 dest_node = 0;
	guint8 token = 0;

	parse_transport_frame(buffer, length, log_options, &flags, &format, &src_subnet, &src_node, &dest_subnet, &dest_node, &token);

	guint8 index_node = (src_node & 0x0F) - 1;

	if (index_node < MAX_EXCHANGE_NODE_COUNT)
	{

		if (flags == 0)
		{
			switch(format)
			{
				case IFSF_DATA_FORMAT:   		//ACK
					if (safe_get_node_exchange_state(index_node) == ifsfnes_SendReply)
					{
						safe_set_node_exchange_state(index_node, ifsfnes_Free);
					}
					else
					{
						safe_increment_transport_token(index_node);
					}
					break;

				case IFSF_HEARTBEAT_FORMAT:   	//HEARTBEAT

	//				printf("0000\n");
					if ((src_node & 0x0F) - 1 < MAX_NODE_COUNT)
					{
			//			printf("!!!!\n");
						if (safe_get_node_stage(index_node) == ifsfns_Offline)
						{
							safe_set_node_stage(index_node, ifsfns_ConnectReady);
				//			printf("222  %d, %d\n", index_node, safe_get_node_stage(index_node));
						}
					}
					safe_set_last_heartbeat_time(index_node, get_date_time());
					break;
			}
		}
		else
		{
			if ( dest_subnet == IFSF_MASTER_SUBNET && dest_node == (0x80 | IFSF_MASTER_NODE) && (length - 4) > (sizeof(LonTransportFrame) - sizeof(guint8*)) )
			{
				IFSFNodeData* data = &nodes_data->units[index_node];

				if (parse_ifsf_frame(&buffer[IFSF_DATA_OFFSET], data, length - IFSF_DATA_OFFSET - sizeof(guint16), log_options ) &&
					safe_get_node_stage(index_node) > ifsfns_Offline)
				{
					safe_set_reply_transport_token(index_node, token);
					safe_set_reply_ifsf_token(index_node, data->token);

					switch(data->message_type)
					{
						case ifsfmt_Write:

							break;

						case ifsfmt_Read:
							{
								safe_set_node_exchange_state(index_node, ifsfnes_SendReply);


								guint8 buffer_length = prepare_ack_message(buffer, src_node, token);
								if (send_func(buffer, buffer_length, log_options))
								{
									//TODO safe_set_uart_error_counter(active_disp_index, 0);
								}

								safe_increment_transport_token(index_node);

								parse_ifsf_message(data, log_options);

							}
							break;

						case ifsfmt_UnsWAck:

							break;

						case ifsfmt_Answer:
						case ifsfmt_Ack:
							{
								if (data->token == safe_get_ifsf_token(index_node) && safe_get_node_exchange_state(index_node) == ifsfnes_SendCommand)
								{
									parse_ifsf_message(data, log_options);

									adjust_node_stage(index_node, log_options);


									if ( data->database_address.main_address >= ifsfdbid_FuellingPointId1 && data->database_address.main_address <= ifsfdbid_FuellingPointId4  )
									{
										guint disp_addr =  (data->src_node << 4) | (data->database_address.main_address & 0x0F);

										guint8 disp_index = 0;

										if (safe_get_disp_index_by_addr(disp_addr, &disp_index) == de_NoError)
										{
											interpret_original_fp_state(disp_index);
											adjust_disp_stage(disp_index, log_options);
										}
										else
										{
											//g_printf("Error disp index\n");
										}
									}
									if (data->database_address.main_address == ifsfdbid_ProductData && safe_get_node_stage(index_node) == ifsfns_Online &&
										safe_get_send_price_disp_index(index_node) >=0 )
									{
										adjust_disp_stage(safe_get_send_price_disp_index(index_node), log_options);
									}

									safe_set_node_exchange_state(index_node, ifsfnes_Free);
									safe_increment_ifsf_token(index_node);

								}
								guint8 buffer_length = prepare_ack_message(buffer, src_node, token);
								if (send_func(buffer, buffer_length, log_options))
								{
									//TODO safe_set_uart_error_counter(active_disp_index, 0);
								}
							}
							break;

						case ifsfmt_Uns:
							{

								if ( data->database_address.main_address >= ifsfdbid_FuellingPointId1 && data->database_address.main_address <= ifsfdbid_FuellingPointId4  )
								{
									parse_ifsf_message(data, log_options);

									guint disp_addr =  (data->src_node << 4) | (data->database_address.main_address & 0x0F);

									guint8 disp_index = 0;

									if (safe_get_disp_index_by_addr(disp_addr, &disp_index) == de_NoError)
									{
										interpret_original_fp_state(disp_index);
								//		adjust_disp_stage(disp_index, log_options);
									}
									else
									{
										//g_printf("Error disp index\n");
									}
								}

								guint8 buffer_length = prepare_ack_message(buffer, src_node, token);
								if (send_func(buffer, buffer_length, log_options))
								{
									//TODO safe_set_uart_error_counter(active_disp_index, 0);
								}
							}
							break;
					}
					memset(data, 0x00, sizeof(IFSFNodeData));
				}
				else
				{
					guint8 buffer_length = prepare_ack_message(buffer, src_node, token);
					if (send_func(buffer, buffer_length, log_options))
					{
						//TODO safe_set_uart_error_counter(active_disp_index, 0);
					}
				}
			}
		}
	}

}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	IFSFNodesData nodes_data = {0x00};


	IFSFParseStage ps = ifsfps_WaitComm;
	guint8 len = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				switch(ps)
				{
					case ifsfps_WaitComm:
						if (buff[i] == 0x1a)
						{
							length = 0;
							buffer[length++] = buff[i];
							ps = ifsfps_WaitLength;
						}
						break;

					case ifsfps_WaitLength:
						buffer[length++] = buff[i];
						len = buff[i];
						ps = ifsfps_ReadData;
						break;

					case ifsfps_ReadData:
						if (len > 0)
						{
							buffer[length++] = buff[i];
							len--;
						}
						if (len == 0)
						{
							parse_frame(buffer, length, &nodes_data, log_options);

							ps = ifsfps_WaitComm;
						}
						break;
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
		else
		{
		//	add_log(&log_file, TRUE, TRUE, "Read timeout");
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}
