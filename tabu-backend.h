void tabu_backend_init ();

void tabu_backend_stop ();
void tabu_backend_pause ();
void tabu_backend_play ();
void tabu_backend_play_uri (gchar *uri);

gboolean tabu_backend_is_playing();
gboolean tabu_backend_is_paused();
gint64 tabu_backend_get_current_length ();
gint64 tabu_backend_get_current_position ();

gint64 tabu_backend_get_volume ();
void tabu_backend_set_volume (gdouble volume);
