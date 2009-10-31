/* tabu-controls.h */

#ifndef _TABU_CONTROLS_H
#define _TABU_CONTROLS_H

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define TABU_TYPE_CONTROLS tabu_controls_get_type()

#define TABU_CONTROLS(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  TABU_TYPE_CONTROLS, TabuControls))

#define TABU_CONTROLS_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  TABU_TYPE_CONTROLS, TabuControlsClass))

#define TABU_IS_CONTROLS(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  TABU_TYPE_CONTROLS))

typedef struct _TabuControls TabuControls;
typedef struct _TabuControlsClass TabuControlsClass;


struct _TabuControls
{
  GtkDrawingArea parent;
  
  gboolean add_clicked;
  gboolean addfile_over;
  gboolean addfolder_over;
  
  gboolean del_clicked;
  gboolean clear_all_over;
  gboolean clear_selected_over;

  gboolean pbar_visible;
  gboolean vol_visible;

  gboolean volume_clicked;
};

struct _TabuControlsClass
{
  GtkDrawingAreaClass parent_class;
};

GType tabu_controls_get_type (void);

TabuControls *tabu_controls_new (void);
void tabu_controls_sync_playicon (GtkWidget *widget, gpointer data);

G_END_DECLS

#endif /* _TABU_CONTROLS_H */
