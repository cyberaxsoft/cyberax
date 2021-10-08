#ifndef UART_H_
#define UART_H_

gint open_uart(gchar* port_name, gboolean trace_log, gboolean system_log);
void close_uart(gint port_descriptor);


#endif /* UART_H_ */
