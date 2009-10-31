/* tabu-window.c */

#include "tabu-window.h"

#define TABU_ICON_CLOSE DATA_DIR "/tabu/close.png"
#define TABU_ICON_CLOSEOVER DATA_DIR "/tabu/close-over.png"
#define TABU_ICON_RESTORE DATA_DIR "/tabu/restore.png"
#define TABU_ICON_RESTOREOVER DATA_DIR "/tabu/restore-over.png"

static GtkType tabu_window_type = 0;

static void tabu_window_class_init ();
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
      NULL
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
  g_print ("\ntabu-window-add");
  gtk_box_pack_start ( 
    GTK_BOX (window->main_box), 
    GTK_WIDGET (widget), 
    expand, 
    fill, 
    padding);

  gtk_widget_show_all (GTK_WIDGET (window->main_box));
}

static void
on_alpha_screen_changed (GtkWidget* widget)
{                       
	GdkScreen*   screen   = gtk_widget_get_screen (widget);
	GdkColormap* colormap = gdk_screen_get_rgba_colormap (screen);
      
	if (!colormap)
		colormap = gdk_screen_get_rgb_colormap (screen);

	gtk_widget_set_colormap (widget, colormap);
}

gboolean 
on_drawingarea_title_button_press_event ( 
  GtkWidget *widget, 
  GdkEventButton *event)
{
    if ( event->type == GDK_BUTTON_PRESS )
    {
      if ( event->button == 1 )
      {        
        gtk_window_begin_move_drag ( 
          GTK_WINDOW ( gtk_widget_get_toplevel ( widget ) ),
          event->button,
          event->x_root,
          event->y_root,
          event->time );   

        if ((event->y > 7) && (event->y < 28))
        {
          if ((event->x > (widget->allocation.width - 30)) 
                    && (event->x < (widget->allocation.width - 10)))
          {
            /*tabu_player_quit ();                        */
            gtk_main_quit ();
          }          
          else if ((event->x > (10)) 
                    && (event->x < (34)))
          { 
            gtk_window_iconify (GTK_WINDOW (widget));
            TABU_WINDOW (widget)->is_minimized = TRUE;
          }
        }
      }
    }

    return FALSE;
}

gboolean
on_drawingarea_title_motion_notify_event ( 
  GtkWidget *widget, 
  GdkEventMotion *event, 
  gpointer data )
{
  TABU_WINDOW (data)->pointer_over_restore_icon = FALSE;
  TABU_WINDOW (data)->pointer_over_close_icon = FALSE;
  
  if ((event->y > 7) && (event->y < 28))
  {
    if ((event->x > 10) 
        && (event->x < 35))
    {
      TABU_WINDOW (data)->pointer_over_restore_icon = TRUE;
    }
    else if ((event->x > (widget->allocation.width - 30)) 
              && (event->x < (widget->allocation.width - 10)))
    {
      TABU_WINDOW (data)->pointer_over_close_icon = TRUE;
    }
  }
  gtk_widget_queue_draw (widget);

  return FALSE;
}


gboolean
on_drawingarea_title_expose_event ( GtkWidget *widget )
{
  cairo_t *cr;
  cairo_pattern_t *pat;
  double x0;
  double y0;
  double y1;
  double x1;
  double radio;

  x0 = widget->allocation.x;
  y0 = widget->allocation.y;
  y1 = y0 + widget->allocation.height;
  x1 = x0 + widget->allocation.width;
  radio = 10;

  /* fondo transparente para el title */
  cr = gdk_cairo_create ( widget->window );
  cairo_rectangle (cr, 0, 0, x1, y1);
  cairo_set_source_rgba ( cr, 1.0f, 1.0f, 1.0f, 0.0f );
	cairo_set_operator ( cr, CAIRO_OPERATOR_SOURCE );
	cairo_paint ( cr );
  cairo_destroy ( cr );

  /* path del title */
  cr = gdk_cairo_create ( widget->window );
  cairo_arc ( cr, x0 + radio, y0 + radio, radio, 180 * ( 3.14 / 180 ), 270 * ( 3.14 / 180 ) );
  cairo_line_to ( cr, x1 - radio, y0 );
  cairo_arc ( cr, x1 - radio, y0 + radio, radio, 270 * ( 3.14 / 180 ) , 0 );
  cairo_line_to ( cr, x1, y1 );
  cairo_line_to ( cr, x0, y1 );
  cairo_close_path ( cr );

  /* pintamos el path del title con un patron lineal horizontal */
	cairo_set_operator ( cr, CAIRO_OPERATOR_SOURCE );
  pat = cairo_pattern_create_linear ( x1/2, y0,  x1/2, y1 );
  cairo_pattern_add_color_stop_rgba ( pat, 1, 0.0f, 0.0f, 0.0f, 0.9f );
  cairo_pattern_add_color_stop_rgba ( pat, 0, 0.2f, 0.2f, 0.2f, 0.9f );
  cairo_set_source ( cr, pat );
	cairo_fill ( cr );
  cairo_pattern_destroy ( pat );

  /* escribimos el text en el title. */
  cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
  cairo_set_font_size ( cr, 11.0 );
  cairo_set_source_rgb ( cr, 1.0, 1.0, 1.0 );
  cairo_move_to ( cr, x1/2 - 40, y1/2 + 5 );
  cairo_show_text ( cr, "Tabu - 2.0" );
  cairo_stroke ( cr );
  
  /* dibujamos el Close Button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  cairo_set_line_width ( cr, 1.0 );
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
  cairo_move_to ( cr, x1 - 19, y1/2 - 4 );
  cairo_line_to ( cr, x1 - 11, y1/2 + 4 );
  cairo_move_to (cr, x1 - 19, y1/2 + 4);
  cairo_line_to ( cr, x1 - 11, y1/2 - 4 );
  cairo_stroke ( cr ); 

  /* dibujamos el Restore Button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  cairo_set_line_width ( cr, 1.0 );
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
  cairo_arc (cr, x0 + 15, y1/2, 4, 0, 6.28);
  cairo_stroke ( cr ); 

  cairo_destroy ( cr );
        
  return FALSE;
}

gboolean
icon_activate_cb (GtkWidget *widget)
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
tabu_window_class_init ()
{
}

static void
tabu_window_init (TabuWindow *self)
{
  self->pointer_over_restore_icon = FALSE;
  self->pointer_over_close_icon = FALSE;
  self->is_minimized = FALSE;

  gtk_window_set_default_size (GTK_WINDOW (self), 300, 450); 
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER);  
  gtk_widget_set_app_paintable (GTK_WIDGET (self), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);

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

  self->drawingarea_title = gtk_drawing_area_new ();
  gtk_widget_set_size_request (GTK_WIDGET (self->drawingarea_title), -1, 25);

  gtk_widget_add_events (
    GTK_WIDGET (self->drawingarea_title), 
    GDK_BUTTON_PRESS_MASK);

  gtk_widget_add_events (
    GTK_WIDGET (self->drawingarea_title), 
    GDK_POINTER_MOTION_MASK);

  g_signal_connect ( 
    self->drawingarea_title, 
    "expose-event", 
    G_CALLBACK (on_drawingarea_title_expose_event), 
    self);

  g_signal_connect ( 
    self->drawingarea_title, 
    "motion-notify-event", 
    G_CALLBACK (on_drawingarea_title_motion_notify_event), 
    self);

  g_signal_connect_swapped ( 
    self->drawingarea_title, 
    "button-press-event", 
    G_CALLBACK (on_drawingarea_title_button_press_event), 
    self);

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

  on_alpha_screen_changed (GTK_WIDGET (self));
  gtk_widget_realize (GTK_WIDGET (self));
	gdk_window_set_back_pixmap (GTK_WIDGET (self)->window, NULL, FALSE);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (self), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (self), TRUE);  

  gtk_widget_show_all (GTK_WIDGET (self));
}

GtkWidget *
tabu_window_new (void)
{
  return (GTK_WIDGET (g_object_new (tabu_window_get_type(), NULL)));
}
