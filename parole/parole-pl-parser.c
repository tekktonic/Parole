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
 * Based on code from:
 * 
 * Totem pl parser:	Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Bastien Nocera
 * 		        Copyright (C) 2003, 2004 Colin Walters <walters@rhythmbox.org>
 * 
 * xfmedia:		Copyright (c) 2004-2005 Brian Tarricone, <bjt23@cornell.edu>
 * 
 * Thanks :).
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>
#include <libxfce4util/libxfce4util.h>

#include <parole/parole-file.h>

#include "parole-pl-parser.h"
#include "parole-filters.h"
#include "parole-debug.h"
#include "parole-enum-types.h"

typedef struct
{
    GSList    *list;
    gboolean  started;
    gchar    *uri;
    gchar    *title;
    
} ParoleParserData;

static void
parole_xspf_xml_start (GMarkupParseContext *context, const gchar *element_name,
		       const gchar **attribute_names, const gchar **attribute_values,
		       gpointer user_data, GError **error)
{
    ParoleParserData *data = user_data;
    
    if ( !data->started )
    {
	if (!g_ascii_strcasecmp (element_name, "playlist") )
	    data->started = TRUE;
    }
}

static void
parole_xspf_xml_text (GMarkupParseContext *context, const gchar *text, gsize text_len,  
		      gpointer user_data, GError **error)
{
    ParoleParserData *data = user_data;
    const gchar *element_name;
    
    if (!data->started)
        return;
    
    element_name = g_markup_parse_context_get_element (context);
    
    if (!g_ascii_strcasecmp (element_name, "location") ) 
    {
        if (data->uri) 
	{
            g_free (data->uri);
            data->uri = NULL;
        }
	
        if (text_len > 0)
            data->uri = g_strdup (text);
    }
    else if (!g_ascii_strcasecmp (element_name, "title") ) 
    {
        if (data->title) 
	{
            g_free (data->title);
            data->title = NULL;
        }
	
        if (text_len > 0)
            data->title = g_strdup (text);
    }
}

static void
parole_xspf_xml_end (GMarkupParseContext *context, const gchar *element_name,
		     gpointer user_data, GError **error)
{
    ParoleParserData *data = user_data;
    ParoleFile *file;
    
    if (! g_ascii_strcasecmp (element_name, "playlist"))
        data->started = FALSE;
	
    if (!data->started)
        return;
	
    if (!g_ascii_strcasecmp (element_name, "track")) 
    {
        if (data->uri) 
	{
	    file = parole_file_new_with_display_name (data->uri, data->title);
	    data->list = g_slist_append (data->list, file);
	    
	    g_free (data->uri);
	    data->uri = NULL;
	}
	
	if ( data->title)
	{
	    g_free (data->title);
	    data->title = NULL;
	}
    }
}

static void
parole_asx_xml_start (GMarkupParseContext *context, const gchar *element_name,
		      const gchar **attribute_names, const gchar **attribute_values,
		      gpointer user_data, GError **error)
{
    guint i;
    ParoleParserData *data = user_data;
    
    if ( !data->started )
    {
	if (!g_ascii_strcasecmp (element_name, "asx") )
	    data->started = TRUE;
	else
	    return;
    }
    
    if (!g_ascii_strcasecmp (element_name, "ref") ) 
    {
        if (data->uri) 
	{
            g_free (data->uri);
            data->uri = NULL;
        }
	
        for ( i = 0; attribute_names[i]; i++) 
	{
            if ( !g_ascii_strcasecmp (attribute_names[i], "href")) 
	    {
                data->uri = g_strdup (attribute_values[i]);
                break;
            }
        }
    }
}

static void
parole_asx_xml_text (GMarkupParseContext *context, const gchar *text, gsize text_len,  
		     gpointer user_data, GError **error)
{
    ParoleParserData *data = user_data;
    const gchar *element_name;
    
    if (!data->started)
        return;
    
    element_name = g_markup_parse_context_get_element (context);
    
    if (!g_ascii_strcasecmp (element_name, "title") ) 
    {
        if (data->title) 
	{
            g_free (data->title);
            data->title = NULL;
        }
	
        if (text_len > 0)
            data->title = g_strdup (text);
    }
}

static void
parole_asx_xml_end (GMarkupParseContext *context, const gchar *element_name,
		    gpointer user_data, GError **error)
{
    ParoleParserData *data = user_data;
    ParoleFile *file;
    
    if (!g_ascii_strcasecmp (element_name, "ASX"))
        data->started = FALSE;
	
    if (!data->started)
        return;
	
    if (!g_ascii_strcasecmp (element_name, "entry")) 
    {
        if (data->uri) 
	{
	    file = parole_file_new_with_display_name (data->uri, data->title);
	    data->list = g_slist_append (data->list, file);
	    
	    g_free (data->uri);
	    data->uri = NULL;
	}
	
	if ( data->title)
	{
	    g_free (data->title);
	    data->title = NULL;
	}
    }
}

static GSList *
parole_pl_parser_parse_asx (const gchar *filename)
{
    ParoleParserData data;
    GFile *file;
    gchar *contents;
    GError *error = NULL;
    gsize size;
    GMarkupParseContext *pctx;
    GMarkupParser parser = {
        parole_asx_xml_start,
        parole_asx_xml_end,
        parole_asx_xml_text,
        NULL,
        NULL
    };
    
    data.list = NULL;
    data.title = data.uri = NULL;
    
    file = g_file_new_for_path (filename);
    
    if ( !g_file_load_contents (file, NULL, &contents, &size, NULL, NULL) )
	goto out;
    
    if ( g_utf8_validate (contents, -1, NULL) == FALSE) 
    {
	gchar *fixed;
	fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
	if (fixed != NULL) 
	{
	    g_free (contents);
	    contents = fixed;
	}
    }
    
    pctx = g_markup_parse_context_new (&parser, 0, &data, NULL);
    
    if ( !g_markup_parse_context_parse (pctx, contents, size, &error) )
    {
	if ( error )
	{
	    g_critical ("Unable to parse asx file : %s : %s\n", filename, error->message);
	    g_error_free (error);
	}
    }
    else
    {
	if ( !g_markup_parse_context_end_parse (pctx, &error) )
	{
            g_critical ("Unable to finish parsing ASX playlist file %s", error->message);
            g_error_free (error);
        }
    }
    
    g_markup_parse_context_free (pctx);
    
out:
    g_object_unref (file);
    return data.list;
}

static GSList *
parole_pl_parser_parse_m3u (const gchar *filename)
{
    GFile *file;
    gchar **lines;
    gchar *contents;
    GSList *list = NULL;
    gsize size;
    guint num_lines;
    const gchar *split_char;
    guint i;
    
    file = g_file_new_for_path (filename);
    
    if ( !g_file_load_contents (file, NULL, &contents, &size, NULL, NULL) )
	goto out;
    
    if ( g_utf8_validate (contents, -1, NULL) == FALSE) 
    {
	gchar *fixed;
	fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
	if (fixed != NULL) 
	{
	    g_free (contents);
	    contents = fixed;
	}
    }
    
    if ( strstr (contents,"\x0d") == NULL) 
    {
	split_char = "\n";
    } 
    else 
    {
	split_char = "\x0d\n";
    }
    
    lines = g_strsplit (contents, split_char, 0);
    g_free (contents);
    
    num_lines = g_strv_length (lines);
    num_lines--; /* Drop the terminating NULL */

    for ( i = 0; lines[i] != NULL; i++)
    {
	if ( lines[i][0] == '\0' || lines[i][0] == '#')
	    continue;

	list = g_slist_append (list, parole_file_new (lines[i]));
    }

    g_strfreev (lines);
out:

    g_object_unref (file);

    return list;
}

static GSList *
parole_pl_parser_parse_pls (const gchar *filename)
{
    XfceRc *rcfile;
    GSList *list = NULL;
    ParoleFile *file;
    const gchar *file_entry, *title_entry;
    guint i, nentries;
    gchar key[128];

    rcfile = xfce_rc_simple_open (filename, TRUE);

    if ( xfce_rc_has_group (rcfile, "playlist") ) 
    {
        xfce_rc_set_group (rcfile, "playlist");
        
        nentries = xfce_rc_read_int_entry (rcfile, "NumberOfEntries", 0);

        for (i = 1; i <= nentries; i++) 
	{
            g_snprintf (key, 128, "File%d", i);
	    
            file_entry = xfce_rc_read_entry (rcfile, key, NULL);
	    
            if (!file_entry)
                continue;
	    
	    g_snprintf (key, 128, "Title%d", i);
	    
            title_entry = xfce_rc_read_entry (rcfile, key, NULL);
	    
	    file = parole_file_new_with_display_name (file_entry, title_entry);
	    list = g_slist_append (list, file);
	}
    }
    
    xfce_rc_close (rcfile);
    
    return list;
}

static GSList *
parole_pl_parser_parse_xspf (const gchar *filename)
{
    ParoleParserData data;
    GFile *file;
    gchar *contents;
    GError *error = NULL;
    gsize size;
    GMarkupParseContext *pctx;
    GMarkupParser parser = {
        parole_xspf_xml_start,
        parole_xspf_xml_end,
        parole_xspf_xml_text,
        NULL,
        NULL
    };
    
    data.list = NULL;
    data.title = data.uri = NULL;
    
    file = g_file_new_for_path (filename);
    
    if ( !g_file_load_contents (file, NULL, &contents, &size, NULL, NULL) )
	goto out;
    
    if ( g_utf8_validate (contents, -1, NULL) == FALSE) 
    {
	gchar *fixed;
	fixed = g_convert (contents, -1, "UTF-8", "ISO8859-1", NULL, NULL, NULL);
	if (fixed != NULL) 
	{
	    g_free (contents);
	    contents = fixed;
	}
    }
    
    pctx = g_markup_parse_context_new (&parser, 0, &data, NULL);
    
    if ( !g_markup_parse_context_parse (pctx, contents, size, &error) )
    {
	if ( error )
	{
	    g_critical ("Unable to parse xspf file : %s : %s\n", filename, error->message);
	    g_error_free (error);
	}
    }
    else
    {
	if ( !g_markup_parse_context_end_parse (pctx, &error) )
	{
            g_critical ("Unable to finish parsing xspf playlist file %s", error->message);
            g_error_free (error);
        }
    }
    
    g_markup_parse_context_free (pctx);
    
out:
    g_object_unref (file);
    return data.list;
}

static GSList *
parole_pl_parser_parse (ParolePlFormat format, const gchar *filename)
{
    GSList *list = NULL;
    
    switch (format)
    {
	case PAROLE_PL_FORMAT_M3U:
	    list = parole_pl_parser_parse_m3u (filename);
	    break;
	case PAROLE_PL_FORMAT_PLS:
	    list = parole_pl_parser_parse_pls (filename);
	    break;
	case PAROLE_PL_FORMAT_ASX:
	    list = parole_pl_parser_parse_asx (filename);
	    break;
	case PAROLE_PL_FORMAT_XSPF:
	    list = parole_pl_parser_parse_xspf (filename);
	    break;
	default:
	    break;
    }

    return list;
}

static gboolean
parole_pl_parser_save_m3u (FILE *f, GSList *files)
{
    guint len;
    guint i;
    
    fputs ("#EXTM3U\n\n", f);
    
    len = g_slist_length (files);
    
    for ( i = 0; i < len; i++)
    {
	ParoleFile *file;
	file = g_slist_nth_data (files, i);
	fprintf (f, "#EXTINF:-1,%s\n", parole_file_get_display_name (file));
	fprintf (f, "%s\n\n", parole_file_get_file_name (file));
    }
    
    return TRUE;
}

static gboolean
parole_pl_parser_save_pls (FILE *f, GSList *files)
{
    guint len;
    guint i;
    gchar key[128];
    
    len = g_slist_length (files);
    
    fprintf (f, "[playlist]\nNumberOfEntries=%d\n", len);
    
    for ( i = 1; i <= len; i++)
    {
	ParoleFile *file;
	file = g_slist_nth_data (files, i - 1);
	g_snprintf (key, 128, "File%d", i);
	fprintf (f, "%s=%s\n", key, parole_file_get_file_name (file));
	g_snprintf (key, 128, "Title%d", i);
	fprintf (f, "%s=%s\n\n", key, parole_file_get_display_name (file));
    }
    
    return TRUE;
}

static gboolean
parole_pl_parser_save_asx (FILE *f, GSList *files)
{
    guint len;
    guint i;
    
    len = g_slist_length (files);

    fputs ("<ASX VERSION=\"3.0\">\n", f);
    
    for ( i = 0; i < len; i++)
    {
	ParoleFile *file;
	file = g_slist_nth_data (files, i);
	fprintf (f, "  <ENTRY>\n   <TITLE>%s</TITLE>\n    <REF HREF=\"%s\"/>\n  </ENTRY>\n", 
		 parole_file_get_display_name (file),
		 parole_file_get_uri (file));
    }
    
    fputs ("</ASX>\n", f);
    
    return TRUE;
}

static gboolean
parole_pl_parser_save_xspf (FILE *f, GSList *files)
{
    guint len;
    guint i;
    
    len = g_slist_length (files);
    
    fputs ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	   "<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n", f);
	   
    fputs (" <trackList>\n", f);
    
    for ( i = 0; i < len; i++)
    {
	ParoleFile *file;
	
	file = g_slist_nth_data (files, i);
	fprintf (f, "  <track>\n    <title>%s</title>\n    <location>%s</location>\n  </track>\n", 
		 parole_file_get_display_name (file), parole_file_get_uri (file));
    }
    
    fputs (" </trackList>\n<playlist>", f);
    
    return TRUE;
}

/**
 * parole_pl_parser_guess_format_from_extension:
 * @filename: a filename.
 * 
 * Guess a playlist format from the filename extension.
 * 
 * Returns: PAROLE_PL_FORMAT_UNKNOWN if unable to get the playlist format, and a valid  
 *          playlist format otherwise.
 * 
 * Since: 0.2
 */
ParolePlFormat
parole_pl_parser_guess_format_from_extension (const gchar *filename)
{
    if ( g_str_has_suffix (filename, ".m3u") || g_str_has_suffix (filename, ".M3U") )
	return PAROLE_PL_FORMAT_M3U;
	
    if ( g_str_has_suffix (filename, ".pls") || g_str_has_suffix (filename, ".PLS") )
	return PAROLE_PL_FORMAT_PLS;
	
    if ( g_str_has_suffix (filename, ".xspf") || g_str_has_suffix (filename, ".XSPF") )
	return PAROLE_PL_FORMAT_XSPF;
	
    if ( g_str_has_suffix (filename, ".asx") || g_str_has_suffix (filename, ".ASX") )
	return PAROLE_PL_FORMAT_ASX;
	
    if ( g_str_has_suffix (filename, ".wax") || g_str_has_suffix (filename, ".WAX") )
	return PAROLE_PL_FORMAT_XSPF;
	
    return PAROLE_PL_FORMAT_UNKNOWN;
}

/**
 * parole_pl_parser_guess_format_from_data:
 * @filename: a filename.
 * 
 * Guess a playlist format from its data.
 * 
 * Returns: PAROLE_PL_FORMAT_UNKNOWN if unable to get the playlist format, and a valid  
 *          playlist format otherwise.
 * 
 * Since: 0.2
 */
ParolePlFormat
parole_pl_parser_guess_format_from_data (const gchar *filename)
{
    GFile *file;
    gchar *contents = NULL;
    gsize size;
    
    ParolePlFormat format = PAROLE_PL_FORMAT_UNKNOWN;
    
    file = g_file_new_for_path (filename);

    if ( !g_file_load_contents (file, NULL, &contents, &size, NULL, NULL ) )
    {
	g_debug ("Unable to load content of file=%s", filename);
	goto out;
    }

    if ( strstr (contents, "<ASX VERSION") )
	format = PAROLE_PL_FORMAT_ASX;
    else if ( strstr (contents, "<trackList>") || strstr (contents, "<tracklist>") )
	format = PAROLE_PL_FORMAT_XSPF;
    else if ( strstr (contents, "NumberOfEntries") )
	format = PAROLE_PL_FORMAT_PLS;
    else 
	/* try to load the file as M3U*/
	format = PAROLE_PL_FORMAT_M3U;

    g_free (contents);
out:
    g_object_unref (file);
    return format;
}

/**
 * parole_pl_parser_save_from_files:
 * @files: a #GSList list of #ParoleFile files.
 * @filename: a filename to save.
 * @format: a #ParolePlFormat format of the playlist.
 * 
 * Saves a #GSList containing a list of #ParoleFile files to filename.
 * 
 * 
 * Returns: TRUE if the playlist was saved, FALSE otherwise.
 * 
 * Since: 0.2
 **/
gboolean parole_pl_parser_save_from_files (GSList *files, const gchar *filename, ParolePlFormat format)
{
    FILE *f;
    gboolean ret_val;

    PAROLE_DEBUG_ENUM_FULL (format, PAROLE_ENUM_TYPE_PL_FORMAT, "Saving playlist %s ", filename);

    f = fopen (filename, "w");
    
    switch (format)
    {
	case PAROLE_PL_FORMAT_M3U:
	    ret_val = parole_pl_parser_save_m3u (f, files);
	    break;
	case PAROLE_PL_FORMAT_PLS:
	    ret_val = parole_pl_parser_save_pls (f, files);
	    break;
	case PAROLE_PL_FORMAT_ASX:
	    ret_val = parole_pl_parser_save_asx (f, files);
	    break;
	case PAROLE_PL_FORMAT_XSPF:
	    ret_val = parole_pl_parser_save_xspf (f, files);
	    break;
	default:
	    break;
    }
    
    fclose (f);
    
    return ret_val;
}

/**
 * parole_pl_parser_parse_from_file_by_extension:
 * @filename: a filename.
 * 
 * 
 * Returns: a #GSList containts a list of #Parolefile parsed from the playlist, 
 *          or NULL if no files were parsed.
 * 
 * Since: 0.2
 */
GSList *parole_pl_parser_parse_from_file_by_extension (const gchar *filename)
{
    ParolePlFormat format = PAROLE_PL_FORMAT_UNKNOWN;
    GSList *list = NULL;
    
    if ( (format = parole_pl_parser_guess_format_from_extension (filename)) == PAROLE_PL_FORMAT_UNKNOWN &&
	 (format = parole_pl_parser_guess_format_from_data (filename)) == PAROLE_PL_FORMAT_UNKNOWN ) 
    {
	g_debug ("Unable to guess playlist format : %s", filename);
	goto out;
    }

    PAROLE_DEBUG_ENUM_FULL (format, PAROLE_ENUM_TYPE_PL_FORMAT, "playlist %s ", filename);
    list = parole_pl_parser_parse (format, filename);
	
out:
    return list;
}

/**
 * parole_pl_parser_parse_all_from_file:
 * @filename: a filename
 * 
 * This function tries to parse a playlist without guessing the playlist format.
 * 
 * 
 * Returns: a #GSList containts a list of #Parolefile parsed from the playlist, 
 *          or NULL if no files were parsed.
 * 
 * Since: 0.2
 */
GSList *parole_pl_parser_parse_all_from_file (const gchar *filename)
{
    GSList *list = NULL;
    
    list = parole_pl_parser_parse_asx (filename);
    list = g_slist_concat (list, parole_pl_parser_parse_m3u (filename));
    list = g_slist_concat (list, parole_pl_parser_parse_pls (filename));
    list = g_slist_concat (list, parole_pl_parser_parse_xspf (filename));
    
    return list;
}

/**
 * parole_pl_parser_can_parse_data:
 * @data: data.
 * @len: length of data.
 * 
 * Get if the Parole parser can parse from the passed data.
 * 
 * Returns: TRUE if it can parse from the data, FALSE otherwise.
 * 
 * Since: 0.2
 */
gboolean parole_pl_parser_can_parse_data (const guchar *data, gint len)
{
    gchar *mime_type = NULL;
    gboolean result_uncertain;
    gboolean result = FALSE;

    mime_type = g_content_type_guess (NULL, data, len,  &result_uncertain);
    
    if ( mime_type && result_uncertain == FALSE )
    {
	GtkFileFilter *filter = g_object_ref_sink (parole_get_supported_playlist_filter ());
	GtkFileFilterInfo filter_info;
	g_debug ("Mime_type=%s", mime_type);
	filter_info.mime_type = mime_type;
    
	filter_info.contains = GTK_FILE_FILTER_MIME_TYPE;
    
	result = gtk_file_filter_filter (filter, &filter_info);
	g_object_unref (filter);
	g_free (mime_type);
    }
    
    return result;
}
