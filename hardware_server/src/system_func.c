
#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "logger.h"
#include "system_func.h"


GMutex	   threads_mutex;

void threads_mutex_init()
{
	g_mutex_init(&threads_mutex);
}

void lock_threads_mutex()
{
	g_mutex_lock(&threads_mutex);
}

void unlock_threads_mutex()
{
	g_mutex_unlock(&threads_mutex);
}

void wait_threads_mutex()
{
	g_mutex_lock(&threads_mutex);
	g_mutex_unlock(&threads_mutex);
}

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

const gchar* bool_to_str(gboolean value)
{
	if (value)
	{
		return "True";
	}
	else
	{
		return "False";
	}
}

guint count_utf8_code_points(gchar* buffer)
{
	guint count = 0;
    while (*buffer)
    {
        count += (*buffer++ & 0xC0) != 0x80;
    }
    return count;
}

const gchar* device_type_to_str(DeviceType value)
{
	switch(value)
	{
		case dt_System: 						return "System";
		case dt_DispencerController:		 	return "DispencerController";
		case dt_TankGaugeSystem: 				return "TankGaugeSystem";
		case dt_FiscalRegister: 				return "FiscalRegister";
		case dt_Terminal: 						return "Terminal";
		case dt_CustomerDisplay: 				return "CustomerDisplay";
		case dt_PricePole: 						return "PricePole";
		case dt_BillValidator: 					return "BillValidator";
		case dt_InputController: 				return "InputController";
		case dt_SensorController: 				return "SensorController";
		default: return "Undefined";

	}
}

const gchar* server_command_to_str(HardwareServerCommand command)
{
	switch(command)
	{
		case hsc_None: 							return "None";
		case hsc_GetDeviceStatus: 				return "GetDeviceStatus";
		case hsc_GetConfiguration: 				return "GetConfiguration";

		case hsc_DCGetData: 					return "DCGetData";
		case hsc_DCGetCounters: 				return "DCGetCounters";
		case hsc_DCSetVolumeDose: 				return "DCSetVolumeDose";
		case hsc_DCSetSumDose: 					return "DCSetSumDose";
		case hsc_DCSetUnlimDose: 				return "DCSetUnlimDose";
		case hsc_DCStart: 						return "DCStart";
		case hsc_DCStop: 						return "DCStop";
		case hsc_DCPayment: 					return "DCPayment";
		case hsc_DCReset: 						return "DCReset";
		case hsc_DCExecuteExtendedFunction: 	return "DCExecuteExtendedFunction";
		case hsc_DCPriceUpdate: 				return "DCPriceUpdate";
		case hsc_DCResume: 						return "DCResume";
		case hsc_DCSuspend: 					return "DCSuspend";

		case hsc_TGSGetTankData: 				return "TGSGetTankData";
		case hsc_TGSGetAlarms: 					return "TGSGetAlarms";

		case hsc_FRPlaySound: 					return "FRPlaySound";
		case hsc_FRCutting: 					return "FRCutting";
		case hsc_FROpenDrawer: 					return "FROpenDrawer";
		case hsc_FRSetDateTime: 				return "FRSetDateTime";
		case hsc_FRXReport: 					return "FRXReport";
		case hsc_FRZReport: 					return "FRZReport";
		case hsc_FRMoneyIncome: 				return "FRMoneyIncome";
		case hsc_FRMoneyOutput: 				return "FRMoneyOutput";
		case hsc_FRFiscalReceipt: 				return "FRFiscalReceipt";
		case hsc_FRCancelFiscalReceipt: 		return "FRCancelFiscalReceipt";
		case hsc_FRRepeatDocument: 				return "FRRepeatDocument";
		case hsc_FRTextDocument: 				return "FRTextDocument";
		case hsc_FROpenShift: 					return "FROpenShift";
		case hsc_FRCorrectionReceipt: 			return "FRCorrectionReceipt";

		case hsc_CDClear: 						return "CDClear";
		case hsc_CDAdd: 						return "CDAdd";
		case hsc_CDEdit: 						return "CDEdit";
		case hsc_CDDelete: 						return "CDDelete";
		case hsc_CDShowMessage: 				return "CDShowMessage";

		case hsc_PPGetData:						return "PPGetData";
		case hsc_PPSetPrices: 					return "PPSetPrices";

		case hsc_BVReset: 						return "BVReset";
		case hsc_BVEnable: 						return "BVEnable";
		case hsc_BVDisable: 					return "BVDisable";

		case hsc_ICGetData: 					return "ICGetData";

		case hsc_SCGetSensorData: 				return "SCGetSensorData";

		case hsc_Reset:							return "Reset";

		case hsc_GetCommonServerConfig: 		return "GetCommonServerConfig";
		case hsc_UpdateCommonServerConfig: 		return "UpdateCommonServerConfig";
		case hsc_GetClientProfiles: 			return "GetClientProfiles";
		case hsc_AddClientProfile: 				return "AddClientProfile";
		case hsc_UpdateClientProfile: 			return "UpdateClientProfile";
		case hsc_DeleteClientProfile: 			return "DeleteClientProfile";
		case hsc_GetDispencerControllerConfig:	return "GetDispencerControllerConfig";
		case hsc_AddDispencerController: 		return "AddDispencerController";
		case hsc_UpdateDispencerController: 	return "UpdateDispencerController";
		case hsc_DeleteDispencerController: 	return "DeleteDispencerController";

		case hsc_GetTgsConfig:					return "GetTgsConfig";
		case hsc_AddTgs:						return "AddTgs";
		case hsc_UpdateTgs:						return "UpdateTgs";
		case hsc_DeleteTgs:						return "DeleteTgs";

		case hsc_GetPricePoleControllerConfig:	return "GetPricePoleControllerConfig";
		case hsc_AddPricePoleController:		return "AddPricePoleController";
		case hsc_UpdatePricePoleController:		return "UpdatePricePoleController";
		case hsc_DeletePricePoleController:		return "DeletePricePoleController";

		case hsc_GetSensorControllerConfig:		return "GetSensorControllerConfig";
		case hsc_AddSensorController:			return "AddSensorController";
		case hsc_UpdateSensorController:		return "UpdateSensorController";
		case hsc_DeleteSensorController:		return "DeleteSensorController";


		default: 								return "Undefined";

	}
}

gboolean compare_strings(gchar* str1, gchar* str2)
{
	gboolean result = FALSE;

	if (str1 !=NULL &&  str2 !=NULL )
	{
		if (strcmp(str1,str2) == 0)
		{
			result = TRUE;
		}
	}
	else if (str1 == NULL &&  str2 == NULL)
	{
		result = TRUE;
	}

	return result;

}
