#include <gtk/gtk.h>

#define	SPACING	5

static void on_button_load_image_clicked(GtkWidget *button, gpointer data);
static void load_image(const gchar *filename);

static GtkWidget	*window;
static GtkWidget	*image;
static GtkWidget	*entry_image_filename;

int main(int argc, char *argv[])
{
	GtkWidget	*vbox;
	GtkWidget	*hbox;
	GtkWidget	*frame_image;
	GtkWidget	*button_load_image;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING);
	frame_image = gtk_frame_new(NULL);
	image = gtk_image_new();
	button_load_image = gtk_button_new_with_label("Загрузить картинку");
	entry_image_filename = gtk_entry_new();

	load_image("/home/chip/Pictures/beach.png");

	gtk_container_set_border_width(GTK_CONTAINER(window), SPACING);
	gtk_widget_set_sensitive(entry_image_filename, FALSE);
	gtk_container_add(GTK_CONTAINER(frame_image), image);
	gtk_box_pack_start(GTK_BOX(vbox), button_load_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry_image_filename, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), frame_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	g_signal_connect(G_OBJECT(button_load_image), "clicked",
		G_CALLBACK(on_button_load_image_clicked), NULL);
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

	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		load_image(filename);
		g_free(filename);
	}

	gtk_widget_destroy (dialog);
}

static void load_image(const gchar *filename)
{
	gtk_image_set_from_file(GTK_IMAGE(image), filename);
	gtk_entry_set_text(GTK_ENTRY(entry_image_filename), filename);
}
