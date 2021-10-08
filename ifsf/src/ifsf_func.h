#ifndef IFSF_FUNC_H_
#define IFSF_FUNC_H_

#define LENGTH_POS					1

#define IFSF_START_FLAG				0x12

#define IFSF_HEARTBEAT_FLAGS		0x00
#define IFSF_REQUEST_FLAGS			0x01
#define IFSF_RESPONCE_FLAGS			0x00


#define IFSF_HEARTBEAT_FORMAT		0x31
#define IFSF_DATA_FORMAT			0x09


#define IFSF_MASTER_SUBNET			0x02
#define IFSF_MASTER_NODE			0x01
#define IFSF_SYSTEM_SUBNET			0x00
#define IFSF_SYSTEM_NODE			0x01
#define IFSF_GROUP					0x01
#define IFSF_DISP_SUBNET			0x01


#define IFSF_MC_0					0x00
#define IFSF_MC_2					0x02
#define IFSF_BL						0x80

#define HEALTHBIT_TIMEOUT			10

typedef enum _IFSFCommServiceDbId
{
	ifsfcsdbid_CommProtoVer			= 0x01,
	ifsfcsdbid_LocalNodeAddr		= 0x02,
	ifsfcsdbid_RecipientAddrTable	= 0x03,
	ifsfcsdbid_HeartbeatInterval	= 0x04,
	ifsfcsdbid_MaxBlockLength		= 0x05,
	ifsfcsdbid_AddRecipientAddr		= 0x0b,
	ifsfcsdbid_RemoveRecipientAddr	= 0x0c,
}IFSFCommServiceDbId;

typedef enum _IFSFCalculatorId
{
	ifsfcid_NbProducts				= 0x02,
	ifsfcid_NbFuellingModes			= 0x03,
	ifsfcid_NbMeters				= 0x04,
	ifsfcid_NbFp					= 0x05,
	ifsfcid_CountryCode				= 0x06,
	ifsfcid_BlendTolerance			= 0x07,
	ifsfcid_DriveOfLightsMode		= 0x08,
	ifsfcid_OPTLightMode			= 0x09,
	ifsfcid_ClearDisplayMode		= 0x0A,
	ifsfcid_AuthStateMode			= 0x0B,
	ifsfcid_StandAloneAuth			= 0x0C,
	ifsfcid_MaxAuthTime				= 0x0D,
	ifsfcid_MaxTimeWOProg			= 0x15,
	ifsfcid_MinFuellingVol			= 0x16,
	ifsfcid_MinDisplayVol			= 0x17,
	ifsfcid_MinGuardTime			= 0x18,
	ifsfcid_PulserErrTolerance		= 0x1A,
	ifsfcid_TimeDisplayProductName	= 0x1C,
	ifsfcid_DigitsVolLayot			= 0x28,
	ifsfcid_DigitsAmountLayot		= 0x29,
	ifsfcid_DigitsUnitPrice			= 0x2A,
	ifsfcid_UnitPriceMultFact		= 0x2B,
	ifsfcid_AmountRoundingFact		= 0x2C,
	ifsfcid_PresetRoundingAmount	= 0x2D,
	ifsfcid_PriceSetNb				= 0x2E,
	ifsfcid_ManufacturerId			= 0x32,
	ifsfcid_DispencerModel			= 0x33,
	ifsfcid_CalculatorType			= 0x34,
	ifsfcid_CalculatorSerialNo		= 0x35,
	ifsfcid_ApplSoftwareVer			= 0x36,
	ifsfcid_WMSoftwareVer			= 0x37,
	ifsfcid_WMSoftwareDate			= 0x38,
	ifsfcid_WMSecurityType			= 0x39,
	ifsfcid_ProtocolVer				= 0x3A,
	ifsfcid_SWChangeDate			= 0x3B,
	ifsfcid_SWChangePersonnalNb		= 0x3C,
	ifsfcid_SWCheckSum				= 0x3D,
	ifsfcid_CalcIllumination		= 0x46,
	ifsfcid_LCDBacklightSwitch		= 0x47,
	ifsfcid_DisplayIntensity		= 0x48,
	ifsfcid_WMPolynomial			= 0x50,
	ifsfcid_WMSeed					= 0x51,

}IFSFCalculatorId;

typedef enum _IFSFProductId
{
	ifsfpid_ProdNb					= 0x02,
	ifsfpid_ProdDescription			= 0x03,
	ifsfpid_VapRecoverConst			= 0x0A,

}IFSFProductId;

typedef enum _IFSFProductPerFuellingModeId
{
	ifsfppfmid_FuellingModeName 	= 0x01,
	ifsfppfmid_ProdPrice		 	= 0x02,
	ifsfppfmid_MaxVol			 	= 0x03,
	ifsfppfmid_MaxFillTime		 	= 0x04,
	ifsfppfmid_MaxAuthTime		 	= 0x05,
	ifsfppfmid_UserMaxVolume	 	= 0x06,

}IFSFProductPerFuellingModeId;

typedef enum _IFSFFuellingPointId
{
	ifsffpid_NbTranBufferNotPaid	= 0x02,
	ifsffpid_NbLogicalNozzle		= 0x04,
	ifsffpid_DefaultFuellingMode    = 0x07,
	ifsffpid_DriveOffLightSwitch	= 0x0a,
	ifsffpid_FpState				= 0x14,
	ifsffpid_LogNozState			= 0x15,
	ifsffpid_AssignContrId			= 0x16,
	ifsffpid_ZeroTRMode				= 0x18,
	ifsffpid_LogNozMask				= 0x19,
	ifsffpid_RemoteAmountPreset		= 0x1B,
	ifsffpid_RemoteVolumePreset		= 0x1C,
	ifsffpid_CurrentAmount			= 0x22,
	ifsffpid_CurrentVolume			= 0x23,
	ifsffpid_CurrentUnitPrice		= 0x24,
	ifsffpid_OpenFp					= 0x3C,
	ifsffpid_ReleaseFp				= 0x3E,
	ifsffpid_SuspendFp				= 0x40,
	ifsffpid_ResumeFp				= 0x41,

}IFSFFuellingPointId;




typedef enum _IFSF_FuellingTransactionId
{
	ifsfftid_TrSeqNb				= 0x01,
	ifsfftid_TrContrId				= 0x02,
	ifsfftid_TrReleaseToken			= 0x03,
	ifsfftid_TrFuellingMode			= 0x04,
	ifsfftid_TrAmount				= 0x05,
	ifsfftid_TrVolume				= 0x06,
	ifsfftid_TrUnitPrice			= 0x07,
	ifsfftid_TrLogNoz				= 0x08,
	ifsfftid_TrPriceSetNb			= 0x09,
	ifsfftid_TrProdNb				= 0x0A,
	ifsfftid_TrProdDescription		= 0x0B,
	ifsfftid_TrErrorCode			= 0x0C,
	ifsfftid_TrAverageTemp			= 0x0D,
	ifsfftid_TrSecurityChksum		= 0x0E,
	ifsfftid_M1SubVolume			= 0x0F,
	ifsfftid_M2SubVolume			= 0x10,
	ifsfftid_TrTaxAmount			= 0x11,
	ifsfftid_TrBuffContrId			= 0x14,
	ifsfftid_TransState				= 0x15,
	ifsfftid_ClearTransaction		= 0x1e,
	ifsfftid_LockTransaction		= 0x1f,
	ifsfftid_UnlockTransaction		= 0x20,

}IFSF_FuellingTransactionId;

typedef enum _IFSF_LogicalNozzleId
{
	ifsflnid_PRId					= 0x01,
	ifsflnid_HoseExpansionVol		= 0x03,
	ifsflnid_SlowFlowValveActiv		= 0x04,
	ifsflnid_PhisicalNozzleId		= 0x05,
	ifsflnid_Meter1Id				= 0x07,
	ifsflnid_Meter1BlendRatio		= 0x08,
	ifsflnid_Meter2Id				= 0x09,
	ifsflnid_LogicalNozzleType		= 0x0A,
	ifsflnid_PresetValveActivation	= 0x0B,
	ifsflnid_LogNozVolTotal			= 0x14,
	ifsflnid_LogNozAmountTotal		= 0x15,
	ifsflnid_NoTrTotal				= 0x16,
	ifsflnid_LogNozSAVolTotal		= 0x1E,
	ifsflnid_LogNozSAAmountTotal	= 0x1F,
	ifsflnid_NoTrSATotal			= 0x20,
}IFSF_LogicalNozzleId;

guint32 packed_bcd_to_bin(guint8* buff, guint8 size);

guint64 get_date_time(void);
void get_error_description(guint8 error_code, guchar* buffer);

void bin_to_packet_bcd(guint32 val, guint8* buff, guint8 size);

const gchar* ds_to_str(DispencerState state);

const gchar* ifsf_message_type_to_str(IFSFMessageType message_type);
const gchar* ifsf_database_id_to_str(IFSFDatabaseId id);
const gchar* ifsf_message_status_to_str(IFSFMessageAcknowledgeStatus message_status);
const gchar* ifsf_data_status_to_str(IFSFDataAcknowledgeStatus message_status);
const gchar* ifsf_fp_state_to_str(IFSF_FPState fp_state);
const gchar* ifsf_tr_state_to_str(IFSF_TRState tr_state);

const gchar* ifsf_data_id_to_str(IFSFDbAddress database_addr,  guint8 id);
IFSFDataType ifsf_return_data_type(IFSFDbAddress database_addr,  guint8 id);

guint8 prepare_healthbit_message(guint8* buffer);
guint8 prepare_addr_message(guint8* buffer, guint8 disp_index);
guint8 prepare_timeout_message(guint8* buffer, guint8 disp_index);
guint8 prepare_config_message(guint8* buffer, guint8 disp_index);
guint8 prepare_set_id_product_message(guint8* buffer, guint8 node_index, guint8 product_index);
guint8 prepare_nozzle_count_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_nozzle_conf_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint8 nozzle_index);
guint8 prepare_nozzle_total_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_set_price_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 price);
guint8 prepare_calculator_conf_message(guint8* buffer, guint8 node_index);
guint8 prepare_set_product_param_message(guint8* buffer, guint8 node_index, guint8 product_index, guint8 fuelling_mode_index);
guint8 prepare_fp_param_conf_message(guint8* buffer, guint8 node_index, guint8 disp_index);

guint8 prepare_get_pump_state_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_allowed_nozzles_message(guint8* buffer, guint8 node_index, guint8 disp_index, gboolean allowed);

guint8 prepare_zero_func_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_open_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_authorize_nozzle_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_preset_volume_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 volume);
guint8 prepare_preset_amount_message(guint8* buffer, guint8 node_index, guint8 disp_index, guint32 amount);
guint8 prepare_start_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_suspend_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_resume_fp_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_get_filling_data_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_get_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_clear_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_lock_transaction_message(guint8* buffer, guint8 node_index, guint8 disp_index);
guint8 prepare_ack_message(guint8* buffer, guint8 disp_num, guint8 transport_token);
guint8 prepare_product_price_message(guint8* buffer, guint8 node_index, guint8 disp_index);

guint8 prepare_communication_service_reply_message(guint8* buffer, guint8* data, guint8 data_length, guint8 src_subnet,guint8 src_node);

guint8 prepare_light_switch_message(guint8* buffer, guint8 disp_num, guint8 transport_token, guint8 ifsf_token, gboolean light_switch);


#endif /* IFSF_FUNC_H_ */
