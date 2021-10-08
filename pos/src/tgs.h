
#ifndef TGS_H_
#define TGS_H_

#define MAX_TANK_COUNT				128

typedef struct _Tank
{
	guint32 	num;
	gfloat 		full_level;
	gfloat 		level;
	gfloat		volume;
	gfloat		weight;
	gfloat		density;
	gfloat		temperature;
	gfloat		water_level;
}Tank;

void set_start_tank_value();

void set_tank_value_from_num(guint32 num, gfloat level, gfloat volume, gfloat weight, gfloat density, gfloat temperature, gfloat water_level);
Tank* get_tank_from_num(guint32 num);



#endif /* TGS_H_ */
