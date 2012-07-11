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

#ifndef __PAROLE_STATUSBAR_H
#define __PAROLE_STATUSBAR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PAROLE_TYPE_STATUSBAR        (parole_statusbar_get_type () )
#define PAROLE_STATUSBAR(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), PAROLE_TYPE_STATUSBAR, ParoleStatusbar))
#define PAROLE_IS_STATUSBAR(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), PAROLE_TYPE_STATUSBAR))

typedef struct ParoleStatusbarPrivate ParoleStatusbarPrivate;

typedef struct
{
    GObject         		parent;
    ParoleStatusbarPrivate     *priv;
    
} ParoleStatusbar;

typedef struct
{
    GObjectClass 		parent_class;
    
} ParoleStatusbarClass;

GType        			parole_statusbar_get_type        (void) G_GNUC_CONST;

ParoleStatusbar       	       *parole_statusbar_new             (void);

void				parole_statusbar_set_visible 	 (ParoleStatusbar *bar,
								  gboolean visible);

G_END_DECLS

#endif /* __PAROLE_STATUSBAR_H */
