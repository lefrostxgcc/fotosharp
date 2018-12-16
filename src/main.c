#include <gtk/gtk.h>
#include <math.h>

#define	SPACING	5

struct ch_color
{
	guchar red;
	guchar green;
	guchar blue;
	guchar alpha;
};

static void on_button_load_image_clicked(GtkWidget *button, gpointer data);
static void on_button_change_image_clicked(GtkWidget *button, gpointer data);
static void on_button_save_image_clicked(GtkWidget *button, gpointer data);
static void load_image(const gchar *filename);
static void change_image(void);
static void save_image(GtkWidget *img, const gchar *filename);
static void change_grayscale(struct ch_color *in, struct ch_color *out);
static void change_brightness(struct ch_color *in, struct ch_color *out,
	double brightness);
static void change_contrast(struct ch_color *in, struct ch_color *out,
	double contrast);
static void change_correction(struct ch_color *in, struct ch_color *out,
	int (*corr_func)(int));
static int	change_comp_brightness(int component, double brightness);
static int	change_comp_contrast(int component, double contrast);
static void get_pixel(GdkPixbuf *pixbuf, int x, int y, struct ch_color *color);
static void set_pixel(GdkPixbuf *pixbuf, int x, int y, struct ch_color *color);

static int correction_line(int component);
static int correction_sin(int component);
static int correction_exp(int component);
static int correction_log(int component);

struct ch_correction
{
	const char *name;
	int (*fn)(int);
} static corrections[] = 
{
	{"Линейная коррекция",	correction_line},
	{"Синусоидальная",		correction_sin},
	{"Экспоненциальная",	correction_exp},
	{"Логарифмическая",		correction_log}
};

static const int corrections_size = sizeof corrections / sizeof corrections[0];
static double correction_k;

static GdkPixbuf	*last_load_image_pixbuf;
static GtkWidget	*window;
static GtkWidget	*image;
static GtkWidget	*entry_image_filename;
static GtkWidget	*checkbox_grayscale;
static GtkWidget	*scale_brightness;
static GtkWidget	*scale_contrast;
static GtkWidget	*combo_box_correction;

int main(int argc, char *argv[])
{
	GtkWidget		*vbox;
	GtkWidget		*hbox;
	GtkWidget		*frame_image;
	GtkWidget		*button_load_image;
	GtkWidget		*button_change_image;
	GtkWidget		*button_save_image;
	GtkWidget		*label_brightness;
	GtkWidget		*label_contrast;
	GtkListStore	*store_correction;
	GtkCellRenderer	*render;
	GtkTreeIter		iter;

	gtk_init(&argc, &argv);

	correction_k = 8 * log(2) / 255.0;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),
		"Попиксельная обработка изображений");
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING);
	frame_image = gtk_frame_new(NULL);
	image = gtk_image_new();
	button_load_image = gtk_button_new_with_label("Загрузить картинку");
	entry_image_filename = gtk_entry_new();
	label_brightness = gtk_label_new("Яркость: (%)");
	label_contrast = gtk_label_new("Контрастность: (%)");
	scale_brightness = gtk_scale_new_with_range(
		GTK_ORIENTATION_HORIZONTAL, -50, 50, 1);
	gtk_range_set_value(GTK_RANGE(scale_brightness), 0);
	scale_contrast = gtk_scale_new_with_range(
		GTK_ORIENTATION_HORIZONTAL, -50, 50, 1);
	gtk_range_set_value(GTK_RANGE(scale_contrast), 0);
	checkbox_grayscale = gtk_check_button_new_with_label("Сделать чёрно-белым");
	button_change_image = gtk_button_new_with_label("Изменить картинку");
	button_save_image = gtk_button_new_with_label("Сохранить в файл");

	store_correction = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	for (int i = 0; i < corrections_size; i++)
	{
		gtk_list_store_append(store_correction, &iter);
		gtk_list_store_set(store_correction, &iter, 0, corrections[i].name,
			1, corrections[i].fn, -1);
	}

	combo_box_correction =
		gtk_combo_box_new_with_model(GTK_TREE_MODEL(store_correction));
	g_object_unref(store_correction);

	render = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo_box_correction),
		render, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo_box_correction),
		render,
		"text", 0,
		NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box_correction), 0);

	gtk_widget_set_valign(button_change_image, GTK_ALIGN_END);
	gtk_widget_set_valign(button_save_image, GTK_ALIGN_END);

	load_image("/home/chip/Pictures/beach.png");

	gtk_container_set_border_width(GTK_CONTAINER(window), SPACING);
	gtk_widget_set_sensitive(entry_image_filename, FALSE);
	gtk_container_add(GTK_CONTAINER(frame_image), image);
	gtk_box_pack_start(GTK_BOX(vbox), button_load_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry_image_filename, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), checkbox_grayscale, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_brightness, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), scale_brightness, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), label_contrast, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), scale_contrast, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), combo_box_correction, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), button_change_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), button_save_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), frame_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	g_signal_connect(G_OBJECT(button_load_image), "clicked",
		G_CALLBACK(on_button_load_image_clicked), NULL);
	g_signal_connect(G_OBJECT(button_change_image), "clicked",
		G_CALLBACK(on_button_change_image_clicked), NULL);
	g_signal_connect(G_OBJECT(button_save_image), "clicked",
		G_CALLBACK(on_button_save_image_clicked), NULL);
	g_signal_connect(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);
	gtk_main();
}

static void on_button_load_image_clicked(GtkWidget *button, gpointer data)
{
	GtkWidget	*dialog;
	gchar		*filename;

	dialog = gtk_file_chooser_dialog_new(NULL, GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"Cancel", GTK_RESPONSE_CANCEL,
		"Open", GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
		g_get_home_dir());

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		load_image(filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void on_button_change_image_clicked(GtkWidget *button, gpointer data)
{
	change_image();
}

static void on_button_save_image_clicked(GtkWidget *button, gpointer data)
{
	GtkWidget	*dialog;
	gchar		*filename;

	dialog = gtk_file_chooser_dialog_new(NULL, GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		"Cancel", GTK_RESPONSE_CANCEL,
		"Save", GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
		TRUE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
		g_get_home_dir());
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
		"fotosharp.png");

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		save_image(image, filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void load_image(const gchar *filename)
{
	gtk_image_set_from_file(GTK_IMAGE(image), filename);
	gtk_entry_set_text(GTK_ENTRY(entry_image_filename), filename);
	if (last_load_image_pixbuf)
		g_object_unref(last_load_image_pixbuf);
	last_load_image_pixbuf = gdk_pixbuf_copy(
		gtk_image_get_pixbuf(GTK_IMAGE(image)));
}

static void save_image(GtkWidget *img, const gchar *filename)
{
	GdkPixbuf	*pixbuf;

	pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(img));
	gdk_pixbuf_save(pixbuf, filename, "png", NULL, NULL);
}

static void change_image(void)
{
	struct ch_color	image_color;
	struct ch_color	result_color;
	GdkPixbuf		*image_pixbuf;
	GdkPixbuf		*result_pixbuf;
	GtkTreeModel	*model;
	gpointer		corr_func;
	GtkTreeIter		iter;
	int				row;
	int				col;
	int				width;
	int				height;

	image_pixbuf = last_load_image_pixbuf;
	result_pixbuf = gdk_pixbuf_copy(image_pixbuf);
	width = gdk_pixbuf_get_width(image_pixbuf);
	height = gdk_pixbuf_get_height(image_pixbuf);
	gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo_box_correction), &iter);
	model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box_correction));
	gtk_tree_model_get(model, &iter, 1, &corr_func, -1);
	for (row = 0; row < height; row++)
		for (col = 0; col < width; col++)
		{
			get_pixel(image_pixbuf, col, row, &image_color);
			result_color = image_color;
			if (gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(checkbox_grayscale)))
			{
					change_grayscale(&image_color, &result_color);
			}
			if (gtk_range_get_value(GTK_RANGE(scale_brightness)) != 0)
			{
					change_brightness(&result_color, &result_color,
						gtk_range_get_value(GTK_RANGE(scale_brightness)));
			}
			if (gtk_range_get_value(GTK_RANGE(scale_contrast)) != 0)
			{
					change_contrast(&result_color, &result_color,
						gtk_range_get_value(GTK_RANGE(scale_contrast)));
			}
			if (corr_func != correction_line)
				change_correction(&result_color, &result_color, corr_func);
			set_pixel(result_pixbuf, col, row, &result_color);
		}
	gtk_image_set_from_pixbuf(GTK_IMAGE(image), result_pixbuf);
	g_object_unref(result_pixbuf);
}

static void change_grayscale(struct ch_color *in, struct ch_color *out)
{
	int		avg;

	avg = (in->red + in->green + in->blue) / 3;
	out->red = avg;
	out->green = avg;
	out->blue = avg;
}

static void change_brightness(struct ch_color *in, struct ch_color *out,
	double brightness)
{
	out->red = change_comp_brightness(in->red, brightness);
	out->green = change_comp_brightness(in->green, brightness);
	out->blue = change_comp_brightness(in->blue, brightness);
}

static void change_contrast(struct ch_color *in, struct ch_color *out,
	double contrast)
{
	out->red = change_comp_contrast(in->red, contrast);
	out->green = change_comp_contrast(in->green, contrast);
	out->blue = change_comp_contrast(in->blue, contrast);
}

static void change_correction(struct ch_color *in, struct ch_color *out,
	int (*corr_func)(int))
{
	out->red = corr_func(in->red);
	out->green = corr_func(in->green);
	out->blue = corr_func(in->blue);
}

static int	change_comp_brightness(int component, double brightness)
{
	int result;

	result = component + (brightness / 100) * 128;
	if (result > 255)
		result = 255;
	else if (result < 0)
		result = 0;
	return result;
}

static int	change_comp_contrast(int component, double contrast)
{
	int result;

	if (contrast < 0)
		result = component + (contrast / 100) * (component - 128);
	else
		result = 128 + (component - 128) / (1 - contrast / 100);
	if (result > 255)
		result = 255;
	else if (result < 0)
		result = 0;
	return result;
}

static int correction_line(int component)
{
	return component;
}

static int correction_sin(int c)
{
	int		result;

	result = (255 / 2.0) * sin(G_PI / 255.0 * c - G_PI_2) + (255 / 2.0);
	if (result < 0)
		result = 0;
	if (result > 255)
		result = 255;
	return result;
}

static int correction_exp(int component)
{
	int		result;

	result = exp(correction_k * component) - 1;
	if (result < 0)
		result = 0;
	if (result > 255)
		result = 255;
	return result;
}

static int correction_log(int component)
{
	int		result;

	result = log(component + 1) / correction_k;
	if (result < 0)
		result = 0;
	if (result > 255)
		result = 255;
	return result;
}

static void get_pixel(GdkPixbuf *pixbuf, int x, int y, struct ch_color *color)
{
	guchar	*pixels;
	guchar	*p;
	int		width;
	int		height;
	int		rowstride;
	int		n_channels;

	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);

	if (x < 0 || x > width || y < 0 || y > height)
		return;

	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	p = pixels + y * rowstride + x * n_channels;
	color->red		= p[0];
	color->green	= p[1];
	color->blue 	= p[2];
	if (gdk_pixbuf_get_has_alpha(pixbuf))
		color->alpha = p[3];
}

static void set_pixel(GdkPixbuf *pixbuf, int x, int y, struct ch_color *color)
{
	guchar	*pixels;
	guchar	*p;
	int		width;
	int		height;
	int		rowstride;
	int		n_channels;

	n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);

	if (x < 0 || x > width || y < 0 || y > height)
		return;

	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);

	p = pixels + y * rowstride + x * n_channels;
	p[0] = color->red;
	p[1] = color->green;
	p[2] = color->blue;
	if (gdk_pixbuf_get_has_alpha(pixbuf))
		p[3] = color->alpha;
}
