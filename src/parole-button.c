/*
 * * Copyright (C) 2009-2011 Ali <aliov@xfce.org>
 *
 * Licensed under the GNU General Public License Version 2
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


/*
 * Based on code from gpm-button (gnome power manager)
 * Copyright (C) 2006-2007 Richard Hughes <richard@hughsie.com>
 * 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_XF86_KEYSYM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/X.h>
#include <X11/XF86keysym.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <glib.h>

#include <libxfce4util/libxfce4util.h>

#include "parole-button.h"
#include "parole-debug.h"
#include "enum-gtypes.h"

static void parole_button_finalize   (GObject *object);

#define PAROLE_BUTTON_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE((o), PAROLE_TYPE_BUTTON, ParoleButtonPrivate))

static struct
{
    ParoleButtonKey    key;
    guint              key_code;
} parole_key_map [PAROLE_KEY_NUMBERS] = { {0, 0}, };

struct ParoleButtonPrivate
{
    GdkScreen	*screen;
    GdkWindow   *window;
    
};

enum
{
    BUTTON_PRESSED,
    LAST_SIGNAL
};

#define DUPLICATE_SHUTDOWN_TIMEOUT 4.0f

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(ParoleButton, parole_button, G_TYPE_OBJECT)

static guint
parole_button_get_key (unsigned int keycode)
{
    ParoleButtonKey key = PAROLE_KEY_UNKNOWN;
    
    guint i;
    
    for ( i = 0; i < G_N_ELEMENTS (parole_key_map); i++)
    {
	if ( parole_key_map [i].key_code == keycode )
	    key = parole_key_map [i].key;
    }
    
    return key;
}

static GdkFilterReturn
parole_button_filter_x_events (GdkXEvent *xevent, GdkEvent *ev, gpointer data)
{
    ParoleButtonKey key;
    ParoleButton *button;
    
    XEvent *xev = (XEvent *) xevent;
    
    if ( xev->type != KeyPress )
    	return GDK_FILTER_CONTINUE;
    
    key = parole_button_get_key (xev->xkey.keycode);
    
    if ( key != PAROLE_KEY_UNKNOWN )
    {
	button = (ParoleButton *) data;
    
	PAROLE_DEBUG_ENUM ("Key press", key, ENUM_GTYPE_BUTTON_KEY);
    
	g_signal_emit (G_OBJECT(button), signals[BUTTON_PRESSED], 0, key);
	return GDK_FILTER_REMOVE;
    }
    
    return GDK_FILTER_CONTINUE;
}

static gboolean
parole_button_grab_keystring (ParoleButton *button, guint keycode)
{
    Display *display;
    guint ret;
    guint modmask = 0;
    
    display = GDK_DISPLAY ();
    
    gdk_error_trap_push ();

    ret = XGrabKey (display, keycode, modmask,
		    GDK_WINDOW_XID (button->priv->window), True,
		    GrabModeAsync, GrabModeAsync);
		    
    if ( ret == BadAccess )
    {
	g_warning ("Failed to grab modmask=%u, keycode=%li",
		    modmask, (long int) keycode);
	return FALSE;
    }
	
    ret = XGrabKey (display, keycode, LockMask | modmask,
		    GDK_WINDOW_XID (button->priv->window), True,
		    GrabModeAsync, GrabModeAsync);
			
    if (ret == BadAccess)
    {
	g_warning ("Failed to grab modmask=%u, keycode=%li",
		   LockMask | modmask, (long int) keycode);
	return FALSE;
    }

    gdk_flush ();
    gdk_error_trap_pop ();
    return TRUE;
}


static gboolean
parole_button_xevent_key (ParoleButton *button, guint keysym , ParoleButtonKey key)
{
    guint keycode = XKeysymToKeycode (GDK_DISPLAY(), keysym);

    if ( keycode == 0 )
    {
	g_warning ("could not map keysym %x to keycode\n", keysym);
	return FALSE;
    }
    
    if ( !parole_button_grab_keystring(button, keycode)) 
    {
    	g_warning ("Failed to grab %i\n", keycode);
	return FALSE;
    }
    
    PAROLE_DEBUG_ENUM_FULL (key, ENUM_GTYPE_BUTTON_KEY, "Grabbed key %li ", (long int) keycode);
    
    parole_key_map [key].key_code = keycode;
    parole_key_map [key].key = key;
    
    return TRUE;
}

static void
parole_button_setup (ParoleButton *button)
{
    button->priv->screen = gdk_screen_get_default ();
    button->priv->window = gdk_screen_get_root_window (button->priv->screen);
    
    parole_button_xevent_key (button, XF86XK_AudioPlay, PAROLE_KEY_AUDIO_PLAY);
    parole_button_xevent_key (button, XF86XK_AudioStop, PAROLE_KEY_AUDIO_STOP);
    parole_button_xevent_key (button, XF86XK_AudioPrev, PAROLE_KEY_AUDIO_PREV);
    parole_button_xevent_key (button, XF86XK_AudioNext, PAROLE_KEY_AUDIO_NEXT);

    gdk_window_add_filter (button->priv->window, 
			   parole_button_filter_x_events, button);
}

static void
parole_button_class_init(ParoleButtonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    signals [BUTTON_PRESSED] = 
        g_signal_new ("button-pressed",
                      PAROLE_TYPE_BUTTON,
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (ParoleButtonClass, button_pressed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__ENUM,
                      G_TYPE_NONE, 1, ENUM_GTYPE_BUTTON_KEY);

    object_class->finalize = parole_button_finalize;

    g_type_class_add_private (klass, sizeof (ParoleButtonPrivate));
}

static void
parole_button_init (ParoleButton *button)
{
    button->priv = PAROLE_BUTTON_GET_PRIVATE (button);
    
    button->priv->screen = NULL;
    button->priv->window = NULL;
    
    parole_button_setup (button);
}

static void
parole_button_finalize (GObject *object)
{
    G_OBJECT_CLASS(parole_button_parent_class)->finalize(object);
}

ParoleButton *
parole_button_new (void)
{
    ParoleButton *button = NULL;
    
    button = g_object_new (PAROLE_TYPE_BUTTON, NULL);
    
    return button;
}

#endif /*HAVE_XF86_KEYSYM*/
