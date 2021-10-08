#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>


#include "configuration.h"
#include "logger.h"
#include "system_func.h"
#include "tgs_device.h"

const gchar* tgs_status_to_str(TgsDeviceStatus value)
{
	switch(value)
	{
		case tgss_NoError: return "NoError";
		case tgss_ErrorConnection: return "ErrorConnection";
		case tgss_ErrorConfiguration: return "ErrorConfiguration";
		case tgss_ErrorLogging: return "ErrorLogging";
		case tgss_UndefinedError: return "UndefinedError";
		case tgss_NotInitializeDriver: return "NotInitializeDriver";
		default: return "Undefined";

	}
}

const gchar* tgs_error_to_str(TgsDeviceError error)
{
	switch(error)
	{
		case tgse_NoError: return "NoError";
		case tgse_WrongCommand: return "WrongCommand";
		case tgse_ErrorConnection: return "ErrorConnection";
		case tgse_ErrorConfiguration: return "ErrorConfiguration";
		case tgse_ErrorLogging: return "ErrorLogging";
		case tgse_FaultDispencerIndex : return "FaultDispencerIndex";
		case tgse_FaultNozzleIndex: return "FaultNozzleIndex";
		case tgse_Undefined: return "Undefined";
		case tgse_OutOfRange: return "OutOfRange";
		case tgse_AlreadyInit: return "AlreadyInit";
		case tgse_FaultExtFuncIndex: return "FaultExtFuncIndex";
		case tgse_NotInitializeDriver: return "NotInitializeDriver";
		case tgse_FaultDispencerNum: return "FaultDispencerNum";
		case tgse_FaultNozzleNum: return "FaultNozzleNum";
		case tgse_FaultDispencerState: return "FaultDispencerState";
		case tgse_FaultNozzleState: return "FaultNozzleState";
		case tgse_DispencerBusy: return "DispencerBusy";
		default: return "Undefined";
	}
}


gchar* tgs_status_description(TgsDeviceStatus  status, guint32* size)
{

	gchar* result = NULL;
	gsize length = 0;
	switch(status)
	{
		case tgss_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgss_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgss_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgss_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgss_UndefinedError:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgss_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined status", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}

gchar* tgs_error_description(TgsDeviceError  error, guint32* size)
{
	gchar* result = NULL;
	gsize length = 0;
	switch(error)
	{
		case tgse_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_WrongCommand:
			result = g_convert_with_fallback("Wrong command", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultDispencerIndex:
			result = g_convert_with_fallback("Fault dispencer index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultNozzleIndex:
			result = g_convert_with_fallback("Fault nozzle index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_Undefined:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_OutOfRange:
			result = g_convert_with_fallback("Out of range", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_AlreadyInit:
			result = g_convert_with_fallback("Already init", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultExtFuncIndex:
			result = g_convert_with_fallback("Fault extended function index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultDispencerNum:
			result = g_convert_with_fallback("Fault dispencer number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultNozzleNum:
			result = g_convert_with_fallback("Fault nozzle number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultDispencerState:
			result = g_convert_with_fallback("Fault dispencer state", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_FaultNozzleState:
			result = g_convert_with_fallback("Fault nozzle state", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case tgse_DispencerBusy:
			result = g_convert_with_fallback("Dispencer busy", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;


		default:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}


