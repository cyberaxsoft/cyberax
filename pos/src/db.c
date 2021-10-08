#include <glib.h>
#include <glib/gstdio.h>
#include <libpq-fe.h>

#include <gtk/gtk.h>

#include "config.h"
#include "db.h"

PGconn* db_connect(DBConfig* db_config)
{
	PGconn* result = NULL;

	gchar* connection_string = g_strdup_printf("hostaddr=%s port=%d user=%s password=%s dbname=%s ", db_config->address, db_config->port,
							db_config->user_name, db_config->user_password, db_config->db_name);

	if (connection_string !=NULL)
	{
		result = PQconnectdb(connection_string);
		g_free(connection_string);

		if (PQstatus(result) != CONNECTION_OK)
		{
		    fprintf(stderr, "ERROR: Connection to database failed: %s", PQerrorMessage(result));
		    PQfinish(result);

		    return NULL;
		}
		else
		{
			return result;
		}
	}
	else
	{
		return NULL;
	}
}

void db_disconnect(PGconn* connection)
{
	if (connection != NULL)
	{
		PQfinish(connection);
	}
}

gboolean db_exec(PGconn* connection, gchar* querry)
{
	gboolean result = FALSE;

	PGresult* res = PQexec(connection, querry);

	if(PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		return result;
	}
	else
	{
		gint ncols = PQnfields(res);

		for(gint i = 0; i < ncols; i++)
		{
			gchar *name = PQfname(res, i);
		    printf(" %s", name);
		}

		 gint nrows = PQntuples(res);
		 for(gint i = 0; i < nrows; i++)
		 {
			 for (gint j = 0; j < ncols; j++)
			 {
		        gchar* field = PQgetvalue(res, i, j);
			    g_free(field);
			 }
		 }
		 PQclear(res);
		 res = NULL;
		 result = TRUE;
	}

	return result;

}

