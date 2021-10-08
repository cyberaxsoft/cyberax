#include <glib.h>
#include <glib/gstdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/time.h>


#include "logger.h"
#include "pss.h"
#include "pss_client_thread.h"
#include "pss_tlv.h"
#include "pss_client_data.h"
#include "pss_func.h"

guint8 add_date_time_field(guint8* buffer, struct tm* timeinfo)
{
	guint8 pos = 0;

	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_year + 1900, 2);
	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_mon + 1, 1);
	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_mday, 1);
	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_hour, 1);
	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_min, 1);
	pos+=add_bcd_field(&buffer[pos], timeinfo->tm_sec, 1);

	return pos;
}

guint8 add_bcd_field(guint8* buffer, guint32 value, guint8 length)
{
	guint8 pos = 0;

	if (length > 0)
	{
		for (guint8 i = 0; i < length; i++)
		{
			buffer[pos++] = 0x00;
		}

		bin_to_packet_bcd(value, &buffer[pos-1], length);
	}

	return pos;

}

guint8 prepare_pss_reply_header(guint8* buffer, guint32 port, gboolean ex_mode, PSSCommand command_code, PSSReplyCode reply_code, guint8 subcode)
{
	guint8 pos = 0;

	buffer[pos++] = port_to_apc_code(port);
	if (ex_mode)
	{
		buffer[pos++] = 0x00;
		buffer[pos++] = command_code;
		buffer[pos++] = 0x80;
	}
	else
	{
		buffer[pos++] = reply_code;
	}
	buffer[pos++] = subcode;

	return pos;
}

guint8 prepare_pss_mult_reply_header(guint8* buffer, guint32 port, gboolean ex_mode, PSSCommand command_code, PSSReplyCode reply_code, guint8 subcode)
{
	guint8 pos = 0;

	buffer[pos++] = port_to_apc_code(port);
	if (ex_mode)
	{
		buffer[pos++] = 0x7E;
		buffer[pos++] = 0x80;
		buffer[pos++] = 0xFF;
		buffer[pos++] = command_code;
	}
	else
	{
		buffer[pos++] = 0xFE;
		buffer[pos++] = 0x00;
		buffer[pos++] = reply_code;
	}
	buffer[pos++] = subcode;

	return pos;

}

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}

void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size)
{
	size*=2;

	while(size--)
	{
		if (size & 0x01) *(buff) = val % 10;
		else *(buff--) |= (val % 10) << 4;
		val /= 10;
	}
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

guint8 add_BIN8(guint8* buffer, guint8 value)
{
	guint8 pos = 0;

	buffer[pos++] = 0x30 | (value >> 4);
	buffer[pos++] = 0x30 | (value & 0x0F);

	return pos;
}

guint8 add_BIN16(guint8* buffer, guint16 value)
{
	guint8 pos = 0;

	buffer[pos++] = 0x30 | (value >> 12);
	buffer[pos++] = 0x30 | ((value >> 8) & 0x0F);
	buffer[pos++] = 0x30 | ((value >> 4) & 0x0F);
	buffer[pos++] = 0x30 | (value & 0x0F);

	return pos;
}

guint8 parse_BIN8(guint8* buffer)
{
	guint8 result = (buffer[0] & 0x0F);
	result = (result << 4) | (buffer[1] & 0x0F);
	return result;
}

guint16 parse_BIN16(guint8* buffer)
{
	guint16 result = 0;

	for (guint8 i = 0; i < 4; i++)
		result = (result << 4) | (buffer[i] & 0x0F);

	return result;
}

guint16 parse_BIN16_LSB(guint8* buffer)
{
	return (((buffer[2] & 0x0F) << 4) | (buffer[3] & 0x0F) << 8 ) | (((buffer[0] & 0x0F) << 4) | (buffer[1] & 0x0F));
}

guint32 parse_BIN32_LSB(guint8* buffer)
{
	guint32 result = 0;

	for (guint8 i = 0; i < 8; i++)
	{
		result = (buffer[i] & 0x0F) << (4 * i) | result;

	}
	return result;
}

guint32 parse_BCD(guint8* buffer, guint8 length)
{
	guint32 result = 0;
	guint8 len = length * 2;

	for (guint8 i = 0; i < len; i++)
	{
		result = (result * 10) + (buffer[i] & 0x0F);
	}
	return result;
}

guint8 parse_ASCII_byte(guint8* buffer)
{
	return (buffer[1] & 0x0F) * 10 +  (buffer[3] & 0x0F);
}


guint16 parse_pss_ascii(guint8* dest, guint8* src, guint8 max_length)
{
	guint8 length = parse_BIN8(src);

	memset(dest, 0x00, max_length);

	if (length < max_length)
	{
		guint8* txt = &src[2];
		guint16 len = length * 2;
		guchar symbol = 0x00;

		while(len--)
		{
			if (len & 0x01)
			{
				symbol = *(txt++) & 0x0F;
			}
			else
			{
				symbol = (symbol << 4) | (*(txt++) & 0x0F);
				*(dest++) = symbol;
			}
		}

		return length * 2 + 2;
	}
	else
	{
		return  2 + (length * 2);
	}
}

gint32 prepare_dpl_frame(guint8* dest, guint8* src, guint16 src_length)
{
	guint32 pos = 0;
	guint8 crc = 0;

	dest[pos++] = ctrl_SOH;
	pos += add_BIN8( &dest[pos], DPL_VERSION);
	pos += add_BIN8( &dest[pos], DPL_ENCODING_TYPE);

	if (src_length > 0)
	{
		for (guint16 i = 0; i < src_length; i++)
		{
			crc^=src[i];
		}
	}

	pos += add_BIN8( &dest[pos], crc);
	pos += add_BIN16( &dest[pos], src_length);

	dest[pos++] = ctrl_STX;

	if (src_length > 0)
	{
		for (guint16 i = 0; i < src_length; i++)
		{
			if (pos < EXCHANGE_BUFFER_SIZE - 2)
			{
				pos += add_BIN8( &dest[pos], src[i]);

			}
			else
			{
				return -1;
			}
		}
	}

	if (pos < EXCHANGE_BUFFER_SIZE - 1)
	{
		dest[pos++] = ctrl_ETX;
	}
	else
	{
		return -1;
	}

	return pos;

}

guint8 port_to_apc_code(guint32 port)
{
	guint8 result = 0;

	switch(port)
	{
		case SUPERVISED_PORT: 				result = APC1; break;
		case SUPERVISED_MESSAGES_PORT: 		result = APC2; break;
		case FALLBACK_CONSOLE: 				result = APC3; break;
		case UNSUPERVISED_PORT:				result = APC4; break;
		case UNSUPERVISED_MESSAGES_PORT: 	result = APC5; break;
		case MSG_PAYMENT_SERVER: 			result = APC6; break;
		case CONTROL_PAYMENT_SERVER: 		result = APC7; break;
		case PIN_PAD_INTERFACE: 			result = APC8; break;
		case REMOTE_LOG_INTERFACE: 			result = APC9; break;

	}

	return result;
}

guint8 return_pos_device_type(guint32 port)
{
	guint8 result = 0;

	switch(port)
	{
		case SUPERVISED_PORT: 				result = 1; break;
		case FALLBACK_CONSOLE: 				result = 2; break;
		case MSG_PAYMENT_SERVER: 			result = 4; break;

		default: result = 0;

	}

	return result;
}

void socket_send(gint32 sock, guchar* buffer, guint32 size)
{
	send(sock, buffer, size, 0);

}


gint prepare_heartbeat_frame(guint8* buffer)
{
	return prepare_dpl_frame(buffer, NULL, 0);
}

const gchar* fc_gen_par_id_to_str(FcGenParId id)
{
	switch(id)
	{
		case fcgpi_PriceIncreaseDelay: 			return "PriceIncreaseDelay";
		case fcgpi_PriceDecreaseDelay: 			return "PriceDecreaseDelay";
		case fcgpi_DefaultLanguageCode: 		return "DefaultLanguageCode";
		case fcgpi_DisableFpTotalsError: 		return "DisableFpTotalsError";
		case fcgpi_EnableDemoEncryption: 		return "EnableDemoEncryption";
		case fcgpi_CurrencyCode: 				return "CurrencyCode";
		case fcgpi_FcPumpTotalsHandlingMode:	return "FcPumpTotalsHandlingMode";
		case fcgpi_FcShiftNo: 					return "FcShiftNo";
		case fcgpi_SpecificProject: 			return "SpecificProject";
		case fcgpi_VatRate: 					return "VatRate";

		default: return "Undefined";

	}
}

const gchar* port_to_str(guint32 port)
{
	switch(port)
	{
		case SUPERVISED_PORT: 				return "APC1";
		case SUPERVISED_MESSAGES_PORT: 		return "APC2";
		case FALLBACK_CONSOLE: 				return "APC3";
		case UNSUPERVISED_PORT:				return "APC4";
		case UNSUPERVISED_MESSAGES_PORT: 	return "APC5";
		case MSG_PAYMENT_SERVER: 			return "APC6";
		case CONTROL_PAYMENT_SERVER: 		return "APC7";
		case PIN_PAD_INTERFACE: 			return "APC8";
		case REMOTE_LOG_INTERFACE: 			return "APC9";
		default: return "Undefined";
	}
}

const gchar* fc_par_group_id_to_str(FcParGroupId id)
{
	switch(id)
	{
		case fcpgid_ServiceModes: 				return "ServiceModes";
		case fcpgid_FuellingModes: 				return "FuellingModes";
		case fcpgid_FuellingModeGroups: 		return "FuellingModeGroups";
		case fcpgid_GradeTexts: 				return "GradeTexts";
		case fcpgid_GradeDescriptors: 			return "GradeDescriptors";
		case fcpgid_GradeCodesWithText: 		return "GradeCodesWithText";
		case fcpgid_WashControl: 				return "WashControl";
		case fcpgid_GradeTextsWithSize: 		return "GradeTextsWithSize";

		//payment control
		case fcpgid_PCEptTimers: 				return "PCEptTimers";
		case fcpgid_PCEptTexts: 				return "PCEptTexts";
		case fcpgid_PCRcpFormats: 				return "PCRcpFormats";
		case fcpgid_PCRcpLineDefinitions: 		return "PCRcpLineDefinitions";
		case fcpgid_PCCardMatchTables: 			return "PCCardMatchTables";
		case fcpgid_PCCardTypeTables: 			return "PCCardTypeTables";
		case fcpgid_PCCardHandlingTables: 		return "PCCardHandlingTables";
		case fcpgid_PCFpNames: 					return "PCFpNames";
		case fcpgid_PCRcpFixedTexts: 			return "PCRcpFixedTexts";
		case fcpgid_PCFcGradeSets: 				return "PCFcGradeSets";
		case fcpgid_PCEptActionsDefinitions: 	return "PCEptActionsDefinitions";
		case fcpgid_PCReceiptHeaders: 			return "PCReceiptHeaders";
		case fcpgid_PCRcpItemLists: 			return "PCRcpItemLists";
		case fcpgid_PCReceiptFooters: 			return "PCReceiptFooters";

		case fcpgid_FloorLimitMargins: 			return "FloorLimitMargins";
		case fcpgid_GlobalFuellingLimits: 		return "GlobalFuellingLimits";

		//wetstock control
		case fcpgid_WSCProdoctTexts: 			return "WSCProdoctTexts";
		case fcpgid_WSCTankGaugeAlarmConfig: 	return "WSCTankGaugeAlarmConfig";
		case fcpgid_WSCTanks: 					return "WSCTanks";
		case fcpgid_WSCTankAssignedParameters: 	return "WSCTankAssignedParameters";
		case fcpgid_EPSSetup: 					return "EPSSetup";
		case fcpgid_SiteParameters: 			return "SiteParameters";
		case fcpgid_OperatorParameters: 		return "OperatorParameters";
		case fcpgid_GeneralFcParmeters: 		return "GeneralFcParmeters";
		case fcpgid_ExternalSystems: 			return "ExternalSystems";
		case fcpgid_ForecourtControllerTables: 	return "ForecourtControllerTables";
		case fcpgid_BackOfficeRecordDefinitions:return "BackOfficeRecordDefinitions";
		case fcpgid_PSSInformationBase: 		return "PSSInformationBase";
		default: return "Undefined";
	}
}

const gchar* fc_serv_mode_par_id_to_str(FcServModeParId id)
{
	switch(id)
	{
		case fcsmpi_AutoAuthorizeLimit: 		return "Auto Authorize Limit";
		case fcsmpi_MaxPreAuthorizeTime: 		return "Max Pre Authorize Time";
		case fcsmpi_MaxNzLaydownTime: 			return "Max Nz Laydown Time";
		case fcsmpi_ZeroTransToPos: 			return "Zero Trans To Pos";
		case fcsmpi_MoneyDueInTransBufStatus: 	return "Money Due In Trans Buf Status";
		case fcsmpi_MinTransVol: 				return "Min Trans Vol";
		case fcsmpi_MinTransMoney:				return "Min Trans Money";
		case fcsmpi_SupTransBufferSize: 		return "Sup Trans Buffer Size";
		case fcsmpi_UnsupTransBufferSize: 		return "Unsup Trans Buffer Size";
		case fcsmpi_StoreAtPreAuthorize: 		return "Store At Pre Authorize";
		case fcsmpi_VolInTransBufStatus: 		return "Vol In Trans Buf Status";
		case fcsmpi_AuthorizeAtModeSelection: 	return "Authorize At Mode Selection";
		case fcsmpi_MnoConsecutiveZeroTrans: 	return "Mno Consecutive Zero Trans";
		case fcsmpi_AutoClearTransDelayTime: 	return "Auto Clear Trans Delay Time";
		case fcsmpi_PumpLightMode: 				return "Pump Light Mode";
		case fcsmpi_StopFpOnVehicleTag: 		return "Stop Fp On Vehicle Tag";
		case fcsmpi_UseVehicleTagReadingButton: return "Use Vehicle Tag Reading Button";

		default: return "Undefined";

	}
}

const gchar* fc_fuelling_mode_par_id_to_str(FcFuellingModeParId id)
{
	switch(id)
	{
		case fcfmpi_FuellingType: 					return "Fuelling Type";
		case fcfmpi_MaxTimeToReachMinLimit: 		return "Max Time To Reach Min Limit";
		case fcfmpi_MaxTimeWithoutProgress: 		return "Max Time Without Progress";
		case fcfmpi_MaxTransVolume: 				return "Max Trans Volume";
		case fcfmpi_MaxTransMoney: 					return "Max Trans Money";
		case fcfmpi_MaxFuellingTime: 				return "Max Fuelling Time";
		case fcfmpi_MaxPresetVolOverrunErrLimit:	return "Max Preset Vol Overrun Err Limit";
		case fcfmpi_ClrDisplayDelayTime: 			return "Clr Display Delay Time";
		case fcfmpi_ClrDisplayWhenCurTrDisappear: 	return "Clr Display When Cur Tr Disappear";
		case fcfmpi_MinSubPumpRuntimeBeforeStart: 	return "Min Sub Pump Runtime Before Start";
		case fcfmpi_MaxTransVolumeE: 				return "Max Trans Volume Extended";
		case fcfmpi_MaxTransMoneyE: 				return "Max Trans Money Extended";

		default: return "Undefined";

	}
}

const gchar* fc_global_fuelling_limits_par_id_to_str(FcDispenceLimitsParId id)
{
	switch(id)
	{
		case fcdlpi_GlobalMoneyLimit: 				return "Global Money Limit";
		case fcdlpi_GlobalVolumeLimit: 				return "Global Volume Limit";

		default: return "Undefined";

	}
}

const gchar* fp_install_par_id_to_str(FpInstallParId id)
{
	switch(id)
	{
		case fpipi_PumpInterfaceType: 				return "Pump Interface Type";
		case fpipi_PssChannelNo: 					return "Pss Channel No";
		case fpipi_DeviceCommunicationAddress: 		return "Device Communication Address";
		case fpipi_FpGradeOptions: 					return "Fp Grade Options";
		case fpipi_FpGradeOptionsPars: 				return "Fp Grade Options Pars";
		case fpipi_PumpInterfaceTypeGeneral: 		return "Pump Interface Type General";
		case fpipi_PumpInterfaceTypeProtocol: 		return "Pump Interface Type Protocol";
		case fpipi_PumpDecimalPositionInMoney: 		return "Pump Decimal Position In Money";
		case fpipi_PumpDecimalPositionInVolume: 	return "Pump Decimal Position In Volume";
		case fpipi_PumpDecimalPositionInPrice: 		return "Pump Decimal Position In Price";
		case fpipi_PumpDecimalPositionInMoneyTot: 	return "Pump Decimal Position In Money Tot";
		case fpipi_PumpDecimalPositionInVolumeTot: 	return "Pump Decimal Position In Volume Tot";
		case fpipi_PumpInterfaceConfigString: 		return "Pump Interface Config String";
		case fpipi_NomimalNormalSpeed: 				return "Nomimal Normal Speed";
		case fpipi_NominalHighSpeed: 				return "Nominal High Speed";
		case fpipi_HighSpeedTriggerLevel: 			return "High Speed Trigger Level";

		default: return "Undefined";

	}
}
const gchar* server_device_type_to_str(PSSServerDeviceType device_type)
{
	switch(device_type)
	{
		case psdt_System: 							return "System";
		case psdt_DispencerController: 				return "Dispencer Controller";
		case psdt_TankGaugeSystem: 					return "Tank Gauge System";
		case psdt_FiscalRegister: 					return "Fiscal Register";
		case psdt_Terminal: 						return "Terminal";
		case psdt_CustomerDisplay: 					return "Customer Display";
		case psdt_PricePoleController:				return "Price Pole Controller";
		case psdt_BillValidator: 					return "Bill Validator";
		case psdt_InputController: 					return "Input Controller";
		case psdt_SensorController: 				return "Sensor Controller";

		default: return "Undefined";
	}
}

const gchar* fp_main_state_to_str(PSSFpMainState fp_state)
{
	switch(fp_state)
	{
		case fpms_Unconfigured: 					return "Uncobnfigured";
		case fpms_Closed: 							return "Closed";
		case fpms_Idle: 							return "Idle";
		case fpms_Error: 							return "Error";
		case fpms_Calling: 							return "Calling";
		case fpms_PreAuthorized: 					return "Pre-Authorized";
		case fpms_Starting: 						return "Starting";
		case fpms_StartingPaused: 					return "Starting Paused";
		case fpms_StartingTerminated: 				return "Starting Terminated";
		case fpms_Fuelling: 						return "Fuelling";
		case fpms_FuellingPaused: 					return "Fuelling Paused";
		case fpms_FuellingTerminated: 				return "Fuelling Terminated";
		case fpms_Unavailable: 						return "Unavailable";
		case fpms_UnavailableAndCalling: 			return "Unavailable And Calling";

		default: return "Undefined";

	}
}

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

PSSFpMainState decode_dispencer_state(DispencerState value, OrderType order_type)
{
	switch (value)
	{
		case ds_NotInitialize: 			return fpms_Error;
		case ds_Busy: 					return fpms_Error;
		case ds_Free:
			switch (order_type)
			{
				case ot_Free: 			return fpms_Idle;
				case ot_Volume: 		return fpms_PreAuthorized;
				case ot_Sum: 			return fpms_PreAuthorized;
				case ot_Unlim: 			return fpms_PreAuthorized;
				default:
					return fpms_Error;
			}
			break;
		case ds_NozzleOut: 				return fpms_Calling;
		case ds_Filling: 				return fpms_Fuelling;
		case ds_Stopped: 				return fpms_FuellingTerminated;
		case ds_Finish: 				return fpms_Unavailable;
		case ds_ConnectionError: 		return fpms_Error;
		default:
			return fpms_Error;
	}
}


