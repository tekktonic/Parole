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

#if !defined (__PAROLE_H_INSIDE__) && !defined (PAROLE_COMPILATION)
#error "Only <parole.h> can be included directly."
#endif

#ifndef __PAROLE_MEDIA_FILE_H
#define __PAROLE_MEDIA_FILE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PAROLE_TYPE_FILE        (parole_file_get_type () )
#define PAROLE_FILE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), PAROLE_TYPE_FILE, ParoleFile))
#define PAROLE_IS_FILE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), PAROLE_TYPE_FILE))

typedef struct _ParoleFile      ParoleFile;
typedef struct _ParoleFileClass ParoleFileClass;

struct _ParoleFile
{
    GObject         		parent;
};

struct _ParoleFileClass
{
    GObjectClass 		parent_class;
};

GType        			parole_file_get_type        	  (void) G_GNUC_CONST;

ParoleFile       	       *parole_file_new             	  (const gchar *filename);

ParoleFile       	       *parole_file_new_with_display_name (const gchar *filename,
								   const gchar *display_name);

const gchar   G_CONST_RETURN   *parole_file_get_file_name 	  (const ParoleFile *file) G_GNUC_PURE;

const gchar   G_CONST_RETURN   *parole_file_get_display_name 	  (const ParoleFile *file) G_GNUC_PURE;

const gchar   G_CONST_RETURN   *parole_file_get_uri 		  (const ParoleFile *file) G_GNUC_PURE;

const gchar   G_CONST_RETURN   *parole_file_get_content_type      (const ParoleFile *file) G_GNUC_PURE;

G_END_DECLS

#endif /* __PAROLE_MEDIA_FILE_H */
