#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>

void gw_application_private_init (GwApplication *app)
{
      GwApplicationPrivate *priv;

      priv = GW_APPLICATION_GET_PRIVATE (app);

      priv->arg_dictionary = NULL;
      priv->arg_query = NULL;
      priv->arg_version_switch = FALSE;
      //priv->last_focused = NULL;

      priv->engine = lw_engine_new (
       gw_output_append_edict_results_cb,
       gw_output_append_kanjidict_results_cb,
       gw_output_append_examplesdict_results_cb,
       gw_output_append_unknowndict_results_cb,
       gw_output_append_less_relevant_header_cb,
       gw_output_append_more_relevant_header_cb,
       gw_output_prepare_search_cb,
       gw_output_cleanup_search_cb
      );

      priv->preferences = lw_preferences_new ();
      priv->dictinfolist = gw_dictinfolist_new (20, priv->preferences);
      priv->block_new_searches = 0;

      priv->tagtable = gw_texttagtable_new ();
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG, gw_application_sync_tag_cb, priv);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG, gw_application_sync_tag_cb, priv);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_FG, gw_application_sync_tag_cb, priv);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_BG, gw_application_sync_tag_cb, priv);
      lw_preferences_add_change_listener_by_schema (
        priv->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_COMMENT_FG, gw_application_sync_tag_cb, priv);

#ifdef OS_MINGW
      GtkSettings *settings;
      settings = gtk_settings_get_default ();
      g_object_set (settings, "gtk-theme-name", "MS-Windows", NULL);
      g_object_set (settings, "gtk-menu-images", FALSE, NULL);
      g_object_set (settings, "gtk-button-images", FALSE, NULL);
      g_object_set (settings, "gtk-cursor-blink", FALSE, NULL);
      g_object_set (settings, "gtk-alternative-button-order", TRUE, NULL);
      g_object_unref (settings);
#endif
}

void gw_application_private_finalize (GwApplication *app)
{
    GwApplicationPrivate *priv;

    priv = GW_APPLICATION_GET_PRIVATE (priv);

    gw_dictinfolist_free (priv->dictinfolist);
    g_option_context_free (priv->context);
    g_free(priv->arg_query);
    lw_engine_free (priv->engine);
    lw_preferences_free (priv->preferences);
}
