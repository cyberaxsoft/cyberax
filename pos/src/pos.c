/*
 ============================================================================
 Name        : pos.c
 Author      : Konstantin Mazalov
 Version     :
 Copyright   : (C) Copyright 2021 All rights reserved
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>

#include "pos.h"
#include "config.h"
#include "db.h"
#include "main_window.h"
#include "message_dlg.h"
#include "tgs.h"

guint32 width = DEFAULT_SCREEN_WIDTH;
guint32 height = DEFAULT_SCREEN_HEIGHT;
gboolean fullscreen = FALSE;

void parse_param(int argc, char* argv[])
{
	if (argc > 0)
	{
		for (guint i = 0; i < argc; i++)
		{
			if (strstr(argv[i],"--width:")!=NULL)
			{
				sscanf(strstr(argv[i],"--width:") + strlen("--width:"),"%d",&width);
			}
			else if (strstr(argv[i],"--height:")!=NULL)
			{
				sscanf(strstr(argv[i],"--height:") + strlen("--height:"),"%d",&width);
			}
			else if (strstr(argv[i],"--fullscreen:")!=NULL)
			{
				sscanf(strstr(argv[i],"--fullscreen:") + strlen("--fullscreen:"),"%d",&fullscreen);
			}
		}

	}
}

int main(int argc, char *argv[])
{

	set_start_tank_value();

	gtk_init(&argc, &argv);

	parse_param(argc, argv);


	gchar* conf_filename = get_conf_filename(argc, argv);
	PosConfig pos_config = {0x00};
	PGconn* db_conn = NULL;

	create_main_window(width, height, fullscreen);

	while(!read_settings(conf_filename, &pos_config))
	{
		show_error_dlg(GTK_WINDOW(get_main_window()), "Configuration error");
		show_config_window(GTK_WINDOW(get_main_window()), conf_filename, pos_config);
	}

	db_conn = db_connect(&pos_config.db_config);

	while( db_conn == NULL )
	{
		show_error_dlg(GTK_WINDOW(get_main_window()), "DB connection error");
		show_config_window(GTK_WINDOW(get_main_window()), conf_filename, pos_config);

		while(!read_settings(conf_filename, &pos_config))
		{
			show_error_dlg(GTK_WINDOW(get_main_window()), "Configuration error");
			show_config_window(GTK_WINDOW(get_main_window()), conf_filename, pos_config);
		}

		db_conn = db_connect(&pos_config.db_config);
	}

	gchar* user_guid = logon(GTK_WINDOW(get_main_window()),db_conn);

	while( user_guid != NULL )
	{
		show_error_dlg(GTK_WINDOW(get_main_window()), "Logon error");

		user_guid = logon(GTK_WINDOW(get_main_window()),db_conn);
	}

	draw_main_window(&pos_config);


	gtk_main();

	db_disconnect(db_conn);

	if (conf_filename != NULL)
	{
		g_free(conf_filename);
	}

	return EXIT_SUCCESS;
}
