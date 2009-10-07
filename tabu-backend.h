void tabu_backend_clear ();
void tabu_backend_stop ();
void tabu_backend_play ();
void tabu_backend_next ();
void tabu_backend_previous ();
void tabu_backend_play_selection ( GtkTreeSelection *selection );
void tabu_backend_set_volume (gdouble volume);
gchar * tabu_backend_get_pipeline_uri ( );
gboolean tabu_backend_is_playing();
