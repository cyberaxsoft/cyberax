#ifndef DRIVER_H_
#define DRIVER_H_

#define DRIVER_DESCRIPTION_LENGTH	128

typedef enum _DriverError
{
	de_NoError				= 0x00,
	de_WrongCommand			= 0x01,
	de_ErrorConnection 		= 0x02,
	de_ErrorConfiguration 	= 0x03,
	de_ErrorLogging			= 0x04,
	de_FaultDispencerIndex 	= 0x05,
	de_FaultNozzleIndex  	= 0x06,
	de_Undefined			= 0x07,
	de_OutOfRange			= 0x08,
	de_AlreadyInit			= 0x09,
	de_FaultExtFuncIndex	= 0x0A,
	de_NotInitializeDriver 	= 0x0B,
	de_FaultDispencerNum	= 0x0C,
	de_FaultNozzleNum		= 0x0D,
	de_FaultDispencerState	= 0x0E,
	de_FaultNozzleState		= 0x0F,
	de_DispencerBusy		= 0x10,

}DriverError;

typedef enum _DriverStatus
{
	drs_NoError				= 0x00,
	drs_ErrorConnection		= 0x01,
	drs_ErrorConfiguration	= 0x02,
	drs_ErrorLogging		= 0x03,
	drs_UndefinedError		= 0x04,
	drs_NotInitializeDriver	= 0x05,
}DriverStatus;

typedef enum _ConnectionType
{
	ct_Uart					= 0x00,
	ct_TcpIp				= 0x01,
}ConnectionType;


#endif /* DRIVER_H_ */
