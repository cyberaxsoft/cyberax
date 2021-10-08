#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>

#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_parse.h"
#include "pss_dc_device.h"
#include "pss_xml.h"
#include "pss_func.h"

PSSData pss_data = {0x00};
guint32 door_sensor_num = 0;
guint8 door_sensor_param_num = 0;
GMutex	   pss_data_mutex;

gboolean server_conf_is_load = FALSE;
GMutex	   scl_mutex;


gboolean door_switch = FALSE;
GMutex sc_mutex;

void init_sc_mutex()
{
	g_mutex_init(&sc_mutex);
}

void init_pss_data_mutex()
{
	g_mutex_init(&pss_data_mutex);
	g_mutex_init(&scl_mutex);
}

void set_door_settings(guint32 new_sensor_num, guint8 new_param_num)
{
	g_mutex_lock(&pss_data_mutex);

	door_sensor_num = new_sensor_num;
	door_sensor_param_num = new_param_num;

	g_mutex_unlock(&pss_data_mutex);

}

void get_door_settings(guint32* p_sensor_num, guint8* p_param_num)
{
	g_mutex_lock(&pss_data_mutex);

	*p_sensor_num = door_sensor_num;
	*p_param_num = door_sensor_param_num;

	g_mutex_unlock(&pss_data_mutex);

}

gboolean get_door_switch()
{
	gboolean result = FALSE;

	g_mutex_lock(&sc_mutex);

	result = door_switch;

	g_mutex_unlock(&sc_mutex);

	return result;
}

void set_door_switch(gboolean new_value)
{
	g_mutex_lock(&sc_mutex);

	door_switch = new_value;

	g_mutex_unlock(&sc_mutex);
}



gboolean get_server_conf_is_load()
{
	gboolean result = FALSE;

	g_mutex_lock(&pss_data_mutex);

	result = server_conf_is_load;

	g_mutex_unlock(&pss_data_mutex);

	return result;
}

void clear_service_devices()
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.server_devices.count > 0)
	{
		for (guint8 i = 0; i < pss_data.devices.server_devices.count; i++)
		{
			PSSServerDevice* device = &pss_data.devices.server_devices.units[i];
			if (device->guid !=NULL)
			{
				g_free(device->guid);
				device->guid = NULL;
			}
			if (device->ip_address !=NULL)
			{
				g_free(device->ip_address);
				device->ip_address = NULL;
			}
			device->device_type = psdt_System;
			device->id = 0;
			device->port = 0;
			device->timeout = 0;

			PSSServerDeviceUnits* units = &device->units;

			if (units->count > 0)
			{
				for (guint8 j = 0; j < units->count; j++)
				{
					units->units[j] = 0;
				}

				units->count = 0;
			}

		}
		pss_data.devices.server_devices.count = 0;
	}

	g_mutex_unlock(&pss_data_mutex);
}

void clear_service_device(PSSServerDevice* device )
{
	if (device->guid !=NULL)
	{
		g_free(device->guid);
		device->guid = NULL;
	}
	if (device->ip_address !=NULL)
	{
		g_free(device->ip_address);
		device->ip_address = NULL;
	}

	device->device_type = psdt_System;
	device->id = 0;
	device->port = 0;
	device->timeout = 0;

	PSSServerDeviceUnits* units = &device->units;

	if (units->count > 0)
	{
		for (guint8 j = 0; j < units->count; j++)
		{
			units->units[j] = 0;
		}

		units->count = 0;
	}
}

void add_server_device(PSSServerDevice* device )
{
	g_mutex_lock(&pss_data_mutex);

	PSSServerDevice* dst = &pss_data.devices.server_devices.units[pss_data.devices.server_devices.count];

	dst->device_type = device->device_type;
	if (dst->guid!=NULL)
	{
		g_free(dst->guid);
		dst->guid = NULL;
	}
	dst->guid = g_strdup(device->guid);
	dst->id = device->id;
	if (dst->ip_address!=NULL)
	{
		g_free(dst->ip_address);
		dst->ip_address = NULL;
	}
	dst->ip_address = g_strdup(device->ip_address);
	dst->port = device->port;
	dst->timeout = device->timeout;
	dst->units.count = device->units.count;

	if (device->units.count > 0)
	{
		for (guint8 i = 0; i < device->units.count; i++ )
		{
			dst->units.units[i] = device->units.units[i];
		}
	}

	pss_data.devices.server_devices.count++;

	g_mutex_unlock(&pss_data_mutex);
}

void set_server_conf_is_load(gboolean new_value)
{
	g_mutex_lock(&pss_data_mutex);

	server_conf_is_load = new_value;

	g_mutex_unlock(&pss_data_mutex);

}

gboolean load_pss_data()
{
	gchar* conf = NULL;

	get_conf_filename(&conf);

	gboolean result = FALSE;

	g_mutex_lock(&pss_data_mutex);
	result = load_pss_xml_data(conf, &pss_data);
	g_mutex_unlock(&pss_data_mutex);

	g_free(conf);

	return result;
}

gboolean save_pss_data()
{
	gboolean result = FALSE;

	gchar* conf = NULL;

	get_conf_filename(&conf);

	g_mutex_lock(&pss_data_mutex);
	result = save_pss_xml_data(conf, &pss_data);
	g_mutex_unlock(&pss_data_mutex);

	g_free(conf);

	return result;
}

void get_fc_status(PSSFcStatus* status)
{
	g_mutex_lock(&pss_data_mutex);

	*status = pss_data.common.fc_status;

	g_mutex_unlock(&pss_data_mutex);
}

void get_fc_log_configuration(gchar** log_dir, gboolean* log_enable, gboolean* log_trace)
{
	g_mutex_lock(&pss_data_mutex);

	*log_dir = g_strdup(pss_data.common.log_dir);
	*log_enable = pss_data.common.log_enable;
	*log_trace = pss_data.common.log_trace;

	g_mutex_unlock(&pss_data_mutex);

}

void set_fc_log_configuration(gchar* log_dir, gboolean log_enable, gboolean log_trace)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.common.log_dir!=NULL)
	{
		g_free(pss_data.common.log_dir);
		pss_data.common.log_dir = NULL;
	}
	pss_data.common.log_dir  = g_strdup(log_dir);

	pss_data.common.log_enable = log_enable;
	pss_data.common.log_trace = log_trace;

	g_mutex_unlock(&pss_data_mutex);

}

//void get_price_group_by_id(guint8 id, PSSPriceGroup* pg)
//{
//	g_mutex_lock(&pss_data_mutex);
//
//	if (pss_data.general_configuration.general_functions.price_group_count > 0)
//	{
//		for (guint i = 0; i < pss_data.general_configuration.general_functions.price_group_count; i++)
//		{
//			if (pss_data.general_configuration.general_functions.price_groups[i].id == id)
//			{
//				*pg = pss_data.general_configuration.general_functions.price_groups[i];
//				break;
//			}
//		}
//	}
//
//	g_mutex_unlock(&pss_data_mutex);
//
//}


void get_service_mode_by_id(guint8 id, PSSServiceMode* sm)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.service_mode_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.service_mode_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.service_modes[i].id == id)
			{
				*sm = pss_data.general_configuration.dispence_control.service_modes[i];
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void set_service_mode_by_id(guint8 id, PSSServiceMode sm)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.service_mode_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.service_mode_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.service_modes[i].id == id)
			{
				pss_data.general_configuration.dispence_control.service_modes[i] = sm;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void get_fuelling_mode_by_id(guint8 id, PSSFuellingMode* fm)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.fuelling_mode_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.fuelling_mode_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.fuelling_modes[i].id == id)
			{
				*fm = pss_data.general_configuration.dispence_control.fuelling_modes[i];
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void set_fuelling_mode_by_id(guint8 id, PSSFuellingMode fm)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.fuelling_mode_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.fuelling_mode_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.fuelling_modes[i].id == id)
			{
				pss_data.general_configuration.dispence_control.fuelling_modes[i] = fm;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void get_global_fuelling_limits(PSSDispenceLimits* dl)
{
	g_mutex_lock(&pss_data_mutex);

	*dl = pss_data.general_configuration.dispence_control.dispence_limits;

	g_mutex_unlock(&pss_data_mutex);
}

void set_global_fuelling_limits(PSSDispenceLimits dl)
{
	g_mutex_lock(&pss_data_mutex);

	pss_data.general_configuration.dispence_control.dispence_limits = dl;

	g_mutex_unlock(&pss_data_mutex);
}

void get_fuelling_mode_group_by_id(guint8 id, PSSFuellingModeGroup* fmg)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.fuelling_mode_group_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.fuelling_mode_group_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.fuelling_mode_groups[i].id == id)
			{
				*fmg = pss_data.general_configuration.dispence_control.fuelling_mode_groups[i];
				break;
			}
		}
	}
	g_mutex_unlock(&pss_data_mutex);
}

void get_fuelling_mode_group_by_index(guint8 index, PSSFuellingModeGroup* fmg)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.fuelling_mode_group_count > index)
	{
		*fmg = pss_data.general_configuration.dispence_control.fuelling_mode_groups[index];
	}
	g_mutex_unlock(&pss_data_mutex);
}

void set_fuelling_mode_group_by_id(guint8 id, PSSFuellingModeGroup fmg)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.general_configuration.dispence_control.fuelling_mode_group_count > 0)
	{
		for (guint i = 0; i < pss_data.general_configuration.dispence_control.fuelling_mode_group_count; i++)
		{
			if (pss_data.general_configuration.dispence_control.fuelling_mode_groups[i].id == id)
			{
				pss_data.general_configuration.dispence_control.fuelling_mode_groups[i] = fmg;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}


void get_fc_general_functions(PSSGeneralFunctions* gf)
{
	g_mutex_lock(&pss_data_mutex);

	*gf = pss_data.general_configuration.general_functions;

	g_mutex_unlock(&pss_data_mutex);
}

void update_fc_master_reset_date_and_time(time_t new_value)
{
	g_mutex_lock(&pss_data_mutex);
	pss_data.common.fc_status.fc_master_reset_date_and_time = new_value;
	pss_data.common.fc_status.fc_reset_date_and_time = new_value;
	g_mutex_unlock(&pss_data_mutex);
}

void update_fc_reset_date_and_time(time_t new_value)
{
	g_mutex_lock(&pss_data_mutex);
	pss_data.common.fc_status.fc_reset_date_and_time = new_value;
	g_mutex_unlock(&pss_data_mutex);
}

void update_fc_price_set_id(guint8 new_value)
{
	g_mutex_lock(&pss_data_mutex);
	pss_data.common.fc_status.fc_price_set_id = new_value;
	pss_data.common.fc_status.fc_price_set_date_and_time = time(NULL);
	g_mutex_unlock(&pss_data_mutex);
}

guint8 get_fc_operation_mode_no()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.common.fc_status.fc_operation_mode_no;

	g_mutex_unlock(&pss_data_mutex);

	return result;
}

void update_fc_operation_mode_no(guint8 new_value)
{
	g_mutex_lock(&pss_data_mutex);
	pss_data.common.fc_status.fc_operation_mode_no = new_value;
	pss_data.common.fc_status.fc_operation_mode_date_and_time = time(NULL);

	if (pss_data.devices.fuelling_points.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.fuelling_points.count; i++)
		{
			PSSFuellingPoint* fp = &pss_data.devices.fuelling_points.units[i];

			if (fp->operation_mode_count > 0)
			{
				for (guint8 j = 0; j < fp->operation_mode_count; j++)
				{
					PSSOperationMode* operation_mode =  &fp->operation_modes[j];
					if (operation_mode->id == new_value)
					{
						fp->operation_mode_no = new_value;
						break;
					}
				}
			}

		}
	}
	g_mutex_unlock(&pss_data_mutex);
}

void update_fc_single_price(guint8 price_group_id, guint8 grade_id, guint32 new_value)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_PRICE_GROUP_COUNT; i++)
	{
		PSSPriceGroup* price_group = &pss_data.general_configuration.general_functions.price_groups[i];

		if (price_group->id == price_group_id)
		{
			for (guint8 j = 0; j < MAX_GRADE_COUNT; j++)
			{
				PSSGradePrice* grade_price = &price_group->grade_prices[j];

				if (grade_price->grade_id == grade_id)
				{
					grade_price->price = new_value;
					g_mutex_unlock(&pss_data_mutex);
					return;
				}
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}


void master_reset()
{
	g_mutex_lock(&pss_data_mutex);

	//delete all price_sets

	pss_data.general_configuration.general_functions.price_group_count = 0;

	memset(&pss_data.general_configuration.general_functions.price_groups,0x00, sizeof(pss_data.general_configuration.general_functions.price_groups));

	g_mutex_unlock(&pss_data_mutex);
}

guint8 get_device_groups_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.fuelling_points.count > 0)
	{
		result++;
	}
	if (pss_data.devices.price_poles.count > 0)
	{
		result++;
	}
	if (pss_data.devices.tank_gauges.count > 0)
	{
		result++;
	}
	if (pss_data.devices.tank_gauge_sub_devices.count > 0)
	{
		result++;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_fuelling_point_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.fuelling_points.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tgs_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.tank_gauges.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tgsds_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.tank_gauge_sub_devices.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_price_pole_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.price_poles.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_server_device_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.server_devices.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}


guint8 get_tank_gauge_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.tank_gauges.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tank_gauge_sub_device_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.tank_gauge_sub_devices.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_fuelling_mode_group_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.general_configuration.dispence_control.fuelling_mode_group_count;

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_fuelling_point_id_by_index(guint8 index )
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index < MAX_FUELLING_POINT_COUNT)
	{
		result = pss_data.devices.fuelling_points.units[index].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tgs_id_by_index(guint8 index )
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index < MAX_TANK_COUNT)
	{
		result = pss_data.devices.tank_gauges.units[index].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_price_pole_id_by_index(guint8 index )
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index < MAX_PRICE_POLE_COUNT)
	{
		result = pss_data.devices.price_poles.units[index].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tank_gauge_id_by_index(guint8 index )
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index < MAX_TANK_COUNT)
	{
		result = pss_data.devices.tank_gauges.units[index].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

guint8 get_tank_gauge_sub_device_id_by_index(guint8 index )
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index < MAX_TANK_COUNT)
	{
		result = pss_data.devices.tank_gauge_sub_devices.units[index].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

void close_fp(guint8 fp_id )
{
	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* devices = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
	{
		if (i < devices->count)
		{
			PSSFuellingPoint* fp = &devices->units[i];

			if (fp->id == fp_id || fp_id == 0)
			{
				fp->is_close = TRUE;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}
void get_price_group_by_id(guint8 id, PSSPriceGroup* pg)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_PRICE_GROUP_COUNT; i++)
	{
		if (pss_data.general_configuration.general_functions.price_groups[i].id == id )
		{
			*pg = pss_data.general_configuration.general_functions.price_groups[i];
			break;
		}
	}
	g_mutex_unlock(&pss_data_mutex);

}

void reset_fp_param(guint8 fp_id )
{
	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* devices = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
	{
		if (i < devices->count)
		{
			PSSFuellingPoint* fp = &devices->units[i];

			if (fp->id == fp_id)
			{
				memset(&pss_data.devices.fuelling_points.units[i].preset_data, 0x00, sizeof(PSSPresetData));

				fp->sub_state&=~FP_SS_IS_PRESET;
				fp->sub_state &=~ FP_SS_IS_E_STOPPED;
				fp->sub_state &= ~FP_SS2_FUELLING_HALTED;

				fp->fuelling_volume = 0;
				fp->fuelling_money = 0;

				fp->reset = TRUE;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);

	save_pss_data();

}

void fp_price_update(PSSFuellingPoint fp, guint8 op_mode_no, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d update prices dispencer %d:", params->client_index, fp.id);

	if (fp.id > 0)
	{
		if (fp.operation_mode_no == op_mode_no)
		{
			if (fp.operation_mode_count > 0)
			{
				for (guint8 j = 0; j < fp.operation_mode_count; j++)
				{
					if (fp.operation_modes[j].id == op_mode_no)
					{
						if (fp.operation_modes[j].service_mode_count > 0)
						{
							//TODO берем первую попавшуюся SM, потом реализовать выбор

							PSSPriceGroup pg = {0x00};
							get_price_group_by_id(fp.operation_modes[j].service_modes[0].price_group_id, &pg);

							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d prices pg %d getted", params->client_index, fp.operation_modes[j].service_modes[0].price_group_id);


							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d Create price pack:", params->client_index);

							PricePacks price_packs = {0x00};

							price_packs.unit_count = MIN(fp.grade_option_count, MAX_NOZZLE_COUNT);

							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d nozzle pack count:", params->client_index, price_packs.unit_count);


							for (guint8 k = 0; k < fp.grade_option_count; k++)
							{
								if (fp.grade_options[k].id > 0)
								{
									price_packs.units[fp.grade_options[k].id - 1].id = fp.grade_options[k].id;

									for (guint8 l = 0; l < pg.grade_price_count; l++)
									{
										if (pg.grade_prices[l].grade_id == fp.grade_options[k].grade_id)
										{
											price_packs.units[fp.grade_options[k].id - 1].price = pg.grade_prices[l].price;
										}
									}

									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
											"client %d 	Nozzle num %d: price: %d", params->client_index,
											price_packs.units[fp.grade_options[k].id - 1].id,
											price_packs.units[fp.grade_options[k].id - 1].price);

								}
							}
							//ищем устройство на которое отправлять
							guint8 device_index = 0;
							if (get_client_device_index_by_id(params->client_index, fp.id, &device_index, psdt_DispencerController))
							{
								//если нашли - отправляем
								//TODO
								send_update_price_message(params, device_index, fp.id, &price_packs);

							}
						}
					}
				}
			}
		}
	}
}

void price_update(PSSPriceGroup* price_group, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable, "client %d update prices :", params->client_index);

	PricePacks price_packs = {0x00};

	price_packs.unit_count = MIN(price_group->grade_price_count, MAX_NOZZLE_COUNT);

	add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d price pack count:", params->client_index, price_packs.unit_count);

	for (guint8 i = 0; i < price_packs.unit_count; i++)
	{
		PSSGradePrice* grade_price = &price_group->grade_prices[i];
		price_packs.units[i].id = grade_price->grade_id;
		price_packs.units[i].price = grade_price->price;
	}

	//отправляем на все ценовые стеллы

	for (guint8 i = 0; i < MAX_SERVER_DEVICE_COUNT; i++)
	{
		if (get_client_device_type_by_index(params->client_index, i) == psdt_PricePoleController)
		{
			send_prices_message(params, i, &price_packs);

		}

	}
}

void stop_fp(guint8  fp_id, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;
	guint8 device_index = 0;
	guint32 port = get_client_port(params->client_index);

	if (fp_id > 0)
	{
		if (get_client_device_index_by_id(params->client_index, fp_id, &device_index, psdt_DispencerController))
		{
			send_stop_message(params, device_index, fp_id);

			g_mutex_lock(&pss_data_mutex);

			PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

			for (guint8 i = 0; i < fps->count; i++)
			{
				PSSFuellingPoint* fp = &fps->units[i];

				if (fp->id ==  fp_id)
				{
					fp->main_state = fpms_FuellingPaused;
				//	fp->sub_state = 251;
					break;
				}
			}

			g_mutex_unlock(&pss_data_mutex);

			send_fp_status_req_3(fp_id, params,  FALSE, port);

		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! device not found!", params->client_index);
		}
	}
	else
	{
		guint8 fp_count = get_fuelling_point_count();

		if (fp_count > 0)
		{
			for (guint8 i = 0; i < fp_count; i++)
			{
				guint8 tmp_fp_id = get_fuelling_point_id_by_index(i);

				if (get_client_device_index_by_id(params->client_index, tmp_fp_id, &device_index, psdt_DispencerController))
				{
					send_stop_message(params, device_index, fp_id);

					g_mutex_lock(&pss_data_mutex);

					PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

					for (guint8 i = 0; i < fps->count; i++)
					{
						PSSFuellingPoint* fp = &fps->units[i];

						if (fp->id ==  fp_id)
						{
							fp->main_state = fpms_FuellingPaused;
						//	fp->wait_send = TRUE;
							//fp->reset = TRUE;
						//	fp->sub_state = 251;
							break;
						}
					}

					g_mutex_unlock(&pss_data_mutex);

					send_fp_status_req_3(fp_id, params,  FALSE, port);

				}
				else
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		Error! device not found!", params->client_index);
				}

			}
		}
	}

}

void suspend_fp(guint8  fp_id, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;
	guint8 device_index = 0;
	guint32 port = get_client_port(params->client_index);

	if (fp_id > 0)
	{
		if (get_client_device_index_by_id(params->client_index, fp_id, &device_index, psdt_DispencerController))
		{
			send_suspend_message(params, device_index, fp_id);

			g_mutex_lock(&pss_data_mutex);

			PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

			for (guint8 i = 0; i < fps->count; i++)
			{
				PSSFuellingPoint* fp = &fps->units[i];

				if (fp->id ==  fp_id)
				{
					fp->main_state = fpms_FuellingPaused;
					fp->sub_state |= FP_SS_IS_E_STOPPED;
					break;
				}
			}

			g_mutex_unlock(&pss_data_mutex);

			send_fp_status_req_3(fp_id, params,  FALSE, port);

		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! device not found!", params->client_index);
		}
	}
	else
	{
		guint8 fp_count = get_fuelling_point_count();

		if (fp_count > 0)
		{
			for (guint8 i = 0; i < fp_count; i++)
			{
				guint8 tmp_fp_id = get_fuelling_point_id_by_index(i);

				if (get_client_device_index_by_id(params->client_index, tmp_fp_id, &device_index, psdt_DispencerController))
				{
					send_suspend_message(params, device_index, fp_id);

					g_mutex_lock(&pss_data_mutex);

					PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

					for (guint8 i = 0; i < fps->count; i++)
					{
						PSSFuellingPoint* fp = &fps->units[i];

						if (fp->id ==  fp_id)
						{
							fp->main_state = fpms_FuellingPaused;
							fp->sub_state |= FP_SS_IS_E_STOPPED;

						//	fp->wait_send = TRUE;
							//fp->reset = TRUE;

							break;
						}
					}

					g_mutex_unlock(&pss_data_mutex);

					send_fp_status_req_3(fp_id, params,  FALSE, port);

				}
				else
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		Error! device not found!", params->client_index);
				}

			}
		}
	}

}


void resume_fp(guint8  fp_id, PSSClientThreadFuncParam* params)
{
	LogParams* log_params = params->log_params;
	guint8 device_index = 0;
	guint32 port = get_client_port(params->client_index);

	if (fp_id > 0)
	{
		if (get_client_device_index_by_id(params->client_index, fp_id, &device_index, psdt_DispencerController))
		{
			send_resume_message(params, device_index, fp_id);

			g_mutex_lock(&pss_data_mutex);

			PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

			for (guint8 i = 0; i < fps->count; i++)
			{
				PSSFuellingPoint* fp = &fps->units[i];

				if (fp->id ==  fp_id)
				{
					fp->main_state = fpms_Fuelling;
					fp->sub_state &=~ FP_SS_IS_E_STOPPED;
					break;
				}
			}

			g_mutex_unlock(&pss_data_mutex);

			send_fp_status_req_3(fp_id, params,  FALSE, port);

		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! device not found!", params->client_index);
		}
	}
	else
	{
		guint8 fp_count = get_fuelling_point_count();

		if (fp_count > 0)
		{
			for (guint8 i = 0; i < fp_count; i++)
			{
				guint8 tmp_fp_id = get_fuelling_point_id_by_index(i);

				if (get_client_device_index_by_id(params->client_index, tmp_fp_id, &device_index, psdt_DispencerController))
				{
					send_resume_message(params, device_index, fp_id);

					g_mutex_lock(&pss_data_mutex);

					PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

					for (guint8 i = 0; i < fps->count; i++)
					{
						PSSFuellingPoint* fp = &fps->units[i];

						if (fp->id ==  fp_id)
						{
							fp->main_state = fpms_Fuelling;
							fp->sub_state &=~ FP_SS_IS_E_STOPPED;
							break;
						}
					}

					g_mutex_unlock(&pss_data_mutex);

					send_fp_status_req_3(fp_id, params,  FALSE, port);

				}
				else
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		Error! device not found!", params->client_index);
				}

			}
		}
	}

}

void update_fp(PSSFuellingPoint fp)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
	{
		if (pss_data.devices.fuelling_points.units[i].id == fp.id )
		{
			pss_data.devices.fuelling_points.units[i] = fp;
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}


void reset_fp(guint8  fp_id, PSSClientThreadFuncParam* params, PSSCommand pss_command, guint8 subcode, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;
	guint8 device_index = 0;

	if (get_client_device_index_by_id(params->client_index, fp_id, &device_index, psdt_DispencerController))
	{
		if (get_current_command(params->client_index, device_index) == hsc_None)
		{
			set_current_command(params->client_index, device_index, hsc_DCReset);
			set_pss_command(params->client_index, device_index, pss_command);
			set_pss_command_subcode(params->client_index, device_index, subcode);
			set_pss_ex_mode(params->client_index, device_index, ex_mode);

			set_current_command_sent_time(params->client_index, device_index, get_date_time());
			send_reset_message(params, device_index, fp_id);

			reset_fp_param(fp_id);
		}
		else
		{
			send_rejected_reply(params, ex_mode, pss_command, subcode);
		}
	}
	else
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
				"client %d 		Error! device not found!", params->client_index);
	}

}

void authorize_fp(PSSFuellingPoint fp, PSSClientThreadFuncParam* params, PSSCommand pss_command, guint8 subcode, gboolean ex_mode)
{
	LogParams* log_params = params->log_params;

	if (fp.grade_id > 0)
	{
		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
				"client %d authorize Fp %d (GradeID %d)", params->client_index, fp.id, fp.grade_id);

		guint8 operation_mode_no = 0;

		if (fp.operation_mode_no == 0)
		{
			operation_mode_no = get_fc_operation_mode_no();
		}
		else
		{
			operation_mode_no = fp.operation_mode_no;
		}

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
				"client %d 	operation mode no: %d", params->client_index, operation_mode_no);


		if (fp.operation_mode_count > 0)
		{
			for (guint8 i = 0; i < fp.operation_mode_count; i++)
			{
				if (fp.operation_modes[i].id == operation_mode_no)
				{
					if (fp.operation_modes[i].service_mode_count > 0)
					{
						for (guint8 j = 0; j < fp.operation_modes[i].service_mode_count; j++ )
						{
							if (fp.preset_data.sm_id == 0 || fp.preset_data.sm_id == fp.operation_modes[i].service_modes[j].service_mode_id)
							{
								fp.sm_id = fp.operation_modes[i].service_modes[j].service_mode_id;

								if (fp.preset_data.pg_id == 0)
								{
									fp.preset_data.pg_id = fp.operation_modes[i].service_modes[j].price_group_id;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
											"client %d 		set price group id: %d", params->client_index, fp.preset_data.pg_id);
								}

								if (fp.preset_data.fmg_id == 0)
								{
									fp.preset_data.fmg_id = fp.operation_modes[i].service_modes[j].fuelling_mode_group_id;
									add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
											"client %d 		set fuelling mode group id: %d", params->client_index, fp.preset_data.fmg_id);
								}
							}
						}
					}
				}
			}
		}
		//TODO
		//определить цену

		guint32 price = 0;

		if (fp.preset_data.pg_id > 0)
		{
			PSSPriceGroup pg = {0x00};
			get_price_group_by_id(fp.preset_data.pg_id, &pg);

			if (pg.id == fp.preset_data.pg_id)
			{
				if (pg.grade_price_count > 0)
				{
					for (guint8 i = 0; i < pg.grade_price_count; i++)
					{
						if (pg.grade_prices[i].grade_id == fp.grade_id)
						{
							price = pg.grade_prices[i].price;
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Set price for grade ID %d: %d", params->client_index, fp.grade_id, price);
							break;
						}
						else
						{
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		pg.grade_prices[i].grade_id = %d!", params->client_index, pg.grade_prices[i].grade_id);

						}
					}
				}
				else
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		Error! Price group ID %d is empty!", params->client_index, fp.preset_data.pg_id);
					send_rejected_reply(params, ex_mode, pss_command, subcode);

					return;

				}
			}
			else
			{
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
						"client %d 		Error! Price group ID %d not found!", params->client_index, fp.preset_data.pg_id);
				send_rejected_reply(params, ex_mode, pss_command, subcode);

				return;
			}
		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! Price group ID %d not found!", params->client_index, fp.preset_data.pg_id);
			send_rejected_reply(params, ex_mode, pss_command, subcode);

			return;
		}

		if (price == 0)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! Price not found!", params->client_index);
			send_rejected_reply(params, ex_mode, pss_command, subcode);

			return;
		}

		guint volume_limit = 0;
		guint money_limit = 0;
		guint8 fm_id = 0;

		add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
				"client %d 		Limits:", params->client_index);

		//определяем лимиты
		if (fp.preset_data.fmg_id > 0)
		{


			PSSFuellingModeGroup fmg = {0x00};
			get_fuelling_mode_group_by_id(fp.preset_data.fmg_id, &fmg);

			if (fmg.fuelling_mode_group_item_count > 0)
			{
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
						"client %d 		FMG: %d", params->client_index, fmg.id);


				for (guint8 i = 0; i < fmg.fuelling_mode_group_item_count; i++)
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		%d - %d", params->client_index,fmg.fuelling_mode_group_items[i].grade_id, fp.grade_id);


					if (fmg.fuelling_mode_group_items[i].grade_id == fp.grade_id)
					{
						fm_id = fmg.fuelling_mode_group_items[i].fuelling_mode_id;
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		set fuelling mode ID: %d", params->client_index, fm_id);
						break;
					}
				}
			}
			else
			{
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
						"client %d 		Error! Fuelling modes count = 0!", params->client_index);
				send_rejected_reply(params, ex_mode, pss_command, subcode);

				return;

			}

			if (fm_id > 0)
			{
				PSSFuellingMode fm = {0x00};
				get_fuelling_mode_by_id(fm_id, &fm);

				if (fm.id == fm_id)
				{
					volume_limit = fm.max_trans_volume;
					money_limit = fm.max_trans_money;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		set volume limit: %d, money_limit: %d", params->client_index, volume_limit, money_limit);
				}
				else
				{
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		Error! Fuelling mode ID %d not found!", params->client_index, fm_id);
					send_rejected_reply(params, ex_mode, pss_command, subcode);

					return;
				}
			}
			else
			{
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
						"client %d 		Error! Fuelling mode ID%d not found!", params->client_index, fm_id);
				send_rejected_reply(params, ex_mode, pss_command, subcode);

				return;
			}
		}
		else
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! Fuelling mode group ID %d not found!", params->client_index, fp.preset_data.fmg_id);
			send_rejected_reply(params, ex_mode, pss_command, subcode);

			return;
		}

		//определить номер пистолета
		guint8 nozzle_num = 0;

		if (fp.grade_option_count > 0)
		{
			for (guint8 i = 0; i < fp.grade_option_count; i++)
			{
				if (fp.grade_id == fp.grade_options[i].grade_id)
				{
					nozzle_num = fp.grade_options[i].id;
					add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
							"client %d 		set nozzle num: %d", params->client_index, nozzle_num);
					break;
				}
			}
		}

		if (nozzle_num == 0)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! Nozzle num not found!", params->client_index);
			send_rejected_reply(params, ex_mode, pss_command, subcode);

			return;
		}

		//проверить разрешения пистолета

		if (fp.preset_data.valid_grades_count > 0)
		{
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Check grade ID:", params->client_index);

			gboolean grade_is_present = FALSE;
			for (guint8 i = 0; i < fp.preset_data.valid_grades_count; i++)
			{
				if (fp.preset_data.valid_grades[i] == fp.grade_id)
				{
					grade_is_present = TRUE;
					break;
				}
			}
			if (!grade_is_present)
			{
				//TODO
				add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
						"client %d 		Error! Grade fault!", params->client_index);
				send_rejected_reply(params, ex_mode, pss_command, subcode);

				return;
			}
		}
		//найти устройство
		guint8 device_index = 0;
		if (get_client_device_index_by_id(params->client_index, fp.id, &device_index, psdt_DispencerController))
		{
			fp.preset_data.grade_id = fp.grade_id;

			switch(fp.preset_data.preset_type)
			{
				case ppt_NoLimit:
					if (volume_limit > 0)
					{
						if (get_current_command(params->client_index, device_index) == hsc_None)
						{
							set_current_command(params->client_index, device_index, hsc_DCSetVolumeDose);
							set_pss_command(params->client_index, device_index, pss_command);
							set_pss_command_subcode(params->client_index, device_index, subcode);
							set_pss_ex_mode(params->client_index, device_index, ex_mode);

							set_current_command_sent_time(params->client_index, device_index, get_date_time());
							send_volume_preset_message(params, device_index, fp.id, nozzle_num, price, volume_limit);
						}
						else
						{
							send_rejected_reply(params, ex_mode, pss_command, subcode);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Error! Current command not empty empty!", params->client_index);
						}
					}
					else
					{
						send_rejected_reply(params, ex_mode, pss_command, subcode);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		Error! Volume limit is empty!", params->client_index);

					}
					break;

				case ppt_VolumePresetLimit:
					if (fp.preset_data.preset_value > 0)
					{
						if (get_current_command(params->client_index, device_index) == hsc_None)
						{
							set_current_command(params->client_index, device_index, hsc_DCSetVolumeDose);
							set_pss_command(params->client_index, device_index, pss_command);
							set_pss_command_subcode(params->client_index, device_index, subcode);
							set_pss_ex_mode(params->client_index, device_index, ex_mode);

							set_current_command_sent_time(params->client_index, device_index, get_date_time());
							send_volume_preset_message(params, device_index, fp.id, nozzle_num, price, fp.preset_data.preset_value);
						}
						else
						{
							send_rejected_reply(params, ex_mode, pss_command, subcode);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Error! Current command not empty empty!", params->client_index);
						}
					}
					else
					{
						send_rejected_reply(params, ex_mode, pss_command, subcode);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		Error! Preset volume is empty!", params->client_index);
					}
					break;

				case ppt_MoneyPresetLimit:
					if (fp.preset_data.preset_value > 0)
					{
						if (get_current_command(params->client_index, device_index) == hsc_None)
						{
							set_current_command(params->client_index, device_index, hsc_DCSetSumDose);
							set_pss_command(params->client_index, device_index, pss_command);
							set_pss_command_subcode(params->client_index, device_index, subcode);
							set_pss_ex_mode(params->client_index, device_index, ex_mode);

							set_current_command_sent_time(params->client_index, device_index, get_date_time());
							send_amount_preset_message(params, device_index, fp.id, nozzle_num, price, fp.preset_data.preset_value);
						}
						else
						{
							send_rejected_reply(params, ex_mode, pss_command, subcode);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Error! Current command not empty empty!", params->client_index);
						}


					}
					else
					{
						send_rejected_reply(params, ex_mode, pss_command, subcode);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		Error! Preset amount is empty!", params->client_index);

					}
					break;

				case ppt_MoneyFloorLimit:  //TODO
					if (money_limit > 0)
					{
						if (get_current_command(params->client_index, device_index) == hsc_None)
						{
							set_current_command(params->client_index, device_index, hsc_DCSetSumDose);
							set_pss_command(params->client_index, device_index, pss_command);
							set_pss_command_subcode(params->client_index, device_index, subcode);
							set_pss_ex_mode(params->client_index, device_index, ex_mode);

							set_current_command_sent_time(params->client_index, device_index, get_date_time());
							send_amount_preset_message(params, device_index, fp.id, nozzle_num, price, money_limit);
						}
						else
						{
							send_rejected_reply(params, ex_mode, pss_command, subcode);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Error! Current command not empty empty!", params->client_index);
						}

					}
					else
					{
						send_rejected_reply(params, ex_mode, pss_command, subcode);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		Error! Floor money limit is empty!", params->client_index);
					}
					break;

				case ppt_VolumeFloorLimit: //TODO
					if (volume_limit > 0)
					{
						if (get_current_command(params->client_index, device_index) == hsc_None)
						{
							set_current_command(params->client_index, device_index, hsc_DCSetVolumeDose);
							set_pss_command(params->client_index, device_index, pss_command);
							set_pss_command_subcode(params->client_index, device_index, subcode);
							set_pss_ex_mode(params->client_index, device_index, ex_mode);

							set_current_command_sent_time(params->client_index, device_index, get_date_time());
							send_volume_preset_message(params, device_index, fp.id, nozzle_num, price, volume_limit);
						}
						else
						{
							send_rejected_reply(params, ex_mode, pss_command, subcode);
							add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
									"client %d 		Error! Current command not empty empty!", params->client_index);
						}


					}
					else
					{
						send_rejected_reply(params, ex_mode, pss_command, subcode);
						add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
								"client %d 		Error! Floor volume limit is empty!", params->client_index);

					}
					break;
			}

			fp.main_state = fpms_PreAuthorized;
		}
		else
		{
			send_rejected_reply(params, ex_mode, pss_command, subcode);
			add_log(log_params, TRUE, TRUE, params->log_trace, params->log_enable,
					"client %d 		Error! device not found!", params->client_index);
		}

	}
	update_fp(fp);
}

void delete_fp_sup_trans(guint8 fp_id, guint32 trans_seq_no)
{
	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < fps->count; i++)
	{
		PSSFuellingPoint* fp = &fps->units[i];

		if (fp->id ==  fp_id)
		{
			PSSTransactions* trs = &fp->sup_transactions;

			if (trs->count > 0)
			{
				gboolean flag = FALSE;

				for (guint8 j = 0; j < trs->count; j++ )
				{
					PSSTransaction* tr = &trs->units[j];

					if (tr->trans_seq_no == trans_seq_no || flag)
					{
						if ( j < (MAX_TRANSACTION_SEQ_COUNT - 1))
						{
							pss_data.devices.fuelling_points.units[i].sup_transactions.units[j] = pss_data.devices.fuelling_points.units[i].sup_transactions.units[j+1];

							pss_data.devices.fuelling_points.units[i].sup_transactions.count--;
							flag = TRUE;
						}
						else
						{
							memset(&pss_data.devices.fuelling_points.units[i].sup_transactions.units[j], 0x00, sizeof(PSSTransaction));
						}
					}
				}
			}
			break;
		}
	}


	g_mutex_unlock(&pss_data_mutex);

}

void get_sup_transaction(guint8 fp_id, guint32 trans_seq_no, PSSTransaction* transaction)
{
	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < fps->count; i++)
	{
		PSSFuellingPoint* fp = &fps->units[i];

		if (fp->id ==  fp_id)
		{
			PSSTransactions* trs = &fp->sup_transactions;

			if (trs->count > 0)
			{
				for (guint8 j = 0; j < trs->count; j++ )
				{
					PSSTransaction* tr = &trs->units[j];

					if (tr->trans_seq_no == trans_seq_no)
					{
						*transaction = pss_data.devices.fuelling_points.units[i].sup_transactions.units[j];
						break;
					}
				}
			}
			break;
		}
	}


	g_mutex_unlock(&pss_data_mutex);

}

void set_fp_main_state_by_id(guint8 fp_id, PSSFpMainState new_value )
{
	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < fps->count; i++)
	{
		PSSFuellingPoint* fp = &fps->units[i];

		if (fp->id ==  fp_id)
		{
			fp->main_state = new_value;
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

PSSFpMainState get_fp_main_state_by_id(guint8 fp_id)
{
	PSSFpMainState result = fpms_Unconfigured;

	g_mutex_lock(&pss_data_mutex);

	PSSFuellingPoints* fps = &pss_data.devices.fuelling_points;

	for (guint8 i = 0; i < fps->count; i++)
	{
		PSSFuellingPoint* fp = &fps->units[i];

		if (fp->id ==  fp_id)
		{
			result = fp->main_state;
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

	return result;
}


void set_fps_main_state(PSSServerDeviceUnits* ids, PSSFpMainState new_value )
{
	if (ids->count > 0)
	{
		g_mutex_lock(&pss_data_mutex);

		PSSFuellingPoints* devices = &pss_data.devices.fuelling_points;

		for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
		{
			if (i < devices->count)
			{
				PSSFuellingPoint* fp = &devices->units[i];

				for (guint8 j = 0; j < ids->count; j++)
				{
					if (fp->id == ids->units[j])
					{
						fp->main_state = new_value;
					}

				}
			}
		}

	g_mutex_unlock(&pss_data_mutex);
	}
}
void set_fps_sub_state(PSSServerDeviceUnits* ids, guint8 new_value )
{
	if (ids->count > 0)
	{
		g_mutex_lock(&pss_data_mutex);

		PSSFuellingPoints* devices = &pss_data.devices.fuelling_points;

		for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
		{
			if (i < devices->count)
			{
				PSSFuellingPoint* fp = &devices->units[i];

				for (guint8 j = 0; j < ids->count; j++)
				{
					if (fp->id == ids->units[j])
					{
						fp->sub_state = new_value;
					}

				}
			}
		}

	g_mutex_unlock(&pss_data_mutex);
	}
}


void delete_fuelling_point(guint8 fc_device_id)
{
	for (guint8 i = 0; i < pss_data.devices.fuelling_points.count; i++)
	{
		if (pss_data.devices.fuelling_points.units[i].id == fc_device_id)
		{
			if (i < pss_data.devices.fuelling_points.count - 1)
			{
				for (guint8 j = i; j < pss_data.devices.fuelling_points.count - 1; j++)
					pss_data.devices.fuelling_points.units[j] = pss_data.devices.fuelling_points.units[j + 1];
			}
			memset(&pss_data.devices.fuelling_points.units[pss_data.devices.fuelling_points.count], 0x00, sizeof(PSSFuellingPoint));
			pss_data.devices.fuelling_points.count--;
		}
	}
}

void delete_price_pole(guint8 fc_device_id)
{
	for (guint8 i = 0; i < pss_data.devices.price_poles.count; i++)
	{
		if (pss_data.devices.price_poles.units[i].id == fc_device_id)
		{
			if (i < pss_data.devices.price_poles.count - 1)
			{
				for (guint8 j = i; j < pss_data.devices.price_poles.count - 1; j++)
					pss_data.devices.price_poles.units[j] = pss_data.devices.price_poles.units[j + 1];

			}
			memset(&pss_data.devices.price_poles.units[pss_data.devices.price_poles.count], 0x00, sizeof(PSSPricePole));
			pss_data.devices.price_poles.count--;
		}
	}
}

void delete_tank_gauge(guint8 fc_device_id)
{
	for (guint8 i = 0; i < pss_data.devices.tank_gauges.count; i++)
	{
		if (pss_data.devices.tank_gauges.units[i].id == fc_device_id)
		{
			if (i < pss_data.devices.tank_gauges.count - 1)
			{
				for (guint8 j = i; j < pss_data.devices.tank_gauges.count - 1; j++)
					pss_data.devices.tank_gauges.units[j] = pss_data.devices.tank_gauges.units[j + 1];
			}
			memset(&pss_data.devices.tank_gauges.units[pss_data.devices.tank_gauges.count], 0x00, sizeof(PSSTankGauge));
			pss_data.devices.tank_gauges.count--;
		}
	}
}

void delete_tank_gauge_sub_device(guint8 fc_device_id)
{
	for (guint8 i = 0; i < pss_data.devices.tank_gauge_sub_devices.count; i++)
	{
		if (pss_data.devices.tank_gauge_sub_devices.units[i].id == fc_device_id)
		{
			if (i < pss_data.devices.tank_gauge_sub_devices.count - 1)
			{
				for (guint8 j = i; j < pss_data.devices.tank_gauge_sub_devices.count - 1; j++)
					pss_data.devices.tank_gauge_sub_devices.units[j] = pss_data.devices.tank_gauge_sub_devices.units[j + 1];
			}
			memset(&pss_data.devices.tank_gauge_sub_devices.units[pss_data.devices.tank_gauge_sub_devices.count], 0x00, sizeof(PSSTankGaugeSubDevice));
			pss_data.devices.tank_gauge_sub_devices.count--;
		}
	}
}

gboolean clear_ext_install_data(guint8 install_msg_code_0, guint8 install_msg_code_1,guint8 install_msg_code_2, guint8 fc_device_id)
{
	gboolean result = FALSE;

	g_mutex_lock(&pss_data_mutex);

	if (install_msg_code_1 == 0 && install_msg_code_2 == 0)
	{
		if (fc_device_id == 0)
		{
			memset(&pss_data.devices.fuelling_points, 0x00, sizeof(pss_data.devices.fuelling_points));
			memset(&pss_data.devices.price_poles, 0x00, sizeof(pss_data.devices.price_poles));
			memset(&pss_data.devices.tank_gauges, 0x00, sizeof(pss_data.devices.tank_gauges));
			memset(&pss_data.devices.tank_gauge_sub_devices, 0x00, sizeof(pss_data.devices.tank_gauge_sub_devices));
			result = TRUE;
		}
		else
		{
			delete_fuelling_point(fc_device_id);
			delete_price_pole(fc_device_id);
			delete_tank_gauge(fc_device_id);
			delete_tank_gauge_sub_device(fc_device_id);
		}
	}
	else
	{
		switch(install_msg_code_1)
		{
			case 0x01:
				switch(install_msg_code_1)
				{
					case 0x06:
						if (fc_device_id == 0)
						{
							memset(&pss_data.devices.tank_gauge_sub_devices, 0x00, sizeof(pss_data.devices.tank_gauge_sub_devices));
							result = TRUE;
						}
						else
						{
							delete_tank_gauge_sub_device(fc_device_id);
						}

						break;
				}
				break;

		}

	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}


gboolean clear_install_data(guint8 install_msg_code, guint8 fc_device_id)
{
	gboolean result = FALSE;

	g_mutex_lock(&pss_data_mutex);

	switch (install_msg_code)
	{
		case ALL_DEVICES_INSTALL_MSG_CODE:
			if (fc_device_id == 0)
			{
				memset(&pss_data.devices.fuelling_points, 0x00, sizeof(pss_data.devices.fuelling_points));
				memset(&pss_data.devices.price_poles, 0x00, sizeof(pss_data.devices.price_poles));
				memset(&pss_data.devices.tank_gauges, 0x00, sizeof(pss_data.devices.tank_gauges));
				memset(&pss_data.devices.tank_gauge_sub_devices, 0x00, sizeof(pss_data.devices.tank_gauge_sub_devices));
				result = TRUE;
			}
			else
			{
				delete_fuelling_point(fc_device_id);
				delete_price_pole(fc_device_id);
				delete_tank_gauge(fc_device_id);
				delete_tank_gauge_sub_device(fc_device_id);
			}
			break;

		case FUELLING_POINT_INSTALL_MSG_CODE:
			if (fc_device_id == 0)
			{
				memset(&pss_data.devices.fuelling_points, 0x00, sizeof(pss_data.devices.fuelling_points));
				result = TRUE;
			}
			else
			{
				delete_fuelling_point(fc_device_id);
			}
			break;

		case PRICE_POLE_INSTALL_MSG_CODE:
			if (fc_device_id == 0)
			{
				memset(&pss_data.devices.price_poles, 0x00, sizeof(pss_data.devices.price_poles));
				result = TRUE;
			}
			else
			{
				delete_price_pole(fc_device_id);
			}
			break;

		case TANK_GAUGE_INSTALL_MSG_CODE:
			if (fc_device_id == 0)
			{
				memset(&pss_data.devices.tank_gauges, 0x00, sizeof(pss_data.devices.tank_gauges));
				result = TRUE;
			}
			else
			{
				delete_tank_gauge(fc_device_id);
			}
			break;


	}

	g_mutex_unlock(&pss_data_mutex);

	return result;

}

void get_general_params(PSSGeneralParams* params)
{
	g_mutex_lock(&pss_data_mutex);

	*params = pss_data.general_configuration.general_params;

	g_mutex_unlock(&pss_data_mutex);
}

void get_server_device_by_index(guint8 index, PSSServerDevice* device)
{
	if (index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_data_mutex);

		*device = pss_data.devices.server_devices.units[index];

		g_mutex_unlock(&pss_data_mutex);
	}
}

void get_server_device_guid_by_index(guint8 index, gchar** guid)
{
	if (index < MAX_SERVER_DEVICE_COUNT)
	{
		g_mutex_lock(&pss_data_mutex);

		if (pss_data.devices.server_devices.units[index].guid != NULL)
		{
//			if (guid!=NULL)
//			{
//				g_free(guid);
//				guid = NULL;
//
//			}

			*guid = g_strdup(pss_data.devices.server_devices.units[index].guid);
		}

		g_mutex_unlock(&pss_data_mutex);
	}
}

void get_server_devices(PSSServerDevices* devices)
{
	g_mutex_lock(&pss_data_mutex);

	*devices = pss_data.devices.server_devices;

	g_mutex_unlock(&pss_data_mutex);
}

void set_general_params(PSSGeneralParams params)
{
	g_mutex_lock(&pss_data_mutex);

	pss_data.general_configuration.general_params = params;

	g_mutex_unlock(&pss_data_mutex);
}

gint8 get_new_grade_option_index(PSSFuellingPoint* fp, guint8 id)
{
	gint8 result = -1;

	for (guint8 i = 0; i < MAX_NOZZLE_COUNT; i++)
	{
		if (fp->grade_options[i].id == id || fp->grade_options[i].id == 0)
		{
			return i;
		}
	}

	return result;
}

void update_fp_operation_mode(PSSFuellingPoint* fp, PSSOperationMode om)
{
	for (guint8 i = 0; i < MAX_OPERATION_MODE_COUNT; i++)
	{
		if (fp->operation_modes[i].id == om.id)
		{
			fp->operation_modes[i] = om;
			break;
		}
		else if (fp->operation_modes[i].id == 0)
		{
			fp->operation_modes[i] = om;
			fp->operation_mode_count++;
			break;
		}
	}

}

void install_new_fp(PSSFuellingPoint new_fp)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_FUELLING_POINT_COUNT; i++)
	{
		if (pss_data.devices.fuelling_points.units[i].id == new_fp.id )
		{
			pss_data.devices.fuelling_points.units[i] = new_fp;
			g_printf("		update fuelling point id: %d\n",new_fp.id);

			break;
		}
		else if (pss_data.devices.fuelling_points.units[i].id == 0)
		{
			pss_data.devices.fuelling_points.units[i] = new_fp;
			pss_data.devices.fuelling_points.count++;
			g_printf("		install new fuelling point id: %d\n",new_fp.id);
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}

void install_new_tg(PSSTankGauge new_tg)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (pss_data.devices.tank_gauges.units[i].id == new_tg.id )
		{
			pss_data.devices.tank_gauges.units[i] = new_tg;
			g_printf("		update tank gauge id: %d\n",new_tg.id);

			break;
		}
		else if (pss_data.devices.tank_gauges.units[i].id == 0)
		{
			pss_data.devices.tank_gauges.units[i] = new_tg;
			pss_data.devices.tank_gauges.count++;
			g_printf("		install new tank gauge id: %d\n",new_tg.id);
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}

void install_new_tgsd(PSSTankGaugeSubDevice new_tgsd)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (pss_data.devices.tank_gauge_sub_devices.units[i].id == new_tgsd.id )
		{
			pss_data.devices.tank_gauge_sub_devices.units[i] = new_tgsd;
			g_printf("		update tank gauge sub device id: %d\n",new_tgsd.id);

			break;
		}
		else if (pss_data.devices.tank_gauge_sub_devices.units[i].id == 0)
		{
			pss_data.devices.tank_gauge_sub_devices.units[i] = new_tgsd;
			pss_data.devices.tank_gauge_sub_devices.count++;
			g_printf("		install new tank gauge sub device id: %d\n",new_tgsd.id);
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}


void install_new_pp(PSSPricePole new_pp)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_PRICE_POLE_COUNT; i++)
	{
		if (pss_data.devices.price_poles.units[i].id == new_pp.id )
		{
			pss_data.devices.price_poles.units[i] = new_pp;
			g_printf("		update price pole id: %d\n",new_pp.id);

			break;
		}
		else if (pss_data.devices.price_poles.units[i].id == 0)
		{
			pss_data.devices.price_poles.units[i] = new_pp;
			pss_data.devices.price_poles.count++;
			g_printf("		install new price pole id: %d\n",new_pp.id);
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}

void update_tg_tank_height(guint8 tg_id, guint32 tank_height)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if ((pss_data.devices.tank_gauges.units[i].id == tg_id) || ( (pss_data.devices.tank_gauges.units[i].id > 0) &&  tg_id == 0) )
		{
			pss_data.devices.tank_gauges.units[i].tank_height = tank_height;
			g_printf("		update tank gauge id %d height: %d\n",pss_data.devices.tank_gauges.units[i].id, tank_height);
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}

void update_tg_point(guint8 tg_id, guint8 index_point, guint32 volume)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if ((pss_data.devices.tank_gauges.units[i].id == tg_id) || ( (pss_data.devices.tank_gauges.units[i].id > 0) &&  tg_id == 0) )
		{
			if (index_point < MAX_TGS_POINT_COUNT)
				pss_data.devices.tank_gauges.units[i].points[index_point] = volume;

		}
	}

	g_mutex_unlock(&pss_data_mutex);

}

guint8 get_price_group_id_by_index(guint index_price_group)
{
	guint8 price_group_id = 0;

	g_mutex_lock(&pss_data_mutex);

	if (index_price_group < pss_data.general_configuration.general_functions.price_group_count)
	{
		price_group_id = pss_data.general_configuration.general_functions.price_groups[index_price_group].id;
	}

	g_mutex_unlock(&pss_data_mutex);

	return price_group_id;
}


void get_tg_by_id(guint8 id, PSSTankGauge* tg)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (pss_data.devices.tank_gauges.units[i].id == id )
		{
			*tg = pss_data.devices.tank_gauges.units[i];
			break;
		}
	}
	g_mutex_unlock(&pss_data_mutex);

}
void get_tgsd_by_id(guint8 id, PSSTankGaugeSubDevice* tgsd)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (pss_data.devices.tank_gauge_sub_devices.units[i].id == id )
		{
			*tgsd = pss_data.devices.tank_gauge_sub_devices.units[i];
			break;
		}
	}
	g_mutex_unlock(&pss_data_mutex);

}

void get_pp_by_id(guint8 id, PSSPricePole* pp)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_PRICE_POLE_COUNT; i++)
	{
		if (pss_data.devices.price_poles.units[i].id == id )
		{
			*pp = pss_data.devices.price_poles.units[i];
			break;
		}
	}
	g_mutex_unlock(&pss_data_mutex);

}

void get_pp_by_index(guint8 index, PSSPricePole* pp)
{
	g_mutex_lock(&pss_data_mutex);

	if (index < pss_data.devices.price_poles.count )
	{
		*pp = pss_data.devices.price_poles.units[index];
	}

	g_mutex_unlock(&pss_data_mutex);

}

guint8 get_pp_count()
{
	guint8 result = 0;

	g_mutex_lock(&pss_data_mutex);

	result = pss_data.devices.price_poles.count;

	g_mutex_unlock(&pss_data_mutex);

	return result;
}


void update_price_group(PSSPriceGroup price_group)
{
	g_mutex_lock(&pss_data_mutex);

	for (guint8 i = 0; i < MAX_PRICE_GROUP_COUNT; i++)
	{
		if (pss_data.general_configuration.general_functions.price_groups[i].id == price_group.id )
		{
			pss_data.general_configuration.general_functions.price_groups[i] = price_group;
			g_printf("		update price group id: %d\n",price_group.id);

			break;
		}
		else if (pss_data.general_configuration.general_functions.price_groups[i].id == 0)
		{
			pss_data.general_configuration.general_functions.price_groups[i] = price_group;
			pss_data.general_configuration.general_functions.price_group_count++;
			g_printf("		install new price group id: %d\n",price_group.id);
			break;
		}
	}

	g_mutex_unlock(&pss_data_mutex);

}


void get_fuelling_point_by_id(guint8 id, PSSFuellingPoint* fp)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.fuelling_points.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.fuelling_points.count; i++)
		{
			if (pss_data.devices.fuelling_points.units[i].id == id)
			{
				*fp = pss_data.devices.fuelling_points.units[i];
				break;
			}
		}
	}
	g_mutex_unlock(&pss_data_mutex);
}

void get_tgs_by_id(guint8 id, PSSTankGauge* tg)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.tank_gauges.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.tank_gauges.count; i++)
		{
			if (pss_data.devices.tank_gauges.units[i].id == id)
			{
				*tg = pss_data.devices.tank_gauges.units[i];
				break;
			}
		}
	}
	g_mutex_unlock(&pss_data_mutex);
}

void get_tank_by_id(guint8 id, PSSTank* tank)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.tanks.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.tanks.count; i++)
		{
			if (pss_data.devices.tanks.units[i].id == id)
			{
				*tank = pss_data.devices.tanks.units[i];
				break;
			}
		}
	}
	g_mutex_unlock(&pss_data_mutex);
}



void get_fuelling_point_by_index(guint8 index, PSSFuellingPoint* fp)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.fuelling_points.count > 0)
	{
		if (index < pss_data.devices.fuelling_points.count)
		{
			*fp = pss_data.devices.fuelling_points.units[index];
		}
	}
	g_mutex_unlock(&pss_data_mutex);
}

void set_fuelling_point_by_id(guint8 id, PSSFuellingPoint fp)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.fuelling_points.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.fuelling_points.count; i++)
		{
			if (pss_data.devices.fuelling_points.units[i].id == id)
			{
				pss_data.devices.fuelling_points.units[i] = fp;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void set_tank_by_id(guint8 id, PSSTank tank)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.tanks.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.tanks.count; i++)
		{
			if (pss_data.devices.tanks.units[i].id == id)
			{
				pss_data.devices.tanks.units[i] = tank;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}
void set_pp_by_id(guint8 id, PSSPricePole pp)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.price_poles.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.price_poles.count; i++)
		{
			if (pss_data.devices.price_poles.units[i].id == id)
			{
				pss_data.devices.price_poles.units[i] = pp;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void set_tgs_by_id(guint8 id, PSSTankGauge tank_gauge)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.tank_gauges.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.tank_gauges.count; i++)
		{
			if (pss_data.devices.tank_gauges.units[i].id == id)
			{
				pss_data.devices.tank_gauges.units[i] = tank_gauge;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}

void set_tgsds_by_id(guint8 id, PSSTankGaugeSubDevice tank_gauge_sub_device)
{
	g_mutex_lock(&pss_data_mutex);

	if (pss_data.devices.tank_gauge_sub_devices.count > 0)
	{
		for (guint i = 0; i < pss_data.devices.tank_gauge_sub_devices.count; i++)
		{
			if (pss_data.devices.tank_gauge_sub_devices.units[i].id == id)
			{
				pss_data.devices.tank_gauge_sub_devices.units[i] = tank_gauge_sub_device;
				break;
			}
		}
	}

	g_mutex_unlock(&pss_data_mutex);
}


