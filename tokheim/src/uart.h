#ifndef UART_H_
#define UART_H_

gint open_uart(gchar* port_name, gboolean trace_log, gboolean system_log);
void set_settings_uart(gint port_descriptor, guint32 uart_baudrate, guint8 uart_byte_size, gchar* uart_parity,  guint8 uart_stop_bits, guint32 timeout, gboolean trace_log, gboolean system_log);
void close_uart(gint port_descriptor);


#endif /* UART_H_ */
