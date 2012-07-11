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

#ifndef __RC_UTILS_
#define __RC_UTILS_

#include <libxfce4util/libxfce4util.h>

#define PAROLE_RESOURCE_FILE 		"xfce4/parole/parole-media-player.rc"
#define PAROLE_HISTORY_FILE 		"xfce4/parole/history"

#define PAROLE_RC_GROUP_GENERAL		"General"
#define PAROLE_RC_GROUP_PLUGINS		"Plugins"

XfceRc                 *parole_get_resource_file        (const gchar *group, 
							 gboolean readonly);

void			parole_rc_write_entry_bool	(const gchar *property,
							 const gchar *group,
							 gboolean value);

void			parole_rc_write_entry_int	(const gchar *property,
							 const gchar *group,
							 gint value);
							 
void 			parole_rc_write_entry_string	(const gchar *property, 
							 const gchar *group,
							 const gchar *value);

void 			parole_rc_write_entry_list	(const gchar *property, 
							 const gchar *group,
							 gchar **value);
							 
gboolean 		parole_rc_read_entry_bool	(const gchar *property,
							 const gchar *group,
							 gboolean fallback);

gint			parole_rc_read_entry_int	(const gchar *property,
							 const gchar *group,
							 gint fallback);

const gchar	       *parole_rc_read_entry_string	(const gchar *property,
							 const gchar *group,
							 const gchar *fallback);

gchar	       	      **parole_rc_read_entry_list	(const gchar *property,
							 const gchar *group);

gchar                 **parole_get_history	        (void);

gchar                 **parole_get_history_full	        (const gchar *relpath);

void			parole_insert_line_history	(const gchar *line);		

void			parole_insert_line_history_full	(const gchar *relpath,
							 const gchar *line);		

void 			parole_clear_history_file	(void);

void 			parole_clear_history_file_full  (const gchar *relpath);

#endif /* __RC_UTILS_ */
