/* tabu-playlist.c */
#include <gdk/gdkkeysyms.h>
#include <gst/gst.h>
#include <tag_c.h>
#include <string.h>

#include "tabu-playlist.h"
#include "tabu-backend.h"
#include "tabu.h"

static GtkType tabu_playlist_type = 0;

/* privated functions */
static void tabu_playlist_class_init (TabuPlaylistClass *klass);
static void tabu_playlist_init (TabuPlaylist *self);
static void tabu_playlist_row_activated_callback ( 
  GtkTreeView *view, 
  GtkTreePath *path, 
  GtkTreeViewColumn *column, 
  gpointer data);
static void tabu_playlist_key_press_callback ( 
  GtkWidget *tview, 
  GdkEventKey *event,   
  gpointer data);

GType
tabu_playlist_get_type (void)
{
	/* if the type is not registered, then register it */
	if (!tabu_playlist_type)
	{
		static const GTypeInfo tabu_playlist_type_info =
		{
			sizeof (TabuPlaylistClass),
			NULL,
			NULL,
			(GClassInitFunc) tabu_playlist_class_init,
			NULL,
			NULL,
			sizeof (TabuPlaylist),
			0,
			(GInstanceInitFunc) tabu_playlist_init,
		};
		tabu_playlist_type = g_type_register_static (
      GTK_TYPE_SCROLLED_WINDOW, 
      "TabuPlaylist", 
      &tabu_playlist_type_info, 
      0);
	}

	return tabu_playlist_type;
}

gchar *
_get_formatted_song ( gchar *filename )
{
  TagLib_File *file;
  TagLib_Tag *tag;
  const TagLib_AudioProperties *properties;
  gchar *title = NULL;
  gchar *artist = NULL; 
  int minutes, seconds;
  gchar *row = NULL;

	file = taglib_file_new ( filename );

  if ( file == NULL )
  {
  	return ( NULL );
  }

	tag = taglib_file_tag ( file );  
  properties = taglib_file_audioproperties ( file );

  title = taglib_tag_title ( tag );
  artist = taglib_tag_artist ( tag );
  seconds = taglib_audioproperties_length(properties) % 60;
  minutes = (taglib_audioproperties_length(properties) - seconds) / 60;

  if ( strlen ( title )  == 0 )
  {
    gchar **tokens = NULL;
    int i = 0;

    tokens = g_strsplit ( filename, "/", 0 );
    if ( tokens != NULL )
    {
      while ( tokens[i] != NULL )
        i++;
  
      title = g_strdup ( tokens[i-1] );      
    }
    g_strfreev ( tokens );
  }

  if ( strlen ( artist ) == 0 )
    artist = "Unknown";

  /* track duration */  

  gchar duration[25];
  snprintf( duration, 24, "%i:%02i", minutes, seconds);  

  /* row constructor */
	row = g_strconcat (   
    "<span variant='smallcaps'>",
    g_markup_escape_text ( title, -1 ),
    "</span> - <small><i>",
    g_markup_escape_text ( artist, -1 ), 
    "</i> : ", duration, "</small>", 
    NULL );

  /*g_free ( title );
  g_free ( artist );*/

  taglib_tag_free_strings ( );
  taglib_file_free ( file );

  return ( g_strdup (row) );
}

void
tabu_playlist_scroll_to_iter (TabuPlaylist *playlist, GtkTreeIter iter)
{
  g_return_if_fail (playlist != NULL);

  GtkTreePath *path = gtk_tree_model_get_path (
    GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store), 
    &iter);

  gtk_tree_view_scroll_to_cell (
    GTK_TREE_VIEW (TABU_PLAYLIST (playlist)->tree_view), 
    path, 
    NULL, 
    FALSE, 
    0, 
    0);
}

void
tabu_playlist_clear (TabuPlaylist *playlist)
{
  gtk_list_store_clear (TABU_PLAYLIST (playlist)->list_store);
}

void
tabu_playlist_crop_selection (TabuPlaylist *playlist, GtkTreeSelection *selection)
{
  GtkTreeModel *store;
  GtkTreeIter iter;
  GList *selected;
  GList *references = NULL;
  GList *to_remove = NULL;

  selected = gtk_tree_selection_get_selected_rows ( selection, &store );
  while (selected != NULL)
  {
    references = g_list_append ( references, gtk_tree_row_reference_new (store, selected->data));
    selected = g_slist_next ( selected );
  }

  references = g_list_first (references);
  gtk_tree_model_get_iter_first (store, &iter);
  do
  {
    if (g_list_find (references, gtk_tree_row_reference_new (store, gtk_tree_model_get_path (store, &iter))) == NULL)
    {
      to_remove = g_list_append (to_remove, gtk_tree_row_reference_new (store, gtk_tree_model_get_path (store, &iter)));      
    }
  }while (gtk_tree_model_iter_next (store, &iter));

  to_remove = g_list_first (to_remove);
  while ( to_remove!= NULL)
  {        
    g_print ("remove row");
    /* ok, now remove the song from the playlist */
    gtk_tree_model_get_iter (store, &iter, gtk_tree_row_reference_get_path(to_remove->data));
    gtk_list_store_remove ( GTK_LIST_STORE ( store ), &iter);   
    to_remove = g_slist_next (to_remove); 
  }
  
  g_list_foreach (references, gtk_tree_row_reference_free, NULL);
  g_list_free (references);

  g_list_foreach (selected, gtk_tree_path_free, NULL);
  g_list_free (selected);
}

void
tabu_playlist_remove_selection (TabuPlaylist *playlist, GtkTreeSelection *selection)
{
  GtkTreeModel *store;
  GtkTreeIter iter;
  GtkTreePath *path;
  GList *selected;
  GList *references = NULL;

  selected = gtk_tree_selection_get_selected_rows ( selection, &store );
  while (selected != NULL)
  {
    references = g_list_append ( references, gtk_tree_row_reference_new (store, selected->data));
    selected = g_slist_next ( selected );
  }

  references = g_list_first (references);
  while (references != NULL)
  {
    path = gtk_tree_row_reference_get_path (references->data);

    if ( gtk_tree_model_get_iter (GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist())->list_store), &iter, path))
    {
      gchar *song = NULL;
      gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store ), &iter, 2, &song, -1 );

      if ( song && tabu_backend_get_pipeline_uri() )
      {
        /* if the song to be removed is the currently playing song */
        if ( !strcmp ( song, tabu_backend_get_pipeline_uri() ) )
        {
          tabu_backend_stop();
          tabu_controls_sync_playicon (TABU_CONTROLS(tabu_get_controls())->play_button, NULL);
        }      
      }

      g_free ( song );

      /* ok, now remove the song from the playlist */
      gtk_list_store_remove ( GTK_LIST_STORE ( store ), &iter );
    } 
    
    gtk_tree_path_free (path);    
    references = g_slist_next ( references );
  }

  g_list_foreach (references, gtk_tree_row_reference_free, NULL);
  g_list_free (references);

  g_list_foreach (selected, gtk_tree_path_free, NULL);
  g_list_free (selected);
}

void
tabu_playlist_append (TabuPlaylist *playlist, gchar* filename, gchar *uri)
{  
  g_return_if_fail (playlist != NULL);

	GtkTreeIter iter;

	gchar *row = _get_formatted_song ( filename );
  
  if ( row == NULL )
    return;

	gtk_list_store_append (TABU_PLAYLIST (playlist)->list_store, &iter);

  gtk_list_store_set (
    TABU_PLAYLIST (playlist)->list_store, &iter, 0, "", 1, row, 2, uri, -1);

  g_free ( row );
}

void
on_tabu_playlist_row_activated_event ( 
  GtkTreeView *view, 
  GtkTreePath *path, 
  GtkTreeViewColumn *column, 
  gpointer data )
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection ( view );
  tabu_backend_play_selection ( selection );
  tabu_controls_sync_playicon (TABU_CONTROLS(tabu_get_controls())->play_button, NULL);
}

gboolean
on_tabu_playlist_key_press_event ( 
  GtkWidget *view, 
  GdkEventKey *event, 
  gpointer data )
{
  GtkTreeSelection *selection = gtk_tree_view_get_selection (
    GTK_TREE_VIEW (view));

  if ( event->type == GDK_KEY_PRESS )
  {
    switch ( event->keyval )
    {
      /* if the delete key is pressed. remove the item from the playlist */
      case GDK_Delete:
        tabu_playlist_remove_selection ( tabu_get_playlist(), selection );
        break;

      default: 
        return FALSE;
    }
  }
}


void
_cell_data_function(
  GtkTreeViewColumn *col,
  GtkCellRenderer   *renderer,
  GtkTreeModel      *model,
  GtkTreeIter       *iter,
  gpointer           user_data)
{
  
}

static void
tabu_playlist_class_init (TabuPlaylistClass *klass)
{
}

static void
tabu_playlist_init (TabuPlaylist *self)
{ 
  gtk_scrolled_window_set_shadow_type (
    GTK_SCROLLED_WINDOW (self),
    GTK_SHADOW_IN);

  gtk_scrolled_window_set_hadjustment (
    GTK_SCROLLED_WINDOW (self),
    NULL);

  gtk_scrolled_window_set_vadjustment (
    GTK_SCROLLED_WINDOW (self),
    NULL);

  gtk_scrolled_window_set_policy (
    GTK_SCROLLED_WINDOW (self), 
    GTK_POLICY_NEVER, 
    GTK_POLICY_AUTOMATIC);
  
  self->tree_view = gtk_tree_view_new ( );

  gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (self->tree_view), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->tree_view), FALSE);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->tree_view), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (self->tree_view), TRUE);

	gtk_tree_selection_set_mode ( 
    GTK_TREE_SELECTION ( 
      gtk_tree_view_get_selection (GTK_TREE_VIEW (self->tree_view))), 
      GTK_SELECTION_MULTIPLE);

  self->cell_renderer = gtk_cell_renderer_text_new ();
  g_object_set (self->cell_renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  GtkTreeViewColumn *pointercol = gtk_tree_view_column_new_with_attributes(
    "Pointer",
    GTK_CELL_RENDERER (self->cell_renderer),
    "markup", 0,
    NULL );

  self->tree_view_column = gtk_tree_view_column_new_with_attributes(
    "Cancion",
    GTK_CELL_RENDERER (self->cell_renderer),
    "markup", 1,
    NULL );

  gtk_tree_view_column_set_sizing (self->tree_view_column, GTK_TREE_VIEW_COLUMN_FIXED);
  gtk_tree_view_column_set_sizing (pointercol, GTK_TREE_VIEW_COLUMN_FIXED);  
//  gtk_tree_view_column_set_fixed_width (pointercol, 15 );;
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (self->tree_view_column), TRUE);

  gtk_tree_view_insert_column (
    GTK_TREE_VIEW (self->tree_view), 
    pointercol, 
    -1);

  gtk_tree_view_insert_column (
    GTK_TREE_VIEW (self->tree_view), 
    self->tree_view_column, 
    -1);

  self->list_store = gtk_list_store_new ( 
    3, 
    G_TYPE_STRING, 
    G_TYPE_STRING,
    G_TYPE_STRING);

  gtk_tree_view_set_model (
    GTK_TREE_VIEW (self->tree_view), 
    GTK_TREE_MODEL (self->list_store));  

  g_signal_connect (
    G_OBJECT (self->tree_view), 
    "row-activated", 
    G_CALLBACK (on_tabu_playlist_row_activated_event), 
    NULL);

  g_signal_connect ( 
    G_OBJECT (self->tree_view), 
    "key-press-event", 
    G_CALLBACK (on_tabu_playlist_key_press_event), 
    NULL);
  
	gtk_container_add ( 
    GTK_CONTAINER ( GTK_SCROLLED_WINDOW (self)), 
    GTK_WIDGET (self->tree_view));
}

GtkWidget *
tabu_playlist_new (void)
{
  return (GTK_WIDGET (g_object_new (tabu_playlist_get_type(), NULL)));
}
