#include <gst/gst.h>
#include <string.h>
#include "tabu.h"
#include "tabu-playlist.h"

static GstElement *pipeline = NULL;
static GstElement *audio_sink = NULL;

gint64
tabu_backend_get_current_length ()
{
  gint64 song_length;
  GstFormat fmt = GST_FORMAT_TIME;

  gst_element_query_duration (pipeline, &fmt, &song_length);  
  return ( song_length );
}

gint64
tabu_backend_get_current_position ()
{
  gint64 song_position;
  GstFormat fmt = GST_FORMAT_TIME;

  gst_element_query_position (pipeline, &fmt, &song_position);
  return ( song_position );
}

gint64
tabu_backend_get_volume ()
{
  gdouble vol;

  g_object_get (G_OBJECT (pipeline), "volume", &vol, NULL); 
  return ((gint64) (vol * 100));
}

void
tabu_backend_set_volume (gdouble vol)
{
  g_print ("\nvol %f", vol);
  g_object_set ( G_OBJECT(pipeline), "volume", vol, NULL);
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

gboolean
tabu_backend_is_paused()
{
  GstState state;
  gst_element_get_state ( GST_ELEMENT ( pipeline ), &state, NULL, GST_CLOCK_TIME_NONE);
  if (state == GST_STATE_PAUSED)
  {
    return (TRUE);
  }
  
  return (FALSE);
}


void
tabu_backend_stop ()
{
  gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_NULL );
}

void
tabu_backend_pause ()
{
  gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PAUSED );
}

void
tabu_backend_play ()
{
  gst_element_set_state ( GST_ELEMENT ( pipeline ), GST_STATE_PLAYING );
}

void
tabu_backend_play_uri (gchar *uri)
{ 
  if (uri != NULL)
  {
      gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);  
      g_object_set( G_OBJECT ( pipeline ), "uri", uri, NULL );    
      gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);  
  }
}


static gboolean
tabu_backend_gstreamer_bus_call ( GstBus *bus, GstMessage *msg, gpointer data )
{
  switch ( GST_MESSAGE_TYPE ( msg ) )
  {
    case GST_MESSAGE_EOS:
    {
      tabu_backend_play_uri (tabu_playlist_next (tabu_get_playlist (), FALSE));      
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

  pipeline = gst_element_factory_make ( "playbin", "playbin");
	
	audio_sink = gst_element_factory_make("autoaudiosink", "audio-sink");	

	g_object_set(G_OBJECT(pipeline), "audio-sink", audio_sink,	NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE ( pipeline ) );

  gst_bus_add_watch ( bus, tabu_backend_gstreamer_bus_call, NULL );

	gst_element_set_state(pipeline, GST_STATE_READY); 

  gst_object_unref ( bus );
}
