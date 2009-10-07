/* tabu-controls.c */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

#include "tabu-controls.h"
#include "tabu-backend.h"
#include "tabu.h"

static GtkType tabu_controls_type = 0;

static void tabu_controls_class_init (TabuControlsClass *klass);
static void tabu_controls_init (TabuControls *self);
void show_file_chooser_dialog(GtkWidget *widget, gpointer data);
void tabu_controls_pack (TabuControls *widget);
void tabu_controls_unpack (TabuControls *widget);
void tabu_controls_unpack_adddialog (TabuControls *widget);

static gchar * _file_chooser_last_folder = NULL;

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
		};
		tabu_controls_type = g_type_register_static (
      GTK_TYPE_HBOX, 
      "TabuControls", 
      &tabu_controls_type_info, 
      0);
	}

	return tabu_controls_type;
}

static void
volume_button_value_changed (
  GtkWidget *widget,
  gdouble volume,
  gpointer data)
{
  //g_message ("volume changed to %f", volume);
  //tabu_backend_set_volume (volume);
}

void
clear_playlist()
{
  tabu_playlist_clear (tabu_get_playlist());
  tabu_backend_clear ();
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
show_folder_chooser_dialog(GtkWidget *widget, gpointer data)
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

  gtk_file_chooser_set_filter (selection, filter);

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
show_file_chooser_dialog(GtkWidget *widget, gpointer data)
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

  gtk_file_chooser_set_filter (selection, filter);

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

void
tabu_controls_sync_playicon(GtkWidget *widget, gpointer data)
{
  if (tabu_backend_is_playing())
  {
    gtk_button_set_image (
      GTK_BUTTON (widget), 
      gtk_image_new_from_stock (GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_BUTTON));
  }
  else
  {
    gtk_button_set_image (
      GTK_BUTTON (widget), 
      gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON));
  }
}

static void
tabu_controls_class_init (TabuControlsClass *klass)
{
}

void
tabu_controls_pack (TabuControls *widget)
{  
  gtk_box_pack_start (GTK_BOX (widget->box1), GTK_WIDGET (widget->add_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box1), GTK_WIDGET (widget->del_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box1), GTK_WIDGET (widget->empty_widget1), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box2), GTK_WIDGET (widget->prev_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box2), GTK_WIDGET (widget->play_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box2), GTK_WIDGET (widget->next_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box3), GTK_WIDGET (widget->empty_widget2), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box3), GTK_WIDGET (widget->pref_button), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (widget->box3), GTK_WIDGET (widget->vol_button), FALSE, FALSE, 0);  
  
  gtk_box_pack_start (GTK_BOX (widget), GTK_WIDGET (widget->box1), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (widget), GTK_WIDGET (widget->box2), FALSE, FALSE, 0);  
  gtk_box_pack_start (GTK_BOX (widget), GTK_WIDGET (widget->box3), TRUE, TRUE, 0);    
}

static void
tabu_controls_init (TabuControls *self)
{
  gtk_box_set_homogeneous (GTK_BOX (self), TRUE);
  self->box1 = gtk_hbox_new (FALSE, 0);
  self->box2 = gtk_hbox_new (FALSE, 0);
  self->box3 = gtk_hbox_new (FALSE, 0);

  /* Play Button */

  self->play_button = gtk_button_new();
  gtk_button_set_relief (
    GTK_BUTTON (self->play_button),
    GTK_RELIEF_NONE);
  gtk_button_set_image (
    GTK_BUTTON (self->play_button), 
    gtk_image_new_from_stock (GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON));  

  g_signal_connect (
    G_OBJECT(self->play_button),
    "clicked",
    G_CALLBACK (tabu_backend_play),
    NULL);

  g_signal_connect (  
    G_OBJECT (self->play_button),
    "clicked",
    G_CALLBACK (tabu_controls_sync_playicon),
    NULL);

  /* Next Button */

  self->next_button = gtk_button_new();
  gtk_button_set_relief (
    GTK_BUTTON (self->next_button),
    GTK_RELIEF_NONE);
  gtk_button_set_image (
    GTK_BUTTON (self->next_button), 
    gtk_image_new_from_stock (GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_BUTTON));

  g_signal_connect (
    G_OBJECT (self->next_button),
    "clicked",
    G_CALLBACK (tabu_backend_next),
    NULL);

  g_signal_connect_swapped (  
    G_OBJECT (self->next_button),
    "clicked",
    G_CALLBACK (tabu_controls_sync_playicon),
    self->play_button);

  /* Previous Button */

  self->prev_button = gtk_button_new();
  gtk_button_set_relief (
    GTK_BUTTON (self->prev_button),
    GTK_RELIEF_NONE);
  gtk_button_set_image (
    GTK_BUTTON (self->prev_button), 
    gtk_image_new_from_stock (GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_BUTTON));

  g_signal_connect (
    G_OBJECT (self->prev_button),
    "clicked",
    G_CALLBACK (tabu_backend_previous),
    NULL);

  g_signal_connect_swapped (  
    G_OBJECT (self->prev_button),
    "clicked",
    G_CALLBACK (tabu_controls_sync_playicon),
    self->play_button);

  /* Add Button */

  self->add_menu = gtk_menu_new ();
  self->add_menu_item = gtk_image_menu_item_new_with_label ("Carpeta");
  gtk_image_menu_item_set_image (
    GTK_IMAGE_MENU_ITEM (self->add_menu_item), 
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON));

  gtk_menu_shell_append (GTK_MENU_SHELL (self->add_menu), self->add_menu_item);
  gtk_widget_show (self->add_menu_item);

  self->add_button = gtk_menu_tool_button_new (
    gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON), 
    "");

  gtk_menu_tool_button_set_menu (self->add_button, self->add_menu);
  
  g_signal_connect ( 
    G_OBJECT (self->add_button),
    "clicked",
    G_CALLBACK (show_file_chooser_dialog),
    self);

  g_signal_connect ( 
    G_OBJECT (self->add_menu_item),
    "activate",
    G_CALLBACK (show_folder_chooser_dialog),
    self);


  /* Remove/Clean Button */
  self->del_menu = gtk_menu_new ();

  self->del_menu_item_1 = gtk_image_menu_item_new_with_label ("Todo");
  gtk_image_menu_item_set_image (
    GTK_IMAGE_MENU_ITEM (self->del_menu_item_1), 
    gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON));

  gtk_menu_shell_append (GTK_MENU_SHELL (self->del_menu), self->del_menu_item_1);
  gtk_widget_show (self->del_menu_item_1);

  self->del_menu_item_2 = gtk_image_menu_item_new_with_label ("Recortar");
  gtk_image_menu_item_set_image (
    GTK_IMAGE_MENU_ITEM (self->del_menu_item_2), 
    gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON));

  gtk_menu_shell_append (GTK_MENU_SHELL (self->del_menu), self->del_menu_item_2);
  gtk_widget_show (self->del_menu_item_2);

  self->del_button = gtk_menu_tool_button_new(
    gtk_image_new_from_stock (GTK_STOCK_REMOVE, GTK_ICON_SIZE_BUTTON),
    "");

  gtk_menu_tool_button_set_menu (self->del_button, self->del_menu);

  g_signal_connect (
    G_OBJECT (self->del_menu_item_1),
    "activate",
    G_CALLBACK (clear_playlist),
    NULL);

  /* Volume Button */

  self->vol_button = gtk_volume_button_new();
  gtk_button_set_relief (
    GTK_BUTTON (self->vol_button),
    GTK_RELIEF_NONE);

  gtk_widget_set_sensitive (GTK_WIDGET (self->vol_button), FALSE);

  g_signal_connect (
    self->vol_button, 
    "value-changed",
		G_CALLBACK (volume_button_value_changed),
		self);

  /* Preferences Button */

  self->pref_button = gtk_button_new();
  gtk_button_set_relief (
    GTK_BUTTON (self->pref_button),
    GTK_RELIEF_NONE);
  gtk_button_set_image (
    GTK_BUTTON (self->pref_button), 
    gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_BUTTON));
  
  gtk_widget_set_sensitive (GTK_WIDGET (self->pref_button), FALSE);

  /* empty widgets used for box filling */

  self->empty_widget1 = gtk_label_new ("");
  self->empty_widget2 = gtk_label_new ("");
  
  tabu_controls_pack (self); 
  gtk_widget_show_all (GTK_WIDGET (self));
}

TabuControls *
tabu_controls_new (void)
{
  return g_object_new (tabu_controls_get_type(), NULL);
}
