/*
 * Copyright © 2010-2011 Linaro Limited
 * Copyright © 2013 Canonical Ltd
 *
 * This file is part of the glmark2 OpenGL (ES) 2.0 benchmark.
 *
 * glmark2 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * glmark2 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * glmark2.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Alexandros Frantzis
 *  Siarhei Siamashka
 */

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <EGL/egl.h>

#include "native-state-fb.h"

static bool is_android = false;
static bool is_mali400 = false;

static int xres, yres;

static mali400_native_window_t mali400_native_window = { 0, 0 };

NativeStateFB::~NativeStateFB()
{
}

bool
NativeStateFB::init_display()
{
    int fd;
    struct stat st;
    struct fb_var_screeninfo vinfo;

    if ((fd = open("/dev/fb0", O_RDWR)) == -1)
        return false;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
        close(fd);
        return false;
    }

    xres = vinfo.xres;
    yres = vinfo.yres;
    close(fd);

    if (stat("/dev/mali", &st) != -1)
        is_mali400 = true;

    return true;
}

void*
NativeStateFB::display()
{
    return (void *)EGL_DEFAULT_DISPLAY;
}

bool
NativeStateFB::create_window(WindowProperties const& properties)
{
    EGLDisplay egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    properties_ = properties;

    if (egl_display != EGL_NO_DISPLAY)
        if (strcmp(eglQueryString(egl_display, EGL_VENDOR), "Android") == 0)
            is_android = true;

    if (is_android || properties_.fullscreen) {
        properties_.width = xres;
        properties_.height = yres;
    }

    if (properties_.width > xres || properties_.height > yres)
        return false;

    mali400_native_window.width = properties_.width;
    mali400_native_window.height = properties_.height;
    return true;
}

void*
NativeStateFB::window(WindowProperties& properties)
{
    properties = properties_;
    if (is_android || !is_mali400)
        return NULL;
    return (void *)&mali400_native_window;
}

void
NativeStateFB::visible(bool /*visible*/)
{
}

bool
NativeStateFB::should_quit()
{
    return false;
}
