#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>


#include "configuration.h"
#include "logger.h"
#include "system_func.h"
#include "ppc_device.h"

const gchar* ppc_status_to_str(PpcDeviceStatus value)
{
	switch(value)
	{
		case ppcds_NoError: return "NoError";
		case ppcds_ErrorConnection: return "ErrorConnection";
		case ppcds_ErrorConfiguration: return "ErrorConfiguration";
		case ppcds_ErrorLogging: return "ErrorLogging";
		case ppcds_UndefinedError: return "UndefinedError";
		case ppcds_NotInitializeDriver: return "NotInitializeDriver";
		default: return "Undefined";

	}
}

const gchar* ppc_error_to_str(PpcDeviceError error)
{
	switch(error)
	{
		case ppce_NoError: return "NoError";
		case ppce_WrongCommand: return "WrongCommand";
		case ppce_ErrorConnection: return "ErrorConnection";
		case ppce_ErrorConfiguration: return "ErrorConfiguration";
		case ppce_ErrorLogging: return "ErrorLogging";
		case ppce_Undefined: return "Undefined";
		case ppce_OutOfRange: return "OutOfRange";
		case ppce_AlreadyInit: return "AlreadyInit";
		case ppce_NotInitializeDriver: return "NotInitializeDriver";
		default: return "Undefined";
	}
}

const gchar* price_pole_state_to_str(PricePoleState value)
{
	switch(value)
	{
		case pps_NotInitialize: return "NotInitialize";
		case pps_Online: return "Online";
		case pps_Offline: return "Offline";
		default: return "Undefined";

	}
}


gchar* ppc_status_description(PpcDeviceStatus  status, guint32* size)
{

	gchar* result = NULL;
	gsize length = 0;
	switch(status)
	{
		case ppcds_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppcds_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppcds_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppcds_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppcds_UndefinedError:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppcds_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined status", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}

gchar* ppc_error_description(PpcDeviceError  error, guint32* size)
{
	gchar* result = NULL;
	gsize length = 0;
	switch(error)
	{
		case ppce_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_WrongCommand:
			result = g_convert_with_fallback("Wrong command", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_Undefined:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_OutOfRange:
			result = g_convert_with_fallback("Out of range", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_AlreadyInit:
			result = g_convert_with_fallback("Already init", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case ppce_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}


