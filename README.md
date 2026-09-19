# labline

labline is a status panel for labwc, similar to that of dwm. Works well
with other wlroots compositors that have static workspaces.

That being said, an effort was made not to stray from labwc upstream
in terms of supported protocols and core design, so I don't guarantee
100% compatibility with other compositors. The main offender here is
probably the way labwc handles workspaces (stretching them across all
outputs, as opposed to, for example, per-output workspace groups in
Sway).

## Installation

### Compiling from source

To build labline, you would use the same build tools and dependencies as with
labwc.

Simply run:
```
meson setup build/
meson compile -C build/
```

Run-time dependencies include:
- wayland
- cairo, pango

Build dependencies include:
- meson, ninja, gcc/clang
- wayland-protocols
- scdoc *(optional: manpages)*

### From the AUR

I'm planning to publish a PKGBUILD on the AUR shortly after the first release.

## Usage

A simple example with piped input:
```sh
while sleep 1; do date; done | ./build/labline &
```

For configuration options, consult the manpage:
```sh
man ./build/docs/labline.1
```
