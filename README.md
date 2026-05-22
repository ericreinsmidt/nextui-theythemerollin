# TheyTheMeRollin

A theme manager for [NextUI](https://github.com/LoveRetro/NextUI) on TrimUI handhelds.

Browse, download, and apply community-made wallpaper and icon themes directly on your device. Mix and match individual wallpapers and icons per system for a fully custom setup.

## Supported Devices

- **tg5040** — TrimUI Brick, Smart Pro
- **tg5050** — TrimUI Smart Pro S

## Features

- **Browse** themes from a community catalog with live preview images
- **Download and install** themes with one button
- **Apply** full theme packs, wallpaper sets, or icon sets
- **Customize** — mix and match wallpapers and icons per system from any installed theme
- **Clear** individual wallpapers or icons to revert to defaults
- **Backup and restore** your current theme before making changes
- **Async preview loading** — previews download in the background and cache as you scroll
- **Delete** installed themes to free up space

## How It Works

TheyTheMeRollin fetches a theme catalog from GitHub when you launch it. You can browse available themes, see previews, download them, and apply them to your device. Each theme can include wallpapers (menu backgrounds and game list backgrounds) and icons (system icons on the main menu).

Before applying a theme, the app automatically backs up your current wallpapers and icons so you can restore them later.

## Menu Structure

- **Themes** — full packages with both wallpapers and icons
- **Wallpapers** — wallpaper packs (menu backgrounds)
- **Icons** — icon packs (system icons)
- **Customize** — per-system mix and match from installed themes
- **Restore Backup** — revert to your original theme

## Installation

### Manual Installation

1. Download the latest release for your device from the [Releases](https://github.com/ericreinsmidt/nextui-theythemerollin/releases) page
2. Unzip and copy the `TheyTheMeRollin.pak` folder to:
   - Brick / Smart Pro: `SD_ROOT/Tools/tg5040/`
   - Smart Pro S: `SD_ROOT/Tools/tg5050/`
3. Launch from the Tools menu

## Creating Themes

Want to make a theme? Check the [Theme Catalog](https://github.com/ericreinsmidt/nextui-theme-catalog) for submission instructions and a full guide on theme structure.

## Building from Source

Requires the NextUI Docker toolchain for your target platform:

```bash
# Brick / Smart Pro (tg5040)
cd ports/tg5040
docker run --rm -v "$(cd ../.. && pwd)":/build -w /build/ports/tg5040 \
  ghcr.io/loveretro/tg5040-toolchain make clean all

# Smart Pro S (tg5050)
cd ports/tg5050
docker run --rm -v "$(cd ../.. && pwd)":/build -w /build/ports/tg5050 \
  ghcr.io/loveretro/tg5050-toolchain make clean all
```

Package for distribution:

```bash
cd ports/tg5040 && zip -r ../../dist/TheyTheMeRollin.tg5040.pak.zip pak/
cd ports/tg5050 && zip -r ../../dist/TheyTheMeRollin.tg5050.pak.zip pak/
```

## Credits

Built with [apostrophe](https://github.com/LoveRetro/apostrophe) and [pakkit](https://github.com/LoveRetro/pakkit).

## License

MIT
