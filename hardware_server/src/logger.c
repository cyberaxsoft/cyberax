#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"

gboolean init_log_dir(LogParams* log_params)
{
	gboolean result = FALSE;

	struct stat sb;

	if (stat(log_params->path, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		g_printf("%s : %s directory exist\n", log_params->prefix, log_params->path);
	}
	else
	{
		g_printf("%s : %s directory not exist, creating\n", log_params->prefix, log_params->path);

		if (mkdir(log_params->path, 0700) == 0)
		{
			g_printf("%s : %s directory created\n", log_params->prefix, log_params->path);
		}
		else
		{
			return result;
		}
	}
	result = TRUE;

	return result;
}

void create_log(LogParams* log_params)//gchar* path, const gchar* prefix, GMutex* log_mutex, const gchar* log_name)
{
	g_mutex_lock(&log_params->mutex);

	if (log_params->enable)
	{
		gchar* filename = g_strdup_printf("%s/%s.log",
												log_params->path,
												log_params->prefix);

		g_printf("%s : create %s\n", log_params->prefix, filename);

		if (filename!=NULL && log_params->log == NULL)
		{
			log_params->log = fopen(filename, "a");
			g_free(filename);
		}
	}

	g_mutex_unlock(&log_params->mutex);

}

void close_log(LogParams* log_params)
{
	g_mutex_lock(&log_params->mutex);

	if (log_params->log!=NULL)
		fclose(log_params->log);

	log_params->log = NULL;

	g_mutex_unlock(&log_params->mutex);


}

void delete_old_logs(LogParams* log_params)
{
	if (log_params->save_days == 0) return;

	g_printf("%s : Deleting old log files (%d, %d)...\n", log_params->prefix, log_params->save_days, log_params->file_size);

	time_t current_time;
	time (&current_time);

	DIR *dir = opendir(log_params->path);
	if(dir)
	{
		struct dirent *ent;
		while((ent = readdir(dir)) != NULL)
		{
			if (strstr(ent->d_name, log_params->prefix) != NULL && strstr(ent->d_name, ".log") != NULL)
			{
				gchar* filename = g_strdup_printf("%s/%s", log_params->path, ent->d_name);

				struct stat _fileStatbuff;

				if ((stat(filename, &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode)))
				{

				}
				else
				{
					if (_fileStatbuff.st_atim.tv_sec < (current_time - (log_params->save_days * 86400) ))
					{
						remove(filename);
						g_printf("%s deleted!\n", filename);
					}
				}


				g_free(filename);
			}
		}
	}
	else
	{
		g_printf("Error opening directory\n");
	}
}

void add_log(LogParams* log_params, gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ... )
		//FILE** log, GMutex* log_mutex, const gchar* prefix, gboolean new_str, gboolean close_str, gboolean trace, gboolean active, guint32 fsize, guint32 sdays, const gchar* format, ...)
{
	va_list args;
	va_start(args, format);

	gchar* string = g_strdup_vprintf(format, args);

	if (trace)
	{
		if (new_str)
		{
			g_printf("%s : ",log_params->prefix);
		}
		g_printf("%s", string);
		if (close_str)
		{
			g_printf("\n");

		}
	}

	va_end(args);

	if (log_params->log!=NULL && active)
	{
		gchar* output_txt = NULL;

		if (new_str)
		{
			struct tm* timeinfo;
			struct timeval tv;
			gettimeofday (&tv, NULL);

			time_t rawtime;
			time (&rawtime);
			timeinfo = localtime(&rawtime);

			output_txt = g_strdup_printf("%04d.%02d.%02d %02d:%02d:%02d.%03ld : %s",timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec / 1000, string);
		}
		else
		{
			output_txt = g_strdup_printf("%s", string);
		}

		g_mutex_lock(&log_params->mutex);

		fprintf(log_params->log,"%s",output_txt);

		if (close_str)
		{
			fprintf(log_params->log,"\n");

		}

		fflush(log_params->log);

		if (log_params->file_size > 0)
		{
			struct stat _fileStatbuff;

			if ((fstat(fileno(log_params->log), &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode)))
			{
				//_file_size = -1;
			}
			else
			{
				if (_fileStatbuff.st_size > (BYTE_IN_MB * log_params->file_size ))
				{
					close_log(log_params);

					struct tm* timeinfo;
					struct timeval tv;
					gettimeofday (&tv, NULL);

					time_t rawtime;
					time (&rawtime);
					timeinfo = localtime(&rawtime);

					gchar* new_filename = g_strdup_printf("%s/%s_%04d%02d%02d_%02d%02d%02d_%03ld.log", log_params->path, log_params->prefix,timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec / 1000);
					gchar* old_filename = g_strdup_printf("%s/%s.log", log_params->path, log_params->prefix);

					rename(old_filename, new_filename);

					g_free(old_filename);
					g_free(new_filename);

					create_log(log_params);

					delete_old_logs(log_params);
				}
			}
		}

		g_mutex_unlock(&log_params->mutex);

		g_free(output_txt);

	}
	g_free(string);
}

const gchar* logging_level_to_str(LoggingLevel value)
{
	switch(value)
	{
		case ll_System: 						return "System";
		case ll_Critical: 						return "Critical";
		case ll_Error: 							return "Error";
		case ll_Warning: 						return "Warning";
		case ll_Information: 					return "Information";
		case ll_Debug: 							return "Debug";
		default: return "Undefined";

	}

}

void create_log_params(LogParams* params, gboolean enable, gchar* path, gchar* prefix, guint32 file_size, guint32 save_days)
{
	params->enable = enable;
	params->file_size = file_size;
	params->log = NULL;
	g_mutex_init(&params->mutex) ;
	params->path = g_strdup(path);
	params->prefix = g_strdup(prefix);
	params->save_days = save_days;
}

void destroy_log_params(LogParams* params)
{
	params->enable = FALSE;
	params->file_size = DEF_LOG_FILE_SIZE;
	g_mutex_clear(&params->mutex);
	params->save_days = DEF_SAVE_DAYS;

	if (params->path!=NULL)
	{
		g_free(params->path);
		params->path = NULL;
	}

	if (params->prefix!=NULL)
	{
		g_free(params->prefix);
		params->prefix = NULL;
	}

}
