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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#if defined(__linux__)
#include <linux/cdrom.h>
#endif

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include "parole-disc.h"
#include "parole-builder.h"
#include "gmarshal.h"

static void parole_disc_finalize   (GObject *object);

#define PAROLE_DISC_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), PAROLE_TYPE_DISC, ParoleDiscPrivate))

struct ParoleDiscPrivate
{
    GVolumeMonitor *monitor;
    GPtrArray      *array;
    
    GtkWidget      *dvd_menu;
    GtkWidget	   *cd_menu;
    
    gboolean	    needs_update;
};

enum
{
    DISC_SELECTED,
    LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (ParoleDisc, parole_disc, G_TYPE_OBJECT)

typedef struct
{
    GtkWidget      *mi;
    gchar          *uri;
    gchar          *device;
    ParoleDiscKind  kind;
    
} MountData;

static void
free_mount_data (gpointer data)
{
    MountData *mount;
    
    mount = (MountData *) data;
    
    if ( mount->uri )
	g_free (mount->uri);
	
    if ( mount->device )
	g_free (mount->device);
	
    g_free (mount);
}

static void
parole_disc_media_activate_cb (GtkWidget *widget, ParoleDisc *disc)
{
    MountData *data;
    
    data = g_object_get_data (G_OBJECT (widget), "mount-data");
    
    g_signal_emit (G_OBJECT (disc), signals [DISC_SELECTED], 0, data->uri, data->device);
}


static void
parole_disc_insert_menu_item (ParoleDisc *disc, MountData *data, const gchar *label)
{
    GtkWidget *menu;
    GtkWidget *img;
    
    data->mi = gtk_image_menu_item_new_with_label (label);
	
    img = gtk_image_new_from_stock (GTK_STOCK_CDROM, GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (data->mi), 
				   img);
    gtk_widget_show (data->mi);
    gtk_widget_show (img);
	
    g_object_set_data (G_OBJECT (data->mi),
		      "mount-data", data);
	
    switch (data->kind )
    {
	case PAROLE_DISC_CDDA:
	case PAROLE_DISC_SVCD:
	case PAROLE_DISC_VCD:
	    menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (disc->priv->cd_menu));
	    break;
	case PAROLE_DISC_DVD:
	    menu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (disc->priv->dvd_menu));
	    break;
	default:
	    g_warn_if_reached ();
	    break;
    }
    
    gtk_menu_shell_insert (GTK_MENU_SHELL (menu), data->mi, 2);
    
    g_signal_connect (data->mi, "activate",
		      G_CALLBACK (parole_disc_media_activate_cb), disc);
    
}

static MountData *
parole_disc_get_mount_data (ParoleDisc *disc, 
			    const gchar *uri, 
			    const gchar *device,
			    ParoleDiscKind kind)
{
    MountData *data;
    
    data = g_new0 (MountData, 1);
    
    data->kind = kind;
    data->uri = data->device = NULL;
    data->uri = g_strdup (uri);
    data->device = g_strdup (device);
    
    return data;
}

static void
parole_disc_add_mount_to_menu (ParoleDisc *disc, GMount *mount, const gchar *device)
{
    GFile *file;
    gchar **content_type;
    guint i;
    ParoleDiscKind kind;
    gchar *uri = NULL;
    
    file = g_mount_get_root (mount);
    
    if ( g_file_has_uri_scheme (file, "cdda") )
    {
	kind = PAROLE_DISC_CDDA;
	uri = g_strdup ("cdda://");
	goto got_cdda;
    }
	
    content_type = g_content_type_guess_for_tree (file);

    for ( i = 0; content_type && content_type[i]; i++)
    {
	TRACE ("Checking disc content type : %s", content_type[i]);
	    
	if ( !g_strcmp0 (content_type[i], "x-content/video-dvd") )
	{
	    kind = PAROLE_DISC_DVD;
	    uri = g_strdup ("dvd:/");
	    break;
	}
	else if ( !g_strcmp0 (content_type[i], "x-content/video-vcd") )
	{
	    kind = PAROLE_DISC_VCD;
	    uri = g_strdup ("vcd:/");
	    break;
	}
	else if ( !g_strcmp0 (content_type[i], "x-content/video-svcd") )
	{
	    kind = PAROLE_DISC_SVCD;
	    uri = g_strdup ("svcd:/");
	    break;
	}
	else if ( !g_strcmp0 (content_type[i], "x-content/audio-cdda") )
	{
	    kind = PAROLE_DISC_CDDA;
	    uri = g_strdup ("cdda://");
	    break;
	}
    }
    
    if ( content_type )
	g_strfreev (content_type);

got_cdda:
    if ( kind != PAROLE_DISC_UNKNOWN )
    {
	MountData *data;
	gchar *name;
	gchar *label;
	
	name = g_mount_get_name (mount);
	label = g_strdup_printf ("%s '%s'", _("Play Disc"), name);
	
	data = parole_disc_get_mount_data (disc, uri, device, kind);
	parole_disc_insert_menu_item (disc, data, label);
	
	if ( uri )
	    g_free (uri);
	
	g_ptr_array_add (disc->priv->array, data);
	g_free (label);
	g_free (name);
    }
    
    g_object_unref (file);
}

static void
parole_disc_check_cdrom (ParoleDisc *disc, GVolume *volume, const gchar *device)
{
#if defined(__linux__)
    gint fd;
    gint drive;

    TRACE ("device : %s", device);
    
    if ( (fd = open (device, O_RDONLY)) < 0 )
    {
	g_debug ("Failed to open device : %s", device);
	disc->priv->needs_update = TRUE;
	goto out;
    }
    
    if ( (drive = ioctl (fd, CDROM_DRIVE_STATUS, NULL)) )
    {
	if ( drive == CDS_DRIVE_NOT_READY )
	{
	    g_print ("Drive :%s is not yet ready\n", device);
	    disc->priv->needs_update = TRUE;
	}
	else if ( drive == CDS_DISC_OK )
	{
	    if ( (drive = ioctl (fd, CDROM_DISC_STATUS, NULL)) > 0 )
	    {
		if ( drive == CDS_AUDIO || drive == CDS_MIXED )
		{
		    MountData *data;
		    data = parole_disc_get_mount_data (disc, "cdda://", device, PAROLE_DISC_CDDA);
		    parole_disc_insert_menu_item (disc, data, g_volume_get_name (volume));
		    g_ptr_array_add (disc->priv->array, data);
		}
	    }
	}
    }
    
    close (fd);
out:
    ;
#endif /* if defined(__linux__) */
}

static void
parole_disc_add_drive (ParoleDisc *disc, GDrive *drive, const gchar *device)
{
    GList *list;
    guint len;
    guint i;
    
    list = g_drive_get_volumes (drive);
    len = g_list_length (list);
    
    for ( i = 0; i < len; i++)
    {
	GVolume *volume;
	GMount *mount;
	
	volume = g_list_nth_data (list, i);
	TRACE ("Volume name %s", g_volume_get_name (volume));
	
	mount = g_volume_get_mount (volume);
	if ( mount )
	{
	    TRACE ("Mount name : %s", g_mount_get_name (mount));
	    parole_disc_add_mount_to_menu (disc, mount, device);
	    g_object_unref (mount);
	}
	else
	{
	    /* Could be a cdda?*/
	    parole_disc_check_cdrom (disc, volume, device);
	}
    }
    
    g_list_foreach (list, (GFunc) g_object_unref, NULL);
    g_list_free (list);
}

static void
parole_disc_get_drives (ParoleDisc *disc)
{
    GList *list;
    guint len;
    guint i;
    
    list = g_volume_monitor_get_connected_drives (disc->priv->monitor);
    
    len = g_list_length (list);

    /*
     * Set the update flag here because it can be set later to TRUE
     * in case a device is not yet ready.
     */
    disc->priv->needs_update = FALSE;
    
    for ( i = 0; i < len; i++)
    {
	GDrive *drive;
	gchar *device = NULL;
	
	drive = g_list_nth_data (list, i);
	
	if ( g_drive_can_eject (drive) && g_drive_has_media (drive) )
	{
	    device = g_drive_get_identifier (drive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
	    parole_disc_add_drive (disc, drive, device);
	    if ( device )
		g_free (device);
	}
    }
    
    g_list_foreach (list, (GFunc) g_object_unref, NULL);
    g_list_free (list);
}

static void
parole_disc_select_cb (GtkItem *item, ParoleDisc *disc)
{
    if ( disc->priv->needs_update )
	parole_disc_get_drives (disc);
}

static void
parole_disc_monitor_changed_cb (GVolumeMonitor *monitor, gpointer *device, ParoleDisc *disc)
{
    guint i;
    
    TRACE ("Changed");
    for ( i = 0 ; i < disc->priv->array->len; i++)
    {
	MountData *data;
	
	data = g_ptr_array_index (disc->priv->array, i);
	gtk_widget_destroy (data->mi);
	g_ptr_array_remove_index (disc->priv->array, i);
	g_free (data);
    }
    
    disc->priv->needs_update = TRUE;
}

static void
parole_disc_class_init (ParoleDiscClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    
    signals[DISC_SELECTED] = 
        g_signal_new ("disc-selected",
                      PAROLE_TYPE_DISC,
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (ParoleDiscClass, disc_selected),
                      NULL, NULL,
		      _gmarshal_VOID__STRING_STRING,
                      G_TYPE_NONE, 2, 
		      G_TYPE_STRING, G_TYPE_STRING);
		      
    object_class->finalize = parole_disc_finalize;

    g_type_class_add_private (klass, sizeof (ParoleDiscPrivate));
}

static void
parole_disc_init (ParoleDisc *disc)
{
    GtkBuilder *builder;
    
    disc->priv = PAROLE_DISC_GET_PRIVATE (disc);
    
    builder = parole_builder_get_main_interface ();
    
    disc->priv->array = g_ptr_array_new ();
    disc->priv->needs_update = TRUE;
    
    disc->priv->monitor = g_volume_monitor_get ();
    
    g_signal_connect (G_OBJECT (disc->priv->monitor), "volume-added",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
    
    g_signal_connect (G_OBJECT (disc->priv->monitor), "volume-removed",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
    
    g_signal_connect (G_OBJECT (disc->priv->monitor), "mount-added",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
		      
    g_signal_connect (G_OBJECT (disc->priv->monitor), "mount-removed",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
    
    g_signal_connect (G_OBJECT (disc->priv->monitor), "drive-disconnected",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
    
    g_signal_connect (G_OBJECT (disc->priv->monitor), "drive-eject-button",
		      G_CALLBACK (parole_disc_monitor_changed_cb), disc);
    
    disc->priv->dvd_menu = GTK_WIDGET (gtk_builder_get_object (builder, "dvd-menu"));
    disc->priv->cd_menu = GTK_WIDGET (gtk_builder_get_object (builder, "cd-menu"));
    
    g_signal_connect (gtk_builder_get_object (builder, "media-menu-item"), "select",
	              G_CALLBACK (parole_disc_select_cb), disc);
		      
    g_object_unref (builder);
}

static void
parole_disc_finalize (GObject *object)
{
    ParoleDisc *disc;

    disc = PAROLE_DISC (object);
    
    g_object_unref (disc->priv->monitor);
    
    g_ptr_array_foreach (disc->priv->array, (GFunc) free_mount_data, NULL);
    g_ptr_array_free (disc->priv->array, TRUE);

    G_OBJECT_CLASS (parole_disc_parent_class)->finalize (object);
}

ParoleDisc *
parole_disc_new (void)
{
    ParoleDisc *disc = NULL;
    disc = g_object_new (PAROLE_TYPE_DISC, NULL);
    return disc;
}
