#ifndef IDCSENSOR_H_
#define IDCSENSOR_H_

#define BUFFER_SIZE					128

#define CTRL_SOH					0x01
#define CTRL_ETX					0x1D
#define IDC_REFRESH_COMM			0x02

#define SENSOR_ID_OFFSET			1
#define PORT_OFFSET					2
#define SENSOR_ADDR_OFFSET			3
#define TANK_OFFSET					4
#define TYPE_OFFSET					5
#define OFFLINE_OFFSET				6
#define PARAM_DATA_OFFSET			7

#define PARAM_ID_OFFSET				0
#define PARAM_VALUE_OFFSET			1
#define PARAM_CODE_OFFSET			5
#define PARAM_STATUS_OFFSET			6

#define PARAM_DATA_LENGTH			7

typedef enum _IdcSensorFrameStage
{
	isfs_WaitSOH					= 0,
	isfs_WaitLength					= 1,
	isfs_ReadData					= 2,
}IdcSensorFrameStage;

typedef struct _Sensor
{
	guint32							num;
	guint8							addr;

	SensorParams					params;

	gboolean						online;

}Sensor;

ssize_t read_func(guint8* buffer);
gboolean send_func(guint8* buffer, guint16 size, LogOptions log_options);

void safe_set_status(DriverStatus new_status);

#endif /* IDCSENSOR_H_ */
