#ifndef MESSAGE_DLG_H_
#define MESSAGE_DLG_H_

#define ERROR_PNG			"images/stop_48.png"
#define INFO_PNG			"images/info_48.png"
#define WARNING_PNG			"images/warning_48.png"
#define CONFIRM_PNG			"images/info_48.png"


#define ERROR_TITLE_STR		"Ошибка"
#define INFO_TITLE_STR		"Информация"
#define WARNING_TITLE_STR	"Предупреждение"
#define CONFIRM_TITLE_STR	"Подтверждение"


typedef enum _MessageDlgType
{
	mdt_Error			= 0,
	mdt_Information		= 1,
	mdt_Warning			= 2,
	mdt_Confirmation	= 3,
}MessageDlgType;

gint show_error_dlg(GtkWindow* parent, gchar* message);
gint show_information_dlg(GtkWindow* parent, gchar* message);
gint show_warning_dlg(GtkWindow* parent, gchar* message);
gint show_confirmation_dlg(GtkWindow* parent, gchar* message);
#endif /* MESSAGE_DLG_H_ */
