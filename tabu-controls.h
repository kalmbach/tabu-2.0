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
  GtkHBox parent;

  GtkWidget *box1;
  GtkWidget *box2;
  GtkWidget *box3;

  GtkWidget *play_button;
  GtkWidget *next_button;
  GtkWidget *prev_button;
  GtkWidget *add_button;
  GtkWidget *del_button;
  GtkWidget *vol_button;
  GtkWidget *pref_button;

  GtkWidget *empty_widget1;
  GtkWidget *empty_widget2;

  GtkWidget *add_menu;
  GtkWidget *add_menu_item;

  GtkWidget *del_menu;
  GtkWidget *del_menu_item_1;
  GtkWidget *del_menu_item_2;
};

struct _TabuControlsClass
{
  GtkHBoxClass parent_class;
};

GType tabu_controls_get_type (void);

TabuControls *tabu_controls_new (void);
void tabu_controls_sync_playicon (GtkWidget *widget, gpointer data);

G_END_DECLS

#endif /* _TABU_CONTROLS_H */
