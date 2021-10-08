#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "ifsf.h"
#include "config.h"
#include "ifsf_func.h"

guint8			active_disp_index;
guint8			dispencer_count;
Dispencer		dispencers[MAX_DISP_COUNT] = {0x00};
IFSFNode		nodes[MAX_NODE_COUNT] = {0x00};
GMutex			driver_mutex;

void init_driver_mutex()
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}
}

void safe_set_start_driver_state(LibConfig* configuration)
{
	g_mutex_lock(&driver_mutex);

	dispencer_count = configuration->dispencer_count;

	if (dispencer_count > 0)
	{
		for (guint8 i = 0; i < dispencer_count; i++)
		{
			dispencers[i].num = configuration->dispencers[i].num;
			dispencers[i].addr = configuration->dispencers[i].addr;
			dispencers[i].node_index = ((configuration->dispencers[i].addr & 0xF0) >> 4 ) - 1;
			dispencers[i].nozzle_count = configuration->dispencers[i].nozzle_count;

			dispencers[i].active_nozzle_index = -1;
			dispencers[i].current_amount = 0;
			dispencers[i].current_volume = 0;
			dispencers[i].dispencer_state = ds_NotInitialize;
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
					dispencers[i].nozzles[j].num = configuration->dispencers[i].nozzles[j].num;
					dispencers[i].nozzles[j].grade = configuration->dispencers[i].nozzles[j].grade;
				}
			}
		}
	}

	g_mutex_unlock(&driver_mutex);

}

guint8 safe_get_node_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].node_index;
	}

	g_mutex_unlock(&driver_mutex);

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

//--------------------------------------------------------  preset --------------------------------------------------------------------

void safe_get_preset(guint8 disp_index, gint8* nozzle_index, guint32* price, guint32* volume, guint32* amount)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		*nozzle_index 	= dispencers[disp_index].preset_nozzle_index;
		*price 			= dispencers[disp_index].preset_price;
		*volume 		= dispencers[disp_index].preset_volume;
		*amount 		= dispencers[disp_index].preset_amount;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  nozzle_num --------------------------------------------------------------------

guint8 safe_get_nozzle_num(guint8 disp_index, guint8 nozzle_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[nozzle_index].num;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  nozzle_count --------------------------------------------------------------------

guint8 safe_get_nozzle_count(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].nozzle_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  send_price_disp_index --------------------------------------------------------------------

gint8 safe_get_send_price_disp_index(gint8 node_index)
{
	gint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].send_price_disp_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_send_price_disp_index(guint8 node_index, gint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].send_price_disp_index = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}


//--------------------------------------------------------  send_command_timeout --------------------------------------------------------------------

guint8 safe_get_send_command_timeout(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].send_command_timeout;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_send_command_timeout(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].send_command_timeout = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

void safe_increment_send_command_timeout(guint8 node_index)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].send_command_timeout++;
	}

	g_mutex_unlock(&driver_mutex);

}


//--------------------------------------------------------  node_stage --------------------------------------------------------------------

IFSFNodeStage safe_get_node_stage(guint8 node_index)
{
	IFSFNodeStage result = ifsfns_Offline;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].stage;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_node_stage(guint8 node_index, IFSFNodeStage new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].stage = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

IFSFNodeStage safe_get_node_stage_by_disp_index(guint8 disp_index)
{
	IFSFNodeStage result = ifsfns_Offline;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].stage;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_node_stage_by_disp_index(guint8 disp_index, IFSFNodeStage new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].stage = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  send_command --------------------------------------------------------------------

IFSFNodeExchangeState safe_get_node_exchange_state(guint8 node_index)
{
	IFSFNodeExchangeState result = ifsfnes_Free;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].exchange_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_node_exchange_state(guint8 node_index, IFSFNodeExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].exchange_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

IFSFNodeExchangeState safe_get_node_exchange_state_by_disp_index(guint8 disp_index)
{
	IFSFNodeExchangeState result = ifsfnes_Free;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].exchange_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_node_exchange_state_by_disp_index(guint8 disp_index, IFSFNodeExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].exchange_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}


//--------------------------------------------------------  last_heartbeat_time --------------------------------------------------------------------

guint64 safe_get_last_heartbeat_time(guint8 node_index)
{
	guint64 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].last_heartbeat_time;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_last_heartbeat_time(guint8 node_index, guint64 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].last_heartbeat_time = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

guint64 safe_get_last_heartbeat_time_by_disp_index(guint8 disp_index)
{
	guint64 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].last_heartbeat_time;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_last_heartbeat_time_by_disp_index(guint8 disp_index, guint64 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].last_heartbeat_time = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  product_count --------------------------------------------------------------------

guint8 safe_get_product_count(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].product_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_product_count(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].product_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

guint8 safe_get_product_count_by_disp_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].product_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_product_count_by_disp_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].product_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  fuelling_mode_count --------------------------------------------------------------------

guint8 safe_get_fuelling_mode_count(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].fuelling_mode_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_fuelling_mode_count(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].fuelling_mode_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

guint8 safe_get_fuelling_mode_count_by_disp_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].fuelling_mode_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_fuelling_mode_count_by_disp_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].fuelling_mode_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  config_product_index --------------------------------------------------------------------

guint8 safe_get_config_product_index(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].config_product_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_config_product_index(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].config_product_index = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

void safe_increment_config_product_index(guint8 node_index)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].config_product_index++;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  config_fuelling_mode_index --------------------------------------------------------------------

guint8 safe_get_config_fuelling_mode_index(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].config_fuelling_mode_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_config_fuelling_mode_index(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].config_fuelling_mode_index = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

void safe_increment_config_fuelling_mode_index(guint8 node_index)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].config_fuelling_mode_index++;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  fuelling_poit_count --------------------------------------------------------------------

guint8 safe_get_fuelling_point_count(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].fuelling_point_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_fuelling_point_count(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].fuelling_point_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

guint8 safe_get_fuelling_point_count_by_disp_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].fuelling_point_count;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_fuelling_point_count_by_disp_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].fuelling_point_count = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  exchange_stage --------------------------------------------------------------------

ExchangeStage safe_get_exchange_stage(guint8 disp_index)
{
	ExchangeStage result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].exchange_stage;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_exchange_stage(guint8 disp_index, ExchangeStage new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].exchange_stage = new_value;
	}

	g_mutex_unlock(&driver_mutex);

}

//--------------------------------------------------------  nozzle_price --------------------------------------------------------------------

guint32 safe_get_nozzle_price(guint8 disp_index, guint8 nozzle_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[nozzle_index].price;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  product_id --------------------------------------------------------------------

void safe_set_nozzle_product_id(guint8 disp_index, guint8 nozzle_index, guint8 value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].product_id = value;
		}
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  nozzle_counter --------------------------------------------------------------------

void safe_set_nozzle_counter(guint8 disp_index, guint8 nozzle_index, guint32 value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].counter = value;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  disp_addr --------------------------------------------------------------------

DriverError safe_get_disp_addr(guint8 disp_index, guint8* disp_addr)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		*disp_addr = dispencers[disp_index].addr;

		result = de_NoError;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  current_command --------------------------------------------------------------------

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

//--------------------------------------------------------  dispencer_state --------------------------------------------------------------------

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

//--------------------------------------------------------  is_pay --------------------------------------------------------------------

gboolean safe_get_is_pay(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].is_pay;
	}

	g_mutex_unlock(&driver_mutex);

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

//--------------------------------------------------------  current_volume --------------------------------------------------------------------

guint32 safe_get_current_volume(guint8 disp_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].current_volume;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_current_volume(guint8 disp_index, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_volume = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  current_amount --------------------------------------------------------------------

guint32 safe_get_current_amount(guint8 disp_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].current_amount;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_current_amount(guint8 disp_index, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_amount = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  original state --------------------------------------------------------------------

IFSF_FPState safe_get_original_state(guint8 disp_index)
{
	IFSF_FPState result = ifsffps_Unknown;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].original_state;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_original_state(guint8 disp_index, IFSF_FPState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].original_state = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  transaction_num --------------------------------------------------------------------

guint16 safe_get_transaction_num(guint8 disp_index)
{
	guint16 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].transaction_num;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_transaction_num(guint8 disp_index, guint16 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].transaction_num = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  current_price --------------------------------------------------------------------

guint32 safe_get_current_price(guint8 disp_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].current_price;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_current_price(guint8 disp_index, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_price = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  reply_transport_token --------------------------------------------------------------------

guint8 safe_get_reply_transport_token_by_disp_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].reply_transport_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_reply_transport_token_by_disp_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].reply_transport_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

guint8 safe_get_reply_transport_token(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].reply_transport_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_reply_transport_token(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].reply_transport_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  reply_ifsf_token --------------------------------------------------------------------

guint8 safe_get_reply_ifsf_token_by_disp_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = nodes[dispencers[disp_index].node_index].reply_ifsf_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_reply_ifsf_token_by_disp_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		nodes[dispencers[disp_index].node_index].reply_ifsf_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

guint8 safe_get_reply_ifsf_token(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].reply_ifsf_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_reply_ifsf_token(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].reply_ifsf_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  transport_token --------------------------------------------------------------------

guint8 safe_get_transport_token(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].transport_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_transport_token(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].transport_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_increment_transport_token(guint8 node_index)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].transport_token++;

		if (nodes[node_index].transport_token > 0x0F)
		{
			nodes[node_index].transport_token = 0;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  ifsf_token --------------------------------------------------------------------

guint8 safe_get_ifsf_token(guint8 node_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		result = nodes[node_index].ifsf_token;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_ifsf_token(guint8 node_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].ifsf_token = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_increment_ifsf_token(guint8 node_index)
{
	g_mutex_lock(&driver_mutex);

	if (node_index  < MAX_NODE_COUNT)
	{
		nodes[node_index].ifsf_token++;

		if (nodes[node_index].ifsf_token > 0x0F)
		{
			nodes[node_index].ifsf_token = 0;
		}
	}

	g_mutex_unlock(&driver_mutex);
}


//--------------------------------------------------------  current_nozzle_index --------------------------------------------------------------------

guint8 safe_get_current_nozzle_index(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].current_nozzle_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_current_nozzle_index(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_nozzle_index = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_increment_current_nozzle_index(guint8 disp_index)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].current_nozzle_index++;
	}

	g_mutex_unlock(&driver_mutex);
}

gboolean safe_compare_disp_values(guint8 disp_index)
{
	gboolean result = FALSE;



	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{

		switch(dispencers[disp_index].order_type)
		{
			case ot_Volume:
				if (dispencers[disp_index].current_volume == dispencers[disp_index].preset_volume) result = TRUE;
				break;

			case ot_Amount:
				if (dispencers[disp_index].current_amount == dispencers[disp_index].preset_amount) result = TRUE;
				break;

			case ot_Unlim:
				result = FALSE;
				break;

			default:
				result = FALSE;
				break;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//--------------------------------------------------------  error --------------------------------------------------------------------

guint8 safe_get_error(guint8 disp_index)
{
	guint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].error;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_error(guint8 disp_index, guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].error = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_active_nozzle_index(guint8 disp_index, gint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].active_nozzle_index = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

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

DriverError safe_get_nozzle_product_id(guint8 disp_index, guint8 nozzle_index, guint8* product_id)
{
	DriverError result = de_Undefined;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			*product_id = dispencers[disp_index].nozzles[nozzle_index].product_id;
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


gint8 safe_get_preset_nozzle_index(guint8 disp_index)
{
	gint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].preset_nozzle_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

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

gint8 safe_get_active_nozzle_index(guint8 disp_index)
{
	gint8 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].active_nozzle_index;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_disp_clear(guint8 disp_index)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{

		dispencers[disp_index].dispencer_state = ds_Free;

		dispencers[disp_index].transaction_num = 0;

		dispencers[disp_index].preset_amount = 0;
		dispencers[disp_index].preset_nozzle_index = -1;
		dispencers[disp_index].preset_order_type = ot_Free;
		dispencers[disp_index].preset_price = 0;
		dispencers[disp_index].preset_volume = 0;

		dispencers[disp_index].order_type = ot_Free;
		dispencers[disp_index].active_nozzle_index = -1;
		dispencers[disp_index].current_amount = 0;
		dispencers[disp_index].current_volume = 0;
		dispencers[disp_index].current_price = 0;

		dispencers[disp_index].start = FALSE;
		dispencers[disp_index].is_pay = FALSE;
//		dispencers[disp_index].emergency_stop = FALSE;
		dispencers[disp_index].reset = FALSE;
		dispencers[disp_index].send_prices = FALSE;

		dispencers[disp_index].suspend = FALSE;
		dispencers[disp_index].resume = FALSE;

	}

	g_mutex_unlock(&driver_mutex);
}

void safe_disp_reset(guint8 disp_index)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].dispencer_state = ds_NotInitialize;
	//	dispencers[disp_index].exchange_state = es_Free;

		dispencers[disp_index].order_type = ot_Free;
		dispencers[disp_index].active_nozzle_index = -1;
		dispencers[disp_index].current_amount = 0;
		dispencers[disp_index].current_volume = 0;

		dispencers[disp_index].preset_nozzle_index = -1;
		dispencers[disp_index].preset_order_type = ot_Free;
		dispencers[disp_index].preset_price = 0;
		dispencers[disp_index].preset_volume = 0;
		dispencers[disp_index].preset_amount = 0;


	//	dispencers[disp_index].emergency_stop = FALSE;
		dispencers[disp_index].reset = FALSE;
		dispencers[disp_index].start = FALSE;
		dispencers[disp_index].is_pay = FALSE;
		dispencers[disp_index].error = 0;
	}

	g_mutex_unlock(&driver_mutex);
}

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


void safe_copy_disp_order_type(guint8 disp_index)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].order_type = dispencers[disp_index].preset_order_type;
	}

	g_mutex_unlock(&driver_mutex);
}


OrderType safe_get_order_type(guint8 disp_index)
{
	OrderType result = ot_Free;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].order_type;
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}


