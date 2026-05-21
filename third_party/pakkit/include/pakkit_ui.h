/*
 * PakKit UI — Reusable UI components for NextUI paks
 * Header-only library. Built on top of Apostrophe.
 *
 * Usage:
 *   #define PAKKIT_UI_IMPLEMENTATION
 *   #include "pakkit_ui.h"
 *
 * Requires Apostrophe to be initialized before use.
 */

#ifndef PAKKIT_UI_H
#define PAKKIT_UI_H

#include "apostrophe.h"

/* -----------------------------------------------------------------------
 * Configuration
 * ----------------------------------------------------------------------- */

#ifndef PAKKIT_MAX_MENU_ITEMS
#define PAKKIT_MAX_MENU_ITEMS 12
#endif

#ifndef PAKKIT_SCROLL_STEP
#define PAKKIT_SCROLL_STEP 85
#endif

/* -----------------------------------------------------------------------
 * Hint bar (minimal footer replacement)
 * ----------------------------------------------------------------------- */

typedef struct {
    const char *button;
    const char *label;
} pakkit_hint;

void pakkit_draw_hints(pakkit_hint *hints, int count);

/* -----------------------------------------------------------------------
 * Loading screen
 * ----------------------------------------------------------------------- */

void pakkit_loading(const char *message);

/* -----------------------------------------------------------------------
 * Progress bar
 * ----------------------------------------------------------------------- */

void pakkit_progress(const char *message, int current, int total);

/* -----------------------------------------------------------------------
 * Toggle (drawing primitive)
 * ----------------------------------------------------------------------- */

int pakkit_draw_toggle(int x, int y, int on);

/* -----------------------------------------------------------------------
 * Tabs (drawing primitive)
 * ----------------------------------------------------------------------- */

typedef struct {
    const char *label;
} pakkit_tab;

int pakkit_draw_tabs(pakkit_tab *tabs, int count, int active);

/* -----------------------------------------------------------------------
 * Message dialog
 * ----------------------------------------------------------------------- */

void pakkit_message(const char *message, const char *button_label);

/* -----------------------------------------------------------------------
 * Confirm dialog
 * ----------------------------------------------------------------------- */

int pakkit_confirm(const char *message, const char *confirm_label,
                   const char *cancel_label);

/* -----------------------------------------------------------------------
 * Menu component
 * ----------------------------------------------------------------------- */

typedef struct {
    const char *label;
} pakkit_menu_item;

typedef struct {
    int selected_index;   /* -1 if cancelled */
} pakkit_menu_result;

int pakkit_menu(const char *title, pakkit_menu_item *items, int count,
                pakkit_menu_result *result);

/* -----------------------------------------------------------------------
 * List component
 * ----------------------------------------------------------------------- */

#ifndef PAKKIT_MAX_LIST_ITEMS
#define PAKKIT_MAX_LIST_ITEMS 64
#endif

#ifndef PAKKIT_LIST_VISIBLE
#define PAKKIT_LIST_VISIBLE 8
#endif

typedef enum {
    PAKKIT_ACTION_SELECTED,
    PAKKIT_ACTION_BACK,
    PAKKIT_ACTION_SECONDARY,
    PAKKIT_ACTION_TERTIARY,
} pakkit_action;

typedef struct {
    const char *label;
} pakkit_list_item;

typedef struct {
    const char      *title;
    pakkit_hint     *hints;
    int              hint_count;
    ap_button        secondary_button;   /* AP_BTN_NONE to disable */
    ap_button        tertiary_button;    /* AP_BTN_NONE to disable */
    int              initial_index;      /* starting cursor position */
    bool             text_width_pills;   /* size pills to text width instead of full width */
} pakkit_list_opts;

typedef struct {
    int            selected_index;   /* -1 if cancelled */
    pakkit_action  action;
} pakkit_list_result;

int pakkit_list(pakkit_list_opts *opts, pakkit_list_item *items, int count,
                pakkit_list_result *result);

/* -----------------------------------------------------------------------
 * Detail/About screen component
 * ----------------------------------------------------------------------- */

typedef struct {
    const char *key;
    const char *value;
} pakkit_info_pair;

typedef struct {
    const char       *title;
    const char       *subtitle;
    pakkit_info_pair *info;
    int               info_count;
    const char      **credits;
    int               credit_count;
} pakkit_detail_opts;

void pakkit_detail_screen(pakkit_detail_opts *opts);

/* -----------------------------------------------------------------------
 * Scroll helper
 * ----------------------------------------------------------------------- */

#ifndef PAKKIT_SCROLL_SMOOTH_FACTOR
#define PAKKIT_SCROLL_SMOOTH_FACTOR 0.15f
#endif

typedef struct {
    int scroll_y;
    int target_scroll_y;
    int last_max_scroll;
} pakkit_scroll_state;

void pakkit_scroll_handle_input(pakkit_scroll_state *s, int direction, int step);
void pakkit_scroll_animate(pakkit_scroll_state *s);
void pakkit_scroll_update(pakkit_scroll_state *s, int content_height, int viewport_height);

/* -----------------------------------------------------------------------
 * Keyboard component
 * ----------------------------------------------------------------------- */

#ifndef PAKKIT_KB_MAX_TEXT
#define PAKKIT_KB_MAX_TEXT 512
#endif

#ifndef PAKKIT_KB_MAX_SHORTCUTS
#define PAKKIT_KB_MAX_SHORTCUTS 8
#endif

typedef struct {
    char text[PAKKIT_KB_MAX_TEXT];
} pakkit_keyboard_result;

typedef struct {
    const char  *prompt;          /* hint text shown above input field */
    const char **shortcuts;       /* optional shortcut strings, NULL if none */
    int          shortcut_count;  /* 0 if no shortcuts */
} pakkit_keyboard_opts;

int pakkit_keyboard(const char *initial_text, pakkit_keyboard_opts *opts,
                    pakkit_keyboard_result *result);



/* -----------------------------------------------------------------------
 * Implementation
 * ----------------------------------------------------------------------- */

#ifdef PAKKIT_UI_IMPLEMENTATION

/* --- Hint bar --- */

void pakkit_draw_hints(pakkit_hint *hints, int count) {
    if (!hints || count <= 0) return;

    int sw = ap_get_screen_width();
    int sh = ap_get_screen_height();
    int pad = AP_DS(5);
    TTF_Font *font = ap_get_font(AP_FONT_TINY);
    ap_color color = ap_get_theme()->hint;
    int y = sh - TTF_FontHeight(font) - pad;

    int x = pad * 2;
    for (int i = 0; i < count - 1; i++) {
        char buf[64];
        if (hints[i].label[0] == '\0')
            snprintf(buf, sizeof(buf), "%s", hints[i].button);
        else
            snprintf(buf, sizeof(buf), "%s: %s", hints[i].button, hints[i].label);
        int w = ap_draw_text(font, buf, x, y, color);
        x += w + pad * 3;
    }

    if (count > 0) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s: %s",
                 hints[count - 1].button, hints[count - 1].label);
        int w = ap_measure_text(font, buf);
        ap_draw_text(font, buf, sw - w - pad * 2, y, color);
    }
}

/* --- Loading screen --- */

void pakkit_loading(const char *message) {
    if (!message) message = "Loading...";

    int sw = ap_get_screen_width();
    int sh = ap_get_screen_height();
    int pad = AP_DS(5);

    TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
    ap_color text_color = ap_get_theme()->text;

    int max_w = sw - pad * 8;
    int msg_h = ap_measure_wrapped_text_height(font_small, message, max_w);

    ap_clear_screen();
    ap_draw_background();
    ap_draw_text_wrapped(font_small, message, pad * 4, (sh - msg_h) / 2,
                         max_w, text_color, AP_ALIGN_CENTER);
    ap_present();
}

/* --- Progress bar --- */

void pakkit_progress(const char *message, int current, int total) {
    int sw = ap_get_screen_width();
    int sh = ap_get_screen_height();
    int pad = AP_DS(5);

    TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
    TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

    ap_theme *theme = ap_get_theme();
    ap_color text_color = theme->text;
    ap_color hint_color = theme->hint;
    ap_color highlight  = theme->highlight;

    ap_clear_screen();
    ap_draw_background();

    int center_y = sh / 2;

    /* Message above bar */
    if (message && message[0]) {
        int max_w = sw - pad * 8;
        int msg_h = ap_measure_wrapped_text_height(font_small, message, max_w);
        ap_draw_text_wrapped(font_small, message, pad * 4, center_y - msg_h - pad * 3,
                             max_w, text_color, AP_ALIGN_CENTER);
    }

    /* Progress bar */
    int bar_w = sw - pad * 12;
    int bar_h = AP_DS(8);
    int bar_x = (sw - bar_w) / 2;
    int bar_y = center_y;

    /* Background track */
    ap_color track_color = { hint_color.r, hint_color.g, hint_color.b, 60 };
    ap_draw_rect(bar_x, bar_y, bar_w, bar_h, track_color);

    /* Fill */
    if (total > 0 && current > 0) {
        int fill_w = (current * bar_w) / total;
        if (fill_w > bar_w) fill_w = bar_w;
        ap_draw_rect(bar_x, bar_y, fill_w, bar_h, highlight);
    }

    /* Counter text below bar */
    if (total > 0) {
        char counter[32];
        snprintf(counter, sizeof(counter), "%d / %d", current, total);
        int counter_w = ap_measure_text(font_tiny, counter);
        ap_draw_text(font_tiny, counter, (sw - counter_w) / 2,
                     bar_y + bar_h + pad * 2, hint_color);
    }

    ap_present();
}

/* --- Toggle (drawing primitive) --- */

int pakkit_draw_toggle(int x, int y, int on) {
    int pad = AP_DS(5);
    ap_theme *theme = ap_get_theme();
    ap_color highlight = theme->highlight;
    ap_color hint_color = theme->hint;

    /* Toggle dimensions */
    TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
    int h = TTF_FontHeight(font_small);
    int w = h * 2;
    int knob_size = h - pad;
    int knob_pad = pad / 2;

    /* Track */
    ap_color track_color;
    if (on) {
        track_color = highlight;
    } else {
        track_color = (ap_color){ hint_color.r, hint_color.g, hint_color.b, 80 };
    }
    ap_draw_pill(x, y, w, h, track_color);

    /* Knob */
    ap_color knob_color = {255, 255, 255, 255};
    int knob_x;
    if (on) {
        knob_x = x + w - knob_size - knob_pad;
    } else {
        knob_x = x + knob_pad;
    }
    int knob_y = y + knob_pad;
    ap_draw_circle(knob_x + knob_size / 2, knob_y + knob_size / 2,
                   knob_size / 2, knob_color);

    return w;
}

/* --- Tabs (drawing primitive) --- */

int pakkit_draw_tabs(pakkit_tab *tabs, int count, int active) {
    if (!tabs || count <= 0) return 0;

    int sw = ap_get_screen_width();
    int pad = AP_DS(5);

    TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);

    ap_theme *theme = ap_get_theme();
    ap_color hint_color = theme->hint;
    ap_color highlight  = theme->highlight;
    ap_color hl_text    = theme->highlighted_text;

    int tab_h = TTF_FontHeight(font_small) + pad * 3;
    int tab_y = pad;

    /* Calculate tab widths — equal distribution */
    int total_gap = pad * (count - 1);
    int avail_w = sw - pad * 4;
    int tab_w = (avail_w - total_gap) / count;

    int tx = pad * 2;
    for (int i = 0; i < count; i++) {
        if (i == active) {
            ap_draw_pill(tx, tab_y, tab_w, tab_h, highlight);
            int label_w = ap_measure_text(font_small, tabs[i].label);
            ap_draw_text(font_small, tabs[i].label,
                         tx + (tab_w - label_w) / 2,
                         tab_y + (tab_h - TTF_FontHeight(font_small)) / 2,
                         hl_text);
        } else {
            ap_color tab_bg = { hint_color.r, hint_color.g, hint_color.b, 40 };
            ap_draw_pill(tx, tab_y, tab_w, tab_h, tab_bg);
            int label_w = ap_measure_text(font_small, tabs[i].label);
            ap_draw_text(font_small, tabs[i].label,
                         tx + (tab_w - label_w) / 2,
                         tab_y + (tab_h - TTF_FontHeight(font_small)) / 2,
                         hint_color);
        }
        tx += tab_w + pad;
    }

    /* Divider below tabs */
    int divider_y = tab_y + tab_h + pad;
    ap_draw_rect(pad * 3, divider_y, sw - pad * 6, 1, hint_color);

    /* Return content start y */
    return divider_y + pad * 2;
}

/* --- Menu --- */

int pakkit_menu(const char *title, pakkit_menu_item *items, int count,
                pakkit_menu_result *result) {
    int cursor = 0;
    int running = 1;

    /* Use selected_index as initial cursor hint if valid */
    if (result->selected_index >= 0 && result->selected_index < count) {
        cursor = result->selected_index;
    }
    result->selected_index = -1;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed) {
                switch (ev.button) {
                    case AP_BTN_B:
                        if (!ev.repeated) {
                            result->selected_index = -1;
                            running = 0;
                        }
                        break;
                    case AP_BTN_A:
                        if (!ev.repeated) {
                            result->selected_index = cursor;
                            running = 0;
                        }
                        break;
                    case AP_BTN_UP:
                        cursor--;
                        if (cursor < 0) cursor = count - 1;
                        break;
                    case AP_BTN_DOWN:
                        cursor++;
                        if (cursor >= count) cursor = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        ap_clear_screen();
        ap_draw_background();

        int sw = ap_get_screen_width();
        int pad = AP_DS(5);

        TTF_Font *font_med   = ap_get_font(AP_FONT_MEDIUM);
        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;
        ap_color highlight  = theme->highlight;
        ap_color hl_text    = theme->highlighted_text;

        int y = pad * 3;
        ap_draw_text(font_med, title, pad * 3, y, hint_color);
        y += TTF_FontHeight(font_med) + pad * 3;

        ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
        y += pad * 3;

        int item_h = TTF_FontHeight(font_small) + pad * 3;

        for (int i = 0; i < count; i++) {
            int pill_y = y + i * item_h;
            int text_y = pill_y + (item_h - TTF_FontHeight(font_small)) / 2;

            if (i == cursor) {
                int tw = 0;
                TTF_SizeText(font_small, items[i].label, &tw, NULL);
                int pill_w = tw + pad * 4;
                ap_draw_pill(pad * 2, pill_y, pill_w, item_h, highlight);
                ap_draw_text(font_small, items[i].label, pad * 4, text_y, hl_text);
            } else {
                ap_draw_text(font_small, items[i].label, pad * 4, text_y, text_color);
            }
        }

        pakkit_hint hints[] = {
            {.button = "B",.label = "Back" },
            {.button = "A",.label = "Select" },
        };
        pakkit_draw_hints(hints, 2);

        ap_present();
    }

    return (result->selected_index >= 0) ? AP_OK : AP_CANCELLED;
}

/* --- List --- */

int pakkit_list(pakkit_list_opts *opts, pakkit_list_item *items, int count,
                pakkit_list_result *result) {
    int cursor = 0;
    int scroll = 0;
    int running = 1;

    if (opts->initial_index > 0 && opts->initial_index < count) {
        cursor = opts->initial_index;
    }

    result->selected_index = -1;
    result->action = PAKKIT_ACTION_BACK;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed) {
                switch (ev.button) {
                    case AP_BTN_B:
                        if (!ev.repeated) {
                            result->selected_index = -1;
                            result->action = PAKKIT_ACTION_BACK;
                            running = 0;
                        }
                        break;
                    case AP_BTN_A:
                        if (!ev.repeated) {
                            result->selected_index = cursor;
                            result->action = PAKKIT_ACTION_SELECTED;
                            running = 0;
                        }
                        break;
                    case AP_BTN_UP:
                        cursor--;
                        if (cursor < 0) cursor = count - 1;
                        break;
                    case AP_BTN_DOWN:
                        cursor++;
                        if (cursor >= count) cursor = 0;
                        break;
                    default:
                        if (!ev.repeated && opts->secondary_button != AP_BTN_NONE
                            && ev.button == opts->secondary_button) {
                            result->selected_index = cursor;
                            result->action = PAKKIT_ACTION_SECONDARY;
                            running = 0;
                        }
                        if (!ev.repeated && opts->tertiary_button != AP_BTN_NONE
                            && ev.button == opts->tertiary_button) {
                            result->selected_index = cursor;
                            result->action = PAKKIT_ACTION_TERTIARY;
                            running = 0;
                        }
                        break;
                }
            }
        }

        int sw = ap_get_screen_width();
        int sh = ap_get_screen_height();
        int pad = AP_DS(5);

        TTF_Font *font_med   = ap_get_font(AP_FONT_MEDIUM);
        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
        TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

        int item_h = TTF_FontHeight(font_small) + pad * 3;
        int title_h = TTF_FontHeight(font_med) + pad * 3 + 1 + pad * 3;
        int hint_h = TTF_FontHeight(font_tiny) + pad * 2;
        int list_area_h = sh - title_h - hint_h - pad;
        int visible = list_area_h / item_h;
        if (visible < 1) visible = 1;

        if (cursor < scroll) scroll = cursor;
        if (cursor >= scroll + visible) scroll = cursor - visible + 1;

        ap_clear_screen();
        ap_draw_background();

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;
        ap_color highlight  = theme->highlight;
        ap_color hl_text    = theme->highlighted_text;

        int y = pad * 3;
        if (opts->title) {
            ap_draw_text(font_med, opts->title, pad * 3, y, hint_color);
            y += TTF_FontHeight(font_med) + pad * 3;
        }

        ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
        y += pad * 3;

        int list_top = y;
        SDL_Rect clip = { 0, list_top, sw, list_area_h };
        SDL_RenderSetClipRect(ap__g.renderer, &clip);

        for (int i = scroll; i < count && i < scroll + visible; i++) {
            int item_y = list_top + (i - scroll) * item_h;
            int text_y = item_y + (item_h - TTF_FontHeight(font_small)) / 2;

            if (i == cursor) {
                int pill_w = sw - pad * 4;
                if (opts->text_width_pills) {
                    int tw = ap_measure_text_ellipsized(font_small, items[i].label, sw - pad * 8);
                    pill_w = tw + pad * 4;
                }
                ap_draw_pill(pad * 2, item_y, pill_w, item_h, highlight);
                ap_draw_text_ellipsized(font_small, items[i].label,
                                        pad * 4, text_y, hl_text, sw - pad * 8);
            } else {
                ap_draw_text_ellipsized(font_small, items[i].label,
                                        pad * 4, text_y, text_color, sw - pad * 8);
            }
        }

        SDL_RenderSetClipRect(ap__g.renderer, NULL);

        if (count > visible) {
            int bar_x = sw - pad * 2;
            int bar_h = list_area_h;
            int thumb_h = (visible * bar_h) / count;
            if (thumb_h < pad * 2) thumb_h = pad * 2;
            int thumb_y = list_top + (scroll * (bar_h - thumb_h)) / (count - visible);
            ap_color bar_color = { hint_color.r, hint_color.g, hint_color.b, 80 };
            ap_color thumb_color = { hint_color.r, hint_color.g, hint_color.b, 160 };
            ap_draw_rect(bar_x, list_top, 3, bar_h, bar_color);
            ap_draw_rect(bar_x, thumb_y, 3, thumb_h, thumb_color);
        }

        if (opts->hints && opts->hint_count > 0)
            pakkit_draw_hints(opts->hints, opts->hint_count);

        ap_present();
    }

    return (result->action == PAKKIT_ACTION_SELECTED) ? AP_OK : AP_CANCELLED;
}

/* --- Message dialog --- */

void pakkit_message(const char *message, const char *button_label) {
    if (!button_label) button_label = "OK";
    int running = 1;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed && !ev.repeated) {
                if (ev.button == AP_BTN_A || ev.button == AP_BTN_B)
                    running = 0;
            }
        }

        ap_clear_screen();
        ap_draw_background();

        int sw = ap_get_screen_width();
        int sh = ap_get_screen_height();
        int pad = AP_DS(5);

        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
        TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;

        int max_w = sw - pad * 8;
        int msg_h = ap_measure_wrapped_text_height(font_small, message, max_w);
        int msg_y = (sh - msg_h) / 2;
        ap_draw_text_wrapped(font_small, message, pad * 4, msg_y, max_w,
                             text_color, AP_ALIGN_CENTER);

        char hint_buf[64];
        snprintf(hint_buf, sizeof(hint_buf), "A: %s", button_label);
        int hint_w = ap_measure_text(font_tiny, hint_buf);
        int hint_y = sh - TTF_FontHeight(font_tiny) - pad;
        ap_draw_text(font_tiny, hint_buf, (sw - hint_w) / 2, hint_y, hint_color);

        ap_present();
    }
}

/* --- Confirm dialog --- */

int pakkit_confirm(const char *message, const char *confirm_label,
                   const char *cancel_label) {
    if (!confirm_label) confirm_label = "Confirm";
    if (!cancel_label) cancel_label = "Cancel";
    int confirmed = 0;
    int running = 1;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed && !ev.repeated) {
                if (ev.button == AP_BTN_A) {
                    confirmed = 1;
                    running = 0;
                } else if (ev.button == AP_BTN_B) {
                    confirmed = 0;
                    running = 0;
                }
            }
        }

        ap_clear_screen();
        ap_draw_background();

        int sw = ap_get_screen_width();
        int sh = ap_get_screen_height();
        int pad = AP_DS(5);

        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;

        int max_w = sw - pad * 8;
        int msg_h = ap_measure_wrapped_text_height(font_small, message, max_w);
        int msg_y = (sh - msg_h) / 2;
        ap_draw_text_wrapped(font_small, message, pad * 4, msg_y, max_w,
                             text_color, AP_ALIGN_CENTER);

        pakkit_hint hints[] = {
            {.button = "B",.label = cancel_label },
            {.button = "A",.label = confirm_label },
        };
        pakkit_draw_hints(hints, 2);

        ap_present();
    }

    return confirmed;
}

/* --- Scroll helper --- */

void pakkit_scroll_handle_input(pakkit_scroll_state *s, int direction, int step) {
    s->target_scroll_y += direction * step;
    if (s->target_scroll_y < 0) s->target_scroll_y = 0;
    if (s->target_scroll_y > s->last_max_scroll) s->target_scroll_y = s->last_max_scroll;
}

void pakkit_scroll_animate(pakkit_scroll_state *s) {
    int diff = s->target_scroll_y - s->scroll_y;
    if (diff == 0) return;
    int step = (int)((float)diff * PAKKIT_SCROLL_SMOOTH_FACTOR);
    if (step == 0) step = (diff > 0) ? 1 : -1;
    s->scroll_y += step;
    if (s->scroll_y != s->target_scroll_y) ap_request_frame();
}

void pakkit_scroll_update(pakkit_scroll_state *s, int content_height, int viewport_height) {
    int max_scroll = content_height - viewport_height;
    if (max_scroll < 0) max_scroll = 0;
    s->last_max_scroll = max_scroll;
    if (s->target_scroll_y > s->last_max_scroll) s->target_scroll_y = s->last_max_scroll;
    if (s->scroll_y > s->last_max_scroll) s->scroll_y = s->last_max_scroll;
}

/* --- Detail screen --- */

void pakkit_detail_screen(pakkit_detail_opts *opts) {
    int running = 1;
    pakkit_scroll_state scroll = {0};

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed) {
                switch (ev.button) {
                    case AP_BTN_B:
                        if (!ev.repeated) running = 0;
                        break;
                    case AP_BTN_UP:
                        pakkit_scroll_handle_input(&scroll, -1, PAKKIT_SCROLL_STEP);
                        break;
                    case AP_BTN_DOWN:
                        pakkit_scroll_handle_input(&scroll, 1, PAKKIT_SCROLL_STEP);
                        break;
                    default:
                        break;
                }
            }
        }

        pakkit_scroll_animate(&scroll);

        ap_clear_screen();
        ap_draw_background();

        int sw = ap_get_screen_width();
        int sh = ap_get_screen_height();
        int pad = AP_DS(5);

        TTF_Font *font_large = ap_get_font(AP_FONT_LARGE);
        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
        TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;

        int hint_font_h = TTF_FontHeight(font_tiny);
        int footer_h = hint_font_h + pad * 2;
        int content_top = pad;
        int content_bottom = sh - footer_h;
        int content_h = content_bottom - content_top;

        SDL_Rect clip = { 0, content_top, sw, content_h };
        SDL_RenderSetClipRect(ap__g.renderer, &clip);

        int y = content_top - scroll.scroll_y;

        if (opts->title) {
            ap_draw_text(font_large, opts->title, pad * 3, y, text_color);
            y += TTF_FontHeight(font_large) + pad;
        }

        if (opts->subtitle) {
            ap_draw_text(font_small, opts->subtitle, pad * 3, y, hint_color);
            y += TTF_FontHeight(font_small) + pad * 3;
        } else {
            y += pad * 2;
        }

        ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
        y += pad * 3;

        if (opts->info && opts->info_count > 0) {
            int label_x = pad * 3;
            int value_x = pad * 3 + AP_DS(80);
            int row_h = TTF_FontHeight(font_small) + pad;

            for (int i = 0; i < opts->info_count; i++) {
                ap_draw_text(font_small, opts->info[i].key, label_x, y, hint_color);
                ap_draw_text(font_small, opts->info[i].value, value_x, y, text_color);
                y += row_h;
            }

            y += pad * 2;
            ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
            y += pad * 3;
        }

        if (opts->credits && opts->credit_count > 0) {
            int row_h = TTF_FontHeight(font_small) + pad;
            for (int i = 0; i < opts->credit_count; i++) {
                ap_color c = (i == 0) ? text_color : hint_color;
                ap_draw_text(font_small, opts->credits[i], pad * 3, y, c);
                y += row_h;
            }
            y += pad * 2;
        }

        int total_content = y + scroll.scroll_y - content_top;

        SDL_RenderSetClipRect(ap__g.renderer, NULL);

        pakkit_scroll_update(&scroll, total_content, content_h);

        pakkit_hint hints[] = {
            {.button = "B",.label = "Back" },
        };
        pakkit_draw_hints(hints, 1);

        ap_present();
    }
}

/* --- Keyboard --- */

#define PAKKIT_KB_ROWS      4
#define PAKKIT_KB_COLS_MAX  10

typedef enum {
    PAKKIT_KB_PAGE_LOWER,
    PAKKIT_KB_PAGE_UPPER,
    PAKKIT_KB_PAGE_SYMBOLS,
} pakkit_kb_page;

typedef struct {
    const char *keys[PAKKIT_KB_COLS_MAX];
    int count;
} pakkit_kb_row;

static const pakkit_kb_row pakkit__kb_lower[PAKKIT_KB_ROWS] = {
    {.keys = {"q","w","e","r","t","y","u","i","o","p"},.count = 10 },
    {.keys = {"a","s","d","f","g","h","j","k","l",""},.count = 9 },
    {.keys = {"z","x","c","v","b","n","m",".","-","@"},.count = 10 },
    {.keys = {"Shift","Space","Sym","","","","","","",""},.count = 3 },
};

static const pakkit_kb_row pakkit__kb_upper[PAKKIT_KB_ROWS] = {
    {.keys = {"Q","W","E","R","T","Y","U","I","O","P"},.count = 10 },
    {.keys = {"A","S","D","F","G","H","J","K","L",""},.count = 9 },
    {.keys = {"Z","X","C","V","B","N","M",".","-","@"},.count = 10 },
    {.keys = {"abc","Space","Sym","","","","","","",""},.count = 3 },
};

static const pakkit_kb_row pakkit__kb_symbols[PAKKIT_KB_ROWS] = {
    {.keys = {"1","2","3","4","5","6","7","8","9","0"},.count = 10 },
    {.keys = {"!","#","$","%","^","&","*","(",")",""},.count = 9 },
    {.keys = {"-","_","=","+","/",":",";","'","\"","?"},.count = 10 },
    {.keys = {".","Space","abc","","","","","","",""},.count = 3 },
};

static const pakkit_kb_row *pakkit__kb_get_page(pakkit_kb_page page) {
    switch (page) {
        case PAKKIT_KB_PAGE_UPPER:   return pakkit__kb_upper;
        case PAKKIT_KB_PAGE_SYMBOLS: return pakkit__kb_symbols;
        default:                     return pakkit__kb_lower;
    }
}

static void pakkit__kb_key_geometry(const pakkit_kb_row *rows, int row, int col,
                                     int sw, int pad, int key_gap,
                                     int *out_x, int *out_w) {
    int col_count = rows[row].count;
    int avail_w = sw - pad * 4;
    int total_gap = key_gap * (col_count - 1);
    int content_w = avail_w - total_gap;

    int total_units = 0;
    for (int c = 0; c < col_count; c++) {
        const char *k = rows[row].keys[c];
        if (k[0] == '\0') continue;
        if (strcmp(k, "Space") == 0) total_units += 4;
        else if (strcmp(k, "Shift") == 0 || strcmp(k, "abc") == 0 ||
                 strcmp(k, "Sym") == 0) total_units += 2;
        else total_units += 1;
    }
    if (total_units == 0) total_units = 1;

    int unit_w = content_w / total_units;
    int kx = pad * 2;

    for (int c = 0; c < col_count; c++) {
        const char *k = rows[row].keys[c];
        if (k[0] == '\0') continue;

        int kw;
        if (strcmp(k, "Space") == 0) kw = unit_w * 4;
        else if (strcmp(k, "Shift") == 0 || strcmp(k, "abc") == 0 ||
                 strcmp(k, "Sym") == 0) kw = unit_w * 2;
        else kw = unit_w;

        int is_last = 1;
        for (int nc = c + 1; nc < col_count; nc++) {
            if (rows[row].keys[nc][0] != '\0') { is_last = 0; break; }
        }
        if (is_last) kw = (pad * 2 + avail_w) - kx;

        if (c == col) { *out_x = kx; *out_w = kw; return; }
        kx += kw + key_gap;
    }
    *out_x = pad * 2;
    *out_w = unit_w;
}

static int pakkit__kb_map_col(const pakkit_kb_row *from_rows, int from_row, int from_col,
                               const pakkit_kb_row *to_rows, int to_row,
                               int sw, int pad, int key_gap) {
    int from_x, from_w;
    pakkit__kb_key_geometry(from_rows, from_row, from_col, sw, pad, key_gap, &from_x, &from_w);
    int from_cx = from_x + from_w / 2;

    int best_col = 0, best_dist = 999999;
    for (int c = 0; c < to_rows[to_row].count; c++) {
        if (to_rows[to_row].keys[c][0] == '\0') continue;
        int to_x, to_w;
        pakkit__kb_key_geometry(to_rows, to_row, c, sw, pad, key_gap, &to_x, &to_w);
        int to_cx = to_x + to_w / 2;
        int dist = abs(to_cx - from_cx);
        if (dist < best_dist) { best_dist = dist; best_col = c; }
    }
    return best_col;
}

int pakkit_keyboard(const char *initial_text, pakkit_keyboard_opts *opts,
                    pakkit_keyboard_result *result) {
    memset(result, 0, sizeof(*result));
    if (initial_text)
        strncpy(result->text, initial_text, PAKKIT_KB_MAX_TEXT - 1);

    int text_len = (int)strlen(result->text);
    int cursor_row = 0, cursor_col = 0;
    int shortcut_active = 0;
    int shortcut_col = 0;
    pakkit_kb_page page = PAKKIT_KB_PAGE_LOWER;
    int running = 1;
    int confirmed = 0;

    int has_shortcuts = (opts && opts->shortcuts && opts->shortcut_count > 0);

    while (running) {
        const pakkit_kb_row *rows = pakkit__kb_get_page(page);

        int sw = ap_get_screen_width();
        int pad = AP_DS(5);
        int key_gap = pad;

        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (!ev.pressed) continue;

            switch (ev.button) {
                case AP_BTN_START:
                    if (!ev.repeated) { confirmed = 1; running = 0; }
                    break;
                case AP_BTN_Y:
                    if (!ev.repeated) { confirmed = 0; running = 0; }
                    break;
                case AP_BTN_B:
                    if (text_len > 0) {
                        result->text[--text_len] = '\0';
                    }
                    break;
                case AP_BTN_UP:
                    if (shortcut_active) {
                    } else if (cursor_row == 0 && has_shortcuts) {
                        shortcut_active = 1;
                        if (shortcut_col >= opts->shortcut_count)
                            shortcut_col = opts->shortcut_count - 1;
                    } else if (cursor_row > 0) {
                        int old_row = cursor_row;
                        cursor_row--;
                        cursor_col = pakkit__kb_map_col(rows, old_row, cursor_col,
                                                         rows, cursor_row, sw, pad, key_gap);
                    }
                    break;
                case AP_BTN_DOWN:
                    if (shortcut_active) {
                        shortcut_active = 0;
                        cursor_row = 0;
                        if (cursor_col >= rows[cursor_row].count)
                            cursor_col = rows[cursor_row].count - 1;
                    } else if (cursor_row < PAKKIT_KB_ROWS - 1) {
                        int old_row = cursor_row;
                        cursor_row++;
                        cursor_col = pakkit__kb_map_col(rows, old_row, cursor_col,
                                                         rows, cursor_row, sw, pad, key_gap);
                    }
                    break;
                case AP_BTN_LEFT:
                    if (shortcut_active) {
                        shortcut_col--;
                        if (shortcut_col < 0) shortcut_col = opts->shortcut_count - 1;
                    } else {
                        cursor_col--;
                        while (cursor_col >= 0 && rows[cursor_row].keys[cursor_col][0] == '\0')
                            cursor_col--;
                        if (cursor_col < 0) {
                            cursor_col = rows[cursor_row].count - 1;
                            while (cursor_col > 0 && rows[cursor_row].keys[cursor_col][0] == '\0')
                                cursor_col--;
                        }
                    }
                    break;
                case AP_BTN_RIGHT:
                    if (shortcut_active) {
                        shortcut_col++;
                        if (shortcut_col >= opts->shortcut_count) shortcut_col = 0;
                    } else {
                        cursor_col++;
                        while (cursor_col < rows[cursor_row].count &&
                               rows[cursor_row].keys[cursor_col][0] == '\0')
                            cursor_col++;
                        if (cursor_col >= rows[cursor_row].count) {
                            cursor_col = 0;
                        }
                    }
                    break;
                case AP_BTN_A:
                    if (ev.repeated) break;
                    if (shortcut_active && has_shortcuts) {
                        const char *sc = opts->shortcuts[shortcut_col];
                        int sc_len = (int)strlen(sc);
                        if (text_len + sc_len < PAKKIT_KB_MAX_TEXT) {
                            strcat(result->text, sc);
                            text_len += sc_len;
                        }
                    } else {
                        const char *key = rows[cursor_row].keys[cursor_col];
                        if (strcmp(key, "Space") == 0) {
                            if (text_len + 1 < PAKKIT_KB_MAX_TEXT) {
                                result->text[text_len++] = ' ';
                                result->text[text_len] = '\0';
                            }
                        } else if (strcmp(key, "Shift") == 0) {
                            page = PAKKIT_KB_PAGE_UPPER;
                            rows = pakkit__kb_get_page(page);
                            cursor_col = pakkit__kb_map_col(rows, cursor_row, cursor_col,
                                                             rows, cursor_row, sw, pad, key_gap);
                        } else if (strcmp(key, "abc") == 0) {
                            page = PAKKIT_KB_PAGE_LOWER;
                            rows = pakkit__kb_get_page(page);
                            cursor_col = pakkit__kb_map_col(rows, cursor_row, cursor_col,
                                                             rows, cursor_row, sw, pad, key_gap);
                        } else if (strcmp(key, "Sym") == 0) {
                            page = PAKKIT_KB_PAGE_SYMBOLS;
                            rows = pakkit__kb_get_page(page);
                            cursor_col = pakkit__kb_map_col(rows, cursor_row, cursor_col,
                                                             rows, cursor_row, sw, pad, key_gap);
                        } else if (key[0] != '\0') {
                            int kl = (int)strlen(key);
                            if (text_len + kl < PAKKIT_KB_MAX_TEXT) {
                                strcat(result->text, key);
                                text_len += kl;
                            }
                            if (page == PAKKIT_KB_PAGE_UPPER && kl == 1)
                                page = PAKKIT_KB_PAGE_LOWER;
                        }
                    }
                    break;
                case AP_BTN_L1:
                    if (page == PAKKIT_KB_PAGE_LOWER) page = PAKKIT_KB_PAGE_UPPER;
                    else if (page == PAKKIT_KB_PAGE_UPPER) page = PAKKIT_KB_PAGE_LOWER;
                    break;
                case AP_BTN_R1:
                    if (page == PAKKIT_KB_PAGE_SYMBOLS) page = PAKKIT_KB_PAGE_LOWER;
                    else page = PAKKIT_KB_PAGE_SYMBOLS;
                    break;
                default:
                    break;
            }
        }

        rows = pakkit__kb_get_page(page);

        if (cursor_row >= PAKKIT_KB_ROWS) cursor_row = PAKKIT_KB_ROWS - 1;
        if (cursor_col >= rows[cursor_row].count) cursor_col = rows[cursor_row].count - 1;
        while (cursor_col > 0 && rows[cursor_row].keys[cursor_col][0] == '\0')
            cursor_col--;

        ap_clear_screen();
        ap_draw_background();

        int sh = ap_get_screen_height();

        TTF_Font *font_med   = ap_get_font(AP_FONT_MEDIUM);
        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
        TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;
        ap_color highlight  = theme->highlight;
        ap_color hl_text    = theme->highlighted_text;

        int y = pad * 2;

        if (opts && opts->prompt && opts->prompt[0]) {
            ap_draw_text(font_tiny, opts->prompt, pad * 3, y, hint_color);
            y += TTF_FontHeight(font_tiny) + pad;
        }

        int field_h = TTF_FontHeight(font_med) + pad * 2;
        ap_color field_bg = { hint_color.r, hint_color.g, hint_color.b, 40 };
        ap_draw_rect(pad * 2, y, sw - pad * 4, field_h, field_bg);

        char display_text[PAKKIT_KB_MAX_TEXT + 2];
        snprintf(display_text, sizeof(display_text), "%s|", result->text);
        ap_draw_text_ellipsized(font_med, display_text,
                                pad * 3, y + pad, text_color, sw - pad * 6);
        y += field_h + pad * 2;

        if (has_shortcuts) {
            int sc_h = TTF_FontHeight(font_tiny) + pad * 2;
            int sc_x = pad * 2;
            for (int i = 0; i < opts->shortcut_count; i++) {
                int sc_w = ap_measure_text(font_tiny, opts->shortcuts[i]) + pad * 3;
                if (shortcut_active && i == shortcut_col) {
                    ap_draw_rect(sc_x, y, sc_w, sc_h, highlight);
                    ap_draw_text(font_tiny, opts->shortcuts[i],
                                 sc_x + pad + pad / 2, y + pad, hl_text);
                } else {
                    ap_color sc_bg = { hint_color.r, hint_color.g, hint_color.b, 60 };
                    ap_draw_rect(sc_x, y, sc_w, sc_h, sc_bg);
                    ap_draw_text(font_tiny, opts->shortcuts[i],
                                 sc_x + pad + pad / 2, y + pad, text_color);
                }
                sc_x += sc_w + pad;
            }
            y += sc_h + pad * 2;
        }

        int grid_area_h = sh - y - TTF_FontHeight(font_tiny) - pad * 3;
        int row_h = grid_area_h / PAKKIT_KB_ROWS;

        for (int r = 0; r < PAKKIT_KB_ROWS; r++) {
            int col_count = rows[r].count;
            if (col_count <= 0) continue;

            int ky = y + r * row_h;

            for (int c = 0; c < col_count; c++) {
                const char *key = rows[r].keys[c];
                if (key[0] == '\0') continue;

                int kx, kw;
                pakkit__kb_key_geometry(rows, r, c, sw, pad, key_gap, &kx, &kw);
                int kh = row_h - key_gap;

                int is_selected = (!shortcut_active && r == cursor_row && c == cursor_col);

                if (is_selected) {
                    ap_draw_rect(kx, ky, kw, kh, highlight);
                    int tw = ap_measure_text(font_small, key);
                    ap_draw_text(font_small, key, kx + (kw - tw) / 2,
                                 ky + (kh - TTF_FontHeight(font_small)) / 2, hl_text);
                } else {
                    ap_color key_bg = { hint_color.r, hint_color.g, hint_color.b, 40 };
                    ap_draw_rect(kx, ky, kw, kh, key_bg);
                    int tw = ap_measure_text(font_small, key);
                    ap_draw_text(font_small, key, kx + (kw - tw) / 2,
                                 ky + (kh - TTF_FontHeight(font_small)) / 2, text_color);
                }
            }
        }

        pakkit_hint hints[] = {
            {.button = "B",.label = "Delete" },
            {.button = "Y",.label = "Cancel" },
            {.button = "Start",.label = "Done" },
        };
        pakkit_draw_hints(hints, 3);

        ap_present();
    }

    return confirmed ? AP_OK : AP_CANCELLED;
}

#endif /* PAKKIT_UI_IMPLEMENTATION */
#endif /* PAKKIT_UI_H */