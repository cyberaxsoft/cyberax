#ifndef PSS_H_
#define PSS_H_

#define DEF_STR_LENGTH				255
#define DEF_IP_ADDRESS_LENGTH		16
#define DEF_DC_TIMEOUT				200
#define SOCKET_BIND_TIMEOUT			1000

#define HW_SERVER_CONNECT_TIMEOUT	1000   //ms

//#define CONFIG_FILENAME				"config.xml"

#define DPL_VERSION					0
#define	DPL_ENCODING_TYPE			0

#define FC_HW_TYPE					0
#define FC_HW_VERSION_NUMBER		0
#define FC_SW_TYPE					0
#define FC_SW_VERSION_NUMBER		0x41038215
#define FC_SW_DATETIME				0x20140619


#define SUPERVISED_PORT				5001
#define SUPERVISED_MESSAGES_PORT	5002
#define FALLBACK_CONSOLE			5003
#define UNSUPERVISED_PORT			5004
#define UNSUPERVISED_MESSAGES_PORT	5005
#define MSG_PAYMENT_SERVER			5006
#define CONTROL_PAYMENT_SERVER		5007
#define PIN_PAD_INTERFACE			5008
#define REMOTE_LOG_INTERFACE		5009

#define MAX_DISP_COUNT				16
#define MAX_DC_COUNT				16
#define MAX_NOZZLE_COUNT			8
#define MAX_LOG_SIZE				1048576
#define MAX_SERVER_DEVICE_UNIT_COUNT 128

#define  EXCHANGE_BUFFER_SIZE		16384
#define  SOCKET_BUFFER_SIZE			1024
#define  MAX_BAD_FRAME				3
#define  MAX_STRING_LENGTH			255

#define	 PSS_HEARTBEAT_TIMEOUT		10000

#define PSS_DATALENGTH_OFFSET		7
#define PSS_COUNTRY_CODE			7

#define PSS_MAX_TRANSACTION_PARAM_COUNT		128

#define PSS_MAX_TANK_UNITS_COUNT	18

#define	SENSOR_PARAM_VALUE_SIZE		8
#define	MAX_SENSOR_PARAM_COUNT		16

typedef enum _PSSServerDeviceType
{
	psdt_System							= 0x00,
	psdt_DispencerController			= 0x01,
	psdt_TankGaugeSystem				= 0x02,
	psdt_FiscalRegister 				= 0x03,
	psdt_Terminal						= 0x04,
	psdt_CustomerDisplay				= 0x05,
	psdt_PricePoleController			= 0x06,
	psdt_BillValidator					= 0x07,
	psdt_InputController				= 0x08,
	psdt_SensorController				= 0x09,

}PSSServerDeviceType;

typedef enum _PSSExchangeState
{
	psses_WaitSOH			= 0,
	psses_WaitSTX			= 1,
	psses_WaitETX			= 2,
}PSSExchangeState;

typedef enum _PSSAPCCode
{
	APC1					= 0x01,
	APC2					= 0x02,
	APC3					= 0x03,
	APC4					= 0x04,
	APC5					= 0x05,
	APC6					= 0x06,
	APC7					= 0x07,
	APC8					= 0x08,
	APC9					= 0x09,
}PSSAPCCode;

typedef enum _FrameControlCharacter
{
	ctrl_SOH			= 0x01,
	ctrl_STX			= 0x02,
	ctrl_ETX			= 0x03,
	ctrl_ETB			= 0x17,
}FrameControlCharacter;

typedef enum _OrderType
{
	ot_Free					= 0x00,
	ot_Volume				= 0x01,
	ot_Sum					= 0x02,
	ot_Unlim				= 0x03,
}OrderType;

typedef enum _ServerCommand
{
	sc_None							= 0x00000000,
	sc_GetDeviceStatus				= 0x00000001,
	sc_GetConfiguration				= 0x00000002,

	sc_DCGetData					= 0x01010001,
	sc_DCGetCounters				= 0x01010002,
	sc_DCSetVolumeDose				= 0x01010003,
	sc_DCSetSumDose					= 0x01010004,
	sc_DCSetUnlimDose				= 0x01010005,
	sc_DCStart						= 0x01010006,
	sc_DCStop						= 0x01010007,
	sc_DCPayment					= 0x01010008,
	sc_DCReset						= 0x01010009,
	sc_DCExecuteExtendedFunction	= 0x0101000A,
	sc_DCPriceUpdate				= 0x0101000B,

	sc_TGSGetTankData				= 0x01020001,
	sc_TGSGetAlarms					= 0x01020002,

	sc_FRPlaySound					= 0x01030001,
	sc_FRCutting					= 0x01030002,
	sc_FROpenDrawer					= 0x01030003,
	sc_FRSetDateTime				= 0x01030004,
	sc_FRXReport					= 0x01030005,
	sc_FRZReport					= 0x01030006,
	sc_FRMoneyIncome				= 0x01030007,
	sc_FRMoneyOutput				= 0x01030008,
	sc_FRFiscalReceipt				= 0x01030009,
	sc_FRCancelFiscalReceipt		= 0x0103000A,
	sc_FRRepeatDocument				= 0x0103000B,
	sc_FRTextDocument				= 0x0103000C,
	sc_FROpenShift					= 0x0103000D,
	sc_FRCorrectionReceipt			= 0x0103000E,

	sc_CDClear						= 0x01050001,
	sc_CDAdd						= 0x01050002,
	sc_CDEdit						= 0x01050003,
	sc_CDDelete						= 0x01050004,
	sc_CDShowMessage				= 0x01050005,

	sc_PPSetPrice					= 0x01060001,

	sc_BVReset						= 0x01070001,
	sc_BVEnable						= 0x01070002,
	sc_BVDisable					= 0x01070003,

	sc_ICGetData					= 0x01080001,

	sc_SCGetData					= 0x01090001,

}ServerCommand;

typedef enum _PSSCommand
{
	pssc_None						= 0x00,
	pssc_FcLogon_req				= 0x01,
	pssc_DiffComm					= 0x02,
	pssc_FcDateAndTime_req			= 0x03,
	pssc_FcStatus_req				= 0x06,
	pssc_FcInstallStatus_req		= 0x07,
	pssc_FcPriceSetStatus_req		= 0x08,
	pssc_FcOperationModeStatus_req	= 0x09,
	pssc_change_FcParameters		= 0x0A,
	pssc_FcParameterSet_req			= 0x0B,
	pssc_load_FcPriceSet			= 0x0C,
	pssc_FcPriceSet_req				= 0x0D,
	pssc_set_FcOperationModeNo		= 0x0E,
	pssc_install_Fp					= 0x10,
	pssc_FpGradeTotals_req			= 0x11,
	pssc_PumpGradeTotals_req		= 0x12,
	pssc_FpStatus_req				= 0x15,
	pssc_FpSupTransBufStatus_req	= 0x16,
	pssc_FpInfo_req					= 0x18,
	pssc_FpFuellingData_req			= 0x19,
	pssc_load_FpOperationModeSet	= 0x20,
	pssc_open_fp					= 0x22,
	pssc_close_fp					= 0x23,
	pssc_authorize_Fp				= 0x24,
	pssc_estop_Fp					= 0x27,
	pssc_cancel_estop_Fp			= 0x28,
	pssc_reset_Fp					= 0x2D,
	pssc_FpSupTrans_req				= 0x2E,
	pssc_clr_FpSupTrans				= 0x31,

	pssc_install_Pp					= 0x37,
	pssc_PpStatus_req				= 0x38,
	pssc_load_PpOperationModeSet	= 0x3A,
	pssc_open_Pp					= 0x3B,
	pssc_close_Pp					= 0x3C,
	pssc_reset_Pp					= 0x3F,


	pssc_install_TankGauge			= 0x40,
	pssc_TgData_Req					= 0x41,
	pssc_TgStatus_Req				= 0x42,

	pssc_PssPeripheralsStatus_req	= 0x76,
	pssc_PosConnectionStatus_req	= 0x77,
	pssc_clr_install_data			= 0x7D,
	pssc_ExtendedFunc				= 0xFF,
}PSSCommand;

typedef enum _PSSReplyCode
{
	pssc_Rejected					= 0x80,
	pssc_FcLogon_ack				= 0x81,
	pssc_set_FcDateAndTime			= 0x82,
	pssc_FcDateAndTime				= 0x83,
	pssc_FcStatus					= 0x86,
	pssc_FcInstallStatus			= 0x87,
	pssc_FcPriceSetStatus			= 0x88,
	pssc_FcOperationModeStatus		= 0x89,
	pssc_change_FcParameter_ack		= 0x8A,
	pssc_FcParameterSet				= 0x8B,
	pssc_load_FcPriceSetAck			= 0x8C,
	pssc_FcPriceSet					= 0x8D,
	pssc_set_FcOperationModeNo_ack  = 0x8E,
	pssc_install_Fp_asc				= 0x90,
	pssc_FpGradeTotals				= 0x91,
	pssc_PumpGradeTotals			= 0x92,
	pssc_FpStatus					= 0x95,
	pssc_FpSupTransBufStatus		= 0x96,
	pssc_FpInfo						= 0x98,
	pssc_FpFuellingData				= 0x99,
	pssc_load_FpOperationModeSet_ack= 0xA0,
	pssc_open_fp_ack				= 0xA2,
	pssc_close_fp_ack				= 0xA3,
	pssc_authorize_Fp_ack			= 0xA4,
	pssc_estop_Fp_ack				= 0xA7,
	pssc_cancel_estop_Fp_ack		= 0xA8,
	pssc_reset_Fp_ack				= 0xAD,
	pssc_FpSupTrans					= 0xAE,
	pssc_clr_FpSupTrans_ack			= 0xB1,

	pssc_install_Pp_ack				= 0xB7,
	pssc_PpStatus_ack				= 0xB8,
	pssc_load_PpOperationModeSet_ack= 0xBA,
	pssc_open_Pp_ack				= 0xBB,
	pssc_close_Pp_ack				= 0xBC,
	pssc_reset_Pp_ack				= 0xBF,

	pssc_install_TankGauge_ack		= 0xC0,
	pssc_TgData						= 0xC1,
	pssc_TgStatus					= 0xC2,

	pssc_PssPeripheralStatus		= 0xF6,
	pssc_PosConnectionStatus		= 0xF7,
	pssc_clr_install_data_ack		= 0xFD,
}PSSReplyCode;


typedef struct _Nozzle
{
	guint8			num;
	guint32			price;
	guint32			counter;
}Nozzle;

typedef enum _DispencerState
{
	ds_NotInitialize		= 0x00,
	ds_Busy					= 0x01,
	ds_Free					= 0x02,
	ds_NozzleOut			= 0x03,
	ds_Filling				= 0x04,
	ds_Stopped				= 0x05,
	ds_Finish				= 0x06,
	ds_ConnectionError		= 0x07,
}DispencerState;

typedef struct _PricePack
{
	guint8					id;
	guint32					price;
}PricePack;

typedef struct _PricePacks
{
	PricePack				units[MAX_NOZZLE_COUNT];
	guint8					unit_count;
}PricePacks;


typedef struct _Dispencer
{
	guint32				num;
	guint32				dispencer_controller_id;
	guint8				nozzle_count;
	Nozzle				nozzles[MAX_NOZZLE_COUNT];

	DispencerState		state;

	OrderType 			preset_order_type;
	guint8				preset_nozzle_num;
	guint32				preset_price;
	guint32				preset_volume;
	guint32				preset_amount;

	OrderType 			order_type;
	guint8				active_nozzle_num;
	guint32				current_price;
	guint32				current_volume;
	guint32				current_amount;

	guint8				is_pay;

	guint8				error;

}Dispencer;

typedef struct _Tank
{
	guint32				num;
	guint32				tank_controller_id;

	gfloat 				height;
	gfloat				volume;
	gfloat				weight;
	gfloat				density;
	gfloat				temperature;
	gfloat				waterlevel;

	gboolean			online;
}Tank;

typedef enum _SensorParamType
{
	spt_Float				= 0x00,
}SensorParamType;

typedef struct _SensorParam
{
	guint8					num;
	SensorParamType			type;
	guint8					value[SENSOR_PARAM_VALUE_SIZE];
}SensorParam;

typedef struct _SensorParams
{
	SensorParam				params[MAX_SENSOR_PARAM_COUNT];
	guint8					param_count;
}SensorParams;


typedef struct _Sensor
{
	guint32					num;
	guint8					addr;

	SensorParams			params;

	gboolean				online;

	gboolean				data_is_changed;

}Sensor;
//---------------------------------------------------------------------------------------------------------------

#define MAX_GRADE_COUNT							16
#define MAX_PRICE_GROUP_COUNT					8
#define MAX_SERVICE_MODE_COUNT					16
#define MAX_FUELLING_MODE_COUNT					16
#define MAX_FUELLING_MODE_GROUP_COUNT			8
#define MAX_RECEIPT_LINE_COUNT					24
#define MAX_RECEIPT_HEADER_COUNT				4
#define MAX_RECEIPT_FORMAT_COUNT				4
#define MAX_RECEIPT_FORMAT_COUNT				4
#define MAX_TANK_COUNT							16
#define MAX_FUELLING_POINT_COUNT				16
#define MAX_OPERATION_MODE_COUNT				16
#define MAX_HEIGHT_ALARM_COUNT					16
#define MAX_VOLUME_ALARM_COUNT					16
#define MAX_PRICE_POLE_COUNT					4
#define MAX_PAYMENT_TERMINAL_COUNT				8
#define MAX_SELECT_OPTION_COUNT					8
#define MAX_PARAM_COUNT							128
#define MAX_LANG_CODE_LENGTH					3
#define MAX_CURRENCY_CODE_LENGTH				3
#define MAX_FC_SHIFT_NO_LENGTH					3
#define MAX_CONFIG_STRING_LENGTH				128
#define MAX_SERVER_DEVICE_COUNT					16
#define MAX_NOZZLE_ID_LENGTH					16
#define MAX_ATTENDANT_ACCAUNT_ID_LENGTH			16
#define MAX_MULTIMESSAGE_FRAME_LENGTH			255
#define MAX_TRANSACTION_SEQ_COUNT				16
#define MAX_TGS_POINT_COUNT						20
#define MAX_TGS_PARAM_COUNT						2
#define MAX_TGS_SUB_DEV_ID_COUNT				1


#define MAX_IP_ADDRESS_LENGTH					16
#define GUID_LENGTH								40

#define RECONNECT_TIMEOUT						1000

#define ALL_DEVICES_INSTALL_MSG_CODE			0x00
#define FUELLING_POINT_INSTALL_MSG_CODE			0x10
#define PRICE_POLE_INSTALL_MSG_CODE				0x37
#define TANK_GAUGE_INSTALL_MSG_CODE				0x40
#define EPT_INSTALL_MSG_CODE					0x50

#define FP_SS_IS_LOCKED_BY_POS					0x01
#define FP_SS_IS_SUPERVISED						0x02
#define FP_SS_IS_ONLINE							0x04
#define FP_SS_IS_E_STOPPED						0x08
#define FP_SS_HAS_FREE_BUFFER					0x10
#define FP_SS_IS_IN_ERROR_STATE					0x20
#define FP_SS_HAS_ACTIVE_GRADES					0x40
#define FP_SS_IS_PRESET							0x80

#define FP_SS2_PUMP_TOTALS_READY				0x01
#define FP_SS2_VRM_ALARMS						0x02
#define FP_SS2_VRM_ERRORS						0x04
#define FP_SS2_PUMP_IN_MANUAL_MODE				0x08
#define FP_SS2_PRICES_LOCKED					0x10
#define FP_SS2_NOZZLE_HAS_A_TAG_READER			0x20
#define FP_SS2_FUELLING_HALTED					0x40
#define FP_SS2_PUMP_TOTALS_SYNC_WITH_PUMP		0x80

#define TBF_STORED_TRANS						0x01
#define TBF_ERROR_TRANS							0x02
#define TBF_TRAN_MIN_LIMIT						0x04
#define TBF_PREPAY_MODE_USED					0x08
#define TBF_NOT_USED_1							0x10
#define TBF_MONEY_IS_NEGATIVE					0x40
#define TBF_NOT_USED_2							0x80

#define PP_SS_ONLINE							0x01
#define PP_SS_ERROR_ACTIVE						0x02


typedef enum _PSSPricePoleMainState
{
	pppms_Unconfigured					= 0x00,
	pppms_Closed						= 0x01,
	pppms_Idle							= 0x02,
	pppms_Error							= 0x03,
	pppms_Updating						= 0x04,
	pppms_Suspended						= 0x05,
}PSSPricePoleMainState;

typedef enum _PSSTankDataItemId
{
	ptdii_TankProductLevel				= 1,
	ptdii_TankWaterLevel				= 2,
	ptdii_TankTotalObservedVol			= 3,
	ptdii_TankWaterVol					= 4,
	ptdii_TankGrossObservedVol			= 5,
	ptdii_TankGrossStdVol				= 6,
	ptdii_TankAvailableRoom				= 7,
	ptdii_TankAverageTemp				= 8,
	ptdii_TankDataLastUpdateDateTime	= 9,
	ptdii_TankMaxSafeFillCapacity		= 10,
	ptdii_TankShellCapacity				= 11,
	ptdii_TankProductMass				= 14,
	ptdii_TankProductDensity			= 15,
	ptdii_TankProductTcDensity			= 16,
	ptdii_TankDensityProbeTemp			= 17,
	ptdii_TankTempSensor1				= 41,
	ptdii_TankTempSensor2				= 42,
	ptdii_TankTempSensor3				= 43,
}PSSTankDataItemId;


typedef enum _PSSTgMainState
{
	ptgms_Unconfigured					= 0x00,
	ptgms_Operative						= 0x02,
	ptgms_Alarm							= 0x03,
	ptgms_Error							= 0x04,
}PSSTgMainState;


typedef enum _PSSTgCalibState
{
	ptcs_Idle							= 0x01,
	ptcs_Cancelled						= 0x02,
	ptcs_Started						= 0x03,
	ptcs_InProgress						= 0x04,
	ptcs_Ending							= 0x05,
	ptcs_Completed						= 0x06,
	ptcs_CompletedButWithError			= 0x07,
}PSSTgCalibState;

typedef enum _PSSTgCalibStatus
{
	ptcst_Idle							= 0x01,
	ptcst_Cancelled						= 0x02,
	ptcst_Started						= 0x03,
	ptcst_InProgress					= 0x04,
	ptcst_Ending						= 0x05,
	ptcst_Completed						= 0x06,
	ptcst_CompletedButWithError			= 0x07,
}PSSTgCalibStatus;

typedef enum _PSSTgsTgParId
{
	pttpi_VR20Point						= 1,
	pttpi_TankHeight					= 2,
}PSSTgsTgParId;


typedef enum _PSSPresetType
{
	ppt_NoLimit							= 0x00,
	ppt_VolumePresetLimit				= 0x01,
	ppt_MoneyPresetLimit				= 0x02,
	ppt_MoneyFloorLimit					= 0x03,
	ppt_VolumeFloorLimit				= 0x04,
}PSSPresetType;

typedef enum _PSSAuthParId
{
	papi_SmId							= 1,
	papi_FmgId							= 2,
	papi_PgId							= 3,
	papi_ValidGrades					= 4,
	papi_StartLimit						= 5,
	papi_FpTransReturnData				= 6,
	papi_LogData						= 7,
	papi_AutoLockId						= 8,
	papi_FpGradePriceDiscount			= 9,
	papi_LockFpPrices					= 12,
	papi_StartLimit_e					= 13,
	papi_FpGradePriceDiscounts_e		= 14,
	papi_FpTransReturnData2				= 15,
}PSSAuthParId;

typedef enum _FpInstallParId
{
	fpipi_PumpInterfaceType				= 1,
	fpipi_PssChannelNo					= 2,
	fpipi_DeviceCommunicationAddress	= 3,
	fpipi_FpGradeOptions				= 4,
	fpipi_FpGradeOptionsPars			= 5,
	fpipi_PumpInterfaceTypeGeneral		= 6,
	fpipi_PumpInterfaceTypeProtocol		= 7,
	fpipi_PumpDecimalPositionInMoney	= 8,
	fpipi_PumpDecimalPositionInVolume	= 9,
	fpipi_PumpDecimalPositionInPrice	= 10,
	fpipi_PumpDecimalPositionInMoneyTot	= 11,
	fpipi_PumpDecimalPositionInVolumeTot= 12,
	fpipi_PumpInterfaceConfigString		= 13,
	fpipi_NomimalNormalSpeed			= 14,
	fpipi_NominalHighSpeed				= 15,
	fpipi_HighSpeedTriggerLevel			= 16,
}FpInstallParId;

typedef enum _FcDispenceLimitsParId
{
	fcdlpi_GlobalMoneyLimit				= 1,
	fcdlpi_GlobalVolumeLimit			= 2,

}FcDispenceLimitsParId;

typedef enum _FcFuellingModeParId
{
	fcfmpi_FuellingType					= 1,
	fcfmpi_MaxTimeToReachMinLimit		= 2,
	fcfmpi_MaxTimeWithoutProgress		= 3,
	fcfmpi_MaxTransVolume				= 4,
	fcfmpi_MaxTransMoney				= 5,
	fcfmpi_MaxFuellingTime				= 6,
	fcfmpi_MaxPresetVolOverrunErrLimit	= 7,
	fcfmpi_ClrDisplayDelayTime			= 8,
	fcfmpi_ClrDisplayWhenCurTrDisappear	= 9,
	fcfmpi_MinSubPumpRuntimeBeforeStart	= 10,
	fcfmpi_MaxTransVolumeE				= 11,
	fcfmpi_MaxTransMoneyE				= 12,

}FcFuellingModeParId;

typedef enum _FcServModeParId
{
	fcsmpi_AutoAuthorizeLimit			= 1,
	fcsmpi_MaxPreAuthorizeTime			= 2,
	fcsmpi_MaxNzLaydownTime				= 3,
	fcsmpi_ZeroTransToPos				= 4,
	fcsmpi_MoneyDueInTransBufStatus		= 5,
	fcsmpi_MinTransVol					= 6,
	fcsmpi_MinTransMoney				= 7,
	fcsmpi_SupTransBufferSize			= 8,
	fcsmpi_UnsupTransBufferSize			= 9,
	fcsmpi_StoreAtPreAuthorize			= 10,
	fcsmpi_VolInTransBufStatus			= 11,
	fcsmpi_AuthorizeAtModeSelection		= 12,
	fcsmpi_MnoConsecutiveZeroTrans		= 14,
	fcsmpi_AutoClearTransDelayTime		= 15,
	fcsmpi_PumpLightMode				= 16,
	fcsmpi_StopFpOnVehicleTag			= 17,
	fcsmpi_UseVehicleTagReadingButton	= 18,

}FcServModeParId;

typedef enum _FcGenParId
{
	fcgpi_PriceIncreaseDelay			= 1,
	fcgpi_PriceDecreaseDelay			= 2,
	fcgpi_DefaultLanguageCode			= 3,
	fcgpi_DisableFpTotalsError			= 4,
	fcgpi_EnableDemoEncryption			= 5,
	fcgpi_CurrencyCode					= 6,
	fcgpi_FcPumpTotalsHandlingMode		= 10,
	fcgpi_FcShiftNo						= 50,
	fcgpi_SpecificProject				= 54,
	fcgpi_VatRate						= 55,

}FcGenParId;


typedef enum _FcParGroupId
{
	fcpgid_ServiceModes					= 1,
	fcpgid_FuellingModes				= 2,
	fcpgid_FuellingModeGroups			= 3,
	fcpgid_GradeTexts					= 4,
	fcpgid_GradeDescriptors				= 5,
	fcpgid_GradeCodesWithText			= 6,
	fcpgid_WashControl					= 7,
	fcpgid_GradeTextsWithSize			= 8,

	//payment control
	fcpgid_PCEptTimers					= 10,
	fcpgid_PCEptTexts					= 11,
	fcpgid_PCRcpFormats					= 12,
	fcpgid_PCRcpLineDefinitions			= 13,
	fcpgid_PCCardMatchTables			= 14,
	fcpgid_PCCardTypeTables				= 15,
	fcpgid_PCCardHandlingTables			= 16,
	fcpgid_PCFpNames					= 17,
	fcpgid_PCRcpFixedTexts				= 18,
	fcpgid_PCFcGradeSets				= 19,
	fcpgid_PCEptActionsDefinitions		= 20,
	fcpgid_PCReceiptHeaders				= 21,
	fcpgid_PCRcpItemLists				= 22,
	fcpgid_PCReceiptFooters				= 23,

	fcpgid_FloorLimitMargins			= 24,
	fcpgid_GlobalFuellingLimits			= 25,

	//wetstock control
	fcpgid_WSCProdoctTexts				= 40,
	fcpgid_WSCTankGaugeAlarmConfig		= 41,
	fcpgid_WSCTanks						= 42,
	fcpgid_WSCTankAssignedParameters	= 43,
	fcpgid_EPSSetup						= 50,
	fcpgid_SiteParameters				= 90,
	fcpgid_OperatorParameters			= 91,
	fcpgid_GeneralFcParmeters			= 92,
	fcpgid_ExternalSystems				= 93,
	fcpgid_ForecourtControllerTables	= 97,
	fcpgid_BackOfficeRecordDefinitions	= 98,
	fcpgid_PSSInformationBase			= 99,


}FcParGroupId;

typedef enum _PSSTransParId
{
	tpi_FcShiftNo						= 30,
	tpi_ReceiptNo						= 31,
	tpi_BatchNo							= 32,
	tpi_AuthId							= 41,
	tpi_SmId							= 42,
	tpi_FmId							= 43,
	tpi_FpId							= 44,
	tpi_FcPriceGroupId					= 45,
	tpi_FcPriceSetId					= 46,
	tpi_StartLimit						= 47,
	tpi_StartLimit_e					= 48,
	tpi_CurrencyCode					= 49,
	tpi_FcGradeId						= 51,
	tpi_Price							= 52,
	tpi_Vol								= 53,
	tpi_Money							= 54,
	tpi_SecurityTelegram				= 55,
	tpi_FpGradeOptionNo					= 56,
	tpi_FcGradeDescriptor				= 57,
	tpi_SecurityTelegramTypePSS			= 58,
	tpi_SecurityTelegramTypePSS_e		= 59,
	tpi_StartDate						= 61,
	tpi_StartTime						= 62,
	tpi_TransSeqNo						= 63,
	tpi_Price_e							= 64,
	tpi_Vol_e							= 65,
	tpi_Money_e							= 66,
	tpi_TransTerminationStatus			= 71,
	tpi_TransErrorCode					= 72,
	tpi_FinishDate						= 73,
	tpi_FinishTime						= 74,
	tpi_PumpPreset						= 75,
	tpi_PumpPreset_e					= 76,
	tpi_TransAtcInfo					= 79,
	tpi_FpTransReturnData				= 80,
	tpi_FpTransReturnData2				= 81,

}PSSTransParId;

typedef enum _PSSFpMainState
{
	fpms_Unconfigured					= 0x00,
	fpms_Closed							= 0x01,
	fpms_Idle							= 0x02,
	fpms_Error							= 0x03,
	fpms_Calling						= 0x04,
	fpms_PreAuthorized					= 0x05,
	fpms_Starting						= 0x06,
	fpms_StartingPaused					= 0x07,
	fpms_StartingTerminated				= 0x08,
	fpms_Fuelling						= 0x09,
	fpms_FuellingPaused					= 0x0A,
	fpms_FuellingTerminated				= 0x0B,
	fpms_Unavailable					= 0x0C,
	fpms_UnavailableAndCalling			= 0x0D,
}PSSFpMainState;

typedef enum _PSSFpStatusParID
{
	fpspid_SubState2					= 1,
	fpspid_AvailableSms					= 2,
	fpspid_AvailableGrades				= 3,
	fpspid_GradeOptionNo				= 4,
	fpspid_FuellingDataVolE				= 5,
	fpspid_FuellingDataMonE				= 6,
	fpspid_AttendantAccountId			= 7,
	fpspid_NozzleID						= 9,
	fpspid_OperationModeNo				= 10,
	fpspid_PriceGroupID					= 11,
	fpspid_NozzleTagReaderId			= 12,
}PSSFpStatusParID;

typedef enum _ThreadStatus
{
	ts_Undefined			= 0x00,
	ts_Active				= 0x01,
	ts_Destroing			= 0x02,
	ts_Destroyed			= 0x03,
}ThreadStatus;

typedef struct _PSSServerDeviceUnits
{
	guint32						units[MAX_SERVER_DEVICE_UNIT_COUNT];
	guint8						count;
}PSSServerDeviceUnits;



typedef struct _PSSThreadParam
{
	guint32 					port;
	gchar* 						log_dir;
	gboolean 					log_trace;
	gboolean 					log_enable;
	guint32						file_size;
	guint32						save_days;
}PSSThreadParam;

typedef struct _PSSServConfThreadParam
{
	gchar*						ip_address;
	guint16						port;
	gchar*						guid;

	LogParams* 					log_params;
	gboolean 					log_trace;
	gboolean 					log_enable;
}PSSServConfThreadParam;

typedef struct _PSSServConfReadThreadParam
{
	guint32						sock;

	LogParams* 					log_params;
	gboolean 					log_trace;
	gboolean 					log_enable;

	gchar*						guid;
	gchar*						ip_address;
}PSSServConfReadThreadParam;


typedef struct _PSSServerDevice
{
	guint8						id;
	PSSServerDeviceType			device_type;
	gchar*						ip_address;
	guint16						port;
	guint32						timeout;
	gchar*						guid;
	PSSServerDeviceUnits		units;
}PSSServerDevice;

typedef struct _PSSServerDevices
{
	PSSServerDevice				units[MAX_SERVER_DEVICE_COUNT];
	guint8						count;
}PSSServerDevices;


typedef struct _PSSGrade
{
	guint8						id;
	gchar* 						name;
}PSSGrade;

typedef struct _PSSGradePrice
{
	guint8						grade_id;
	guint32 					price;
}PSSGradePrice;

typedef struct _PSSPriceGroup
{
	guint8						id;
	PSSGradePrice 				grade_prices[MAX_GRADE_COUNT];
	guint8						grade_price_count;
}PSSPriceGroup;

typedef struct _PSSGeneralParams
{
	guint32						price_increase_delay;
	guint32						price_decrease_delay;
	gchar						default_language_code[MAX_LANG_CODE_LENGTH + 1];
	gboolean					disable_fp_totals_error;
	gboolean					enable_demo_encription;
	gchar						currency_code[MAX_CURRENCY_CODE_LENGTH + 1];
	guint8						fc_pump_totals_handling_mode;
	gchar						fc_shift_no[MAX_FC_SHIFT_NO_LENGTH + 1];
	guint32						vat_rate;

}PSSGeneralParams;

typedef struct _PSSGeneralFunctions
{
	PSSGrade 					grades[MAX_GRADE_COUNT];
	guint8						grade_count;
	PSSPriceGroup 				price_groups[MAX_PRICE_GROUP_COUNT];
	guint8						price_group_count;
}PSSGeneralFunctions;

typedef struct _PSSServiceMode
{
	guint8						id;
	guint8						auto_autorize_limit;
	guint32						max_pre_auth_time;
	guint32						max_nz_lay_down_time;
	gboolean					zero_trans_to_pos;
	gboolean					money_due_in_trans_buffer_status;
	guint8						min_trans_vol;
	guint8						min_trans_money;
	guint8						sup_trans_buffer_size;
	guint8						unsup_trans_buffer_size;
	gboolean					store_at_pre_authorize;
	gboolean					vol_in_trans_buffer_status;
	gboolean					authorize_at_mode_selection;
	guint8						mno_consecutive_zero_trans;
	guint32						auto_clear_trans_delay_time;
	guint8						pump_light_mode;
	gboolean					stop_fp_on_vehicle_tag;
	gboolean					use_vehicle_tag_reading_button;
	guint32						auto_unlock_trans_delay_time;
}PSSServiceMode;

typedef struct _PSSFuellingMode
{
	guint8						id;
	guint16						fuelling_type;
	guint32						max_time_to_reach_min_limit;
	guint32						max_time_without_progress;
	guint32						max_trans_volume;
	guint32						max_trans_money;
	guint32						max_fuelling_time;
	guint8						max_preset_vol_overrun_err_limit;
	guint32						clr_display_delay_time;
	gboolean					clr_display_when_cur_tr_disappear;
	guint8						min_sub_pump_runtime_before_start;
	guint32						max_trans_volume_e;
	guint32						max_trans_money_e;

}PSSFuellingMode;

typedef struct _PSSFuellingModeGroupItem
{
	guint8						grade_id;
	guint8						fuelling_mode_id;
}PSSFuellingModeGroupItem;


typedef struct _PSSFuellingModeGroup
{
	guint8						id;
	PSSFuellingModeGroupItem   	fuelling_mode_group_items[MAX_GRADE_COUNT];
	guint8						fuelling_mode_group_item_count;
}PSSFuellingModeGroup;

typedef struct _PSSDispenceLimits
{
	guint32						global_money_limit;
	guint32						global_volume_limit;

}PSSDispenceLimits;

typedef struct _PSSDispenceControl
{
	PSSServiceMode 				service_modes[MAX_SERVICE_MODE_COUNT];
	guint8						service_mode_count;
	PSSFuellingMode 			fuelling_modes[MAX_FUELLING_MODE_COUNT];
	guint8						fuelling_mode_count;
	PSSFuellingModeGroup 		fuelling_mode_groups[MAX_FUELLING_MODE_GROUP_COUNT];
	guint8						fuelling_mode_group_count;
	PSSDispenceLimits			dispence_limits;
}PSSDispenceControl;

typedef struct _PSSReceiptLine
{
	guint8						id;
	gchar*						definition;
}PSSReceiptLine;


typedef struct _PSSReceiptHeader
{
	guint8						id;
	guint8						receipt_line_ids[MAX_RECEIPT_LINE_COUNT];
	guint8						receipt_line_id_count;
}PSSReceiptHeader;

typedef struct _PSSReceiptFormat
{
	guint8						id;
	guint8						receipt_line_ids[MAX_RECEIPT_LINE_COUNT];
	guint8						receipt_line_id_count;
}PSSReceiptFormat;


typedef struct _PSSReceipts
{
	PSSReceiptLine 				receipt_lines[MAX_RECEIPT_LINE_COUNT];
	guint8						receipt_line_count;
	PSSReceiptHeader 			receipt_headers[MAX_RECEIPT_HEADER_COUNT];
	guint8						receipt_header_count;
	PSSReceiptFormat 			receipt_formats[MAX_RECEIPT_FORMAT_COUNT];
	guint8						receipt_format_count;
}PSSReceipts;

typedef struct _PSSPaymentControl
{
	PSSReceipts 				receipts;
}PSSPaymentControl;


typedef struct _PSSGeneralConfiguration
{
	PSSGeneralParams 			general_params;
	PSSGeneralFunctions 		general_functions;
	PSSDispenceControl 			dispence_control;
	PSSPaymentControl 			payment_control;

}PSSGeneralConfiguration;

typedef struct _PSSTransaction
{
	guint16						trans_seq_no;
	guint8						sm_id;
	guint8						trans_lock_id;
	guint8						flags;
	guint8						grade_id;
	guint32						money;
	guint32						volume;
}PSSTransaction;

typedef struct _PSSTransactions
{
	PSSTransaction				units[MAX_TRANSACTION_SEQ_COUNT];
	guint8						count;
	guint32						last_trans_seq_no;
}PSSTransactions;


typedef struct _PSSTank
{
	guint8 						id;

	guint32						height;
	guint32						volume;
	guint32						weight;
	guint32						density;
	guint32						temperature;
	guint32						waterlevel;

	gboolean					online;

}PSSTank;

typedef struct _PSSTanks
{
	PSSTank						units[MAX_TANK_COUNT];
	guint8 						count;
}PSSTanks;


typedef struct PSSTankConnection
{
	guint8 						tank_id;
	guint8 						part;
}PSSTankConnection;

typedef struct _PSSGradeOption
{
	guint8						id;
	guint8						grade_id;
	PSSTankConnection 			tank_connections[MAX_TANK_COUNT];
	guint8 						tank_connection_count;
	gchar 						nozzle_id[MAX_NOZZLE_ID_LENGTH];
	guint8 						nozzle_tag_reader_id;

	guint32						volume_total;
	guint32						money_total;

}PSSGradeOption;

typedef struct _PSSPumpIntDecimalPositions
{
	guint8						in_price;
	guint8						in_volume;
	guint8						in_money;
	guint8 						in_volume_total;
	guint8						in_money_total;
}PSSPumpIntDecimalPositions;

typedef struct _PSSOperationModeServiceMode
{
	guint8						service_mode_id;
	guint8						fuelling_mode_group_id;
	guint8						price_group_id;
}PSSOperationModeServiceMode;


typedef struct _PSSOperationMode
{
	guint8						id;
	guint8						type;
	PSSOperationModeServiceMode service_modes[MAX_SERVICE_MODE_COUNT];
	guint8						service_mode_count;

}PSSOperationMode;

typedef struct _PSSLimits
{
	guint32						floor_limit_margin;
}PSSLimits;

typedef struct _PSSPresetData
{
	guint8 						pos_id;
	guint8 						sm_id;
	guint8 						fmg_id;
	guint8 						pg_id;
	guint8 						valid_grades_count;
	guint8 						valid_grades[MAX_GRADE_COUNT];
	guint8 						grade_id;
	PSSPresetType 				preset_type;
	guint32 					preset_value;
}PSSPresetData;

typedef struct _PSSFuellingPoint
{
	guint8						id;
	guint8						server_device_id;
	guint8						pss_port_no;
	guint16						interface_type_general;
	guint16						interface_type_protocol;
	guint16						pss_ext_protocol_id;
	guint8						phisical_address_type;
	guint32						phisical_address;
	guint16						phisical_address_port;
	guint8						device_sub_address;
	PSSGradeOption 				grade_options[MAX_NOZZLE_COUNT];
	guint8						grade_option_count;
	PSSPumpIntDecimalPositions 	pump_interface_decimal_positions;
	guint8						operation_mode_no;
	PSSOperationMode			operation_modes[MAX_OPERATION_MODE_COUNT];
	guint8 						operation_mode_count;
	gchar						config_string[MAX_CONFIG_STRING_LENGTH];
	guint16						nominal_normal_speed;
	guint16 					nominal_high_speed;
	guint16 					high_speed_trigger_level;
	PSSLimits 					limits;

	gboolean					is_close;
	guint8						sm_id;
	PSSFpMainState				main_state;
	guint8						sub_state;
	guint8						sub_state2;
	guint8						locked_id;
	guint8						grade_id;
	guint32						fuelling_volume;
	guint32						fuelling_money;
	gchar						attendant_accaunt_id[MAX_ATTENDANT_ACCAUNT_ID_LENGTH];

	PSSPresetData				preset_data;

	PSSTransactions				sup_transactions;
	PSSTransactions				unsup_transactions;

	guint32						volume_total;
	guint32						money_total;

	gboolean					reset;
	gboolean					wait_send;

}PSSFuellingPoint;

typedef struct _PSSFuellingPoints
{
	PSSFuellingPoint 			units[MAX_FUELLING_POINT_COUNT];
	guint8						count;
}PSSFuellingPoints;

typedef struct _PSSAlarms
{
	guint8 						height_alarms[MAX_HEIGHT_ALARM_COUNT];
	guint8						height_alarm_count;
	guint8 						volume_alarms[MAX_VOLUME_ALARM_COUNT];
	guint8						volume_alarm_count;

}PSSAlarms;

typedef struct _PSSExtraInstallData
{
	PSSAlarms 					alarms;
}PSSExtraInstallData;

typedef struct _PSSTankGauge
{
	guint8						id;
	guint8						server_device_id;
	guint8						pss_port_no;
	guint16						interface_type;
	guint8						phisical_address;
	guint8						phisical_sub_address;
	guint8						tank_id;
	guint32						tank_height;
	guint32						points[MAX_TGS_POINT_COUNT];
	PSSExtraInstallData 		extra_install_data;
	PSSTgMainState				main_state;
	guint8						sub_state;

}PSSTankGauge;

typedef struct _PSSTankGaugeSubDevice
{
	guint8						id;
	guint16						interface_type;
	guint8						pss_channel_no;
	guint8						main_address;
	guint8						sub_address;
	guint8						phisical_sub_address;
	guint8						sub_dev_ids[MAX_TGS_SUB_DEV_ID_COUNT];
	guint8						sub_dev_ids_count;
}PSSTankGaugeSubDevice;

typedef struct _PSSTankGauges
{
	PSSTankGauge				units[MAX_TANK_COUNT];
	guint8						count;
}PSSTankGauges;

typedef struct _PSSTankGaugeSubDevices
{
	PSSTankGaugeSubDevice		units[MAX_TANK_COUNT];
	guint8						count;
}PSSTankGaugeSubDevices;

typedef struct _PSSPricePositionOption
{
	guint8						option_no;
	guint8						fc_price_group_id;
	guint8						fc_grade_id;
}PSSPricePositionOption;


typedef struct _PSSPricePole
{
	guint8						id;
	guint8						server_device_id;
	guint8						pss_port_no;
	guint16						interface_type;
	guint8						phisical_address;
	PSSPricePositionOption		options[MAX_GRADE_COUNT];
	guint8						option_count;
	PSSPricePoleMainState		main_state;
	guint8						sub_state;
}PSSPricePole;

typedef struct _PSSPricePoles
{
	PSSPricePole				units[MAX_PRICE_POLE_COUNT];
	guint8						count;
}PSSPricePoles;

typedef struct _PSSSelectOption
{
	guint8						id;
}PSSSelectOption;

typedef struct _PSSPaymentTerminal
{
	guint8						id;
	guint8						server_device_id;
	guint8						pss_port_no;
	guint16						interface_type;
	guint8						phisical_address;
	PSSSelectOption 			select_options[MAX_SELECT_OPTION_COUNT];
	guint8 						select_option_count;
}PSSPaymentTerminal;

typedef struct _PSSPaymentTerminals
{
	PSSPaymentTerminal			units[MAX_PAYMENT_TERMINAL_COUNT];
	guint8						count;
}PSSPaymentTerminals;

typedef struct _PSSDevices
{
	PSSServerDevices			server_devices;
	PSSTanks 					tanks;
	PSSFuellingPoints 			fuelling_points;
	PSSTankGauges 				tank_gauges;
	PSSTankGaugeSubDevices		tank_gauge_sub_devices;
	PSSPricePoles 				price_poles;
	PSSPaymentTerminals 		payment_terminals;
}PSSDevices;

typedef struct _PSSFcStatus
{
	guint8						fc_status_1_flags;
	guint8						fc_status_2_flags;
	guint8						fc_service_msg_seq_no;
	time_t						fc_master_reset_date_and_time;
	guint8						fc_master_reset_code;
	time_t						fc_reset_date_and_time;
	guint8						fc_reset_code;
	guint8						fc_price_set_id;
	time_t						fc_price_set_date_and_time;
	guint8						fc_operation_mode_no;
	time_t						fc_operation_mode_date_and_time;


}PSSFcStatus;


typedef struct _PSSCommon
{
	gchar*						log_dir;
	gboolean					log_enable;
	gboolean					log_trace;
	PSSFcStatus					fc_status;
}PSSCommon;

typedef struct _PSSData
{
	PSSCommon					common;
	PSSGeneralConfiguration 	general_configuration;
	PSSDevices 					devices;
}PSSData;


void lock_threads_mutex();
void unlock_threads_mutex();

void get_conf_filename(gchar** value);


#endif
