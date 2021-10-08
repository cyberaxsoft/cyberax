#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tw.h"

FILE* log_file = NULL;
GMutex	log_mutex;

LibConfig configuration;
GMutex	conf_mutex;

gboolean 		driver_init;
guint8			active_disp_index;
guint8			last_sended_disp_index;
guint8			dispencer_count;
Dispencer		dispencers[MAX_DISP_COUNT];
GMutex			driver_mutex;


DriverStatus status = drs_NotInitializeDriver;
GMutex	status_mutex;

gint uart_descriptor = -1;
gint32 sock = -1;

GThread* 		write_thread = NULL;
gboolean 		write_thread_terminating = FALSE;
gboolean 		write_thread_terminated = FALSE;
GMutex			write_thread_mutex;


GThread* 		read_thread = NULL;
gboolean 		read_thread_terminating = FALSE;
gboolean 		read_thread_terminated = FALSE;
GMutex			read_thread_mutex;



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

//-------------------------------------------------   setters and getters -----------------------------------------------------------

gboolean safe_get_write_thread_terminating()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminating;

	g_mutex_unlock(&write_thread_mutex);

	return result;

}

gboolean safe_get_write_thread_terminated()
{
	gboolean result = FALSE;

	g_mutex_lock(&write_thread_mutex);

	result = write_thread_terminated;

	g_mutex_unlock(&write_thread_mutex);

	return result;

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

void safe_set_write_thread_terminating(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminating = new_value;

	g_mutex_unlock(&write_thread_mutex);

}

void safe_set_write_thread_terminated(gboolean new_value)
{
	g_mutex_lock(&write_thread_mutex);

	write_thread_terminated = new_value;

	g_mutex_unlock(&write_thread_mutex);

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

gboolean safe_get_driver_init()
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	result = driver_init;

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_driver_init(gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	driver_init = new_value;

	g_mutex_unlock(&driver_mutex);
}

void safe_set_configuration(LibConfig new_configuration)
{
	g_mutex_lock(&conf_mutex);

	configuration = new_configuration;

	g_mutex_unlock(&conf_mutex);
}

void safe_get_log_options(LogOptions* log_options)
{
	g_mutex_lock(&conf_mutex);

	*log_options = configuration.log_options;

	g_mutex_unlock(&conf_mutex);

}

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

void safe_set_driver_settings()
{
	g_mutex_lock(&conf_mutex);
	g_mutex_lock(&driver_mutex);

	dispencer_count = configuration.dispencer_count;

	if (dispencer_count > 0)
	{
		for (guint8 i = 0; i < dispencer_count; i++)
		{
			dispencers[i].num = configuration.dispencers[i].num;
			dispencers[i].addr = configuration.dispencers[i].addr;
			dispencers[i].nozzle_count = configuration.dispencers[i].nozzle_count;

			dispencers[i].active_nozzle_index = -1;
			dispencers[i].current_amount = 0;
			dispencers[i].current_volume = 0;
			dispencers[i].dispencer_state = ds_NotInitialize;
//			dispencers[i].emergency_stop = FALSE;
//			dispencers[i].exchange_state = des_Free;
			dispencers[i].is_pay = FALSE;
			dispencers[i].order_type = ot_Free;
			dispencers[i].preset_amount = 0;
			dispencers[i].preset_nozzle_index = -1;
			dispencers[i].preset_order_type = ot_Free;
			dispencers[i].preset_price = 0;
			dispencers[i].preset_volume = 0;
			dispencers[i].reset = FALSE;
			dispencers[i].start = FALSE;
			dispencers[i].send_prices = FALSE;

			if (dispencers[i].nozzle_count > 0)
			{
				for (guint8 j = 0; j < dispencers[i].nozzle_count; j++)
				{
					dispencers[i].nozzles[j].num = configuration.dispencers[i].nozzles[j].num;
					dispencers[i].nozzles[j].grade = configuration.dispencers[i].nozzles[j].grade;
				}

			}
		}
	}

	g_mutex_unlock(&driver_mutex);
	g_mutex_unlock(&conf_mutex);

}

ConnectionType safe_get_connection_type()
{
	ConnectionType result = ct_Uart;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.connection_type;

	g_mutex_unlock(&conf_mutex);

	return result;
}

gchar* safe_get_port()
{
	gchar* result = NULL;

	g_mutex_lock(&conf_mutex);

	result = g_strdup(configuration.conn_options.port);

	g_mutex_unlock(&conf_mutex);

	return result;
}

gchar* safe_get_ip_address()
{
	gchar* result = NULL;

	g_mutex_lock(&conf_mutex);

	result = g_strdup(configuration.conn_options.ip_address);

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_ip_port()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.conn_options.ip_port;

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_get_decimal_point_positions(guint8* price_dp, guint8* volume_dp, guint8* amount_dp )
{
	g_mutex_lock(&conf_mutex);

	*price_dp = configuration.decimal_point_options.dp_price;
	*volume_dp = configuration.decimal_point_options.dp_volume;
	*amount_dp = configuration.decimal_point_options.dp_amount;

	g_mutex_unlock(&conf_mutex);
}

guint8 safe_get_disp_count()
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	result = dispencer_count;

	g_mutex_unlock(&driver_mutex);

	return result;
}

DriverError safe_get_disp_index_by_num(guint32 disp_num, guint8* disp_index)
{
	DriverError result = de_OutOfRange;

	g_mutex_lock(&conf_mutex);

	if (configuration.dispencer_count > 0)
	{
		for (guint8 i = 0; i < configuration.dispencer_count; i++)
		{
			if (configuration.dispencers[i].num == disp_num)
			{
				*disp_index = i;
				result = de_NoError;
				break;
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------acitve_disp_index--------------------------------------------------------------
void safe_set_active_disp_index(guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	active_disp_index = new_value;

	g_mutex_unlock(&driver_mutex);
}

guint8 safe_get_active_disp_index()
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	result = active_disp_index;

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_increment_active_disp_index()
{
	g_mutex_lock(&driver_mutex);

	active_disp_index++;

	if (active_disp_index >= dispencer_count)
	{
		active_disp_index = 0;
	}

	g_mutex_unlock(&driver_mutex);
}

//------------------------------------------------------auto_payment--------------------------------------------------------------

gboolean safe_get_auto_payment()
{
	gboolean result = FALSE;

	g_mutex_lock(&conf_mutex);

	result = configuration.auto_payment;

	g_mutex_unlock(&conf_mutex);

	return result;
}

//------------------------------------------------------ preset_nozle_num -----------------------------------------------------------------

guint8 safe_get_preset_nozzle_num(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (dispencers[disp_index].preset_nozzle_index >= 0 && dispencers[disp_index].preset_nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[dispencers[disp_index].preset_nozzle_index].num;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

//------------------------------------------------------ active_nozle_num -----------------------------------------------------------------

guint8 safe_get_active_nozzle_num(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (dispencers[disp_index].active_nozzle_index >= 0 && dispencers[disp_index].active_nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[dispencers[disp_index].active_nozzle_index].num;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

//------------------------------------------------------ error description -----------------------------------------------------------------

void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{
		case 0x00: memcpy(buffer, "No error", strlen("No error")); break;

		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
	}
}


//----------------------------------------------------------------- disp_state -------------------------------------------------------------

guchar safe_get_disp_state(guint8 disp_index, guint8* disp_state, guint8* preset_order_type, guint8* preset_nozzle_num,
		guint32* preset_price, guint32* preset_volume, guint32* preset_amount, guint8* order_type, guint8* active_nozzle_num,
		guint32* current_price, guint32* current_volume, guint32* current_amount,
		guint8* is_pay, guint8* error, guchar* error_description)
{
	DriverError result = de_Undefined;

	*preset_nozzle_num = safe_get_preset_nozzle_num(disp_index);
	*active_nozzle_num = safe_get_active_nozzle_num(disp_index);


	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		*disp_state = dispencers[disp_index].dispencer_state;
		*preset_order_type = dispencers[disp_index].preset_order_type;
		*preset_price = dispencers[disp_index].preset_price;
		*preset_volume = dispencers[disp_index].preset_volume;
		*preset_amount = dispencers[disp_index].preset_amount;

		*order_type = dispencers[disp_index].order_type;
		*current_price = dispencers[disp_index].current_price;
		*current_volume = dispencers[disp_index].current_volume;
		*current_amount = dispencers[disp_index].current_amount;

		*is_pay = dispencers[disp_index].is_pay;
		*error = dispencers[disp_index].error;

		get_error_description(dispencers[disp_index].error, error_description);

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);


	return result;
}

DriverCommand safe_get_current_command(guint8 disp_index)
{
	DriverCommand result = drc_Free;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].current_command;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_current_command(guint8 disp_index, DriverCommand new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_command = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

DriverError safe_get_nozzle_index_by_num(guint8 disp_index, guint8 nozzle_num, guint8* nozzle_index)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (disp_index  < configuration.dispencer_count)
	{
		if (configuration.dispencers[disp_index].nozzle_count > 0)
		{
			for (guint8 i = 0; i < configuration.dispencers[disp_index].nozzle_count; i++ )
			{
				if (configuration.dispencers[disp_index].nozzles[i].num == nozzle_num)
				{
					*nozzle_index = i;
					result = de_NoError;
					break;
				}
			}
		}
		else
		{
			result = de_FaultNozzleNum;
		}
	}
	else
	{
		result = de_FaultDispencerIndex;
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}

DriverError safe_get_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32* counter)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			*counter = dispencers[disp_index].nozzles[nozzle_index].counter;
			result = de_NoError;
		}
		else
		{
			result = de_FaultNozzleIndex;
		}
	}
	else
	{
		result = de_FaultDispencerIndex;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

DriverError safe_get_nozzle_counter_by_nums(guint32 disp_num, guint8 nozzle_num, guint32* counter)
{
	guint8 disp_index = 0;
	guint8 nozzle_index = 0;

	DriverError result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		result = safe_get_nozzle_index_by_num(disp_index, nozzle_num, &nozzle_index);

		if (result == de_NoError)
		{
			result = safe_get_nozzle_counter(disp_index, nozzle_index, counter);
		}
	}

	return result;
}

OrderType safe_get_preset_order_type(guint8 disp_index)
{
	OrderType result = ot_Free;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].preset_order_type;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

guint32 safe_get_timeout_read()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.timeout_options.t_read;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_timeout_write()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.timeout_options.t_write;

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint32 safe_get_full_tank_volume()
{
	guint32 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.full_tank_volume;

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_is_pay(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].is_pay = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

DispencerState safe_get_dispencer_state(guint8 disp_index)
{
	DispencerState result = ds_NotInitialize;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].dispencer_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_dispencer_state(guint8 disp_index, DispencerState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].dispencer_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_nozzle_price(guint8 disp_index, guint8 nozzle_index, guint32 value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].price = value;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_preset(guint8 disp_index, gint8 nozzle_index, guint32 price, guint32 volume, guint32 amount, OrderType order_type)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].preset_nozzle_index = nozzle_index;
			dispencers[disp_index].preset_price = price;
			dispencers[disp_index].preset_volume = volume;
			dispencers[disp_index].preset_amount = amount;
			dispencers[disp_index].preset_order_type = order_type;
		}
	}

	g_mutex_unlock(&driver_mutex);

}


DriverError safe_set_disp_preset(guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 volume, guint32 amount, OrderType order_type)
{
	guint8 disp_index = 0;
	guint8 nozzle_index = 0;

	DriverError result = de_Undefined;

	result = safe_get_disp_index_by_num(disp_num, &disp_index);

	if (result == de_NoError)
	{
		result = safe_get_nozzle_index_by_num(disp_index, nozzle_num, &nozzle_index);

		if (result == de_NoError)
		{
			if (safe_get_preset_order_type(disp_index) == ot_Free )
			{
				safe_set_nozzle_price(disp_index, nozzle_index, price);
				safe_set_preset(disp_index, nozzle_index, price, volume, amount, order_type);
				result = de_NoError;
			}
			else
			{
				result = de_DispencerBusy;
			}
		}
	}

	return result;
}

//--------------------------------------------------------  suspend --------------------------------------------------------------------

gboolean safe_get_suspend(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].suspend;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_suspend(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].suspend = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  resume --------------------------------------------------------------------

gboolean safe_get_resume(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].resume;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_resume(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].resume = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  reset --------------------------------------------------------------------

gboolean safe_get_reset(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].reset;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_reset(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].reset = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  prices --------------------------------------------------------------------

void safe_set_prices(guint8 disp_index, guint32 price1, guint32 price2, guint32 price3, guint32 price4, guint32 price5, guint32 price6, guint32 price7, guint32 price8)
{

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (dispencers[disp_index].nozzle_count > 0)
		{
			dispencers[disp_index].nozzles[0].price = price1;
		}
		if (dispencers[disp_index].nozzle_count > 1)
		{
			dispencers[disp_index].nozzles[1].price = price2;
		}
		if (dispencers[disp_index].nozzle_count > 2)
		{
			dispencers[disp_index].nozzles[2].price = price3;
		}
		if (dispencers[disp_index].nozzle_count > 3)
		{
			dispencers[disp_index].nozzles[3].price = price4;
		}
		if (dispencers[disp_index].nozzle_count > 4)
		{
			dispencers[disp_index].nozzles[4].price = price5;
		}
		if (dispencers[disp_index].nozzle_count > 5)
		{
			dispencers[disp_index].nozzles[5].price = price6;
		}
		if (dispencers[disp_index].nozzle_count > 6)
		{
			dispencers[disp_index].nozzles[6].price = price7;
		}
		if (dispencers[disp_index].nozzle_count > 7)
		{
			dispencers[disp_index].nozzles[7].price = price8;
		}
	}
	g_mutex_unlock(&driver_mutex);

}
gboolean safe_get_send_prices(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].send_prices;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_send_prices(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].send_prices = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  start --------------------------------------------------------------------

gboolean safe_get_disp_start(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].start;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}


void safe_set_disp_start(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].start = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//-----------------------------------------------------------------------------------------------------------------------------------

gpointer read_thread_func(gpointer data)
{


	safe_set_read_thread_terminated(TRUE);

	return NULL;

}


gpointer write_thread_func(gpointer data)
{
	write_thread_terminated = FALSE;

	safe_set_write_thread_terminated(TRUE);

	return NULL;
}


//-----------------------------------------------------------------------------------------------------------------------------------

guchar init_lib(LibConfig config)
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}

	if (conf_mutex.p == NULL)
	{
		g_mutex_init(&conf_mutex);
	}

	if (status_mutex.p == NULL)
	{
		g_mutex_init(&status_mutex);
	}

	if (write_thread_mutex.p == NULL)
	{
		g_mutex_init(&write_thread_mutex);
	}

	if (read_thread_mutex.p == NULL)
	{
		g_mutex_init(&read_thread_mutex);
	}

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
				log_file = create_log(log_options.dir, LOG_PREFIX, &log_mutex, LOG_FILENAME);
				if (log_file == NULL)
				{
					g_printf("%s : log create error\n", LOG_PREFIX);
					safe_set_status(drs_ErrorLogging);
					return de_ErrorLogging;
				}
			}
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Init lib ------------------------------------------------------------------------------------------------");
		}
		else
		{
			safe_set_status(drs_ErrorConfiguration);
			return de_ErrorConfiguration;
		}

		safe_set_driver_settings();

		safe_set_active_disp_index(0);




	switch (safe_get_connection_type())
	{
		case  ct_Uart:
			{
				gchar* port = safe_get_port();

					if (port!=NULL)
					{
						uart_descriptor = open_uart(port, &log_file, log_options, &log_mutex, LOG_PREFIX );

						if (uart_descriptor !=-1)
						{
							add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Open port %s", port);

						    read_thread = g_thread_new("ifsf_read_thread", read_thread_func, NULL);
						    if (read_thread == NULL)
						    {
						    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF error starting read thread");
						    }
						    else
						    {
						    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF read thread started");
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }

						    write_thread = g_thread_new("ifsf_write_thread", write_thread_func, NULL);
						    if (write_thread == NULL)
						    {
						    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF error starting write thread");
						    }
						    else
						    {
						    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF write thread started");
						    	read_thread->priority = G_THREAD_PRIORITY_LOW;
						    }

						}
						else
						{
							add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Open port %s error", port);
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
						add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Socket create error (address: %s port: %d)", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Socket create (address: %s port: %d)", ip_address, ip_port);

					addr.sin_port = htons( ip_port );
					addr.sin_addr.s_addr = inet_addr(ip_address);
					addr.sin_family = AF_INET;

					add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Connecting to %s %d...", ip_address, ip_port);

					if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
					{
						add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Connection error %s %d...", ip_address, ip_port);
						return de_ErrorConnection;
					}
					add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.system, "Connected %s %d...", ip_address, ip_port);

				    read_thread = g_thread_new("ifsf_read_thread", read_thread_func, NULL);
				    if (read_thread == NULL)
				    {
				    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF error starting read thread");
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;

				    }
				    else
				    {
				    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF read thread started");
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    }

				    write_thread = g_thread_new("ifsf_write_thread", write_thread_func, NULL);
				    if (write_thread == NULL)
				    {
				    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF error starting write thread");
						safe_set_status(drs_ErrorConnection);
						return de_ErrorConnection;
				    }
				    else
				    {
				    	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "IFSF write thread started");
				    	read_thread->priority = G_THREAD_PRIORITY_LOW;
				    }
				}
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

		add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Close lib");

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

guchar get_decimal_point(guchar* price_dp, guchar* volume_dp, guchar* amount_dp)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	safe_get_decimal_point_positions(price_dp, volume_dp, amount_dp);

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get decimal points: p:%d v:%d a:%d", *price_dp, *volume_dp, *amount_dp);

	return de_NoError;
}


guchar get_extended_func_count(guchar* count)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get extended func count %d", 0);

	*count = 0;

	return de_NoError;
}

guchar get_extended_func_name(guchar index, guchar* name)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get extended func name error (out of range)");

	return de_OutOfRange;
}

guchar extended_func(guchar index, guint32 disp_num)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests,"Extended func error (out of range)");

	return de_OutOfRange;
}

guchar get_disp_count(guint8* count)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	*count = safe_get_disp_count();

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get disp count %d", *count);


	return de_NoError;
}

guchar get_disp_info(guint8 index_disp, guint32* num, guint8* addr, guint8* nozzle_count )
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (index_disp < configuration.dispencer_count)
	{
		*num = configuration.dispencers[index_disp].num;
		*addr = configuration.dispencers[index_disp].addr;
		*nozzle_count = configuration.dispencers[index_disp].nozzle_count;

		add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get disp info i:%d num:%d addr:%d nozzle_count:%d", index_disp, *num, *addr, *nozzle_count);

		result = de_NoError;
	}
	else
	{
		add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get disp info error (out of range)");
		result = de_OutOfRange;

	}
	g_mutex_unlock(&conf_mutex);

	return result;
}

guchar get_nozzle_info(guint8 index_disp, guint8 index_nozzle,  guint8* num, guint8* grade )
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	DriverError result = de_Undefined;

	g_mutex_lock(&conf_mutex);

	if (index_disp < configuration.dispencer_count)
	{
		if (index_nozzle < configuration.dispencers[index_disp].nozzle_count)
		{
			*num = configuration.dispencers[index_disp].nozzles[index_nozzle].num;
			*grade = configuration.dispencers[index_disp].nozzles[index_nozzle].grade;

			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get nozzle info id:%d in:%d num:%d grade:%d", index_disp, index_nozzle, *num, *grade);
			result = de_NoError;
		}
		else
		{
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get nozzle info error (out of range)");
			result = de_OutOfRange;
		}
	}
	else
	{
		add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get nozzle info error (out of range)");
		result = de_OutOfRange;
	}
	g_mutex_unlock(&conf_mutex);

	return result;
}



guchar get_status()
{
	return safe_get_status();
}


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
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state return: ds = %d (%s), pot = %d, pnn = %d, pp = %d, "
					"pv = %d, pa = %d, ot = %d, ann = %d, cp = %d, cv = %d, ca = %d, "
					"is_pay = %d, error = %d, error_description = %s", disp_num, *disp_state, ds_to_str(*disp_state), *preset_order_type, *preset_nozzle_num,
					*preset_price, *preset_volume, *preset_amount, *order_type, *active_nozzle_num,
					*current_price, *current_volume, *current_amount, *is_pay, *error, error_description);
		}
		else
		{
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state return error (%d)", disp_num, result);
		}
	}
	else
	{
		add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get dispencer %d state (index not found) return error %d", disp_num, result);
	}

	return result;

}

guchar get_counter(guint32 disp_num, guint8 nozzle_num, guint32* counter)
{
	if (!safe_get_driver_init())
	{
		return de_NotInitializeDriver;
	}

	LogOptions log_options = {0x00};
	safe_get_log_options(&log_options);


	guchar result = safe_get_nozzle_counter_by_nums(disp_num, nozzle_num, counter);

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Get counter dipsencer %d nozzle %d  return result %d (counter %d)",disp_num, nozzle_num, result, *counter);

	return result;

}

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
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests,"Dispencer %d payment return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system,"Dispencer %d payment return result %d",disp_num, result);

	return result;

}

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

		guint8 price_dp = 0;
		guint8 volume_dp = 0;
		guint8 amount_dp = 0;

		safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{

			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set volume dose (n: %d,p: %d, v: %d ) return result %d",disp_num, nozzle_num, price, volume,  de_DispencerBusy);
				return de_DispencerBusy;
			}

			result = safe_set_disp_preset(disp_num, nozzle_num, price, volume, 0, ot_Volume);

			if (safe_get_auto_payment())
			{
				safe_set_is_pay(disp_index, TRUE);
			}

			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set volume dose (n: %d,p: %d, v: %d ) return result %d",disp_num, nozzle_num, price, volume,  result);
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set volume dose (n: %d,p: %d, v: %d ) return result %d",disp_num, nozzle_num, price, volume,  result);

		}
	}
	return result;
}


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


		guint8 price_dp = 0;
		guint8 volume_dp = 0;
		guint8 amount_dp = 0;

		safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{

			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set amount dose (n: %d,p: %d, a: %d ) return result %d",disp_num, nozzle_num, price, amount,  de_DispencerBusy);
				return de_DispencerBusy;
			}


			result = safe_set_disp_preset(disp_num, nozzle_num, price, 0, amount, ot_Amount);

			if (safe_get_auto_payment())
			{
				safe_set_is_pay(disp_index, TRUE);
			}


			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set amount dose (n: %d,p: %d, a: %d ) return result %d",disp_num, nozzle_num, price, amount,  result);
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set amount dose (n: %d,p: %d, a: %d ) return result %d",disp_num, nozzle_num, price, amount,  result);
		}
	}

	return result;

}

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


		guint8 price_dp = 0;
		guint8 volume_dp = 0;
		guint8 amount_dp = 0;

		safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

		guint8 disp_index = 0;

		result = safe_get_disp_index_by_num(disp_num, &disp_index);

		if (result == de_NoError)
		{

			if (safe_get_current_command(disp_index)!= drc_Free)
			{
				add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set unlim dose (n: %d,p: %d ) return result %d",disp_num, nozzle_num, price,  de_DispencerBusy);
				return de_DispencerBusy;
			}

			guint32 volume = safe_get_full_tank_volume();

			result = safe_set_disp_preset(disp_num, nozzle_num, price, volume, 0, ot_Volume);

			if (safe_get_auto_payment())
			{
				safe_set_is_pay(disp_index, TRUE);
			}


			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d set unlim dose (n: %d,p: %d ) return result %d",disp_num, nozzle_num, price,  result);
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set unlim dose (n: %d,p: %d ) return result %d",disp_num, nozzle_num, price,  result);

		}
	}

	return result;
}

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

		if (safe_get_preset_order_type(disp_index) != ot_Free )// && safe_get_original_state(disp_index) < dds_Authorized TODO
		{
			safe_set_disp_start(disp_index, TRUE);
		}
		else
		{
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d start return result %d",disp_num, de_FaultDispencerState);
			return de_FaultDispencerState;
		}
	}

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d start return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d start return result %d",disp_num, result);

	return result;

}

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
			safe_set_suspend(disp_index, TRUE);
		}

	}
	else
	{
		guint8 disp_count = safe_get_disp_count();
		for (guint8 i = 0; i < disp_count; i++)
		{
			safe_set_suspend(i, TRUE);
		}
	}


	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.requests, "Dispencer %d stop return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d stop return result %d",disp_num, result);


	return result;
}

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

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d reset return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.system, "Dispencer %d reset return result %d",disp_num, result);

	return result;
}

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
			safe_set_send_prices(disp_index, TRUE);
		}
		else
		{
			add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE, log_options.trace, log_options.system, "Dispencer %d set prices return result %d",disp_num, de_DispencerBusy);
			return de_DispencerBusy;
		}
	}

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d set prices return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.system, "Dispencer %d set prices return result %d",disp_num, result);

	return result;

}

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
		safe_set_suspend(disp_index, TRUE);
	}

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d suspend return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.system, "Dispencer %d suspend return result %d",disp_num, result);

	return result;
}

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

	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.requests, "Dispencer %d resume return result %d",disp_num, result);
	add_log(&log_file, &log_mutex, LOG_PREFIX, TRUE, TRUE,log_options.trace, log_options.system, "Dispencer %d resume return result %d",disp_num, result);

	return result;
}

