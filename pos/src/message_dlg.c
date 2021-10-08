#include <gtk/gtk.h>

#include "message_dlg.h"


gint show_dlg(GtkWindow* parent, MessageDlgType dlg_type, gchar* message)
{
	gint result = GTK_RESPONSE_REJECT;

	GtkWidget*  image = NULL;
	GtkWidget* 	dialog = NULL;

	GtkWidget*  label = gtk_label_new (message);

	switch (dlg_type)
	{
		case mdt_Error:
			image = gtk_image_new_from_file (ERROR_PNG);
			dialog = gtk_dialog_new_with_buttons (ERROR_TITLE_STR,
					 	 	 	 	 	 	 	 	 parent,
			                                         GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
													 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
			break;

		case mdt_Information:
			image = gtk_image_new_from_file (INFO_PNG);
			dialog = gtk_dialog_new_with_buttons (INFO_TITLE_STR,
					 	 	 	 	 	 	 	 	 parent,
			                                         GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
													 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
			break;

		case mdt_Warning:
			image = gtk_image_new_from_file (WARNING_PNG);
			dialog = gtk_dialog_new_with_buttons (WARNING_TITLE_STR,
					 	 	 	 	 	 	 	 	 parent,
			                                         GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
													 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, NULL);
			break;

		case mdt_Confirmation:
			image = gtk_image_new_from_file (CONFIRM_PNG);
			dialog = gtk_dialog_new_with_buttons (CONFIRM_TITLE_STR,
					 	 	 	 	 	 	 	 	 parent,
			                                         GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
													 GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
			break;
	}


	gtk_container_set_border_width (GTK_CONTAINER(dialog), 20);

	GtkWidget* hbox = gtk_hbox_new(FALSE, 20);

	gtk_container_add (GTK_CONTAINER (hbox), image);
	gtk_container_add (GTK_CONTAINER (hbox), label);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), hbox);

	gtk_widget_show_all (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);

	return result;
}
gint show_error_dlg(GtkWindow* parent, gchar* message)
{
	return show_dlg(parent, mdt_Error, message );
}

gint show_information_dlg(GtkWindow* parent, gchar* message)
{
	return show_dlg(parent, mdt_Information, message);
}

gint  show_warning_dlg(GtkWindow* parent, gchar* message)
{
	return show_dlg(parent, mdt_Warning, message);
}

gint show_confirmation_dlg(GtkWindow* parent, gchar* message)
{
	return show_dlg(parent, mdt_Confirmation, message);

}
