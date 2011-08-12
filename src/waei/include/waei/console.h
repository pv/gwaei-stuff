#ifndef GW_CONSOLE_INCLUDED
#define GW_CONSOLE_INCLUDED

#include <waei/console-callbacks.h>

void w_console_no_result(LwSearchItem*);

void w_console_about (WApplication*);
void w_console_list (WApplication*);
void w_console_start_banner (WApplication*);
void w_console_print_available_dictionaries (WApplication*);
void w_console_print_installable_dictionaries (WApplication*);

gboolean w_console_install_dictinst (WApplication*, GError**);
gboolean w_console_uninstall_dictinfo (WApplication*, GError**);

void w_console_handle_error (WApplication*, GError**);
gboolean w_console_search (WApplication*, GError**);

#endif
