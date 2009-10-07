#include <gtk/gtk.h>
#include "tabu-window.h"
#include "tabu-controls.h"
#include "tabu-playlist.h"
#include "tabu-backend.h"

GtkWidget *window;
GtkWidget *controls;
GtkWidget *playlist;

TabuPlaylist *
tabu_get_playlist()
{
  return (TABU_PLAYLIST (playlist));
}

GtkWidget *
tabu_get_window()
{
  return (GTK_WIDGET (window));
}

TabuControls *
tabu_get_controls()
{
  return (TABU_CONTROLS (controls));
}

int
main ( int argc, char *argv[] )
{
  gtk_init( &argc, &argv );
  
  window = tabu_window_new ();    
  controls = tabu_controls_new ();
  playlist = tabu_playlist_new ();
  
  tabu_window_add (TABU_WINDOW (window), GTK_WIDGET (playlist), TRUE, TRUE, 0);  
  tabu_window_add (TABU_WINDOW (window), GTK_WIDGET (controls), FALSE, FALSE, 0);  

  gtk_widget_show_all (controls);
  gtk_widget_show_all (window);
  tabu_backend_init ();
  gtk_main ( );

  return 0;
}
