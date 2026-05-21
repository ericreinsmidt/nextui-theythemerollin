/*
 * TheyTheMeRollin — Theme Manager for NextUI
 *
 * Browse, download, and apply wallpapers and icons on TrimUI Brick and Smart Pro.
 * Themes are fetched from a catalog and installed to the device filesystem.
 *
 * Top-level menu:
 *   Themes     — full theme packages (wallpapers + icons), browse/installed
 *   Wallpapers — wallpaper packs and per-system management, browse/installed
 *   Icons      — icon packs and per-system management, browse/installed
 */

#define AP_IMPLEMENTATION
#include "apostrophe.h"

#define PAKKIT_SCROLL_STEP 85
#define PAKKIT_UI_IMPLEMENTATION
#include "pakkit_ui.h"

#include "cJSON.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

/* -----------------------------------------------------------------------
 * Constants
 * ----------------------------------------------------------------------- */

#define MAX_PATH_LEN     1024
#define MAX_PATH_BUF     2048
#define MAX_NAME_LEN      256
#define MAX_ENTRIES       256

#define CATALOG_URL      "https://raw.githubusercontent.com/ericreinsmidt/nextui-theme-catalog/main/catalog.json"

/* On-device paths */
#define SDCARD_ROOT      "/mnt/SDCARD"
#define ROOT_BG          SDCARD_ROOT "/bg.png"
#define ROMS_DIR         SDCARD_ROOT "/Roms"
#define ROMS_MEDIA_DIR   SDCARD_ROOT "/Roms/.media"
#define COLLECTIONS_DIR  SDCARD_ROOT "/Collections"
#define RECENTLY_PLAYED  SDCARD_ROOT "/Recently Played"
#define TOOLS_DIR        SDCARD_ROOT "/Tools/tg5040"

#define DATA_DIR         SDCARD_ROOT "/.userdata/shared/theythemerollin"
#define THEMES_DIR       DATA_DIR "/themes"
#define BACKUP_DIR       DATA_DIR "/backup"

/* -----------------------------------------------------------------------
 * Types
 * ----------------------------------------------------------------------- */

typedef enum {
    CATEGORY_THEMES,
    CATEGORY_WALLPAPERS,
    CATEGORY_ICONS,
} category_t;

typedef struct {
    char id[MAX_NAME_LEN];
    char name[MAX_NAME_LEN];
    char author[MAX_NAME_LEN];
    char version[64];
    char description[512];
    bool has_wallpapers;
    bool has_icons;
    char wallpaper_mode[32];
    int  wallpaper_count;
    int  icon_count;
    char url[MAX_PATH_LEN];
    char preview_url[MAX_PATH_LEN];
    bool installed;
} catalog_entry;

typedef struct {
    char name[MAX_NAME_LEN];
    char tag[64];
    char rom_dir[MAX_PATH_LEN];
} system_entry;

typedef struct {
    catalog_entry *entries[MAX_ENTRIES];
    int count;
} filtered_list;

/* -----------------------------------------------------------------------
 * State
 * ----------------------------------------------------------------------- */

static catalog_entry catalog[MAX_ENTRIES];
static int catalog_count = 0;

static system_entry systems[MAX_ENTRIES];
static int system_count = 0;

static char pak_dir[MAX_PATH_LEN];

/* -----------------------------------------------------------------------
 * System discovery
 * ----------------------------------------------------------------------- */

static void discover_systems(void) {
    system_count = 0;
    DIR *d = opendir(ROMS_DIR);
    if (!d) return;

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL && system_count < MAX_ENTRIES) {
        if (ent->d_name[0] == '.') continue;

        char *open = strrchr(ent->d_name, '(');
        char *close = strrchr(ent->d_name, ')');
        if (!open || !close || close <= open) continue;

        system_entry *s = &systems[system_count];
        snprintf(s->name, MAX_NAME_LEN, "%s", ent->d_name);

        size_t tag_len = (size_t)(close - open + 1);
        if (tag_len >= sizeof(s->tag)) tag_len = sizeof(s->tag) - 1;
        memcpy(s->tag, open, tag_len);
        s->tag[tag_len] = '\0';

        snprintf(s->rom_dir, MAX_PATH_LEN, "%s/%s", ROMS_DIR, ent->d_name);
        system_count++;
    }
    closedir(d);
}

/* -----------------------------------------------------------------------
 * Installed scan
 * ----------------------------------------------------------------------- */

static void scan_installed(void) {
    for (int i = 0; i < catalog_count; i++) {
        char path[MAX_PATH_BUF];
        snprintf(path, sizeof(path), "%s/%s/manifest.json", THEMES_DIR, catalog[i].id);
        catalog[i].installed = (access(path, F_OK) == 0);
    }
}

/* -----------------------------------------------------------------------
 * Catalog fetch + parse
 * ----------------------------------------------------------------------- */

#define CATALOG_FILE  DATA_DIR "/catalog.json"

static bool fetch_catalog_json(void) {
    char cmd[MAX_PATH_BUF];
    snprintf(cmd, sizeof(cmd),
             "curl -sfk -o \"%s\" -m 15 \"%s\"",
             CATALOG_FILE, CATALOG_URL);
    return (system(cmd) == 0);
}

static char *read_file_contents(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (len <= 0 || len > 4 * 1024 * 1024) {
        fclose(f);
        return NULL;
    }

    char *buf = malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }

    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static void parse_entry(cJSON *obj, catalog_entry *e) {
    cJSON *v;
    v = cJSON_GetObjectItem(obj, "id");
    if (v && v->valuestring) snprintf(e->id, sizeof(e->id), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "name");
    if (v && v->valuestring) snprintf(e->name, sizeof(e->name), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "author");
    if (v && v->valuestring) snprintf(e->author, sizeof(e->author), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "version");
    if (v && v->valuestring) snprintf(e->version, sizeof(e->version), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "description");
    if (v && v->valuestring) snprintf(e->description, sizeof(e->description), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "has_wallpapers");
    e->has_wallpapers = (v && cJSON_IsTrue(v));

    v = cJSON_GetObjectItem(obj, "has_icons");
    e->has_icons = (v && cJSON_IsTrue(v));

    v = cJSON_GetObjectItem(obj, "wallpaper_mode");
    if (v && v->valuestring) snprintf(e->wallpaper_mode, sizeof(e->wallpaper_mode), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "wallpaper_count");
    e->wallpaper_count = (v && cJSON_IsNumber(v)) ? v->valueint : 0;

    v = cJSON_GetObjectItem(obj, "icon_count");
    e->icon_count = (v && cJSON_IsNumber(v)) ? v->valueint : 0;

    v = cJSON_GetObjectItem(obj, "url");
    if (v && v->valuestring) snprintf(e->url, sizeof(e->url), "%s", v->valuestring);

    v = cJSON_GetObjectItem(obj, "preview_url");
    if (v && v->valuestring) snprintf(e->preview_url, sizeof(e->preview_url), "%s", v->valuestring);

    e->installed = false;
}

static int find_catalog_entry(const char *id) {
    for (int i = 0; i < catalog_count; i++) {
        if (strcmp(catalog[i].id, id) == 0) return i;
    }
    return -1;
}

static void merge_entries_from_array(cJSON *arr) {
    if (!arr || !cJSON_IsArray(arr)) return;

    cJSON *obj;
    cJSON_ArrayForEach(obj, arr) {
        cJSON *id_val = cJSON_GetObjectItem(obj, "id");
        if (!id_val || !id_val->valuestring) continue;

        int idx = find_catalog_entry(id_val->valuestring);
        if (idx >= 0) {
            /* Entry already exists — merge flags */
            cJSON *v;
            v = cJSON_GetObjectItem(obj, "has_wallpapers");
            if (v && cJSON_IsTrue(v)) catalog[idx].has_wallpapers = true;
            v = cJSON_GetObjectItem(obj, "has_icons");
            if (v && cJSON_IsTrue(v)) catalog[idx].has_icons = true;
        } else if (catalog_count < MAX_ENTRIES) {
            parse_entry(obj, &catalog[catalog_count]);
            catalog_count++;
        }
    }
}

static bool parse_catalog(void) {
    char *json = read_file_contents(CATALOG_FILE);
    if (!json) return false;

    cJSON *root = cJSON_Parse(json);
    free(json);
    if (!root) return false;

    catalog_count = 0;

    /* Parse all three sections — entries are deduplicated by id */
    merge_entries_from_array(cJSON_GetObjectItem(root, "themes"));
    merge_entries_from_array(cJSON_GetObjectItem(root, "wallpapers"));
    merge_entries_from_array(cJSON_GetObjectItem(root, "icons"));

    cJSON_Delete(root);
    return (catalog_count > 0);
}

/* -----------------------------------------------------------------------
 * Filter catalog for a category
 * ----------------------------------------------------------------------- */

static filtered_list filter_catalog(category_t cat, bool installed_only) {
    filtered_list f = {0};
    for (int i = 0; i < catalog_count; i++) {
        bool match = false;
        switch (cat) {
            case CATEGORY_THEMES:
                match = catalog[i].has_wallpapers && catalog[i].has_icons;
                break;
            case CATEGORY_WALLPAPERS:
                match = catalog[i].has_wallpapers;
                break;
            case CATEGORY_ICONS:
                match = catalog[i].has_icons;
                break;
        }
        if (match && (!installed_only || catalog[i].installed)) {
            f.entries[f.count++] = &catalog[i];
        }
    }
    return f;
}

/* -----------------------------------------------------------------------
 * File operations
 * ----------------------------------------------------------------------- */

static void copy_file(const char *src, const char *dst) {
    char dir[MAX_PATH_BUF];
    snprintf(dir, sizeof(dir), "%s", dst);
    char *slash = strrchr(dir, '/');
    if (slash) {
        *slash = '\0';
        char cmd[MAX_PATH_BUF + 32];
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir);
        system(cmd);
    }

    char cmd[MAX_PATH_BUF * 2 + 16];
    snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s\"", src, dst);
    system(cmd);
}

/* -----------------------------------------------------------------------
 * Backup current theme
 * ----------------------------------------------------------------------- */

static void backup_current(void) {
    char cmd[MAX_PATH_BUF * 3];

    /* Clear old backup */
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"/*", BACKUP_DIR);
    system(cmd);

    /* Back up root wallpaper */
    if (access(ROOT_BG, F_OK) == 0) {
        snprintf(cmd, sizeof(cmd), "mkdir -p \"%s/wallpapers\"", BACKUP_DIR);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "cp \"%s\" \"%s/wallpapers/root.png\"",
                 ROOT_BG, BACKUP_DIR);
        system(cmd);
    }

    /* Back up per-system wallpapers and icons */
    for (int i = 0; i < system_count; i++) {
        char src[MAX_PATH_BUF];
        char dst[MAX_PATH_BUF];

        /* System bg */
        snprintf(src, sizeof(src), "%s/.media/bg.png", systems[i].rom_dir);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/wallpapers/systems/%s.png",
                     BACKUP_DIR, systems[i].tag);
            copy_file(src, dst);
        }

        /* System bglist */
        snprintf(src, sizeof(src), "%s/.media/bglist.png", systems[i].rom_dir);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/wallpapers/lists/%s.png",
                     BACKUP_DIR, systems[i].tag);
            copy_file(src, dst);
        }

        /* System icon */
        snprintf(src, sizeof(src), "%s/%s.png", ROMS_MEDIA_DIR, systems[i].name);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/icons/systems/%s.png",
                     BACKUP_DIR, systems[i].tag);
            copy_file(src, dst);
        }
    }

    /* Back up special icons */
    char specials[][2][MAX_PATH_LEN] = {
        {SDCARD_ROOT "/.media/Collections.png",       "icons/collections.png"},
        {SDCARD_ROOT "/.media/Recently Played.png",    "icons/recently_played.png"},
        {SDCARD_ROOT "/Tools/.media/tg5040.png",       "icons/tools.png"},
    };
    for (int i = 0; i < 3; i++) {
        if (access(specials[i][0], F_OK) == 0) {
            char dst[MAX_PATH_BUF];
            snprintf(dst, sizeof(dst), "%s/%s", BACKUP_DIR, specials[i][1]);
            copy_file(specials[i][0], dst);
        }
    }
}

static bool has_backup(void) {
    char path[MAX_PATH_BUF];
    snprintf(path, sizeof(path), "%s/wallpapers", BACKUP_DIR);
    if (access(path, F_OK) == 0) return true;
    snprintf(path, sizeof(path), "%s/icons", BACKUP_DIR);
    if (access(path, F_OK) == 0) return true;
    return false;
}

static void restore_backup(void) {
    /* Restore root wallpaper */
    char root_src[MAX_PATH_BUF];
    snprintf(root_src, sizeof(root_src), "%s/wallpapers/root.png", BACKUP_DIR);
    if (access(root_src, F_OK) == 0) {
        copy_file(root_src, ROOT_BG);
    }

    /* Restore per-system */
    for (int i = 0; i < system_count; i++) {
        char src[MAX_PATH_BUF];
        char dst[MAX_PATH_BUF];

        snprintf(src, sizeof(src), "%s/wallpapers/systems/%s.png",
                 BACKUP_DIR, systems[i].tag);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/.media/bg.png", systems[i].rom_dir);
            copy_file(src, dst);
        }

        snprintf(src, sizeof(src), "%s/wallpapers/lists/%s.png",
                 BACKUP_DIR, systems[i].tag);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/.media/bglist.png", systems[i].rom_dir);
            copy_file(src, dst);
        }

        snprintf(src, sizeof(src), "%s/icons/systems/%s.png",
                 BACKUP_DIR, systems[i].tag);
        if (access(src, F_OK) == 0) {
            snprintf(dst, sizeof(dst), "%s/%s.png", ROMS_MEDIA_DIR, systems[i].name);
            copy_file(src, dst);
        }
    }

    /* Restore special icons */
    char specials[][2][MAX_PATH_LEN] = {
        {"icons/collections.png",       SDCARD_ROOT "/.media/Collections.png"},
        {"icons/recently_played.png",   SDCARD_ROOT "/.media/Recently Played.png"},
        {"icons/tools.png",             SDCARD_ROOT "/Tools/.media/tg5040.png"},
    };
    for (int i = 0; i < 3; i++) {
        char src[MAX_PATH_BUF];
        snprintf(src, sizeof(src), "%s/%s", BACKUP_DIR, specials[i][0]);
        if (access(src, F_OK) == 0) {
            copy_file(src, specials[i][1]);
        }
    }
}

/* -----------------------------------------------------------------------
 * Apply theme
 * ----------------------------------------------------------------------- */

static void apply_theme(catalog_entry *entry) {
    char theme_dir[MAX_PATH_LEN];
    snprintf(theme_dir, sizeof(theme_dir), "%s/%s", THEMES_DIR, entry->id);

    if (entry->has_wallpapers) {
        if (strcmp(entry->wallpaper_mode, "universal") == 0) {
            char src[MAX_PATH_BUF];
            snprintf(src, sizeof(src), "%s/wallpapers/wallpaper.png", theme_dir);
            if (access(src, F_OK) == 0) {
                copy_file(src, ROOT_BG);
                for (int i = 0; i < system_count; i++) {
                    char dst[MAX_PATH_BUF];
                    snprintf(dst, sizeof(dst), "%s/.media/bg.png", systems[i].rom_dir);
                    copy_file(src, dst);
                    snprintf(dst, sizeof(dst), "%s/.media/bglist.png", systems[i].rom_dir);
                    copy_file(src, dst);
                }
            }
        } else {
            char root_src[MAX_PATH_BUF];
            snprintf(root_src, sizeof(root_src), "%s/wallpapers/root.png", theme_dir);
            bool has_root = (access(root_src, F_OK) == 0);

            if (has_root) {
                copy_file(root_src, ROOT_BG);
            }

            for (int i = 0; i < system_count; i++) {
                char src[MAX_PATH_BUF];
                char dst[MAX_PATH_BUF];

                snprintf(src, sizeof(src), "%s/wallpapers/systems/%s.png",
                         theme_dir, systems[i].tag);
                if (access(src, F_OK) != 0 && has_root) {
                    snprintf(src, sizeof(src), "%s", root_src);
                }
                if (access(src, F_OK) == 0) {
                    snprintf(dst, sizeof(dst), "%s/.media/bg.png", systems[i].rom_dir);
                    copy_file(src, dst);
                }

                snprintf(src, sizeof(src), "%s/wallpapers/lists/%s.png",
                         theme_dir, systems[i].tag);
                if (access(src, F_OK) != 0 && has_root) {
                    snprintf(src, sizeof(src), "%s", root_src);
                }
                if (access(src, F_OK) == 0) {
                    snprintf(dst, sizeof(dst), "%s/.media/bglist.png", systems[i].rom_dir);
                    copy_file(src, dst);
                }
            }
        }
    }

    if (entry->has_icons) {
        char src[MAX_PATH_BUF];
        char dst[MAX_PATH_BUF];

        for (int i = 0; i < system_count; i++) {
            snprintf(src, sizeof(src), "%s/icons/systems/%s.png",
                     theme_dir, systems[i].tag);
            if (access(src, F_OK) == 0) {
                snprintf(dst, sizeof(dst), "%s/%s.png", ROMS_MEDIA_DIR, systems[i].name);
                copy_file(src, dst);
            }
        }

        snprintf(src, sizeof(src), "%s/icons/collections.png", theme_dir);
        if (access(src, F_OK) == 0) {
            copy_file(src, SDCARD_ROOT "/.media/Collections.png");
        }

        snprintf(src, sizeof(src), "%s/icons/recently_played.png", theme_dir);
        if (access(src, F_OK) == 0) {
            copy_file(src, SDCARD_ROOT "/.media/Recently Played.png");
        }

        snprintf(src, sizeof(src), "%s/icons/tools.png", theme_dir);
        if (access(src, F_OK) == 0) {
            copy_file(src, SDCARD_ROOT "/Tools/.media/tg5040.png");
        }
    }
}

/* -----------------------------------------------------------------------
 * Download + install theme
 * ----------------------------------------------------------------------- */

#define UNZIP_BIN  SDCARD_ROOT "/.tmp_update/tg5040/unzip"

static bool download_theme(catalog_entry *entry) {
    char tmp_file[MAX_PATH_BUF];
    snprintf(tmp_file, sizeof(tmp_file), DATA_DIR "/tmp_download.zip");

    /* Download */
    char cmd[MAX_PATH_BUF * 3];
    snprintf(cmd, sizeof(cmd),
             "curl -sfLk -o \"%s\" -m 60 \"%s\"",
             tmp_file, entry->url);
    if (system(cmd) != 0) {
        remove(tmp_file);
        return false;
    }

    /* Extract into themes/<id>/ */
    char dest[MAX_PATH_BUF];
    snprintf(dest, sizeof(dest), "%s/%s", THEMES_DIR, entry->id);
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dest);
    system(cmd);

    snprintf(cmd, sizeof(cmd),
             "\"%s\" -o -q \"%s\" -d \"%s\"",
             UNZIP_BIN, tmp_file, dest);
    if (system(cmd) != 0) {
        remove(tmp_file);
        return false;
    }

    remove(tmp_file);

    /* Verify manifest exists */
    char manifest[MAX_PATH_BUF];
    snprintf(manifest, sizeof(manifest), "%s/%s/manifest.json", THEMES_DIR, entry->id);
    if (access(manifest, F_OK) != 0) {
        /* Clean up failed install */
        snprintf(cmd, sizeof(cmd), "rm -rf \"%s/%s\"", THEMES_DIR, entry->id);
        system(cmd);
        return false;
    }

    entry->installed = true;
    return true;
}

/* -----------------------------------------------------------------------
 * Entry detail screen
 * ----------------------------------------------------------------------- */

typedef enum {
    DETAIL_BACK,
    DETAIL_ACTION,
    DETAIL_DELETE,
} detail_result_t;

static detail_result_t show_entry_detail(catalog_entry *e) {
    char subtitle[MAX_NAME_LEN + 32];
    snprintf(subtitle, sizeof(subtitle), "by %s", e->author);

    char wp_info[64];
    if (e->has_wallpapers) {
        snprintf(wp_info, sizeof(wp_info), "%s (%d)",
                 e->wallpaper_mode, e->wallpaper_count);
    } else {
        snprintf(wp_info, sizeof(wp_info), "None");
    }

    char icon_info[32];
    if (e->has_icons) {
        snprintf(icon_info, sizeof(icon_info), "%d", e->icon_count);
    } else {
        snprintf(icon_info, sizeof(icon_info), "None");
    }

    char status[32];
    snprintf(status, sizeof(status), "%s", e->installed ? "Installed" : "Not installed");

    pakkit_info_pair info[] = {
        {"Version",     e->version},
        {"Wallpapers",  wp_info},
        {"Icons",       icon_info},
        {"Status",      status},
    };

    const char *credits[] = {e->description};

    pakkit_detail_opts opts = {
        .title = e->name,
        .subtitle = subtitle,
        .info = info,
        .info_count = 4,
        .credits = credits,
        .credit_count = (e->description[0] != '\0') ? 1 : 0,
    };

    /* Load preview image — from disk if installed, download if not */
    SDL_Texture *preview_tex = NULL;
    int preview_w = 0, preview_h = 0;
    {
        char preview_path[MAX_PATH_BUF];
        snprintf(preview_path, sizeof(preview_path), "%s/%s/preview.png",
                 THEMES_DIR, e->id);

        /* If not on disk, try downloading to cache */
        if (access(preview_path, F_OK) != 0 && e->preview_url[0] != '\0') {
            char cache_dir[MAX_PATH_BUF];
            snprintf(cache_dir, sizeof(cache_dir), DATA_DIR "/previews");
            char cmd[MAX_PATH_BUF * 3];
            snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", cache_dir);
            system(cmd);

            snprintf(preview_path, sizeof(preview_path),
                     DATA_DIR "/previews/%s.png", e->id);
            if (access(preview_path, F_OK) != 0) {
                snprintf(cmd, sizeof(cmd),
                         "curl -sfLk -o \"%s\" -m 10 \"%s\"",
                         preview_path, e->preview_url);
                system(cmd);
            }
        }

        if (access(preview_path, F_OK) == 0) {
            preview_tex = ap_load_image(preview_path);
            if (preview_tex) {
                SDL_QueryTexture(preview_tex, NULL, NULL, &preview_w, &preview_h);
            }
        }
    }

    /* Custom loop — same as pakkit_detail_screen but with A/X button actions */
    pakkit_scroll_state scroll = {0};
    detail_result_t result = DETAIL_BACK;

    bool running = true;
    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed && !ev.repeated) {
                if (ev.button == AP_BTN_B) { result = DETAIL_BACK; running = false; }
                if (ev.button == AP_BTN_A) { result = DETAIL_ACTION; running = false; }
                if (ev.button == AP_BTN_X && e->installed) { result = DETAIL_DELETE; running = false; }
            }
            if (ev.pressed) {
                if (ev.button == AP_BTN_UP)
                    pakkit_scroll_handle_input(&scroll, -1, PAKKIT_SCROLL_STEP);
                if (ev.button == AP_BTN_DOWN)
                    pakkit_scroll_handle_input(&scroll,  1, PAKKIT_SCROLL_STEP);
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

        /* Title */
        ap_draw_text(font_large, opts.title, pad * 3, y, text_color);
        y += TTF_FontHeight(font_large) + pad;

        /* Subtitle (author) */
        ap_draw_text(font_small, opts.subtitle, pad * 3, y, hint_color);
        y += TTF_FontHeight(font_small) + pad * 3;

        ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
        y += pad * 3;

        /* Info pairs */
        int label_x = pad * 3;
        int value_x = pad * 3 + AP_DS(80);
        int row_h = TTF_FontHeight(font_small) + pad;

        for (int i = 0; i < opts.info_count; i++) {
            ap_draw_text(font_small, opts.info[i].key, label_x, y, hint_color);
            ap_draw_text(font_small, opts.info[i].value, value_x, y, text_color);
            y += row_h;
        }

        y += pad * 2;
        ap_draw_rect(pad * 3, y, sw - pad * 6, 1, hint_color);
        y += pad * 3;

        /* Description */
        if (opts.credit_count > 0) {
            ap_draw_text(font_small, opts.credits[0], pad * 3, y, text_color);
            y += row_h;
        }

        /* Preview image */
        if (preview_tex) {
            y += pad * 2;
            int max_w = sw - pad * 6;
            float scale = (float)max_w / (float)preview_w;
            if (scale > 1.0f) scale = 1.0f;
            int draw_w = (int)(preview_w * scale);
            int draw_h = (int)(preview_h * scale);
            int draw_x = (sw - draw_w) / 2;
            ap_draw_image(preview_tex, draw_x, y, draw_w, draw_h);
            y += draw_h + pad * 2;
        }

        int total_content = y + scroll.scroll_y - content_top;
        SDL_RenderSetClipRect(ap__g.renderer, NULL);
        pakkit_scroll_update(&scroll, total_content, content_h);

        /* Hints */
        const char *action_label = e->installed ? "APPLY" : "DOWNLOAD";
        if (e->installed) {
            pakkit_hint hints[] = {
                {"B", "BACK"},
                {"X", "DELETE"},
                {"A", action_label},
            };
            pakkit_draw_hints(hints, 3);
        } else {
            pakkit_hint hints[] = {
                {"B", "BACK"},
                {"A", action_label},
            };
            pakkit_draw_hints(hints, 2);
        }

        ap_present();
    }

    if (preview_tex) SDL_DestroyTexture(preview_tex);
    return result;
}

/* -----------------------------------------------------------------------
 * Delete installed theme
 * ----------------------------------------------------------------------- */

static void delete_theme(catalog_entry *entry) {
    char cmd[MAX_PATH_BUF * 2];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s/%s\"", THEMES_DIR, entry->id);
    system(cmd);
    entry->installed = false;
}

/* -----------------------------------------------------------------------
 * Preview helper — returns path to preview image, downloading if needed
 * ----------------------------------------------------------------------- */

static bool resolve_preview_path(catalog_entry *e, char *out, int out_size) {
    /* Check installed location first */
    snprintf(out, out_size, "%s/%s/preview.png", THEMES_DIR, e->id);
    if (access(out, F_OK) == 0) return true;

    /* Check/download to cache */
    snprintf(out, out_size, DATA_DIR "/previews/%s.png", e->id);
    if (access(out, F_OK) == 0) return true;

    if (e->preview_url[0] != '\0') {
        system("mkdir -p \"" DATA_DIR "/previews\"");
        char cmd[MAX_PATH_BUF * 3];
        snprintf(cmd, sizeof(cmd),
                 "curl -sfLk -o \"%s\" -m 10 \"%s\"",
                 out, e->preview_url);
        if (system(cmd) == 0 && access(out, F_OK) == 0) return true;
    }

    out[0] = '\0';
    return false;
}

/* -----------------------------------------------------------------------
 * UI flows — each returns when the user backs out
 * ----------------------------------------------------------------------- */

static void show_entry_list(const char *title, filtered_list *list) {
    if (list->count == 0) {
        pakkit_message("Nothing here yet", "OK");
        return;
    }

    int cursor = 0;
    int scroll = 0;
    int last_preview = -1;
    SDL_Texture *preview_tex = NULL;
    int preview_w = 0, preview_h = 0;

    bool running = true;
    int selected = -1;

reenter:
    running = true;
    selected = -1;

    while (running) {
        ap_input_event ev;
        while (ap_poll_input(&ev)) {
            if (ev.pressed) {
                switch (ev.button) {
                    case AP_BTN_B:
                        if (!ev.repeated) running = false;
                        break;
                    case AP_BTN_A:
                        if (!ev.repeated) { selected = cursor; running = false; }
                        break;
                    case AP_BTN_UP:
                        cursor--;
                        if (cursor < 0) cursor = list->count - 1;
                        break;
                    case AP_BTN_DOWN:
                        cursor++;
                        if (cursor >= list->count) cursor = 0;
                        break;
                    default:
                        break;
                }
            }
        }

        /* Load preview when cursor changes */
        if (cursor != last_preview) {
            if (preview_tex) { SDL_DestroyTexture(preview_tex); preview_tex = NULL; }
            char preview_path[MAX_PATH_BUF];
            if (resolve_preview_path(list->entries[cursor], preview_path, sizeof(preview_path))) {
                preview_tex = ap_load_image(preview_path);
                if (preview_tex) {
                    SDL_QueryTexture(preview_tex, NULL, NULL, &preview_w, &preview_h);
                }
            }
            last_preview = cursor;
        }

        int sw = ap_get_screen_width();
        int sh = ap_get_screen_height();
        int pad = AP_DS(5);

        /* Layout: preview full screen, list overlaid on left */
        int list_area_w = preview_tex ? (sw * 2 / 5) : sw;

        TTF_Font *font_med   = ap_get_font(AP_FONT_MEDIUM);
        TTF_Font *font_small = ap_get_font(AP_FONT_SMALL);
        TTF_Font *font_tiny  = ap_get_font(AP_FONT_TINY);

        int item_h = TTF_FontHeight(font_small) + pad * 3;
        int title_h = TTF_FontHeight(font_med) + pad * 3 + 1 + pad * 3;
        int hint_h = TTF_FontHeight(font_tiny) + pad * 2;
        int list_h = sh - title_h - hint_h - pad;
        int visible = list_h / item_h;
        if (visible < 1) visible = 1;

        if (cursor < scroll) scroll = cursor;
        if (cursor >= scroll + visible) scroll = cursor - visible + 1;

        ap_clear_screen();
        ap_draw_background();

        /* Preview as full-screen background */
        if (preview_tex) {
            float scale_w = (float)sw / (float)preview_w;
            float scale_h = (float)sh / (float)preview_h;
            float scale = (scale_w > scale_h) ? scale_w : scale_h;
            int draw_w = (int)(preview_w * scale);
            int draw_h = (int)(preview_h * scale);
            int draw_x = (sw - draw_w) / 2;
            int draw_y = (sh - draw_h) / 2;
            ap_draw_image(preview_tex, draw_x, draw_y, draw_w, draw_h);

        }

        ap_theme *theme = ap_get_theme();
        ap_color text_color = theme->text;
        ap_color hint_color = theme->hint;
        ap_color highlight  = theme->highlight;
        ap_color hl_text    = theme->highlighted_text;

        int y = pad * 3;
        ap_draw_text(font_med, title, pad * 3, y, hint_color);
        y += TTF_FontHeight(font_med) + pad * 3;

        int list_top = y;

        int max_text_w = list_area_w - pad * 8;

        SDL_Rect clip = { 0, list_top, list_area_w, list_h };
        SDL_RenderSetClipRect(ap__g.renderer, &clip);

        for (int i = scroll; i < list->count && i < scroll + visible; i++) {
            int item_y = list_top + (i - scroll) * item_h;
            int text_y = item_y + (item_h - TTF_FontHeight(font_small)) / 2;

            if (i == cursor) {
                int tw = ap_measure_text_ellipsized(font_small, list->entries[i]->name, max_text_w);
                int pill_w = tw + pad * 4;
                ap_draw_pill(pad * 2, item_y, pill_w, item_h, highlight);
                ap_draw_text_ellipsized(font_small, list->entries[i]->name,
                                        pad * 4, text_y, hl_text, max_text_w);
            } else {
                ap_draw_text_ellipsized(font_small, list->entries[i]->name,
                                        pad * 4, text_y, text_color, max_text_w);
            }
        }

        SDL_RenderSetClipRect(ap__g.renderer, NULL);

        /* Scrollbar */
        if (list->count > visible) {
            int bar_x = list_area_w - pad * 2;
            int thumb_h = (visible * list_h) / list->count;
            if (thumb_h < pad * 2) thumb_h = pad * 2;
            int thumb_y = list_top + (scroll * (list_h - thumb_h)) / (list->count - visible);
            ap_color bar_color = { hint_color.r, hint_color.g, hint_color.b, 80 };
            ap_color thumb_color = { hint_color.r, hint_color.g, hint_color.b, 160 };
            ap_draw_rect(bar_x, list_top, 3, list_h, bar_color);
            ap_draw_rect(bar_x, thumb_y, 3, thumb_h, thumb_color);
        }

        pakkit_hint hints[] = {
            {"B", "BACK"},
            {"A", "SELECT"},
        };
        pakkit_draw_hints(hints, 2);

        ap_present();
    }

    if (preview_tex) { SDL_DestroyTexture(preview_tex); preview_tex = NULL; }

    if (selected < 0) return;

    catalog_entry *e = list->entries[selected];

    /* Show detail screen — A=action, B=back, X=delete */
    detail_result_t dr = show_entry_detail(e);

    if (dr == DETAIL_BACK) {
        cursor = selected;
        last_preview = -1;
        goto reenter;
    }

    if (dr == DETAIL_DELETE) {
        if (pakkit_confirm("Delete this theme?", "DELETE", "CANCEL")) {
            delete_theme(e);
            pakkit_message("Deleted.", "OK");
        }
    } else if (e->installed) {
        pakkit_loading("Backing up & applying...");
        backup_current();
        apply_theme(e);
        pakkit_message("Theme applied!", "OK");
    } else {
        pakkit_loading("Downloading...");
        if (download_theme(e)) {
            pakkit_message("Downloaded!", "OK");
            if (pakkit_confirm("Apply now?", "APPLY", "LATER")) {
                pakkit_loading("Backing up & applying...");
                backup_current();
                apply_theme(e);
                pakkit_message("Theme applied!", "OK");
            }
        } else {
            pakkit_message("Download failed.\nCheck your connection.", "OK");
        }
    }

    /* Return to list after action */
    cursor = selected;
    last_preview = -1;
    goto reenter;
}

static void show_installed_list(const char *title, filtered_list *list) {
    if (list->count == 0) {
        pakkit_message("Nothing here yet", "OK");
        return;
    }

    int cursor = 0;
    for (;;) {
        pakkit_list_item items[MAX_ENTRIES];
        for (int i = 0; i < list->count; i++) {
            items[i].label = list->entries[i]->name;
        }

        pakkit_hint hints[] = {
            {"B", "BACK"},
            {"X", "DELETE"},
            {"A", "APPLY"},
        };

        pakkit_list_opts opts = {0};
        opts.title = title;
        opts.hints = hints;
        opts.hint_count = 3;
        opts.initial_index = cursor;
        opts.text_width_pills = true;
        opts.secondary_button = AP_BTN_X;

        pakkit_list_result result;
        pakkit_list(&opts, items, list->count, &result);

        if (result.action == PAKKIT_ACTION_BACK)
            return;

        cursor = result.selected_index;
        catalog_entry *e = list->entries[cursor];

        if (result.action == PAKKIT_ACTION_SECONDARY) {
            /* Delete */
            if (pakkit_confirm("Delete this theme?", "DELETE", "CANCEL")) {
                delete_theme(e);
                pakkit_message("Deleted.", "OK");
                /* Refresh the list */
                list->entries[cursor] = list->entries[list->count - 1];
                list->count--;
                if (list->count == 0) return;
                if (cursor >= list->count) cursor = list->count - 1;
            }
        } else {
            /* Apply */
            pakkit_loading("Backing up & applying...");
            backup_current();
            apply_theme(e);
            pakkit_message("Theme applied!", "OK");
        }
    }
}

static void show_browse_installed(const char *category_name, category_t cat) {
    pakkit_menu_result result = {0};
    for (;;) {
        pakkit_menu_item items[] = {
            {"Browse"},
            {"Installed"},
        };

        char title[128];
        snprintf(title, sizeof(title), "TheyTheMeRollin - %s", category_name);

        pakkit_menu(title, items, 2, &result);

        if (result.selected_index < 0)
            return;

        int cursor = result.selected_index;

        if (cursor == 0) {
            /* Browse — show all catalog entries for this category */
            filtered_list list = filter_catalog(cat, false);
            char browse_title[128];
            snprintf(browse_title, sizeof(browse_title), "%s - Browse", category_name);
            show_entry_list(browse_title, &list);
        } else {
            /* Installed — direct apply/delete, no detail screen */
            scan_installed();
            filtered_list list = filter_catalog(cat, true);
            char installed_title[128];
            snprintf(installed_title, sizeof(installed_title), "%s - Installed", category_name);
            show_installed_list(installed_title, &list);
        }
    }
}

/* -----------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------- */

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    /* Resolve pak directory */
    const char *env_pak = getenv("THEYTHEMEROLLIN_PAK_DIR");
    if (env_pak) {
        snprintf(pak_dir, sizeof(pak_dir), "%s", env_pak);
    } else {
        snprintf(pak_dir, sizeof(pak_dir), ".");
    }

    /* Ensure data directories exist */
    system("mkdir -p \"" THEMES_DIR "\"");
    system("mkdir -p \"" BACKUP_DIR "\"");

    /* Init display */
    ap_config cfg = {
        .window_title = "TheyTheMeRollin",
        .is_nextui = true,
        .disable_background = true,
    };
    if (ap_init(&cfg) != AP_OK) {
        return 1;
    }

    /* Set app background color */
    ap_get_theme()->background = (ap_color){30, 30, 35, 255};

    /* Discover systems on device */
    discover_systems();

    /* Fetch and parse theme catalog */
    pakkit_loading("Fetching catalog...");
    if (!fetch_catalog_json()) {
        /* Network failed — try cached copy */
        if (!parse_catalog()) {
            pakkit_message("Could not load catalog.\nCheck your internet connection.", "OK");
        }
    } else {
        if (!parse_catalog()) {
            pakkit_message("Catalog data is invalid.", "OK");
        }
    }
    scan_installed();

    /* Main menu loop */
    pakkit_menu_result result = {0};
    for (;;) {
        pakkit_menu_item items[] = {
            {"Themes"},
            {"Wallpapers"},
            {"Icons"},
            {"Restore Backup"},
        };

        pakkit_menu("TheyTheMeRollin", items, 4, &result);

        if (result.selected_index < 0)
            break;

        if (result.selected_index < 3) {
            const char *names[] = {"Themes", "Wallpapers", "Icons"};
            category_t cats[] = {CATEGORY_THEMES, CATEGORY_WALLPAPERS, CATEGORY_ICONS};
            show_browse_installed(names[result.selected_index], cats[result.selected_index]);
        } else {
            /* Restore backup */
            if (!has_backup()) {
                pakkit_message("No backup found.", "OK");
            } else if (pakkit_confirm("Restore previous theme?", "RESTORE", "CANCEL")) {
                pakkit_loading("Restoring...");
                restore_backup();
                pakkit_message("Backup restored!", "OK");
            }
        }
    }

    ap_quit();
    return 0;
}
