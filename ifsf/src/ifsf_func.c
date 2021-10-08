#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "ifsf.h"
#include "config.h"
#include "driver_state.h"

#include "sys/time.h"
#include "ifsf_func.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{

		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
	}
}


guint32 packed_bcd_to_bin(guint8* buff, guint8 size)
{
	guint32 result = 0;

	size *= 2;

	while (size--)
	{
		result *= 10;

		if(size & 0x01)
		{
			result += (*(buff) >> 4);
		}
		else
		{
			result += (*(buff) & 0x0F);
			buff++;

		}

	}

	return result;
}


void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size)
{
	size*=2;

	while(size--)
	{
		if (size & 0x01) *(buff) = val % 10;
		else *(buff--) |= (val % 10) << 4;
		val /= 10;
	}
}

const gchar* ds_to_str(DispencerState state)
{
	switch(state)
	{
		case ds_NotInitialize: return "NotInitialize";
		case ds_Busy: return "Busy";
		case ds_Free: return "Free";
		case ds_NozzleOut: return "NozzleOut";
		case ds_Filling: return "Filling";
		case ds_Stopped: return "Stopped";
		case ds_Finish: return "Finish";
		case ds_ConnectionError: return "ConnectionError";
		default: return "Undefined";
	}
}

guint8 prepare_healthbit_message(guint8* buffer)
{
	guint8 result = 0;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_HEARTBEAT_FLAGS;
	buffer[result++] = IFSF_HEARTBEAT_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_SYSTEM_SUBNET;
	buffer[result++] = IFSF_SYSTEM_NODE;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_addr_message(guint8* buffer, guint8 node_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 6;
	guint8  db_addr_len = 1;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_2;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_2;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_CommunicationService;

	buffer[result++] = ifsfcsdbid_AddRecipientAddr;
	buffer[result++] = 0x02;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_calculator_conf_message(guint8* buffer, guint8 node_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 20;
	guint8  db_addr_len = 1;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;

	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_Calculator;

	buffer[result++] = ifsfcid_DriveOfLightsMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[result++] = ifsfcid_OPTLightMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	buffer[result++] = ifsfcid_ClearDisplayMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0xE2;

	buffer[result++] = ifsfcid_AuthStateMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[result++] = ifsfcid_MaxTimeWOProg;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	buffer[result++] = ifsfcid_MinFuellingVol;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_timeout_message(guint8* buffer, guint8 node_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 5;
	guint8  db_addr_len = 1;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_2;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_2;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_CommunicationService;

	buffer[result++] = ifsfcsdbid_HeartbeatInterval;
	buffer[result++] = 0x01;
	buffer[result++] = SEND_COMMAND_TIMEOUT;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_config_message(guint8* buffer, guint8 node_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 5;
	guint8  db_addr_len = 1;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_Calculator;

	buffer[result++] = ifsfcid_NbProducts;
	buffer[result++] = ifsfcid_NbFuellingModes;
	buffer[result++] = ifsfcid_NbFp;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_set_product_param_message(guint8* buffer, guint8 node_index, guint8 product_index, guint8 fuelling_mode_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 13;
	guint8  db_addr_len = 6;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_ProductData;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(product_index + 1, &buffer[result - 1], 4);
	buffer[result++] = 0x10 | (fuelling_mode_index + 1);

	buffer[result++] = ifsfppfmid_MaxFillTime;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	buffer[result++] = ifsfppfmid_MaxAuthTime;
	buffer[result++] = 0x01;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}
guint8 prepare_set_id_product_message(guint8* buffer, guint8 node_index, guint8 product_index)
{
	guint8 result = 0;

	guint8 node_id = node_index + 1;
	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);
	guint16 length = 8;
	guint8  db_addr_len = 1;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x40 | (product_index + 1);

	buffer[result++] = ifsfpid_ProdNb;
	buffer[result++] = 0x04;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;

	bin_to_packet_bcd(product_index + 1, &buffer[result - 1], 4);


	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_communication_service_reply_message(guint8* buffer, guint8* data, guint8 data_length, guint8 src_subnet,guint8 src_node)
{
	guint8 transport_token = safe_get_transport_token(src_node  - 1);
	guint8 ifsf_token = safe_get_reply_ifsf_token(src_node  - 1);
	guint8  db_addr_len = 1;
	guint16 length = 0;

	guint8 result = 0;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | src_node;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_2;

	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = src_node;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;

	buffer[result++] = IFSF_MC_2;
	buffer[result++] = IFSF_BL;

	buffer[result++] = (ifsfmt_Answer << 5) | ifsf_token;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = db_addr_len; 					length++;
 	buffer[result++] = ifsfdbid_CommunicationService;   length++;

	if (data_length > 0)
	{
		for (guint8 i = 0; i < data_length; i++)
		{
			buffer[result++] = data[i];					length++;
			switch (data[i])
			{
				case ifsfcsdbid_CommProtoVer:

					break;

				case ifsfcsdbid_LocalNodeAddr:

					break;

				case ifsfcsdbid_RecipientAddrTable:

					break;

				case ifsfcsdbid_HeartbeatInterval:
					buffer[result++] = 0x01;			length++;
					buffer[result++] = HEALTHBIT_TIMEOUT; length++;
					break;

				case ifsfcsdbid_MaxBlockLength:

					break;

				case ifsfcsdbid_AddRecipientAddr:

					break;

				case ifsfcsdbid_RemoveRecipientAddr:

					break;

			}
		}
	}

	buffer[18] = length >> 8;
	buffer[19] = length & 0xFF;
	buffer[LENGTH_POS] = result - 2;

	return result;
}


guint8 prepare_zero_func_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 7;
	guint8  db_addr_len = 4;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;

	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;

	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = 0x20;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;

	buffer[result++] = 0x01;
	buffer[result++] = 0x15;

	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_nozzle_count_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 3;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_NbLogicalNozzle;

	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_nozzle_conf_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint8 nozzle_index)
{
	guint8 result = 0;

	guint16 length = 6;
	guint8  db_addr_len = 2;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = 0x10 | (nozzle_index + 1);

	buffer[result++] = ifsflnid_PRId;
	buffer[result++] = 0x01;
	buffer[result++] = nozzle_index + 1;

	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_fp_param_conf_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 8;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = ifsffpid_NbTranBufferNotPaid;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[result++] = ifsffpid_DefaultFuellingMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x02;

	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_nozzle_total_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 4;
	guint8  db_addr_len = 2;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;


	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = 0x10 | (safe_get_current_nozzle_index(disp_index) + 1);

	buffer[result++] = ifsflnid_LogNozVolTotal;

	buffer[LENGTH_POS] = result - 2;

	return result;

}

guint8 prepare_set_price_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 price)
{
	guint8 result = 0;

	guint16 length = 13;
	guint8  db_addr_len = 6;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint8 product_id = safe_get_active_nozzle_index(disp_index) + 1;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_ProductData;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(product_id, &buffer[result - 1], 4);
	buffer[result++] = 0x12;

	buffer[result++] = ifsfppfmid_ProdPrice;
	buffer[result++] = 0x04;
	buffer[result++] = 0x04;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(price, &buffer[result - 1], 3);

	buffer[LENGTH_POS] = result - 2;

	return result;


}

guint8 prepare_product_price_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 13;
	guint8  db_addr_len = 6;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint8 product_id = safe_get_current_nozzle_index(disp_index) + 1;

	guint32 price = safe_get_nozzle_price(disp_index, safe_get_current_nozzle_index(disp_index));

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;

	buffer[result++] = db_addr_len;
	buffer[result++] = ifsfdbid_ProductData;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(product_id, &buffer[result - 1], 4);
	buffer[result++] = 0x12;

	buffer[result++] = ifsfppfmid_ProdPrice;
	buffer[result++] = 0x04;
	buffer[result++] = 0x04;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(price, &buffer[result - 1], 3);

	buffer[LENGTH_POS] = result - 2;

	return result;


}

guint8 prepare_get_pump_state_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 5;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_FpState;
	buffer[result++] = ifsffpid_LogNozState;
	buffer[result++] = ifsffpid_AssignContrId;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_open_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 4;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_OpenFp;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_allowed_nozzles_message(guint8* buffer, guint8 node_index, guint8 disp_index, gboolean allowed)
{
	guint8 result = 0;

	guint16 length = 5;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_LogNozMask;
	buffer[result++] = 0x01;
	if (allowed)
	{
		buffer[result++] = 0x0F;
	}
	else
	{
		buffer[result++] = 0x00;
	}

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_authorize_nozzle_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 5;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint8 index_nozzle = safe_get_active_nozzle_index(disp_index);

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_LogNozMask;
	buffer[result++] = 0x01;
	buffer[result++] = 1 << index_nozzle;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_get_counter_message(guint8* buffer, guint8 disp_addr, guint8 transport_token, guint8 ifsf_token, guint8 nozzle)
{
	guint8 result = 0;

	guint16 length = 5;
	guint8  db_addr_len = 2;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | ((disp_addr & 0xF0) >> 4 );
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = ((disp_addr & 0xF0) >> 4 );
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (disp_addr & 0x0F);
	buffer[result++] = 0x10 | nozzle;
	buffer[result++] = ifsflnid_LogNozVolTotal;
	buffer[result++] = ifsflnid_LogNozAmountTotal;

	buffer[LENGTH_POS] = result - 2;

	return result;
}


guint8 prepare_preset_volume_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 volume)
{
	guint8 result = 0;

	guint16 length = 12;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = ifsffpid_ZeroTRMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[result++] = ifsffpid_RemoteVolumePreset;
	buffer[result++] = 0x05;
	buffer[result++] = 0x06;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(volume, &buffer[result-1], 4);



	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_preset_amount_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 amount)
{
	guint8 result = 0;

	guint16 length = 12;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = ifsffpid_ZeroTRMode;
	buffer[result++] = 0x01;
	buffer[result++] = 0x01;

	buffer[result++] = ifsffpid_RemoteAmountPreset;
	buffer[result++] = 0x05;
	buffer[result++] = 0x06;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(amount, &buffer[result-1], 4);



	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_start_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 4;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = ifsffpid_ReleaseFp;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_light_switch_message(guint8* buffer, guint8 disp_addr, guint8 transport_token, guint8 ifsf_token, gboolean light_switch)
{
	guint8 result = 0;

	guint16 length = 5;
	guint8  db_addr_len = 1;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | ((disp_addr & 0xF0) >> 4 );
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = ((disp_addr & 0xF0) >> 4 );
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (disp_addr & 0x0F);

	buffer[result++] = ifsffpid_DriveOffLightSwitch;
	buffer[result++] = 0x01;
	buffer[result++] = light_switch;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_suspend_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 4;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_SuspendFp;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_resume_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 4;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = ifsffpid_ResumeFp;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_get_filling_data_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 7;
	guint8  db_addr_len = 1;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);

	buffer[result++] = ifsffpid_FpState;
	buffer[result++] = ifsffpid_LogNozState;
	buffer[result++] = ifsffpid_CurrentVolume;
	buffer[result++] = ifsffpid_CurrentAmount;
	buffer[result++] = ifsffpid_CurrentUnitPrice;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_get_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 11;
	guint8  db_addr_len = 4;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint16 transaction_num = safe_get_transaction_num(disp_index);

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Read << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = 0x21;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(transaction_num, &buffer[result-1], 2);

 	buffer[result++] = ifsfftid_TrVolume;
 	buffer[result++] = ifsfftid_TrAmount;
 	buffer[result++] = ifsfftid_TrUnitPrice;
 	buffer[result++] = ifsfftid_TrSeqNb;
 	buffer[result++] = ifsfftid_TransState;
 	buffer[result++] = ifsfftid_TrLogNoz;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_clear_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 7;
	guint8  db_addr_len = 4;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint16 transaction_num = safe_get_transaction_num(disp_index);

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = 0x21;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(transaction_num, &buffer[result-1], 2);

 	buffer[result++] = ifsfftid_ClearTransaction;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}

guint8 prepare_lock_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index)
{
	guint8 result = 0;

	guint16 length = 11;
	guint8  db_addr_len = 4;

	guint8 transport_token = safe_get_transport_token(node_index);
	guint8 ifsf_token = safe_get_ifsf_token(node_index);

	guint8 addr = 0;
	safe_get_disp_addr(disp_index, &addr);
	guint8 node_id = (addr & 0xF0) >> 4;

	guint16 transaction_num = safe_get_transaction_num(disp_index);

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_REQUEST_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | node_id;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = transport_token;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = node_id;
	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = IFSF_MASTER_NODE;
	buffer[result++] = IFSF_MC_0;
	buffer[result++] = IFSF_BL;
	buffer[result++] = (ifsfmt_Write << 5) | ifsf_token;
	buffer[result++] = length >> 8;
	buffer[result++] = length & 0xFF;
	buffer[result++] = db_addr_len;
	buffer[result++] = 0x20 | (addr & 0x0F);
	buffer[result++] = 0x21;

	buffer[result++] = 0x00;
	buffer[result++] = 0x00;
	bin_to_packet_bcd(transaction_num, &buffer[result-1], 2);

 	buffer[result++] = ifsfftid_LockTransaction;
	buffer[result++] = 0x00;

	buffer[LENGTH_POS] = result - 2;

	return result;
}


guint8 prepare_ack_message(guint8* buffer, guint8 disp_node, guint8 transport_token)
{
	guint8 result = 0;

	buffer[result++] = IFSF_START_FLAG;
	buffer[result++] = 0x00;   //length
	buffer[result++] = IFSF_RESPONCE_FLAGS;
	buffer[result++] = IFSF_DATA_FORMAT;

	buffer[result++] = IFSF_MASTER_SUBNET;
	buffer[result++] = 0x80 | IFSF_MASTER_NODE;
	buffer[result++] = IFSF_DISP_SUBNET;
	buffer[result++] = 0x80 | disp_node;
	buffer[result++] = IFSF_GROUP;
	buffer[result++] = 0x20 | transport_token;

	buffer[LENGTH_POS] = result - 2;

	return result;
}


const gchar* ifsf_data_id_to_str(IFSFDbAddress database_addr,  guint8 id)
{
	switch (database_addr.main_address)
	{
		case ifsfdbid_CommunicationService:
			switch (id)
			{
				case 0x01: return "Communication_Protocol_Ver";
				case 0x02: return "Local_Node_Address";
				case 0x03: return "Recipient_Addr_Table";
				case 0x04: return "Heartbeat_Interval";
				case 0x05: return "Max_Block_Length";

				case 0x0B: return "Add_Recipient_Addr";
				case 0x0C: return "Remove_Recipient_Addr";

				default: return "Undefined id";
			}
			break;

		case ifsfdbid_Calculator:
			switch (id)
			{
				case 0x02: return "Nb_Products";
				case 0x03: return "Nb_Fuelling_Modes";
				case 0x04: return "Nb_Meters";
				case 0x05: return "Nb_FP";
				case 0x06: return "Country_Code";
				case 0x07: return "Blend_Tolerance";
				case 0x08: return "Drive_Off_Lights_Mode";
				case 0x09: return "OPT_Light_Mode";
				case 0x0A: return "Clear_Display_Mode";
				case 0x0B: return "Auth_State_Mode";
				case 0x0C: return "Stand_Alone_Auth";
				case 0x0D: return "Max_Auth_Time";
				case 0x15: return "Max_Time_W/O_Prog";
				case 0x16: return "Min_Fuelling_Vol";
				case 0x17: return "Min_Display_Vol";
				case 0x18: return "Min_Guard_Time";
				case 0x1A: return "Pulser_Err_Tolerance";
				case 0x1C: return "Time_Display_Product_Name";
				case 0x28: return "Digits_Vol_Layout";
				case 0x29: return "Digits_Amount_Layout";
				case 0x2A: return "Digits_Unit_Price";
				case 0x2B: return "Unit_Price_Mult_Fact";
				case 0x2C: return "Amount_Rounding_Type";
				case 0x2D: return "Preset_Rounding_Amount";
				case 0x2E: return "Price_Set_Nb";
				case 0x32: return "Manufacturer_Ids";
				case 0x33: return "Dispenser_Model";
				case 0x34: return "Calculator_Type";
				case 0x35: return "Calculator_Serial_No";
				case 0x36: return "Appl_Software_Ver";
				case 0x37: return "W&M_Software_Ver";
				case 0x38: return "W&M_Software_Date";
				case 0x39: return "W&M_Security_Type";
				case 0x3A: return "Protocol_Ver";
				case 0x3B: return "SW_Change_Date";
				case 0x3C: return "SW_Change_Personal_Nb";
				case 0x3D: return "SW_Checksum";
				case 0x46: return "Calc_Illumination";
				case 0x47: return "LCD_Backlight_Switch";
				case 0x48: return "Display_Intensity";
				case 0x50: return "W&M_Polynomial";
				case 0x51: return "W&M_Seed";

				default: return "Undefined id";
			}

			break;

		case ifsfdbid_FuellingPointId1:
		case ifsfdbid_FuellingPointId2:
		case ifsfdbid_FuellingPointId3:
		case ifsfdbid_FuellingPointId4:
			if (database_addr.fuelling_point_detal == 0)
			{
				switch (id)
				{
					case 0x01: return "FP_Name";
					case 0x02: return "Nb_Tran_Buffer_Not_Paid";
					case 0x03: return "Nb_Of_Historic_Trans";
					case 0x04: return "Nb_Logical_Nozzle";
					case 0x06: return "Loudspeaker_Switch";
					case 0x07: return "Default_Fuelling_Mode";
					case 0x08: return "Leak_Log_Noz_Mask";
					case 0x0A: return "Drive_Off_Light_Switch";
					case 0x0B: return "OPT_Light_Switch";
					case 0x14: return "FP_State";
					case 0x15: return "Log_Noz_State";
					case 0x16: return "Assign_Contr_Id";
					case 0x17: return "Release_Mode";
					case 0x18: return "ZeroTR_Mode";
					case 0x19: return "Log_Noz_Mask";
					case 0x1A: return "Config_Lock";
					case 0x1B: return "Remote_Amount_Prepay";
					case 0x1C: return "Remote_Volume_Preset";
					case 0x20: return "Release_Token";
					case 0x21: return "Fuelling_Mode";
					case 0x29: return "Transaction_Sequence_Nb";
					case 0x1D: return "Current_TR_Seq_Nb";
					case 0x1E: return "Release_Contr_Id";
					case 0x1F: return "Suspend_Contr_Id";
					case 0x22: return "Current_Amount";
					case 0x23: return "Current_Volume";
					case 0x24: return "Current_Unit_Price";
					case 0x25: return "Current_Log_Noz";
					case 0x26: return "Current_Prod_Nb";
					case 0x27: return "Current_TR_Error_Code";
					case 0x28: return "Current_Average_Temp";
					case 0x2A: return "Current_Price_Set_Nb";
					case 0x2B: return "Multi_Nozzle_Type";
					case 0x2C: return "Multi_Nozzle_State";
					case 0x2D: return "Multi_Nozzle_Status_Message";
					case 0x33: return "Local_Vol_Preset";
					case 0x34: return "Local_Amount_Prepay";
					case 0x3B: return "Running_Transaction_Message_Frequency";
					case 0x3C: return "Open_FP";
					case 0x3D: return "Close_FP";
					case 0x3E: return "Release_FP";
					case 0x3F: return "Terminate_FP";
					case 0x40: return "Suspend_FP";
					case 0x41: return "Resume_FP";
					case 0x42: return "Clear_Display";
					case 0x43: return "Leak_Command";
					case 0x50: return "FP_Alarm";
					case 0x64: return "FP_Status_Message";
					case 0x65: return "FP_Multi_Nozzle_Status_Message";
					case 0x66: return "FP_Running_Transaction_Message";

					default: return "Undefined id";
				}
			}
			else if (database_addr.fuelling_point_detal >= 0x11 && database_addr.fuelling_point_detal<= 0x18)
			{
				switch (id)
				{
					case ifsflnid_PRId: 					return "PR_Id";
					case ifsflnid_PhisicalNozzleId: 		return "Physical_Noz_Id";
					case ifsflnid_Meter1Id: 				return "Meter_1_Id";
					case ifsflnid_Meter1BlendRatio: 		return "Meter_1_Blend_Ratio";
					case ifsflnid_Meter2Id: 				return "Meter_2_Id";
					case ifsflnid_LogicalNozzleType: 		return "Logical_Nozzle_Type";
					case ifsflnid_HoseExpansionVol: 		return "Hose_Expansion_Vol";
					case ifsflnid_SlowFlowValveActiv: 		return "Slow_Flow_Valve_Activ";
					case ifsflnid_PresetValveActivation: 	return "Preset_Valve_Activation";
					case ifsflnid_LogNozVolTotal: 			return "Log_Noz_Vol_Total";
					case ifsflnid_LogNozAmountTotal: 		return "Log_Noz_Amount_Total";
					case ifsflnid_NoTrTotal: 				return "No_TR_Total";
					case ifsflnid_LogNozSAVolTotal: 		return "Log_Noz_SA_Vol_Total";
					case ifsflnid_LogNozSAAmountTotal: 		return "Log_Noz_SA_Amount_Total";
					case ifsflnid_NoTrSATotal: 				return "No_TR_SA_Total";

					default: return "Undefined id";
				}
			}
			else if (database_addr.fuelling_point_detal == 0x21)
			{
				switch (id)
				{
					case 0x01: return "TR_Seq_Nb";
					case 0x02: return "TR_Contr_Id";
					case 0x03: return "TR_Release_Token";
					case 0x04: return "TR_Fuelling_Mode";
					case 0x05: return "TR_Amount";
					case 0x06: return "TR_Volume";
					case 0x07: return "TR_Unit_Price";
					case 0x08: return "TR_Log_Noz";
					case 0x09: return "TR_Price_Set_Nb";
					case 0x0A: return "TR_Prod_Nb";
					case 0x0B: return "TR_Prod_Description";
					case 0x0C: return "TR_Error_Code";
					case 0x0D: return "TR_Average_Temp";
					case 0x0E: return "TR_Security_Chksum";
					case 0x0F: return "M1_Sub_Volume";
					case 0x10: return "M2_Sub_Volume";
					case 0x11: return "TR_Tax_Amount";
					case 0x15: return "Trans_State";
					case 0x14: return "TR_Buff_Contr_Id";
					case 0x1E: return "Clear_Transaction";
					case 0x1F: return "Lock_Transaction";
					case 0x20: return "Unlock_Transaction";

					case 0x64: return "TR_Buff_Status_Message";


					default: return "Undefined id";
				}
			}
			else if (database_addr.fuelling_point_detal == 0x41)
			{
				switch (id)
				{
					case 0x01: return "FP_Error_Type";
					case 0x02: return "FP_Err_Description";
					case 0x03: return "FP_Error_Total";
					case 0x05: return "FP_Error_State";

					case 0x64: return "FP_Error_Type_Mes";

					default: return "Undefined id";
				}
			}
			else
			{
					return "Undefined detal id";
			}

			break;


		case ifsfdbid_Product1:
		case ifsfdbid_Product2:
		case ifsfdbid_Product3:
		case ifsfdbid_Product4:
		case ifsfdbid_Product5:
		case ifsfdbid_Product6:
		case ifsfdbid_Product7:
		case ifsfdbid_Product8:
			switch (id)
			{
				case 0x02: return "Prod_Nb";
				case 0x03: return "Prod_Description";
				case 0x0A: return "Vap_Recover_Const";

				default: return "Undefined id";
			}
			break;

		case ifsfdbid_ProductData:
			switch (id)
			{
				case 0x01: return "Fuelling_Mode_Name";
				case 0x02: return "Prod_Price";
				case 0x03: return "Max_Vol";
				case 0x04: return "Max_Fill_Time";
				case 0x05: return "Max_Auth_Time";
				case 0x06: return "User_Max_Volume";


				default: return "Undefined id";
			}
			break;


		case ifsfdbid_Meter1:
		case ifsfdbid_Meter2:
		case ifsfdbid_Meter3:
		case ifsfdbid_Meter4:
		case ifsfdbid_Meter5:
		case ifsfdbid_Meter6:
		case ifsfdbid_Meter7:
		case ifsfdbid_Meter8:
		case ifsfdbid_Meter9:
		case ifsfdbid_Meter10:
		case ifsfdbid_Meter11:
		case ifsfdbid_Meter12:
		case ifsfdbid_Meter13:
		case ifsfdbid_Meter14:
		case ifsfdbid_Meter15:
		case ifsfdbid_Meter16:
			switch (id)
			{
				case 0x01: return "Meter_Type";
				case 0x02: return "Meter_Puls_Vol_Fact";
				case 0x03: return "Meter_Calib_Fact";
				case 0x04: return "PR_Id";
				case 0x14: return "Meter_Total";

				default: return ifsfdt_Undefined;
			}
			break;

		default: return "Undefined database addr";

	}
}

IFSFDataType ifsf_return_data_type(IFSFDbAddress database_addr,  guint8 id)
{
	switch (database_addr.main_address)
	{
		case ifsfdbid_CommunicationService:
			switch (id)
			{
				case 0x01: return ifsfdt_Bcd;
				case 0x02: return ifsfdt_Binary;
				case 0x03: return ifsfdt_AddrTable;
				case 0x04: return ifsfdt_Binary;
				case 0x05: return ifsfdt_Binary;

				case 0x0B: return ifsfdt_Binary;
				case 0x0C: return ifsfdt_Binary;

				default: return ifsfdt_Undefined;
			}
			break;

		case ifsfdbid_Calculator:
			switch (id)
			{
				case 0x02: return ifsfdt_Binary;
				case 0x03: return ifsfdt_Binary;
				case 0x04: return ifsfdt_Binary;
				case 0x05: return ifsfdt_Binary;
				case 0x06: return ifsfdt_Bcd;
				case 0x07: return ifsfdt_Bcd;
				case 0x08: return ifsfdt_Binary;
				case 0x09: return ifsfdt_Binary;
				case 0x0A: return ifsfdt_Binary;
				case 0x0B: return ifsfdt_Binary;
				case 0x0C: return ifsfdt_Binary;
				case 0x0D: return ifsfdt_Binary;
				case 0x15: return ifsfdt_Binary;
				case 0x16: return ifsfdt_Binary;
				case 0x17: return ifsfdt_Binary;
				case 0x18: return ifsfdt_Binary;
				case 0x1A: return ifsfdt_Binary;
				case 0x1C: return ifsfdt_Binary;
				case 0x28: return ifsfdt_Bcd;
				case 0x29: return ifsfdt_Bcd;
				case 0x2A: return ifsfdt_Bcd;
				case 0x2B: return ifsfdt_Binary;
				case 0x2C: return ifsfdt_Bcd;
				case 0x2D: return ifsfdt_Bcd;
				case 0x2E: return ifsfdt_Bcd;
				case 0x32: return ifsfdt_Ascii;
				case 0x33: return ifsfdt_Ascii;
				case 0x34: return ifsfdt_Ascii;
				case 0x35: return ifsfdt_Ascii;
				case 0x36: return ifsfdt_Ascii;
				case 0x37: return ifsfdt_Bcd;
				case 0x38: return ifsfdt_Date;
				case 0x39: return ifsfdt_Binary;
				case 0x3A: return ifsfdt_Bcd;
				case 0x3B: return ifsfdt_Date;
				case 0x3C: return ifsfdt_Bcd;
				case 0x3D: return ifsfdt_Ascii;
				case 0x46: return ifsfdt_Binary;
				case 0x47: return ifsfdt_Binary;
				case 0x48: return ifsfdt_Binary;
				case 0x50: return ifsfdt_Binary;
				case 0x51: return ifsfdt_Binary;

				default: return ifsfdt_Undefined;
			}

			break;

		case ifsfdbid_FuellingPointId1:
		case ifsfdbid_FuellingPointId2:
		case ifsfdbid_FuellingPointId3:
		case ifsfdbid_FuellingPointId4:
			if (database_addr.fuelling_point_detal == 0)
			{
				switch (id)
				{
					case 0x01: return ifsfdt_Ascii;
					case 0x02: return ifsfdt_Binary;
					case 0x03: return ifsfdt_Binary;
					case 0x04: return ifsfdt_Binary;
					case 0x06: return ifsfdt_Binary;
					case 0x07: return ifsfdt_Binary;
					case 0x08: return ifsfdt_Binary;
					case 0x0A: return ifsfdt_Binary;
					case 0x0B: return ifsfdt_Binary;
					case 0x14: return ifsfdt_Binary;
					case 0x15: return ifsfdt_Binary;
					case 0x16: return ifsfdt_Binary;
					case 0x17: return ifsfdt_Binary;
					case 0x18: return ifsfdt_Binary;
					case 0x19: return ifsfdt_Binary;
					case 0x1A: return ifsfdt_Binary;
					case 0x1B: return ifsfdt_Amount;
					case 0x1C: return ifsfdt_Volume;
					case 0x20: return ifsfdt_Binary;
					case 0x21: return ifsfdt_Binary;
					case 0x29: return ifsfdt_Bcd;
					case 0x1D: return ifsfdt_Bcd;
					case 0x1E: return ifsfdt_Binary;
					case 0x1F: return ifsfdt_Binary;
					case 0x22: return ifsfdt_Amount;
					case 0x23: return ifsfdt_Volume;
					case 0x24: return ifsfdt_UnitPrice;
					case 0x25: return ifsfdt_Binary;
					case 0x26: return ifsfdt_Bcd;
					case 0x27: return ifsfdt_Binary;
					case 0x28: return ifsfdt_Temp;
					case 0x2A: return ifsfdt_Bcd;
					case 0x2B: return ifsfdt_Binary;
					case 0x2C: return ifsfdt_Binary;
					case 0x2D: return ifsfdt_Binary;
					case 0x33: return ifsfdt_Volume;
					case 0x34: return ifsfdt_Amount;
					case 0x3B: return ifsfdt_Bcd;
					case 0x3C: return ifsfdt_Undefined;
					case 0x3D: return ifsfdt_Undefined;
					case 0x3E: return ifsfdt_Undefined;
					case 0x3F: return ifsfdt_Undefined;
					case 0x40: return ifsfdt_Undefined;
					case 0x41: return ifsfdt_Undefined;
					case 0x42: return ifsfdt_Undefined;
					case 0x43: return ifsfdt_Undefined;
					case 0x50: return ifsfdt_Binary;
					case 0x64: return ifsfdt_Undefined;
					case 0x65: return ifsfdt_Binary;
					case 0x66: return ifsfdt_Undefined;

					default: return ifsfdt_Undefined;
				}
			}
			else if (database_addr.fuelling_point_detal >= 0x11 && database_addr.fuelling_point_detal<= 0x18)
			{
				switch (id)
				{
					case 0x01: return ifsfdt_Binary;
					case 0x05: return ifsfdt_Binary;
					case 0x07: return ifsfdt_Binary;
					case 0x08: return ifsfdt_Bcd;
					case 0x09: return ifsfdt_Binary;
					case 0x0A: return ifsfdt_Binary;
					case 0x03: return ifsfdt_Binary;
					case 0x04: return ifsfdt_Binary;
					case 0x0B: return ifsfdt_Binary;
					case 0x14: return ifsfdt_LongVolume;
					case 0x15: return ifsfdt_LongAmount;
					case 0x16: return ifsfdt_LongNumber;
					case 0x1E: return ifsfdt_LongVolume;
					case 0x1F: return ifsfdt_LongAmount;
					case 0x20: return ifsfdt_LongNumber;

					default: return ifsfdt_Undefined;
				}
			}
			else if (database_addr.fuelling_point_detal == 0x21 )
			{
				switch (id)
				{
					case 0x01: return ifsfdt_Bcd;
					case 0x02: return ifsfdt_Binary;
					case 0x03: return ifsfdt_Binary;
					case 0x04: return ifsfdt_Binary;
					case 0x05: return ifsfdt_Amount;
					case 0x06: return ifsfdt_Volume;
					case 0x07: return ifsfdt_UnitPrice;
					case 0x08: return ifsfdt_Binary;
					case 0x09: return ifsfdt_Bcd;
					case 0x0A: return ifsfdt_Bcd;
					case 0x0B: return ifsfdt_Ascii;
					case 0x0C: return ifsfdt_Binary;
					case 0x0D: return ifsfdt_Temp;
					case 0x0E: return ifsfdt_Binary;
					case 0x0F: return ifsfdt_Volume;
					case 0x10: return ifsfdt_Volume;
					case 0x11: return ifsfdt_Amount;
					case 0x15: return ifsfdt_Binary;
					case 0x14: return ifsfdt_Binary;
					case 0x1E: return ifsfdt_Undefined;
					case 0x1F: return ifsfdt_Undefined;
					case 0x20: return ifsfdt_Undefined;

					case 0x64: return ifsfdt_Undefined;

					default: return ifsfdt_Undefined;
				}
			}

			else if (database_addr.fuelling_point_detal == 0x41 )
			{
				switch (id)
				{
					case 0x01: return ifsfdt_Binary;
					case 0x02: return ifsfdt_Ascii;
					case 0x03: return ifsfdt_Binary;
					case 0x05: return ifsfdt_Binary;

					case 0x64: return ifsfdt_Undefined;

					default: return ifsfdt_Undefined;
				}
			}
			else
			{
				return ifsfdt_Undefined;
			}

			break;

		case ifsfdbid_Product1:
		case ifsfdbid_Product2:
		case ifsfdbid_Product3:
		case ifsfdbid_Product4:
		case ifsfdbid_Product5:
		case ifsfdbid_Product6:
		case ifsfdbid_Product7:
		case ifsfdbid_Product8:
			switch (id)
			{
				case 0x02: return ifsfdt_Bcd;
				case 0x03: return ifsfdt_Ascii;
				case 0x0A: return ifsfdt_Binary;

				default: return ifsfdt_Undefined;
			}
			break;


			case ifsfdbid_ProductData:
				switch (id)
				{
					case 0x01: return ifsfdt_Ascii;
					case 0x02: return ifsfdt_UnitPrice;
					case 0x03: return ifsfdt_Volume;
					case 0x04: return ifsfdt_Binary;
					case 0x05: return ifsfdt_Binary;
					case 0x06: return ifsfdt_Volume;

					default: return ifsfdt_Undefined;
				}
				break;


		case ifsfdbid_Meter1:
		case ifsfdbid_Meter2:
		case ifsfdbid_Meter3:
		case ifsfdbid_Meter4:
		case ifsfdbid_Meter5:
		case ifsfdbid_Meter6:
		case ifsfdbid_Meter7:
		case ifsfdbid_Meter8:
		case ifsfdbid_Meter9:
		case ifsfdbid_Meter10:
		case ifsfdbid_Meter11:
		case ifsfdbid_Meter12:
		case ifsfdbid_Meter13:
		case ifsfdbid_Meter14:
		case ifsfdbid_Meter15:
		case ifsfdbid_Meter16:
			switch (id)
			{
				case 0x01: return ifsfdt_Binary;
				case 0x02: return ifsfdt_Binary;
				case 0x03: return ifsfdt_Bcd;
				case 0x04: return ifsfdt_Binary;
				case 0x14: return ifsfdt_LongVolume;

				default: return ifsfdt_Undefined;
			}


			break;

		default: return ifsfdt_Undefined;

	}

}

guint32 packet_bcd_to_bin(guint8* buff, guint8 size)
{
	guint32 result = 0;

	size*=2;

	while(size--)
	{
		result *= 10;
		result+= (size & 0x01) ? *(buff) >> 4 : *(buff++) & 0x0F;
	}

	return result;
}


const gchar* ifsf_message_type_to_str(IFSFMessageType message_type)
{
	switch(message_type)
	{
		case ifsfmt_Read: 		return "Read";
		case ifsfmt_Answer:		return "Answer";
		case ifsfmt_Write: 		return "Write";
		case ifsfmt_UnsWAck: 	return "Unsolicited with Acknowledge";
		case ifsfmt_Uns:		return "Unsolicited";
		case ifsfmt_Ack: 		return "Acknowledge";

		default: return "Undefined";
	}


}

const gchar* ifsf_database_id_to_str(IFSFDatabaseId id)
{
	switch(id)
	{
		case ifsfdbid_CommunicationService:	return "Communication service";

		default: return "Undefined";
	}
}

const gchar* ifsf_message_status_to_str(IFSFMessageAcknowledgeStatus message_status)
{
	switch(message_status)
	{
		case ifsfmas_Ack: 					return "Ack";
		case ifsfmas_NodeNotReachable:		return "Recipient node not reachable";
		case ifsfmas_ApplOutOfOrder: 		return "Application out of order or not existent on the recipient node";
		case ifsfmas_BlockMissing: 			return "Inconsistent message (block missing)";
		case ifsfmas_DataNotAcceptable:		return "Message refused, some of the data is not acceptable, detailed information follows";
		case ifsfmas_UnknownDbAddress: 		return "Message refused, unknown data base address";
		case ifsfmas_DeviceBusy: 			return "Message refused, receiving device is busy";
		case ifsfmas_MessageUnexpected: 	return "Message unexpected";
		case ifsfmas_DeviceAlreadyLocked: 	return "Device already locked";

		default: return "Undefined";
	}


}


const gchar* ifsf_data_status_to_str(IFSFDataAcknowledgeStatus message_status)
{
	switch(message_status)
	{
		case ifsfdas_Ack: 					return "Ack";
		case ifsfdas_InvalidValue:			return "Invalid value";
		case ifsfdas_NotWritable: 			return "Not Writable";
		case ifsfdas_CommandRefused: 		return "Command refused in that state";
		case ifsfdas_DataNotExist:			return "Data does not exist in this device";
		case ifsfdas_CommandNotUnderstood: 	return "Command not understood";
		case ifsfdas_CommandNotAccepted: 	return "Command not accepted";

		default: return "Undefined";
	}
}

const gchar* ifsf_fp_state_to_str(IFSF_FPState fp_state)
{
	switch(fp_state)
	{
		case ifsffps_Unknown: 				return "Unknown";
		case ifsffps_Inoperative:			return "Inopertive";
		case ifsffps_Closed: 				return "Closed";
		case ifsffps_Idle: 					return "Idle";
		case ifsffps_Calling:				return "Calling";
		case ifsffps_Authorized: 			return "Authorized";
		case ifsffps_Started: 				return "Started";
		case ifsffps_SuspendedStarted: 		return "Suspended started";
		case ifsffps_Fuelling: 				return "Fuelling";
		case ifsffps_SuspendedFuelling: 	return "Suspended fuelling";

		default: return "Undefined";
	}
}

const gchar* ifsf_tr_state_to_str(IFSF_TRState tr_state)
{
	switch(tr_state)
	{
		case ifsftrs_Unknown: 				return "Unknown";
		case ifsftrs_Cleared:				return "Cleared";
		case ifsftrs_Payable: 				return "Payable";
		case ifsftrs_Locked: 				return "Locked";

		default: return "Undefined";
	}
}



