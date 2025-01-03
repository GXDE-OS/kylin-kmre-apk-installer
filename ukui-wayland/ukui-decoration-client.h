/* Generated by wayland-scanner 1.18.0 */

#ifndef CUSTOM_CLIENT_PROTOCOL_H
#define CUSTOM_CLIENT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @page page_custom The custom protocol
 * @section page_ifaces_custom Interfaces
 * - @subpage page_iface_ukui_decoration - UKUI Wayland extension
 * @section page_copyright_custom Copyright
 * <pre>
 *
 * Copyright (C) 2015 The Qt Company Ltd.
 * Contact: http://www.qt.io/licensing/
 *
 * This file is part of the examples of the Qt Wayland module
 *
 * $QT_BEGIN_LICENSE:BSD$
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * * Neither the name of The Qt Company Ltd nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 *
 * $QT_END_LICENSE$
 * </pre>
 */
struct ukui_decoration;
struct wl_surface;

/**
 * @page page_iface_ukui_decoration ukui_decoration
 * @section page_iface_ukui_decoration_desc Description
 *
 * This example shows how to add extra functionality to Wayland
 * through an extension. This is the global object of the extension.
 * @section page_iface_ukui_decoration_api API
 * See @ref iface_ukui_decoration.
 */
/**
 * @defgroup iface_ukui_decoration The ukui_decoration interface
 *
 * This example shows how to add extra functionality to Wayland
 * through an extension. This is the global object of the extension.
 */
extern const struct wl_interface ukui_decoration_interface;

#define UKUI_DECORATION_MOVE_SURFACE 0
#define UKUI_DECORATION_SET_UKUI_DECORATION_MODE 1
#define UKUI_DECORATION_SET_UNITY_BORDER_RADIUS 2


/**
 * @ingroup iface_ukui_decoration
 */
#define UKUI_DECORATION_MOVE_SURFACE_SINCE_VERSION 1
/**
 * @ingroup iface_ukui_decoration
 */
#define UKUI_DECORATION_SET_UKUI_DECORATION_MODE_SINCE_VERSION 1
/**
 * @ingroup iface_ukui_decoration
 */
#define UKUI_DECORATION_SET_UNITY_BORDER_RADIUS_SINCE_VERSION 1

/** @ingroup iface_ukui_decoration */
static inline void
ukui_decoration_set_user_data(struct ukui_decoration *ukui_decoration, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) ukui_decoration, user_data);
}

/** @ingroup iface_ukui_decoration */
static inline void *
ukui_decoration_get_user_data(struct ukui_decoration *ukui_decoration)
{
	return wl_proxy_get_user_data((struct wl_proxy *) ukui_decoration);
}

static inline uint32_t
ukui_decoration_get_version(struct ukui_decoration *ukui_decoration)
{
	return wl_proxy_get_version((struct wl_proxy *) ukui_decoration);
}

/** @ingroup iface_ukui_decoration */
static inline void
ukui_decoration_destroy(struct ukui_decoration *ukui_decoration)
{
	wl_proxy_destroy((struct wl_proxy *) ukui_decoration);
}

/**
 * @ingroup iface_ukui_decoration
 *
 * Inform the compositor that the client has a new surface that is
 * covered by the extension.
 */
static inline void
ukui_decoration_move_surface(struct ukui_decoration *ukui_decoration, struct wl_surface *surface)
{
	wl_proxy_marshal((struct wl_proxy *) ukui_decoration,
			 UKUI_DECORATION_MOVE_SURFACE, surface);
}

/**
 * @ingroup iface_ukui_decoration
 *
 * The compositor should perform a move animation on the surface.
 */
static inline void
ukui_decoration_set_ukui_decoration_mode(struct ukui_decoration *ukui_decoration, struct wl_surface *surface, uint32_t mode)
{
	wl_proxy_marshal((struct wl_proxy *) ukui_decoration,
			 UKUI_DECORATION_SET_UKUI_DECORATION_MODE, surface, mode);
}

/**
 * @ingroup iface_ukui_decoration
 *
 * The compositor should perform a move animation on the surface.
 */
static inline void
ukui_decoration_set_unity_border_radius(struct ukui_decoration *ukui_decoration, struct wl_surface *surface, uint32_t topleft, uint32_t topright, uint32_t bottomleft, uint32_t bottomright)
{
	wl_proxy_marshal((struct wl_proxy *) ukui_decoration,
			 UKUI_DECORATION_SET_UNITY_BORDER_RADIUS, surface, topleft, topright, bottomleft, bottomright);
}

#ifdef  __cplusplus
}
#endif

#endif
