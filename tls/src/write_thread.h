#ifndef WRITE_THREAD_H_
#define WRITE_THREAD_H_

void init_write_thread_mutex();

gboolean safe_get_write_thread_terminating();
gboolean safe_get_write_thread_terminated();
void safe_set_write_thread_terminating(gboolean new_value);
void safe_set_write_thread_terminated(gboolean new_value);

gpointer write_thread_func(gpointer data);


#endif /* WRITE_THREAD_H_ */
