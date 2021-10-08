#ifndef LOGGER_H_
#define LOGGER_H_

#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gmodule.h>
#include <gio/gio.h>
#include <glib/gthread.h>



gboolean init_log_dir(gchar* dir, const gchar* prefix);
FILE* create_log(gchar* path, const gchar* prefix, GMutex* log_mutex, const gchar* log_name);
void close_log(FILE* filename);
void add_log(FILE** log, GMutex* log_mutex, const gchar* prefix, gboolean new_str, gboolean close_str, gboolean trace, gboolean active, const gchar* format, ...);


#endif /* LOGGER_H_ */
