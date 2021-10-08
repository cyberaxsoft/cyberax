#ifndef CONFIG_H_
#define CONFIG_H_

#define BORDER_WIDTH		5

#define CONF_PNG			"images/conf_48.png"

typedef struct _DBConfig
{
	gchar* 					address;
	guint16 				port;
	gchar* 					db_name;
	gchar* 					user_name;
	gchar* 					user_password;

}DBConfig;

typedef struct _CommonConfig
{
	gchar* 					guid;
}CommonConfig;

typedef struct _UserInterfaceConfig
{
	gboolean 				tanks_bar_enable;
}UserInterfaceConfig;

typedef struct _PosConfig
{
	CommonConfig 			common_config;
	DBConfig				db_config;
	UserInterfaceConfig		user_interface_config;
}PosConfig;

gchar* get_conf_filename(int argc, char *argv[]);
gboolean read_settings(const gchar* filename, PosConfig* config);

void  show_config_window(GtkWindow* parent, const gchar* filename, PosConfig pos_config);

#endif /* CONFIG_H_ */
