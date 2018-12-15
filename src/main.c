#include <gtk/gtk.h>

#define	SPACING	5

int main(int argc, char *argv[])
{
	GtkWidget	*window;
	GtkWidget	*vbox;
	GtkWidget	*hbox;
	GtkWidget	*frame_image;
	GtkWidget	*image;
	GtkWidget	*button_load_image;
	GtkWidget	*entry_image_filename;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, SPACING);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SPACING);
	frame_image = gtk_frame_new(NULL);
	image = gtk_image_new();
	button_load_image = gtk_button_new_with_label("Загрузить картинку");
	entry_image_filename = gtk_entry_new();

	gtk_container_set_border_width(GTK_CONTAINER(window), SPACING);
	gtk_widget_set_sensitive(entry_image_filename, FALSE);
	gtk_container_add(GTK_CONTAINER(frame_image), image);
	gtk_box_pack_start(GTK_BOX(vbox), button_load_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), entry_image_filename, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), frame_image, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	g_signal_connect(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_show_all(window);
	gtk_main();
}
