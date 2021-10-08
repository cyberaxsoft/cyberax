#ifndef READ_THREAD_H_
#define READ_THREAD_H_

void init_read_thread_mutex();
void init_rx_stage_mutex();

void safe_set_read_thread_terminating(gboolean new_value);
gboolean safe_get_read_thread_terminated();
void safe_set_read_thread_terminated(gboolean new_value);

gpointer read_thread_func(gpointer data);


#endif /* READ_THREAD_H_ */
