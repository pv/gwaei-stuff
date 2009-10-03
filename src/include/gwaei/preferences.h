void gwaei_pref_set_int (char*, int);
int gwaei_pref_get_int (char*, int);
int gwaei_pref_get_default_int (char*, int);

void gwaei_pref_set_boolean (char*, gboolean);
gboolean gwaei_pref_get_boolean (char*, gboolean);
gboolean gwaei_pref_get_default_boolean (char*, gboolean);

void gwaei_pref_set_string (char*, const char*);
char* gwaei_pref_get_string (char*, char*, char*, int);
const char* gwaei_pref_get_default_string (char*, char*);

void gwaei_prefs_initialize_preferences(void);

