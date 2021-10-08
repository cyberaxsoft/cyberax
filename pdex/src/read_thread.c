#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "pdex.h"
#include "config.h"
#include "driver_state.h"
#include "pdex_func.h"

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



void adjust_disp_finish(guint8 disp_index, LogOptions log_options)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d adjust disp finish", disp_index);
	if (safe_compare_disp_values(disp_index))
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d compare finish return true", disp_index);
		if (safe_get_is_pay(disp_index) || safe_get_reset(disp_index))
		{
			safe_disp_clear(disp_index);
//			safe_set_exchange_state(disp_index,es_GetStatus);
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d wait payment", disp_index);
//			safe_set_exchange_state(disp_index,es_WaitPayment);
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Dispencer index %d wait payment", disp_index);
//		safe_set_exchange_state(disp_index,es_WaitPayment);
	}
}

void pdex_interpret_data_reply(guint8 disp_index, guint8* buffer, guint8 length, LogOptions log_options)
{
	guint8 channel = buffer[PDEX_ADDRESS_OFFSET] & 0x0F;
	PdexReplyCode reply_type = buffer[PDEX_COMM_OFFSET];

	guint8 auth_code[TRK_AUTH_CODE_LENGTH] = {0x00};

	safe_set_char_ready(disp_index, ctrl_ACK);

	switch(reply_type)
	{
		case prc_Initialization:
			pdex_create_authorization_code(auth_code, &buffer[PDEX_DATA_OFFSET], PDEX_START_ADDRESS + channel);
			safe_set_auth_code(disp_index, auth_code);
			safe_set_exchange_state(disp_index, es_Initialize);
			break;

		case prc_Error:
			{
//				uint32_t error_code = pdex_ascii_to_uint32(&info->buffer[PDEX_DATA_OFFSET], PDEX_ERROR_CODE_LENGTH);
//
//				if (trk_info->exchange_state == des_Initialize && error_code == PDEX_INIT_ERROR_CODE )
//				{
//					trk_info->exchange_state = des_Status;
//					trk_info->pool_ready = false;
//				}
//				else
//				{
//					trk_info->protocol_error = error_code;
//
//					if (error_code!=0)
//					{
//						TRACE(PSTR("ERR%d "), error_code);
//
//						switch(trk_info->exchange_state)
//						{
//							case des_Status:
//								trk_info->exchange_state = des_DataReq;
//								break;
//
//							case des_DataReq:
//								if (trk_info->number_of_grades > 0)
//								{
//									trk_info->exchange_state = des_GetCounters;
//									trk_info->current_counter_index = 0;
//								}
//								else
//								{
//									trk_info->exchange_state = des_Idle;
//								}
//								break;
//
//							case des_GetCounters:
//								if (trk_info->current_counter_index >= trk_info->number_of_grades)
//								{
//									trk_info->current_counter_index = 0;
//									trk_info->exchange_state = des_Idle;
//								}
//								break;
//						}
//						trk_info->pool_ready = false;
//					}
//					else
//					{
//						trk_info->pool_ready = true;
//					}
//				}
			}
			break;

		case prc_DisplayDataRequest:
//			pdex_interpret_display_data(info);
//			trk_info->pool_ready = true;
			break;

		case prc_Status:
//			pdex_interpret_status(info);
//			trk_info->pool_ready = true;

			break;

		case prc_GetCounter:
//			pdex_interpret_counter(info);
//			trk_info->pool_ready = true;
			break;
	}

}

void pdex_interpret_ack_reply(guint8 disp_index, guint8* buffer, guint8 length, LogOptions log_options)
{
	switch (safe_get_exchange_state(disp_index))
	{
		case es_Undefined:

			break;

		case es_Initialize:
			safe_set_exchange_state(disp_index, es_Undefined);
			break;
	}
}

void pdex_interpret_can_reply(guint8 disp_index, guint8* buffer, guint8 length, LogOptions log_options)
{
	switch (safe_get_exchange_state(disp_index))
	{
		case es_Undefined:
			safe_set_exchange_state(disp_index, es_Status);
			break;

		case es_Initialize:

			break;
	}
}


void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");

	guint8 addr = buffer[PDEX_ADDRESS_OFFSET] & 0x0F;
	guint8 ctrl = buffer[PDEX_CTRL_OFFSET];

	guint8 disp_index = 0;

	if (safe_get_disp_index_by_addr(addr, &disp_index) == de_NoError)
	{
		switch (ctrl)
		{
			case ctrl_STX:
				if (pdex_crc(&buffer[PDEX_CTRL_OFFSET], length - PDEX_DATA_OFFSET) == ((buffer[length - 2] << 8) | buffer[length - 1]))
				{
					pdex_interpret_data_reply(disp_index, buffer, length, log_options);
				}
				else
				{
					safe_set_char_ready(disp_index, ctrl_NAK);
				}
				break;

			case ctrl_ACK:
				pdex_interpret_ack_reply(disp_index, buffer, length, log_options);
				safe_set_char_ready(disp_index, ctrl_ENQ);
				break;

			case ctrl_NAK:

				break;

			case ctrl_CAN:
				pdex_interpret_can_reply(disp_index, buffer, length, log_options);
				break;
		}
		safe_set_command_sended(disp_index, FALSE);
	}
}


gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	PdexFrameStage frame_stage = pfs_ReadSyn;

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
				//add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "<< %02X", buff[i]);
				switch(frame_stage)
				{
					case pfs_ReadSyn:
						if (buff[i] == ctrl_SYN)
						{
							frame_stage = pfs_WaitAddr;
						}
						break;

					case pfs_WaitAddr:
						if (buff[i] != ctrl_SYN)
						{
							length = 0;
							buffer[length++] = buff[i];
							frame_stage = pfs_WaitData;
						}
						break;

					case pfs_WaitData:
						buffer[length++] = buff[i];
						switch(buff[i])
						{
							case ctrl_CAN:
							case ctrl_ACK:
							case ctrl_NAK:
								frame_stage = pfs_ReadSyn;
								interpret_reply(buffer, length, log_options);
								break;

							case ctrl_STX:
								frame_stage = pfs_WaitEtx;
								break;

							default:
								add_log(TRUE, TRUE, log_options.trace, log_options.system, "Exchange error");
								frame_stage = pfs_ReadSyn;
								break;
						}
						break;

					case pfs_WaitEtx:
						buffer[length++] = buff[i];
						if (buff[i] == ctrl_ETX)
						{
							frame_stage = pfs_WaitCrc1;
						}
						break;

					case pfs_WaitCrc1:
						buffer[length++] = buff[i];
						frame_stage = pfs_WaitCrc2;
						break;

					case pfs_WaitCrc2:
						buffer[length++] = buff[i];
						frame_stage = pfs_ReadSyn;
						interpret_reply(buffer, length, log_options);
						break;
				}
			}
		}
		else if (byte_count < 0)
		{
			safe_set_status(drs_ErrorConnection);
		}
	}

	safe_set_read_thread_terminated(TRUE);

	return NULL;
}

