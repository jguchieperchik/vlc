/*****************************************************************************
 * gtk2_window.cpp: GTK2 implementation of the Window class
 *****************************************************************************
 * Copyright (C) 2003 VideoLAN
 * $Id: gtk2_window.cpp,v 1.29 2003/04/28 12:00:13 asmax Exp $
 *
 * Authors: Cyril Deguet     <asmax@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111,
 * USA.
 *****************************************************************************/

#ifdef GTK2_SKINS

//--- GENERAL ---------------------------------------------------------------
//#include <math.h>

//--- VLC -------------------------------------------------------------------
#include <vlc/intf.h>

//--- GTK2 ------------------------------------------------------------------
#include <gdk/gdk.h>
#include <glib.h>

//--- SKIN ------------------------------------------------------------------
#include "../os_api.h"
#include "../src/anchor.h"
#include "../controls/generic.h"
#include "../src/window.h"
#include "../os_window.h"
#include "../src/event.h"
#include "../os_event.h"
#include "../src/graphics.h"
#include "../os_graphics.h"
#include "../src/skin_common.h"
#include "../src/theme.h"


//---------------------------------------------------------------------------
// Skinable Window
//---------------------------------------------------------------------------
GTK2Window::GTK2Window( intf_thread_t *p_intf, GdkWindow *gwnd, int x, int y,
    bool visible, int transition, int normalalpha, int movealpha,
    bool dragdrop, string name )
    : SkinWindow( p_intf, x, y, visible, transition, normalalpha, movealpha,
              dragdrop )
{
    // Set handles
    gWnd           = gwnd;
    gc = gdk_gc_new( gwnd );

    Name        = name;

    LButtonDown = false;
    RButtonDown = false;

    // Removing fading effect
    Transition  = 0;
/*
    // Set position parameters
    CursorPos    = new POINT;
    WindowPos    = new POINT;

    // Create Tool Tip Window
    ToolTipWindow = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWnd, 0, GetModuleHandle( NULL ), 0);

    // Create Tool Tip infos
    ToolTipInfo.cbSize = sizeof(TOOLINFO);
    ToolTipInfo.uFlags = TTF_SUBCLASS|TTF_IDISHWND;
    ToolTipInfo.hwnd = hWnd;
    ToolTipInfo.hinst = GetModuleHandle( NULL );
    ToolTipInfo.uId = (unsigned int)hWnd;
    ToolTipInfo.lpszText = NULL;
    ToolTipInfo.rect.left = ToolTipInfo.rect.top = 0;
        ToolTipInfo.rect.right = ToolTipInfo.rect.bottom = 0;

    SendMessage( ToolTipWindow, TTM_ADDTOOL, 0,
                    (LPARAM)(LPTOOLINFO) &ToolTipInfo );
*/
    if( DragDrop )
    {
        // register the listview as a drop target
        DropObject = new GTK2DropObject( p_intf );
        gdk_window_register_dnd( gwnd );
    }

    // Create Tool Tip window
/*    GdkWindowAttr attr;
    attr.event_mask = GDK_ALL_EVENTS_MASK;
    attr.width = 100;
    attr.height = 100;
    attr.window_type = GDK_WINDOW_CHILD;
    attr.wclass = GDK_INPUT_OUTPUT;
    gint mask = 0;
    ToolTipWindow = gdk_window_new( gwnd, &attr, mask);*/

}
//---------------------------------------------------------------------------
GTK2Window::~GTK2Window()
{
/*    delete CursorPos;
    delete WindowPos;

    if( hWnd != NULL )
    {
        DestroyWindow( hWnd );
    }
    if( ToolTipWindow != NULL )
    {
        DestroyWindow( ToolTipWindow );
    }
    if( DragDrop )
    {
        // Remove the listview from the list of drop targets
        RevokeDragDrop( hWnd );
        DropTarget->Release();
        // Uninitialize the OLE library
        OleUninitialize();
    }*/
    if( gWnd )
    {
        gdk_window_destroy( gWnd );
    }
}
//---------------------------------------------------------------------------
void GTK2Window::OSShow( bool show )
{
    if( show )
    {
        gdk_window_show( gWnd );
        gdk_window_move( gWnd, Left, Top );
    }
    else
    {
        gdk_window_hide( gWnd );
    }
}
//---------------------------------------------------------------------------
bool GTK2Window::ProcessOSEvent( Event *evt )
{
    unsigned int msg = evt->GetMessage();
    unsigned int p1  = evt->GetParam1();
    int          p2  = evt->GetParam2();

    switch( msg )
    {
        case GDK_EXPOSE:
            RefreshFromImage( 0, 0, Width, Height );
            return true;
 
        case GDK_MOTION_NOTIFY:
            if( LButtonDown )
                MouseMove( (int)( (GdkEventButton *)p2 )->x,
                           (int)( (GdkEventButton *)p2 )->y, 1 );
            else if( RButtonDown )
                MouseMove( (int)( (GdkEventButton *)p2 )->x,
                           (int)( (GdkEventButton *)p2 )->y, 2 );
            else
                MouseMove( (int)( (GdkEventButton *)p2 )->x,
                           (int)( (GdkEventButton *)p2 )->y, 0 );
            gdk_window_get_pointer( gWnd, 0, 0, 0 );
            return true;

        case GDK_BUTTON_PRESS:
            // Raise all the windows
            for( list<SkinWindow *>::const_iterator win = 
                    p_intf->p_sys->p_theme->WindowList.begin();
                    win != p_intf->p_sys->p_theme->WindowList.end(); win++ )
            {
                gdk_window_raise( ( (GTK2Window *)(*win) )->GetHandle() );
            }
          
            switch( ( (GdkEventButton *)p2 )->button )
            {
                case 1:
                    // Left button
                    LButtonDown = true;
                    MouseDown( (int)( (GdkEventButton *)p2 )->x,
                               (int)( (GdkEventButton *)p2 )->y, 1 );
                    break;

                case 3:
                    // Right button
                    RButtonDown = true;
                    MouseDown( (int)( (GdkEventButton *)p2 )->x,
                               (int)( (GdkEventButton *)p2 )->y, 2 );
                    break;

                default:
                    break;
            }
            return true;

        case GDK_BUTTON_RELEASE:
            switch( ( (GdkEventButton *)p2 )->button )
            {
                case 1:
                    // Left button
                    LButtonDown = false;
                    MouseUp( (int)( (GdkEventButton *)p2 )->x,
                             (int)( (GdkEventButton *)p2 )->y, 1 );
                    break;

                case 3:
                    // Right button
                    RButtonDown = false;
                    MouseUp( (int)( (GdkEventButton *)p2 )->x,
                             (int)( (GdkEventButton *)p2 )->y, 2 );
                    break;

                default:
                    break;
            }
            return true;

        case GDK_LEAVE_NOTIFY:
            OSAPI_PostMessage( this, WINDOW_LEAVE, 0, 0 );
            return true;

        case GDK_2BUTTON_PRESS:
            MouseDblClick( (int)( (GdkEventButton *)p2 )->x,
                           (int)( (GdkEventButton *)p2 )->y, 1 );
            return true;

        case GDK_DROP_START:
            DropObject->HandleDropStart( ( (GdkEventDND *)p2 )->context );
            return true;

        case GDK_SCROLL:
            switch( ( (GdkEventScroll *)p2 )->direction )
            {
                case GDK_SCROLL_UP:
                    MouseScroll( ( (GdkEventScroll *)p2 )->x,
                                 ( (GdkEventScroll *)p2 )->y,
                                 MOUSE_SCROLL_UP);
                    break;
                case GDK_SCROLL_DOWN:
                    MouseScroll( ( (GdkEventScroll *)p2 )->x,
                                 ( (GdkEventScroll *)p2 )->y,
                                 MOUSE_SCROLL_DOWN);
                    break;
            }
            return true;

        default:
            return false;
    }
}
//---------------------------------------------------------------------------
void GTK2Window::SetTransparency( int Value )
{
/*    if( Value > -1 )
        Alpha = Value;
    SetLayeredWindowAttributes( hWnd, 0, Alpha, LWA_ALPHA | LWA_COLORKEY );
    UpdateWindow( hWnd );*/
}
//---------------------------------------------------------------------------
void GTK2Window::RefreshFromImage( int x, int y, int w, int h )
{
    // Initialize painting
/*    HDC DC = GetWindowDC( hWnd );

    // Draw image on window
    BitBlt( DC, x, y, w, h, ( (GTK2Graphics *)Image )->GetImageHandle(),
            x, y, SRCCOPY );

    // Release window device context
    ReleaseDC( hWnd, DC );

*/ 
    GdkDrawable *drawable = (( GTK2Graphics* )Image )->GetImage();
    GdkImage *image = gdk_drawable_get_image( drawable, 0, 0, Width, Height );
    
    gdk_draw_drawable( gWnd, gc, drawable, x, y, x, y, w, h );

    // Mask for transparency
    GdkRegion *region = gdk_region_new();
    for( int line = 0; line < Height; line++ )
    {
        int start = 0, end = 0;
        while( start < Width )
        {
            while( start < Width && gdk_image_get_pixel( image, start, line ) == 0 )
            {
                start++;
            } 
            end = start;
            while( end < Width && gdk_image_get_pixel( image, end, line ) != 0)
            {
                end++;
            }
            GdkRectangle rect;
            rect.x = start;
            rect.y = line;
            rect.width = end - start + 1;
            rect.height = 1;
            GdkRegion *rectReg = gdk_region_rectangle( &rect );
            gdk_region_union( region, rectReg );
            gdk_region_destroy( rectReg );
            start = end + 1;
        }
    }
    gdk_window_shape_combine_region( gWnd, region, 0, 0 );
    gdk_region_destroy( region );
}
//---------------------------------------------------------------------------
void GTK2Window::WindowManualMove()
{
    // Get mouse cursor position
    int x, y;
    OSAPI_GetMousePos( x, y );

    // Move window and chek for magnetism
    p_intf->p_sys->p_theme->MoveSkinMagnet( this,
        WindowX + x - CursorX, WindowY + y - CursorY );

}
//---------------------------------------------------------------------------
void GTK2Window::WindowManualMoveInit()
{
    gdk_window_get_pointer( gdk_get_default_root_window(), &CursorX, &CursorY,
                            NULL );
    WindowX = Left;
    WindowY = Top;
}
//---------------------------------------------------------------------------
void GTK2Window::Move( int left, int top )
{
    Left = left;
    Top  = top;
    gdk_window_move( gWnd, left, top );
}
//---------------------------------------------------------------------------
void GTK2Window::Size( int width, int height )
{
    Width  = width;
    Height = height;
    gdk_window_resize( gWnd, width, height );
}
//---------------------------------------------------------------------------
void GTK2Window::ChangeToolTipText( string text )
{
/*    if( text == "none" )
    {
        if( ToolTipText != "none" )
        {
            ToolTipText = "none";
            ToolTipInfo.lpszText = NULL;
            SendMessage( ToolTipWindow, TTM_ACTIVATE, 0 , 0 );
        }
    }
    else
    {
        if( text != ToolTipText )
        {
            ToolTipText = text;
            ToolTipInfo.lpszText = (char *)ToolTipText.c_str();
            SendMessage( ToolTipWindow, TTM_ACTIVATE, 1 , 0 );
            SendMessage( ToolTipWindow, TTM_UPDATETIPTEXT, 0,
                             (LPARAM)(LPTOOLINFO)&ToolTipInfo );
        }
    }
*/
}
//---------------------------------------------------------------------------

#endif
