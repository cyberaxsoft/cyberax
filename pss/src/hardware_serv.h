#ifndef HARDWARE_SERV_H_
#define HARDWARE_SERV_H_

#define SERVER_REPLY_TIMEOUT		1000

typedef enum _HwServerExchangeState
{
	hses_GetDcConf				= 0,
	hses_GetTgsConf				= 1,
	hses_GetPpConf				= 2,
	hses_GetScConf				= 3,
	hses_Complete				= 4,
}HwServerExchangeState;



void init_hs_mutex();
void lock_hs_mutex();
void unlock_hs_mutex();

gpointer serv_conf_thread_func(gpointer data);


#endif
