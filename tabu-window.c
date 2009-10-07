/* tabu-window.c */

#include "tabu-window.h"

#define TABU_ICON_CLOSE DATA_DIR "/tabu/close.png"
#define TABU_ICON_CLOSEOVER DATA_DIR "/tabu/close-over.png"
#define TABU_ICON_RESTORE DATA_DIR "/tabu/restore.png"
#define TABU_ICON_RESTOREOVER DATA_DIR "/tabu/restore-over.png"

static GtkType tabu_window_type = 0;

static void tabu_window_class_init (TabuWindowClass *klass);
static void tabu_window_init (TabuWindow *self);

GType
tabu_window_get_type (void)
{
	/* if the type is not registered, then register it */
	if (!tabu_window_type)
	{
		static const GTypeInfo tabu_window_type_info =
		{
			sizeof (TabuWindowClass),
			NULL,
			NULL,
			(GClassInitFunc) tabu_window_class_init,
			NULL,
			NULL,
			sizeof (TabuWindow),
			0,
			(GInstanceInitFunc) tabu_window_init,
		};
		tabu_window_type = g_type_register_static (
      GTK_TYPE_WINDOW, 
      "TabuWindow", 
      &tabu_window_type_info, 
      0);
	}

	return tabu_window_type;
}

void
tabu_window_add ( TabuWindow *window, 
  GtkWidget *widget, 
  gboolean expand, 
  gboolean fill, 
  guint padding )
{
  gtk_box_pack_start ( 
    GTK_BOX (window->main_box), 
    GTK_WIDGET (widget), 
    expand, 
    fill, 
    padding);

  gtk_widget_show_all (GTK_WIDGET (window->main_box));
}

gboolean
icon_activate_cb (GtkWidget *widget, gpointer data)
{
  if (TABU_WINDOW (widget)->is_minimized)
  {
    gtk_window_deiconify (GTK_WINDOW (widget));
    TABU_WINDOW (widget)->is_minimized = FALSE;
  }
  else
  {
    gtk_window_iconify (GTK_WINDOW (widget));
    TABU_WINDOW (widget)->is_minimized = TRUE;
  }
  return TRUE;
}

static void
tabu_window_class_init (TabuWindowClass *klass)
{
}

static void
tabu_window_init (TabuWindow *self)
{
  self->pointer_over_restore_icon = FALSE;
  self->pointer_over_close_icon = FALSE;
  self->is_minimized = FALSE;

  gtk_window_set_default_size (GTK_WINDOW (self), 450, 300); 
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);  

  self->icon_filename = DATA_DIR "/tabu/tabu.png";

	gtk_window_set_icon_from_file (
    GTK_WINDOW (self), 
    self->icon_filename, 
    NULL);	

  GtkStatusIcon *icon;
  icon = gtk_status_icon_new_from_file (self->icon_filename);
  gtk_status_icon_set_tooltip (icon, "Tabu 2.0");
  gtk_status_icon_set_visible (icon, TRUE);

  g_signal_connect (self, "destroy", G_CALLBACK (gtk_main_quit), NULL);  

  g_signal_connect_swapped (
    icon, 
    "activate",   
    G_CALLBACK (icon_activate_cb), 
    self);

  self->main_box = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (
    GTK_BOX (self->main_box), 
    GTK_WIDGET (self->drawingarea_title), 
    FALSE, 
    FALSE, 
    0);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->main_box));
  gtk_widget_realize (GTK_WIDGET (self));
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (self), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (self), TRUE);  

  gtk_widget_show_all (GTK_WIDGET (self));
}

GtkWidget *
tabu_window_new (void)
{
  return (GTK_WIDGET (g_object_new (tabu_window_get_type(), NULL)));
}
