#include <glib.h>
#include <hidapi.h>

#include "logger.h"
#include "port_router.h"


PortRouterConfig configuration = {0x00};
GMutex	conf_mutex;

void init_conf_mutex()
{
	g_mutex_init(&conf_mutex);
}

void safe_get_serial_number(guint8 device_index, gchar** result)
{
	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		*result = g_strdup(configuration.devices[device_index].serial_number);
	}

	g_mutex_unlock(&conf_mutex);
}

void safe_get_log_options(LogOptions* options)
{
	g_mutex_lock(&conf_mutex);

	*options = configuration.log_options;

	g_mutex_unlock(&conf_mutex);
}

gboolean safe_get_is_connected(guint8 device_index)
{
	gboolean result = False;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].is_connected;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_is_connected(guint8 device_index, gboolean value)
{
	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		configuration.devices[device_index].is_connected = value;
	}

	g_mutex_unlock(&conf_mutex);
}

gboolean safe_get_stoping(guint8 device_index)
{
	gboolean result = False;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].stoping;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_stoping(guint8 device_index, gboolean value)
{
	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		configuration.devices[device_index].stoping = value;
	}

	g_mutex_unlock(&conf_mutex);
}

gboolean safe_get_usb_message_sended(guint8 device_index)
{
	gboolean result = False;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].usb_message_sended;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_usb_message_sended(guint8 device_index, gboolean value)
{
	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		configuration.devices[device_index].usb_message_sended = value;
	}

	g_mutex_unlock(&conf_mutex);
}

UsbDeviceStage safe_get_device_stage(guint8 device_index)
{
	UsbDeviceStage result = uds_Undefined;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].stage;
	}

	g_mutex_unlock(&conf_mutex);

	return result;

}

void safe_set_device_stage(guint8 device_index, UsbDeviceStage value)
{
	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		configuration.devices[device_index].stage = value;
	}

	g_mutex_unlock(&conf_mutex);
}

void safe_close_hid(guint8 device_index)
{
	g_mutex_lock(&configuration.devices[device_index].hid_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		if (configuration.devices[device_index].handle != NULL)
		{
			hid_close(configuration.devices[device_index].handle);
			configuration.devices[device_index].handle = NULL;
		}
	}

	g_mutex_unlock(&configuration.devices[device_index].hid_mutex);
}

guint16 safe_get_vendor_id(guint8 device_index)
{
	guint16 result = 0;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].vendor_id;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint16 safe_get_product_id(guint8 device_index)
{
	guint16 result = 0;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].product_id;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint8 safe_get_idc_id(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].idc_id;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

guint8 safe_get_port_count(guint8 device_index)
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		result = configuration.devices[device_index].port_count;
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

gint8 safe_get_port_index_by_num(guint8 device_index, guint8 port_num)
{
	guint8 result = -1;

	g_mutex_lock(&conf_mutex);

	if (device_index < MAX_DEVICE_COUNT)
	{
		if (configuration.devices[device_index].port_count > 0)
		{
			for (guint8 i = 0; i < configuration.devices[device_index].port_count; i++ )
			{
				if (configuration.devices[device_index].ports[i].num == port_num)
				{
					result = i;
					break;
				}
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

void safe_set_port_sock(guint8 device_index, guint8 port_index, gint32 new_value)
{
	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	port->sock = new_value;

	g_mutex_unlock(&port->mutex);
}

gint32 safe_get_port_sock(guint8 device_index, guint8 port_index)
{
	gint32 result = -1;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->sock;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint32 safe_get_baudrate(guint8 device_index, guint8 port_index)
{
	guint32 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->baudrate;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_byte_size(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->byte_size;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_stop_bits(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->stop_bits;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_parity(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->parity;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_duplex(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->duplex;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_debug(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->debug;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_passtrough(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->passtrough;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_invert_rx(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->invert_rx;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_invert_tx(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->invert_tx;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_req_timeout(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->req_timeout;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_recv_timeout(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->req_timeout;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_application(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->application;

	g_mutex_unlock(&port->mutex);

	return result;
}

void safe_set_port_stage(guint8 device_index, guint8 port_index, UsbDevicePortStage new_value)
{
	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	port->stage = new_value;

	g_mutex_unlock(&port->mutex);

}

UsbDevicePortStage safe_get_port_stage(guint8 device_index, guint8 port_index)
{
	UsbDevicePortStage result = udps_Undefined;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->stage;

	g_mutex_unlock(&port->mutex);

	return result;
}

void safe_set_client_sock(guint8 device_index, guint8 port_index, guint8 client_index, gint32 new_value)
{
	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	if (client_index < MAX_CLIENT_COUNT)
	{
		port->client_socks[client_index] = new_value;
	}

	g_mutex_unlock(&port->mutex);

}

gint32 safe_get_client_sock(guint8 device_index, guint8 port_index, guint8 client_index)
{
	gint32 result = -1;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	if (client_index < MAX_CLIENT_COUNT)
	{
		result = port->client_socks[client_index];
	}
	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_port_num(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->num;

	g_mutex_unlock(&port->mutex);

	return result;
}

guint8 safe_get_interface_id(guint8 device_index, guint8 port_index)
{
	guint8 result = 0;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	result = port->interface_id;

	g_mutex_unlock(&port->mutex);

	return result;
}

gint8 find_next_client_index(guint8 device_index, guint8 port_index, guint8 max_client_count)
{
	gint result = -1;

	UsbDevicePort* port = &configuration.devices[device_index].ports[port_index];

	g_mutex_lock(&port->mutex);

	for (guint8 i = 0; i < max_client_count; i++)
	{
		if (port->client_socks[i] == cs_Free)
		{
			port->client_socks[i] = cs_ConnectReady;
			result = i;
			break;
		}
	}

	g_mutex_unlock(&port->mutex);

	return result;
}

gint8 find_next_mult_client_index(guint8 mult_device_index, guint8 max_client_count)
{
	gint result = -1;

	MultDevice* device = &configuration.mult_devices[mult_device_index];

	g_mutex_lock(&device->mutex);

	for (guint8 i = 0; i < max_client_count; i++)
	{
		if (device->client_socks[i] == cs_Free)
		{
			device->client_socks[i] = cs_ConnectReady;
			result = i;
			break;
		}
	}

	g_mutex_unlock(&device->mutex);

	return result;
}

UsbDevicePort* get_port(guint8 device_index, guint8 port_index)
{
	UsbDevicePort* result = NULL;

	result = &configuration.devices[device_index].ports[port_index];

	return result;
}

UsbDevice* get_device(guint8 device_index)
{
	UsbDevice* result = NULL;

	result = &configuration.devices[device_index];

	return result;
}

MultDevice* get_mult_device(guint8 device_index)
{
	MultDevice* result = NULL;

	result = &configuration.mult_devices[device_index];

	return result;
}

GMutex* get_device_hid_mutex(guint8 device_index)
{
	GMutex* result = NULL;

	result = &configuration.devices[device_index].hid_mutex;

	return result;
}

hid_device* get_hid_device(guint8 device_index)
{
	hid_device* result = NULL;

	result = configuration.devices[device_index].handle;

	return result;
}

PortRouterConfig* get_configuration()
{
	PortRouterConfig* result = NULL;

	result = &configuration;

	return result;
}

void safe_set_mult_client_sock(guint8 device_index, guint8 client_index, gint32 new_value)
{
	MultDevice* device = &configuration.mult_devices[device_index];

	g_mutex_lock(&device->mutex);

	if (client_index < MAX_CLIENT_COUNT)
	{
		device->client_socks[client_index] = new_value;
	}

	g_mutex_unlock(&device->mutex);

}

void set_device_indexes()
{
	if (configuration.mult_device_count > 0)
	{
		for (guint8 i = 0; i < configuration.mult_device_count; i++)
		{
			MultDeviceUnits* mult_device_units = &configuration.mult_devices[i].units;

			if (mult_device_units->unit_count > 0)
			{
				for (guint8 j = 0; j < mult_device_units->unit_count; j++)
				{
					MultDeviceUnit* unit = &mult_device_units->units[j];

					if (unit->serial_number != NULL && strlen(unit->serial_number) > 0 && configuration.device_count > 0)
					{
						for (guint8 k = 0; k < configuration.device_count; k++)
						{
							if (configuration.devices[k].serial_number != NULL && strlen(configuration.devices[k].serial_number) > 0 )
							{
								if (strcmp(unit->serial_number, configuration.devices[k].serial_number) == 0)
								{
									unit->device_index = k;
								}
							}
						}
					}
				}
			}
		}
	}
}

gboolean check_device_port_is_mult(gchar* serial_number, guint8 port_num)
{
	gboolean result = False;

	if (configuration.mult_device_count > 0)
	{
		for (guint8 i = 0; i < configuration.mult_device_count; i++)
		{
			MultDeviceUnits* mult_device_units = &configuration.mult_devices[i].units;

			if (mult_device_units->unit_count > 0)
			{
				for (guint8 j = 0; j < mult_device_units->unit_count; j++)
				{
					MultDeviceUnit* unit = &mult_device_units->units[j];

					if (unit->serial_number != NULL && strlen(unit->serial_number) > 0 && serial_number!=NULL && strlen(serial_number) > 0)
					{
						if (strcmp(unit->serial_number, serial_number) == 0 && port_num == unit->num)
						{
							return True;
						}
					}
				}
			}
		}
	}

	return result;
}

guint8 safe_get_mult_device_count()
{
	guint8 result = 0;

	g_mutex_lock(&conf_mutex);

	result = configuration.mult_device_count;

	g_mutex_unlock(&conf_mutex);

	return result;
}

gboolean device_present_in_mult_device(guint8 index_mult_device, gchar* serial_number, guint8 port_num)
{
	gboolean result = False;

	g_mutex_lock(&conf_mutex);

	if (index_mult_device < configuration.mult_device_count)
	{
		MultDeviceUnits* mult_device_units = &configuration.mult_devices[index_mult_device].units;

		if (mult_device_units->unit_count > 0)
		{
			for (guint8 i = 0; i < mult_device_units->unit_count; i++)
			{
				MultDeviceUnit* unit = &mult_device_units->units[i];

				if (unit->serial_number != NULL && strlen(unit->serial_number) > 0 && serial_number!=NULL && strlen(serial_number) > 0)
				{
					if (strcmp(unit->serial_number, serial_number) == 0 && port_num == unit->num)
					{
						return True;
					}
				}
			}
		}
	}

	g_mutex_unlock(&conf_mutex);

	return result;
}

