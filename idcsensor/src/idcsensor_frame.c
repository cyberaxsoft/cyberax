#include <glib.h>
#include <stdio.h>

#include "driver.h"
#include "logger.h"
#include "idcsensor.h"

guint8 prepare_get_data_frame(guint8* buffer)
{
	guint8 result = 0;

	buffer[result++] = IDC_REFRESH_COMM;

	return result;
}
