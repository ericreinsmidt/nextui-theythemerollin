# PakKit

Reusable UI components for [NextUI](https://github.com/LoveRetro/NextUI) pak development on TrimUI handheld devices.

Header-only C library built on top of [Apostrophe](https://github.com/Helaas/Apostrophe). Drop it into your project and go.

## Components

### pakkit_menu

Selection menu with highlighted pill cursor.

    pakkit_menu_item items[] = {
        {.label = "Option 1" },
        {.label = "Option 2" },
        {.label = "Option 3" },
    };

    pakkit_menu_result result;
    int rc = pakkit_menu("My Menu", items, 3, &result);
    if (rc == AP_OK) {
        // result.selected_index has the selection
    }

- Title with divider
- Pill highlight on selected item
- B: Back, A: Select
- Returns AP_OK on select, AP_CANCELLED on back

### pakkit_list

Scrollable list with optional secondary/tertiary button actions.

    pakkit_hint hints[] = {
        {.button = "B",.label = "Back" },
        {.button = "X",.label = "Refresh" },
        {.button = "A",.label = "Open" },
    };

    pakkit_list_opts opts = {.title            = "My List",.hints            = hints,.hint_count       = 3,.secondary_button = AP_BTN_X,.tertiary_button  = AP_BTN_NONE,.initial_index    = 0,
    };

    pakkit_list_item items[] = {
        {.label = "Item 1" },
        {.label = "Item 2" },
    };

    pakkit_list_result result;
    pakkit_list(&opts, items, 2, &result);

    switch (result.action) {
        case PAKKIT_ACTION_SELECTED:  /* A pressed */ break;
        case PAKKIT_ACTION_BACK:      /* B pressed */ break;
        case PAKKIT_ACTION_SECONDARY: /* X pressed */ break;
        case PAKKIT_ACTION_TERTIARY:  /* Y pressed */ break;
    }

- Scroll indicator when items exceed visible area
- Ellipsized labels for long text
- Cursor position restore via initial_index

### pakkit_detail_screen

Scrollable info/about screen with key-value pairs and credits.

    pakkit_info_pair info[] = {
        {.key = "Version",.value = "1.0.0" },
        {.key = "Author",.value = "Your Name" },
    };

    const char *credits[] = {
        "My App by Your Name",
        "Built with PakKit + Apostrophe",
        "For NextUI by LoveRetro",
    };

    pakkit_detail_opts opts = {.title        = "My App",.subtitle     = "A cool NextUI pak",.info         = info,.info_count   = 2,.credits      = credits,.credit_count = 3,
    };
    pakkit_detail_screen(&opts);

- First credit line in text color, rest in hint color
- Scrollable with Up/Down
- B: Back

### pakkit_keyboard

On-screen QWERTY keyboard with optional shortcut buttons.

    // Plain text entry
    pakkit_keyboard_opts name_opts = {.prompt = "Enter a name" };
    pakkit_keyboard_result result;
    int rc = pakkit_keyboard("", &name_opts, &result);
    if (rc == AP_OK) {
        // result.text has the input
    }

    // With shortcut buttons
    const char *shortcuts[] = { ".com", ".org", "/rss", "/feed" };
    pakkit_keyboard_opts url_opts = {.prompt         = "Enter URL",.shortcuts      = shortcuts,.shortcut_count = 4,
    };
    pakkit_keyboard("https://", &url_opts, &result);

- QWERTY grid with Shift, Symbols, and Space
- Optional shortcut row for common strings
- Auto-lowercase after typing one uppercase letter
- L1/R1 shoulder buttons for quick Shift/Symbols toggle
- A: Type key, B: Backspace, Start: Confirm, Y: Cancel

### pakkit_message

Centered message dialog, dismissed with A or B.

    pakkit_message("Operation complete.", "OK");

### pakkit_confirm

Two-button confirmation dialog.

    if (pakkit_confirm("Delete this item?", "Delete", "Cancel")) {
        // User confirmed
    }

- B: Cancel (returns 0), A: Confirm (returns 1)

### pakkit_draw_hints

Minimal text hint bar drawn at the bottom of the screen.

    pakkit_hint hints[] = {
        {.button = "B",.label = "Back" },
        {.button = "A",.label = "Select" },
    };
    pakkit_draw_hints(hints, 2);

- Left-aligned hints, last hint right-aligned
- Uses theme hint color and tiny font

## Usage

PakKit is header-only. Add the include directory to your project and include with the implementation define in **one** C file:

    #define PAKKIT_UI_IMPLEMENTATION
    #include "pakkit_ui.h"

Requires Apostrophe to be initialized before calling any PakKit functions.

## Integration

Add PakKit as a git submodule:

    git submodule add https://github.com/ericreinsmidt/pakkit.git third_party/pakkit

Add to your Makefile CFLAGS:

    -I$(WORKSPACE)/third_party/pakkit/include

## Used By

- [NextFeed](https://github.com/ericreinsmidt/nextui-rss-reader) -- RSS/Atom reader
- [Nimbus](https://github.com/ericreinsmidt/nextui-nimbus) -- Weather app

## Dependencies

- [Apostrophe](https://github.com/Helaas/Apostrophe) UI toolkit (must be initialized first)
- SDL2, SDL2_ttf (provided by the NextUI toolchain)

## License

MIT -- see [LICENSE](LICENSE) for details.

## Credits

- **PakKit** by Eric Reinsmidt
- Built on [Apostrophe](https://github.com/Helaas/Apostrophe) by [Helaas](https://github.com/Helaas)
- For [NextUI](https://github.com/LoveRetro/NextUI) by [LoveRetro](https://github.com/LoveRetro)
