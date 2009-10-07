/* tabu-window.h */

#ifndef _TABU_WINDOW_H
#define _TABU_WINDOW_H

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TABU_TYPE_WINDOW tabu_window_get_type()

#define TABU_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TABU_TYPE_WINDOW, TabuWindow))

#define TABU_WINDOW_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TABU_TYPE_WINDOW, TabuWindowClass))

#define TABU_IS_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TABU_TYPE_WINDOW))

typedef struct _TabuWindow TabuWindow;
typedef struct _TabuWindowClass TabuWindowClass;


struct _TabuWindow
{
  GtkWindow parent;

  gchar *icon_filename;
  GtkWidget *main_box;

  GtkWidget *drawingarea_title;

  gboolean pointer_over_restore_icon;
  gboolean pointer_over_close_icon;

  gboolean is_minimized;
};

struct _TabuWindowClass
{
  GtkWindowClass parent_class;
};

GType tabu_window_get_type (void);

GtkWidget *tabu_window_new (void);

void tabu_window_add (
  TabuWindow *window,
  GtkWidget *widget, 
  gboolean expand, 
  gboolean fill, 
  guint padding);

G_END_DECLS

#endif /* _TABU_WINDOW_H */
