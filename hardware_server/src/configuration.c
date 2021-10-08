#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "configuration.h"
#include "xml.h"
#include "logger.h"
#include "system_func.h"
#include "dc_device.h"
#include "tgs_device.h"
#include "fr_device.h"
#include "ppc_device.h"
#include "sc_device.h"

ServConfig configuration = {0x00};
GMutex	   configuration_mutex;

gchar* 	filename_conf = NULL;

static guint8 dev_index = 0;

void configuration_initialization(gchar* filename)
{
	if (filename_conf != NULL)
	{
		g_free(filename_conf);
		filename_conf = NULL;
	}
	filename_conf = g_strdup(filename);

	g_mutex_init(&configuration_mutex);
}

gboolean read_conf()
{
	return read_settings(filename_conf, &configuration);
}

void join_dc_thread(guint8 index)
{
	g_mutex_lock(&configuration_mutex);

	g_thread_join(configuration.devices.dispencer_controllers[index].device_thread);
	configuration.devices.dispencer_controllers[index].device_thread = NULL;

	g_mutex_unlock(&configuration_mutex);

}

void join_tgs_thread(guint8 index)
{
	g_mutex_lock(&configuration_mutex);

	g_thread_join(configuration.devices.tgs[index].device_thread);
	configuration.devices.tgs[index].device_thread = NULL;

	g_mutex_unlock(&configuration_mutex);

}

void join_fr_thread(guint8 index)
{
	g_mutex_lock(&configuration_mutex);

	g_thread_join(configuration.devices.fiscal_registers[index].device_thread);
	configuration.devices.fiscal_registers[index].device_thread = NULL;

	g_mutex_unlock(&configuration_mutex);

}

void join_ppc_thread(guint8 index)
{
	g_mutex_lock(&configuration_mutex);

	g_thread_join(configuration.devices.price_pole_controllers[index].device_thread);
	configuration.devices.price_pole_controllers[index].device_thread = NULL;

	g_mutex_unlock(&configuration_mutex);

}

void join_sc_thread(guint8 index)
{
	g_mutex_lock(&configuration_mutex);

	g_thread_join(configuration.devices.sensor_controllers[index].device_thread);
	configuration.devices.sensor_controllers[index].device_thread = NULL;

	g_mutex_unlock(&configuration_mutex);

}

gboolean start_dc_thread(guint8 index, guint8 device_index)
{
	gboolean result = FALSE;

	dev_index = device_index;

	g_mutex_lock(&configuration_mutex);

	configuration.devices.dispencer_controllers[index].device_thread = g_thread_new("dc_device_thread", dc_device_thread_func, &dev_index);
	if (configuration.devices.dispencer_controllers[index].device_thread == NULL)
	{
		result = FALSE;
	}
	else
	{
		configuration.devices.dispencer_controllers[index].device_thread->priority = G_THREAD_PRIORITY_LOW;
		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

gboolean start_tgs_thread(guint8 index, guint8 device_index)
{
	gboolean result = FALSE;

	dev_index = device_index;

	g_mutex_lock(&configuration_mutex);

	configuration.devices.tgs[index].device_thread = g_thread_new("tgs_device_thread", tgs_device_thread_func, &dev_index);
	if (configuration.devices.tgs[index].device_thread == NULL)
	{
		result = FALSE;
	}
	else
	{
		configuration.devices.tgs[index].device_thread->priority = G_THREAD_PRIORITY_LOW;
		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);


	return result;

}

gboolean start_fr_thread(guint8 index, guint8 device_index)
{
	gboolean result = FALSE;

	dev_index = device_index;

	g_mutex_lock(&configuration_mutex);

	configuration.devices.fiscal_registers[index].device_thread = g_thread_new("fr_device_thread", fr_device_thread_func, &dev_index);
	if (configuration.devices.fiscal_registers[index].device_thread == NULL)
	{
		result = FALSE;
	}
	else
	{
		configuration.devices.fiscal_registers[index].device_thread->priority = G_THREAD_PRIORITY_LOW;
		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

gboolean start_ppc_thread(guint8 index, guint8 device_index)
{
	gboolean result = FALSE;

	dev_index = device_index;

	g_mutex_lock(&configuration_mutex);

	configuration.devices.price_pole_controllers[index].device_thread = g_thread_new("pp_device_thread", ppc_device_thread_func, &dev_index);
	if (configuration.devices.price_pole_controllers[index].device_thread == NULL)
	{
		result = FALSE;
	}
	else
	{
		configuration.devices.price_pole_controllers[index].device_thread->priority = G_THREAD_PRIORITY_LOW;
		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);


	return result;

}

gboolean start_sc_thread(guint8 index, guint8 device_index)
{
	gboolean result = FALSE;

	dev_index = device_index;

	g_mutex_lock(&configuration_mutex);

	configuration.devices.sensor_controllers[index].device_thread = g_thread_new("sc_device_thread", sensor_controller_device_thread_func, &dev_index);
	if (configuration.devices.sensor_controllers[index].device_thread == NULL)
	{
		result = FALSE;
	}
	else
	{
		configuration.devices.sensor_controllers[index].device_thread->priority = G_THREAD_PRIORITY_LOW;
		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);


	return result;

}


void write_conf()
{
	g_mutex_lock(&configuration_mutex);

	if (filename_conf!=NULL)
	{
		write_settings(filename_conf, &configuration);
	}

	g_mutex_unlock(&configuration_mutex);
}

guint8 get_dc_count()
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	result = configuration.devices.dispencer_controller_count;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

guint8 get_tgs_count()
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	result = configuration.devices.tgs_count;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

guint8 get_fr_count()
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	result = configuration.devices.fiscal_register_count;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

guint8 get_ppc_count()
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	result = configuration.devices.price_pole_controller_count;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

guint8 get_sc_count()
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	result = configuration.devices.sensor_controller_count;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean get_dc_conf_changed(guint8 index)
{
	gboolean result = 0;

	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices.dispencer_controllers[index].changed;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean get_tgs_conf_changed(guint8 index)
{
	gboolean result = 0;

	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices.tgs[index].changed;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean get_fr_conf_changed(guint8 index)
{
	gboolean result = 0;

	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices.fiscal_registers[index].changed;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean get_ppc_conf_changed(guint8 index)
{
	gboolean result = 0;

	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices.price_pole_controllers[index].changed;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean get_sc_conf_changed(guint8 index)
{
	gboolean result = 0;

	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices.sensor_controllers[index].changed;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}


void set_dc_conf_changed(guint8 index, gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		configuration.devices.dispencer_controllers[index].changed = new_value;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_tgs_conf_changed(guint8 index, gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		configuration.devices.tgs[index].changed = new_value;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_fr_conf_changed(guint8 index, gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		configuration.devices.fiscal_registers[index].changed = new_value;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_ppc_conf_changed(guint8 index, gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		configuration.devices.price_pole_controllers[index].changed = new_value;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_sc_conf_changed(guint8 index, gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		configuration.devices.sensor_controllers[index].changed = new_value;
	}

	g_mutex_unlock(&configuration_mutex);
}

void get_dc_conf( guint8 index, DispencerControllerConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		DispencerControllerConf* src_dc_conf = &configuration.devices.dispencer_controllers[index];

		result->id = src_dc_conf->id;

		result->index = index;

		result->device_thread = src_dc_conf->device_thread;
		result->thread_status = src_dc_conf->thread_status;

		if (result->name != NULL)
		{
			g_free(result->name);
			result->name = NULL;
		}
		if(src_dc_conf->name!=NULL)
		{
			result->name = g_strdup(src_dc_conf->name);
		}

		result->port = src_dc_conf->port;
		result->enable = src_dc_conf->enable;
		result->command_timeout = src_dc_conf->command_timeout;
		result->interval = src_dc_conf->interval;

		if (result->module_name != NULL)
		{
			g_free(result->module_name);
			result->module_name = NULL;
		}
		if(src_dc_conf->module_name!=NULL)
		{
			result->module_name = g_strdup(src_dc_conf->module_name);
		}

		if (result->log_dir != NULL)
		{
			g_free(result->log_dir);
			result->log_dir = NULL;
		}
		if(src_dc_conf->log_dir!=NULL)
		{
			result->log_dir = g_strdup(src_dc_conf->log_dir);
		}

		result->log_enable = src_dc_conf->log_enable;
		result->log_trace = src_dc_conf->log_trace;

		result->file_size = src_dc_conf->file_size;
		result->save_days = src_dc_conf->save_days;

		//module config
		DCLibConfig* dst_mc = &result->module_config;
		DCLibConfig* src_mc = &src_dc_conf->module_config;

		//log config
		LibLogOptions* dst_log = &dst_mc->log_options;
		LibLogOptions* src_log = &src_mc->log_options;
		dst_log->enable = src_log->enable;
		if (dst_log->dir != NULL)
		{
			g_free(dst_log->dir);
			dst_log->dir = NULL;
		}
		if(src_log->dir!=NULL)
		{
			dst_log->dir= g_strdup(src_log->dir);
		}

		dst_log->file_size = src_log->file_size;
		dst_log->save_days = src_log->save_days;

		dst_log->trace = src_log->trace;
		dst_log->system = src_log->system;
		dst_log->requests = src_log->requests;
		dst_log->frames = src_log->frames;
		dst_log->parsing = src_log->parsing;

		//connections
		ConnOptions* dst_conn = &dst_mc->conn_options;
		ConnOptions* src_conn = &src_mc->conn_options;
		dst_conn->connection_type = src_conn->connection_type;
		if (dst_conn->port != NULL)
		{
			g_free(dst_conn->port);
			dst_conn->port = NULL;
		}
		if(src_conn->port!=NULL)
		{
			dst_conn->port= g_strdup(src_conn->port);
		}
		if (dst_conn->ip_address != NULL)
		{
			g_free(dst_conn->ip_address);
			dst_conn->ip_address = NULL;
		}
		if(src_conn->ip_address!=NULL)
		{
			dst_conn->ip_address= g_strdup(src_conn->ip_address);
		}
		dst_conn->ip_port = src_conn->ip_port;
		dst_conn->uart_baudrate = src_conn->uart_baudrate;
		dst_conn->uart_byte_size = src_conn->uart_byte_size;
		if(src_conn->uart_parity!=NULL)
		{
			dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
		}
		dst_conn->uart_stop_bits = src_conn->uart_stop_bits;

		//timeouts
		TimeoutOptions* dst_to =  &dst_mc->timeout_options;
		TimeoutOptions* src_to =  &src_mc->timeout_options;
		dst_to->t_read = src_to->t_read;
		dst_to->t_write = src_to->t_write;

		//decimal points
		DCDecimalPointOptions* dst_dpo = &dst_mc->decimal_point_options;
		DCDecimalPointOptions* src_dpo = &src_mc->decimal_point_options;
		dst_dpo->dp_price = src_dpo->dp_price;
		dst_dpo->dp_volume = src_dpo->dp_volume;
		dst_dpo->dp_amount = src_dpo->dp_amount;

		dst_mc->counters_enable = src_mc->counters_enable;
		dst_mc->auto_start = src_mc->auto_start;
		dst_mc->auto_payment = src_mc->auto_payment;
		dst_mc->full_tank_volume = src_mc->full_tank_volume;

		//mapping
		dst_mc->dispencer_count = src_mc->dispencer_count;

		if (dst_mc->dispencer_count > 0)
		{
			for (guint8 j = 0; j < dst_mc->dispencer_count; j++)
			{
				dst_mc->dispencers[j] = src_mc->dispencers[j];
			}
		}
	}
	g_mutex_unlock(&configuration_mutex);
}

void get_tgs_conf( guint8 index, TgsConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		TgsConf* src_tgs_conf = &configuration.devices.tgs[index];

		result->id = src_tgs_conf->id;

		result->device_thread = src_tgs_conf->device_thread;
		result->thread_status = src_tgs_conf->thread_status;

		if (result->name != NULL)
		{
			g_free(result->name);
			result->name = NULL;
		}
		if(src_tgs_conf->name!=NULL)
		{
			result->name = g_strdup(src_tgs_conf->name);
		}

		result->port = src_tgs_conf->port;
		result->enable = src_tgs_conf->enable;
		result->command_timeout = src_tgs_conf->command_timeout;
		result->interval = src_tgs_conf->interval;

		if (result->module_name != NULL)
		{
			g_free(result->module_name);
			result->module_name = NULL;
		}
		if(src_tgs_conf->module_name!=NULL)
		{
			result->module_name = g_strdup(src_tgs_conf->module_name);
		}

		if (result->log_dir != NULL)
		{
			g_free(result->log_dir);
			result->log_dir = NULL;
		}
		if(src_tgs_conf->log_dir!=NULL)
		{
			result->log_dir = g_strdup(src_tgs_conf->log_dir);
		}

		result->log_enable = src_tgs_conf->log_enable;
		result->log_trace = src_tgs_conf->log_trace;

		result->file_size = src_tgs_conf->file_size;
		result->save_days = src_tgs_conf->save_days;

		//module config
		TGSLibConfig* dst_mc = &result->module_config;
		TGSLibConfig* src_mc = &src_tgs_conf->module_config;

		//log config
		LibLogOptions* dst_log = &dst_mc->log_options;
		LibLogOptions* src_log = &src_mc->log_options;
		dst_log->enable = src_log->enable;
		if (dst_log->dir != NULL)
		{
			g_free(dst_log->dir);
			dst_log->dir = NULL;
		}
		if(src_log->dir!=NULL)
		{
			dst_log->dir= g_strdup(src_log->dir);
		}

		dst_log->file_size = src_log->file_size;
		dst_log->save_days = src_log->save_days;

		dst_log->trace = src_log->trace;
		dst_log->system = src_log->system;
		dst_log->requests = src_log->requests;
		dst_log->frames = src_log->frames;
		dst_log->parsing = src_log->parsing;

		//connections
		ConnOptions* dst_conn = &dst_mc->conn_options;
		ConnOptions* src_conn = &src_mc->conn_options;
		dst_conn->connection_type = src_conn->connection_type;
		if (dst_conn->port != NULL)
		{
			g_free(dst_conn->port);
			dst_conn->port = NULL;
		}
		if(src_conn->port!=NULL)
		{
			dst_conn->port= g_strdup(src_conn->port);
		}
		if (dst_conn->ip_address != NULL)
		{
			g_free(dst_conn->ip_address);
			dst_conn->ip_address = NULL;
		}
		if(src_conn->ip_address!=NULL)
		{
			dst_conn->ip_address= g_strdup(src_conn->ip_address);
		}
		dst_conn->ip_port = src_conn->ip_port;
		dst_conn->uart_baudrate = src_conn->uart_baudrate;
		dst_conn->uart_byte_size = src_conn->uart_byte_size;
		if(src_conn->uart_parity!=NULL)
		{
			dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
		}
		dst_conn->uart_stop_bits = src_conn->uart_stop_bits;

		//timeouts
		TimeoutOptions* dst_to =  &dst_mc->timeout_options;
		TimeoutOptions* src_to =  &src_mc->timeout_options;
		dst_to->t_read = src_to->t_read;
		dst_to->t_write = src_to->t_write;

		//mapping
		dst_mc->tank_count = src_mc->tank_count;

		if (dst_mc->tank_count > 0)
		{
			for (guint8 j = 0; j < dst_mc->tank_count; j++)
			{
				dst_mc->tanks[j].num = src_mc->tanks[j].num;
				dst_mc->tanks[j].channel = src_mc->tanks[j].channel;
			}
		}
	}
	g_mutex_unlock(&configuration_mutex);
}

void get_fr_conf( guint8 index, FiscalRegisterConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		FiscalRegisterConf* src_fr_conf = &configuration.devices.fiscal_registers[index];

		result->id = src_fr_conf->id;

		result->index = index;

		result->device_thread = src_fr_conf->device_thread;
		result->thread_status = src_fr_conf->thread_status;

		if (result->name != NULL)
		{
			g_free(result->name);
			result->name = NULL;
		}
		if(src_fr_conf->name!=NULL)
		{
			result->name = g_strdup(src_fr_conf->name);
		}

		result->port = src_fr_conf->port;
		result->enable = src_fr_conf->enable;
		result->command_timeout = src_fr_conf->command_timeout;
		result->interval = src_fr_conf->interval;

		if (result->module_name != NULL)
		{
			g_free(result->module_name);
			result->module_name = NULL;
		}
		if(src_fr_conf->module_name!=NULL)
		{
			result->module_name = g_strdup(src_fr_conf->module_name);
		}

		if (result->log_dir != NULL)
		{
			g_free(result->log_dir);
			result->log_dir = NULL;
		}
		if(src_fr_conf->log_dir!=NULL)
		{
			result->log_dir = g_strdup(src_fr_conf->log_dir);
		}

		result->log_enable = src_fr_conf->log_enable;
		result->log_trace = src_fr_conf->log_trace;

		result->file_size = src_fr_conf->file_size;
		result->save_days = src_fr_conf->save_days;

		//module config
		FRLibConfig* dst_mc = &result->module_config;
		FRLibConfig* src_mc = &src_fr_conf->module_config;

		//log config
		LibLogOptions* dst_log = &dst_mc->log_options;
		LibLogOptions* src_log = &src_mc->log_options;
		dst_log->enable = src_log->enable;
		if (dst_log->dir != NULL)
		{
			g_free(dst_log->dir);
			dst_log->dir = NULL;
		}
		if(src_log->dir!=NULL)
		{
			dst_log->dir= g_strdup(src_log->dir);
		}

		dst_log->file_size = src_log->file_size;
		dst_log->save_days = src_log->save_days;

		dst_log->trace = src_log->trace;
		dst_log->system = src_log->system;
		dst_log->requests = src_log->requests;
		dst_log->frames = src_log->frames;
		dst_log->parsing = src_log->parsing;

		//connections
		ConnOptions* dst_conn = &dst_mc->conn_options;
		ConnOptions* src_conn = &src_mc->conn_options;
		dst_conn->connection_type = src_conn->connection_type;
		if (dst_conn->port != NULL)
		{
			g_free(dst_conn->port);
			dst_conn->port = NULL;
		}
		if(src_conn->port!=NULL)
		{
			dst_conn->port= g_strdup(src_conn->port);
		}
		if (dst_conn->ip_address != NULL)
		{
			g_free(dst_conn->ip_address);
			dst_conn->ip_address = NULL;
		}
		if(src_conn->ip_address!=NULL)
		{
			dst_conn->ip_address= g_strdup(src_conn->ip_address);
		}
		dst_conn->ip_port = src_conn->ip_port;
		dst_conn->uart_baudrate = src_conn->uart_baudrate;
		dst_conn->uart_byte_size = src_conn->uart_byte_size;
		if(src_conn->uart_parity!=NULL)
		{
			dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
		}
		dst_conn->uart_stop_bits = src_conn->uart_stop_bits;

		//timeouts
		TimeoutOptions* dst_to =  &dst_mc->timeout_options;
		TimeoutOptions* src_to =  &src_mc->timeout_options;
		dst_to->t_read = src_to->t_read;
		dst_to->t_write = src_to->t_write;


		dst_mc->protocol_type = src_mc->protocol_type;
		dst_mc->auto_drawer = src_mc->auto_drawer;
		dst_mc->auto_cutting = src_mc->auto_cutting;
		dst_mc->cash_num = src_mc->cash_num;
		dst_mc->bn_num = src_mc->bn_num;
		dst_mc->bonus_num = src_mc->bonus_num;
		dst_mc->time_sync = src_mc->time_sync;

	}
	g_mutex_unlock(&configuration_mutex);
}

void get_ppc_conf( guint8 index, PpcConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		PpcConf* src_pp_conf = &configuration.devices.price_pole_controllers[index];

		result->id = src_pp_conf->id;

		result->device_thread = src_pp_conf->device_thread;
		result->thread_status = src_pp_conf->thread_status;

		if (result->name != NULL)
		{
			g_free(result->name);
			result->name = NULL;
		}
		if(src_pp_conf->name!=NULL)
		{
			result->name = g_strdup(src_pp_conf->name);
		}

		result->port = src_pp_conf->port;
		result->enable = src_pp_conf->enable;
		result->command_timeout = src_pp_conf->command_timeout;
		result->interval = src_pp_conf->interval;

		if (result->module_name != NULL)
		{
			g_free(result->module_name);
			result->module_name = NULL;
		}
		if(src_pp_conf->module_name!=NULL)
		{
			result->module_name = g_strdup(src_pp_conf->module_name);
		}

		if (result->log_dir != NULL)
		{
			g_free(result->log_dir);
			result->log_dir = NULL;
		}
		if(src_pp_conf->log_dir!=NULL)
		{
			result->log_dir = g_strdup(src_pp_conf->log_dir);
		}

		result->log_enable = src_pp_conf->log_enable;
		result->log_trace = src_pp_conf->log_trace;

		result->file_size = src_pp_conf->file_size;
		result->save_days = src_pp_conf->save_days;

		//module config
		PPCLibConfig* dst_mc = &result->module_config;
		PPCLibConfig* src_mc = &src_pp_conf->module_config;

		//log config
		LibLogOptions* dst_log = &dst_mc->log_options;
		LibLogOptions* src_log = &src_mc->log_options;
		dst_log->enable = src_log->enable;
		if (dst_log->dir != NULL)
		{
			g_free(dst_log->dir);
			dst_log->dir = NULL;
		}
		if(src_log->dir!=NULL)
		{
			dst_log->dir= g_strdup(src_log->dir);
		}

		dst_log->file_size = src_log->file_size;
		dst_log->save_days = src_log->save_days;

		dst_log->trace = src_log->trace;
		dst_log->system = src_log->system;
		dst_log->requests = src_log->requests;
		dst_log->frames = src_log->frames;
		dst_log->parsing = src_log->parsing;

		//connections
		ConnOptions* dst_conn = &dst_mc->conn_options;
		ConnOptions* src_conn = &src_mc->conn_options;
		dst_conn->connection_type = src_conn->connection_type;
		if (dst_conn->port != NULL)
		{
			g_free(dst_conn->port);
			dst_conn->port = NULL;
		}
		if(src_conn->port!=NULL)
		{
			dst_conn->port= g_strdup(src_conn->port);
		}
		if (dst_conn->ip_address != NULL)
		{
			g_free(dst_conn->ip_address);
			dst_conn->ip_address = NULL;
		}
		if(src_conn->ip_address!=NULL)
		{
			dst_conn->ip_address= g_strdup(src_conn->ip_address);
		}
		dst_conn->ip_port = src_conn->ip_port;
		dst_conn->uart_baudrate = src_conn->uart_baudrate;
		dst_conn->uart_byte_size = src_conn->uart_byte_size;
		if(src_conn->uart_parity!=NULL)
		{
			dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
		}
		dst_conn->uart_stop_bits = src_conn->uart_stop_bits;

		//timeouts
		TimeoutOptions* dst_to =  &dst_mc->timeout_options;
		TimeoutOptions* src_to =  &src_mc->timeout_options;
		dst_to->t_read = src_to->t_read;
		dst_to->t_write = src_to->t_write;

		//decimal points
		PPCDecimalPointOptions* dst_dpo = &dst_mc->decimal_point_options;
		PPCDecimalPointOptions* src_dpo = &src_mc->decimal_point_options;
		dst_dpo->dp_price = src_dpo->dp_price;

		//mapping
		dst_mc->price_pole_count = src_mc->price_pole_count;

		if (dst_mc->price_pole_count > 0)
		{
			for (guint8 j = 0; j < dst_mc->price_pole_count; j++)
			{
				dst_mc->price_poles[j].num = src_mc->price_poles[j].num;
				dst_mc->price_poles[j].grade = src_mc->price_poles[j].grade;
				dst_mc->price_poles[j].symbol_count = src_mc->price_poles[j].symbol_count;
			}
		}
	}
	g_mutex_unlock(&configuration_mutex);
}

void get_sc_conf( guint8 index, ScConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (index < MAX_DEVICE_COUNT)
	{
		ScConf* src_sc_conf = &configuration.devices.sensor_controllers[index];

		result->id = src_sc_conf->id;

		result->device_thread = src_sc_conf->device_thread;
		result->thread_status = src_sc_conf->thread_status;

		if (result->name != NULL)
		{
			g_free(result->name);
			result->name = NULL;
		}
		if(src_sc_conf->name!=NULL)
		{
			result->name = g_strdup(src_sc_conf->name);
		}

		result->port = src_sc_conf->port;
		result->enable = src_sc_conf->enable;
		result->command_timeout = src_sc_conf->command_timeout;
		result->interval = src_sc_conf->interval;

		if (result->module_name != NULL)
		{
			g_free(result->module_name);
			result->module_name = NULL;
		}
		if(src_sc_conf->module_name!=NULL)
		{
			result->module_name = g_strdup(src_sc_conf->module_name);
		}

		if (result->log_dir != NULL)
		{
			g_free(result->log_dir);
			result->log_dir = NULL;
		}
		if(src_sc_conf->log_dir!=NULL)
		{
			result->log_dir = g_strdup(src_sc_conf->log_dir);
		}

		result->log_enable = src_sc_conf->log_enable;
		result->log_trace = src_sc_conf->log_trace;

		result->file_size = src_sc_conf->file_size;
		result->save_days = src_sc_conf->save_days;

		//module config
		SCLibConfig* dst_mc = &result->module_config;
		SCLibConfig* src_mc = &src_sc_conf->module_config;

		//log config
		LibLogOptions* dst_log = &dst_mc->log_options;
		LibLogOptions* src_log = &src_mc->log_options;
		dst_log->enable = src_log->enable;
		if (dst_log->dir != NULL)
		{
			g_free(dst_log->dir);
			dst_log->dir = NULL;
		}
		if(src_log->dir!=NULL)
		{
			dst_log->dir= g_strdup(src_log->dir);
		}

		dst_log->file_size = src_log->file_size;
		dst_log->save_days = src_log->save_days;

		dst_log->trace = src_log->trace;
		dst_log->system = src_log->system;
		dst_log->requests = src_log->requests;
		dst_log->frames = src_log->frames;
		dst_log->parsing = src_log->parsing;

		//connections
		ConnOptions* dst_conn = &dst_mc->conn_options;
		ConnOptions* src_conn = &src_mc->conn_options;
		dst_conn->connection_type = src_conn->connection_type;
		if (dst_conn->port != NULL)
		{
			g_free(dst_conn->port);
			dst_conn->port = NULL;
		}
		if(src_conn->port!=NULL)
		{
			dst_conn->port= g_strdup(src_conn->port);
		}
		if (dst_conn->ip_address != NULL)
		{
			g_free(dst_conn->ip_address);
			dst_conn->ip_address = NULL;
		}
		if(src_conn->ip_address!=NULL)
		{
			dst_conn->ip_address= g_strdup(src_conn->ip_address);
		}
		dst_conn->ip_port = src_conn->ip_port;
		dst_conn->uart_baudrate = src_conn->uart_baudrate;
		dst_conn->uart_byte_size = src_conn->uart_byte_size;
		if(src_conn->uart_parity!=NULL)
		{
			dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
		}
		dst_conn->uart_stop_bits = src_conn->uart_stop_bits;

		//timeouts
		TimeoutOptions* dst_to =  &dst_mc->timeout_options;
		TimeoutOptions* src_to =  &src_mc->timeout_options;
		dst_to->t_read = src_to->t_read;
		dst_to->t_write = src_to->t_write;

		//mapping
		dst_mc->sensor_count = src_mc->sensor_count;

		if (dst_mc->sensor_count > 0)
		{
			for (guint8 j = 0; j < dst_mc->sensor_count; j++)
			{
				dst_mc->sensors[j].num = src_mc->sensors[j].num;
				dst_mc->sensors[j].addr = src_mc->sensors[j].addr;

				dst_mc->sensors[j].param_count = src_mc->sensors[j].param_count;

				if (dst_mc->sensors[j].param_count > 0)
				{
					for(guint8 k = 0; k < dst_mc->sensors[j].param_count; k++)
					{
						dst_mc->sensors[j].params[k].num = src_mc->sensors[j].params[k].num;
						dst_mc->sensors[j].params[k].type = src_mc->sensors[j].params[k].type;
					}
				}

			}
		}
	}
	g_mutex_unlock(&configuration_mutex);
}
void get_common_conf( CommonConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (result->server_name != NULL)
	{
		g_free(result->server_name);
		result->server_name = NULL;
	}

	if (configuration.common_conf.server_name!=NULL)
	{
		result->server_name = g_strdup(configuration.common_conf.server_name);
	}
	result->port = configuration.common_conf.port;
	if (result->log_dir != NULL)
	{
		g_free(result->log_dir);
		result->log_dir = NULL;
	}
	if (configuration.common_conf.log_dir!=NULL)
	{
		result->log_dir = g_strdup(configuration.common_conf.log_dir);
	}
	result->log_enable = configuration.common_conf.log_enable;
	result->log_trace = configuration.common_conf.log_trace;


	result->file_size = configuration.common_conf.file_size;
	result->save_days = configuration.common_conf.save_days;

	if (result->conn_log_dir != NULL)
	{
		g_free(result->conn_log_dir);
		result->conn_log_dir = NULL;
	}
	if (configuration.common_conf.conn_log_dir!=NULL)
	{
		result->conn_log_dir = g_strdup(configuration.common_conf.conn_log_dir);
	}
	result->conn_log_enable = configuration.common_conf.conn_log_enable;
	result->conn_log_frames = configuration.common_conf.conn_log_frames;
	result->conn_log_parsing = configuration.common_conf.conn_log_parsing;
	result->conn_log_trace = configuration.common_conf.conn_log_trace;

	result->conn_log_file_size = configuration.common_conf.conn_log_file_size;
	result->conn_log_save_days = configuration.common_conf.conn_log_save_days;


	g_mutex_unlock(&configuration_mutex);

}

gboolean get_common_conf_changed()
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	result = configuration.common_conf.changed;

	g_mutex_unlock(&configuration_mutex);

	return result;
}

void set_common_conf_changed(gboolean new_value)
{

	g_mutex_lock(&configuration_mutex);

	configuration.common_conf.changed = new_value;

	g_mutex_unlock(&configuration_mutex);

}

void get_device_confs(DeviceConfs* result)
{
	g_mutex_lock(&configuration_mutex);

	result->dispencer_controller_count = configuration.devices.dispencer_controller_count;

	if (result->dispencer_controller_count > 0)
	{
		for (guint8 i = 0; i < result->dispencer_controller_count; i++)
		{
			DispencerControllerConf* dst_dc_conf = &result->dispencer_controllers[i];
			DispencerControllerConf* src_dc_conf = &configuration.devices.dispencer_controllers[i];

			dst_dc_conf->id = src_dc_conf->id;

			if (dst_dc_conf->name != NULL)
			{
				g_free(dst_dc_conf->name);
				dst_dc_conf->name = NULL;
			}
			if(src_dc_conf->name!=NULL)
			{
				dst_dc_conf->name = g_strdup(src_dc_conf->name);
			}

			dst_dc_conf->port = src_dc_conf->port;
			dst_dc_conf->enable = src_dc_conf->enable;
			dst_dc_conf->command_timeout = src_dc_conf->command_timeout;
			dst_dc_conf->interval = src_dc_conf->interval;

			if (dst_dc_conf->module_name != NULL)
			{
				g_free(dst_dc_conf->module_name);
				dst_dc_conf->module_name = NULL;
			}
			if(src_dc_conf->module_name!=NULL)
			{
				dst_dc_conf->module_name = g_strdup(src_dc_conf->module_name);
			}

			if (dst_dc_conf->log_dir != NULL)
			{
				g_free(dst_dc_conf->log_dir);
				dst_dc_conf->log_dir = NULL;
			}
			if(src_dc_conf->log_dir!=NULL)
			{
				dst_dc_conf->log_dir = g_strdup(src_dc_conf->log_dir);
			}

			dst_dc_conf->log_enable = src_dc_conf->log_enable;
			dst_dc_conf->log_trace = src_dc_conf->log_trace;

			dst_dc_conf->file_size = src_dc_conf->file_size;
			dst_dc_conf->save_days = src_dc_conf->save_days;


			//module config
			DCLibConfig* dst_mc = &dst_dc_conf->module_config;
			DCLibConfig* src_mc = &src_dc_conf->module_config;

			//log config
			LibLogOptions* dst_log = &dst_mc->log_options;
			LibLogOptions* src_log = &src_mc->log_options;
			dst_log->enable = src_log->enable;
			if (dst_log->dir != NULL)
			{
				g_free(dst_log->dir);
				dst_log->dir = NULL;
			}
			if(src_log->dir!=NULL)
			{
				dst_log->dir= g_strdup(src_log->dir);
			}

			dst_log->file_size = src_log->file_size;
			dst_log->save_days = src_log->save_days;

			dst_log->trace = src_log->trace;
			dst_log->system = src_log->system;
			dst_log->requests = src_log->requests;
			dst_log->frames = src_log->frames;
			dst_log->parsing = src_log->parsing;

			//connections
			ConnOptions* dst_conn = &dst_mc->conn_options;
			ConnOptions* src_conn = &src_mc->conn_options;
			dst_conn->connection_type = src_conn->connection_type;
			if (dst_conn->port != NULL)
			{
				g_free(dst_conn->port);
				dst_conn->port = NULL;
			}
			if(src_conn->port!=NULL)
			{
				dst_conn->port= g_strdup(src_conn->port);
			}
			if (dst_conn->ip_address != NULL)
			{
				g_free(dst_conn->ip_address);
				dst_conn->ip_address = NULL;
			}
			if(src_conn->ip_address!=NULL)
			{
				dst_conn->ip_address= g_strdup(src_conn->ip_address);
			}
			dst_conn->ip_port = src_conn->ip_port;
			dst_conn->uart_baudrate = src_conn->uart_baudrate;
			dst_conn->uart_byte_size = src_conn->uart_byte_size;
			if(src_conn->uart_parity!=NULL)
			{
				dst_conn->uart_parity= g_strdup(src_conn->uart_parity);
			}
			dst_conn->uart_stop_bits = src_conn->uart_stop_bits;


			//timeouts
			TimeoutOptions* dst_to =  &dst_mc->timeout_options;
			TimeoutOptions* src_to =  &src_mc->timeout_options;
			dst_to->t_read = src_to->t_read;
			dst_to->t_write = src_to->t_write;

			//decimal points
			DCDecimalPointOptions* dst_dpo = &dst_mc->decimal_point_options;
			DCDecimalPointOptions* src_dpo = &src_mc->decimal_point_options;
			dst_dpo->dp_price = src_dpo->dp_price;
			dst_dpo->dp_volume = src_dpo->dp_volume;
			dst_dpo->dp_amount = src_dpo->dp_amount;

			dst_mc->counters_enable = src_mc->counters_enable;
			dst_mc->auto_start = src_mc->auto_start;
			dst_mc->auto_payment = src_mc->auto_payment;
			dst_mc->full_tank_volume = src_mc->full_tank_volume;

			//mapping
			dst_mc->dispencer_count = src_mc->dispencer_count;

			if (dst_mc->dispencer_count > 0)
			{
				for (guint8 j = 0; j < dst_mc->dispencer_count; j++)
				{
					dst_mc->dispencers[j] = src_mc->dispencers[j];
				}
			}
		}
	}

	result->tgs_count = configuration.devices.tgs_count;

	if (result->tgs_count > 0)
	{
		for (guint8 i = 0; i < result->tgs_count; i++)
		{
			TgsConf* dst_tgs_conf = &result->tgs[i];
			TgsConf* src_tgs_conf = &configuration.devices.tgs[i];

			dst_tgs_conf->id = src_tgs_conf->id;

			if (dst_tgs_conf->name != NULL)
			{
				g_free(dst_tgs_conf->name);
				dst_tgs_conf->name = NULL;
			}
			if(src_tgs_conf->name!=NULL)
			{
				dst_tgs_conf->name = g_strdup(src_tgs_conf->name);
			}

			dst_tgs_conf->port = src_tgs_conf->port;
			dst_tgs_conf->enable = src_tgs_conf->enable;
			dst_tgs_conf->command_timeout = src_tgs_conf->command_timeout;
			dst_tgs_conf->interval = src_tgs_conf->interval;

			if (dst_tgs_conf->module_name != NULL)
			{
				g_free(dst_tgs_conf->module_name);
				dst_tgs_conf->module_name = NULL;
			}
			if(src_tgs_conf->module_name!=NULL)
			{
				dst_tgs_conf->module_name = g_strdup(src_tgs_conf->module_name);
			}

			if (dst_tgs_conf->log_dir != NULL)
			{
				g_free(dst_tgs_conf->log_dir);
				dst_tgs_conf->log_dir = NULL;
			}
			if(src_tgs_conf->log_dir!=NULL)
			{
				dst_tgs_conf->log_dir = g_strdup(src_tgs_conf->log_dir);
			}

			dst_tgs_conf->log_enable = src_tgs_conf->log_enable;
			dst_tgs_conf->log_trace = src_tgs_conf->log_trace;

			//module config
			TGSLibConfig* dst_mc = &dst_tgs_conf->module_config;
			TGSLibConfig* src_mc = &src_tgs_conf->module_config;

			//log config
			LibLogOptions* dst_log = &dst_mc->log_options;
			LibLogOptions* src_log = &src_mc->log_options;
			dst_log->enable = src_log->enable;
			if (dst_log->dir != NULL)
			{
				g_free(dst_log->dir);
				dst_log->dir = NULL;
			}
			if(src_log->dir!=NULL)
			{
				dst_log->dir= g_strdup(src_log->dir);
			}
			dst_log->trace = src_log->trace;
			dst_log->system = src_log->system;
			dst_log->requests = src_log->requests;
			dst_log->frames = src_log->frames;
			dst_log->parsing = src_log->parsing;

			//connections
			ConnOptions* dst_conn = &dst_mc->conn_options;
			ConnOptions* src_conn = &src_mc->conn_options;
			dst_conn->connection_type = src_conn->connection_type;
			if (dst_conn->port != NULL)
			{
				g_free(dst_conn->port);
				dst_conn->port = NULL;
			}
			if(src_conn->port!=NULL)
			{
				dst_conn->port= g_strdup(src_conn->port);
			}
			if (dst_conn->ip_address != NULL)
			{
				g_free(dst_conn->ip_address);
				dst_conn->ip_address = NULL;
			}
			if(src_conn->ip_address!=NULL)
			{
				dst_conn->ip_address= g_strdup(src_conn->ip_address);
			}
			dst_conn->ip_port = src_conn->ip_port;

			//timeouts
			TimeoutOptions* dst_to =  &dst_mc->timeout_options;
			TimeoutOptions* src_to =  &src_mc->timeout_options;
			dst_to->t_read = src_to->t_read;
			dst_to->t_write = src_to->t_write;

			//mapping
			dst_mc->tank_count = src_mc->tank_count;

			if (dst_mc->tank_count > 0)
			{
				for (guint8 j = 0; j < dst_mc->tank_count; j++)
				{
					dst_mc->tanks[j].num = src_mc->tanks[j].num;
					dst_mc->tanks[j].channel = src_mc->tanks[j].channel;
				}
			}
		}
	}

	result->fiscal_register_count = configuration.devices.fiscal_register_count;

	if (result->fiscal_register_count > 0)
	{
		for (guint8 i = 0; i < result->fiscal_register_count; i++)
		{
			FiscalRegisterConf* dst_fr_conf = &result->fiscal_registers[i];
			FiscalRegisterConf* src_fr_conf = &configuration.devices.fiscal_registers[i];

			dst_fr_conf->id = src_fr_conf->id;

			if (dst_fr_conf->name != NULL)
			{
				g_free(dst_fr_conf->name);
				dst_fr_conf->name = NULL;
			}
			if(src_fr_conf->name!=NULL)
			{
				dst_fr_conf->name = g_strdup(src_fr_conf->name);
			}

			dst_fr_conf->port = src_fr_conf->port;
			dst_fr_conf->enable = src_fr_conf->enable;
			dst_fr_conf->command_timeout = src_fr_conf->command_timeout;
			dst_fr_conf->interval = src_fr_conf->interval;

			if (dst_fr_conf->module_name != NULL)
			{
				g_free(dst_fr_conf->module_name);
				dst_fr_conf->module_name = NULL;
			}
			if(src_fr_conf->module_name!=NULL)
			{
				dst_fr_conf->module_name = g_strdup(src_fr_conf->module_name);
			}

			if (dst_fr_conf->log_dir != NULL)
			{
				g_free(dst_fr_conf->log_dir);
				dst_fr_conf->log_dir = NULL;
			}
			if(src_fr_conf->log_dir!=NULL)
			{
				dst_fr_conf->log_dir = g_strdup(src_fr_conf->log_dir);
			}

			dst_fr_conf->log_enable = src_fr_conf->log_enable;
			dst_fr_conf->log_trace = src_fr_conf->log_trace;

			dst_fr_conf->file_size = src_fr_conf->file_size;
			dst_fr_conf->save_days = src_fr_conf->save_days;


			//module config
			FRLibConfig* dst_mc = &dst_fr_conf->module_config;
			FRLibConfig* src_mc = &src_fr_conf->module_config;

			//log config
			LibLogOptions* dst_log = &dst_mc->log_options;
			LibLogOptions* src_log = &src_mc->log_options;
			dst_log->enable = src_log->enable;
			if (dst_log->dir != NULL)
			{
				g_free(dst_log->dir);
				dst_log->dir = NULL;
			}
			if(src_log->dir!=NULL)
			{
				dst_log->dir= g_strdup(src_log->dir);
			}

			dst_log->file_size = src_log->file_size;
			dst_log->save_days = src_log->save_days;

			dst_log->trace = src_log->trace;
			dst_log->system = src_log->system;
			dst_log->requests = src_log->requests;
			dst_log->frames = src_log->frames;
			dst_log->parsing = src_log->parsing;

			//connections
			ConnOptions* dst_conn = &dst_mc->conn_options;
			ConnOptions* src_conn = &src_mc->conn_options;
			dst_conn->connection_type = src_conn->connection_type;
			if (dst_conn->port != NULL)
			{
				g_free(dst_conn->port);
				dst_conn->port = NULL;
			}
			if(src_conn->port!=NULL)
			{
				dst_conn->port= g_strdup(src_conn->port);
			}
			if (dst_conn->ip_address != NULL)
			{
				g_free(dst_conn->ip_address);
				dst_conn->ip_address = NULL;
			}
			if(src_conn->ip_address!=NULL)
			{
				dst_conn->ip_address= g_strdup(src_conn->ip_address);
			}
			dst_conn->ip_port = src_conn->ip_port;

			//timeouts
			TimeoutOptions* dst_to =  &dst_mc->timeout_options;
			TimeoutOptions* src_to =  &src_mc->timeout_options;
			dst_to->t_read = src_to->t_read;
			dst_to->t_write = src_to->t_write;

			dst_mc->protocol_type = src_mc->protocol_type;
			dst_mc->auto_drawer = src_mc->auto_drawer;
			dst_mc->auto_cutting = src_mc->auto_cutting;
			dst_mc->cash_num = src_mc->cash_num;
			dst_mc->bn_num = src_mc->bn_num;
			dst_mc->bonus_num = src_mc->bonus_num;
			dst_mc->time_sync = src_mc->time_sync;

		}
	}

	result->price_pole_controller_count = configuration.devices.price_pole_controller_count;

	if (result->price_pole_controller_count > 0)
	{
		for (guint8 i = 0; i < result->price_pole_controller_count; i++)
		{
			PpcConf* dst_ppc_conf = &result->price_pole_controllers[i];
			PpcConf* src_ppc_conf = &configuration.devices.price_pole_controllers[i];

			dst_ppc_conf->id = src_ppc_conf->id;

			if (dst_ppc_conf->name != NULL)
			{
				g_free(dst_ppc_conf->name);
				dst_ppc_conf->name = NULL;
			}
			if(src_ppc_conf->name!=NULL)
			{
				dst_ppc_conf->name = g_strdup(src_ppc_conf->name);
			}

			dst_ppc_conf->port = src_ppc_conf->port;
			dst_ppc_conf->enable = src_ppc_conf->enable;
			dst_ppc_conf->command_timeout = src_ppc_conf->command_timeout;
			dst_ppc_conf->interval = src_ppc_conf->interval;

			if (dst_ppc_conf->module_name != NULL)
			{
				g_free(dst_ppc_conf->module_name);
				dst_ppc_conf->module_name = NULL;
			}
			if(src_ppc_conf->module_name!=NULL)
			{
				dst_ppc_conf->module_name = g_strdup(src_ppc_conf->module_name);
			}

			if (dst_ppc_conf->log_dir != NULL)
			{
				g_free(dst_ppc_conf->log_dir);
				dst_ppc_conf->log_dir = NULL;
			}
			if(src_ppc_conf->log_dir!=NULL)
			{
				dst_ppc_conf->log_dir = g_strdup(src_ppc_conf->log_dir);
			}

			dst_ppc_conf->log_enable = src_ppc_conf->log_enable;
			dst_ppc_conf->log_trace = src_ppc_conf->log_trace;

			//module config
			PPCLibConfig* dst_mc = &dst_ppc_conf->module_config;
			PPCLibConfig* src_mc = &src_ppc_conf->module_config;

			//log config
			LibLogOptions* dst_log = &dst_mc->log_options;
			LibLogOptions* src_log = &src_mc->log_options;
			dst_log->enable = src_log->enable;
			if (dst_log->dir != NULL)
			{
				g_free(dst_log->dir);
				dst_log->dir = NULL;
			}
			if(src_log->dir!=NULL)
			{
				dst_log->dir= g_strdup(src_log->dir);
			}
			dst_log->trace = src_log->trace;
			dst_log->system = src_log->system;
			dst_log->requests = src_log->requests;
			dst_log->frames = src_log->frames;
			dst_log->parsing = src_log->parsing;

			//connections
			ConnOptions* dst_conn = &dst_mc->conn_options;
			ConnOptions* src_conn = &src_mc->conn_options;
			dst_conn->connection_type = src_conn->connection_type;
			if (dst_conn->port != NULL)
			{
				g_free(dst_conn->port);
				dst_conn->port = NULL;
			}
			if(src_conn->port!=NULL)
			{
				dst_conn->port= g_strdup(src_conn->port);
			}
			if (dst_conn->ip_address != NULL)
			{
				g_free(dst_conn->ip_address);
				dst_conn->ip_address = NULL;
			}
			if(src_conn->ip_address!=NULL)
			{
				dst_conn->ip_address= g_strdup(src_conn->ip_address);
			}
			dst_conn->ip_port = src_conn->ip_port;

			//timeouts
			TimeoutOptions* dst_to =  &dst_mc->timeout_options;
			TimeoutOptions* src_to =  &src_mc->timeout_options;
			dst_to->t_read = src_to->t_read;
			dst_to->t_write = src_to->t_write;

			//decimal points
			PPCDecimalPointOptions* dst_dpo = &dst_mc->decimal_point_options;
			PPCDecimalPointOptions* src_dpo = &src_mc->decimal_point_options;
			dst_dpo->dp_price = src_dpo->dp_price;

			//mapping
			dst_mc->price_pole_count = src_mc->price_pole_count;

			if (dst_mc->price_pole_count > 0)
			{
				for (guint8 j = 0; j < dst_mc->price_pole_count; j++)
				{
					dst_mc->price_poles[j].num = src_mc->price_poles[j].num;
					dst_mc->price_poles[j].grade = src_mc->price_poles[j].grade;
					dst_mc->price_poles[j].symbol_count = src_mc->price_poles[j].symbol_count;
				}
			}
		}
	}

	result->sensor_controller_count = configuration.devices.sensor_controller_count;

	if (result->sensor_controller_count > 0)
	{
		for (guint8 i = 0; i < result->sensor_controller_count; i++)
		{
			ScConf* dst_sc_conf = &result->sensor_controllers[i];
			ScConf* src_sc_conf = &configuration.devices.sensor_controllers[i];

			dst_sc_conf->id = src_sc_conf->id;

			if (dst_sc_conf->name != NULL)
			{
				g_free(dst_sc_conf->name);
				dst_sc_conf->name = NULL;
			}
			if(src_sc_conf->name!=NULL)
			{
				dst_sc_conf->name = g_strdup(src_sc_conf->name);
			}

			dst_sc_conf->port = src_sc_conf->port;
			dst_sc_conf->enable = src_sc_conf->enable;
			dst_sc_conf->command_timeout = src_sc_conf->command_timeout;
			dst_sc_conf->interval = src_sc_conf->interval;

			if (dst_sc_conf->module_name != NULL)
			{
				g_free(dst_sc_conf->module_name);
				dst_sc_conf->module_name = NULL;
			}
			if(src_sc_conf->module_name!=NULL)
			{
				dst_sc_conf->module_name = g_strdup(src_sc_conf->module_name);
			}

			if (dst_sc_conf->log_dir != NULL)
			{
				g_free(dst_sc_conf->log_dir);
				dst_sc_conf->log_dir = NULL;
			}
			if(src_sc_conf->log_dir!=NULL)
			{
				dst_sc_conf->log_dir = g_strdup(src_sc_conf->log_dir);
			}

			dst_sc_conf->log_enable = src_sc_conf->log_enable;
			dst_sc_conf->log_trace = src_sc_conf->log_trace;

			//module config
			SCLibConfig* dst_mc = &dst_sc_conf->module_config;
			SCLibConfig* src_mc = &src_sc_conf->module_config;

			//log config
			LibLogOptions* dst_log = &dst_mc->log_options;
			LibLogOptions* src_log = &src_mc->log_options;
			dst_log->enable = src_log->enable;
			if (dst_log->dir != NULL)
			{
				g_free(dst_log->dir);
				dst_log->dir = NULL;
			}
			if(src_log->dir!=NULL)
			{
				dst_log->dir= g_strdup(src_log->dir);
			}
			dst_log->trace = src_log->trace;
			dst_log->system = src_log->system;
			dst_log->requests = src_log->requests;
			dst_log->frames = src_log->frames;
			dst_log->parsing = src_log->parsing;

			//connections
			ConnOptions* dst_conn = &dst_mc->conn_options;
			ConnOptions* src_conn = &src_mc->conn_options;
			dst_conn->connection_type = src_conn->connection_type;
			if (dst_conn->port != NULL)
			{
				g_free(dst_conn->port);
				dst_conn->port = NULL;
			}
			if(src_conn->port!=NULL)
			{
				dst_conn->port= g_strdup(src_conn->port);
			}
			if (dst_conn->ip_address != NULL)
			{
				g_free(dst_conn->ip_address);
				dst_conn->ip_address = NULL;
			}
			if(src_conn->ip_address!=NULL)
			{
				dst_conn->ip_address= g_strdup(src_conn->ip_address);
			}
			dst_conn->ip_port = src_conn->ip_port;

			//timeouts
			TimeoutOptions* dst_to =  &dst_mc->timeout_options;
			TimeoutOptions* src_to =  &src_mc->timeout_options;
			dst_to->t_read = src_to->t_read;
			dst_to->t_write = src_to->t_write;

			//mapping
			dst_mc->sensor_count = src_mc->sensor_count;

			if (dst_mc->sensor_count > 0)
			{
				for (guint8 j = 0; j < dst_mc->sensor_count; j++)
				{
					dst_mc->sensors[j].num = src_mc->sensors[j].num;
					dst_mc->sensors[j].addr = src_mc->sensors[j].addr;

					dst_mc->sensors[j].param_count = src_mc->sensors[j].param_count;

					if (dst_mc->sensors[j].param_count > 0)
					{
						for(guint8 k = 0; k < dst_mc->sensors[j].param_count; k++)
						{
							dst_mc->sensors[j].params[k].num = src_mc->sensors[j].params[k].num;
							dst_mc->sensors[j].params[k].type = src_mc->sensors[j].params[k].type;
						}
					}

				}
			}
		}
	}


	g_mutex_unlock(&configuration_mutex);
}

void get_profiles_conf( ProfilesConf* result)
{
	g_mutex_lock(&configuration_mutex);

	result->profiles_count = configuration.profiles_conf.profiles_count;

	if (result->profiles_count > 0)
	{
		for (guint8 i = 0; i < result->profiles_count; i++)
		{
			Profile* dst = &result->profiles[i];
			Profile* src = &configuration.profiles_conf.profiles[i];

			dst->id = src->id;

			if (dst->name != NULL)
			{
				g_free(dst->name);
				dst->name = NULL;
			}
			if(src->name!=NULL)
			{
				dst->name = g_strdup(src->name);
			}

			dst->enable = src->enable;

			if (dst->guid != NULL)
			{
				g_free(dst->guid);
				dst->guid = NULL;
			}
			if(src->guid!=NULL)
			{
				dst->guid = g_strdup(src->guid);
			}
			dst->access_level = src->access_level;

		}
	}

	g_mutex_unlock(&configuration_mutex);

}

gboolean add_client_profile(Profile profile)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.profiles_conf.profiles_count < MAX_CLIENT_COUNT)
	{
		configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].id = profile.id;
		if(configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].name != NULL)
		{
			g_free(configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].name);
			configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].name = NULL;
		}
		if(profile.name != NULL)
		{
			configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].name = g_strdup(profile.name);
		}
		configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].enable = profile.enable;

		if(configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].guid != NULL)
		{
			g_free(configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].guid);
			configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].guid = NULL;
		}
		if(profile.guid != NULL)
		{
			configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].guid = g_strdup(profile.guid);
		}
		configuration.profiles_conf.profiles[configuration.profiles_conf.profiles_count].access_level = profile.access_level;

		configuration.profiles_conf.profiles_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean update_client_profile(Profile profile)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.profiles_conf.profiles_count > 0)
	{
		for (guint8 i = 0; i < configuration.profiles_conf.profiles_count; i++)
		{
			if (configuration.profiles_conf.profiles[i].id == profile.id)
			{
				if(configuration.profiles_conf.profiles[i].name != NULL)
				{
					g_free(configuration.profiles_conf.profiles[i].name);
					configuration.profiles_conf.profiles[i].name = NULL;
				}
				if (profile.name!=NULL)
				{
					configuration.profiles_conf.profiles[i].name = g_strdup(profile.name);
				}
				configuration.profiles_conf.profiles[i].enable = profile.enable;

				if(configuration.profiles_conf.profiles[i].guid != NULL)
				{
					g_free(configuration.profiles_conf.profiles[i].guid);
					configuration.profiles_conf.profiles[i].guid = NULL;
				}
				if (profile.guid!=NULL)
				{
				configuration.profiles_conf.profiles[i].guid = g_strdup(profile.guid);
				}
				configuration.profiles_conf.profiles[i].access_level = profile.access_level;

				result = TRUE;
			}
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

gboolean delete_client_profile(Profile client_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	gboolean id_found = FALSE;

	if (configuration.profiles_conf.profiles_count > 0)
	{
		for (guint8 i = 0; i < configuration.profiles_conf.profiles_count; i++)
		{
			if (configuration.profiles_conf.profiles[i].id == client_conf.id || id_found == TRUE)
			{
				id_found = TRUE;

				if (i == configuration.profiles_conf.profiles_count - 1)
				{
					configuration.profiles_conf.profiles[i].id = 0;
					if(configuration.profiles_conf.profiles[i].name!=NULL)
					{
						g_free(configuration.profiles_conf.profiles[i].name);
						configuration.profiles_conf.profiles[i].name = NULL;
					}
					if(configuration.profiles_conf.profiles[i].guid!=NULL)
					{
						g_free(configuration.profiles_conf.profiles[i].guid);
						configuration.profiles_conf.profiles[i].guid = NULL;
					}
					configuration.profiles_conf.profiles[i].enable = FALSE;
					configuration.profiles_conf.profiles[i].access_level = 0;


					configuration.profiles_conf.profiles_count--;
					break;
				}
				else
				{
					configuration.profiles_conf.profiles[i].id = configuration.profiles_conf.profiles[i+1].id;
					if(configuration.profiles_conf.profiles[i].name!=NULL)
					{
						g_free(configuration.profiles_conf.profiles[i].name);
						configuration.profiles_conf.profiles[i].name = NULL;
					}
					if(configuration.profiles_conf.profiles[i+1].guid!=NULL)
					{
						configuration.profiles_conf.profiles[i].guid = g_strdup(configuration.profiles_conf.profiles[i+1].guid);
					}
					configuration.profiles_conf.profiles[i].enable = configuration.profiles_conf.profiles[i+1].enable;
					configuration.profiles_conf.profiles[i].access_level = configuration.profiles_conf.profiles[i+1].access_level;
				}
			}
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean add_dc_conf(DispencerControllerConf dc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.dispencer_controller_count < MAX_DEVICE_COUNT)
	{
		DispencerControllerConf* dest = &configuration.devices.dispencer_controllers[configuration.devices.dispencer_controller_count];

		dest->index = configuration.devices.dispencer_controller_count;

		dest->id = dc_conf.id;

		if(dest->name != NULL)
		{
			g_free(dest->name);
			dest->name = NULL;
		}
		if (dc_conf.name!=NULL)
		{
			dest->name = g_strdup(dc_conf.name);
		}
		dest->port = dc_conf.port;
		dest->enable = dc_conf.enable;
		dest->command_timeout = dc_conf.command_timeout;
		dest->interval = dc_conf.interval;

		if(dest->module_name != NULL)
		{
			g_free(dest->module_name);
			dest->module_name = NULL;
		}
		if (dc_conf.module_name!=NULL)
		{
			dest->module_name = g_strdup(dc_conf.module_name);
		}

		if(dest->log_dir != NULL)
		{
			g_free(dest->log_dir);
			dest->log_dir = NULL;
		}
		if (dc_conf.log_dir!=NULL)
		{
			dest->log_dir = g_strdup(dc_conf.log_dir);
		}
		dest->log_enable = dc_conf.log_enable;
		dest->log_trace = dc_conf.log_trace;

		DCLibConfig* module_config = &dest->module_config;

		LibLogOptions* log_options = &module_config->log_options;

		log_options->enable = dc_conf.module_config.log_options.enable;

		if(log_options->dir != NULL)
		{
			g_free(log_options->dir);
			log_options->dir = NULL;
		}
		if (dc_conf.module_config.log_options.dir!=NULL)
		{
			log_options->dir = g_strdup(dc_conf.module_config.log_options.dir);
		}

		log_options->file_size = dc_conf.module_config.log_options.file_size;
		log_options->save_days = dc_conf.module_config.log_options.save_days;

		log_options->trace = dc_conf.module_config.log_options.trace;
		log_options->system = dc_conf.module_config.log_options.system;
		log_options->requests = dc_conf.module_config.log_options.requests;
		log_options->frames = dc_conf.module_config.log_options.frames;
		log_options->parsing = dc_conf.module_config.log_options.parsing;

		ConnOptions* conn_options = &module_config->conn_options;

		conn_options->connection_type = dc_conf.module_config.conn_options.connection_type;

		if(conn_options->port != NULL)
		{
			g_free(conn_options->port);
			conn_options->port = NULL;
		}
		if (dc_conf.module_config.conn_options.port!=NULL)
		{
			conn_options->port = g_strdup(dc_conf.module_config.conn_options.port);
		}

		if(conn_options->ip_address != NULL)
		{
			g_free(conn_options->ip_address);
			conn_options->ip_address = NULL;
		}
		if (dc_conf.module_config.conn_options.ip_address!=NULL)
		{
			conn_options->ip_address = g_strdup(dc_conf.module_config.conn_options.ip_address);
		}
		conn_options->port = dc_conf.module_config.conn_options.port;

		TimeoutOptions* timeout_options = &module_config->timeout_options;

		timeout_options->t_read = dc_conf.module_config.timeout_options.t_read;
		timeout_options->t_write = dc_conf.module_config.timeout_options.t_write;


		DCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

		dp_options->dp_price = dc_conf.module_config.decimal_point_options.dp_price;
		dp_options->dp_volume = dc_conf.module_config.decimal_point_options.dp_volume;
		dp_options->dp_amount = dc_conf.module_config.decimal_point_options.dp_amount;

		module_config->counters_enable = dc_conf.module_config.counters_enable;
		module_config->auto_start = dc_conf.module_config.auto_start;
		module_config->auto_payment = dc_conf.module_config.auto_payment;
		module_config->full_tank_volume = dc_conf.module_config.full_tank_volume;

		module_config->dispencer_count = dc_conf.module_config.dispencer_count;

		if (module_config->dispencer_count > 0)
		{
			for (guint8 i = 0; i < module_config->dispencer_count; i++)
			{
				DispencerConf* disp_conf = &module_config->dispencers[i];

				disp_conf->num = dc_conf.module_config.dispencers[i].num;
				disp_conf->addr = dc_conf.module_config.dispencers[i].addr;
				disp_conf->nozzle_count = dc_conf.module_config.dispencers[i].nozzle_count;

				if (disp_conf->nozzle_count > 0)
				{
					for (guint8 j = 0; j < disp_conf->nozzle_count; j++)
					{
						NozzleConf* nozzle_conf = &disp_conf->nozzles[j];

						nozzle_conf->num = dc_conf.module_config.dispencers[i].nozzles[j].num;
						nozzle_conf->grade = dc_conf.module_config.dispencers[i].nozzles[j].grade;
					}
				}
			}
		}

		dest->changed = TRUE;

		configuration.devices.dispencer_controller_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean add_tgs_conf(TgsConf tgs_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.tgs_count < MAX_DEVICE_COUNT)
	{
		TgsConf* dest = &configuration.devices.tgs[configuration.devices.tgs_count];

		dest->index = configuration.devices.tgs_count;

		dest->id = tgs_conf.id;

		if(dest->name != NULL)
		{
			g_free(dest->name);
			dest->name = NULL;
		}
		if (tgs_conf.name!=NULL)
		{
			dest->name = g_strdup(tgs_conf.name);
		}
		dest->port = tgs_conf.port;
		dest->enable = tgs_conf.enable;
		dest->command_timeout = tgs_conf.command_timeout;
		dest->interval = tgs_conf.interval;

		if(dest->module_name != NULL)
		{
			g_free(dest->module_name);
			dest->module_name = NULL;
		}
		if (tgs_conf.module_name!=NULL)
		{
			dest->module_name = g_strdup(tgs_conf.module_name);
		}

		if(dest->log_dir != NULL)
		{
			g_free(dest->log_dir);
			dest->log_dir = NULL;
		}
		if (tgs_conf.log_dir!=NULL)
		{
			dest->log_dir = g_strdup(tgs_conf.log_dir);
		}
		dest->log_enable = tgs_conf.log_enable;
		dest->log_trace = tgs_conf.log_trace;

		TGSLibConfig* module_config = &dest->module_config;

		LibLogOptions* log_options = &module_config->log_options;

		log_options->enable = tgs_conf.module_config.log_options.enable;

		if(log_options->dir != NULL)
		{
			g_free(log_options->dir);
			log_options->dir = NULL;
		}
		if (tgs_conf.module_config.log_options.dir!=NULL)
		{
			log_options->dir = g_strdup(tgs_conf.module_config.log_options.dir);
		}

		log_options->file_size = tgs_conf.module_config.log_options.file_size;
		log_options->save_days = tgs_conf.module_config.log_options.save_days;

		log_options->trace = tgs_conf.module_config.log_options.trace;
		log_options->system = tgs_conf.module_config.log_options.system;
		log_options->requests = tgs_conf.module_config.log_options.requests;
		log_options->frames = tgs_conf.module_config.log_options.frames;
		log_options->parsing = tgs_conf.module_config.log_options.parsing;

		ConnOptions* conn_options = &module_config->conn_options;

		conn_options->connection_type = tgs_conf.module_config.conn_options.connection_type;

		if(conn_options->port != NULL)
		{
			g_free(conn_options->port);
			conn_options->port = NULL;
		}
		if (tgs_conf.module_config.conn_options.port!=NULL)
		{
			conn_options->port = g_strdup(tgs_conf.module_config.conn_options.port);
		}

		if(conn_options->ip_address != NULL)
		{
			g_free(conn_options->ip_address);
			conn_options->ip_address = NULL;
		}
		if (tgs_conf.module_config.conn_options.ip_address!=NULL)
		{
			conn_options->ip_address = g_strdup(tgs_conf.module_config.conn_options.ip_address);
		}
		conn_options->port = tgs_conf.module_config.conn_options.port;

		TimeoutOptions* timeout_options = &module_config->timeout_options;

		timeout_options->t_read = tgs_conf.module_config.timeout_options.t_read;
		timeout_options->t_write = tgs_conf.module_config.timeout_options.t_write;

		module_config->tank_count = tgs_conf.module_config.tank_count;

		if (module_config->tank_count > 0)
		{
			for (guint8 i = 0; i < module_config->tank_count; i++)
			{
				TankConf* tank_conf = &module_config->tanks[i];

				tank_conf->num = tgs_conf.module_config.tanks[i].num;
				tank_conf->channel = tgs_conf.module_config.tanks[i].channel;
			}
		}

		dest->changed = TRUE;

		configuration.devices.tgs_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean add_fr_conf(FiscalRegisterConf fr_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.fiscal_register_count < MAX_DEVICE_COUNT)
	{
		FiscalRegisterConf* dest = &configuration.devices.fiscal_registers[configuration.devices.fiscal_register_count];

		dest->index = configuration.devices.fiscal_register_count;

		dest->id = fr_conf.id;

		if(dest->name != NULL)
		{
			g_free(dest->name);
			dest->name = NULL;
		}
		if (fr_conf.name!=NULL)
		{
			dest->name = g_strdup(fr_conf.name);
		}
		dest->port = fr_conf.port;
		dest->enable = fr_conf.enable;
		dest->command_timeout = fr_conf.command_timeout;
		dest->interval = fr_conf.interval;

		if(dest->module_name != NULL)
		{
			g_free(dest->module_name);
			dest->module_name = NULL;
		}
		if (fr_conf.module_name!=NULL)
		{
			dest->module_name = g_strdup(fr_conf.module_name);
		}

		if(dest->log_dir != NULL)
		{
			g_free(dest->log_dir);
			dest->log_dir = NULL;
		}
		if (fr_conf.log_dir!=NULL)
		{
			dest->log_dir = g_strdup(fr_conf.log_dir);
		}
		dest->log_enable = fr_conf.log_enable;
		dest->log_trace = fr_conf.log_trace;

		FRLibConfig* module_config = &dest->module_config;

		LibLogOptions* log_options = &module_config->log_options;

		log_options->enable = fr_conf.module_config.log_options.enable;

		if(log_options->dir != NULL)
		{
			g_free(log_options->dir);
			log_options->dir = NULL;
		}
		if (fr_conf.module_config.log_options.dir!=NULL)
		{
			log_options->dir = g_strdup(fr_conf.module_config.log_options.dir);
		}

		log_options->file_size = fr_conf.module_config.log_options.file_size;
		log_options->save_days = fr_conf.module_config.log_options.save_days;

		log_options->trace = fr_conf.module_config.log_options.trace;
		log_options->system = fr_conf.module_config.log_options.system;
		log_options->requests = fr_conf.module_config.log_options.requests;
		log_options->frames = fr_conf.module_config.log_options.frames;
		log_options->parsing = fr_conf.module_config.log_options.parsing;

		ConnOptions* conn_options = &module_config->conn_options;

		conn_options->connection_type = fr_conf.module_config.conn_options.connection_type;

		if(conn_options->port != NULL)
		{
			g_free(conn_options->port);
			conn_options->port = NULL;
		}
		if (fr_conf.module_config.conn_options.port!=NULL)
		{
			conn_options->port = g_strdup(fr_conf.module_config.conn_options.port);
		}

		if(conn_options->ip_address != NULL)
		{
			g_free(conn_options->ip_address);
			conn_options->ip_address = NULL;
		}
		if (fr_conf.module_config.conn_options.ip_address!=NULL)
		{
			conn_options->ip_address = g_strdup(fr_conf.module_config.conn_options.ip_address);
		}
		conn_options->port = fr_conf.module_config.conn_options.port;

		TimeoutOptions* timeout_options = &module_config->timeout_options;

		timeout_options->t_read = fr_conf.module_config.timeout_options.t_read;
		timeout_options->t_write = fr_conf.module_config.timeout_options.t_write;

		module_config->protocol_type = fr_conf.module_config.protocol_type;
		module_config->auto_drawer = fr_conf.module_config.auto_drawer;
		module_config->auto_cutting = fr_conf.module_config.auto_cutting;
		module_config->cash_num = fr_conf.module_config.cash_num;
		module_config->bn_num = fr_conf.module_config.bn_num;
		module_config->bonus_num = fr_conf.module_config.bonus_num;
		module_config->time_sync = fr_conf.module_config.time_sync;

		dest->changed = TRUE;

		configuration.devices.fiscal_register_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean add_ppc_conf(PpcConf ppc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.price_pole_controller_count < MAX_DEVICE_COUNT)
	{
		PpcConf* dest = &configuration.devices.price_pole_controllers[configuration.devices.price_pole_controller_count];

		dest->index = configuration.devices.price_pole_controller_count;

		dest->id = ppc_conf.id;

		if(dest->name != NULL)
		{
			g_free(dest->name);
			dest->name = NULL;
		}
		if (ppc_conf.name!=NULL)
		{
			dest->name = g_strdup(ppc_conf.name);
		}
		dest->port = ppc_conf.port;
		dest->enable = ppc_conf.enable;
		dest->command_timeout = ppc_conf.command_timeout;
		dest->interval = ppc_conf.interval;

		if(dest->module_name != NULL)
		{
			g_free(dest->module_name);
			dest->module_name = NULL;
		}
		if (ppc_conf.module_name!=NULL)
		{
			dest->module_name = g_strdup(ppc_conf.module_name);
		}

		if(dest->log_dir != NULL)
		{
			g_free(dest->log_dir);
			dest->log_dir = NULL;
		}
		if (ppc_conf.log_dir!=NULL)
		{
			dest->log_dir = g_strdup(ppc_conf.log_dir);
		}
		dest->log_enable = ppc_conf.log_enable;
		dest->log_trace = ppc_conf.log_trace;

		PPCLibConfig* module_config = &dest->module_config;

		LibLogOptions* log_options = &module_config->log_options;

		log_options->enable = ppc_conf.module_config.log_options.enable;

		if(log_options->dir != NULL)
		{
			g_free(log_options->dir);
			log_options->dir = NULL;
		}
		if (ppc_conf.module_config.log_options.dir!=NULL)
		{
			log_options->dir = g_strdup(ppc_conf.module_config.log_options.dir);
		}

		log_options->file_size = ppc_conf.module_config.log_options.file_size;
		log_options->save_days = ppc_conf.module_config.log_options.save_days;

		log_options->trace = ppc_conf.module_config.log_options.trace;
		log_options->system = ppc_conf.module_config.log_options.system;
		log_options->requests = ppc_conf.module_config.log_options.requests;
		log_options->frames = ppc_conf.module_config.log_options.frames;
		log_options->parsing = ppc_conf.module_config.log_options.parsing;

		ConnOptions* conn_options = &module_config->conn_options;

		conn_options->connection_type = ppc_conf.module_config.conn_options.connection_type;

		if(conn_options->port != NULL)
		{
			g_free(conn_options->port);
			conn_options->port = NULL;
		}
		if (ppc_conf.module_config.conn_options.port!=NULL)
		{
			conn_options->port = g_strdup(ppc_conf.module_config.conn_options.port);
		}

		if(conn_options->ip_address != NULL)
		{
			g_free(conn_options->ip_address);
			conn_options->ip_address = NULL;
		}
		if (ppc_conf.module_config.conn_options.ip_address!=NULL)
		{
			conn_options->ip_address = g_strdup(ppc_conf.module_config.conn_options.ip_address);
		}
		conn_options->port = ppc_conf.module_config.conn_options.port;

		TimeoutOptions* timeout_options = &module_config->timeout_options;

		timeout_options->t_read = ppc_conf.module_config.timeout_options.t_read;
		timeout_options->t_write = ppc_conf.module_config.timeout_options.t_write;

		PPCDecimalPointOptions* dp_options = &module_config->decimal_point_options;
		dp_options->dp_price = ppc_conf.module_config.decimal_point_options.dp_price;


		module_config->price_pole_count = ppc_conf.module_config.price_pole_count;

		if (module_config->price_pole_count > 0)
		{
			for (guint8 i = 0; i < module_config->price_pole_count; i++)
			{
				PricePoleConf* price_pole_conf = &module_config->price_poles[i];

				price_pole_conf->num = ppc_conf.module_config.price_poles[i].num;
				price_pole_conf->grade = ppc_conf.module_config.price_poles[i].grade;
				price_pole_conf->symbol_count = ppc_conf.module_config.price_poles[i].symbol_count;
			}
		}

		dest->changed = TRUE;

		configuration.devices.price_pole_controller_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean add_sc_conf(ScConf sc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.sensor_controller_count < MAX_DEVICE_COUNT)
	{
		ScConf* dest = &configuration.devices.sensor_controllers[configuration.devices.sensor_controller_count];

		dest->index = configuration.devices.sensor_controller_count;

		dest->id = sc_conf.id;

		if(dest->name != NULL)
		{
			g_free(dest->name);
			dest->name = NULL;
		}
		if (sc_conf.name!=NULL)
		{
			dest->name = g_strdup(sc_conf.name);
		}
		dest->port = sc_conf.port;
		dest->enable = sc_conf.enable;
		dest->command_timeout = sc_conf.command_timeout;
		dest->interval = sc_conf.interval;

		if(dest->module_name != NULL)
		{
			g_free(dest->module_name);
			dest->module_name = NULL;
		}
		if (sc_conf.module_name!=NULL)
		{
			dest->module_name = g_strdup(sc_conf.module_name);
		}

		if(dest->log_dir != NULL)
		{
			g_free(dest->log_dir);
			dest->log_dir = NULL;
		}
		if (sc_conf.log_dir!=NULL)
		{
			dest->log_dir = g_strdup(sc_conf.log_dir);
		}
		dest->log_enable = sc_conf.log_enable;
		dest->log_trace = sc_conf.log_trace;

		SCLibConfig* module_config = &dest->module_config;

		LibLogOptions* log_options = &module_config->log_options;

		log_options->enable = sc_conf.module_config.log_options.enable;

		if(log_options->dir != NULL)
		{
			g_free(log_options->dir);
			log_options->dir = NULL;
		}
		if (sc_conf.module_config.log_options.dir!=NULL)
		{
			log_options->dir = g_strdup(sc_conf.module_config.log_options.dir);
		}

		log_options->file_size = sc_conf.module_config.log_options.file_size;
		log_options->save_days = sc_conf.module_config.log_options.save_days;

		log_options->trace = sc_conf.module_config.log_options.trace;
		log_options->system = sc_conf.module_config.log_options.system;
		log_options->requests = sc_conf.module_config.log_options.requests;
		log_options->frames = sc_conf.module_config.log_options.frames;
		log_options->parsing = sc_conf.module_config.log_options.parsing;

		ConnOptions* conn_options = &module_config->conn_options;

		conn_options->connection_type = sc_conf.module_config.conn_options.connection_type;

		if(conn_options->port != NULL)
		{
			g_free(conn_options->port);
			conn_options->port = NULL;
		}
		if (sc_conf.module_config.conn_options.port!=NULL)
		{
			conn_options->port = g_strdup(sc_conf.module_config.conn_options.port);
		}

		if(conn_options->ip_address != NULL)
		{
			g_free(conn_options->ip_address);
			conn_options->ip_address = NULL;
		}
		if (sc_conf.module_config.conn_options.ip_address!=NULL)
		{
			conn_options->ip_address = g_strdup(sc_conf.module_config.conn_options.ip_address);
		}
		conn_options->port = sc_conf.module_config.conn_options.port;

		TimeoutOptions* timeout_options = &module_config->timeout_options;

		timeout_options->t_read = sc_conf.module_config.timeout_options.t_read;
		timeout_options->t_write = sc_conf.module_config.timeout_options.t_write;

		module_config->sensor_count = sc_conf.module_config.sensor_count;

		if (module_config->sensor_count > 0)
		{
			for (guint8 i = 0; i < module_config->sensor_count; i++)
			{
				SensorConf* sensor_conf = &module_config->sensors[i];

				sensor_conf->num = sc_conf.module_config.sensors[i].num;
				sensor_conf->addr = sc_conf.module_config.sensors[i].addr;
				sensor_conf->param_count = sc_conf.module_config.sensors[i].param_count;

				if (sensor_conf->param_count > 0)
				{
					for(guint8 k = 0; k < sensor_conf->param_count; k++)
					{
						sensor_conf->params[k].num = sc_conf.module_config.sensors[i].params[k].num;
						sensor_conf->params[k].type = sc_conf.module_config.sensors[i].params[k].type;
					}
				}
			}
		}

		dest->changed = TRUE;

		configuration.devices.sensor_controller_count++;

		result = TRUE;
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}


gboolean update_dc_conf(DispencerControllerConf dc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.dispencer_controller_count; i++)
	{
		if (configuration.devices.dispencer_controllers[i].id == dc_conf.id)
		{
			DispencerControllerConf* dest = &configuration.devices.dispencer_controllers[i];

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}
			if (dc_conf.name!=NULL)
			{
				dest->name = g_strdup(dc_conf.name);
			}
			dest->port = dc_conf.port;
			dest->enable = dc_conf.enable;
			dest->command_timeout = dc_conf.command_timeout;
			dest->interval = dc_conf.interval;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}
			if (dc_conf.module_name!=NULL)
			{
				dest->module_name = g_strdup(dc_conf.module_name);
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			if (dc_conf.log_dir!=NULL)
			{
				dest->log_dir = g_strdup(dc_conf.log_dir);
			}
			dest->log_enable = dc_conf.log_enable;
			dest->log_trace = dc_conf.log_trace;

			DCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = dc_conf.module_config.log_options.enable;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}
			if (dc_conf.module_config.log_options.dir!=NULL)
			{
				log_options->dir = g_strdup(dc_conf.module_config.log_options.dir);
			}

			log_options->file_size = dc_conf.module_config.log_options.file_size;
			log_options->save_days = dc_conf.module_config.log_options.save_days;

			log_options->trace = dc_conf.module_config.log_options.trace;
			log_options->system = dc_conf.module_config.log_options.system;
			log_options->requests = dc_conf.module_config.log_options.requests;
			log_options->frames = dc_conf.module_config.log_options.frames;
			log_options->parsing = dc_conf.module_config.log_options.parsing;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = dc_conf.module_config.conn_options.connection_type;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}
			if (dc_conf.module_config.conn_options.port!=NULL)
			{
				conn_options->port = g_strdup(dc_conf.module_config.conn_options.port);
			}

			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}
			if (dc_conf.module_config.conn_options.ip_address!=NULL)
			{
				conn_options->ip_address = g_strdup(dc_conf.module_config.conn_options.ip_address);
			}
			conn_options->port = dc_conf.module_config.conn_options.port;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = dc_conf.module_config.timeout_options.t_read;
			timeout_options->t_write = dc_conf.module_config.timeout_options.t_write;


			DCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

			dp_options->dp_price = dc_conf.module_config.decimal_point_options.dp_price;
			dp_options->dp_volume = dc_conf.module_config.decimal_point_options.dp_volume;
			dp_options->dp_amount = dc_conf.module_config.decimal_point_options.dp_amount;

			module_config->counters_enable = dc_conf.module_config.counters_enable;
			module_config->auto_start = dc_conf.module_config.auto_start;
			module_config->auto_payment = dc_conf.module_config.auto_payment;
			module_config->full_tank_volume = dc_conf.module_config.full_tank_volume;

			module_config->dispencer_count = dc_conf.module_config.dispencer_count;

			if (module_config->dispencer_count > 0)
			{
				for (guint8 i = 0; i < module_config->dispencer_count; i++)
				{
					DispencerConf* disp_conf = &module_config->dispencers[i];

					disp_conf->num = dc_conf.module_config.dispencers[i].num;
					disp_conf->addr = dc_conf.module_config.dispencers[i].addr;
					disp_conf->nozzle_count = dc_conf.module_config.dispencers[i].nozzle_count;

					if (disp_conf->nozzle_count > 0)
					{
						for (guint8 j = 0; j < disp_conf->nozzle_count; j++)
						{
							NozzleConf* nozzle_conf = &disp_conf->nozzles[j];

							nozzle_conf->num = dc_conf.module_config.dispencers[i].nozzles[j].num;
							nozzle_conf->grade = dc_conf.module_config.dispencers[i].nozzles[j].grade;
						}
					}
				}
			}

			dest->changed = TRUE;

			result = TRUE;

		}
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean update_tgs_conf(TgsConf tgs_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.tgs_count; i++)
	{
		if (configuration.devices.tgs[i].id == tgs_conf.id)
		{
			TgsConf* dest = &configuration.devices.tgs[i];

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}
			if (tgs_conf.name!=NULL)
			{
				dest->name = g_strdup(tgs_conf.name);
			}
			dest->port = tgs_conf.port;
			dest->enable = tgs_conf.enable;
			dest->command_timeout = tgs_conf.command_timeout;
			dest->interval = tgs_conf.interval;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}
			if (tgs_conf.module_name!=NULL)
			{
				dest->module_name = g_strdup(tgs_conf.module_name);
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			if (tgs_conf.log_dir!=NULL)
			{
				dest->log_dir = g_strdup(tgs_conf.log_dir);
			}
			dest->log_enable = tgs_conf.log_enable;
			dest->log_trace = tgs_conf.log_trace;

			TGSLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = tgs_conf.module_config.log_options.enable;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}
			if (tgs_conf.module_config.log_options.dir!=NULL)
			{
				log_options->dir = g_strdup(tgs_conf.module_config.log_options.dir);
			}

			log_options->file_size = tgs_conf.module_config.log_options.file_size;
			log_options->save_days = tgs_conf.module_config.log_options.save_days;

			log_options->trace = tgs_conf.module_config.log_options.trace;
			log_options->system = tgs_conf.module_config.log_options.system;
			log_options->requests = tgs_conf.module_config.log_options.requests;
			log_options->frames = tgs_conf.module_config.log_options.frames;
			log_options->parsing = tgs_conf.module_config.log_options.parsing;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = tgs_conf.module_config.conn_options.connection_type;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}
			if (tgs_conf.module_config.conn_options.port!=NULL)
			{
				conn_options->port = g_strdup(tgs_conf.module_config.conn_options.port);
			}

			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}
			if (tgs_conf.module_config.conn_options.ip_address!=NULL)
			{
				conn_options->ip_address = g_strdup(tgs_conf.module_config.conn_options.ip_address);
			}
			conn_options->port = tgs_conf.module_config.conn_options.port;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = tgs_conf.module_config.timeout_options.t_read;
			timeout_options->t_write = tgs_conf.module_config.timeout_options.t_write;

			module_config->tank_count = tgs_conf.module_config.tank_count;

			if (module_config->tank_count > 0)
			{
				for (guint8 i = 0; i < module_config->tank_count; i++)
				{
					TankConf* tank_conf = &module_config->tanks[i];

					tank_conf->num = tgs_conf.module_config.tanks[i].num;
					tank_conf->channel = tgs_conf.module_config.tanks[i].channel;
				}
			}

			dest->changed = TRUE;

			result = TRUE;

		}
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean update_fr_conf(FiscalRegisterConf fr_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.fiscal_register_count; i++)
	{
		if (configuration.devices.fiscal_registers[i].id == fr_conf.id)
		{
			FiscalRegisterConf* dest = &configuration.devices.fiscal_registers[i];

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}
			if (fr_conf.name!=NULL)
			{
				dest->name = g_strdup(fr_conf.name);
			}
			dest->port = fr_conf.port;
			dest->enable = fr_conf.enable;
			dest->command_timeout = fr_conf.command_timeout;
			dest->interval = fr_conf.interval;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}
			if (fr_conf.module_name!=NULL)
			{
				dest->module_name = g_strdup(fr_conf.module_name);
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			if (fr_conf.log_dir!=NULL)
			{
				dest->log_dir = g_strdup(fr_conf.log_dir);
			}
			dest->log_enable = fr_conf.log_enable;
			dest->log_trace = fr_conf.log_trace;

			FRLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = fr_conf.module_config.log_options.enable;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}
			if (fr_conf.module_config.log_options.dir!=NULL)
			{
				log_options->dir = g_strdup(fr_conf.module_config.log_options.dir);
			}

			log_options->file_size = fr_conf.module_config.log_options.file_size;
			log_options->save_days = fr_conf.module_config.log_options.save_days;

			log_options->trace = fr_conf.module_config.log_options.trace;
			log_options->system = fr_conf.module_config.log_options.system;
			log_options->requests = fr_conf.module_config.log_options.requests;
			log_options->frames = fr_conf.module_config.log_options.frames;
			log_options->parsing = fr_conf.module_config.log_options.parsing;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = fr_conf.module_config.conn_options.connection_type;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}
			if (fr_conf.module_config.conn_options.port!=NULL)
			{
				conn_options->port = g_strdup(fr_conf.module_config.conn_options.port);
			}

			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}
			if (fr_conf.module_config.conn_options.ip_address!=NULL)
			{
				conn_options->ip_address = g_strdup(fr_conf.module_config.conn_options.ip_address);
			}
			conn_options->port = fr_conf.module_config.conn_options.port;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = fr_conf.module_config.timeout_options.t_read;
			timeout_options->t_write = fr_conf.module_config.timeout_options.t_write;

			module_config->protocol_type = fr_conf.module_config.protocol_type;
			module_config->auto_drawer = fr_conf.module_config.auto_drawer;
			module_config->auto_cutting = fr_conf.module_config.auto_cutting;
			module_config->cash_num = fr_conf.module_config.cash_num;
			module_config->bn_num = fr_conf.module_config.bn_num;
			module_config->bonus_num = fr_conf.module_config.bonus_num;
			module_config->time_sync = fr_conf.module_config.time_sync;

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}


gboolean update_ppc_conf(PpcConf ppc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.price_pole_controller_count; i++)
	{
		if (configuration.devices.price_pole_controllers[i].id == ppc_conf.id)
		{
			PpcConf* dest = &configuration.devices.price_pole_controllers[i];

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}
			if (ppc_conf.name!=NULL)
			{
				dest->name = g_strdup(ppc_conf.name);
			}
			dest->port = ppc_conf.port;
			dest->enable = ppc_conf.enable;
			dest->command_timeout = ppc_conf.command_timeout;
			dest->interval = ppc_conf.interval;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}
			if (ppc_conf.module_name!=NULL)
			{
				dest->module_name = g_strdup(ppc_conf.module_name);
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			if (ppc_conf.log_dir!=NULL)
			{
				dest->log_dir = g_strdup(ppc_conf.log_dir);
			}
			dest->log_enable = ppc_conf.log_enable;
			dest->log_trace = ppc_conf.log_trace;

			PPCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = ppc_conf.module_config.log_options.enable;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}
			if (ppc_conf.module_config.log_options.dir!=NULL)
			{
				log_options->dir = g_strdup(ppc_conf.module_config.log_options.dir);
			}

			log_options->file_size = ppc_conf.module_config.log_options.file_size;
			log_options->save_days = ppc_conf.module_config.log_options.save_days;

			log_options->trace = ppc_conf.module_config.log_options.trace;
			log_options->system = ppc_conf.module_config.log_options.system;
			log_options->requests = ppc_conf.module_config.log_options.requests;
			log_options->frames = ppc_conf.module_config.log_options.frames;
			log_options->parsing = ppc_conf.module_config.log_options.parsing;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = ppc_conf.module_config.conn_options.connection_type;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}
			if (ppc_conf.module_config.conn_options.port!=NULL)
			{
				conn_options->port = g_strdup(ppc_conf.module_config.conn_options.port);
			}

			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}
			if (ppc_conf.module_config.conn_options.ip_address!=NULL)
			{
				conn_options->ip_address = g_strdup(ppc_conf.module_config.conn_options.ip_address);
			}
			conn_options->port = ppc_conf.module_config.conn_options.port;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = ppc_conf.module_config.timeout_options.t_read;
			timeout_options->t_write = ppc_conf.module_config.timeout_options.t_write;

			PPCDecimalPointOptions* dp_options = &module_config->decimal_point_options;
			dp_options->dp_price = ppc_conf.module_config.decimal_point_options.dp_price;

			module_config->price_pole_count = ppc_conf.module_config.price_pole_count;

			if (module_config->price_pole_count > 0)
			{
				for (guint8 i = 0; i < module_config->price_pole_count; i++)
				{
					PricePoleConf* price_pole_conf = &module_config->price_poles[i];

					price_pole_conf->num = ppc_conf.module_config.price_poles[i].num;
					price_pole_conf->grade = ppc_conf.module_config.price_poles[i].grade;
					price_pole_conf->symbol_count = ppc_conf.module_config.price_poles[i].symbol_count;
				}
			}

			dest->changed = TRUE;

			result = TRUE;

		}
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}

gboolean update_sc_conf(ScConf sc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.sensor_controller_count; i++)
	{
		if (configuration.devices.sensor_controllers[i].id == sc_conf.id)
		{
			ScConf* dest = &configuration.devices.sensor_controllers[i];

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}
			if (sc_conf.name!=NULL)
			{
				dest->name = g_strdup(sc_conf.name);
			}
			dest->port = sc_conf.port;
			dest->enable = sc_conf.enable;
			dest->command_timeout = sc_conf.command_timeout;
			dest->interval = sc_conf.interval;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}
			if (sc_conf.module_name!=NULL)
			{
				dest->module_name = g_strdup(sc_conf.module_name);
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			if (sc_conf.log_dir!=NULL)
			{
				dest->log_dir = g_strdup(sc_conf.log_dir);
			}
			dest->log_enable = sc_conf.log_enable;
			dest->log_trace = sc_conf.log_trace;

			SCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = sc_conf.module_config.log_options.enable;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}
			if (sc_conf.module_config.log_options.dir!=NULL)
			{
				log_options->dir = g_strdup(sc_conf.module_config.log_options.dir);
			}

			log_options->file_size = sc_conf.module_config.log_options.file_size;
			log_options->save_days = sc_conf.module_config.log_options.save_days;

			log_options->trace = sc_conf.module_config.log_options.trace;
			log_options->system = sc_conf.module_config.log_options.system;
			log_options->requests = sc_conf.module_config.log_options.requests;
			log_options->frames = sc_conf.module_config.log_options.frames;
			log_options->parsing = sc_conf.module_config.log_options.parsing;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = sc_conf.module_config.conn_options.connection_type;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}
			if (sc_conf.module_config.conn_options.port!=NULL)
			{
				conn_options->port = g_strdup(sc_conf.module_config.conn_options.port);
			}

			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}
			if (sc_conf.module_config.conn_options.ip_address!=NULL)
			{
				conn_options->ip_address = g_strdup(sc_conf.module_config.conn_options.ip_address);
			}
			conn_options->port = sc_conf.module_config.conn_options.port;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = sc_conf.module_config.timeout_options.t_read;
			timeout_options->t_write = sc_conf.module_config.timeout_options.t_write;

			module_config->sensor_count = sc_conf.module_config.sensor_count;

			if (module_config->sensor_count > 0)
			{
				for (guint8 i = 0; i < module_config->sensor_count; i++)
				{
					SensorConf* sensor_conf = &module_config->sensors[i];

					sensor_conf->num = sc_conf.module_config.sensors[i].num;
					sensor_conf->addr = sc_conf.module_config.sensors[i].addr;
					sensor_conf->param_count = sc_conf.module_config.sensors[i].param_count;

					if (sensor_conf->param_count > 0)
					{
						for(guint8 k = 0; k < sensor_conf->param_count; k++)
						{
							sensor_conf->params[k].num = sc_conf.module_config.sensors[i].params[k].num;
							sensor_conf->params[k].type = sc_conf.module_config.sensors[i].params[k].type;
						}
					}
				}
			}

			dest->changed = TRUE;

			result = TRUE;

		}
	}

	g_mutex_unlock(&configuration_mutex);
	return result;
}


gboolean delete_dc_conf(DispencerControllerConf dc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.dispencer_controller_count; i++)
	{
		if (configuration.devices.dispencer_controllers[i].id == dc_conf.id)
		{
			DispencerControllerConf* dest = &configuration.devices.dispencer_controllers[i];

			dest->id = 0;
			dest->index = 0;

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}

			dest->port = 0;
			dest->enable = FALSE;
			dest->command_timeout = 0;
			dest->interval = 0;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			dest->log_enable = FALSE;
			dest->log_trace = FALSE;

			DCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = FALSE;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}

			log_options->file_size = 0;
			log_options->save_days = 0;

			log_options->trace = FALSE;
			log_options->system = FALSE;
			log_options->requests = FALSE;
			log_options->frames = FALSE;
			log_options->parsing = FALSE;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = 0;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}


			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}

			conn_options->port = 0;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = 0;
			timeout_options->t_write = 0;

			DCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

			dp_options->dp_price = 0;
			dp_options->dp_volume = 0;
			dp_options->dp_amount = 0;

			module_config->counters_enable = FALSE;
			module_config->auto_start = FALSE;
			module_config->auto_payment = FALSE;
			module_config->full_tank_volume = 0;

			if (module_config->dispencer_count > 0)
			{
				for (guint8 i = 0; i < module_config->dispencer_count; i++)
				{
					DispencerConf* disp_conf = &module_config->dispencers[i];

					disp_conf->num = 0;
					disp_conf->addr = 0;

					if (disp_conf->nozzle_count > 0)
					{
						for (guint8 j = 0; j < disp_conf->nozzle_count; j++)
						{
							NozzleConf* nozzle_conf = &disp_conf->nozzles[j];

							nozzle_conf->num = 0;
							nozzle_conf->grade = 0;
						}
						disp_conf->nozzle_count = 0;
					}
				}
				module_config->dispencer_count = 0;
			}

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean delete_tgs_conf(TgsConf tgs_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.tgs_count; i++)
	{
		if (configuration.devices.tgs[i].id == tgs_conf.id)
		{
			TgsConf* dest = &configuration.devices.tgs[i];

			dest->id = 0;
			dest->index = 0;

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}

			dest->port = 0;
			dest->enable = FALSE;
			dest->command_timeout = 0;
			dest->interval = 0;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			dest->log_enable = FALSE;
			dest->log_trace = FALSE;

			TGSLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = FALSE;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}

			log_options->file_size = 0;
			log_options->save_days = 0;

			log_options->trace = FALSE;
			log_options->system = FALSE;
			log_options->requests = FALSE;
			log_options->frames = FALSE;
			log_options->parsing = FALSE;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = 0;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}


			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}

			conn_options->port = 0;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = 0;
			timeout_options->t_write = 0;

			if (module_config->tank_count > 0)
			{
				for (guint8 i = 0; i < module_config->tank_count; i++)
				{
					TankConf* tank_conf = &module_config->tanks[i];

					tank_conf->num = 0;
					tank_conf->channel = 0;

				}
				module_config->tank_count = 0;
			}

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean delete_fr_conf(FiscalRegisterConf fr_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.fiscal_register_count; i++)
	{
		if (configuration.devices.fiscal_registers[i].id == fr_conf.id)
		{
			FiscalRegisterConf* dest = &configuration.devices.fiscal_registers[i];

			dest->id = 0;
			dest->index = 0;

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}

			dest->port = 0;
			dest->enable = FALSE;
			dest->command_timeout = 0;
			dest->interval = 0;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			dest->log_enable = FALSE;
			dest->log_trace = FALSE;

			FRLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = FALSE;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}

			log_options->file_size = 0;
			log_options->save_days = 0;

			log_options->trace = FALSE;
			log_options->system = FALSE;
			log_options->requests = FALSE;
			log_options->frames = FALSE;
			log_options->parsing = FALSE;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = 0;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}


			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}

			conn_options->port = 0;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = 0;
			timeout_options->t_write = 0;

			module_config->protocol_type = FALSE;
			module_config->auto_drawer = FALSE;
			module_config->auto_cutting = FALSE;
			module_config->cash_num = 0;
			module_config->bn_num = 0;
			module_config->bonus_num = 0;
			module_config->time_sync = FALSE;

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean delete_ppc_conf(PpcConf ppc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.price_pole_controller_count; i++)
	{
		if (configuration.devices.price_pole_controllers[i].id == ppc_conf.id)
		{
			PpcConf* dest = &configuration.devices.price_pole_controllers[i];

			dest->id = 0;
			dest->index = 0;

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}

			dest->port = 0;
			dest->enable = FALSE;
			dest->command_timeout = 0;
			dest->interval = 0;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			dest->log_enable = FALSE;
			dest->log_trace = FALSE;

			PPCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = FALSE;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}

			log_options->file_size = 0;
			log_options->save_days = 0;

			log_options->trace = FALSE;
			log_options->system = FALSE;
			log_options->requests = FALSE;
			log_options->frames = FALSE;
			log_options->parsing = FALSE;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = 0;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}


			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}

			conn_options->port = 0;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = 0;
			timeout_options->t_write = 0;

			PPCDecimalPointOptions* dp_options = &module_config->decimal_point_options;

			dp_options->dp_price = 0;


			if (module_config->price_pole_count > 0)
			{
				for (guint8 i = 0; i < module_config->price_pole_count; i++)
				{
					PricePoleConf* price_pole_conf = &module_config->price_poles[i];

					price_pole_conf->num = 0;
					price_pole_conf->grade = 0;
					price_pole_conf->symbol_count = 0;

				}
				module_config->price_pole_count = 0;
			}

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

gboolean delete_sc_conf(ScConf sc_conf)
{
	gboolean result = FALSE;

	g_mutex_lock(&configuration_mutex);

	for (guint8 i = 0; i < configuration.devices.sensor_controller_count; i++)
	{
		if (configuration.devices.sensor_controllers[i].id == sc_conf.id)
		{
			ScConf* dest = &configuration.devices.sensor_controllers[i];

			dest->id = 0;
			dest->index = 0;

			if(dest->name != NULL)
			{
				g_free(dest->name);
				dest->name = NULL;
			}

			dest->port = 0;
			dest->enable = FALSE;
			dest->command_timeout = 0;
			dest->interval = 0;

			if(dest->module_name != NULL)
			{
				g_free(dest->module_name);
				dest->module_name = NULL;
			}

			if(dest->log_dir != NULL)
			{
				g_free(dest->log_dir);
				dest->log_dir = NULL;
			}
			dest->log_enable = FALSE;
			dest->log_trace = FALSE;

			SCLibConfig* module_config = &dest->module_config;

			LibLogOptions* log_options = &module_config->log_options;

			log_options->enable = FALSE;

			if(log_options->dir != NULL)
			{
				g_free(log_options->dir);
				log_options->dir = NULL;
			}

			log_options->file_size = 0;
			log_options->save_days = 0;

			log_options->trace = FALSE;
			log_options->system = FALSE;
			log_options->requests = FALSE;
			log_options->frames = FALSE;
			log_options->parsing = FALSE;

			ConnOptions* conn_options = &module_config->conn_options;

			conn_options->connection_type = 0;

			if(conn_options->port != NULL)
			{
				g_free(conn_options->port);
				conn_options->port = NULL;
			}


			if(conn_options->ip_address != NULL)
			{
				g_free(conn_options->ip_address);
				conn_options->ip_address = NULL;
			}

			conn_options->port = 0;

			TimeoutOptions* timeout_options = &module_config->timeout_options;

			timeout_options->t_read = 0;
			timeout_options->t_write = 0;

			if (module_config->sensor_count > 0)
			{
				for (guint8 i = 0; i < module_config->sensor_count; i++)
				{
					SensorConf* sensor_conf = &module_config->sensors[i];

					sensor_conf->num = 0;
					sensor_conf->addr = 0;

					if (sensor_conf->param_count > 0)
					{
						for (guint8 j = 0; j < sensor_conf->param_count; j++)
						{
							sensor_conf->params[j].num = 0;
							sensor_conf->params[j].type = 0;
						}
					}
					sensor_conf->param_count = 0;

				}
				module_config->sensor_count = 0;
			}

			dest->changed = TRUE;

			result = TRUE;
		}
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

void set_reset()
{
	g_mutex_lock(&configuration_mutex);

	configuration.common_conf.changed = TRUE;

	if (configuration.devices.dispencer_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.dispencer_controller_count; i++)
		{
			if (configuration.devices.dispencer_controllers[i].enable)
			{
				configuration.devices.dispencer_controllers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.tgs_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.tgs_count; i++)
		{
			if (configuration.devices.tgs[i].enable)
			{
				configuration.devices.tgs[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.fiscal_register_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.fiscal_register_count; i++)
		{
			if (configuration.devices.fiscal_registers[i].enable)
			{
				configuration.devices.fiscal_registers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.price_pole_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.price_pole_controller_count; i++)
		{
			if (configuration.devices.price_pole_controllers[i].enable)
			{
				configuration.devices.price_pole_controllers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.sensor_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.sensor_controller_count; i++)
		{
			if (configuration.devices.sensor_controllers[i].enable)
			{
				configuration.devices.sensor_controllers[i].changed = TRUE;
			}
		}
	}

	g_mutex_unlock(&configuration_mutex);

}

void set_device_changed()
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.dispencer_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.dispencer_controller_count; i++)
		{
			if (configuration.devices.dispencer_controllers[i].enable)
			{
				configuration.devices.dispencer_controllers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.tgs_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.tgs_count; i++)
		{
			if (configuration.devices.tgs[i].enable)
			{
				configuration.devices.tgs[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.fiscal_register_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.fiscal_register_count; i++)
		{
			if (configuration.devices.fiscal_registers[i].enable)
			{
				configuration.devices.fiscal_registers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.price_pole_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.price_pole_controller_count; i++)
		{
			if (configuration.devices.price_pole_controllers[i].enable)
			{
				configuration.devices.price_pole_controllers[i].changed = TRUE;
			}
		}
	}

	if (configuration.devices.sensor_controller_count > 0)
	{
		for (guint8 i = 0; i < configuration.devices.sensor_controller_count; i++)
		{
			if (configuration.devices.sensor_controllers[i].enable)
			{
				configuration.devices.sensor_controllers[i].changed = TRUE;
			}
		}
	}

	g_mutex_unlock(&configuration_mutex);

}

void update_common_conf(CommonConf src)
{
	g_mutex_lock(&configuration_mutex);

	if (src.server_name !=NULL)
	{
		if (configuration.common_conf.server_name !=NULL)
		{
			g_free(configuration.common_conf.server_name);
			configuration.common_conf.server_name = NULL;
		}
		configuration.common_conf.server_name = g_strdup(src.server_name);

	}

	configuration.common_conf.port = src.port;

	if (src.log_dir !=NULL)
	{
		if (configuration.common_conf.log_dir !=NULL)
		{
			g_free(configuration.common_conf.log_dir);
			configuration.common_conf.log_dir = NULL;
		}
		configuration.common_conf.log_dir = g_strdup(src.log_dir);
	}

	configuration.common_conf.log_enable = src.log_enable;
	configuration.common_conf.log_trace = src.log_trace;


	gboolean reset_device_flag = compare_strings(configuration.common_conf.conn_log_dir, src.conn_log_dir);

	if (configuration.common_conf.conn_log_enable != src.conn_log_enable ||
		configuration.common_conf.conn_log_frames != src.conn_log_frames ||
		configuration.common_conf.conn_log_parsing != src.conn_log_parsing ||
		configuration.common_conf.conn_log_trace != src.conn_log_trace )
	{
		reset_device_flag = TRUE;
	}

	if (configuration.common_conf.conn_log_dir !=NULL)
	{
		g_free(configuration.common_conf.conn_log_dir);
		configuration.common_conf.conn_log_dir = NULL;
	}

	if (src.conn_log_dir !=NULL)
	{
		configuration.common_conf.conn_log_dir = g_strdup(src.conn_log_dir);
	}

	configuration.common_conf.conn_log_enable = src.conn_log_enable;
	configuration.common_conf.conn_log_frames = src.conn_log_frames;
	configuration.common_conf.conn_log_parsing = src.conn_log_parsing;
	configuration.common_conf.conn_log_trace = src.conn_log_trace;

	configuration.common_conf.changed = TRUE;

	g_mutex_unlock(&configuration_mutex);

	if (reset_device_flag)
	{
		set_device_changed();
	}
}

void get_dispencer_controller_conf(guint8 index, DispencerControllerConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.dispencer_controller_count > index)
	{
		*result = configuration.devices.dispencer_controllers[index];
	}

	g_mutex_unlock(&configuration_mutex);

}

void get_tank_gauge_system_conf(guint8 index, TgsConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.tgs_count > index)
	{
		*result = configuration.devices.tgs[index];
	}

	g_mutex_unlock(&configuration_mutex);

}

void get_fiscal_register_conf(guint8 index, FiscalRegisterConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.fiscal_register_count > index)
	{
		*result = configuration.devices.fiscal_registers[index];
	}

	g_mutex_unlock(&configuration_mutex);

}

void get_price_pole_controller_conf(guint8 index, PpcConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.price_pole_controller_count > index)
	{
		*result = configuration.devices.price_pole_controllers[index];
	}

	g_mutex_unlock(&configuration_mutex);

}

void get_sensor_controller_conf(guint8 index, ScConf* result)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.sensor_controller_count > index)
	{
		*result = configuration.devices.sensor_controllers[index];
	}

	g_mutex_unlock(&configuration_mutex);

}

void get_connection_log_conf(gchar** log_dir, gboolean* log_enable, gboolean* log_trace, gboolean* log_frames, gboolean* log_parsing)
{
	g_mutex_lock(&configuration_mutex);

	*log_dir = g_strdup(configuration.common_conf.conn_log_dir);

	*log_enable = configuration.common_conf.conn_log_enable;
	*log_trace = configuration.common_conf.conn_log_trace;
	*log_frames = configuration.common_conf.conn_log_frames;
	*log_parsing = configuration.common_conf.conn_log_parsing;

	g_mutex_unlock(&configuration_mutex);
}

void set_profiles_enable(gboolean new_value)
{
	g_mutex_lock(&configuration_mutex);

	configuration.profiles_conf.enable = new_value;

	g_mutex_unlock(&configuration_mutex);

}

void get_profiles_enable(gboolean* result)
{
	g_mutex_lock(&configuration_mutex);

	*result = configuration.profiles_conf.enable;

	g_mutex_unlock(&configuration_mutex);

}

ThreadStatus get_dc_thread_status(guchar device_index)
{
	ThreadStatus  result = ts_Undefined;

	g_mutex_lock(&configuration_mutex);

	if (device_index < configuration.devices.dispencer_controller_count)
	{
		result = configuration.devices.dispencer_controllers[device_index].thread_status;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

ThreadStatus get_tgs_thread_status(guchar device_index)
{
	ThreadStatus  result = ts_Undefined;

	g_mutex_lock(&configuration_mutex);

	if (device_index < configuration.devices.tgs_count)
	{
		result = configuration.devices.tgs[device_index].thread_status;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

ThreadStatus get_fr_thread_status(guchar device_index)
{
	ThreadStatus  result = ts_Undefined;

	g_mutex_lock(&configuration_mutex);

	if (device_index < configuration.devices.fiscal_register_count)
	{
		result = configuration.devices.fiscal_registers[device_index].thread_status;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

ThreadStatus get_ppc_thread_status(guchar device_index)
{
	ThreadStatus  result = ts_Undefined;

	g_mutex_lock(&configuration_mutex);

	if (device_index < configuration.devices.price_pole_controller_count)
	{
		result = configuration.devices.price_pole_controllers[device_index].thread_status;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

ThreadStatus get_sc_thread_status(guchar device_index)
{
	ThreadStatus  result = ts_Undefined;

	g_mutex_lock(&configuration_mutex);

	if (device_index < configuration.devices.sensor_controller_count)
	{
		result = configuration.devices.sensor_controllers[device_index].thread_status;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;
}

void set_dc_thread_status(guchar device_id, ThreadStatus new_status)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.dispencer_controller_count > device_id)
	{
		configuration.devices.dispencer_controllers[device_id].thread_status = new_status;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_tgs_thread_status(guchar device_id, ThreadStatus new_status)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.tgs_count > device_id)
	{
		configuration.devices.tgs[device_id].thread_status = new_status;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_fr_thread_status(guchar device_id, ThreadStatus new_status)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.fiscal_register_count > device_id)
	{
		configuration.devices.fiscal_registers[device_id].thread_status = new_status;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_ppc_thread_status(guchar device_id, ThreadStatus new_status)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.price_pole_controller_count > device_id)
	{
		configuration.devices.price_pole_controllers[device_id].thread_status = new_status;
	}

	g_mutex_unlock(&configuration_mutex);
}

void set_sc_thread_status(guchar device_id, ThreadStatus new_status)
{
	g_mutex_lock(&configuration_mutex);

	if (configuration.devices.sensor_controller_count > device_id)
	{
		configuration.devices.sensor_controllers[device_id].thread_status = new_status;
	}

	g_mutex_unlock(&configuration_mutex);
}
void set_dispencer_controller_index(guint8 device_index, guint8 dispencer_controller_index)
{
	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.dispencer_controller_count)
	{
		configuration.devices.dispencer_controllers[device_index].dispencer_controller_index = dispencer_controller_index;
	}

	g_mutex_unlock(&configuration_mutex);

}

void set_tgs_index(guint8 device_index, guint8 tgs_index)
{
	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.tgs_count)
	{
		configuration.devices.tgs[device_index].tgs_index = tgs_index;
	}

	g_mutex_unlock(&configuration_mutex);

}

void set_fiscal_register_index(guint8 device_index, guint8 fiscal_register_index)
{
	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.fiscal_register_count)
	{
		configuration.devices.fiscal_registers[device_index].fiscal_register_index = fiscal_register_index;
	}

	g_mutex_unlock(&configuration_mutex);

}

void set_ppc_index(guint8 device_index, guint8 ppc_index)
{
	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.price_pole_controller_count)
	{
		configuration.devices.price_pole_controllers[device_index].ppc_index = ppc_index;
	}

	g_mutex_unlock(&configuration_mutex);

}

void set_sc_index(guint8 device_index, guint8 ppc_index)
{
	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.sensor_controller_count)
	{
		configuration.devices.sensor_controllers[device_index].sc_index = ppc_index;
	}

	g_mutex_unlock(&configuration_mutex);

}

guint8 get_dispencer_controller_index(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.dispencer_controller_count)
	{
		result = configuration.devices.dispencer_controllers[device_index].dispencer_controller_index;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

guint8 get_tgs_index(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.tgs_count)
	{
		result = configuration.devices.tgs[device_index].tgs_index;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

guint8 get_fiscal_register_index(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.fiscal_register_count)
	{
		result = configuration.devices.fiscal_registers[device_index].fiscal_register_index;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

guint8 get_ppc_index(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.price_pole_controller_count)
	{
		result = configuration.devices.price_pole_controllers[device_index].ppc_index;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}

guint8 get_sc_index(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&configuration_mutex);

	if ( device_index < configuration.devices.sensor_controller_count)
	{
		result = configuration.devices.sensor_controllers[device_index].sc_index;
	}

	g_mutex_unlock(&configuration_mutex);

	return result;

}
