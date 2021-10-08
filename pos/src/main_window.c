#include <gtk/gtk.h>
#include <gmodule.h>

#include "config.h"
#include "tgs.h"
#include "disp.h"
#include "main_window.h"

GtkWidget* main_window;
GtkWidget* main_box;

GtkWidget* tanks_box;
GtkWidget* control_box;
GtkWidget* left_box;
GtkWidget* dispencers_box;
GtkWidget* button_shop;

GtkWidget* right_box;
GtkWidget* operation_box;
GtkWidget* system_buttons_box;

GtkWidget* work_box;

GtkWidget* work_control_box;

GtkWidget* work_receipt_box;


GtkWidget* devices_box;

TankButton tank_buttons[MAX_TANK_COUNT] = {0x00};
DispencerButton dispencer_buttons[MAX_DISP_COUNT] = {0x00};


GtkWidget* get_main_window()
{
	return main_window;
}

gint event_delete(GtkWidget* widget, GdkEvent* event, gpointer data)
{
	return(FALSE);
}

void create_main_menu(GtkWidget* parent)
{

	GtkWidget* menu_bar = gtk_menu_bar_new();

	GtkWidget* file_menu = gtk_menu_new(); 	//создаем субменю Файл
	GtkWidget* help_menu = gtk_menu_new();	//создаем субменю Помощ

	GtkWidget* file_menu_item = gtk_menu_item_new_with_label("Файл");			//создаем элемент меню Файл
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), file_menu); 		//назначаем ему суб меню Файл

	GtkWidget* help_menu_item = gtk_menu_item_new_with_label("Помощь");			//создаем элемент меню Помощ
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_menu_item), help_menu);        //назначаем ему субменю Помощь

	GtkWidget* shift_menu_item = gtk_menu_item_new_with_label("Смена");			//создаем элемент меню Смена
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), shift_menu_item);			//назначаем его на субменю Файл

	GtkWidget* quit_menu_item = gtk_menu_item_new_with_label("Выход");			//создаем элемент меню Выход
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_menu_item);			//назначаем его на суб меню файл

	GtkWidget* about_menu_item = gtk_menu_item_new_with_label("О программе");	//создаем элемент меню О программе
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_menu_item);			//назначаем его на субменю Помощь


	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_menu_item);			//назначаем элемент меню Файл в основной бар меню
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_menu_item);			//назначаем элемент меню Помощь в основной бар меню

	gtk_box_pack_start(GTK_BOX(parent), menu_bar, FALSE, FALSE, 0);			//назначаем меню в основной бокс окна
	//gtk_container_add (GTK_CONTAINER (parent), menu_bar);

	//назначаем обработчики

	g_signal_connect(G_OBJECT(quit_menu_item), "activate", G_CALLBACK(gtk_main_quit), NULL);

}

void draw_tank_area(GtkWidget* area)
{
	GdkGC *gc;
	GdkColor color;

	//color.pixel = 0xffffffff;

	GdkGCValues gcvalues;

	gchar* area_name = NULL;

	g_object_get(area, "name", &area_name, NULL);

	guint32 tank_num = atoi(area_name);

	if (GDK_DRAWABLE(area -> window) != NULL)
	{
		gc = gdk_gc_new(GDK_DRAWABLE(area -> window));
		gdk_gc_get_values(gc, &gcvalues);

		gint diametr = MIN(area->allocation.width, area->allocation.height) - (BORDER_WIDTH * 2);

		gdk_color_parse("yellow", &color);

		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_draw_arc(area -> window, gc,TRUE, BORDER_WIDTH, BORDER_WIDTH, diametr, diametr,  0, 23000);



		//*color = area->style->bg;
		gdk_gc_set_rgb_fg_color(gc,area->style->bg);

		Tank* tank = get_tank_from_num(tank_num);

		if (tank != NULL)
		{
			gdk_draw_rectangle(area -> window, gc,TRUE, 0, 0, area->allocation.width, area->allocation.height - ((diametr * tank->level) / tank->full_level) - BORDER_WIDTH);
		}
		else
		{
			gdk_draw_rectangle(area -> window, gc,TRUE, 0, 0, area->allocation.width, area->allocation.height);
		}

		gdk_color_parse("black", &color);
		gdk_gc_set_rgb_fg_color(gc, &color);
		gdk_draw_arc(area -> window, gc,FALSE, BORDER_WIDTH, BORDER_WIDTH, diametr, diametr,  0, 23000);
	}

	if (area_name !=NULL)
	{
		g_free(area_name);
	}

}


void update_tank_buttons()
{
	for (guint i = 0; i < MAX_TANK_COUNT; i++)
	{
		if (tank_buttons[i].base_button!=NULL)
		{
			Tank* tank = get_tank_from_num(tank_buttons[i].num);

			if (tank != NULL)
			{
				gchar* title_string = g_strdup_printf("Резервуар %d", tank->num);
				gchar* level_string = g_strdup_printf("L: %.2f мм", tank->level);
				gchar* volume_string = g_strdup_printf("V: %.2f л", tank->volume);
				gchar* density_string = g_strdup_printf("D: %.4f т/м3", tank->density);
				gchar* weight_string = g_strdup_printf("M: %.2f кг", tank->weight);
				gchar* temperature_string = g_strdup_printf("T: %.2f C", tank->temperature);
				gchar* water_level_string = g_strdup_printf("W: %.2f мм", tank->water_level);

				gtk_label_set_label(GTK_LABEL(tank_buttons[i].title_label), title_string);

				gtk_label_set_label(GTK_LABEL(tank_buttons[i].level_label), level_string);
				gtk_label_set_label(GTK_LABEL(tank_buttons[i].volume_label), volume_string);
				gtk_label_set_label(GTK_LABEL(tank_buttons[i].density_label), density_string);
				gtk_label_set_label(GTK_LABEL(tank_buttons[i].weight_label), weight_string);
				gtk_label_set_label(GTK_LABEL(tank_buttons[i].temp_label), temperature_string);
				gtk_label_set_label(GTK_LABEL(tank_buttons[i].water_label), water_level_string);

				g_free(title_string);
				g_free(level_string);
				g_free(volume_string);
				g_free(density_string);
				g_free(weight_string);
				g_free(temperature_string);
				g_free(water_level_string);

				draw_tank_area(tank_buttons[i].area);

			}
		}
	}
}


static gboolean draw_tank_button_event(GtkWidget* area, GdkEventExpose* event, gpointer data)
{
	draw_tank_area(area);
	return TRUE;
}

void create_tank_button(TankButton* tank_button, guint32 num)
{
	tank_button->num = num;

	tank_button->base_button = gtk_button_new();

	gchar* title_string = g_strdup_printf("Резервуар %d", 0);
	gchar* level_string = g_strdup_printf("L: %.2f мм", 0.00);
	gchar* volume_string = g_strdup_printf("V: %.2f л", 0.00);
	gchar* density_string = g_strdup_printf("D: %.4f т/м3", 0.00);
	gchar* weight_string = g_strdup_printf("M: %.2f кг", 0.00);
	gchar* temperature_string = g_strdup_printf("T: %.2f C", 0.00);
	gchar* water_level_string = g_strdup_printf("W: %.2f мм", 0.00);

	gchar* area_name = g_strdup_printf("%d", num);

	tank_button->main_box = gtk_vbox_new(FALSE, 0);

	tank_button->title_label = gtk_label_new(title_string);
	gtk_label_set_justify(GTK_LABEL(tank_button->title_label), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start(GTK_BOX(tank_button->main_box), tank_button->title_label, FALSE, FALSE, 0);
	g_object_set(tank_button->title_label, "xalign", 0.0, NULL);

	tank_button->body_box = gtk_hbox_new(TRUE, 0);

	tank_button->area = gtk_drawing_area_new();
	g_object_set(tank_button->area, "name", area_name, NULL);

	gtk_container_add (GTK_CONTAINER (tank_button->body_box), tank_button->area);

	g_signal_connect(G_OBJECT(tank_button->area), "expose_event", G_CALLBACK(draw_tank_button_event), NULL);

	tank_button->param_box = gtk_vbox_new(FALSE, 0);

	tank_button->level_label = gtk_label_new(level_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->level_label);
	g_object_set(tank_button->level_label, "xalign", 0.0, NULL);
	tank_button->volume_label = gtk_label_new(volume_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->volume_label);
	g_object_set(tank_button->volume_label, "xalign", 0.0, NULL);
	tank_button->density_label = gtk_label_new(density_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->density_label);
	g_object_set(tank_button->density_label, "xalign", 0.0, NULL);

	tank_button->weight_label = gtk_label_new(weight_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->weight_label);
	g_object_set(tank_button->weight_label, "xalign", 0.0, NULL);

	tank_button->temp_label = gtk_label_new(temperature_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->temp_label);
	g_object_set(tank_button->temp_label, "xalign", 0.0, NULL);
	tank_button->water_label = gtk_label_new(water_level_string);
	gtk_container_add (GTK_CONTAINER (tank_button->param_box), tank_button->water_label);
	g_object_set(tank_button->water_label, "xalign", 0.0, NULL);

	gtk_container_add (GTK_CONTAINER (tank_button->body_box), tank_button->param_box);
	gtk_container_add (GTK_CONTAINER (tank_button->main_box), tank_button->body_box);
	gtk_container_add (GTK_CONTAINER (tank_button->base_button), tank_button->main_box);


	g_free(area_name);

	g_free(title_string);
	g_free(level_string);
	g_free(volume_string);
	g_free(density_string);
	g_free(temperature_string);
	g_free(water_level_string);

	gtk_button_set_focus_on_click(GTK_BUTTON(tank_button->base_button), FALSE);
	g_object_set(tank_button->base_button, "can-focus", FALSE, NULL);

	gtk_box_pack_start(GTK_BOX(tanks_box), tank_button->base_button, FALSE, FALSE, 0);

}

void create_dispencer_button(DispencerButton* dispencer_button, guint32 num)
{

	gchar* num_label_string = g_strdup_printf("%d", num);

	dispencer_button->num = num;

	GdkColor color;

	dispencer_button->base_button = gtk_button_new();

	gtk_widget_set_size_request(dispencer_button->base_button, left_box->allocation.width, left_box->allocation.width / 12);

	dispencer_button->main_box = gtk_hbox_new(0, 0);
	gtk_widget_set_size_request(dispencer_button->main_box,left_box->allocation.width, left_box->allocation.width / 12);


	dispencer_button->num_label = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(dispencer_button->num_label), num_label_string);
	gtk_widget_set_size_request(dispencer_button->num_label, (left_box->allocation.width / 16) - (BORDER_WIDTH * 2), (left_box->allocation.width / 16) - (BORDER_WIDTH * 2));
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->num_label, FALSE, FALSE, 0);
	gtk_entry_set_alignment(GTK_ENTRY(dispencer_button->num_label), 0.5);


	gdk_color_parse("black", &color);

	gtk_widget_modify_base(dispencer_button->num_label, GTK_STATE_NORMAL,  &color );
	gtk_widget_modify_base(dispencer_button->num_label, GTK_STATE_ACTIVE,  &color );
	gtk_widget_modify_base(dispencer_button->num_label, GTK_STATE_SELECTED,  &color );
	gtk_widget_modify_base(dispencer_button->num_label, GTK_STATE_PRELIGHT,  &color );
	gtk_widget_modify_base(dispencer_button->num_label, GTK_STATE_INSENSITIVE,  &color );

	gdk_color_parse("white", &color);
	gtk_widget_modify_text(dispencer_button->num_label, GTK_STATE_NORMAL,  &color );
	gtk_widget_modify_text(dispencer_button->num_label, GTK_STATE_ACTIVE,  &color );
	gtk_widget_modify_text(dispencer_button->num_label, GTK_STATE_SELECTED,  &color );
	gtk_widget_modify_text(dispencer_button->num_label, GTK_STATE_PRELIGHT,  &color );
	gtk_widget_modify_text(dispencer_button->num_label, GTK_STATE_INSENSITIVE,  &color );

	PangoFontDescription *pfd;
	pfd = pango_font_description_from_string ("Sans Bold 12");

	gtk_widget_modify_font(dispencer_button->num_label, pfd);

	dispencer_button->product_entry = gtk_entry_new();
	gtk_widget_set_size_request(dispencer_button->product_entry, (left_box->allocation.width / 5) - (BORDER_WIDTH * 2),(left_box->allocation.width / 16) - (BORDER_WIDTH * 2));
//	gtk_entry_set_text(GTK_ENTRY(dispencer_button->product_entry), "АИ-95");
	gtk_entry_set_alignment(GTK_ENTRY(dispencer_button->product_entry), 0.5);
	gtk_widget_modify_font(dispencer_button->product_entry, pfd);
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->product_entry, FALSE, FALSE, 0);

	dispencer_button->preset_entry = gtk_entry_new();
	gtk_widget_set_size_request(dispencer_button->preset_entry, (left_box->allocation.width / 5) - (BORDER_WIDTH * 2),(left_box->allocation.width / 16) - (BORDER_WIDTH * 2));
//	gtk_entry_set_text(GTK_ENTRY(dispencer_button->preset_entry), "5000.00");
	gtk_entry_set_alignment(GTK_ENTRY(dispencer_button->preset_entry), 1.0);
	gtk_widget_modify_font(dispencer_button->preset_entry, pfd);
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->preset_entry, FALSE, FALSE, 0);

	dispencer_button->order_mode_label = gtk_label_new(" р ");
	gtk_widget_modify_font(dispencer_button->order_mode_label, pfd);
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->order_mode_label, FALSE, FALSE, 0);

	dispencer_button->current_litres_entry = gtk_entry_new();
	gtk_widget_set_size_request(dispencer_button->current_litres_entry, (left_box->allocation.width / 5) - (BORDER_WIDTH * 2),(left_box->allocation.width / 16) - (BORDER_WIDTH * 2));
	gtk_entry_set_alignment(GTK_ENTRY(dispencer_button->current_litres_entry), 1.0);
//	gtk_entry_set_text(GTK_ENTRY(dispencer_button->current_litres_entry), "43.12");
	gtk_widget_modify_font(dispencer_button->current_litres_entry, pfd);
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->current_litres_entry, FALSE, FALSE, 0);

	dispencer_button->current_amount_entry = gtk_entry_new();
	gtk_widget_set_size_request(dispencer_button->current_amount_entry, (left_box->allocation.width / 5) - (BORDER_WIDTH * 2),(left_box->allocation.width / 16) - (BORDER_WIDTH * 2));
	gtk_entry_set_alignment(GTK_ENTRY(dispencer_button->current_amount_entry), 1.0);
//	gtk_entry_set_text(GTK_ENTRY(dispencer_button->current_amount_entry), "2343.22");
	gtk_widget_modify_font(dispencer_button->current_amount_entry, pfd);
	gtk_box_pack_start(GTK_BOX(dispencer_button->main_box), dispencer_button->current_amount_entry, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (dispencer_button->base_button), dispencer_button->main_box);

	gtk_box_pack_start(GTK_BOX(dispencers_box), dispencer_button->base_button, FALSE, FALSE, 0);

	g_free(num_label_string);
}



void create_tank_area()
{
	for (guint8 i = 0; i < 4; i++)
	{
		create_tank_button(&tank_buttons[i], i+1);
	}
}

void create_disp_area()
{
	for (guint8 i = 0; i < 4; i++)
	{
		create_dispencer_button(&dispencer_buttons[i], i+1);
	}
}


void create_main_window(guint32 width, guint32 height, gboolean fullscreen)
{
	if (main_window == NULL)
	{

		main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(main_window), "Cyberax POS");
		gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

		GdkGeometry hints;

		if (fullscreen)
		{
			gtk_window_fullscreen(GTK_WINDOW(main_window));

			hints.min_width = gdk_screen_get_width(gtk_widget_get_screen(main_window));
			hints.max_width = gdk_screen_get_width(gtk_widget_get_screen(main_window));
			hints.min_height = gdk_screen_get_height(gtk_widget_get_screen(main_window));
			hints.max_height = gdk_screen_get_height(gtk_widget_get_screen(main_window));

		}
		else
		{
			gtk_window_set_default_size(GTK_WINDOW(main_window), width, height);

			hints.min_width = width;
			hints.max_width = width;
			hints.min_height = height;
			hints.max_height = height;

		}
		gtk_window_set_geometry_hints( GTK_WINDOW(main_window), main_window, &hints, (GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));

		gtk_container_set_border_width (GTK_CONTAINER(main_window), 0);
		gtk_widget_realize(main_window);

		main_box = gtk_vbox_new(0, 0);
		gtk_container_add (GTK_CONTAINER (main_window), main_box);

		//create_main_menu(main_box);

		tanks_box = gtk_hbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(main_box), tanks_box, FALSE, FALSE, 0);

		control_box = gtk_hbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(main_box), control_box, TRUE, TRUE, 0);



		left_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(control_box), left_box, TRUE, TRUE, 0);

		dispencers_box = gtk_vbox_new(1, 0);
		gtk_box_pack_start(GTK_BOX(left_box), dispencers_box, FALSE, FALSE, 0);

		right_box = gtk_hbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(control_box), right_box, TRUE, TRUE, 0);


		operation_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(right_box), operation_box, TRUE, TRUE, 0);

		work_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(operation_box), work_box, TRUE, TRUE, 0);

		work_control_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(work_box), work_control_box, TRUE, TRUE, 0);

		work_receipt_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(work_box), work_receipt_box, TRUE, TRUE, 0);


		devices_box = gtk_hbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(operation_box), devices_box, FALSE, FALSE, 0);

		system_buttons_box = gtk_vbox_new(0, 0);
		gtk_box_pack_start(GTK_BOX(right_box), system_buttons_box, TRUE, TRUE, 0);

		GtkWidget* status_button = gtk_button_new_with_label("status");
		gtk_box_pack_start(GTK_BOX(main_box), status_button, FALSE, FALSE, 0);


		//обработчики
		g_signal_connect(G_OBJECT(main_window), "delete_event", GTK_SIGNAL_FUNC(event_delete), NULL);
		g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

		gtk_widget_show_all (main_window);
	}

}

void draw_main_window(PosConfig* pos_config)
{
	if (main_window != NULL)
	{
		if (pos_config->user_interface_config.tanks_bar_enable)
		{
			create_tank_area();
			update_tank_buttons();
		}


		create_disp_area();

		button_shop = gtk_button_new_with_label("Магазин");
		gtk_widget_set_size_request(button_shop, left_box->allocation.width, left_box->allocation.width / 12);
		gtk_box_pack_start(GTK_BOX(dispencers_box), button_shop, FALSE, FALSE, 0);

		GtkWidget* orders_button = gtk_button_new_with_label("orders");
		gtk_box_pack_start(GTK_BOX(left_box), orders_button, TRUE, TRUE, 0);

		//create system buttons

		GtkWidget* button_lists = gtk_button_new_with_label("Lists");
		gtk_widget_set_size_request(button_lists, right_box->allocation.height / 8, right_box->allocation.height / 8);
		gtk_box_pack_start(GTK_BOX(system_buttons_box), button_lists, FALSE, FALSE, 0);

		GtkWidget* button_shift = gtk_button_new_with_label("Shift");
		gtk_widget_set_size_request(button_shift, right_box->allocation.height / 8, right_box->allocation.height / 8);
		gtk_box_pack_start(GTK_BOX(system_buttons_box), button_shift, FALSE, FALSE, 0);

		GtkWidget* button_settings = gtk_button_new_with_label("Settings");
		gtk_widget_set_size_request(button_settings, right_box->allocation.height / 8, right_box->allocation.height / 8);
		gtk_box_pack_start(GTK_BOX(system_buttons_box), button_settings, FALSE, FALSE, 0);

		GtkWidget* button_exit = gtk_button_new_with_label("Exit");
		gtk_widget_set_size_request(button_exit, right_box->allocation.height / 8, right_box->allocation.height / 8);
		gtk_box_pack_start(GTK_BOX(system_buttons_box), button_exit, FALSE, FALSE, 0);

		//create work panel

		//GtkWidget* tmp_button1 = gtk_button_new();
		//gtk_widget_set_size_request(tmp_button1, right_box->allocation.width - right_box->allocation.height / 8, work_box->allocation.height);
		//gtk_box_pack_start(GTK_BOX(work_box), tmp_button1, TRUE, TRUE, 0);

		GtkWidget* tmp_button1 = gtk_button_new_with_label("Control");
		gtk_widget_set_size_request(tmp_button1, right_box->allocation.width - right_box->allocation.height / 8, work_box->allocation.height / 10 * 4);
		gtk_box_pack_start(GTK_BOX(work_control_box), tmp_button1, TRUE, TRUE, 0);

		GtkWidget* tmp_button4 = gtk_button_new_with_label("Control operation button");
		gtk_widget_set_size_request(tmp_button4, right_box->allocation.width - right_box->allocation.height / 8, work_box->allocation.height / 10);
		gtk_box_pack_start(GTK_BOX(work_control_box), tmp_button4, FALSE, FALSE, 0);


		GtkWidget* tmp_button3 = gtk_button_new_with_label("Receipt");
		gtk_widget_set_size_request(tmp_button3, right_box->allocation.width - right_box->allocation.height / 8, work_box->allocation.height / 10 * 4);
		gtk_box_pack_start(GTK_BOX(work_receipt_box), tmp_button3, TRUE, TRUE, 0);

		GtkWidget* tmp_button5 = gtk_button_new_with_label("Receipt operation button");
		gtk_widget_set_size_request(tmp_button5, right_box->allocation.width - right_box->allocation.height / 8, work_box->allocation.height / 10);
		gtk_box_pack_start(GTK_BOX(work_receipt_box), tmp_button5, FALSE, FALSE, 0);



		GtkWidget* tmp_button2 = gtk_button_new_with_label("devices");
		gtk_widget_set_size_request(tmp_button2, right_box->allocation.width - right_box->allocation.height / 8, right_box->allocation.width / 8);
		gtk_box_pack_start(GTK_BOX(devices_box), tmp_button2, TRUE, TRUE, 0);


		gtk_widget_show_all (main_window);
	}
}
