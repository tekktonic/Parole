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

#ifndef __PAROLE_PLUGIN_PLAYER_H
#define __PAROLE_PLUGIN_PLAYER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PAROLE_TYPE_PLUGIN_PLAYER        (parole_plugin_player_get_type () )
#define PAROLE_PLUGIN_PLAYER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), PAROLE_TYPE_PLUGIN_PLAYER, ParolePluginPlayer))
#define PAROLE_IS_PLUGIN_PLAYER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), PAROLE_TYPE_PLUGIN_PLAYER))

typedef struct ParolePluginPlayerPrivate ParolePluginPlayerPrivate;

typedef struct
{
    GObject         		   parent;
    ParolePluginPlayerPrivate     *priv;
    
} ParolePluginPlayer;

typedef struct
{
    GObjectClass 		   parent_class;
    
} ParolePluginPlayerClass;

GType        			   parole_plugin_player_get_type        (void) G_GNUC_CONST;

ParolePluginPlayer                *parole_plugin_player_new             (void);

G_END_DECLS

#endif /* __PAROLE_PLUGIN_PLAYER_H */
