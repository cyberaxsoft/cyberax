#ifndef DRIVER_H_
#define DRIVER_H_

#define DRIVER_DESCRIPTION_LENGTH	128
#define	SENSOR_PARAM_VALUE_SIZE		8
#define	MAX_SENSOR_PARAM_COUNT		16
#define MAX_SENSOR_COUNT			255

typedef enum _DriverError
{
	de_NoError						= 0x00,
	de_WrongCommand					= 0x01,
	de_ErrorConnection 				= 0x02,
	de_ErrorConfiguration			= 0x03,
	de_ErrorLogging					= 0x04,
	de_FaultSensorIndex				= 0x05,
	de_FaultParamIndex  			= 0x06,
	de_Undefined					= 0x07,
	de_OutOfRange					= 0x08,
	de_AlreadyInit					= 0x09,
	de_NotInitializeDriver			= 0x0A,
	de_FaultSensorNum				= 0x0B,
	de_FaultParamNum				= 0x0C,
}DriverError;

typedef enum _DriverStatus
{
	ds_NoError						= 0x00,
	ds_ErrorConnection				= 0x01,
	ds_ErrorConfiguration			= 0x02,
	ds_ErrorLogging					= 0x03,
	ds_UndefinedError				= 0x04,
	ds_NotInitializeDriver			= 0x05,
}DriverStatus;

typedef enum _ConnectionType
{
	ct_Uart							= 0x00,
	ct_TcpIp						= 0x01,
}ConnectionType;

typedef enum _SensorParamType
{
	spt_Float						= 0x00,
}SensorParamType;

typedef struct _SensorParam
{
	guint8							num;
	SensorParamType					type;
	guint8							value[SENSOR_PARAM_VALUE_SIZE];
}SensorParam;

typedef struct _SensorParams
{
	SensorParam						params[MAX_SENSOR_PARAM_COUNT];
	guint8							param_count;
}SensorParams;

#endif /* DRIVER_H_ */
