#include <gtk/gtk.h>
#include <gwaei/gwaei.h>

static void gw_addbutton_add_word_cb (GtkWidget*, gpointer);
static gboolean gw_addbutton_enter_notify_event_cb (GtkWidget*, GdkEvent*, gpointer);
static gboolean gw_addbutton_leave_notify_event_cb (GtkWidget*, GdkEvent*, gpointer);
static gboolean gw_addbutton_motion_notify_event_cb (GtkWidget*, GdkEvent*, gpointer);


GtkWidget* 
gw_addbutton_new (GwWindow *window, const gchar *KANJI, const gchar *FURIGANA, const gchar *DEFINITIONS)
{
    GtkWidget *button;
    GtkWidget *image;
    GtkCssProvider *provider;
    char *style_data;
    GtkStyleContext *context;

    button = gtk_button_new ();
    gtk_widget_add_events (button, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_MASK);
    gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
    image = gtk_image_new_from_icon_name ("list-add-symbolic", GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_image_set_pixel_size (GTK_IMAGE (image), 12);
    style_data = "* {\n"
                 "-GtkButton-default-border : 0;\n"
                 "-GtkButton-default-outside-border : 0;\n"
                 "-GtkButton-inner-border: 0;\n"
                 "-GtkWidget-focus-line-width : 0;\n"
                 "-GtkWidget-focus-padding : 0;\n"
                 "padding: 0;\n"
                 "}";
    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (button);
    gtk_css_provider_load_from_data (provider,  style_data, strlen(style_data), NULL); 
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);
  
    if (KANJI != NULL)
      g_object_set_data_full (G_OBJECT (button), "vocabulary-kanji", g_strdup (KANJI), g_free);
    if (FURIGANA != NULL)
      g_object_set_data_full (G_OBJECT (button), "vocabulary-furigana", g_strdup (FURIGANA), g_free);
    if (DEFINITIONS != NULL)
      g_object_set_data_full (G_OBJECT (button), "vocabulary-definitions", g_strdup (DEFINITIONS), g_free);

    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_addbutton_add_word_cb), window);
    g_signal_connect (G_OBJECT (button), "enter-notify-event", G_CALLBACK (gw_addbutton_enter_notify_event_cb), window);
    g_signal_connect (G_OBJECT (button), "leave-notify-event", G_CALLBACK (gw_addbutton_leave_notify_event_cb), window);
    g_signal_connect (G_OBJECT (button), "motion-notify-event", G_CALLBACK (gw_addbutton_motion_notify_event_cb), window);

    gtk_widget_show (image);
    gtk_widget_show (button);

    return button;
}


static void
gw_addbutton_add_word_cb (GtkWidget *widget, gpointer data)
{
    GwWindow *window;
    GwApplication *application;
    GtkWindow *addvocabularywindow;
    GtkButton *button;
    gchar *kanji, *furigana, *definitions;

    window = GW_WINDOW (data);
    application = gw_window_get_application (window);
    button = GTK_BUTTON (widget);
    kanji = g_object_get_data (G_OBJECT (button), "vocabulary-kanji");
    furigana = g_object_get_data (G_OBJECT (button), "vocabulary-furigana");
    definitions = g_object_get_data (G_OBJECT (button), "vocabulary-definitions");

    addvocabularywindow = gw_addvocabularywindow_new (GTK_APPLICATION (application));
    gtk_window_set_transient_for (addvocabularywindow, GTK_WINDOW (window));
    gw_addvocabularywindow_set_kanji (GW_ADDVOCABULARYWINDOW (addvocabularywindow), kanji);
    gw_addvocabularywindow_set_furigana (GW_ADDVOCABULARYWINDOW (addvocabularywindow), furigana);
    gw_addvocabularywindow_set_definitions (GW_ADDVOCABULARYWINDOW (addvocabularywindow), definitions);

    gw_addvocabularywindow_focus_add_button (GW_ADDVOCABULARYWINDOW (addvocabularywindow));

    gtk_widget_show (GTK_WIDGET (addvocabularywindow));
}

static gboolean 
gw_addbutton_enter_notify_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GdkDisplay *display;
    GdkWindow *window;
    GdkCursor *cursor;

    display = gdk_display_get_default ();
    window = gtk_widget_get_window (widget);
    cursor = gdk_cursor_new_for_display (display, GDK_HAND1);

    gdk_window_set_cursor (window, cursor);

    gdk_cursor_unref (cursor);

    return FALSE;
}


static gboolean 
gw_addbutton_leave_notify_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GdkWindow *window;

    window = gtk_widget_get_window (widget);

    gdk_window_set_cursor (window, NULL);

    return FALSE;
}


static gboolean
gw_addbutton_motion_notify_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    return TRUE;
}

