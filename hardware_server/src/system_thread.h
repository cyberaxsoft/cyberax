#ifndef SYSTEM_THREAD_H_
#define SYSTEM_THREAD_H_

void system_init();
gpointer system_main_socket_thread_func(gpointer data);
void close_system_sock();
void set_system_sock_status(SocketStatus new_value);

#endif /* SYSTEM_THREAD_H_ */
