/* wl-clipboard
 *
 * Copyright © 2019 Sergey Bugaev <bugaevc@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types/shell-surface.h"

#include "includes/shell-protocols.h"

void shell_surface_destroy(struct shell_surface *self) {
    self->do_destroy(self);
}


/* Core Wayland implementation */

static void wl_shell_surface_ping
(
    void *data,
    struct wl_shell_surface *shell_surface,
    uint32_t serial
) {
    wl_shell_surface_pong(shell_surface, serial);
}

static void wl_shell_surface_configure
(
    void *data,
    struct wl_shell_surface *shell_surface,
    uint32_t edges,
    int32_t width,
    int32_t height
) {}

static void wl_shell_surface_popup_done
(
    void *data,
    struct wl_shell_surface *shell_surface
) {}

static const struct wl_shell_surface_listener wl_shell_surface_listener = {
    .ping = wl_shell_surface_ping,
    .configure = wl_shell_surface_configure,
    .popup_done = wl_shell_surface_popup_done
};

static void destroy_wl_shell_surface(struct shell_surface *self) {
    struct wl_shell_surface *proxy = (struct wl_shell_surface *) self->proxy;
    wl_shell_surface_destroy(proxy);
}

void init_wl_shell_surface(struct shell_surface *self) {
    struct wl_shell_surface *proxy = (struct wl_shell_surface *) self->proxy;
    wl_shell_surface_add_listener(proxy, &wl_shell_surface_listener, self);
    wl_shell_surface_set_toplevel(proxy);
    wl_shell_surface_set_title(proxy, "wl-clipboard");
    self->do_destroy = destroy_wl_shell_surface;
}


/* xdg-shell implementation */

#ifdef HAVE_XDG_SHELL

static void xdg_toplevel_configure_handler
(
    void *data,
    struct xdg_toplevel *xdg_toplevel,
    int32_t width,
    int32_t height,
    struct wl_array *states
) {}

static void xdg_toplevel_close_handler
(
    void *data,
    struct xdg_toplevel *xdg_toplevel
) {}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler
};

static void xdg_surface_configure_handler
(
    void *data,
    struct xdg_surface *xdg_surface,
    uint32_t serial
) {
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler
};

static void destroy_xdg_surface(struct shell_surface *self) {
    struct xdg_toplevel *toplevel = (struct xdg_toplevel *) self->proxy2;
    struct xdg_surface *proxy = (struct xdg_surface *) self->proxy;
    xdg_toplevel_destroy(toplevel);
    xdg_surface_destroy(proxy);
}

void init_xdg_surface(struct shell_surface *self) {
    struct xdg_surface *proxy = (struct xdg_surface *) self->proxy;
    xdg_surface_add_listener(proxy, &xdg_surface_listener, self);
    struct xdg_toplevel *toplevel = xdg_surface_get_toplevel(proxy);
    self->proxy2 = (struct wl_proxy *) toplevel;
    xdg_toplevel_add_listener(toplevel, &xdg_toplevel_listener, self);
    xdg_toplevel_set_title(toplevel, "wl-clipboard");
    self->do_destroy = destroy_xdg_surface;
}

#endif


/* wlr-layer-shell implementation */

#ifdef HAVE_WLR_LAYER_SHELL

static void layer_surface_configure_handler
(
    void *data,
    struct zwlr_layer_surface_v1 *layer_surface,
    uint32_t serial,
    uint32_t width,
    uint32_t height
) {
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
}

static void layer_surface_closed_handler
(
    void *data,
    struct zwlr_layer_surface_v1 *layer_surface
) {}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure_handler,
    .closed = layer_surface_closed_handler
};

static void destroy_zwlr_layer_surface_v1(struct shell_surface *self) {
    struct zwlr_layer_surface_v1 *proxy =
        (struct zwlr_layer_surface_v1 *) self->proxy;
    zwlr_layer_surface_v1_destroy(proxy);
}

void init_zwlr_layer_surface_v1(struct shell_surface *self) {
    struct zwlr_layer_surface_v1 *proxy =
        (struct zwlr_layer_surface_v1 *) self->proxy;
    zwlr_layer_surface_v1_add_listener(proxy, &layer_surface_listener, self);
    zwlr_layer_surface_v1_set_keyboard_interactivity(proxy, 1);
    self->do_destroy = destroy_zwlr_layer_surface_v1;
}

#endif

