#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#define BORDER_WIDTH	2


typedef struct _TankButton
{
	guint 		num;

	GtkWidget* 	base_button;

	GtkWidget* 	main_box;
	GtkWidget* 	title_label;
	GtkWidget* 	body_box;

	GtkWidget* 	area;
	GtkWidget* 	param_box;

	GtkWidget* 	level_label;
	GtkWidget*	volume_label;
	GtkWidget* 	density_label;
	GtkWidget* 	weight_label;
	GtkWidget* 	temp_label;
	GtkWidget* 	water_label;
}TankButton;

typedef struct _DispencerButton
{
	guint 		num;

	GtkWidget* 	base_button;

	GtkWidget* 	main_box;

	GtkWidget* 	num_label;
	GtkWidget* 	order_mode_label;

	GtkWidget* 	preset_entry;
	GtkWidget* 	product_entry;
	GtkWidget* 	current_litres_entry;
	GtkWidget* 	current_amount_entry;



}DispencerButton;


void create_main_window(guint32 width, guint32 height, gboolean fullscreen);
GtkWidget* get_main_window();
void draw_main_window(PosConfig* pos_config);

#endif /* MAIN_WINDOW_H_ */
