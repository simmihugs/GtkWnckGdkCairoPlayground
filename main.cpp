#include <glib.h>
#include <cairo.h>
#include <iostream>
#include <string>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libwnck/libwnck.h>

const float MAX_WIDTH = 1920.0;
const float MAX_HEIGHT = 1080.0;
const float SCALE_FACTOR = 0.2;
const int ICON_WIDTH = 32;
const int ICON_HEIGHT = 32;

class Co {
public:
  WnckWindow *w;
  GtkWidget *f;

  Co(WnckWindow *w, GtkWidget *f) {
    this->w = w;
    this->f = f;
  } 
};

GdkPixbuf *get_screenshot(WnckWindow *wnck_window);
static void on_draw(Co *co);
void create_window_buttons(GtkWidget *container);

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkWindow *window; 
  { // window setup
    window = (GtkWindow *)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (window, 400, 400);
    gtk_window_set_position     (window, GTK_WIN_POS_CENTER);
    gtk_window_set_title        (window, "Drawing");

    g_signal_connect(window, "destroy", gtk_main_quit, NULL);
  }  

  //Add windows to app
  GtkWidget *container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  create_window_buttons(container);
  
  gtk_container_add(GTK_CONTAINER(window), container);
  
  gtk_widget_show_all((GtkWidget *)window);
  gtk_main();

  return 0;
}

void make_window_active_and_move_to_workspace(WnckWindow *wnck_window)
{
  WnckWorkspace *workspace = wnck_window_get_workspace(wnck_window);
  wnck_workspace_activate (workspace, gtk_get_current_event_time());
  wnck_window_activate(wnck_window, gtk_get_current_event_time());
}

GtkWidget *get_icon_button(WnckWindow *wnck_window)
{
  GtkWidget *image_button = gtk_button_new();
    
  const char *wnck_group_name = wnck_window_get_class_group_name(wnck_window);
  GdkPixbuf *icon_;
  if (std::string(wnck_group_name) == "Alacritty") {
    //icon_ = gdk_pixbuf_new_from_file("/usr/share/pixmaps/Alacritty.png", NULL);
    icon_ = gdk_pixbuf_new_from_file("Alacritty.png", NULL);
  } else {
    icon_ = wnck_window_get_icon(wnck_window);
  }
  icon_ = gdk_pixbuf_scale_simple(icon_, ICON_WIDTH, ICON_HEIGHT, GDK_INTERP_HYPER);
    
  GtkWidget *icon = gtk_image_new_from_pixbuf(icon_);
  gtk_button_set_image(GTK_BUTTON(image_button), icon);
  gtk_button_set_always_show_image(GTK_BUTTON(image_button), true);    

  g_signal_connect_swapped(image_button, "clicked",
			   G_CALLBACK(make_window_active_and_move_to_workspace),
			   wnck_window);
  return image_button;
}

void close_window(WnckWindow *wnck_window)
{
  wnck_window_close(wnck_window, gtk_get_current_event_time());
}

GtkWidget *get_close_button(WnckWindow *wnck_window)
{
  GtkWidget *close_button = gtk_button_new_with_label("X");

  g_signal_connect_swapped(close_button, "clicked",
			   G_CALLBACK(close_window),
			   wnck_window);  

  return close_button;
}

void create_window_buttons(GtkWidget *container)
{
  WnckScreen *wnck_screen = wnck_screen_get_default();
  wnck_screen_force_update(wnck_screen);  
  GList *window_l = wnck_screen_get_windows (wnck_screen);
  for (; window_l != NULL; window_l = window_l->next) {
    WnckWindow *wnck_window = WNCK_WINDOW (window_l->data);
    WnckWorkspace *workspace = wnck_window_get_workspace(wnck_window);
    if (workspace != NULL) {
      const char *group_name = wnck_window_get_class_group_name(wnck_window);
      
      GtkWidget *vertical_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
      GtkWidget *title_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);  
      gtk_container_add(GTK_CONTAINER(title_container), get_icon_button(wnck_window));
      gtk_container_add(GTK_CONTAINER(title_container), gtk_label_new(group_name));
      gtk_container_add(GTK_CONTAINER(title_container), get_close_button(wnck_window));
      gtk_container_add(GTK_CONTAINER(vertical_container), title_container);
    
      // create the are we can draw in
      GtkDrawingArea *drawingArea;
      {
	drawingArea = (GtkDrawingArea *)gtk_drawing_area_new();
	gtk_widget_set_size_request((GtkWidget *)drawingArea, MAX_WIDTH*SCALE_FACTOR, MAX_HEIGHT*SCALE_FACTOR);
	gtk_container_add(GTK_CONTAINER(vertical_container), (GtkWidget *)drawingArea);

        Co *co = new Co(wnck_window, (GtkWidget *)drawingArea);
	g_signal_connect_swapped((GtkWidget *)drawingArea, "draw", G_CALLBACK(on_draw), co);    
      }  

      gtk_container_add(GTK_CONTAINER(container), vertical_container);
    }
  }
}

static void on_draw(Co *co)
{
  GtkWidget *widget = GTK_WIDGET(co->f);
  WnckWindow *wnck_window = co->w;

  // "convert" the G*t*kWidget to G*d*kWindow (no, it's not a GtkWindow!)
  GdkWindow* window = gtk_widget_get_window(widget);  

  cairo_region_t *cairoRegion = cairo_region_create();
  GdkDrawingContext *drawingContext;
  drawingContext = gdk_window_begin_draw_frame(window, cairoRegion);
  { 
    // say: "I want to start drawing"
    cairo_t *cr = gdk_drawing_context_get_cairo_context (drawingContext);

    { // do your drawing
      float scaled_image_width = MAX_WIDTH*SCALE_FACTOR;
      float scaled_image_height = MAX_HEIGHT*SCALE_FACTOR;

      /* Draw a white background */
      cairo_set_source_rgb (cr, 1, 1, 1);
      cairo_rectangle (cr, 0.0, 0.0, scaled_image_width, scaled_image_height);
      cairo_fill (cr);
      cairo_paint(cr);

      if (!wnck_window_is_minimized(wnck_window)) {
	GdkPixbuf *pixbuf = get_screenshot(wnck_window);
	if (pixbuf != NULL) {
	  int width = gdk_pixbuf_get_width (pixbuf);
	  int height = gdk_pixbuf_get_height (pixbuf);

	  float scaled_width = 0.95*width*SCALE_FACTOR;
	  float scaled_height = 0.95*height*SCALE_FACTOR;
      
	  float xoffset = (scaled_image_width - scaled_width) / 2;
	  float yoffset = (scaled_image_height - scaled_height) / 2;

	  if (xoffset <= 0 || yoffset <= 0) {
	    std::cout << "xoffset: " << xoffset
		      << "\nyoffset: " << yoffset
		      << "\n"
		      << std::endl;		    
	  }

	  GdkPixbuf *scale_pixbuf =
	    gdk_pixbuf_scale_simple(pixbuf, scaled_width, scaled_height, GDK_INTERP_HYPER);
	  gdk_cairo_set_source_pixbuf(cr, scale_pixbuf, xoffset, yoffset);

	  cairo_paint(cr);	
	}	
      }
    }

    // say: "I'm finished drawing
    gdk_window_end_draw_frame(window,drawingContext);
  }

  // cleanup
  cairo_region_destroy(cairoRegion);
}

GdkPixbuf *get_screenshot(WnckWindow *wnck_window)
{
  WnckScreen *wnck_screen = wnck_screen_get_default();
  wnck_screen_force_update(wnck_screen);  
  GList *window_l = wnck_screen_get_windows (wnck_screen);
  for (; window_l != NULL; window_l = window_l->next) {
    WnckWindow *wnck_window_tmp = WNCK_WINDOW (window_l->data);
    WnckWorkspace *workspace = wnck_window_get_workspace(wnck_window);
    if (workspace != NULL) {
      if (wnck_window_tmp == wnck_window) {
	GdkScreen *gdk_screen = gdk_screen_get_default();
	GList *windows = gdk_screen_get_window_stack(gdk_screen);	
	for (; windows != NULL; windows = windows->next) {
	  GdkWindow *gdk_window = GDK_WINDOW(windows->data);
	  GdkWindowTypeHint gdkWindowTypeHint = gdk_window_get_type_hint(gdk_window);
	  if (gdkWindowTypeHint == GDK_WINDOW_TYPE_HINT_NORMAL) {
	    gulong wnck_xid = wnck_window_get_xid(wnck_window);
	    gulong gdk_xid  = gdk_x11_window_get_xid(gdk_window);

	    if (wnck_xid == gdk_xid) {
	      int x, y, width, height;
	      gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);
	
	      GdkPixbuf *pixbuf = gdk_pixbuf_get_from_window(gdk_window, x, y, width, height);
	      return pixbuf;
	    }
	  }
	}
      }
    }
  }
  return NULL;
}
