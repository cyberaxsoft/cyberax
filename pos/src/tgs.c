#include <gtk/gtk.h>

#include "tgs.h"


Tank tanks[MAX_TANK_COUNT] = {0x00};

void set_start_tank_value()
{
	for (guint8 i = 0; i < 6; i++)
	{
		tanks[i].num  = i + 1;
		tanks[i].full_level = 2500.00;
		tanks[i].level  = 1024.00 + ( i* 100);
		tanks[i].volume  = 23560.00 + ( i* 100);
		tanks[i].weight  = 19233.00;
		tanks[i].density  = 0.7722;
		tanks[i].temperature  = 12.0;
		tanks[i].water_level  = 0.00;
	}
}


void set_tank_value_from_num(guint32 num, gfloat level, gfloat volume, gfloat weight, gfloat density, gfloat temperature, gfloat water_level)
{
	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (tanks[i].num == num)
		{
			tanks[i].level  = level;
			tanks[i].volume  = volume;
			tanks[i].weight  = weight;
			tanks[i].density  = density;
			tanks[i].temperature  = temperature;
			tanks[i].water_level  = water_level;

			return;
		}
	}
}

Tank* get_tank_from_num(guint32 num)
{
	Tank* result   = NULL;

	for (guint8 i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (tanks[i].num == num)
		{
			return &tanks[i];
		}
	}

	return result;

}
