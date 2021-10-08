#ifndef LOGGER_H_
#define LOGGER_H_

#define SYSTEM_LOG_PREFIX			"system"
#define SYSTEM_CONNECT_LOG_PREFIX	"system_connect"

#define DEF_LOG_FILE_SIZE			10    //10 Mb
#define DEF_SAVE_DAYS				10
#define BYTE_IN_MB					1048576

typedef enum _LoggingLevel
{
	ll_System				= 0x00,
	ll_Critical				= 0x01,
	ll_Error				= 0x02,
	ll_Warning				= 0x03,
	ll_Information			= 0x04,
	ll_Debug				= 0x05,
}LoggingLevel;


typedef struct _LogOptions
{
	gboolean trace;
	gboolean system;
	gboolean requests;
	gboolean frames;
	gboolean parsing;

}LogOptions;

typedef struct _LogParams
{
	FILE* log;
	GMutex mutex;
	gboolean enable;
	gchar* prefix;
	gchar* path;
	guint32 file_size;
	guint32 save_days;
}LogParams;

const gchar* logging_level_to_str(LoggingLevel value);

gboolean init_log_dir(LogParams* log_params);
void create_log(LogParams* log_params);
void close_log(LogParams* log_params);
void add_log(LogParams* log_params, gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ...);

void delete_old_logs(LogParams* log_params);

void create_log_params(LogParams* params, gboolean active, gchar* path, gchar* prefix, guint32 file_size, guint32 save_days);
void destroy_log_params(LogParams* params);

#endif /* LOGGER_H_ */
