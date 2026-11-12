## [Pixel Reader](https://github.com/ealang/pixel-reader)

An ebook reader app for the Miyoo Mini. Supports epub and txt formats, including RTL Arabic book content (shaping, bidi, right alignment).

![Screenshot](resources/demo.gif)

## Miyoo Mini Installation

Supports Onion, MiniUI, and the default/factory OS.

1. [Download the latest release](https://github.com/ealang/pixel-reader/releases). Make sure to get the correct zip file for your OS. For Onion or default/factory OS: `pixel_reader_onion_xxx.zip`. For MiniUI: `pixel_reader_miniui_xxx.zip`. 
2. Extract the zip into the root of your SD card.
3. Boot your device, and the app should now show up in the apps/tools list.

The default location for book files is `Media/Books`.

### Arabic / RTL reading

- Settings → **Text direction**: `Auto` (default), `LTR`, or `RTL`
- **Auto** uses the EPUB `dc:language` tag when present, otherwise detects Arabic script in the text
- Bundled `ui_font_ar.ttf` is always used for Arabic/RTL content, regardless of the selected font

### Bookmarks

While reading:

- **Y** — add or remove a bookmark at the current position
- **START** — open the bookmarks list (same style as TOC)
- In the bookmarks list: **A** jump, **Y** delete, **B** / **START** close

Bookmarks are stored per book (up to 64) and do not affect scroll/render performance.

## Development Reference

### Desktop Build

Install dependencies (Ubuntu):
```
apt install make g++ libxml2-dev libzip-dev libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev
```

No extra libraries are required for RTL (Arabic joining + bidi are built into the reader).

Build:
```
make -j
```

Find app in `build/reader`.

Arabic sample text for manual checks: `src/sandbox/fixtures/arabic_sample.txt`.

### Miyoo Mini Cross-Compile

Cross-compile env is provided by [shauninman/union-miyoomini-toolchain](https://github.com/shauninman/union-miyoomini-toolchain). Docker is required.

Fetch git submodules:
```
git submodule init && git submodule update
```

Start shell:
```
make miyoo-mini-shell
```

Create app packages:
```
./cross-compile/miyoo-mini/create_packages.sh <version num>
```

### Run Tests

[Install gtest](https://github.com/google/googletest/blob/main/googletest/README.md).

```
make test
```
