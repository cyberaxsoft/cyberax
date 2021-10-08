#ifndef TLV_H_
#define TLV_H_

//------------------------------------------------------------------------------------------------------------------------

typedef enum _FrameControlCharacter
{
	ctrl_STX			= 0x02,
	ctrl_ETX			= 0x03,
	ctrl_ETB			= 0x17,
}FrameControlCharacter;

typedef enum _ExchangeState
{
	es_None				= 0,
	es_WaitStartMessage	= 1,
	es_WaitSTX			= 2,
	es_ReadLength1		= 3,
	es_ReadLength2		= 4,
	es_ReadLength3		= 5,
	es_ReadLength4		= 6,
	es_ReadData			= 7,
	es_WaitEndMessage	= 8,
	es_WaitCRC1			= 9,
	es_WaitCRC2			= 10,
}ExchangeState;

typedef enum _ParseState
{
	ps_ReadTag1			= 0,
	ps_ReadTag2			= 1,
	ps_ReadLen			= 2,
	ps_ReadLenEx		= 3,
	ps_ReadData			= 4,
}ParseState;

typedef enum _MessageType
{
	mt_Undefined		= 0,
	mt_Request			= 1,
	mt_Reply			= 2,
	mt_Ack				= 3,
	mt_Nak				= 4,
	mt_Event			= 5,
	mt_Eot				= 6,
}MessageType;

typedef enum _CommandClass
{
	cc_AllDevices		= 0,
	cc_Specific			= 1,
}CommandClass;

typedef enum _ExchangeError
{
	ee_None						= 0x00,
	ee_Format					= 0x01,
	ee_Crc						= 0x02,
	ee_Parse					= 0x03,
	ee_UndefinedClient			= 0x04,
	ee_UndefineCommandCode		= 0x05,
	ee_UndefinedMessageType		= 0x06,
	ee_FaultMessageType			= 0x07,
	ee_LimitBadFrame			= 0x08,
	ee_MessageID				= 0x09,
	ee_FaultParam				= 0x0A,
	ee_TaskBufferError			= 0x0B,
	ee_DeviceIsBusy				= 0x0C,
	ee_FatalDeviceDriverError	= 0x0D,
	ee_DeviceDriverError		= 0x0E,
	ee_Timeout					= 0x0F,
	ee_AccessDenied				= 0x10,
	ee_OutOfRange				= 0x11
}ExchangeError;

typedef struct _TlvUnit
{
	guint16			tag;
	guint32			length;
	guchar*			value;
	gpointer		next;
}TlvUnit;

//---------------------------------------------system---------------------------------------------------------

typedef enum _TlvSystenTag
{
	tst_None						= 0x0000,
	tst_GuidClient					= 0x0001,
	tst_MessageId					= 0x0002,
	tst_MessageType					= 0x0003,
	tst_CommandCode					= 0x0004,
	tst_DeviceNumber				= 0x0005,
	tst_NozzleNumber				= 0x0006,
	tst_Price						= 0x0007,
	tst_Quantity					= 0x0008,
	tst_Amount						= 0x0009,
	tst_ExtendedFuncIndex			= 0x000A,
	tst_DeviceReplyCode				= 0x000B,

	tst_PricePack					= 0x0029,

	tst_ServerCommonSettings		= 0x00A0,
	tst_ClientProfileCount			= 0x00A1,
	tst_ClientProfile				= 0x00A2,
	tst_DispencerControllerCount	= 0x00A3,
	tst_DispencerControllerConfig	= 0x00A4,
	tst_TgsCount					= 0x00A5,
	tst_TgsConfig					= 0x00A6,
	tst_PpcCount					= 0x00A7,
	tst_PpcConfig					= 0x00A8,
	tst_ScCount						= 0x00A9,
	tst_ScConfig					= 0x00AA,


	tst_Error						= 0x0081,
	tst_DeviceStatus				= 0x0082,
	tst_DeviceStatusDescription		= 0x0083,
	tst_DeviceError					= 0x0084,
	tst_DeviceErrorDescription  	= 0x0085,

	tst_DCDeviceConfiguration		= 0x0101,
	tst_DCData						= 0x0102,
	tst_DCCounters					= 0x0103,

	tst_TGSDeviceConfiguration		= 0x0201,
	tst_TGSTankData					= 0x0202,
	tst_TGSAlarms					= 0x0203,
	tst_TGSAlarmsCount				= 0x0204,

	tst_PPCDeviceConfiguration		= 0x0601,
	tst_PPCPricePoleData			= 0x0602,

	tst_SCDeviceConfiguration		= 0x0901,
	tst_SCData						= 0x0902,


}TlvSystemTag;

typedef enum _TlvModuleLogSettingsTag
{
	tmlst_Enable					= 0x0001,
	tmlst_Directory					= 0x0002,
	tmlst_Trace						= 0x0003,
	tmlst_System					= 0x0004,
	tmlst_Requests					= 0x0005,
	tmlst_Frames					= 0x0006,
	tmlst_Parsing					= 0x0007,
	tmlst_FileSize					= 0x0008,
	tmlst_SaveDays					= 0x0009,
}TlvModuleLogSettingsTag;

typedef enum _TlvModuleConnSettingsTag
{
	tmcst_Type						= 0x0001,
	tmcst_Port						= 0x0002,
	tmcst_IPAddress					= 0x0003,
	tmcst_IPPort					= 0x0004,
	tmcst_UartBaudrate				= 0x0005,
	tmcst_UartByteSize				= 0x0006,
	tmcst_UartParity				= 0x0007,
	tmcst_UartStopBits				= 0x0008,
}TlvModuleConnSettingsTag;

typedef enum _TlvModuleTimeoutSettingsTag
{
	tmtst_Read						= 0x0001,
	tmtst_Write						= 0x0002,
}TlvModuleTimeoutSettingsTag;

typedef enum _TlvPricePackTag
{
	tppt_Id							= 0x0001,
	tppt_Price						= 0x0002,
}TlvDCNozzlePricePackTag;



//------------------------------------------- dispencer controller -------------------------------------------------------


typedef enum _TlvDcModuleDpSettingsTag
{
	tdcmdpst_Price					= 0x0001,
	tdcmdpst_Volume					= 0x0002,
	tdcmdpst_Amount					= 0x0003,
}TlvDcModuleDpSettingsTag;

typedef enum _TlvDcModuleMappingTag
{
	tdmmt_DispencerCount			= 0x0001,
	tdmmt_DispencerConfiguration	= 0x0002,
}TlvDcModuleMappingTag;

typedef enum _TlvModuleDispencerConfigurtaionTag
{
	tmdct_Number					= 0x0001,
	tmdct_Address					= 0x0002,
	tmdct_NozzleCount				= 0x0003,
	tmdct_Nozzle					= 0x0004,
}TlvModuleDispencerConfigurationTag;

typedef enum _TlvModuleNozzleConfigurtaionTag
{
	tmnct_Number					= 0x0001,
	tmnct_Grade						= 0x0002,
}TlvModuleNozzleConfigurationTag;

typedef enum _TlvDcModuleSettingsTag
{
	tdcmst_LogSettings				= 0x0001,
	tdcmst_ConnSettings				= 0x0002,
	tdcmst_TimeoutSettings			= 0x0003,
	tdcmst_DPSettings				= 0x0004,
	tdcmst_CountersEnable			= 0x0005,
	tdcmst_AutoStart				= 0x0006,
	tdcmst_AutoPayment				= 0x0007,
	tdcmst_FullTankVolume			= 0x0008,
	tdcmst_ModuleMapping			= 0x0009,

}TlvDcModuleSettingsTag;

typedef enum _TlvDcSystemTag
{
	tdst_Id							= 0x0001,
	tdst_Name						= 0x0002,
	tdst_Port						= 0x0003,
	tdst_Enable						= 0x0004,
	tdst_CommandTimeout				= 0x0005,
	tdst_Interval					= 0x0006,
	tdst_Module						= 0x0007,
	tdst_LogDir						= 0x0008,
	tdst_LogEnable					= 0x0009,
	tdst_LogTrace					= 0x000A,
	tdst_LogFileSize				= 0x000B,
	tdst_LogSaveDays				= 0x000C,
	tdst_ModuleSettings				= 0x000D,

}TlvSystemDcTag;


typedef enum _TlvDCConfigurationTag
{
	tdcct_DispencerCount			= 0x0001,
	tdcct_DispencerConfiguration	= 0x0002,
	tdcct_ExtendedFuncCount			= 0x0003,
	tdcct_ExtendedFunction			= 0x0004,
	tdcct_PriceDP					= 0x0005,
	tdcct_VolumeDP					= 0x0006,
	tdcct_AmountDP					= 0x0007,
}TlvDCConfigurationTag;

typedef enum _TlvDCDispencerConfigurationTag
{
	tdcdct_Number					= 0x0001,
	tdcdct_Address					= 0x0002,
	tdcdct_NozzleCount				= 0x0003,
	tdcdct_Nozzle					= 0x0004,
}TlvDCDispencerConfigurationTag;

typedef enum _TlvDCNozzleConfigurationTag
{
	tdcnct_Number					= 0x0001,
	tdcnct_Grade					= 0x0002,
}TlvDCNozzleConfigurationTag;

typedef enum _TlvDCExtendedFuncTag
{
	tdceft_Index					= 0x0001,
	tdceft_Name						= 0x0002,
}TlvDCExtendedFuncTag;

typedef enum _TlvDCDataTag
{
	tdcdt_Count						= 0x0001,
	tdcdt_DispencerData				= 0x0002,
}TlvDCDataTag;

typedef enum _TlvDCDispencerDataTag
{
	tdcddt_Number					= 0x0001,
	tdcddt_State					= 0x0002,
	tdcddt_OrderType				= 0x0003,
	tdcddt_PresetOrderType			= 0x0004,
	tdcddt_IsPay					= 0x0005,
	tdcddt_PresetNozzleNum			= 0x0006,
	tdcddt_ActiveNozzleNum			= 0x0007,
	tdcddt_PresetPrice				= 0x0008,
	tdcddt_PresetVolume				= 0x0009,
	tdcddt_PresetAmount				= 0x000A,
	tdcddt_CurrentPrice				= 0x000B,
	tdcddt_CurrentVolume			= 0x000C,
	tdcddt_CurrentAmount			= 0x000D,
	tdcddt_ErrorCode				= 0x000E,
	tdcddt_ErrorDescription			= 0x000F,
	tdcddt_Counters					= 0x0010,
}TlvDCDispencerDataTag;

typedef enum _TlvDCCountersTag
{
	tdccnt_Count					= 0x0001,
	tdccnt_DispencerCountersData	= 0x0002,
}TlvDCCountersTag;

typedef enum _TlvDCDispencerCountersDataTag
{
	tdcdcdt_DispencerNumber			= 0x0001,
	tdcdcdt_NozzleCount				= 0x0002,
	tdcdcdt_NozzleCounterData		= 0x0003,
}TlvDCDispencerCountersDataTag;


typedef enum _TlvDCNozzleCounterDataTag
{
	tdcncdt_Number					= 0x0001,
	tdcncdt_Counter					= 0x0002,
}TlvDCNozzleCounterDataTag;

//------------------------------------------- tgs controller -------------------------------------------------------

typedef enum _TlvModuleTankConfigurtaionTag
{
	tmtct_Number					= 0x0001,
	tmtct_Channel					= 0x0002,
}TlvModuleTankConfigurationTag;

typedef enum _TlvTgsModuleMappingTag
{
	ttmmt_TankCount					= 0x0001,
	ttmmt_TankConfiguration			= 0x0002,
}TlvTgsModuleMappingTag;

typedef enum _TlvTgsModuleSettingsTag
{
	ttgsmst_LogSettings				= 0x0001,
	ttgsmst_ConnSettings			= 0x0002,
	ttgsmst_TimeoutSettings			= 0x0003,
	ttgsmst_ModuleMapping			= 0x0004,

}TlvTgsModuleSettingsTag;

typedef enum _TlvTgsSystemTag
{
	ttst_Id							= 0x0001,
	ttst_Name						= 0x0002,
	ttst_Port						= 0x0003,
	ttst_Enable						= 0x0004,
	ttst_CommandTimeout				= 0x0005,
	ttst_Interval					= 0x0006,
	ttst_Module						= 0x0007,
	ttst_LogDir						= 0x0008,
	ttst_LogEnable					= 0x0009,
	ttst_LogTrace					= 0x000A,
	ttst_LogFileSize				= 0x000B,
	ttst_LogSaveDays				= 0x000C,
	ttst_ModuleSettings				= 0x000D,
}TlvTgsSystemTag;

//Configuration

typedef enum _TlvTGSConfigurationTag
{
	ttgsct_TankCount		 		= 0x0001,
	ttgsct_TankConfiguration		= 0x0002,
}TlvTGSConfigurationTag;

typedef enum _TlvTGSTankConfigurationTag
{
	ttgstct_Number					= 0x0001,
	ttgstct_Channel					= 0x0002,
}TlvTGSTankConfigurationTag;

//Data

typedef enum _TlvTGSDataTag
{
	ttgsdt_Count	 				= 0x0001,
	ttgsdt_TankData					= 0x0002,
}TlvTGSDataTag;

typedef enum _TlvTGSTankDataTag
{
	ttgstdt_Number					= 0x0001,
	ttgstdt_Height					= 0x0002,
	ttgstdt_Volume					= 0x0003,
	ttgstdt_Weight					= 0x0004,
	ttgstdt_Density					= 0x0005,
	ttgstdt_Temperature				= 0x0006,
	ttgstdt_WaterLevel				= 0x0007,
	ttgstdt_Online					= 0x0008,
}TlvTGSTankDataTag;

//------------------------------------------- price pole controller -------------------------------------------------------

typedef enum _TlvPpcModuleDpSettingsTag
{
	tmppcdpst_Price					= 0x0001,
}TlvModulePpcDpSettingsTag;

typedef enum _TlvPricePoleModuleConfigurtaionTag
{
	tppmct_Number					= 0x0001,
	tppmct_Grade					= 0x0002,
	tppmct_SymbolCount				= 0x0003,
}TlvPricePoleModuleConfigurtaionTag;

typedef enum _TlvPpcModuleMappingTag
{
	tppcmmt_PricePoleCount			= 0x0001,
	tppcmmt_PricePoleConfiguration	= 0x0002,

}TlvPpcModuleMappingTag;

typedef enum _TlvPpcModuleSettingsTag
{
	tppcmst_LogSettings				= 0x0001,
	tppcmst_ConnSettings			= 0x0002,
	tppcmst_TimeoutSettings			= 0x0003,
	tppcmst_DpSettings				= 0x0004,
	tppcmst_ModuleMapping			= 0x0005,

}TlvPpcModuleSettingsTag;

typedef enum _TlvSystemPpcTag
{
	tspt_Id							= 0x0001,
	tspt_Name						= 0x0002,
	tspt_Port						= 0x0003,
	tspt_Enable						= 0x0004,
	tspt_CommandTimeout				= 0x0005,
	tspt_Interval					= 0x0006,
	tspt_Module						= 0x0007,
	tspt_LogDir						= 0x0008,
	tspt_LogEnable					= 0x0009,
	tspt_LogTrace					= 0x000A,
	tspt_LogFileSize				= 0x000B,
	tspt_LogSaveDays				= 0x000C,
	tspt_ModuleSettings				= 0x000D,

}TlvSystemPpcTag;

//configuration

typedef enum _TlvPpcConfigurationTag
{
	tppcct_PricePoleCount			= 0x0001,
	tppcct_PricePoleConfiguration	= 0x0002,
	tppcct_PriceDp					= 0x0003,
}TlvPpcConfigurationTag;

typedef enum _TlvPpcPricePoleConfigurationTag
{
	tppcppct_Number					= 0x0001,
	tppcppct_Grade					= 0x0002,
	tppcppct_SymbolCount			= 0x0003,
}TlvPpcPricePoleConfigurationTag;

//Data
typedef enum _TlvPpcDataTag
{
	tppcdt_Count	 				= 0x0001,
	tppcdt_PricePoleData			= 0x0002,
}TlvPpcDataTag;

typedef enum _TlvPpcPricePoleDataTag
{
	tppcppdt_Number					= 0x0001,
	tppcppdt_Grade					= 0x0002,
	tppcppdt_Price					= 0x0003,
	tppcppdt_State					= 0x0004,
}TlvPpcPricePoleDataTag;

//------------------------------------------- sensor controller -------------------------------------------------------

typedef enum _TlvModuleSensorParamConfigurtaionTag
{
	tmspct_Number					= 0x0001,
	tmspct_Type						= 0x0002,
}TlvModuleSensorParamConfigurtaionTag;

typedef enum _TlvModuleSensorConfigurtaionTag
{
	tmsct_Number					= 0x0001,
	tmsct_Addr						= 0x0002,
	tmsct_ParamConfiguration		= 0x0003,
}TlvModuleSensorConfigurationTag;

typedef enum _TlvScModuleMappingTag
{
	tscmmt_SensorCount				= 0x0001,
	tscmmt_SensorConfiguration		= 0x0002,
}TlvScModuleMappingTag;

typedef enum _TlvScModuleSettingsTag
{
	tscmst_LogSettings				= 0x0001,
	tscmst_ConnSettings				= 0x0002,
	tscmst_TimeoutSettings			= 0x0003,
	tscmst_ModuleMapping			= 0x0004,
}TlvScModuleSettingsTag;

typedef enum _TlvScSystemTag
{
	tscst_Id						= 0x0001,
	tscst_Name						= 0x0002,
	tscst_Port						= 0x0003,
	tscst_Enable					= 0x0004,
	tscst_CommandTimeout			= 0x0005,
	tscst_Interval					= 0x0006,
	tscst_Module					= 0x0007,
	tscst_LogDir					= 0x0008,
	tscst_LogEnable					= 0x0009,
	tscst_LogTrace					= 0x000A,
	tscst_LogFileSize				= 0x000B,
	tscst_LogSaveDays				= 0x000C,
	tscst_ModuleSettings			= 0x000D,
}TlvScSystemTag;

//Configuration

typedef enum _TlvSCConfigurationTag
{
	tscct_SensorCount		 		= 0x0001,
	tscct_SensorConfiguration		= 0x0002,
}TlvSCConfigurationTag;

typedef enum _TlvSCSensorConfigurationTag
{
	tscsct_Number					= 0x0001,
	tscsct_Addr						= 0x0002,
	tscsct_Param					= 0x0003,
}TlvSCSensorConfigurationTag;

typedef enum _TlvSCSensorParamConfigurationTag
{
	tscspct_Number					= 0x0001,
	tscspct_Type					= 0x0002,
}TlvSCSensorParamConfigurationTag;

//Data

typedef enum _TlvSCDataTag
{
	tscdt_Count	 				= 0x0001,
	tscdt_SensorData			= 0x0002,
}TlvSCDataTag;

typedef enum _TlvSCSensorDataTag
{
	tscsdt_Number					= 0x0001,
	tscsdt_Param					= 0x0002,
	tscsdt_Online					= 0x0003,
}TlvSCSensorDataTag;

typedef enum _TlvSCSensorParamDataTag
{
	tscspdt_Number					= 0x0001,
	tscspdt_Type					= 0x0002,
	tscspdt_Data					= 0x0003,
}TlvSCSensorParamDataTag;

//------------------------------------------- common -------------------------------------------------------

typedef enum _TlvSystemCommonTag
{
	tsct_ProfilesEnable				= 0x0001,
	tsct_ServerName					= 0x0002,
	tsct_ConfigurationPort			= 0x0003,
	tsct_LogDir						= 0x0004,
	tsct_LogEnable					= 0x0005,
	tsct_LogTrace					= 0x0006,
	tsct_LogFileSize				= 0x0007,
	tsct_LogSaveDays				= 0x0008,
	tsct_ConnLogDir					= 0x0009,
	tsct_ConnLogEnable				= 0x000A,
	tsct_ConnLogTrace				= 0x000B,
	tsct_ConnLogFrames				= 0x000C,
	tsct_ConnLogParsing				= 0x000D,
	tsct_ConnLogFileSize			= 0x000E,
	tsct_ConnLogSaveDays			= 0x000F,
}TlvSystemCommonTag;

//------------------------------------------- profiles -------------------------------------------------------

typedef enum _TlvSystemProfileConfTag
{
	tspct_Id						= 0x0001,
	tspct_Name						= 0x0002,
	tspct_Enable					= 0x0003,
	tspct_Guid						= 0x0004,
	tspct_AccessLevel				= 0x0005,
}TlvSystemProfileConfTag;

guint16 calc_crc(guchar* buffer, guint16 start_pos, guint16 length);
guint16 calc_crc_next(guchar byte, guint16 crc);


guint16 calc_new_crc(guchar* buffer, guint16 start_pos, guint16 length);
guint16 calc_new_crc_next(guchar byte, guint16 crc);

gfloat tlv_bin_to_float(guchar* buffer);
void tlv_float_to_bin(gfloat value, guchar* buffer);
gint8 tlv_bool_to_byte(gboolean value);

guint32 tlv_calc_unit_size(TlvUnit* unit);
void tlv_copy_unit(TlvUnit* destination, TlvUnit* source);
guint32 tlv_calc_list_count(TlvUnit* first_unit);
TlvUnit* tlv_find_last_unit(TlvUnit* first_unit);
void tlv_add_unit(TlvUnit** first_unit, TlvUnit* unit);
TlvUnit* tlv_create_unit(guint16 tag, guchar* buffer, guint32 start_pos, guint32 length);
void tlv_delete_units(TlvUnit** first_unit);
ExchangeError tlv_parse_frame(guchar* buffer, guint16 buffer_length, TlvUnit** destination );

guint8 tlv_calc_len(guint32 length);
guchar* tlv_serialize_unit(TlvUnit* unit, guint32* result_length);
guint32 tlv_calc_unit_size(TlvUnit* unit);
guint32 tlv_serialize_units_size(TlvUnit* first_unit);
void tlv_add_serialize_unit(TlvUnit* unit, guchar* buffer, guint32* current_pos );
guchar* tlv_serialize_units(TlvUnit* first_unit, guint32* size);
guchar* tlv_create_transport_frame(TlvUnit* first_unit, guint32* size);

gchar* return_message_type_name(MessageType type);

void add_tlv_unit_32(TlvUnit** units, guint16 tag, guint32 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);
void add_tlv_unit_8(TlvUnit** units, guint16 tag, guint8 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);
void add_tlv_unit_str(TlvUnit** units, guint16 tag, gchar* str, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);
void add_tlv_unit_float(TlvUnit** units, guint16 tag, gfloat value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);
void add_tlv_unit_16(TlvUnit** units, guint16 tag, guint16 value, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);
void add_tlv_unit_bytes(TlvUnit** units, guint16 tag, guint8* buffer, guint8 lenght, LogParams* log_params, gboolean log_trace, gboolean active,  const gchar* format, ...);


void tlv_test();

#endif /* TLV_H_ */
