/*
    Flame F/X - Fire Effects Demonstration Program
    Copyright (C) 1999 Jonathan Metzgar

    E-Mail: microwerx@yahoo.com
    		microwerx@fcmail.com

    URL:
    http://www.geocities.com/SiliconValley/Sector/8648/
    http://skyscraper.fortunecity.com/altavista/565/
    http://welcome.to/microwerx

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <iostream.h>
#include <string.h>
#include <conio.h>
#include <allegro.h>
#include "ewm.h"

void usage ( );
void fire_setup ( );
void start_allegro ( );

class Bitmap
{
public:
	PALETTE palette;
    BITMAP *bmp;

    Bitmap ( );
    ~Bitmap ( );

    void Load ( char *filename );
    void Save ( char *filename );
    void Destroy ( );
};

/* The Majority of this program is controled by this class */
class FireApp
{
public:
	void Init ( );
	void SetScreen ( int vw, int vh, int width, int height, int driver, int pal );
    void Play ( );
    void Kill ( );
private:
	PALETTE palette[4];
    BITMAP *db;
    Bitmap splash;


    int myPalette;
    int myWidth;
    int myHeight;
    int myDriver;
    int myEffect;
    int Vw, Vh;

    unsigned char line[640];

    void InitGraphics ( );

    void FX1 ( );
    void FX2 ( );
    void FX3 ( );
    void FX4 ( );
    void FX5 ( );
    void FX6 ( );
    void FX7 ( );
    void FX8 ( );
};

FireApp theApp;

int main ( int argc, char **argv )
{
	usage ( );

    start_allegro ( );

    theApp.Init ( );

    Bitmap backImage;
    backImage.Load ( "flamefx.pcx" );

    set_gfx_mode ( GFX_VGA, 320, 200, 0, 0 );
    set_palette ( black_palette );
    blit ( backImage.bmp, screen, 0, 0, 0, 0, 320, 200 );
    fade_in ( backImage.palette, 8 );
    rest ( 2000 );
    fade_out ( 8 );
    backImage.Destroy ( );

    fire_setup ( );

    theApp.Play ( );

    theApp.Kill ( );

	return 0;
}

char width_string[10] = "320";
char height_string[10] = "200";
int driver;

DIALOG myDialog[] = {
	{ ewm_clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ ewm_button_proc, 140, 170, 40, 20, 0, 255, 'x', D_EXIT, 0, 0, "E&xit", 0, 0 },
    { ewm_shadow_box_proc, 10, 30, 140, 100, 0, 255, 0, 0, 0, 0, 0, 0, 0, },
    { ewm_shadow_box_proc, 170, 30, 140, 100, 0, 255, 0, 0, 0, 0, 0, 0, 0, },
    { ewm_ctext_proc, 80, 35, 100, 10, 0, 255, 0, 0, 0, 0, "VIDEO MODE", 0, 0 },
    { ewm_radio_proc, 20, 50, 100, 8, 0, 255, 0, 0, 1, 0, "160x120", 0, 0 },
    { ewm_radio_proc, 20, 65, 100, 8, 0, 255, 0, D_SELECTED, 1, 0, "320x200", 0, 0 },
    { ewm_radio_proc, 20, 80, 100, 8, 0, 255, 0, 0, 1, 0, "320x240", 0, 0 },
    { ewm_radio_proc, 20, 95, 100, 8, 0, 255, 0, 0, 1, 0, "320x400", 0, 0 },
    { ewm_radio_proc, 20, 110, 100, 8, 0, 255, 0, 0, 1, 0, "640x480", 0, 0 },
    { ewm_ctext_proc, 240, 35, 100, 10, 0, 255, 0, 0, 0, 0, "F/X RESOLUTION", 0, 0 },
    { ewm_text_proc, 180, 50, 60, 10, 0, 255, 0, 0, 0, 0, "Width:", 0, 0 },
    { ewm_text_proc, 180, 65, 60, 8, 0, 255, 0, 0, 0, 0, "Height:", 0, 0 },
    { ewm_box_proc, 238, 48, 62, 11, 0, 255, 0, 0, 0, 0, 0, 0, 0 },
    { ewm_box_proc, 238, 63, 62, 11, 0, 255, 0, 0, 0, 0, 0, 0, 0 },
    { d_edit_proc, 240, 50, 50, 8, ewm_gui_fg_color, ewm_gui_bg_color, 0, 0, 3, 0, width_string, 0, 0 },
    { d_edit_proc, 240, 65, 50, 8, ewm_gui_fg_color, ewm_gui_bg_color, 0, 0, 3, 0, height_string, 0, 0 },
    { ewm_ctext_proc, 240, 80, 100, 10, 0, 255, 0, 0, 0, 0, "PALETTE", 0, 0 },
    { ewm_radio_proc, 180, 95, 100, 8, 0, 255, 0, D_SELECTED, 2, 0, "Fury", 0, 0 },
    { ewm_radio_proc, 180, 110, 100, 8, 0, 255, 0, 0, 2, 0, "Toxic", 0, 0 },
    { ewm_radio_proc, 240, 95, 100, 8, 0, 255, 0, 0, 2, 0, "Ice", 0, 0 },
    { ewm_radio_proc, 240, 110, 100, 8, 0, 255, 0, 0, 2, 0, "Groovy", 0, 0 },
    { ewm_ctext_proc, 160, 1, 320, 10, 255, 0, 0, 0, 0, 0, "- Flame F/X Setup -", 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void fire_setup ( )
{

    Bitmap backImage;
    backImage.Load ( "palette.pcx" );

    set_gfx_mode ( GFX_VGA, 320, 200, 0, 0 );
    set_palette ( black_palette );
    clear_to_color ( screen, 255 );
    fade_in ( backImage.palette, 8 );
    backImage.Destroy ( );

	char *ptr;
	int i, w, h;
    int s_w, s_h;
    int pal;

    gui_fg_color = 15;
    gui_bg_color = 0;

    bool done = false;
    while ( !done )
    {
	    i = do_dialog ( myDialog, -1 );

        // verify data

        // first get the width and height of the video mode
        if ( myDialog[5].flags & D_SELECTED )
        {
        	s_w = 160;
            s_h = 120;
            driver = GFX_VGA;
        }
        else if ( myDialog[6].flags & D_SELECTED )
        {
        	s_w = 320;
            s_h = 200;
            driver = GFX_VGA;
        }
        else if ( myDialog[7].flags & D_SELECTED )
        {
        	s_w = 320;
            s_h = 240;
            driver = GFX_MODEX;
        }
        else if ( myDialog[8].flags & D_SELECTED )
        {
        	s_w = 320;
            s_h = 400;
            driver = GFX_MODEX;
        }
        else if ( myDialog[9].flags & D_SELECTED )
        {
        	s_w = 640;
            s_h = 480;
            driver = GFX_VESA1;
        }

        // next verify the fire width and height
        w = strtol ( width_string, &ptr, 0 );
        h = strtol ( height_string, &ptr, 0 );

        if ( w > 0 && w <= s_w && h > 0 && h <= s_h )
        	done = true;
		else
        {
        	if ( w <= 0 || w > s_w )
	            alert ( "Invalid width", "", "", "Ok", 0, 0, 0 );
        	if ( h <= 0 || h > s_h )
	            alert ( "Invalid height", "", "", "Ok", 0, 0, 0 );
		}

        // Get the palette type
        if ( myDialog[18].flags & D_SELECTED )
        	pal = 0;
        else if ( myDialog[19].flags & D_SELECTED )
        	pal = 1;
        else if ( myDialog[20].flags & D_SELECTED )
        	pal = 2;
        else if ( myDialog[21].flags & D_SELECTED )
        	pal = 3;
    }
    theApp.SetScreen ( w, h, s_w, s_h, driver, pal );
    clear_keybuf ( );
}

void usage ( )
{
	clrscr ( );
    cout << "Flame F/X Version 1.0, Copyright (C) 1999 Jonathan Metzgar\n"
    	 << "Flame F/X comes with ABSOLUTELY NO WARRANTY;\n"
    	 << "This is free software, and you are welcome to redistribute it\n"
    	 << "under certain conditions. See the LICENSE file for more information.\n"
         << "\n\n"
         << "Usage:\n\n"
         << "  F1 - F8 --- These keys select which fire effect to use\n"
         << "  F9      --- This takes a screen shot\n"
         << "  F10     --- This lets you adjust the screen properties\n"
         << "  F11     --- This puts a sprite on the screen ( from splash.pcx )\n"
         << "  F12     --- This puts particles on the screen\n"
         << "  1-4     --- This selects which palette to use\n"
         << "  ESC     --- This quits the program\n"
         << "\n\nPress a key to start Flame F/X!\n";
	getch ( );
}

void start_allegro ( )
{
	allegro_init ( );
    install_timer ( );
    install_keyboard ( );
    install_mouse ( );
}

//----------------------------------------------------------------------------//
// This function loads up a 768 byte file which contains raw palette data.
// The file is encoded with each red, green, and blue elements using one byte.
// R1, G1, B1, R2, G2, B2, R..., G..., B..., Rn, Gn, Bn, etc...
//----------------------------------------------------------------------------//
void LoadPalette ( char *filename, PALETTE palette )
{
	FILE *fin = fopen ( filename, "rb+" );

    unsigned char data;
    for ( int i = 0; i < 256; i++ )
    {
    	data = fgetc ( fin );
        palette[i].r = data;
        data = fgetc ( fin );
        palette[i].g = data;
        data = fgetc ( fin );
        palette[i].b = data;
    }

    fclose ( fin );
}

void FireApp::Init ( )
{
	myWidth = 160;
    myHeight = 120;
    myDriver = GFX_VGA;
    myPalette = 0;
    myEffect = 0;

    // Load up palettes and images.
    splash.Load ( "splash.pcx" );

    LoadPalette ( "pal1.pal", palette[0] );
    LoadPalette ( "pal2.pal", palette[1] );
    LoadPalette ( "pal3.pal", palette[2] );
    LoadPalette ( "pal4.pal", palette[3] );

    db = create_bitmap ( myWidth + 1, myHeight + 1 );
}

void FireApp::Kill ( )
{
	splash.Destroy ( );
}

void FireApp::Play ( )
{
	set_gfx_mode ( myDriver, myWidth, myHeight, 0, 0 );
    set_palette ( palette[myPalette] );

    InitGraphics ( );

    while ( !key[KEY_ESC] )
    {
    	// update the double buffer with the current effect
    	switch ( myEffect )
        {
        case 0: FX1 ( ); break;
        case 1: FX2 ( ); break;
        case 2: FX3 ( ); break;
        case 3: FX4 ( ); break;
        case 4: FX5 ( ); break;
        case 5: FX6 ( ); break;
        case 6: FX7 ( ); break;
        case 7: FX8 ( ); break;
        }

        // prompt the user for a filename and save a screen shot
        if ( key[KEY_F9] )
        {
        	// screen shot
            BITMAP *ss = create_bitmap ( Vw, Vh );
            blit ( db, ss, 0, 0, 0, 0, Vw, Vh );
            set_gfx_mode ( GFX_VGA, 320, 200, 0, 0 );
            set_palette ( desktop_palette );
            char filename[80] = "";
            if ( file_select ( "Enter Image File", filename, "PCX;BMP;TGA" ) )
            {
            	save_bitmap ( filename, ss, palette[myPalette] );
            }
            destroy_bitmap ( ss );
            clear ( screen );
            clear_keybuf ( );

            set_gfx_mode ( myDriver, myWidth, myHeight, 0, 0 );
			set_palette ( palette[myPalette] );
        }

        // check to see which effect to use
        int oldEffect = myEffect;
        if ( key[KEY_F1] ) myEffect = 0;
        if ( key[KEY_F2] ) myEffect = 1;
        if ( key[KEY_F3] ) myEffect = 2;
        if ( key[KEY_F4] ) myEffect = 3;
        if ( key[KEY_F5] ) myEffect = 4;
        if ( key[KEY_F6] ) myEffect = 5;
        if ( key[KEY_F7] ) myEffect = 6;
        if ( key[KEY_F8] ) myEffect = 7;
        if ( oldEffect != myEffect )
        	clear ( db );

        // go into the setup menu
        if ( key[KEY_F10] )
        {
	        fire_setup ( );
			set_gfx_mode ( myDriver, myWidth, myHeight, 0, 0 );
		    set_palette ( palette[myPalette] );
			InitGraphics ( );
        }

        // plaster a sprite at random positions on the double-buffer
        if ( key[KEY_F11] )
        {
        	int cx, cy;

            // make sure the image is centered on the screen.
            cx = ( db->w >> 1 ) - ( splash.bmp->w >> 1 );
            cy = ( db->w >> 1 ) - ( splash.bmp->w >> 1 );
		    draw_sprite ( db, splash.bmp, cx, cy );
        }

        // plot random blocks on the double buffers ( fire particles )
		if ( key[KEY_F12] )
        {
        	int w1, w2, h1, h2;
        	for ( int i = 0; i < 35; i++ )
            {
            	w1 = random () % db->w;
                w2 = w1 + 2;
            	h1 = random () % db->h;
                h2 = h1 + 2;
            	rectfill ( db, w1, h1, w2, h2, 192 );
            }
        }

        if ( key[KEY_1] )
        {
	        myPalette = 0;
    	    set_palette ( palette[myPalette] );
        }
        if ( key[KEY_2] )
        {
	        myPalette = 1;
    	    set_palette ( palette[myPalette] );
        }
        if ( key[KEY_3] )
        {
	        myPalette = 2;
    	    set_palette ( palette[myPalette] );
        }
        if ( key[KEY_4] )
        {
	        myPalette = 3;
    	    set_palette ( palette[myPalette] );
        }

        // wait for vertical sync and then copy the double-buffer ( keeps frame rate
        // locked at 60-70 FPS on fast machines.
        vsync ( );
        blit ( db, screen, 0, 0, 0, 0, Vw-1, Vh-1 );
    }
}

void FireApp::SetScreen ( int vw, int vh, int width, int height, int driver, int pal )
{
	// set some variables needed for the fire class
	myWidth = width;
    myHeight = height;
    myDriver = driver;
    myPalette = pal;
    Vw = vw;
    Vh = vh;
}

void FireApp::InitGraphics ( )
{
	// create the double buffer
    destroy_bitmap ( db );
    db = create_bitmap ( Vw + 2, Vh + 2 );
}

// cool_random ( )
// returns -1, 0, or 1 randomly
int cool_random ( )
{
    int n = random () % 3;
    switch ( n )
    {
    case 0:
		return -1;
    case 1:
		return 0;
    case 2:
		return 1;
    }
    return 0;
}

/*
* 8 fire effects follow.
* Most effects offer Filters that smooth the fire effect out.
* I will document these later
*/
void FireApp::FX1 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y <= Vh; y++ )
    {
    	for ( int x = 0; x <= Vw; x++ )
        {
        	int data = db->line[y][x];
            data -= 4;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

void FireApp::FX2 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y <= Vh; y++ )
    {
    	for ( int x = 0; x <= Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data += db->line[y+1][x+1];
            data += db->line[y][x-1];
            data >>= 2;
            data -= 1;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

void FireApp::FX3 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y < Vh; y++ )
    {
    	for ( int x = 0; x < Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data >>= 1;
            data--;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x + cool_random () ] = data;
        }
    }
}

void FireApp::FX4 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y <= Vh; y++ )
    {
    	for ( int x = 0; x <= Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data += db->line[y][x+1];
            data += db->line[y][x-1];
            data >>= 2;
            data -= 1;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

void FireApp::FX5 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y <= Vh; y++ )
    {
    	for ( int x = 0; x <= Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data += db->line[y][x+1];
            data += db->line[y+1][x-1];
            data >>= 2;
            data -= 1;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

void FireApp::FX6 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y < Vh; y++ )
    {
    	for ( int x = 0; x < Vw; x++ )
        {
        	int data = db->line[y][x];
            data-=2;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x + cool_random () ] = data;
        }
    }
}

void FireApp::FX7 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y < Vh; y++ )
    {
    	for ( int x = 0; x < Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data >>= 1;
            data-=2;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

void FireApp::FX8 ( )
{
	for ( int x = 0; x < Vw; x++ )
    {
    	line[x] = random () % 256;
        db->line[Vh][x] = line[x];
        db->line[Vh-1][x] = line[x] - 4;
    }

    for ( int y = 1; y <= Vh; y++ )
    {
    	for ( int x = 0; x <= Vw; x++ )
        {
        	int data = db->line[y][x];
            data += db->line[y+1][x];
            data += db->line[y][x+1];
            data += db->line[y][x-1];
            data += db->line[y+1][x+1];
            data += db->line[y+1][x-1];
            data /= 6;
            data -= 1;
            if ( data < 0 )
            	data = 0;
        	db->line[y-1][x] = data;
        }
    }
}

Bitmap::Bitmap ( )
{
	bmp = NULL;
}

Bitmap::~Bitmap ( )
{
	if ( bmp )
    	destroy_bitmap ( bmp );
}

void Bitmap::Load ( char *filename )
{
	if ( bmp )
    	destroy_bitmap ( bmp );
    bmp = load_bitmap ( filename, palette );
}

void Bitmap::Save ( char *filename )
{
	if ( bmp )
    	save_bitmap ( filename, bmp, palette );
}

void Bitmap::Destroy ( )
{
	if ( bmp )
    	destroy_bitmap ( bmp );
    bmp = NULL;
}
