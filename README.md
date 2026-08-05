# labline

Labline is a status panel for Labwc, similar to that of dwm or i3.
Compile it with `make`, pipe some text into it, and you're good to go!

## Command line options
* -a, --anchor [top/bottom]</br>
  anchor the bar to the top/bottom (default is "bottom")
* -f, --font [fontname]</br>
  use the specified font (default is "Monospace 10")
* --sbg/--sfg [HEX]</br>
  customize statusline bg/fg
* --tbg/--tfg [HEX]</br>
  customize active toplevel bg/fg
* --awsbg/--awsfg/--awsbr [HEX]</br>
  customize active workspace bg/fg/border
* --iwsbg/--iwsfg/--iwsbr [HEX]</br>
  customize inactive workspace bg/fg/border
* --uwsbg/--uwsfg/--uwsbr [HEX]</br>
  customize urgent workspace bg/fg/border
