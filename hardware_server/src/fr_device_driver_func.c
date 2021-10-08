#include <glib.h>
#include <glib/gstdio.h>
#include <dlfcn.h>
#include <stdio.h>

#include "logger.h"
#include "system_func.h"
#include "configuration.h"
#include "fr_device.h"
#include "fr_device_data.h"


void set_fr_device_status_from_driver(void* handle_lib, gchar* device_name, guint8 fri)
{
	guchar (*lib_func)();

	FrDeviceStatus status = frs_UndefinedError;

	lib_func = dlsym( handle_lib, "get_status" );
	gchar* error = dlerror();

	if (error != NULL)
	{
		g_printf( "%s error: %s\n", device_name,  error );
	}
	else
	{
		status = (*lib_func)();

		set_fr_device_status(fri, status);
	}

	g_free(error);
}
