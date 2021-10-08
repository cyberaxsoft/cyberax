#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "logger.h"

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
	if (log_mutex->p == NULL)
	{
		g_mutex_init(log_mutex);
	}

	FILE* result = NULL;

	gchar* filename = g_strdup_printf("%s/%s", path, log_name);

	g_printf("%s : create %s\n", prefix, filename);

	if (filename!=NULL)
	{
		result = fopen(filename, "a");
		g_free(filename);
	}

	return result;
}

void close_log(FILE* log)
{
	if (log!=NULL)
		fclose(log);
}

void add_log(FILE** log, GMutex* log_mutex, const gchar* prefix, gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ...)
{
	va_list args;
	va_start(args, format);

	gchar* string = g_strdup_vprintf(format, args);

	if (trace)
	{
		if (new_str)
		{
			g_printf("%s : ",prefix);
		}
		g_printf("%s", string);
		if (close_str)
		{
			g_printf("\n");

		}
	}

	va_end(args);

	if (log!=NULL && active)
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

		g_mutex_lock(log_mutex);

		fprintf(*log,"%s",output_txt);

		if (close_str)
		{
			fprintf(*log,"\n");

		}

		fflush(*log);

		g_mutex_unlock(log_mutex);

		g_free(output_txt);

	}
	g_free(string);
}

