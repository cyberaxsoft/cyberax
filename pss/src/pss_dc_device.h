#ifndef PSS_DC_DEVICE_H_
#define PSS_DC_DEVICE_H_

#define EXEC_COMMAND_TIMEOUT	5000

typedef enum _DcDeviceError
{
	dce_NoError					= 0x00,
	dce_WrongCommand			= 0x01,
	dce_ErrorConnection 		= 0x02,
	dce_ErrorConfiguration 		= 0x03,
	dce_ErrorLogging			= 0x04,
	dce_FaultDispencerIndex 	= 0x05,
	dce_FaultNozzleIndex  		= 0x06,
	dce_Undefined				= 0x07,
	dce_OutOfRange				= 0x08,
	dce_AlreadyInit				= 0x09,
	dce_FaultExtFuncIndex		= 0x0A,
	dce_NotInitializeDriver 	= 0x0B,
	dce_FaultDispencerNum		= 0x0C,
	dce_FaultNozzleNum			= 0x0D,
	dce_FaultDispencerState		= 0x0E,
	dce_FaultNozzleState		= 0x0F,
	dce_DispencerBusy			= 0x10,

}DcDeviceError;

void send_update_price_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, PricePacks* price_packs);
void send_volume_preset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, guint8 nozzle_num, guint32 price, guint32 volume);
void send_amount_preset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num, guint8 nozzle_num, guint32 price, guint32 amount);
void send_reset_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num);
void send_stop_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num);
void send_suspend_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num);
void send_resume_message(PSSClientThreadFuncParam* params, guint8 device_index, guint32 dispencer_num);

gpointer dc_device_func(gpointer data);

#endif /* PSS_DC_DEVICE_H_ */
