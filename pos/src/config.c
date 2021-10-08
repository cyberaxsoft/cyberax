#include <glib.h>
#include <glib/gstdio.h>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

#include <gtk/gtk.h>

#include "pos.h"
#include "config.h"

GtkWidget* 	config_window = NULL;

GtkWidget*  image = NULL;
GtkWidget*  label = NULL;
GtkWidget* hbox_header = NULL;

GtkWidget* hbox_guid = NULL;
GtkWidget* entry_guid = NULL;
GtkWidget*  label_guid = NULL;

GtkWidget* hbox_ip_address = NULL;
GtkWidget* entry_ip_address = NULL;
GtkWidget*  label_ip_address = NULL;

GtkWidget* hbox_port = NULL;
GtkWidget* entry_port = NULL;
GtkWidget*  label_port = NULL;

GtkWidget* hbox_db_name  = NULL;
GtkWidget* entry_db_name = NULL;
GtkWidget*  label_db_name = NULL;

GtkWidget* hbox_user_name = NULL;
GtkWidget* entry_user_name = NULL;
GtkWidget*  label_user_name = NULL;

GtkWidget* hbox_user_password = NULL;
GtkWidget* entry_user_password = NULL;
GtkWidget*  label_user_password = NULL;

gchar* get_conf_filename(int argc, char *argv[])
{
	gchar* result = NULL;

	if (argc > 1 && argv[1] !=NULL)
	{
		result  = g_strdup(argv[1]);
	}
	else
	{
		result  = g_strdup(POS_CONFIG_FILE);
	}

	return result;
}



int is_leaf(xmlNode * node)
{
  xmlNode * child = node->children;

  while(child)
  {
    if(child->type == XML_ELEMENT_NODE) return 0;

    child = child->next;
  }

  return 1;
}

void parse_xml(xmlNode * node, PosConfig* config)
{
	xmlNode * nodes = node->children;

    while(nodes)
    {
        if(nodes->type == XML_ELEMENT_NODE)
        {
        	if (strcmp((gchar*)nodes->name,"common") == 0)
        	{
        		CommonConfig* common_conf = &config->common_config;

        		if (common_conf->guid!=NULL)
        		{
        			g_free(common_conf->guid);
        			common_conf->guid = NULL;
        		}

        		common_conf->guid = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"guid"));

        	}

        	else if (strcmp((gchar*)nodes->name,"db") == 0)
        	{
        		DBConfig* db_conf = &config->db_config;

        		if (db_conf->address!=NULL)
        		{
        			g_free(db_conf->address);
        			db_conf->address = NULL;
        		}
        		db_conf->address = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"address"));

        		db_conf->port = atoi((gchar*)xmlGetProp(nodes,(guchar*)"port"));

        		if (db_conf->db_name!=NULL)
        		{
        			g_free(db_conf->db_name);
        			db_conf->db_name = NULL;
        		}
        		db_conf->db_name = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"db_name"));

        		if (db_conf->user_name!=NULL)
        		{
        			g_free(db_conf->user_name);
        			db_conf->user_name = NULL;
        		}
        		db_conf->user_name = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"user_name"));

        		if (db_conf->user_password!=NULL)
        		{
        			g_free(db_conf->user_password);
        			db_conf->user_password = NULL;
        		}
        		db_conf->user_password = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"user_password"));
        	}

        	else if (strcmp((gchar*)nodes->name,"user_interface") == 0)
        	{
        		UserInterfaceConfig* user_interface_conf = &config->user_interface_config;

        		user_interface_conf->tanks_bar_enable = atoi((gchar*)xmlGetProp(nodes,(guchar*)"tanks_bar_enable"));
        	}
        }
        nodes = nodes->next;
    }
}


gboolean read_settings(const gchar* filename, PosConfig* config)
{
	xmlDoc *doc = xmlReadFile(filename, NULL, 0);

	if (doc == NULL)
	{
		fprintf(stderr,"system : could not parse %s", filename);
		return FALSE;
	}

	xmlNode *root_element = xmlDocGetRootElement(doc);

	parse_xml(root_element, config);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	return TRUE;
}

void add_common(xmlNodePtr root_node)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"common");

  	xmlSetProp(node, (guchar*)"guid", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_guid))));

    xmlAddChild(root_node,node);
}

void add_db(xmlNodePtr root_node)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"db");

  	xmlSetProp(node, (guchar*)"address", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_ip_address))));
  	xmlSetProp(node, (guchar*)"port", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_port))));
  	xmlSetProp(node, (guchar*)"db_name", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_db_name))));
  	xmlSetProp(node, (guchar*)"user_name", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_user_name))));
  	xmlSetProp(node, (guchar*)"user_password", (guchar*)g_strdup_printf("%s",gtk_entry_get_text(GTK_ENTRY(entry_user_password))));

    xmlAddChild(root_node,node);
}

gboolean write_settings(const gchar* filename)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
    if (doc == NULL)
    {
        return FALSE;
    }

    xmlNodePtr root_node = xmlNewDocNode(doc, NULL, BAD_CAST "root", NULL);
    if (root_node == NULL)
    {
        return FALSE;
    }

    xmlDocSetRootElement(doc, root_node);

    add_common(root_node);
    add_db(root_node);

    xmlSaveFormatFile(filename, doc, 1);

    xmlFreeDoc(doc);

	return TRUE;
}


void  show_config_window(GtkWindow* parent, const gchar* filename, PosConfig pos_config)
{
	image = gtk_image_new_from_file (CONF_PNG);
	label = gtk_label_new ("Настройки POS");

	config_window = gtk_dialog_new_with_buttons ("Настройка", parent, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

	gtk_container_set_border_width (GTK_CONTAINER(config_window), 20);


	//header
	GtkWidget* hbox_header = gtk_hbox_new(FALSE, 20);
	gtk_container_add (GTK_CONTAINER (hbox_header), image);
	gtk_container_add (GTK_CONTAINER (hbox_header), label);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_header);


	//GUID
	hbox_guid = gtk_hbox_new(TRUE, 20);
	entry_guid = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_guid), pos_config.common_config.guid);
	label_guid = gtk_label_new ("POS GUID");
	gtk_container_add (GTK_CONTAINER (hbox_guid), label_guid);
	gtk_container_add (GTK_CONTAINER (hbox_guid), entry_guid);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_guid);

	//DB server address
	hbox_ip_address = gtk_hbox_new(TRUE, 20);
	entry_ip_address = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_ip_address), pos_config.db_config.address);
	label_ip_address = gtk_label_new ("IP Address");
	gtk_container_add (GTK_CONTAINER (hbox_ip_address), label_ip_address);
	gtk_container_add (GTK_CONTAINER (hbox_ip_address), entry_ip_address);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_ip_address);

	//DB server port
	gchar* port_str = g_strdup_printf("%d", pos_config.db_config.port);

	hbox_port = gtk_hbox_new(TRUE, 20);
	entry_port = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_port), port_str);
	label_port = gtk_label_new ("Port");
	gtk_container_add (GTK_CONTAINER (hbox_port), label_port);
	gtk_container_add (GTK_CONTAINER (hbox_port), entry_port);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_port);
	g_free(port_str);

	//DB name
	hbox_db_name = gtk_hbox_new(TRUE, 20);
	entry_db_name = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_db_name), pos_config.db_config.db_name);
	label_db_name = gtk_label_new ("DB name");
	gtk_container_add (GTK_CONTAINER (hbox_db_name), label_db_name);
	gtk_container_add (GTK_CONTAINER (hbox_db_name), entry_db_name);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_db_name);

	//user name
	hbox_user_name = gtk_hbox_new(TRUE, 20);
	entry_user_name = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_user_name), pos_config.db_config.user_name);
	label_user_name = gtk_label_new ("Username");
	gtk_container_add (GTK_CONTAINER (hbox_user_name), label_user_name);
	gtk_container_add (GTK_CONTAINER (hbox_user_name), entry_user_name);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_user_name);

	//user password
	hbox_user_password = gtk_hbox_new(TRUE, 20);
	entry_user_password = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry_user_password), pos_config.db_config.user_password);
	label_user_password = gtk_label_new ("Password");
	gtk_container_add (GTK_CONTAINER (hbox_user_password), label_user_password);
	gtk_container_add (GTK_CONTAINER (hbox_user_password), entry_user_password);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(config_window)->vbox), hbox_user_password);

	gtk_entry_set_invisible_char(GTK_ENTRY(entry_user_password), '*');
	gtk_entry_set_visibility(GTK_ENTRY(entry_user_password), FALSE);

	gtk_widget_show_all (config_window);

	gint result = gtk_dialog_run (GTK_DIALOG (config_window));

	if (result == GTK_RESPONSE_ACCEPT)
	{
		write_settings(filename);
	}

	gtk_widget_destroy (config_window);


}

