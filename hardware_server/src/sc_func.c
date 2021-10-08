#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>


#include "configuration.h"
#include "logger.h"
#include "system_func.h"
#include "sc_device.h"

const gchar* sc_status_to_str(ScDeviceStatus value)
{
	switch(value)
	{
		case scs_NoError: return "NoError";
		case scs_ErrorConnection: return "ErrorConnection";
		case scs_ErrorConfiguration: return "ErrorConfiguration";
		case scs_ErrorLogging: return "ErrorLogging";
		case scs_UndefinedError: return "UndefinedError";
		case scs_NotInitializeDriver: return "NotInitializeDriver";
		default: return "Undefined";

	}
}

const gchar* sc_error_to_str(ScDeviceError error)
{
	switch(error)
	{
		case sce_NoError: return "NoError";
		case sce_WrongCommand: return "WrongCommand";
		case sce_ErrorConnection: return "ErrorConnection";
		case sce_ErrorConfiguration: return "ErrorConfiguration";
		case sce_ErrorLogging: return "ErrorLogging";
		case sce_FaultSensorIndex : return "FaultSensorIndex";
		case sce_FaultParamIndex: return "FaultParamIndex";
		case sce_Undefined: return "Undefined";
		case sce_OutOfRange: return "OutOfRange";
		case sce_AlreadyInit: return "AlreadyInit";
		case sce_NotInitializeDriver: return "NotInitializeDriver";
		case sce_FaultSensorNum: return "FaultSensorNum";
		case sce_FaultParamNum: return "FaultParamNum";
		default: return "Undefined";
	}
}

const gchar* spt_to_str(SensorParamType value)
{
	switch(value)
	{
		case spt_Float: return "Float";
		default: return "Undefined";
	}
}

gchar* sc_status_description(ScDeviceStatus  status, guint32* size)
{

	gchar* result = NULL;
	gsize length = 0;
	switch(status)
	{
		case scs_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case scs_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case scs_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case scs_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case scs_UndefinedError:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case scs_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined status", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}

gchar* sc_error_description(ScDeviceError  error, guint32* size)
{
	gchar* result = NULL;
	gsize length = 0;
	switch(error)
	{
		case sce_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_WrongCommand:
			result = g_convert_with_fallback("Wrong command", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_FaultSensorIndex:
			result = g_convert_with_fallback("Fault sensor index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_FaultParamIndex:
			result = g_convert_with_fallback("Fault param index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_Undefined:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_OutOfRange:
			result = g_convert_with_fallback("Out of range", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_AlreadyInit:
			result = g_convert_with_fallback("Already init", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_FaultSensorNum:
			result = g_convert_with_fallback("Fault sensor number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case sce_FaultParamNum:
			result = g_convert_with_fallback("Fault param number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}

gchar* spt_description(SensorParamType  value, guint32* size)
{
	gchar* result = NULL;
	gsize length = 0;
	switch(value)
	{
		case spt_Float:
			result = g_convert_with_fallback("Float", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined param type", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;

}


