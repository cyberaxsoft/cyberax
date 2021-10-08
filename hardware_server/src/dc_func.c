#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>

#include "configuration.h"
#include "logger.h"
#include "system_func.h"
#include "dc_device.h"



const gchar* dispencer_state_to_str(DispencerState value)
{
	switch(value)
	{
		case ds_NotInitialize: return "NotInitialize";
		case ds_Busy: return "Busy";
		case ds_Free: return "Free";
		case ds_NozzleOut: return "NozzleOut";
		case ds_Filling: return "Filling";
		case ds_Stopped: return "Stopped";
		case ds_Finish: return "Finish";
		case ds_ConnectionError: return "ConnectionError";
		default: return "Undefined";

	}
}

const gchar* order_type_to_str(OrderType value)
{
	switch(value)
	{
		case ot_Free: return "Free";
		case ot_Volume: return "Volume";
		case ot_Sum: return "Sum";
		case ot_Unlim: return "Unlimited";
		default: return "Undefined";

	}
}

const gchar* dc_status_to_str(DcDeviceStatus value)
{
	switch(value)
	{
		case dcs_NoError: return "NoError";
		case dcs_ErrorConnection: return "ErrorConnection";
		case dcs_ErrorConfiguration: return "ErrorConfiguration";
		case dcs_ErrorLogging: return "ErrorLogging";
		case dcs_UndefinedError: return "UndefinedError";
		case dcs_NotInitializeDriver: return "NotInitializeDriver";
		default: return "Undefined";

	}
}

const gchar* dc_error_to_str(DcDeviceError error)
{
	switch(error)
	{
		case dce_NoError: return "NoError";
		case dce_WrongCommand: return "WrongCommand";
		case dce_ErrorConnection: return "ErrorConnection";
		case dce_ErrorConfiguration: return "ErrorConfiguration";
		case dce_ErrorLogging: return "ErrorLogging";
		case dce_FaultDispencerIndex : return "FaultDispencerIndex";
		case dce_FaultNozzleIndex: return "FaultNozzleIndex";
		case dce_Undefined: return "Undefined";
		case dce_OutOfRange: return "OutOfRange";
		case dce_AlreadyInit: return "AlreadyInit";
		case dce_FaultExtFuncIndex: return "FaultExtFuncIndex";
		case dce_NotInitializeDriver: return "NotInitializeDriver";
		case dce_FaultDispencerNum: return "FaultDispencerNum";
		case dce_FaultNozzleNum: return "FaultNozzleNum";
		case dce_FaultDispencerState: return "FaultDispencerState";
		case dce_FaultNozzleState: return "FaultNozzleState";
		case dce_DispencerBusy: return "DispencerBusy";
		default: return "Undefined";
	}
}


gchar* dc_status_description(DcDeviceStatus  status, guint32* size)
{

	gchar* result = NULL;
	gsize length = 0;
	switch(status)
	{
		case dcs_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dcs_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dcs_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dcs_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dcs_UndefinedError:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dcs_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		default:
			result = g_convert_with_fallback("Undefined status", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}

gchar* dc_error_description(DcDeviceError  error, guint32* size)
{
	gchar* result = NULL;
	gsize length = 0;
	switch(error)
	{
		case dce_NoError:
			result = g_convert_with_fallback("No error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_WrongCommand:
			result = g_convert_with_fallback("Wrong command", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_ErrorConnection:
			result = g_convert_with_fallback("Error connection", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_ErrorConfiguration:
			result = g_convert_with_fallback("Error configuration", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_ErrorLogging:
			result = g_convert_with_fallback("Error logging", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultDispencerIndex:
			result = g_convert_with_fallback("Fault dispencer index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultNozzleIndex:
			result = g_convert_with_fallback("Fault nozzle index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_Undefined:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_OutOfRange:
			result = g_convert_with_fallback("Out of range", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_AlreadyInit:
			result = g_convert_with_fallback("Already init", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultExtFuncIndex:
			result = g_convert_with_fallback("Fault extended function index", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_NotInitializeDriver:
			result = g_convert_with_fallback("Not initialize driver", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultDispencerNum:
			result = g_convert_with_fallback("Fault dispencer number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultNozzleNum:
			result = g_convert_with_fallback("Fault nozzle number", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultDispencerState:
			result = g_convert_with_fallback("Fault dispencer state", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_FaultNozzleState:
			result = g_convert_with_fallback("Fault nozzle state", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;

		case dce_DispencerBusy:
			result = g_convert_with_fallback("Dispencer busy", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;


		default:
			result = g_convert_with_fallback("Undefined error", -1, "UTF-16", "UTF-8", " ", NULL, &length, NULL);
			break;
	}

	*size = (guint32)length;

	return result;
}
