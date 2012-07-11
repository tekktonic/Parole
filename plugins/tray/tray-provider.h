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

#ifndef TRAY_PROVIDER_H_
#define TRAY_PROVIDER_H_

#include <parole/parole.h>

G_BEGIN_DECLS

typedef struct _TrayProviderClass TrayProviderClass;
typedef struct _TrayProvider      TrayProvider;

#define TRAY_TYPE_PROVIDER             (tray_provider_get_type ())
#define TRAY_PROVIDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TRAY_TYPE_PROVIDER, TrayProvider))
#define TRAY_PROVIDER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TRAY_TYPE_PROVIDER, TrayProviderClass))
#define TRAY_IS_PROVIDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TRAY_TYPE_PROVIDER))
#define TRAY_IS_PROVIDER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TRAY_TYPE_PROVIDER))
#define TRAY_PROVIDER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TRAY_TYPE_PROVIDER, TrayProviderClass))

GType tray_provider_get_type      	(void) G_GNUC_CONST G_GNUC_INTERNAL;

void  tray_provider_register_type	(ParoleProviderPlugin *provider);

G_END_DECLS

#endif /*TRAY_PROVIDER_H_*/
