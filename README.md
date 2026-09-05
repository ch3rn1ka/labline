# labline

labline is a status panel for labwc, similar to that of dwm. Works well
with other wlroots compositors that have static workspaces.

That being said, an effort was made not to stray from labwc upstream
in terms of supported protocols and core design, so I don't guarantee
100% compatibility with other compositors. The main offender here is
probably the way labwc handles workspaces (stretching them across all
outputs), as opposed to, for example, per-output workspace groups in
Sway.

## Installation

### Compiling from source

To build, simply run:
```
meson setup build/
meson compile -C build/
```

Run-time dependencies include:
- wayland
- cairo, pango

Build dependencies include:
- meson, ninja, gcc/clang
- git *(optional: version control)*
- scdoc *(optional: manpages)*
- wayland-protocols

One-liner to install the dependencies on Arch:
```
pacman -S wayland wayland-protocols cairo pango meson gcc git scdoc
```

### From the AUR

I'm planning to publish a PKGBUILD on the AUR shortly after the first release.

## Command line options

* **-h**, **--help**</br>
  print the help message and exit
* **-a**, **--anchor** [top/bottom]</br>
  anchor the bar to the top/bottom (default is "bottom")
* **-f**, **--font** [fontname]</br>
  use the specified font (default is "Monospace 10")
* **--pbg**/**--pfg** [HEX]</br>
  define colors for the accented sections of the panel (primary colors)
* **--sbg**/**--sfg** [HEX]</br>
  define colors for the dim sections of the panel (secondary colors)

## Running

A simple example with piped input:
```sh
while sleep 1; do date; done | labline &
```
