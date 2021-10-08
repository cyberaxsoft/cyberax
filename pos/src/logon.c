#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include <libpq-fe.h>

#include "logon.h"

GtkWidget* 	logon_window = NULL;

gchar* logon(GtkWindow* parent, PGconn* db_conn)
{
	gchar* result = NULL;

	logon_window = gtk_dialog_new_with_buttons ("Регистрация пользователя", parent, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

	gtk_container_set_border_width (GTK_CONTAINER(logon_window), 20);


	GtkWidget*  image = gtk_image_new_from_file (LOGON_PNG);

	//header
	GtkWidget* hbox_header = gtk_hbox_new(FALSE, 10);
	gtk_container_add (GTK_CONTAINER (hbox_header), image);

	GtkWidget* vbox_work = gtk_vbox_new(FALSE, 10);

	GtkWidget* hbox_username = gtk_hbox_new(TRUE, 20);
	GtkWidget* label_username = gtk_label_new ("Имя пользователя");
	GtkWidget* combobox_username = gtk_combo_box_new_text();



	gtk_combo_box_append_text(GTK_COMBO_BOX (combobox_username), "Иванов");
	gtk_combo_box_append_text(GTK_COMBO_BOX (combobox_username), "Петров");
	gtk_combo_box_append_text(GTK_COMBO_BOX (combobox_username), "Сидоров");

	gtk_container_add (GTK_CONTAINER (hbox_username), label_username);
	gtk_container_add (GTK_CONTAINER (hbox_username), combobox_username);

	gtk_container_add (GTK_CONTAINER (vbox_work), hbox_username);

	GtkWidget* hbox_userpass = gtk_hbox_new(TRUE, 20);
	GtkWidget* label_userpass = gtk_label_new ("Пароль");
	GtkWidget* entry_userpass = gtk_entry_new();
	gtk_container_add (GTK_CONTAINER (hbox_userpass), label_userpass);
	gtk_container_add (GTK_CONTAINER (hbox_userpass), entry_userpass);
	gtk_entry_set_invisible_char(GTK_ENTRY(entry_userpass), '*');
	gtk_entry_set_visibility(GTK_ENTRY(entry_userpass), FALSE);

	gtk_container_add (GTK_CONTAINER (vbox_work), hbox_userpass);

	gtk_container_add (GTK_CONTAINER (hbox_header), vbox_work);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(logon_window)->vbox), hbox_header);
	GtkWidget* label_free = gtk_label_new ("");
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(logon_window)->vbox), label_free);

	gtk_widget_show_all (logon_window);

	gint dialog_result = gtk_dialog_run (GTK_DIALOG (logon_window));

	if (dialog_result == GTK_RESPONSE_ACCEPT)
	{

	}

	gtk_widget_destroy (logon_window);

	return result;
}
