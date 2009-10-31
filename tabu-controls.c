/* tabu-controls.c */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gst/gst.h>
#include <string.h>

#include "tabu-controls.h"
#include "tabu-backend.h"
#include "tabu-playlist.h"
#include "tabu.h"

#define TABU_CONTROLS_HEIGTH 35
#define TABU_OPTIONS_HEIGTH 30

#define TABU_ICON_DEL DATA_DIR "/tabu/del.png"
#define TABU_ICON_ADD DATA_DIR "/tabu/add.png"


static GtkType tabu_controls_type = 0;

static void tabu_controls_class_init ();
static void tabu_controls_init (TabuControls *self);
void show_file_chooser_dialog();
void tabu_controls_pack (TabuControls *widget);
void tabu_controls_unpack (TabuControls *widget);
void tabu_controls_unpack_adddialog (TabuControls *widget);

static gchar * _file_chooser_last_folder = NULL;
static gdouble current_volume = 0.5;

GType
tabu_controls_get_type (void)
{
	/* if the type is not registered, then register it */
	if (!tabu_controls_type)
	{
		static const GTypeInfo tabu_controls_type_info =
		{
			sizeof (TabuControlsClass),
			NULL,
			NULL,
			(GClassInitFunc) tabu_controls_class_init,
			NULL,
			NULL,
			sizeof (TabuControls),
			0,
			(GInstanceInitFunc) tabu_controls_init,
      NULL
		};
		tabu_controls_type = g_type_register_static (
      GTK_TYPE_DRAWING_AREA, 
      "TabuControls", 
      &tabu_controls_type_info, 
      0);
	}

	return tabu_controls_type;
}

gboolean
timer_function ( GtkWidget *widget )
{
  if (tabu_backend_is_playing())
    gtk_widget_queue_draw ( widget );

  return TRUE;
}

void recurse_dir(gchar *path)
{
  DIR *p_dir;
  struct dirent *p_file;
  gchar *deep = NULL;

  p_dir = opendir(path);
    
  while ((p_file = readdir(p_dir)) != 0)
  {
    if( p_file->d_type == DT_DIR && strcmp(p_file->d_name,".") != 0 && strcmp(p_file->d_name,"..") != 0)
    {
      deep = g_strconcat( path,"/",p_file->d_name, NULL);
      recurse_dir (deep);
      g_free (deep);
    }  
    else if(p_file->d_type == DT_REG)
    {            
  	  tabu_playlist_append ( 
        TABU_PLAYLIST (tabu_get_playlist ()), 
        g_uri_unescape_string (g_strconcat (path, "/", p_file->d_name, NULL), NULL ),
        g_uri_unescape_string (g_strconcat ("file://", path, "/", p_file->d_name, NULL ), NULL ) );
    }
  }
  closedir(p_dir);
}

void
show_folder_chooser_dialog()
{
  GtkWidget *selection;
  GtkFileFilter *filter;
  gint response;
  GSList *files;

  filter = gtk_file_filter_new ();
  gtk_file_filter_add_mime_type (filter, "application/ogg");
  gtk_file_filter_add_mime_type (filter, "audio/mpeg");


  selection = gtk_file_chooser_dialog_new ( 
    "Add Files", 
    NULL,
    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OK, GTK_RESPONSE_OK, 
    NULL );

  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(selection), filter);

  if ( _file_chooser_last_folder == NULL )
  {
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( selection ), g_get_home_dir ( ) );
  } 
  else
  {
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( selection ), _file_chooser_last_folder );            
  }

  gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( selection ), TRUE );
  gtk_widget_show_all ( selection );

  response = gtk_dialog_run ( GTK_DIALOG ( selection ) );

  if ( response == GTK_RESPONSE_OK )
  {
    g_free ( _file_chooser_last_folder );

    _file_chooser_last_folder = gtk_file_chooser_get_current_folder ( GTK_FILE_CHOOSER ( selection ) );

    files = gtk_file_chooser_get_filenames ( GTK_FILE_CHOOSER ( selection ) );

   
    while ( files != NULL )
    {
      recurse_dir (files->data);
			files = g_slist_next ( files );
    }

    g_slist_free ( files );
  }

  gtk_widget_destroy ( selection );
}


void
show_file_chooser_dialog()
{
  GtkWidget *selection;
  GtkFileFilter *filter;
  gint response;
  GSList *files;

  filter = gtk_file_filter_new ();
  gtk_file_filter_add_mime_type (filter, "application/ogg");
  gtk_file_filter_add_mime_type (filter, "audio/mpeg");

  selection = gtk_file_chooser_dialog_new ( 
    "Add Files", 
    NULL,
    GTK_FILE_CHOOSER_ACTION_OPEN,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OK, GTK_RESPONSE_OK, 
    NULL );

  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (selection), filter);

  if ( _file_chooser_last_folder == NULL )
  {
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( selection ), g_get_home_dir ( ) );
  } 
  else
  {
    gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( selection ), _file_chooser_last_folder );            
  }

  gtk_file_chooser_set_select_multiple ( GTK_FILE_CHOOSER ( selection ), TRUE );
  gtk_widget_show_all ( selection );

  response = gtk_dialog_run ( GTK_DIALOG ( selection ) );

  if ( response == GTK_RESPONSE_OK )
  {
    g_free ( _file_chooser_last_folder );

    _file_chooser_last_folder = gtk_file_chooser_get_current_folder ( GTK_FILE_CHOOSER ( selection ) );

    files = gtk_file_chooser_get_filenames ( GTK_FILE_CHOOSER ( selection ) );

    while ( files != NULL )
    {
  	  tabu_playlist_append ( 
        TABU_PLAYLIST (tabu_get_playlist ()), 
        g_uri_unescape_string ( files->data, NULL ),
        g_strconcat ( "file://", g_uri_unescape_string ( files->data, NULL ), NULL ) );

			files = g_slist_next ( files );
    }

    g_slist_free ( files );
  }

  gtk_widget_destroy ( selection );
}

static gboolean
controls_expose_event (GtkWidget *widget)
{
  cairo_t *cr;
  cairo_pattern_t *pat;
  cairo_surface_t *image;
  double x0 = 0;
  double y0 = 0;
  double y1 = 0;
  double x1 = 0;
  double radio = 10;

  int controlH = TABU_CONTROLS_HEIGTH;
  int optionsH = TABU_OPTIONS_HEIGTH;
  if (!TABU_CONTROLS (widget)->pbar_visible 
      && !TABU_CONTROLS (widget)->volume_clicked)
  {
    if (!TABU_CONTROLS (widget)->add_clicked
        && !TABU_CONTROLS (widget)->del_clicked)
    {
      optionsH = 0;
    }
  }
  
  double centerY1 = ((controlH)/2) + optionsH;

  /* fondo transparente para el title */
  cr = gdk_cairo_create ( widget->window );
  cairo_rectangle ( 
    cr, widget->allocation.x, 
    widget->allocation.y, 
    widget->allocation.width, 
    widget->allocation.height );

  cairo_set_source_rgba ( cr, 1.0f, 1.0f, 1.0f, 0.0f );
	cairo_set_operator ( cr, CAIRO_OPERATOR_SOURCE );
	cairo_paint ( cr );
  cairo_destroy ( cr );

  /* path del title */
  cr = gdk_cairo_create ( widget->window );
  y1 += widget->allocation.height;
  x1 += widget->allocation.width;

  cairo_arc ( cr, x1 - radio, y1 - radio, radio, 0, 1.57 );
  cairo_line_to ( cr, x0 - radio, y1 );
  cairo_arc ( cr, x0 + radio, y1 - radio, radio, 1.5 , 3.14 );
  cairo_line_to ( cr, x0, y0 );
  cairo_line_to ( cr, x1, y0 );
  cairo_close_path ( cr );

  /* pintamos el path del title con un patron lineal horizontal */
	cairo_set_operator ( cr, CAIRO_OPERATOR_SOURCE );
  pat = cairo_pattern_create_linear ( x1/2, y0,  x1/2, y1 );
  cairo_pattern_add_color_stop_rgba ( pat, 0, 0.0f, 0.0f, 0.0f, 0.9f );
  cairo_pattern_add_color_stop_rgba ( pat, 1, 0.0f, 0.0f, 0.0f, 0.9f );
  cairo_set_source ( cr, pat );
	cairo_fill ( cr );
  cairo_pattern_destroy ( pat );

  /* pintamos el fondo de las opciones */
  if (TABU_CONTROLS (widget)->add_clicked
      || TABU_CONTROLS (widget)->del_clicked
      || TABU_CONTROLS (widget)->volume_clicked)
  {
    cairo_set_operator ( cr, CAIRO_OPERATOR_OVER );
    cairo_move_to (cr, x0, y0);
    cairo_line_to (cr, x1, y0);
    cairo_line_to (cr, x1, optionsH);
    cairo_line_to (cr, x0, optionsH);
    cairo_close_path (cr);
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
    cairo_fill (cr);
    cairo_stroke (cr);
  }
  
 /* dibujamos el Play / Pause Button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
  if (tabu_backend_is_playing())
  {
    cairo_set_line_width ( cr, 7.0 );
    cairo_move_to (cr, x1/2 - 5, centerY1 - 11);
    cairo_line_to (cr, x1/2 - 5, centerY1 + 11);
    cairo_move_to (cr, x1/2 + 5, centerY1 - 11);
    cairo_line_to (cr, x1/2 + 5, centerY1 + 11);
  }
  else
  {
    cairo_set_line_width ( cr, 9.0 );
    cairo_move_to ( cr, x1/2 - 6, centerY1 - 11);
    cairo_line_to ( cr, x1/2 + 6, centerY1);
    cairo_line_to ( cr, x1/2 - 6, centerY1 + 11);
  }
  cairo_stroke ( cr );

/* dibujamos el Prev Button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  cairo_set_line_width ( cr, 4.0 );
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.6f );
  cairo_move_to ( cr, x1/2 - 34, centerY1 + 8);
  cairo_line_to ( cr, x1/2 - 44, centerY1);
  cairo_line_to ( cr, x1/2 - 34, centerY1 - 8);
  cairo_move_to ( cr, x1/2 - 44, centerY1);
  cairo_line_to ( cr, x1/2 - 28, centerY1);
  cairo_stroke ( cr );
 
  /* dibujamos el Next Button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  cairo_set_line_width ( cr, 4.0 );
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.6f );
  cairo_move_to ( cr, x1/2 + 34, centerY1 + 8);
  cairo_line_to ( cr, x1/2 + 44, centerY1);
  cairo_line_to ( cr, x1/2 + 34, centerY1 - 8);
  cairo_move_to ( cr, x1/2 + 44, centerY1);
  cairo_line_to ( cr, x1/2 + 28, centerY1);
  cairo_stroke ( cr );

  /* dibujamos el Add Button */  
  if (TABU_CONTROLS (widget)->add_clicked)
  {
    cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
    cairo_move_to (cr, 7, optionsH);
    cairo_line_to (cr, 30, optionsH);
    cairo_line_to (cr, 30, y1-12);
    cairo_arc (cr, 25, y1-12, 5, 0, 1.57);
    cairo_line_to (cr, 12, y1-7);
    cairo_arc (cr, 12, y1-12, 5, 1.57, 3.14);
    cairo_close_path (cr);
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
    cairo_fill (cr);
    cairo_stroke (cr);

    cairo_set_source_rgb ( cr, 0.0, 0.0, 0.0 );
    if (!TABU_CONTROLS (widget)->addfile_over)
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
      cairo_set_font_size ( cr, 12.0 );
    }
    else
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
      cairo_set_font_size ( cr, 14.0 );      
    }
    cairo_move_to ( cr, 10, optionsH/2 + 5 );
    cairo_show_text ( cr, "+Archivo" );
    cairo_stroke (cr);

    if (!TABU_CONTROLS (widget)->addfolder_over)
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
      cairo_set_font_size ( cr, 12.0 );
    }
    else
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
      cairo_set_font_size ( cr, 14.0 );      
    }
    cairo_move_to ( cr, 110, optionsH/2 + 5 );
    cairo_show_text ( cr, "+Carpeta" );
    cairo_stroke ( cr );
  }

  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
  image = cairo_image_surface_create_from_png ( TABU_ICON_ADD ); 
  cairo_set_source_surface ( cr, image, x0 + 10, centerY1 - 11);
  cairo_paint ( cr );
  cairo_stroke ( cr );
  cairo_surface_destroy ( image ); 
  
  
  /* dibujamos el Clear Button */
  
  if (TABU_CONTROLS (widget)->del_clicked)
  {
    cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
    cairo_move_to (cr, 32, optionsH);
    cairo_line_to (cr, 55, optionsH);
    cairo_line_to (cr, 55, y1-12);
    cairo_arc (cr, 50, y1-12, 5, 0, 1.57);
    cairo_line_to (cr, 37, y1-7);
    cairo_arc (cr, 37, y1-12, 5, 1.57, 3.14);
    cairo_close_path (cr);
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
    cairo_fill (cr);
    cairo_stroke (cr);

    cairo_set_source_rgb ( cr, 0.0, 0.0, 0.0 );
    if (!TABU_CONTROLS (widget)->clear_all_over)
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
      cairo_set_font_size ( cr, 12.0 );
    }
    else
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
      cairo_set_font_size ( cr, 14.0 );      
    }
    cairo_move_to ( cr, 10, optionsH/2 + 5 );
    cairo_show_text ( cr, "-Todo" );
    cairo_stroke (cr);

    if (!TABU_CONTROLS (widget)->clear_selected_over)
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
      cairo_set_font_size ( cr, 12.0 );
    }
    else
    {
      cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
      cairo_set_font_size ( cr, 14.0 );      
    }
    cairo_move_to ( cr, 110, optionsH/2 + 5 );
    cairo_show_text ( cr, "-Seleccion" );
    cairo_stroke ( cr );
  }
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER );
  image = cairo_image_surface_create_from_png ( TABU_ICON_DEL );  
  cairo_set_source_surface ( cr, image, x0 + 35, centerY1 - 11);
  cairo_paint ( cr );
  cairo_stroke ( cr );
  cairo_surface_destroy ( image );

  /* dibujamos el progress bar button */
  cairo_set_operator ( cr, CAIRO_OPERATOR_OVER );
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );  
  if (TABU_CONTROLS (widget)->pbar_visible)
  {
    cairo_set_line_width (cr, 2.0);      
  }
  else
  {
    cairo_set_line_width (cr, 1.0);      
  }
  cairo_rectangle (cr, x1 - 56, centerY1 - 3, 14, 5);
  cairo_stroke (cr);

  /* obtenemos y formateamos la posicion/duracion de la cancion */
  gchar time_buffer[25];
  gchar pos_buffer[25];
  
  g_snprintf(pos_buffer, 24, 
    "%u:%02u.%02u", 
    GST_TIME_ARGS ( tabu_backend_get_current_position()));

  g_snprintf(time_buffer, 24, 
    "%u:%02u.%02u", 
    GST_TIME_ARGS ( tabu_backend_get_current_length()));

  /* dibujamos la progress bar */
  if ((!TABU_CONTROLS (widget)->add_clicked)
      && (TABU_CONTROLS (widget)->pbar_visible)
      && tabu_backend_is_playing())
  {
    cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 1.0f );
    cairo_set_font_size ( cr, 11.0 );      
    cairo_move_to ( cr, x1/2 - 60, y0 + 15 );
    cairo_show_text ( cr, g_strconcat(pos_buffer, " / ", time_buffer, NULL) );    
    cairo_stroke ( cr );

    gint64 pos = tabu_backend_get_current_position ();
    gint64 len = tabu_backend_get_current_length ();

    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 1.0f );
    cairo_set_line_width (cr, 7.0);
    cairo_move_to (cr, 10, optionsH - 8);
    cairo_line_to (cr, 10+((x1-20)*pos)/len, optionsH - 8);
    cairo_stroke (cr);
  }  

  /* dibujamos el volume button */
  cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );  
  if (TABU_CONTROLS (widget)->volume_clicked)
  {
    cairo_set_operator ( cr, CAIRO_OPERATOR_OVER ); 
    cairo_move_to (cr, x1 - 32, optionsH);
    cairo_line_to (cr, x1 - 8, optionsH);
    cairo_line_to (cr, x1 - 8, y1-12);
    cairo_arc (cr, x1 - 13, y1-13, 5, 0, 1.57);
    cairo_line_to (cr, x1 - 12, y1-8);
    cairo_arc (cr, x1 - 27, y1-13, 5, 1.57, 3.14);
    cairo_close_path (cr);
    cairo_set_source_rgba ( cr, 1.0f, 0.5f, 0.0f, 0.9f );
    cairo_fill (cr);
    cairo_stroke (cr);

    /* get actual volume and show it */
    gchar volume_buffer[7];
    g_snprintf(volume_buffer, 6, "%u %%", (uint) (current_volume * 100));

    cairo_set_source_rgba ( cr, 0.0f, 0.0f, 0.0f, 0.9f );  

    cairo_select_font_face ( cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
    cairo_set_font_size ( cr, 11.0 );      
    cairo_move_to ( cr, x1/2 - 10, y0 + 10 );
    cairo_show_text ( cr, volume_buffer );    
    cairo_stroke ( cr );

    cairo_set_source_rgba ( cr, 0.0f, 0.0f, 0.0f, 1.0f );  
    cairo_set_line_width (cr, 1.0);
    cairo_move_to (cr, 40, optionsH - 9);
    cairo_line_to (cr, 40, optionsH - 15);
    cairo_line_to (cr, x1-40, optionsH - 15);
    cairo_line_to (cr, x1-40, optionsH - 9);
    cairo_line_to (cr, 40, optionsH - 9);
    cairo_stroke (cr);

    /* set volume */
    cairo_set_source_rgba ( cr, 0.0f, 0.0f, 0.0f, 0.9f );  
    cairo_set_line_width (cr, 7.0);
    cairo_move_to (cr, 40, optionsH - 12);
    cairo_line_to (cr, 40 + ((x1-80)*current_volume), optionsH - 12);
    cairo_stroke (cr);

    /* control + y - */
    cairo_set_line_width (cr, 3.0);
    cairo_move_to (cr, 20, optionsH/2);
    cairo_line_to (cr, 30, optionsH/2);
    cairo_stroke (cr);

    cairo_move_to (cr, (x1-40) + 10, optionsH/2);
    cairo_line_to (cr, (x1-40) + 20, optionsH/2);
    cairo_move_to (cr, (x1-40) + 15, optionsH/2 - 5);
    cairo_line_to (cr, (x1-40) + 15, optionsH/2 + 5);
    cairo_stroke (cr);

  }
  cairo_set_line_width (cr, 2.0);      
  cairo_arc (cr, x1 - 20, centerY1, 3, 0, 6.28);
  cairo_stroke (cr);
  cairo_arc (cr, x1 - 20, centerY1, 6, 3.5, 5.85);
  cairo_stroke (cr);
  cairo_arc (cr, x1 - 20, centerY1, 9, 3.5, 5.85);
  cairo_stroke (cr);

  cairo_destroy ( cr );
        
  return FALSE;
}

gboolean 
controls_button_release_event (
  GtkWidget *widget, 
  GdkEventButton *event, 
  gpointer data)
{   
  int offset = 0;
  if (TABU_CONTROLS (widget)->add_clicked 
      || TABU_CONTROLS (widget)->del_clicked)
  {
    offset = TABU_OPTIONS_HEIGTH;
  }

  if (event->y > 0)
  {
    if ((TABU_CONTROLS (widget)->add_clicked) && (event->y < offset))
    {      
      if ((event->x > 10) && (event->x < 80))
      { 
        show_file_chooser_dialog (widget, data);
      }
      else if ((event->x > 110) && (event->x < 180))
      {
        show_folder_chooser_dialog (widget, data); 
      }
    }
    else if ((TABU_CONTROLS (widget)->del_clicked) && (event->y < offset))
    {      
      if ((event->x > 10) && (event->x < 80))
      { 
        tabu_playlist_clear (tabu_get_playlist ());
      }
      else if ((event->x > 110) && (event->x < 180))
      {
        tabu_playlist_remove_selection (tabu_get_playlist ());
      }
    }
  }

  if (TABU_CONTROLS (widget)->pbar_visible || TABU_CONTROLS(widget)->volume_clicked)
  {
    gtk_widget_set_size_request (
      GTK_WIDGET (widget), -1, 
      TABU_CONTROLS_HEIGTH + TABU_OPTIONS_HEIGTH);            
  }
  else
  {
    gtk_widget_set_size_request (GTK_WIDGET (widget), -1, TABU_CONTROLS_HEIGTH);            
  }

  TABU_CONTROLS (widget)->add_clicked = FALSE;
  TABU_CONTROLS (widget)->del_clicked = FALSE;

  gtk_widget_queue_draw (widget);

  return (FALSE);
}

gboolean 
controls_button_press_event (
  GtkWidget *widget, 
  GdkEventButton *event)
{   
  TABU_CONTROLS (widget)->add_clicked = FALSE;
  TABU_CONTROLS (widget)->del_clicked = FALSE;  

  if (event->type == GDK_BUTTON_PRESS)
  {
    if (event->button == 1)
    {    
      gint offset = 0;
          
      if (widget->allocation.height > TABU_CONTROLS_HEIGTH)
      {
         offset = TABU_OPTIONS_HEIGTH;
      }
      

      /* ------------------------------------------------*/
      /* Volume button      (+)                          */
      /* ------------------------------------------------*/

      if ((TABU_CONTROLS (widget)->volume_clicked)
             &&(event->x > (widget->allocation.width - 30))
             && (event->x < (widget->allocation.width - 20))
             && (event->y > ((TABU_OPTIONS_HEIGTH / 2) - 5))
             && (event->y < ((TABU_OPTIONS_HEIGTH / 2) + 5)))
        {
          if (current_volume < 1)
          {
            current_volume = current_volume + 0.05;
          }
          tabu_backend_set_volume (current_volume);
        }        

      /* ------------------------------------------------*/
      /* Volume button (-)                               */
      /* ------------------------------------------------*/

      else if ((TABU_CONTROLS (widget)->volume_clicked)
             && (event->x > (20))
             && (event->x < (widget->allocation.width - 30))
             && (event->y > ((TABU_OPTIONS_HEIGTH / 2) - 5))
             && (event->y < ((TABU_OPTIONS_HEIGTH / 2) + 5)))
        {
          if (current_volume > 0)
          {
            current_volume = current_volume - 0.05;
          }
          tabu_backend_set_volume (current_volume);
        }        
        
      /* ------------------------------------------------*/
      /* Add Button                                      */
      /* ------------------------------------------------*/

      else if ((event->x > (10)) 
        && (event->x < (26))
        && (event->y > (offset + 7))
        && (event->y < (widget->allocation.height - 8)))
      {
        TABU_CONTROLS (widget)->add_clicked = TRUE;
        TABU_CONTROLS (widget)->volume_clicked = FALSE;
      }

      /* ------------------------------------------------*/
      /* Del Button                                      */
      /* ------------------------------------------------*/
    
      else if ((event->x > (35)) 
        && (event->x < (51))
        && (event->y > (offset + 11))
        && (event->y < (widget->allocation.height - 15)))
      {
        TABU_CONTROLS (widget)->del_clicked = TRUE;
        TABU_CONTROLS (widget)->volume_clicked = FALSE;
      }

      /* ------------------------------------------------*/
      /* location/duration progress bar toggle           */
      /* ------------------------------------------------*/

      else if ((event->x > 246) && (event->x < 254) 
                && (event->y < widget->allocation.height - 10) 
                && (event->y > widget->allocation.height - 22))
      {
        TABU_CONTROLS (widget)->pbar_visible = !TABU_CONTROLS (widget)->pbar_visible;
        TABU_CONTROLS (widget)->volume_clicked = FALSE;
      }            

      /* ------------------------------------------------*/
      /* Volume toggle                                   */
      /* ------------------------------------------------*/
      
      else if ((event->x > (widget->allocation.width - 30)) 
               && (event->x < (widget->allocation.width - 10))
               && (event->y > (widget->allocation.height - 25))
               && (event->y < (widget->allocation.height - 9)))
      {
        TABU_CONTROLS (widget)->volume_clicked = !TABU_CONTROLS (widget)->volume_clicked;
      }

      /* ------------------------------------------------*/
      /* Next song button                                */
      /* ------------------------------------------------*/

      else if ((event->x > ((widget->allocation.width / 2)+ 28))
               && (event->x < ((widget->allocation.width / 2) + 46))
               && (event->y > (offset + (TABU_CONTROLS_HEIGTH / 2) - 8))
               && (event->y < (offset + (TABU_CONTROLS_HEIGTH / 2) + 10)))
      {
        tabu_backend_play_uri (tabu_playlist_next (tabu_get_playlist (), FALSE));
      }

      /* ------------------------------------------------*/
      /* Play/Pause song button                          */
      /* ------------------------------------------------*/

      else if ((event->x > ((widget->allocation.width / 2) - 6))
               && (event->x < ((widget->allocation.width / 2) + 7))
               && (event->y > (offset + (TABU_CONTROLS_HEIGTH / 2) - 11))
               && (event->y < (offset + (TABU_CONTROLS_HEIGTH / 2) + 12)))
      {
        if (tabu_backend_is_playing())
        {
          tabu_backend_pause();
        }
        else if (tabu_backend_is_paused())
        {
          tabu_backend_play();
        }
        else
        {
          tabu_backend_play_uri (tabu_playlist_next (tabu_get_playlist (), FALSE));
        }
      }


      /* Redimensionar los controles si es necesario */    

      if ((TABU_CONTROLS (widget)->pbar_visible) 
           || (TABU_CONTROLS (widget)->volume_clicked) 
           || (TABU_CONTROLS (widget)->add_clicked)
           || (TABU_CONTROLS (widget)->del_clicked))
      {
        gtk_widget_set_size_request (GTK_WIDGET (widget), -1, TABU_CONTROLS_HEIGTH + TABU_OPTIONS_HEIGTH);
      }
      else
      {
        gtk_widget_set_size_request (GTK_WIDGET (widget), -1, TABU_CONTROLS_HEIGTH);
      }
      gtk_widget_queue_draw (widget);
    }
  }

  
  return FALSE;  
}

gboolean
controls_motion_notify_event (
  GtkWidget *widget, 
  GdkEventMotion *event)
{
  TABU_CONTROLS (widget)->addfile_over = FALSE;
  TABU_CONTROLS (widget)->addfolder_over = FALSE;      
  TABU_CONTROLS (widget)->clear_selected_over = FALSE;
  TABU_CONTROLS (widget)->clear_all_over = FALSE;      

  int offset = 0;
  if ((TABU_CONTROLS (widget)->add_clicked) 
       || (TABU_CONTROLS (widget)->del_clicked))
  {
    offset = TABU_OPTIONS_HEIGTH;
  }

  if (event->y > 0)
  {
    if ((TABU_CONTROLS (widget)->add_clicked) && (event->y < offset))
    {      
      if ((event->x > 10) && (event->x < 80))
      { 
        TABU_CONTROLS (widget)->addfile_over = TRUE;
      }
      else if ((event->x > 110) && (event->x < 180))
      {
        TABU_CONTROLS (widget)->addfolder_over = TRUE;        
      }
    }
    else if ((TABU_CONTROLS (widget)->del_clicked) && (event->y < offset))
    {
      if ((event->x > 10) && (event->x < 80))
      { 
        TABU_CONTROLS (widget)->clear_all_over = TRUE;
      }
      else if ((event->x > 110) && (event->x < 180))
      {
        TABU_CONTROLS (widget)->clear_selected_over = TRUE;        
      }
    }
  }

  gtk_widget_queue_draw (widget);

  return FALSE;
}


static void
tabu_controls_class_init ()
{
  
}


static void
tabu_controls_init (TabuControls *self)
{
  self->del_clicked = FALSE;
  self->clear_all_over = FALSE;
  self->clear_selected_over = FALSE; 

  self->add_clicked = FALSE;
  self->addfile_over = FALSE;
  self->addfolder_over = FALSE;

  self->pbar_visible = FALSE;
  self->vol_visible = FALSE;

  self->volume_clicked = FALSE;

  gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_PRESS_MASK);
  gtk_widget_add_events (GTK_WIDGET (self), GDK_BUTTON_RELEASE_MASK);
  gtk_widget_add_events (GTK_WIDGET (self), GDK_POINTER_MOTION_MASK); 

  g_signal_connect (
    self, 
    "expose-event", 
    G_CALLBACK (controls_expose_event), 
    NULL);

  g_signal_connect (
    self, 
    "button-press-event", 
    G_CALLBACK (controls_button_press_event), 
    NULL);

  g_signal_connect (
    self, 
    "motion-notify-event", 
    G_CALLBACK (controls_motion_notify_event), 
    NULL);

  g_signal_connect (
    self,
    "button-release-event",
    G_CALLBACK (controls_button_release_event),
    NULL);

  gtk_widget_set_size_request (GTK_WIDGET (self), -1, TABU_CONTROLS_HEIGTH);

  g_timeout_add (900, (GSourceFunc) timer_function, self);

  gtk_widget_show_all (GTK_WIDGET (self));
}

TabuControls *
tabu_controls_new (void)
{
  return (TABU_CONTROLS (g_object_new (tabu_controls_get_type(), NULL)));  
}
