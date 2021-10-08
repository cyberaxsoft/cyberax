#ifndef PSS_COMMAND_LINE_H_
#define PSS_COMMAND_LINE_H_

gboolean parse_command_line(gint argc, gchar *argv[], gchar** log_dir,  gchar** config_filename, gchar** guid, gchar** ip_address, gboolean* log_enable, gboolean* log_trace,
							 guint32* ip_port, guint32* sensor_num, guint32* sensor_param_num, guint32* file_size, guint32* save_days);


#endif /* PSS_COMMAND_LINE_H_ */
