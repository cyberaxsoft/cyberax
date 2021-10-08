/*
 * port_router_data.h
 *
 *  Created on: 7 окт. 2021 г.
 *      Author: mkv
 */

#ifndef PORT_ROUTER_DATA_H_
#define PORT_ROUTER_DATA_H_

void init_conf_mutex();
void safe_get_serial_number(guint8 device_index, gchar** result);
void safe_get_log_options(LogOptions* options);

gboolean safe_get_is_connected(guint8 device_index);
void safe_set_is_connected(guint8 device_index, gboolean value);

gboolean safe_get_stoping(guint8 device_index);
void safe_set_stoping(guint8 device_index, gboolean value);

gboolean safe_get_usb_message_sended(guint8 device_index);
void safe_set_usb_message_sended(guint8 device_index, gboolean value);

UsbDeviceStage safe_get_device_stage(guint8 device_index);
void safe_set_device_stage(guint8 device_index, UsbDeviceStage value);

void safe_close_hid(guint8 device_index);
guint16 safe_get_vendor_id(guint8 device_index);
guint16 safe_get_product_id(guint8 device_index);
guint8 safe_get_idc_id(guint8 device_index);
guint8 safe_get_port_count(guint8 device_index);
gint8 safe_get_port_index_by_num(guint8 device_index, guint8 port_num);

void safe_set_port_sock(guint8 device_index, guint8 port_index, gint32 new_value);
gint32 safe_get_port_sock(guint8 device_index, guint8 port_index);

guint32 safe_get_baudrate(guint8 device_index, guint8 port_index);
guint8 safe_get_byte_size(guint8 device_index, guint8 port_index);
guint8 safe_get_stop_bits(guint8 device_index, guint8 port_index);
guint8 safe_get_parity(guint8 device_index, guint8 port_index);
guint8 safe_get_duplex(guint8 device_index, guint8 port_index);
guint8 safe_get_debug(guint8 device_index, guint8 port_index);
guint8 safe_get_passtrough(guint8 device_index, guint8 port_index);
guint8 safe_get_invert_rx(guint8 device_index, guint8 port_index);
guint8 safe_get_invert_tx(guint8 device_index, guint8 port_index);
guint8 safe_get_req_timeout(guint8 device_index, guint8 port_index);
guint8 safe_get_recv_timeout(guint8 device_index, guint8 port_index);
guint8 safe_get_application(guint8 device_index, guint8 port_index);

void safe_set_port_stage(guint8 device_index, guint8 port_index, UsbDevicePortStage new_value);
UsbDevicePortStage safe_get_port_stage(guint8 device_index, guint8 port_index);


void safe_set_client_sock(guint8 device_index, guint8 port_index, guint8 client_index, gint32 new_value);
gint32 safe_get_client_sock(guint8 device_index, guint8 port_index, guint8 client_index);

guint8 safe_get_port_num(guint8 device_index, guint8 port_index);
guint8 safe_get_interface_id(guint8 device_index, guint8 port_index);

gint8 find_next_client_index(guint8 device_index, guint8 port_index, guint8 max_client_count);
gint8 find_next_mult_client_index(guint8 mult_device_index, guint8 max_client_count);

UsbDevicePort* get_port(guint8 device_index, guint8 port_index);
UsbDevice* get_device(guint8 device_index);
MultDevice* get_mult_device(guint8 device_index);
GMutex* get_device_hid_mutex(guint8 device_index);
hid_device* get_hid_device(guint8 device_index);
PortRouterConfig* get_configuration();

void safe_set_mult_client_sock(guint8 device_index, guint8 client_index, gint32 new_value);

void set_device_indexes();
gboolean check_device_port_is_mult(gchar* serial_number, guint8 port_num);
guint8 safe_get_mult_device_count();
gboolean device_present_in_mult_device(guint8 index_mult_device, gchar* serial_number, guint8 port_num);

#endif /* PORT_ROUTER_DATA_H_ */
