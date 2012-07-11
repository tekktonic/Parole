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

#ifndef __PAROLE_CONF_H
#define __PAROLE_CONF_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PAROLE_TYPE_CONF        (parole_conf_get_type () )
#define PAROLE_CONF(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), PAROLE_TYPE_CONF, ParoleConf))
#define PAROLE_IS_CONF(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), PAROLE_TYPE_CONF))

   
typedef struct ParoleConfPrivate ParoleConfPrivate;

typedef struct
{
    GObject         		 parent;
    ParoleConfPrivate     	*priv;
    
} ParoleConf;

typedef struct
{
    GObjectClass 		 parent_class;
    
} ParoleConfClass;

GType        			 parole_conf_get_type        	(void) G_GNUC_CONST;

ParoleConf       		*parole_conf_new             	(void);

gboolean			 parole_conf_get_property_bool  (ParoleConf *conf,
								 const gchar *name);

G_END_DECLS

#endif /* __PAROLE_CONF_H */
