#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "topaz.h"
#include "config.h"
#include "topaz_func.h"

guint8			current_disp_index;
guint8			dispencer_count;
Dispencer		dispencers[MAX_DISP_COUNT];

guint8			last_disp_index;
guint8			last_nozzle_index;


GMutex			driver_mutex;

void init_driver_mutex()
{
	if (driver_mutex.p == NULL)
	{
		g_mutex_init(&driver_mutex);
	}
}

//-------------------------------------------------   setters and getters -----------------------------------------------------------

void safe_set_driver_state_from_settings(LibConfig* configuration)
{
	g_mutex_lock(&driver_mutex);

	dispencer_count = configuration->dispencer_count;

	if (dispencer_count > 0)
	{
		for (guint8 i = 0; i < dispencer_count; i++)
		{
			DispencerConf* disp_conf = &configuration->dispencers[i];

			dispencers[i].num = disp_conf->num;
			dispencers[i].addr = disp_conf->addr;
			dispencers[i].nozzle_count = disp_conf->nozzle_count;
			dispencers[i].current_nozzle_index = 0;

			dispencers[i].dispencer_state = ds_NotInitialize;

			dispencers[i].preset_order_type = ot_Free;
			dispencers[i].preset_nozzle_index = -1;
			dispencers[i].preset_amount = 0;
			dispencers[i].preset_price = 0;
			dispencers[i].preset_volume = 0;

			dispencers[i].order_type = ot_Free;
			dispencers[i].active_nozzle_index = -1;
			dispencers[i].current_amount = 0;
			dispencers[i].current_volume = 0;
			dispencers[i].current_price = 0;

			dispencers[i].get_counters = TRUE;

			dispencers[i].error = 0;
			dispencers[i].start = FALSE;
			dispencers[i].is_pay = FALSE;
			dispencers[i].emergency_stop = FALSE;
			dispencers[i].reset = FALSE;
			dispencers[i].suspend = FALSE;
			dispencers[i].resume = FALSE;

			if (dispencers[i].nozzle_count > 0)
			{
				for (guint8 j = 0; j < dispencers[i].nozzle_count; j++)
				{
					dispencers[i].nozzles[j].num = disp_conf->nozzles[j].num;

					dispencers[i].nozzles[j].price = 0;
					dispencers[i].nozzles[j].counter = 0;

					dispencers[i].nozzles[j].state = ces_Undefined;
					dispencers[i].nozzles[j].command_send = FALSE;
					dispencers[i].nozzles[j].volume_length = DEF_VOLUME_LENGTH;
					dispencers[i].nozzles[j].price_length = DEF_PRICE_LENGTH;
					dispencers[i].nozzles[j].amount_length = DEF_AMOUNT_LENGTH;
					dispencers[i].nozzles[j].version = 0;
				}

			}
		}
	}

	g_mutex_unlock(&driver_mutex);

}

void safe_get_current_nozzle(guint8* disp_index, guint8* nozzle_index)
{
	g_mutex_lock(&driver_mutex);

	*disp_index = current_disp_index;
	*nozzle_index = dispencers[current_disp_index].current_nozzle_index;

	g_mutex_unlock(&driver_mutex);

}

void safe_store_last_indexes(guint8 disp_index, guint8 nozzle_index)
{
	g_mutex_lock(&driver_mutex);

	last_disp_index = disp_index;
	last_nozzle_index = nozzle_index;

	g_mutex_unlock(&driver_mutex);

}

void safe_restore_last_indexes(guint8* disp_index, guint8* nozzle_index)
{
	g_mutex_lock(&driver_mutex);

	*disp_index = last_disp_index;
	*nozzle_index = last_nozzle_index;

	g_mutex_unlock(&driver_mutex);

}

void safe_change_channel()
{
	g_mutex_lock(&driver_mutex);

	if (dispencer_count > 0)
	{
		if (dispencers[current_disp_index].current_nozzle_index == dispencers[current_disp_index].active_nozzle_index)
		{
			current_disp_index++;

		}
		else
		{
			dispencers[current_disp_index].current_nozzle_index++;

			if (dispencers[current_disp_index].current_nozzle_index >= dispencers[current_disp_index].nozzle_count)
			{
				dispencers[current_disp_index].current_nozzle_index = 0;
				current_disp_index++;
			}
		}

		if (current_disp_index >= dispencer_count)
		{
			current_disp_index = 0;

			if (dispencers[current_disp_index].active_nozzle_index >= 0)
			{
				dispencers[current_disp_index].current_nozzle_index = dispencers[current_disp_index].active_nozzle_index;
			}
			else
			{
				dispencers[current_disp_index].current_nozzle_index = 0;
			}
		}
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_nozzle_param(guint8 disp_index, guint8 nozzle_index, guint8 volume_length, guint8 price_length, guint8 amount_length)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].volume_length = volume_length;
			dispencers[disp_index].nozzles[nozzle_index].price_length = price_length;
			dispencers[disp_index].nozzles[nozzle_index].amount_length = amount_length;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_get_nozzle_param(guint8 disp_index, guint8 nozzle_index, guint8* volume_length, guint8* price_length, guint8* amount_length)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			*volume_length = dispencers[disp_index].nozzles[nozzle_index].volume_length;
			*price_length = dispencers[disp_index].nozzles[nozzle_index].price_length;
			*amount_length = dispencers[disp_index].nozzles[nozzle_index].amount_length;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

void safe_set_nozzle_version(guint8 disp_index, guint8 nozzle_index, guint32 new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].version = new_value;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

guint32 safe_get_nozzle_version(guint8 disp_index, guint8 nozzle_index)
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[nozzle_index].version;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//------------------------------------------------------command_send--------------------------------------------------------------

gboolean safe_get_command_send(guint8 disp_index, guint8 nozzle_index)
{
	gboolean  result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[nozzle_index].command_send;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_set_command_send(guint8 disp_index, guint8 nozzle_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].command_send = new_value;
		}
	}

	g_mutex_unlock(&driver_mutex);
}

//------------------------------------------------------channel_exchange_state--------------------------------------------------------------

void safe_set_channel_exchange_state(guint8 disp_index, guint8 nozzle_index, ChannelExchangeState new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			dispencers[disp_index].nozzles[nozzle_index].state = new_value;
		}
	}

	g_mutex_unlock(&driver_mutex);

}

ChannelExchangeState safe_get_channel_exchange_state(guint8 disp_index, guint8 nozzle_index)
{
	ChannelExchangeState result = ces_Undefined;

	g_mutex_lock(&driver_mutex);

	if (disp_index < dispencer_count)
	{
		if (nozzle_index < dispencers[disp_index].nozzle_count)
		{
			result = dispencers[disp_index].nozzles[nozzle_index].state;
		}
	}

	g_mutex_unlock(&driver_mutex);

	return result;
}

//------------------------------------------------------acitve_disp_index--------------------------------------------------------------
void safe_set_current_disp_index(guint8 new_value)
{
	g_mutex_lock(&driver_mutex);

	g_print("333\n");
	current_disp_index = new_value;

	g_mutex_unlock(&driver_mutex);
}

guint8 safe_get_current_disp_index()
{
	guint32 result = 0;

	g_mutex_lock(&driver_mutex);

	result = current_disp_index;

	g_mutex_unlock(&driver_mutex);

	return result;
}

void safe_increment_current_disp_index()
{
	g_mutex_lock(&driver_mutex);

	g_print("222\n");

	current_disp_index++;

	if (current_disp_index >= dispencer_count)
	{
		current_disp_index = 0;
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

//--------------------------------------------------------  get_counters --------------------------------------------------------------------

gboolean safe_get_get_counters(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].get_counters;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_get_counters(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].get_counters = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  emergency_stop --------------------------------------------------------------------

gboolean safe_get_emergency_stop(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].emergency_stop;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_emergency_stop(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].emergency_stop = new_value;
	}

	g_mutex_unlock(&driver_mutex);
}

//--------------------------------------------------------  pause --------------------------------------------------------------------

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

//--------------------------------------------------------  nozzle_was_on --------------------------------------------------------------------

gboolean safe_get_nozzle_was_on(guint8 disp_index)
{
	gboolean result = FALSE;

	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		result = dispencers[disp_index].nozzle_was_on;
	}

	g_mutex_unlock(&driver_mutex);

	return result;

}

void safe_set_nozzle_was_on(guint8 disp_index, gboolean new_value)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].nozzle_was_on = new_value;
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

			case ot_Sum:
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
		dispencers[disp_index].active_nozzle_index = -1;
		dispencers[disp_index].current_amount = 0;
		dispencers[disp_index].current_volume = 0;
		dispencers[disp_index].dispencer_state = ds_Free;
		dispencers[disp_index].emergency_stop = FALSE;
		dispencers[disp_index].suspend = FALSE;
		dispencers[disp_index].resume = FALSE;

		dispencers[disp_index].is_pay = FALSE;
		dispencers[disp_index].order_type = ot_Free;
		dispencers[disp_index].preset_amount = 0;
		dispencers[disp_index].preset_nozzle_index = -1;
		dispencers[disp_index].preset_order_type = ot_Free;
		dispencers[disp_index].preset_price = 0;
		dispencers[disp_index].preset_volume = 0;
		dispencers[disp_index].reset = FALSE;
		dispencers[disp_index].start = FALSE;
		dispencers[disp_index].emergency_stop = FALSE;

	}

	g_mutex_unlock(&driver_mutex);
}

void safe_disp_reset(guint8 disp_index)
{
	g_mutex_lock(&driver_mutex);

	if (disp_index  < dispencer_count)
	{
		dispencers[disp_index].dispencer_state = ds_NotInitialize;

		dispencers[disp_index].order_type = ot_Free;
		dispencers[disp_index].active_nozzle_index = -1;
		dispencers[disp_index].current_amount = 0;
		dispencers[disp_index].current_volume = 0;

		dispencers[disp_index].preset_nozzle_index = -1;
		dispencers[disp_index].preset_order_type = ot_Free;
		dispencers[disp_index].preset_price = 0;
		dispencers[disp_index].preset_volume = 0;
		dispencers[disp_index].preset_amount = 0;


		dispencers[disp_index].emergency_stop = FALSE;
		dispencers[disp_index].suspend = FALSE;
		dispencers[disp_index].resume = FALSE;

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

