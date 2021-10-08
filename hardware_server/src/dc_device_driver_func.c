#include <glib.h>
#include <glib/gstdio.h>
#include <dlfcn.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "dc_device.h"
#include "dc_device_data.h"

void set_dc_dp_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace )
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting decimal points...");

	guchar (*get_decimal_point)(guchar* price_dp, guchar* volume_dp, guchar* amount_dp);

	guint8 price_dp = DEF_DC_PRICE_DP;
	guint8 volume_dp = DEF_DC_VOLUME_DP;
	guint8 amount_dp = DEF_DC_AMOUNT_DP;

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

		guchar device_error = (*get_decimal_point)(&price_dp, &volume_dp, &amount_dp);

		if (device_error!= dce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "define decimal points error. Activated default settings.");
		}

		set_dc_device_last_error(dci, device_error);

	}

	set_dc_decimal_pointers(dci, price_dp, volume_dp, amount_dp);
}

void set_ext_funcs_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace)
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting extended function...");

	guchar (*get_extended_func_count)(guchar* count);
	guchar (*get_extended_func_name)(guchar index, gchar* name);

	guint8 count = 0;

	get_extended_func_count = dlsym( handle_lib, "get_extended_func_count" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error );
		g_free(error);
	}
	else
	{
		guchar device_error = (*get_extended_func_count)(&count);

		if (device_error!= dce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting extended function count error.");
			count = 0;
		}

		set_dc_device_last_error(dci, device_error);
	}

	if (count > 0)
	{
		for (guint8 i = 0; i < count; i++)
		{
			get_extended_func_name = dlsym( handle_lib, "get_extended_func_name" );
			error = dlerror();

			if (error != NULL)
			{
				add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
				g_free(error);
			}
			else
			{
				gchar* func_name = NULL;
				guchar device_error = (*get_extended_func_name)(i, func_name);

				if (device_error!= dce_NoError)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting extended function count error.");
					count = 0;
				}
				else
				{
					set_dc_ext_func_name(dci, i, func_name);
				}

				set_dc_device_last_error(dci, device_error);
			}
		}
	}

	set_dc_ext_func_count(dci, count);
}

void set_dispencers_config_from_driver(void* handle_lib, gchar* device_name, guint8 dci, LogParams* log_params, gboolean log_trace)
{
	add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting dispencer configuration...");

	guchar (*get_disp_count)(guchar* count);
	guchar (*get_disp_info)(guint8 index, guint32* num, guint8* addr, guint8* nozzle_count);
	guchar (*get_nozzle_info)(guint8 disp_index, guint8 nozzle_index, guint8* nozzle_num, guint8* nozzle_grade);

	guint8 count = 0;

	get_disp_count = dlsym( handle_lib, "get_disp_count" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error);
		g_free(error);
	}
	else
	{
		g_free(error);

		guchar device_error = (*get_disp_count)(&count);

		set_dc_device_last_error(dci, device_error);

		if (device_error!= dce_NoError)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get dispencer count error. Activated default settings.");
		}
		else
		{
			if (count > 0)
			{
				get_disp_info = dlsym( handle_lib, "get_disp_info" );
				error = dlerror();

				if (error != NULL)
				{
					add_log(log_params, TRUE, TRUE, log_trace, TRUE, "error: %s", error );
					g_free(error);
				}
				else
				{
					g_free(error);

					get_nozzle_info = dlsym( handle_lib, "get_nozzle_info" );
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
							guint32 disp_num = 0;
							guint8 disp_addr = 0;
							guint8 nozzle_count = 0;

							device_error = (*get_disp_info)(i, &disp_num, &disp_addr, &nozzle_count);

							set_dc_device_last_error(dci, device_error);

							if (device_error!= dce_NoError)
							{
								add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting dispencer %d info error.", i );
								count = 0;
							}
							else
							{
								if (nozzle_count > 0)
								{
									for (guint8 j = 0; j < nozzle_count; j++)
									{
										guint8 nozzle_num = 0;
										guint8 nozzle_grade = 0;

										device_error = (*get_nozzle_info)(i,j,&nozzle_num, &nozzle_grade);

										set_dc_device_last_error(dci, device_error);

										if (device_error != dce_NoError)
										{
											add_log(log_params, TRUE, TRUE, log_trace, TRUE, "getting nozzle %d-%d info error.",  i, j );
										}
										else
										{
											set_dc_nozzle_info(dci, i, j, nozzle_num, nozzle_grade );
										}

									}
								}

								set_dc_disp_info(dci, i, disp_num, disp_addr, nozzle_count);
							}
						}
					}
				}
			}
			set_dc_disp_count(dci, count);
		}
	}

}

void set_dc_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 dci)
{
	guchar (*lib_func)();

	DcDeviceStatus status = dcs_UndefinedError;

	lib_func = dlsym( handle_lib, "get_status" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		status = (*lib_func)();

		set_dc_device_status(dci, status);
	}

	g_free(error);
}

DcDeviceError get_counter_from_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32* counter )
{
	guchar (*lib_func)(guint32 p_disp_num, guint8 p_nozzle_num, guint32* p_counter);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "get_counter" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num, nozzle_num, counter);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_reset_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "reset" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_update_prices_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, DCPricePacks* price_packs)
{
	guchar (*lib_func)(guint32 p_disp_num, guint32 price1, guint32 price2,guint32 price3,guint32 price4,guint32 price5,guint32 price6,guint32 price7,guint32 price8);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "set_prices" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		guint32 prices[MAX_NOZZLE_COUNT] = {0x00};

		if (price_packs->nozzle_price_pack_count > 0)
		{
			for (guint8 i = 0; i < price_packs->nozzle_price_pack_count; i++)
			{
				if (price_packs->nozzle_price_packs[i].nozzle_num > 0 && price_packs->nozzle_price_packs[i].nozzle_num <= MAX_NOZZLE_COUNT )
				{
					prices[price_packs->nozzle_price_packs[i].nozzle_num - 1] = price_packs->nozzle_price_packs[i].price;
				}
			}
		}

		result = (*lib_func)(disp_num, prices[0], prices[1], prices[2], prices[3], prices[4], prices[5], prices[6], prices[7]);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_start_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "start" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_stop_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "stop" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_suspend_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "suspend" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_resume_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "resume" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_payment_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num)
{
	guchar (*lib_func)(guint32 p_disp_num);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "payment" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_volume_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 volume)
{
	guchar (*lib_func)(guint32 p_disp_num, guint8 p_nozzle_num, guint32 p_price, guint32 p_volume);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "set_volume_dose" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num, nozzle_num, price, volume);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}
DcDeviceError send_sum_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price, guint32 amount)
{

	guchar (*lib_func)(guint32 p_disp_num, guint8 p_nozzle_num, guint32 p_price, guint32 p_amount);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "set_sum_dose" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num, nozzle_num, price, amount);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_full_tank_dose_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 nozzle_num, guint32 price)
{

	guchar (*lib_func)(guint32 p_disp_num, guint8 p_nozzle_num, guint32 p_price);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "set_full_tank_dose" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num, nozzle_num, price);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

DcDeviceError send_extended_func_to_driver(void* handle_lib, gchar* device_name, guint8 dci, guint32 disp_num, guint8 func_index)
{

	guchar (*lib_func)(guint32 p_disp_num, guint8 p_func_index);

	DcDeviceError result = dce_Undefined;

	lib_func = dlsym( handle_lib, "set_full_tank_dose" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		result = (*lib_func)(disp_num, func_index);

		set_dc_device_last_error(dci, result);
	}

	g_free(error);

	return result;
}

void set_dispencer_state_from_driver(void* handle_lib, gchar* device_name, guint8 dci, guint8  index_disp, LogParams* log_params, gboolean log_trace)
{
	DcDeviceError result = dce_Undefined;

	guchar (*get_disp_state_func)(guint32 p_disp_num, DispencerState* p_disp_state, OrderType* p_preset_order_type, guint8* p_preset_nozzle_num, guint32* p_preset_price, guint32* p_preset_volume,
						guint32* p_preset_amount, OrderType* p_order_type, guint8* p_active_nozzle_num, guint32* p_current_price, guint32* p_current_volume, guint32* p_current_amount,
						gboolean* p_is_pay, guchar* p_error, guchar* p_error_description);


	guchar (*get_counter_func)(guint32 p_disp_num, guint8 p_nozzle_num, guint32* p_counter);

	guint32 disp_num = get_dc_dispencer_num(dci, index_disp);

	get_disp_state_func = dlsym( handle_lib, "get_disp_state" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get_disp_state for dispencer %d error: %s\n", disp_num,  error );

		g_free(error);
	}
	else
	{
		get_counter_func = dlsym( handle_lib, "get_counter" );
		error = dlerror();

		if (error != NULL)
		{
			add_log(log_params, TRUE, TRUE, log_trace, TRUE, "get_counter for dispencer %d error: %s\n",disp_num,  error);
		}
		else
		{
			DispencerState disp_state = ds_NotInitialize;

			OrderType preset_order_type = ot_Free;
			guint8 preset_nozzle_num = 0;
			guint32 preset_price = 0;
			guint32 preset_volume = 0;
			guint32 preset_amount = 0;

			OrderType order_type = ot_Free;
			guint8 active_nozzle_num = 0;
			guint32 current_price = 0;
			guint32 current_volume = 0;
			guint32 current_amount = 0;

			gboolean is_pay = FALSE;
			guchar error = 0x00;
			guchar error_description[MAX_DC_STR_LENGTH] = {0x00};

			guint32 counters[MAX_NOZZLE_COUNT] = {0x00};

			result = (*get_disp_state_func)(disp_num, &disp_state, &preset_order_type, &preset_nozzle_num, &preset_price, &preset_volume, &preset_amount, &order_type, &active_nozzle_num,
							&current_price, &current_volume, &current_amount, &is_pay, &error, error_description);

			gboolean counters_ready = get_dc_dispencer_state(dci, index_disp) != disp_state;

			guint8 nozzle_count = get_dc_dispencer_nozzle_count(dci, index_disp);

			if (disp_state > ds_NotInitialize && disp_state< ds_ConnectionError && counters_ready && nozzle_count > 0)
			{
				for (guint8 i = 0; i < nozzle_count && i < MAX_NOZZLE_COUNT; i++)
				{
					counters[i] = 0;
					guint8 num = 0;
					guint8 grade = 0;

					get_dc_nozzle_info(dci, index_disp, i, &num, &grade);

					result = (*get_counter_func)(disp_num, num, &counters[i]);
				}
			}

			set_dc_dispencer_data(dci, index_disp, device_name, disp_num, result, disp_state,
										preset_order_type, preset_nozzle_num, preset_price, preset_volume, preset_amount,
										order_type,  active_nozzle_num, current_price, current_volume, current_amount,
										is_pay, error, error_description, counters_ready, nozzle_count, counters, log_params, log_trace );


		}
	}
	g_free(error);
}
