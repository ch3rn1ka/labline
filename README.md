# labline

labline is a status panel for labwc, similar to that of dwm or i3.
Compile it with `make`, pipe some text into it, and you're good to go.
Works well with other wlroots compositors that have static workspaces.

## Installation

### Compiling from source

Install dependencies:

* make *
* a C11 compiler *
* wayland
* wayland-protocols
* wlr-protocols
* pango
* cairo
* scdoc (optional: manpages) *
* git (optional: version info) *

*\* Compile-time dep*

Run these commands:
```
make
sudo make install
```

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
