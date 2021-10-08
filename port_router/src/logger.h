#ifndef LOGGER_H_
#define LOGGER_H_

#define LOG_PREFIX					"PortRouter"
#define LOG_FILENAME				"portrouter"

#define DEF_LOG_FILE_SIZE			10    //10 Mb
#define DEF_SAVE_DAYS				10
#define BYTE_IN_MB					1048576

typedef struct _LogOptions
{
	gboolean trace;
	gboolean system;
	gboolean requests;
	gboolean frames;
	gboolean parsing;

	guint32 file_size;
	guint32 save_days;

}LogOptions;

gboolean init_log_dir(gchar* dir, const gchar* prefix);
FILE* create_log(gchar* path, const gchar* prefix, GMutex* log_mutex, const gchar* log_name);
gboolean init_log(gchar* dir, guint32 file_size, guint32 save_days);
void close_log();
void add_log(gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ...);

void delete_old_logs(gchar* path);

#endif /* LOGGER_H_ */
