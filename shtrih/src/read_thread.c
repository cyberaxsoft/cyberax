#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "config.h"
#include "shtrih.h"
#include "driver_state.h"
#include "shtrih_func.h"

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

void save_log(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " << ");

	for (guint16 i = 0; i < length; i++)
	{
		add_log(FALSE, FALSE, log_options.trace,log_options.frames, " %02X", buffer[i]);

	}
	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");
}

void parse_get_state_frame(guint8* buffer, LogOptions log_options)
{
	guint8 pos = 0;

	gchar* software_version =  g_strdup_printf("%c.%c",buffer[pos], buffer[pos+1]); pos+=2;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Software version: %s", software_version);

	guint16 software_release = (buffer[pos+1] << 8) | buffer[pos]; pos+=2;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Software release: %d", software_release);

	gchar* software_date = g_strdup_printf("%02d.%02d.%04d",buffer[pos], buffer[pos+1], buffer[pos+2] + 2000); pos+=3;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Software date: %s", software_date);

	guint8 number = buffer[pos++];
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Number: %d", number);

	guint16 document_number = (buffer[pos+1] << 8) | buffer[pos]; pos+=2;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Document number: %d", document_number);

	guint16 flags = (buffer[pos+1] << 8) | buffer[pos]; pos+=2;
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Flags:");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Operation journal paper: %s", (flags >> 0) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Receipt paper: %s", (flags >> 1) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Optical sensor journal paper: %s", (flags >> 6) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Optical sensor receipt paper: %s", (flags >> 7) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Termoheader journal paper: %s", (flags >> 8) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Termoheader receipt paper: %s", (flags >> 9) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Cover open: %s", (flags >> 10) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Drawer open: %s", (flags >> 11) & 0x01 ? "yes" : "no");
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "		Cover journal open: %s", (flags >> 12) & 0x01 ? "yes" : "no");

	guint8 mode = buffer[pos++];

	if (mode == sm_DocOpened )
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Mode: %s (%d), %s (%d)", shtrih_mode_to_str(mode & 0x0F), mode & 0x0F,
				shtrih_doc_opened_status_to_str( (mode >> 4) & 0x0F), (mode >> 4) & 0x0F);
	}
	else if (mode == sm_FiscalA4)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Mode: %s (%d), %s (%d)", shtrih_mode_to_str(mode & 0x0F), mode & 0x0F,
				shtrih_fiscal_a4_status_to_str( (mode >> 4) & 0x0F), (mode >> 4) & 0x0F);
	}
	else if (mode == sm_PrintA4)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Mode: %s (%d), %s (%d)", shtrih_mode_to_str(mode & 0x0F), mode & 0x0F,
				shtrih_print_a4_status_to_str( (mode >> 4) & 0x0F), (mode >> 4) & 0x0F);

	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Mode: %s (%d)", shtrih_mode_to_str(mode & 0x0F), mode & 0x0F);
	}

	guint8 submode = buffer[pos++];
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Submode: %s (%d)", shtrih_submode_to_str(submode), submode);

	guint8 port = buffer[pos++];
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Port: %d", port);

//	gchar* date = g_strdup_printf("%02d.%02d.%04d",buffer[pos], buffer[pos+1], buffer[pos+2] + 2000); pos+=3;
//	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Date: %s", date);
//
//	gchar* time = g_strdup_printf("%02d:%02d:%02d",buffer[pos], buffer[pos+1], buffer[pos+2]); pos+=3;
//	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "	Time: %s", time);


	g_free(software_version);
	g_free(software_date);
//	g_free(date);
//	g_free(time);
}

void interpret_reply(guint8* buffer, guint8 length, LogOptions log_options)
{
	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Interpret reply:");

	guint8 command = buffer[COMMAND_REPLY_OFFSET];
	guint8 error = buffer[ERROR_REPLY_OFFSET];
	guint8 operator = buffer[OPERATOR_REPLY_OFFSET];

	add_log(TRUE, TRUE, log_options.trace, log_options.parsing, "Command %s (0x%02X), %s (0x%02X), operator %d (0x%02X)", command_to_str(command), command, error_to_str(error), error, operator, operator);


	if (error == se_NoError)
	{
		switch(command)
		{
			case sc_GetExState:
				parse_get_state_frame(&buffer[OPERATOR_REPLY_OFFSET + 1], log_options);
				break;
		}
	}

}

gpointer read_thread_func(gpointer data)
{
	guint8 buffer[READ_BUFFER_SIZE] = {0x00};
	guint8 buff[READ_BUFFER_SIZE] = {0x00};
	guint8 length = 0;

	guint8 message_length = 0;
	guint8 crc = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	while(!safe_get_read_thread_terminating())
	{
		ssize_t byte_count = read_func(buff);
		if (byte_count > 0)
		{
			for (guint8 i = 0; i < byte_count; i++)
			{
			//	add_log(TRUE, TRUE, log_options.trace, log_options.frames, " <<  %02X", buff[i]);

				switch (safe_get_exchange_state())
				{
					case es_Ready:
						break;

					case es_EnqSent:
						if (buff[i] == ctrl_ACK)
						{
							safe_set_exchange_state(es_WaitSTX);
						}
						else if (buff[i] == ctrl_NAK)
						{
							safe_set_exchange_state(es_CommandReady);
						}
						break;

					case es_CommandReady:

						break;

					case es_DataSent:

						break;

					case es_WaitSTX:
						if (buff[i] == ctrl_STX)
						{
							safe_set_exchange_state(es_WaitLength);
							crc = 0;
						}
						else if (buff[i] == ctrl_ACK)
						{
							safe_set_exchange_state(es_WaitSTX);
						}
						else if (buff[i] == ctrl_NAK)
						{
							safe_set_exchange_state(es_CommandReady);
						}
						else
						{
							safe_set_exchange_state(es_Ready);
						}
						break;

					case es_WaitLength:
						message_length = buff[i];
						crc^= buff[i];
						safe_set_exchange_state(es_WaitData);
						length = 0;
						break;

					case es_WaitData:
						buffer[length++] = buff[i];
						crc^= buff[i];
						if (length == message_length)
						{
							safe_set_exchange_state(es_WaitLrc);
						}
						break;

					case es_WaitLrc:
						if (crc == buff[i])
						{
							save_log(buffer, length, log_options);
							interpret_reply(buffer, length, log_options);
							safe_set_exchange_state(es_ReadyAck);
						}
						else
						{
							safe_set_exchange_state(es_ReadyNak);
						}
						break;

					case es_ReadyAck:
						break;

					case es_ReadyNak:

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
