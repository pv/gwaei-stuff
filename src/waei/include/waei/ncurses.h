#ifndef NW_NCURSES_INCLUDED
#define NW_NCURSES_INCLUDED

#include <waei/ncurses-callbacks.h>

typedef enum {
  NW_NCCOLORS_GREENONBLACK,
  NW_NCCOLORS_BLUEONBLACK,
  NW_NCCOLORS_REDONBLACK,
  NW_NCCOLORS_TOTAL
} NWcursesColorPair;


void initialize_ncurses_interface (int, char**);
void nw_initialize_interface_output_generics (void);

void nw_no_result(LwSearchItem*);

void nw_start_ncurses ();

#endif
