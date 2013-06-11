#include "ewm.h"

int ewm_gui_hi_color = 15;	/* Hi-lite color of control */
int ewm_gui_lo_color = 8;	/* Shadow color of control */
int ewm_gui_fg_color = 0;	/* Foreground/Text color of control */
int ewm_gui_bg_color = 7;	/* Background color of control */
int ewm_gui_mg_color = 8;	/* Disabled/Text color of control */

static void dotted_rect(int x1, int y1, int x2, int y2, int fg, int bg)
{
   int x = ((x1+y1) & 1) ? 1 : 0;
   int c;

   for (c=x1; c<=x2; c++) {
      putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
      putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
   }

   for (c=y1+1; c<y2; c++) {
      putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
      putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
   }
}

int ewm_box_proc(int msg, DIALOG *d, int c)
{
	if ( msg == MSG_DRAW )
	{
		int fg = (d->flags & D_DISABLED) ? gui_mg_color : ewm_gui_fg_color;
        int g = ( d->d1 == 1 ) ? 1 : 0;
		rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, ewm_gui_bg_color);
        if ( g == 0 )
        {
			line ( screen, d->x, d->y, d->x+d->w, d->y, ewm_gui_lo_color );
			line ( screen, d->x, d->y, d->x, d->y+d->h, ewm_gui_lo_color );
			line ( screen, d->x, d->y+d->h, d->x+d->w, d->y+d->h, ewm_gui_hi_color );
			line ( screen, d->x+d->w, d->y, d->x+d->w, d->y+d->h, ewm_gui_hi_color );
        }
        else
        {
			line ( screen, d->x, d->y, d->x+d->w, d->y, ewm_gui_hi_color );
			line ( screen, d->x, d->y, d->x, d->y+d->h, ewm_gui_hi_color );
			line ( screen, d->x, d->y+d->h, d->x+d->w, d->y+d->h, ewm_gui_lo_color );
			line ( screen, d->x+d->w, d->y, d->x+d->w, d->y+d->h, ewm_gui_lo_color );
        }
	}

   return D_O_K;
}

int ewm_shadow_box_proc(int msg, DIALOG *d, int c)
{
	if ( msg == MSG_DRAW )
	{
		int fg = (d->flags & D_DISABLED) ? gui_mg_color : ewm_gui_fg_color;
        int g = ( d->d1 == 1 ) ? 1 : 0;
		rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, ewm_gui_bg_color);
        if ( g == 0 )
        {
        	rect ( screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, ewm_gui_lo_color );
        	rect ( screen, d->x+1, d->y+1, d->x+d->w, d->y+d->h, ewm_gui_hi_color );
        }
        else
        {
        	rect ( screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, ewm_gui_hi_color );
        	rect ( screen, d->x+1, d->y+1, d->x+d->w, d->y+d->h, ewm_gui_lo_color );
        }
	}

   return D_O_K;
}

int ewm_button_proc ( int msg, DIALOG *d, int c )
{
	int state1, state2;
	int swap;
	int g;

	switch (msg) {

	case MSG_DRAW:
		if (d->flags & D_SELECTED)
		{
			g = 1;
			state1 = ewm_gui_bg_color;
			state2 = (d->flags & D_DISABLED) ? ewm_gui_mg_color : ewm_gui_fg_color;
			rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, state2);
        	line ( screen, d->x, d->y, d->x+d->w, d->y, ewm_gui_lo_color );
        	line ( screen, d->x, d->y, d->x, d->y+d->h, ewm_gui_lo_color );
        	line ( screen, d->x+1, d->y+d->h, d->x+d->w, d->y+d->h, ewm_gui_hi_color );
        	line ( screen, d->x+d->w, d->y+1, d->x+d->w, d->y+d->h, ewm_gui_hi_color );
		}
		else
		{
			g = 0;
			state1 = (d->flags & D_DISABLED) ? ewm_gui_mg_color : ewm_gui_fg_color;
			state2 = ewm_gui_bg_color;
			rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, state2);
        	line ( screen, d->x, d->y, d->x+d->w-1, d->y, ewm_gui_hi_color );
        	line ( screen, d->x, d->y, d->x, d->y+d->h-1, ewm_gui_hi_color );
        	line ( screen, d->x, d->y+d->h, d->x+d->w, d->y+d->h, ewm_gui_lo_color );
        	line ( screen, d->x+d->w, d->y, d->x+d->w, d->y+d->h, ewm_gui_lo_color );
		}

		text_mode(-1);
		gui_textout(screen, d->dp, d->x+d->w/2+g, d->y+d->h/2-text_height(font)/2+g, state1, TRUE);

		if ((d->flags & D_GOTFOCUS) && (!(d->flags & D_SELECTED) || !(d->flags & D_EXIT)))
			dotted_rect(d->x+1+g, d->y+1+g, d->x+d->w-2+g, d->y+d->h-2+g, state1, state2);
		break;

	case MSG_WANTFOCUS:
		return D_WANTFOCUS;


	case MSG_KEY:
		/* close dialog? */
		if (d->flags & D_EXIT)
		{
		return D_CLOSE;
		}

		/* or just toggle */
		d->flags ^= D_SELECTED;
		scare_mouse();
		SEND_MESSAGE(d, MSG_DRAW, 0);
		unscare_mouse();
		break;

	case MSG_CLICK:
		/* what state was the button originally in? */
		state1 = d->flags & D_SELECTED;
		if (d->flags & D_EXIT)
			swap = FALSE;
		else
			swap = state1;
		/* track the mouse until it is released */
		while (gui_mouse_b())
		{
			state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
					(gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));

			if (swap)
               	state2 = !state2;

			/* redraw? */

			if (((state1) && (!state2)) || ((state2) && (!state1)))
			{
				d->flags ^= D_SELECTED;
				state1 = d->flags & D_SELECTED;
				scare_mouse();
				SEND_MESSAGE(d, MSG_DRAW, 0);
				unscare_mouse();
			}

			/* let other objects continue to animate */
			broadcast_dialog_message(MSG_IDLE, 0);
		}

	    /* should we close the dialog? */
		if ((d->flags & D_SELECTED) && (d->flags & D_EXIT))
		{
			d->flags ^= D_SELECTED;
			return D_CLOSE;
		}
		break;
	}

	return D_O_K;
}

int ewm_radio_proc(int msg, DIALOG *d, int c)
{
	int x, center, r, ret, fg;

	switch(msg)
	{
	case MSG_DRAW:
		fg = (d->flags & D_DISABLED) ? ewm_gui_mg_color : ewm_gui_fg_color;
		text_mode(-1);
		gui_textout(screen, d->dp, d->x+d->h+text_height(font), d->y+(d->h-(text_height(font)-gui_font_baseline))/2, fg, FALSE);

		x = d->x;
		r = d->h/2;
		center = x+r;
		rectfill(screen, x+1, d->y+1, x+d->h-1, d->y+d->h-1, ewm_gui_bg_color);

		switch (d->d2)
		{
		case 1:
			rect(screen, x+1, d->y+1, x+d->h+1, d->y+d->h+1, ewm_gui_lo_color);
			rect(screen, x, d->y, x+d->h, d->y+d->h, ewm_gui_hi_color);
			if (d->flags & D_SELECTED)
				rectfill(screen, x+1+r/2, d->y+1+r/2, x+d->h-r/2, d->y+d->h-r/2, fg);
			break;
		default:
			circle(screen, center+1, d->y+r+1, r, ewm_gui_lo_color);
			circle(screen, center, d->y+r, r, ewm_gui_hi_color);
			if (d->flags & D_SELECTED)
				circlefill(screen, center, d->y+r, r/2, fg);
			break;
		}

		if (d->flags & D_GOTFOCUS)
			dotted_rect(x+1, d->y+1, x+d->h-1, d->y+d->h-1, fg, ewm_gui_bg_color);

		return D_O_K;

	case MSG_KEY:
	case MSG_CLICK:
		if (d->flags & D_SELECTED)
		{
			return D_O_K;
		}
		break;

	case MSG_RADIO:
		if ((c == d->d1) && (d->flags & D_SELECTED))
		{
			d->flags &= ~D_SELECTED;
			scare_mouse();
			SEND_MESSAGE(d, MSG_DRAW, 0);
			unscare_mouse();
		}
		break;
	}

	ret = ewm_button_proc(msg, d, 0);

	if (((msg==MSG_KEY) || (msg==MSG_CLICK)) &&
		(d->flags & D_SELECTED) && (!(d->flags & D_EXIT)))
	{
		d->flags &= ~D_SELECTED;
		broadcast_dialog_message(MSG_RADIO, d->d1);
		d->flags |= D_SELECTED;
	}

	return ret;
}

int ewm_text_proc ( int msg, DIALOG *d, int c )
{
	if ( msg == MSG_DRAW )
	{
		int fg = ( d->flags & D_DISABLED ) ? ewm_gui_mg_color : ewm_gui_fg_color;
		FONT *oldfont = font;

		if (d->dp2)
			font = d->dp2;

		text_mode( ewm_gui_bg_color );
		gui_textout( screen, d->dp, d->x, d->y, fg, FALSE );
		font = oldfont;
	}
	return D_O_K;
}

int ewm_ctext_proc ( int msg, DIALOG *d, int c )
{
	if ( msg == MSG_DRAW )
	{
		int fg = ( d->flags & D_DISABLED ) ? gui_mg_color : ewm_gui_fg_color;
		FONT *oldfont = font;

		if (d->dp2)
			font = d->dp2;

		text_mode( ewm_gui_bg_color );
		gui_textout( screen, d->dp, d->x, d->y, fg, TRUE );
		font = oldfont;
	}
	return D_O_K;
}

int ewm_clear_proc ( int msg, DIALOG *d, int c )
{
	if (msg == MSG_DRAW)
	{
		set_clip ( screen, 0, 0, SCREEN_W-1, SCREEN_H-1 );
		clear_to_color ( screen, ewm_gui_bg_color );
	}
	return D_O_K;
}

