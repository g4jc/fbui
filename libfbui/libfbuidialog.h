
/*=========================================================================
 *
 * libfbui, a library for accessing FBUI (in-kernel framebuffer GUI).
 * Copyright (C) 2003-2005 Zachary Smith, fbui@comcast.net
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this module; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * (See the file COPYING in the main directory of this archive for
 * more details.)
 *
 *=======================================================================*/


#ifndef LIBFBUI_DIALOG
#define LIBFBUI_DIALOG

#include "libfbuifont.h"

extern void fbui_draw_dialog (Display *dpy, Window *win, Font *font, char *expr);

#endif

