# TheyTheMeRollin

A theme manager for [NextUI](https://github.com/LoveRetro/NextUI) on TrimUI handhelds (Brick, Smart Pro).

Browse, download, and apply community-made wallpaper and icon themes directly on your device.

## Features

- **Browse** themes from a community catalog with full-screen preview images
- **Download and install** themes with one button
- **Apply** wallpapers (per-system or universal) and system icons
- **Backup and restore** your current theme before making changes
- **Delete** installed themes to free up space

## How It Works

TheyTheMeRollin fetches a theme catalog from GitHub when you launch it. You can browse available themes, see previews, download them, and apply them to your device. Each theme can include wallpapers (menu backgrounds and game list backgrounds) and icons (system icons on the main menu).

Before applying a theme, the app automatically backs up your current wallpapers and icons so you can restore them later.

## Theme Categories

- **Themes** — full packages with both wallpapers and icons
- **Wallpapers** — wallpaper packs (menu backgrounds)
- **Icons** — icon packs (system icons)

## Installation

TheyTheMeRollin is available in the [Pak Store](https://github.com/LoveRetro/nextui-pak-store). Connect your device to Wi-Fi and install it from the Tools menu.

### Manual Installation

1. Download the latest release from the [Releases](https://github.com/ericreinsmidt/nextui-theythemerollin/releases) page
2. Unzip and copy the `TheyTheMeRollin.pak` folder to `SD_ROOT/Tools/tg5040`
3. Launch from the Tools menu

## Creating Themes

Want to make a theme? Check the [Theme Catalog](https://github.com/ericreinsmidt/nextui-theme-catalog) for submission instructions and a full guide on theme structure.

## Building from Source

Requires the NextUI tg5040 Docker toolchain:

```bash
# Build
cd ports/tg5040
docker run --rm -v "$(cd ../.. && pwd)":/build -w /build/ports/tg5040 \
  ghcr.io/loveretro/tg5040-toolchain make clean all

# Package
bash scripts/package_pak.sh
```

Output: `dist/TheyTheMeRollin.tg5040.pak.zip`

## Credits

Built with [apostrophe](https://github.com/LoveRetro/apostrophe) and [pakkit](https://github.com/LoveRetro/pakkit).

## License

MIT
