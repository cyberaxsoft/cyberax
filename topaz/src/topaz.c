#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>


#include "driver.h"
#include "logger.h"
#include "topaz.h"

#include "config.h"

#include "write_thread.h"
#include "read_thread.h"
#include "driver_state.h"
#include "uart.h"
#include "topaz_func.h"

gboolean 		driver_init;
GMutex			init_mutex;

DriverStatus status = drs_NotInitializeDriver;
GMutex	status_mutex;

gint uart_descriptor = -1;
gint32 sock = -1;

GThread* 		write_thread = NULL;
GThread* 		read_thread = NULL;

//-------------------------------------------------   setters and getters -----------------------------------------------------------

//--------------------------------------------------     driver_init      -----------------------------------------------------------

gboolean safe_get_driver_init()
{
	gboolean result = FALSE;

	g_mutex_lock(&init_mutex);

	result = driver_init;

	g_mutex_unlock(&init_mutex);

	return result;
}

void safe_set_driver_init(gboolean new_value)
{
	g_mutex_lock(&init_mutex);

	driver_init = new_value;

	g_mutex_unlock(&init_mutex);
}

//--------------------------------------------------       status      -----------------------------------------------------------

DriverStatus safe_get_status()
{
	DriverStatus result = drs_UndefinedError;

	g_mutex_lock(&status_mutex);

	result = status;

	g_mutex_unlock(&status_mutex);

	return result;
}

void safe_set_status(DriverStatus new_status)
{
	g_mutex_lock(&status_mutex);

	status = new_status;

	g_mutex_unlock(&status_mutex);
}

//--------------------------------------------------       read      -----------------------------------------------------------

ssize_t read_func(guint8* buffer)
{
	ssize_t result = 0;

	switch (safe_get_connection_type())
	{
		case ct_Uart:
			result = read(uart_descriptor,buffer, READ_BUFFER_SIZE);
			break;

		case ct_TcpIp:
			result = recv(sock, buffer, READ_BUFFER_SIZE, 0);
			break;

	}

	return result;
}

//--------------------------------------------------       send      -----------------------------------------------------------

gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options)
{
	switch (safe_get_connection_type())
	{
		case  ct_Uart:
			{
				guint32 n = write(uart_descriptor, buffer, size);

				if (n < 0)
				{
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Error! Send failed!");
					safe_set_status(drs_ErrorConnection);
					return FALSE;
				}
				else
				{
					add_tx_buffer_to_log(buffer, size , log_options);
					return TRUE;
				}
			}
			break;

		case ct_TcpIp:
			{
				guint32 n = send(sock, buffer, size, 0);

				if (n < 0)
				{
					add_log(TRUE, TRUE,  log_options.trace, log_options.system, "Error! Send failed!");
					safe_set_status(drs_ErrorConnection);
					return FALSE;
				}
				else
				{
					add_tx_buffer_to_log(buffer, size, log_options);

					return TRUE;
				}
			}
			break;
	}
	return TRUE;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												init_lib
//-----------------------------------------------------------------------------------------------------------------------------------

guchar init_lib(LibConfig config)
{
	if (init_mutex.p == NULL)
	{
		g_mutex_init(&init_mutex);
	}

	if (status_mutex.p == NULL)
	{
		g_mutex_init(&status_mutex);
	}

	init_driver_mutex();
	init_conf_mutex();
	init_write_thread_mutex();
	init_read_thread_mutex();

	if (!safe_get_driver_init())
	{
		safe_set_configuration(config);

		g_printf("%s : dispencer count = %d\n", LOG_PREFIX, config.dispencer_count);

		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		if (log_options.dir != NULL )
		{
			if (log_options.enable)
			{
				if (!init_log_dir(log_options.dir, LOG_PREFIX))
				{
					g_printf("%s : log init error\n", LOG_PREFIX);
					safe_set_status(drs_ErrorLogging);
					return de_ErrorLogging;
				}

				if (!init_log(log_options.dir, log_options.file_size, log_options.save_days))
				{
					g_printf("%s : log create error\n", LOG_PREFIX);
					safe_set_status(drs_ErrorLogging);
					return de_ErrorLogging;
				}

			}

			delete_old_logs(log_options.dir);

			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Init lib ------------------------------------------------------------------------------------------------");
		}
		else
		{
			safe_set_status(drs_ErrorConfiguration);
			return de_ErrorConfiguration;
		}

		safe_set_driver_settings();

		safe_set_current_disp_index(0);

		switch (safe_get_connection_type())
		{
			case  ct_Uart:
				{
					gchar* port = safe_get_port();
					guint32 uart_baudrate = safe_get_uart_baudrate();
					guint8 uart_byte_size = safe_get_uart_byte_size();
					gchar* uart_parity = safe_get_uart_parity();
					guint8 uart_stop_bits = safe_get_uart_stop_bits();

					if (port!=NULL)
					{
						uart_descriptor = open_uart(port, log_options.trace, log_options.system);

						if (uart_descriptor !=-1)
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "Open port %s", port);

						    set_settings_uart(uart_descriptor, uart_baudrate, uart_byte_size, uart_parity,  uart_stop_bits,  safe_get_timeout_read(), log_options.trace, log_options.system);

						    read_thread = g_thread_new("topaz_read_thread", read_thread_func, NULL);
						    if (read_thread == NULL)
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting read thread", LOG_PREFIX);
						    }
						    else
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s read thread started", LOG_PREFIX);
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }

						    write_thread = g_thread_new("topaz_write_thread", write_thread_func, NULL);
						    if (write_thread == NULL)
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting write thread", LOG_PREFIX);
						    }
						    else
						    {
						    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s write thread started", LOG_PREFIX);
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }
						}
						else
						{
							add_log(TRUE, TRUE, log_options.trace, log_options.system, "Open port %s error", port);
							safe_set_status(drs_ErrorConnection);

							return de_ErrorConnection;
						}
					}
					else
					{
						safe_set_status(drs_ErrorConfiguration);
						return de_ErrorConfiguration;
					}
				}
				break;

			case  ct_TcpIp:
				{
					gchar* ip_address = safe_get_ip_address();
					guint32 ip_port = safe_get_ip_port();
					struct sockaddr_in addr;

					sock = socket(AF_INET, SOCK_STREAM, 0);

					if(sock < 0)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Socket create error (address: %s port: %d)", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Socket create (address: %s port: %d)", ip_address, ip_port);

					addr.sin_port = htons( ip_port );
					addr.sin_addr.s_addr = inet_addr(ip_address);
					addr.sin_family = AF_INET;

					add_log(TRUE, TRUE, log_options.trace, log_options.system, "Connecting to %s %d...", ip_address, ip_port);

					if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
					{
						add_log(TRUE, TRUE, log_options.trace, log_options.system, "Connection error %s %d...", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(TRUE, TRUE,log_options.trace, log_options.system, "Connected %s %d...", ip_address, ip_port);

				    read_thread = g_thread_new("topaz_read_thread", read_thread_func, NULL);
				    if (read_thread == NULL)
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting read thread", LOG_PREFIX);
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;
				    }
				    else
				    {
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s read thread started", LOG_PREFIX);
				    }

				    write_thread = g_thread_new("topaz_write_thread", write_thread_func, NULL);

			    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s check write thread", LOG_PREFIX);

				    if (write_thread == NULL)
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s error starting write thread" LOG_PREFIX);
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;
				    }
				    else
				    {
				    	add_log(TRUE, TRUE, log_options.trace, log_options.system, "%s write thread started" LOG_PREFIX);
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    }
				}
				break;
		}

		safe_set_driver_init(TRUE);
		safe_set_status(drs_NoError);

		safe_set_write_thread_terminating(FALSE);
		safe_set_read_thread_terminating(FALSE);

		return de_NoError;
	}
	else
	{
		return de_AlreadyInit;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												close_lib
//-----------------------------------------------------------------------------------------------------------------------------------

guchar close_lib()
{
	if (safe_get_driver_init())
	{
		safe_set_write_thread_terminating(TRUE);

		while (!safe_get_write_thread_terminated());

		safe_set_write_thread_terminated(FALSE);

		g_thread_join(write_thread);

		safe_set_read_thread_terminating(TRUE);

		while (!safe_get_read_thread_terminated());

		safe_set_read_thread_terminated(FALSE);

		g_thread_join(read_thread);

		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Close lib");

		switch (safe_get_connection_type())
		{
			case  ct_Uart:
				if (uart_descriptor!=-1)
				{
					close_uart(uart_descriptor);
				}
				break;

			case  ct_TcpIp:
				if (sock > 0)
				{
					close(sock);
					sock = -1;
				}
				break;
		}

		safe_set_status(drs_NotInitializeDriver);

		safe_set_driver_init(FALSE);

		return de_NoError;
	}
	else
	{
		return de_NotInitializeDriver;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_decimal_point
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_decimal_point(guchar* price_dp, guchar* volume_dp, guchar* amount_dp)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	safe_get_decimal_point_positions(price_dp, volume_dp, amount_dp);

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get decimal points: p:%d v:%d a:%d", *price_dp, *volume_dp, *amount_dp);

	return de_NoError;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_extended_func_count
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_extended_func_count(guchar* count)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get extended func count %d", 0);

	*count = 0;

	return de_NoError;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_extended_func_name
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_extended_func_name(guchar index, guchar* name)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get extended func name error (out of range)");

	return de_OutOfRange;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												extended_func
//-----------------------------------------------------------------------------------------------------------------------------------

guchar extended_func(guchar index, guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	add_log(TRUE, TRUE, log_options.trace, log_options.requests,"Extended func error (out of range)");

	return de_OutOfRange;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_disp_count
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_disp_count(guint8* count)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	*count = safe_get_disp_count();

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get disp count %d", *count);

	return de_NoError;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_disp_info
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_disp_info(guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count )
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	DriverError result = safe_get_disp_info(index_disp, num, addr, nozzle_count);

	if (result == de_NoError)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get disp info i:%d num:%d addr:%d nozzle_count:%d", index_disp, *num, *addr, *nozzle_count);
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get disp info error (out of range)");
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_nozzle_info
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_nozzle_info(guint8 index_disp, guint8 index_nozzle,  guint8* num, guint8* grade )
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	DriverError result = safe_get_nozzle_info(index_disp, index_nozzle, num, grade);

	if (result == de_NoError)
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get nozzle info id:%d in:%d num:%d grade:%d", index_disp, index_nozzle, *num, *grade);
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get nozzle info error (out of range)");
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_status
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_status()
{
	return safe_get_status();
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_disp_state
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_disp_state(guint32 disp_num, guint8* disp_state, guint8* preset_order_type, guint8* preset_nozzle_num,
		guint32* preset_price, guint32* preset_volume, guint32* preset_amount, guint8* order_type, guint8* active_nozzle_num,
		guint32* current_price, guint32* current_volume, guint32* current_amount,
		guint8* is_pay, guint8* error, guchar* error_description)
{

	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 disp_index = 0;

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		result = safe_get_disp_state(disp_index, disp_state, preset_order_type, preset_nozzle_num,
				preset_price, preset_volume, preset_amount, order_type, active_nozzle_num,
				current_price, current_volume, current_amount, is_pay, error, error_description);

		if (result == de_NoError)
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state return: ds = %d (%s), pot = %d, pnn = %d, pp = %d, "
					"pv = %d, pa = %d, ot = %d, ann = %d, cp = %d, cv = %d, ca = %d, "
					"is_pay = %d, error = %d, error_description = %s", disp_num, *disp_state, ds_to_str(*disp_state), *preset_order_type, *preset_nozzle_num,
					*preset_price, *preset_volume, *preset_amount, *order_type, *active_nozzle_num,
					*current_price, *current_volume, *current_amount, *is_pay, *error, error_description);
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state return error (%d)", disp_num, result);
		}
	}
	else
	{
		add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state (index not found) return error %d", disp_num, result);
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												get_counter
//-----------------------------------------------------------------------------------------------------------------------------------

guchar get_counter(guint32 disp_num, guint8 nozzle_num, guint32* counter)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = safe_get_nozzle_counter_by_nums(disp_num, nozzle_num, counter);

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Get counter dipsencer %d nozzle %d  return result %d (counter %d)",disp_num, nozzle_num, result, *counter);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												payment
//-----------------------------------------------------------------------------------------------------------------------------------

guchar payment(unsigned int disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guint8 disp_index = 0;

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (safe_get_current_command(disp_index)!= drc_Free)
	{
		return de_DispencerBusy;
	}

	if (result == de_NoError)
	{
		if (safe_get_preset_order_type(disp_index) != ot_Free)
		{
			safe_set_is_pay(disp_index, TRUE);
		}
	}
	add_log(TRUE, TRUE, log_options.trace, log_options.requests,"Dispencer %d payment return result %d",disp_num, result);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												set_volume_dose
//-----------------------------------------------------------------------------------------------------------------------------------

guchar set_volume_dose(guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 volume)
{
	guchar result = de_Undefined;

	if (!safe_get_driver_init())
	{
		result = de_NotInitializeDriver;
	}
	else
	{
		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{
			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set volume dose (n: %d,p: %d, v: %d ) return result %d",disp_num, nozzle_num, price, volume,  de_DispencerBusy);
				return de_DispencerBusy;
			}

			result = safe_set_disp_preset(disp_num, nozzle_num, price, volume, 0, ot_Volume);

			add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set volume dose (n: %d,p: %d, v: %d ) return result %d",disp_num, nozzle_num, price, volume,  result);

		}
	}
	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												set_sum_dose
//-----------------------------------------------------------------------------------------------------------------------------------

guchar set_sum_dose(guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 amount)
{
	guchar result = de_Undefined;

	if (!safe_get_driver_init())
	{
		result = de_NotInitializeDriver;
	}
	else
	{
		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{
			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set amount dose (n: %d,p: %d, a: %d ) return result %d",disp_num, nozzle_num, price, amount,  de_DispencerBusy);
				return de_DispencerBusy;
			}

			result = safe_set_disp_preset(disp_num, nozzle_num, price, 0, amount, ot_Sum);

			add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set amount dose (n: %d,p: %d, a: %d ) return result %d",disp_num, nozzle_num, price, amount,  result);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												set_full_tank_dose
//-----------------------------------------------------------------------------------------------------------------------------------

guchar set_full_tank_dose(guint32 disp_num, guint8 nozzle_num, guint32 price)
{
	guchar result = de_Undefined;

	if (!safe_get_driver_init())
	{
		result = de_NotInitializeDriver;
	}
	else
	{
		LogOptions log_options = {0x00};
		safe_get_log_options(&log_options);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{
			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set unlim dose (n: %d,p: %d ) return result %d",disp_num, nozzle_num, price,  de_DispencerBusy);
				return de_DispencerBusy;
			}

			guint32 volume = safe_get_full_tank_volume();

			result = safe_set_disp_preset(disp_num, nozzle_num, price, volume, 0, ot_Volume);

			add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set unlim dose (n: %d,p: %d ) return result %d",disp_num, nozzle_num, price,  result);

		}
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												start
//-----------------------------------------------------------------------------------------------------------------------------------

guchar start(unsigned int disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	if (result == de_NoError)
	{
		if (safe_get_preset_order_type(disp_index) != ot_Free) //TODO && safe_get_original_state(disp_index) < dds_Authorized)
		{
			safe_set_disp_start(disp_index, TRUE);
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d start return result %d",disp_num, de_FaultDispencerState);
			return de_FaultDispencerState;
		}
	}

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d start return result %d",disp_num, result);

	return result;

}

//-----------------------------------------------------------------------------------------------------------------------------------
//												stop
//-----------------------------------------------------------------------------------------------------------------------------------

guchar stop(guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = de_NoError;
	if(disp_num > 0)
	{
		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{
			safe_set_emergency_stop(disp_index, TRUE);
		}
	}
	else
	{
		guint8 disp_count = safe_get_disp_count();
		for (guint8 i = 0; i < disp_count; i++)
		{
			safe_set_emergency_stop(i, TRUE);
		}
	}

	add_log(TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d stop return result %d",disp_num, result);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												reset
//-----------------------------------------------------------------------------------------------------------------------------------

guchar reset(guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		safe_set_reset(disp_index, TRUE);
	}

	add_log(TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d reset return result %d",disp_num, result);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												set_prices
//-----------------------------------------------------------------------------------------------------------------------------------

guchar set_prices(guint32 disp_num, guint32 price1, guint32 price2, guint32 price3, guint32 price4, guint32 price5, guint32 price6, guint32 price7, guint32 price8)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		if (safe_get_dispencer_state(disp_index) < ds_Filling && safe_get_dispencer_state(disp_index) > ds_Busy)
		{
			safe_set_prices(disp_index, price1, price2, price3, price4, price5, price6, price7, price8);
		}
		else
		{
			add_log(TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set prices return result %d",disp_num, de_DispencerBusy);
			return de_DispencerBusy;
		}
	}

	add_log(TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d set prices return result %d",disp_num, result);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												suspend
//-----------------------------------------------------------------------------------------------------------------------------------

guchar suspend(guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		safe_set_emergency_stop(disp_index, TRUE);
	}

	add_log(TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d suspend return result %d",disp_num, result);

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------------------
//												resume
//-----------------------------------------------------------------------------------------------------------------------------------

guchar resume(guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	guint8 disp_index = 0;

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);

	guchar result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		safe_set_resume(disp_index, TRUE);
	}

	add_log(TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d resume return result %d",disp_num, result);

	return result;
}
