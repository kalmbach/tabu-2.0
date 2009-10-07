#include <gst/gst.h>
#include <string.h>
#include "tabu.h"
#include "tabu-playlist.h"

static GstElement *pipeline = NULL;
static GstElement *audio_sink = NULL;
static GstElement *sink = NULL;
static GstElement *filterbin = NULL;
static GstElement *sinkbin = NULL;
static GstElement *tee = NULL;
static gchar *pipeline_uri = NULL;
static gchar *song = NULL;
static GtkTreeIter iter;

static gboolean _playing = FALSE;

void
tabu_backend_set_iter_playing ( gboolean value)
{
  if (value)
  {
    if (GTK_LIST_STORE (TABU_PLAYLIST (tabu_get_playlist())->list_store)->stamp == iter.stamp)
      gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist ())->list_store), &iter, 1, &song, -1);
  }

  if (song != NULL)
  {
    if (value)
    {
      gtk_list_store_set (
        TABU_PLAYLIST (tabu_get_playlist())->list_store, 
        &iter, 1, g_strconcat("<b>", song, "</b>", NULL), -1);

      /* scroll to song if this is not visible */
      GtkTreePath *path = gtk_tree_model_get_path (
        GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist())->list_store ), 
        &iter );
     
      gtk_tree_view_scroll_to_cell (
        GTK_TREE_VIEW (TABU_PLAYLIST (tabu_get_playlist())->tree_view), 
        path, 
        NULL, 
        FALSE, 
        0, 
        0 );
    } 
    else
    {
      gtk_list_store_set (
        TABU_PLAYLIST (tabu_get_playlist())->list_store, 
        &iter, 1, song,-1);
    }
  }
}

gchar * 
tabu_backend_get_pipeline_uri ()
{
  return (pipeline_uri);
}

gboolean
tabu_backend_is_playing()
{
  GstState state;
  gst_element_get_state ( GST_ELEMENT ( pipeline ), &state, NULL, GST_CLOCK_TIME_NONE);
  if (state == GST_STATE_PLAYING)
  {
    return (TRUE);
  }
  
  return (FALSE);
}

void
tabu_backend_set_volume (gdouble vol)
{
  g_print ("\nvol %f", vol);
  g_object_set ( G_OBJECT(pipeline), "volume", vol, NULL);
}

void
tabu_backend_play_selection ( GtkTreeSelection *selection )
{
  GtkTreeModel *store;
  GList *selected_songs;

  tabu_backend_set_iter_playing (FALSE);
  
  selected_songs = gtk_tree_selection_get_selected_rows ( selection, &store );
  
  if (selected_songs != NULL)
  {
    if (gtk_tree_model_get_iter (GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist())->list_store), &iter, selected_songs->data))
    {
      gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
      g_free ( pipeline_uri );

      gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist ())->list_store), &iter, 2, &pipeline_uri, -1);    
      g_object_set(G_OBJECT(pipeline), "uri", pipeline_uri, NULL);
      gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PLAYING );
    }  
  }
  tabu_backend_set_iter_playing (TRUE);

  g_list_foreach (selected_songs, gtk_tree_path_free, NULL);
  g_list_free (selected_songs);
}

void
tabu_backend_stop ()
{
  /* stop playback */
  gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_NULL );
  g_free ( pipeline_uri );
  pipeline_uri = NULL;
}

void
tabu_backend_play ()
{ 
  tabu_backend_set_iter_playing (FALSE);

  if (pipeline_uri == NULL)
  {
    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (TABU_PLAYLIST(tabu_get_playlist ())->list_store), &iter ))
    {
      return;
    }
    else
    {
      gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (tabu_get_playlist ())->list_store), &iter, 2, &pipeline_uri, -1 );
      g_object_set( G_OBJECT ( pipeline ), "uri", pipeline_uri, NULL );
    }
  }

  if (tabu_backend_is_playing ())
  {     
    gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PAUSED );    
  }
  else if (pipeline_uri != NULL)
  {
    gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PLAYING );
  }

  tabu_backend_set_iter_playing (TRUE);
}

void
tabu_backend_next ()
{
  gboolean iter_exists = FALSE;
  TabuPlaylist *playlist = tabu_get_playlist();

  tabu_backend_set_iter_playing (FALSE);

  if ((GTK_LIST_STORE (TABU_PLAYLIST (playlist)->list_store)->stamp == iter.stamp)
      && (gtk_tree_model_iter_next (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store ), &iter)))
    iter_exists = TRUE;
  else if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store), &iter))
    iter_exists = TRUE;
  
  if (iter_exists)
  {
    g_free ( pipeline_uri );
    gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_NULL );

    gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store), &iter, 2, &pipeline_uri, -1 );
    g_object_set( G_OBJECT ( pipeline ), "uri", pipeline_uri, NULL );
   
    gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PLAYING );    
  }

  tabu_backend_set_iter_playing (TRUE);
}

void
tabu_backend_previous ()
{
  GtkTreePath *path;
  TabuPlaylist *playlist = tabu_get_playlist();

  tabu_backend_set_iter_playing (FALSE);

  if (GTK_LIST_STORE (TABU_PLAYLIST (playlist)->list_store)->stamp == iter.stamp)
  {  
    path = gtk_tree_model_get_path (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store ), &iter );
    if (( path != NULL ) && ( gtk_tree_path_prev ( path ) ))
    {
      if (gtk_tree_model_get_iter (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store), &iter, path))
      {
        g_free ( pipeline_uri );
        gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
      
        gtk_tree_model_get (GTK_TREE_MODEL (TABU_PLAYLIST (playlist)->list_store), &iter, 2, &pipeline_uri, -1 );    
        g_object_set( G_OBJECT ( pipeline ), "uri", pipeline_uri, NULL ); 

        gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PLAYING ); 
      }
    }
  }

  tabu_backend_set_iter_playing (TRUE);
}

void
tabu_backend_clear ()
{
  if (pipeline_uri)
  {
    g_free (pipeline_uri);
    pipeline_uri = NULL;    
  }
 
  /* stop playing the current song */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
}

static gboolean
tabu_backend_gstreamer_bus_call ( GstBus *bus, GstMessage *msg, gpointer data )
{
  switch ( GST_MESSAGE_TYPE ( msg ) )
  {
    case GST_MESSAGE_EOS:
    {
      /*g_print ( "End-of-stream\n" );*/
      tabu_backend_next ();      
      break;
    }
    case GST_MESSAGE_ERROR:
    {
      gchar *debug;
      GError *error;
    
      gst_message_parse_error ( msg, &error, &debug );
      g_free ( debug );
      
      g_print ( "Error: %s\n", error->message );
      break;
    }
    default:
      break;
  }

  return TRUE;
}

void
tabu_backend_quit ()
{
  if ( pipeline != NULL )
  {
    g_free ( pipeline_uri );
    gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_NULL );
    gst_object_unref ( GST_OBJECT ( pipeline ) );
    pipeline = NULL;
  }         
}

void
tabu_backend_init ()
{
  GstBus *bus;

  /* initialize GStreamer */
  gst_init ( NULL, NULL );

  pipeline = gst_element_factory_make ( "playbin", NULL);
  bus = gst_pipeline_get_bus (GST_PIPELINE ( pipeline ) );
  gst_bus_add_watch ( bus, tabu_backend_gstreamer_bus_call, NULL );
  gst_object_unref ( bus );
}
