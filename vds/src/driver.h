#ifndef DRIVER_H_
#define DRIVER_H_

#define DRIVER_DESCRIPTION_LENGTH	128

typedef enum _DriverError
{
	de_NoError						= 0x00,
	de_WrongCommand					= 0x01,
	de_ErrorConnection 				= 0x02,
	de_ErrorConfiguration 			= 0x03,
	de_ErrorLogging					= 0x04,
	de_Undefined					= 0x05,
	de_OutOfRange					= 0x06,
	de_AlreadyInit					= 0x07,
	de_FaultExtFuncIndex			= 0x08,
	de_NotInitializeDriver 			= 0x09,
}DriverError;

typedef enum _DriverStatus
{
	drs_NoError						= 0x00,
	drs_ErrorConnection				= 0x01,
	drs_ErrorConfiguration			= 0x02,
	drs_ErrorLogging				= 0x03,
	drs_UndefinedError				= 0x04,
	drs_NotInitializeDriver			= 0x05,
}DriverStatus;

typedef enum _PricePoleState
{
	pps_NotInitialize				= 0x00,
	pps_Online						= 0x01,
	pps_Offline						= 0x02,
}PricePoleState;

typedef enum _ConnectionType
{
	ct_Uart							= 0x00,
	ct_TcpIp						= 0x01,
}ConnectionType;

#endif /* DRIVER_H_ */
