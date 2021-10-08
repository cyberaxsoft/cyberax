#ifndef SHTRIH_H_
#define SHTRIH_H_

#define MAX_RECEIPT_ITEM_COUNT		512

#define	READ_BUFFER_SIZE			255
#define WRITE_BUFFER_SIZE			255

#define MAX_UART_ERROR				20

#define ctrl_STX					0x02
#define ctrl_ENQ					0x05
#define ctrl_ACK					0x06
#define ctrl_NAK					0x15

#define LENGTH_OFFSET				1

#define ADMIN_PASSWORD				0x0000001E
#define OPERATOR_PASSWORD			0x00000001

#define COMMAND_REPLY_OFFSET		0
#define ERROR_REPLY_OFFSET			1
#define OPERATOR_REPLY_OFFSET		2

typedef enum _ExchangeState
{
	es_Ready			= 0,
	es_EnqSent			= 1,
	es_CommandReady		= 2,
	es_DataSent			= 3,
	es_WaitSTX			= 4,
	es_WaitLength		= 5,
	es_WaitData			= 6,
	es_WaitLrc			= 7,
	es_ReadyAck			= 8,
	es_ReadyNak			= 9,

}ExchangeState;



typedef enum _ShtrihCommand
{
	sc_GettDump					= 0x0001,
	sc_GetData					= 0x0002,
	sc_DataInterrupt			= 0x0003,
	sc_GetState					= 0x0010,
	sc_GetExState				= 0x0011,
	sc_PrintBoldText			= 0x0012,
	sc_Beep						= 0x0013,
	sc_SetExchangeParam			= 0x0014,
	sc_ReadExchangeParam		= 0x0015,
	sc_HardwareReset			= 0x0016,
	sc_PrintText				= 0x0017,
	sc_PrintHeader				= 0x0018,
	sc_TextPrint				= 0x0019,
	sc_GetMoneyReg				= 0x001A,
	sc_GetOperationReg			= 0x001B,
	sc_SaveTable				= 0x001E,
	sc_ReadTable				= 0x001F,
	sc_SetTime					= 0x0021,
	sc_SetDate					= 0x0022,
	sc_ConfirmDate				= 0x0023,
	sc_TablesInit				= 0x0024,
	sc_Cut						= 0x0025,
	sc_GetFontParam				= 0x0026,
	sc_FullRegisterClear		= 0x0027,
	sc_OpenDrawer  				= 0x0028,
	sc_FeedPaper  				= 0x0029,
	sc_InterruptPrintTest		= 0x002B,
	sc_GetOperationRegisters	= 0x002C,
	sc_GetTableStructure		= 0x002D,
	sc_GetRecordStructure		= 0x002E,
	sc_PrintTextFont			= 0x002F,
	sc_XReport					= 0x0040,
	sc_ZReport					= 0x0041,
	sc_PrintImage512			= 0x004D,
	sc_LoadImage512				= 0x004E,
	sc_PrintImage				= 0x004F,
	sc_Income					= 0x0050,
	sc_Output					= 0x0051,
	sc_PrintCliche				= 0x0052,
	sc_CloseDocument			= 0x0053,
	sc_PrintPromoText			= 0x0054,
	sc_GetErrorDescription		= 0x006B,
	sc_Sale						= 0x0080,
	sc_Buy						= 0x0081,
	sc_ReturnSale				= 0x0082,
	sc_ReturnBuy				= 0x0083,
	sc_CloseReceipt				= 0x0085,
	sc_CancelReceipt			= 0x0088,
	sc_SubTotal					= 0x0089,
	sc_RepeatDoc				= 0x008C,
	sc_OpenReceipt				= 0x008D,
	sc_CloseReceiptExt			= 0x008E,
	sc_ContinuePrint			= 0x00B0,
	sc_LoadGraphics 			= 0x00C0,
	sc_PrintGraphics 			= 0x00C1,
	sc_PrintEAN13	 			= 0x00C2,
	sc_PrintGraphicsExt 		= 0x00C3,
	sc_LoadGraphicsExt 			= 0x00C4,
	sc_PrintLine	 			= 0x00C5,
	sc_ZReportCash	 			= 0x00C6,
	sc_PrintBarcodeHardware		= 0x00CB,
	sc_GetPrinterStateExt		= 0x00D0,
	sc_GetPrinterState			= 0x00D1,
	sc_LoadData					= 0x00DD,
	sc_PrintBarcode				= 0x00DE,
	sc_OpenShift				= 0x00E0,
	sc_ExtRequest				= 0x00F7,
	sc_GetDeviceType			= 0x00FC,
	sc_GetFNStatus				= 0xFF01,
	sc_GetFNNumber				= 0xFF02,
	sc_GetFNPeriod				= 0xFF03,
	sc_GetFNVersion				= 0xFF04,
	sc_BeginRegistrationRep		= 0xFF05,
	sc_CloseRegistrationRep		= 0xFF06,
	sc_ResetFN					= 0xFF07,
	sc_CancelDocFN				= 0xFF08,
	sc_GetLastFiscalizationTot	= 0xFF09,
	sc_GetDocFromNum			= 0xFF0A,
	sc_OpenShiftFN				= 0xFF0B,
	sc_TLVSend					= 0xFF0C,
	sc_DiscountOperation		= 0xFF0D,
	sc_GetOpenFNParam			= 0xFF0E,
	sc_GetDataConsFromBuffer	= 0xFF30,
	sc_GetDataBlockFromBuffer	= 0xFF31,
	sc_StartDataSaveToBuffer	= 0xFF32,
	sc_DataSaveToBuffer			= 0xFF33,
	sc_ReregistrationRep		= 0xFF34,
	sc_OpenCorrectionReceipt	= 0xFF35,
	sc_CloseCorrectionReceipt	= 0xFF36,
	sc_StartPaymentStatusRep	= 0xFF37,
	sc_PaymentStatusRep			= 0xFF38,
	sc_GetStatusInfExchange 	= 0xFF39,
	sc_GetTLVDoc		 		= 0xFF3A,
	sc_ReadTLVDoc		 		= 0xFF3B,
	sc_GetOFDDocStatus	 		= 0xFF3C,
	sc_StartClosingFiscalMode	= 0xFF3D,
	sc_CloseFiscalMode			= 0xFF3E,
	sc_GetFDDocWithoutCheck		= 0xFF3F,
	sc_GetCurrentShiftParam		= 0xFF40,
	sc_OpenFnShift				= 0xFF41,
	sc_StartCloseFnShift		= 0xFF42,
	sc_CloseFnShift				= 0xFF43,
	sc_CloseReceiptV2			= 0xFF45,
	sc_OperationV2				= 0xFF46,
	sc_CreateCorrectionReceipt	= 0xFF4A,
	sc_DiscountRosneft			= 0xFF4B,
	sc_GetFiscalizationTotV2	= 0xFF4C,
	sc_SendOperationTLV			= 0xFF4D,
	sc_SaveFnToSD				= 0xFF4E,
	sc_OnlinePayment			= 0xFF50,
	sc_OnlinePaymentStatus		= 0xFF51,
	sc_GetLastOnlinePayment		= 0xFF52,
	sc_GetFiscalizationParam	= 0xFF60,
	sc_CheckMarkGood			= 0xFF61,
	sc_SyncRegisters			= 0xFF62,
	sc_GetMemorySize			= 0xFF63,
	sc_SendTLVFromBuffer		= 0xFF64,
	sc_GetRandomData			= 0xFF65,
	sc_Authorization			= 0xFF66,
	sc_SetMarkGoodPosition		= 0xFF67,
	sc_GetMarkState				= 0xFF68,
	sc_ComplMarkGood			= 0xFF69,
}ShtrihCommand;


typedef enum _ShtrihError
{
	se_NoError					= 0x00,
	se_CommandError				= 0x01,
	se_FNStateError				= 0x02,
	se_FNError					= 0x03,
	se_KSError					= 0x04,
	se_ParamFNError				= 0x05,
	se_DateTimeFault			= 0x07,
	se_DataNotFound 			= 0x08,
	se_ParamFault	 			= 0x09,
	se_TLVSizeError	 			= 0x10,
	se_TransportConnectionError	= 0x11,
	se_BadFN					= 0x12,
	se_ResourceFN				= 0x14,
	se_24h						= 0x16,
	se_DateTimeError			= 0x17,
	se_OFDMessageError			= 0x20,
	se_FNTimeout				= 0x2F,
	se_FNNotResponse			= 0x30,
	se_FaultParam				= 0x33,
	se_NoData					= 0x34,
	se_ParamSettingsCollision	= 0x35,
	se_ParamVersionCollision	= 0x36,
	se_CommandVersionCollision	= 0x37,
	se_EPROMError				= 0x38,
	se_SoftwareError			= 0x39,
	se_ShiftOverload 			= 0x3A,
	se_OperationOpenShift		= 0x3C,
	se_OperationOpenShift2		= 0x3D,
	se_ShiftSectionOverload		= 0x3E,
	se_ShiftDiscountOverload	= 0x3F,
	se_DiscountRangeOverload	= 0x40,
	se_CashRangeOverload		= 0x41,
	se_Type2RangeOverload		= 0x42,
	se_Type3RangeOverload		= 0x43,
	se_Type4RangeOverload		= 0x44,
	se_SumError					= 0x45,
	se_CashError				= 0x46,
	se_TaxError					= 0x47,
	se_OverloadTotalReceipt		= 0x48,
	se_OperationErrorOpenReceipt= 0x49,
	se_OpenReceiptError			= 0x4A,
	se_ReceiptBufferError		= 0x4B,
	se_TaxShiftError			= 0x4C,
	se_BnSumError				= 0x4D,
	se_Shift24					= 0x4E,
	se_PasswordError			= 0x4F,
	se_PrintInProgress			= 0x50,
	se_CashShiftError			= 0x51,
	se_Type2ShiftError			= 0x52,
	se_Type3ShiftError			= 0x53,
	se_Type4ShiftError			= 0x54,
	se_CloseReceiptError		= 0x55,
	se_RepeatDocError			= 0x56,
	se_WaitContinuePrintCmd		= 0x58,
	se_OperatorDocError			= 0x59,
	se_OverloadRangeBonus		= 0x5B,
	se_LowVoltage				= 0x5C,
	se_TableNotDefine			= 0x5D,
	se_OperationError			= 0x5E,
	se_NegativTotal				= 0x5F,
	se_OverloadMultiply			= 0x60,
	se_OverloadPriceRange		= 0x61,
	se_OverloadCountRange		= 0x62,
	se_OverloadDepRange			= 0x63,
	se_NoMoneyInSection			= 0x65,
	se_OverloadSectionMoney		= 0x66,
	se_NoTaxMoney				= 0x68,
	se_OverloadTaxMoney			= 0x69,
	se_PowerErrorI2C			= 0x6A,
	se_NoPaper					= 0x6B,
	se_NoSingleTaxMoney			= 0x6D,
	se_OverloadSingleTaxMoney	= 0x6E,
	se_OverloadOutputShift		= 0x6F,
	se_CutterError				= 0x71,
	se_CommandNotSuppInSubMode	= 0x72,
	se_CommandNotSuppInMode		= 0x73,
	se_RAMError					= 0x74,
	se_PowerError				= 0x75,
	se_PrinterSensorError		= 0x77,
	se_SofwareUpload			= 0x78,
	se_FieldReadOnly			= 0x7A,
	se_HardwareError			= 0x7B,
	se_DateError				= 0x7C,
	se_FormatDateError			= 0x7D,
	se_LengthError				= 0x7E,
	se_OverloadTotalRangeError	= 0x7F,
	se_OverloadCash				= 0x84,
	se_OverloadSaleShift		= 0x85,
	se_OverloadBuyShift			= 0x86,
	se_OverloadReturnSaleShift	= 0x87,
	se_OverloadReturnBuyShift	= 0x88,
	se_OverloadIncomeShift		= 0x89,
	se_OverloadBonusShift		= 0x8A,
	se_OverloadDiscountShift  	= 0x8B,
	se_NegativeBonusTotal		= 0x8C,
	se_NegativeDiscountTotal	= 0x8D,
	se_ZeroTotal				= 0x8E,
	se_VeryBigFieldSize			= 0x90,
	se_overloadPrintArrea		= 0x91,
	se_FieldsCollision			= 0x92,
	se_RAMRestore				= 0x93,
	se_OverloadLimitOperation	= 0x94,
	se_MTError					= 0xA0,
	se_FaultSequence			= 0xA1,
	se_MTBlocked				= 0xA2,
	se_OverloadMTTable			= 0xA3,
	se_BlockTLVError			= 0xA4,
	se_2007Error				= 0xA5,
	se_DateTimeControl			= 0xC0,
	se_HiVoltage				= 0xC2,
	se_ShiftNumError			= 0xC4,
	se_FieldReadOnlyInMode		= 0xC7,
	se_PrinterConnectError		= 0xC8,

}ShtrihError;

typedef enum _ShtrihMode
{
	sm_DataSending				= 0x01,
	sm_ShiftOpened				= 0x02,
	sm_ShiftOpened24			= 0x03,
	sm_ShiftClosed				= 0x04,
	sm_Blocked					= 0x05,
	sm_DateConfirmation			= 0x06,
	sm_DeciamlPointSetup		= 0x07,
	sm_DocOpened				= 0x08,
	sm_TechReset				= 0x09,
	sm_TestPrint				= 0x0A,
	sm_FiscalRep				= 0x0B,
	sm_FiscalA4					= 0x0C,
	sm_PrintA4					= 0x0D,
	sm_FiscalA4Complete			= 0x0E,
}ShtrihMode;

typedef enum _ShtrihSubMode
{
	ssm_Idle					= 0x00,
	ssm_NoPaperPassive			= 0x01,
	ssm_NoPaperActive			= 0x02,
	ssm_WaitingPrintContinue	= 0x03,
	ssm_FiscalRep				= 0x04,
	ssm_Printing				= 0x05,
}ShtrihSubMode;



typedef enum _ShtrihDocOpenedStatus
{
	sdos_Sale					= 0x00,
	sdos_Buy					= 0x01,
	sdos_ReturnSale				= 0x02,
	sdos_ReturnBuy				= 0x03,
	sdos_NotFiscal				= 0x04,
}ShtrihDocOpenedStatus;


typedef enum _ShtrihFiscalA4Status
{
	sfas_Sale					= 0x00,
	sfas_Buy					= 0x01,
	sfas_ReturnSale				= 0x02,
	sfas_ReturnBuy				= 0x03,
}ShtrihFiscalA4Status;

typedef enum _ShtrihPrintA4Status
{
	spas_WaitLoad				= 0x00,
	spas_Loading				= 0x01,
	spas_Positioning			= 0x02,
	spas_Printing				= 0x03,
	spas_Printed				= 0x04,
	spas_Presenting				= 0x05,
	spas_WaitRemoving			= 0x06,

}ShtrihPrintA4Status;

typedef struct _Receipt
{
	guint32 				num;
}Receipt;


ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* SHTRIH_H_ */
