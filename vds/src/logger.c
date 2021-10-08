#include <glib.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

#include "driver.h"
#include "logger.h"

FILE* log_file = NULL;
GMutex	log_mutex;

gchar* log_path = NULL;
guint32 fsize = DEF_LOG_FILE_SIZE;
guint32 sdays = DEF_SAVE_DAYS;

gboolean init_log_dir(gchar* dir, const gchar* prefix)
{
	gboolean result = FALSE;

	struct stat sb;

	if (stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		g_printf("%s : %s directory exist\n", prefix, dir);
	}
	else
	{
		g_printf("%s : %s directory not exist, creating\n", prefix, dir);

		if (mkdir(dir, 0700) == 0)
		{
			g_printf("%s : %s directory created\n", prefix, dir);
		}
		else
		{
			return result;
		}
	}
	result = TRUE;

	return result;
}

FILE* create_log(gchar* path, const gchar* prefix, GMutex* log_mutex, const gchar* log_name)
{

	FILE* result = NULL;

	gchar* filename = g_strdup_printf("%s/%s.log", path, log_name);

	g_printf("%s : create %s\n", prefix, filename);

	if (filename!=NULL)
	{
		result = fopen(filename, "a");
		g_free(filename);
	}

	return result;
}

gboolean init_log(gchar* dir, guint32 file_size, guint32 save_days)
{
	if (log_mutex.p == NULL)
	{
		g_mutex_init(&log_mutex);
	}

	if (log_path != NULL)
	{
		g_free(log_path);
		log_path = NULL;
	}
	log_path = g_strdup(dir);

	fsize = file_size;
	sdays = save_days;


	log_file = create_log(dir, LOG_PREFIX, &log_mutex, LOG_FILENAME);
	if (log_file == NULL)
	{
		return FALSE;
	}

	return TRUE;
}

void close_log()
{
	if (log_file!=NULL)
		fclose(log_file);
}

void delete_old_logs(gchar* path)
{
	if (sdays == 0) return;

	g_printf("%s : Deleting old log files...\n", LOG_PREFIX);

	time_t current_time;
	time (&current_time);

	DIR *dir = opendir(path);
	if(dir)
	{
		struct dirent *ent;
		while((ent = readdir(dir)) != NULL)
		{
			if (strstr(ent->d_name, LOG_FILENAME) != NULL && strstr(ent->d_name, ".log") != NULL)
			{
				gchar* filename = g_strdup_printf("%s/%s", path, ent->d_name);

				struct stat _fileStatbuff;

				if ((stat(filename, &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode)))
				{

				}
				else
				{
					if (_fileStatbuff.st_atim.tv_sec < (current_time - (sdays * 86400) ))
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

void add_log(gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ...)
{
	va_list args;
	va_start(args, format);

	gchar* string = g_strdup_vprintf(format, args);

	if (trace)
	{
		if (new_str)
		{
			g_printf("%s : ",LOG_PREFIX);
		}
		g_printf("%s", string);
		if (close_str)
		{
			g_printf("\n");

		}
	}

	va_end(args);

	if (log_file!=NULL && active)
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

		g_mutex_lock(&log_mutex);

		fprintf(log_file,"%s",output_txt);

		if (close_str)
		{
			fprintf(log_file,"\n");
		}

		fflush(log_file);

		if (fsize > 0)
		{

			struct stat _fileStatbuff;

			if ((fstat(fileno(log_file), &_fileStatbuff) != 0) || (!S_ISREG(_fileStatbuff.st_mode)))
			{
				//_file_size = -1;
			}
			else
			{
				if (_fileStatbuff.st_size > (BYTE_IN_MB * fsize ))
				{
					close_log();

					struct tm* timeinfo;
					struct timeval tv;
					gettimeofday (&tv, NULL);

					time_t rawtime;
					time (&rawtime);
					timeinfo = localtime(&rawtime);

					gchar* new_filename = g_strdup_printf("%s/%s_%04d%02d%02d_%02d%02d%02d_%03ld.log", log_path, LOG_FILENAME,timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,timeinfo->tm_mday,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, tv.tv_usec / 1000);
					gchar* old_filename = g_strdup_printf("%s/%s.log", log_path, LOG_FILENAME);

					rename(old_filename, new_filename);

					g_free(old_filename);
					g_free(new_filename);

					init_log(log_path, fsize, sdays);

					delete_old_logs(log_path);
				}
			}
		}
		g_mutex_unlock(&log_mutex);

		g_free(output_txt);

	}
	g_free(string);
}

void add_tx_buffer_to_log(guint8* buffer, guint16 size, LogOptions log_options)
{
	add_log(TRUE, FALSE, log_options.trace, log_options.frames, " >> ");

	for (guint16 i = 0; i < size; i++)
	{
		add_log(FALSE, FALSE, log_options.trace, log_options.frames, " %02X", buffer[i]);

	}

	add_log(FALSE, TRUE, log_options.trace, log_options.frames, "");
}
