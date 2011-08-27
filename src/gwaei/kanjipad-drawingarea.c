/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file src/gtk-main-callbacks.c
//!
//! @brief Abstraction layer for the drawing area
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static void _kanjipadwindow_initialize_drawingarea (GwKanjipadWindow*);


//!
//! @brief To be written
//!
void gw_kanjipadwindow_free_drawingarea_stroke (GList *stroke)
{
    //Declarations
    GList *iter;

    for (iter = stroke; iter != NULL; iter = iter->next)
    {
      g_free (iter->data);
    }
    g_list_free (stroke);
}


//!
//! @brief To be written
//!
static void gw_kanjipadwindow_annotate_drawingarea_stroke (GwKanjipadWindow *window, GList *stroke, cairo_t *cr, gint index)
{
    GdkPoint *cur, *old;

    // Annotate the stroke with the stroke number - the algorithm
    // for placing the digit is pretty simple. The text is inscribed
    // in a circle tangent to the stroke. The circle will be above
    // and/or to the left of the line

    if (stroke)
    {
      old = (GdkPoint *)stroke->data;
      do
      {
        cur = (GdkPoint *)stroke->data;
        stroke = stroke->next;
      }
      while (stroke && abs(cur->x - old->x) < 5 && abs (cur->y - old->y) < 5);
      
      if (stroke)
      {
        char buffer[16];
        int swidth, sheight;
        gint16 x, y;
        double r;
        double dx = cur->x - old->x;
        double dy = cur->y - old->y;
        double dl = sqrt(dx*dx+dy*dy);
        int sign = (dy <= dx) ? 1 : -1;
        GdkRectangle update_area;

        swidth = 0;
        sheight = 0;

        sprintf (buffer, "%d", index);

        r = sqrt(swidth*swidth + sheight*sheight);
        
        x = 0.5 + old->x + 0.5*r*dx/dl + sign * 0.5*r*dy/dl;
        y = 0.5 + old->y + 0.5*r*dy/dl - sign * 0.5*r*dx/dl;
        
        x -= swidth/2;
        y -= sheight/2;

        update_area.x = x;
        update_area.y = y;
        update_area.width = swidth;
        update_area.height = sheight;
        
        x = CLAMP (x, 0, gtk_widget_get_allocated_width (GTK_WIDGET (window->drawingarea)) - swidth);
        y = CLAMP (y, 0, gtk_widget_get_allocated_height (GTK_WIDGET (window->drawingarea)) - sheight);

        cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
        cairo_set_font_size (cr, 10.0);
        cairo_move_to (cr, x, y);
        cairo_show_text (cr, buffer);
        cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      }
    }
}

//!
//! @brief To be written
//!
static void _kanjipadwindow_initialize_drawingarea (GwKanjipadWindow *window)
{
    //Declarations
    GList *iter;
    int index = 1;
    guint16 width;
    guint16 height;
    cairo_t *cr;
    double half_width;
    double half_height;
    double dashes[] = { 10.0, 10.0,  };
    int ndash;
    double offset;

    GtkStyleContext *context;
    GdkRGBA fgcolorn;
    GdkRGBA bgcolorn;
    GdkRGBA fgcolors;
    GdkRGBA bgcolors;

    //Initializations
    index = 1;
    cr = cairo_create (window->surface);
    width = gtk_widget_get_allocated_width (GTK_WIDGET (window->drawingarea));
    height = gtk_widget_get_allocated_height (GTK_WIDGET (window->drawingarea));
    half_width = ((double) width / 2.0);
    half_height = ((double) height / 2.0);
    ndash  = sizeof (dashes)/sizeof(dashes[0]);
    offset = 0.0;

    context = gtk_widget_get_style_context (GTK_WIDGET (window->drawingarea));
    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &fgcolorn);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_NORMAL, &bgcolorn);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_SELECTED, &fgcolors);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_SELECTED, &bgcolors);


    //Drawing
    cairo_set_source_rgba (cr, fgcolors.red, fgcolors.green, fgcolors.blue, 1.0);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);


    //Draw bounding box
    cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 0.5);
    cairo_set_line_width (cr, 6.0 );
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_stroke (cr);

    //Draw cross hairs
    cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 0.5);
    cairo_set_line_width (cr, 3.0);
    cairo_set_dash (cr, dashes, ndash, offset);

    cairo_move_to (cr, half_width, 0.0);
    cairo_line_to (cr, half_width, height);
    cairo_stroke (cr);
    cairo_move_to (cr, 0.0,  half_height);
    cairo_line_to (cr, width, half_height);
    cairo_stroke (cr);

    //Draw strokes
    cairo_set_dash (cr, NULL, 0, 0);
    cairo_set_line_width (cr, 2.0);
    cairo_set_source_rgba (cr, fgcolorn.red, fgcolorn.green, fgcolorn.blue, 1.0);

    for (iter = window->strokes; iter != NULL; iter = iter->next)
    {
      GdkPoint *cur, *old;
      GList *stroke_list = iter->data;

      old = NULL;

      if (window->annotate)
        gw_kanjipadwindow_annotate_drawingarea_stroke (window, stroke_list, cr, index);

      while (stroke_list)
      {
        cur = (GdkPoint *)stroke_list->data;
        if (old)
        {
          cairo_move_to (cr, old->x, old->y);
          cairo_line_to (cr, cur->x, cur->y);
          cairo_stroke (cr);
        }

        old = cur;
        stroke_list = stroke_list->next;
      }
      
      index++;
    }

    // End drawing here 
    cairo_destroy(cr);

    gtk_widget_queue_draw (GTK_WIDGET (window->drawingarea));
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT int gw_kanjipadwindow_drawingarea_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    //Declarations
    GwKanjipadWindow *window;

    //Initializations
    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    if (window->surface != NULL)
    {
      cairo_surface_destroy (window->surface);
    }

    window->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, event->width, event->height);

    _kanjipadwindow_initialize_drawingarea (window);

    return TRUE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT int gw_kanjipadwindow_drawingarea_draw_cb (GtkWidget *widget, cairo_t *cr, GwKanjipadWindow *window)
{
    if (window->surface == NULL)
      return FALSE;

    cairo_set_source_surface (cr, window->surface, 0, 0);
    cairo_paint (cr);

    return TRUE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT int gw_kanjipadwindow_drawingarea_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer *data)
{
    GwKanjipadWindow *window;
    GdkPoint *point;

    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    if (event->button == 1)
    {
      point = g_new (GdkPoint, 1);
      point->x = event->x;
      point->y = event->y;
      window->curstroke = g_list_append (window->curstroke, point);
      window->instroke = TRUE;
    }

    return TRUE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT int gw_kanjipadwindow_drawingarea_button_release_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer *data)
{
    GwKanjipadWindow *window;
    cairo_t *cr;

    //Initializations
    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    if (window->annotate)
    {
      //Initializations
      cr = cairo_create(window->surface);

      gw_kanjipadwindow_annotate_drawingarea_stroke (window, window->curstroke, cr, g_list_length (window->strokes) + 1);
      cairo_destroy(cr);
      gtk_widget_queue_draw (GTK_WIDGET (window->drawingarea));
    }

    window->strokes = g_list_append (window->strokes, window->curstroke);
    window->curstroke = NULL;
    window->instroke = FALSE;

    _kanjipadwindow_initialize_drawingarea (window);

    return FALSE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT int gw_kanjipadwindow_drawingarea_motion_event_cb (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    //Declarations
    GwKanjipadWindow *window;
    gint x,y;
    GdkModifierType state;
    GdkRectangle rect;
    GdkPoint *point;
    int xmin, ymin, xmax, ymax;
    GdkPoint *old;
    guint16 width;
    guint16 height;
    cairo_t *cr;

    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    if (event->is_hint)
    {
      gdk_window_get_pointer (gtk_widget_get_window (widget), &x, &y, &state);
    }
    else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }

    if (window->instroke == TRUE && (state & GDK_BUTTON1_MASK))
    {
      old = (GdkPoint*) g_list_last (window->curstroke)->data;
      width = gtk_widget_get_allocated_width (GTK_WIDGET (window->drawingarea));
      height = gtk_widget_get_allocated_height (GTK_WIDGET (window->drawingarea));

      //extend line
      cr = cairo_create(window->surface);
      cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
      cairo_set_line_width (cr, 2.0 );
      cairo_move_to (cr, old->x,  old->y);
      cairo_line_to (cr, x, y);
      cairo_stroke (cr);
      cairo_destroy(cr);

      if (old->x < x) { xmin = old->x; xmax = x; }
      else            { xmin = x;      xmax = old->x; }

      if (old->y < y) { ymin = old->y; ymax = y; }
      else            { ymin = y;      ymax = old->y; }

      rect.x = xmin - 1 - 3; 
      rect.y = ymin = 1 - 3;
      rect.width  = xmax - xmin + 2 + 3;
      rect.height = ymax - ymin + 2 + 3;
      gdk_window_invalidate_rect (gtk_widget_get_window (widget), &rect, FALSE);

      point = g_new (GdkPoint, 1);
      point->x = x;
      point->y = y;
      window->curstroke = g_list_append (window->curstroke, point);

      gtk_widget_queue_draw (GTK_WIDGET (window->drawingarea));
    }

    return TRUE;
}


//!
//! @brief To be written
//!
void gw_kanjipadwindow_clear_drawingarea (GwKanjipadWindow *window)
{
    //Declarations
    GList *iter;

    for (iter = window->strokes; iter != NULL; iter = iter->next)
    {
      gw_kanjipadwindow_free_drawingarea_stroke (iter->data);
    }
    g_list_free (window->strokes);

    g_list_free (window->curstroke);

    window->strokes = NULL;
    window->curstroke = NULL;

    _kanjipadwindow_initialize_drawingarea (window);
}


//!
//! @brief To be written
//!
void gw_kanjipadwindow_set_drawingarea_annotate (GwKanjipadWindow *window, gboolean annotate)
{
    if (window->annotate != annotate)
    {
      window->annotate = annotate;
      _kanjipadwindow_initialize_drawingarea (window);
    }
}


//!
//! @brief To be written
//!
void gw_kanjipadwindow_initialize_drawingarea (GwKanjipadWindow *window)
{
    gint mask;

    mask = (
        GDK_EXPOSURE_MASK |
        GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK |
        GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK
    );

    gtk_widget_set_size_request (GTK_WIDGET (window->drawingarea), 100, 100);

    g_signal_connect (window->drawingarea, "configure_event", G_CALLBACK (gw_kanjipadwindow_drawingarea_configure_event_cb), window->toplevel);
    g_signal_connect (window->drawingarea, "draw", G_CALLBACK (gw_kanjipadwindow_drawingarea_draw_cb), window);
    g_signal_connect (window->drawingarea, "button_press_event", G_CALLBACK (gw_kanjipadwindow_drawingarea_button_press_event_cb), window->toplevel);
    g_signal_connect (window->drawingarea, "button_release_event", G_CALLBACK (gw_kanjipadwindow_drawingarea_button_release_event_cb), window->toplevel);
    g_signal_connect (window->drawingarea, "motion_notify_event", G_CALLBACK (gw_kanjipadwindow_drawingarea_motion_event_cb), window->toplevel);
    g_signal_connect (window->drawingarea, "button_release_event", G_CALLBACK (gw_kanjipadwindow_look_up_cb), window->toplevel);

    gtk_widget_add_events (GTK_WIDGET (window->drawingarea), mask);
}


