#ifndef EXTREME_WINDOW_MODULES_H
#define EXTREME_WINDOW_MODULES_H

#include <allegro.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int ewm_gui_hi_color;	/* Hi-lite color of control */
extern int ewm_gui_lo_color;	/* Shadow color of control */
extern int ewm_gui_fg_color;	/* Foreground/Text color of control */
extern int ewm_gui_bg_color;	/* Background color of control */
extern int ewm_gui_mg_color;	/* Disabled/Text color of control */

/* Standard 3D widget controls for Allegro */
int ewm_button_proc ( int msg, DIALOG *d, int c );
int ewm_shadow_box_proc ( int msg, DIALOG *d, int c );
int ewm_box_proc ( int msg, DIALOG *d, int c );
int ewm_radio_proc(int msg, DIALOG *d, int c);
int ewm_text_proc ( int msg, DIALOG *d, int c );
int ewm_ctext_proc ( int msg, DIALOG *d, int c );
int ewm_clear_proc ( int msg, DIALOG *d, int c );

#ifdef __cplusplus
}
#endif

#endif
