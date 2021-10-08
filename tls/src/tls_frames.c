#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "tls.h"

guint8 prepare_inventory_frame(guint8* buffer, guint8 tank_channel)
{
	guint8 result = 0;

	buffer[result++] = ctrl_STX;
	buffer[result++] = ctrl_CommPrefix;
	buffer[result++] = 0x32;
	buffer[result++] = 0x31;
	buffer[result++] = 0x34;

	buffer[result++] = 0x30 + (tank_channel / 10);
	buffer[result++] = 0x30 + (tank_channel % 10);

	return result;
}
