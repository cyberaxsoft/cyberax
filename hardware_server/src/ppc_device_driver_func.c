#include <glib.h>
#include <glib/gstdio.h>
#include <dlfcn.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "ppc_device.h"
#include "ppc_device_data.h"

void set_ppc_dp_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace )
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting decimal points...");

	guchar (*get_decimal_point)(guchar* price_dp);

	guint8 price_dp = DEF_PPC_PRICE_DP;

	get_decimal_point = dlsym( handle_lib, "get_decimal_point" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
		g_free(error);
	}
	else
	{
		g_free(error);

		guchar device_error = (*get_decimal_point)(&price_dp);

		if (device_error!= ppce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "define decimal points error. Activated default settings.");
		}

		set_ppc_device_last_error(ppci, device_error);

	}

	set_ppc_decimal_pointers(ppci, price_dp);
}

void set_price_pole_config_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace)
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting price poles configuration...");

	guchar (*get_price_pole_count)(guchar* count);
	guchar (*get_price_pole_info)(guint8 index, guint8* num, guint8* grade, guint8* symbol_count);

	guint8 count = 0;

	get_price_pole_count = dlsym( handle_lib, "get_price_pole_count" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
		g_free(error);
	}
	else
	{
		g_free(error);

		guchar device_error = (*get_price_pole_count)(&count);

		set_ppc_device_last_error(ppci, device_error);

		if (device_error!= ppce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get price pole count error. Activated default settings.");
		}
		else
		{
			if (count > 0)
			{
				get_price_pole_info = dlsym( handle_lib, "get_price_pole_info" );
				error = dlerror();

				if (error != NULL)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error );
					g_free(error);
				}
				else
				{
					g_free(error);

					for (guint8 i = 0; i < count; i++)
					{
						guint8 price_pole_num = 0;
						guint8 price_pole_grade = 0;
						guint8 price_pole_symbol_count = 0;

						device_error = (*get_price_pole_info)(i, &price_pole_num, &price_pole_grade, &price_pole_symbol_count);

						set_ppc_device_last_error(ppci, device_error);

						if (device_error!= ppce_NoError)
						{
							add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting price pole %d info error.", i );
							count = 0;
						}
						else
						{
							set_ppc_price_pole_info(ppci, i, price_pole_num, price_pole_grade, price_pole_symbol_count);
						}
					}
				}
			}
			set_ppc_price_pole_count(ppci, count);
		}
	}

}

void set_ppc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, LogParams* log_params, gboolean log_trace)
{
	guchar (*lib_func)();

	PpcDeviceStatus status = ppcds_UndefinedError;

	lib_func = dlsym( handle_lib, "get_status" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "%s error: %s\n", device_name,  error );
	}
	else
	{
		status = (*lib_func)();

		set_ppc_device_status(ppci, status);
	}

	g_free(error);
}

void set_price_pole_state_from_driver(void* handle_lib, gchar* device_name, guint8 ppci, guint8  index_price_pole, LogParams* log_params, gboolean log_trace)
{
	guchar (*get_price_pole_state_func)(guint8 p_tank_num, guint8* state, guint32* price);


	guint8 price_pole_num = get_ppc_price_pole_num(ppci, index_price_pole);

	get_price_pole_state_func = dlsym( handle_lib, "get_price_pole_state" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get_price_pole_state for price pole %d error: %s\n", price_pole_num,  error );

		g_free(error);
	}
	else
	{
		guint8 state = 0;
		guint32 price = 0;

		guchar result = (*get_price_pole_state_func)(price_pole_num, &state, &price);

		if (result == ppce_NoError)
		{
			set_ppc_price_pole_data(ppci, index_price_pole, price, state);
		}
	}


}

PpcDeviceError set_price_to_driver(void* handle_lib, gchar* device_name, guint8 ppci, PPCPricePacks price_packs, LogParams* log_params, gboolean log_trace)
{
	PpcDeviceError result = ppce_Undefined;

	guchar (*set_price_func)(guint8 p_grade, guint32 price);


	set_price_func = dlsym( handle_lib, "set_price" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "set_price error: %s\n", error );

		g_free(error);
	}
	else
	{
		if (price_packs.grade_price_pack_count > 0)
		{
			for (guint8 i = 0; i < price_packs.grade_price_pack_count; i++)
			{
				result = (*set_price_func)(price_packs.grade_price_packs[i].grade, price_packs.grade_price_packs[i].price);

				if (result != ppce_NoError)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "set price %d for grade %d error\n", price_packs.grade_price_packs[i].price, price_packs.grade_price_packs[i].grade);
					break;
				}

			}
		}
	}


	return result;
}

