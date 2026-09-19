labline(1)

# NAME

labline - a dead-simple status panel for labwc

# SYNOPSYS

*labline* [options...]

# DESCRIPTION

labline is a status panel for labwc, similar to that of dwm. Works well
with other wlroots compositors that have static workspaces.

That being said, an effort was made not to stray from labwc in terms of
supported protocols and core design, so 100% compatibility with other
compositors is not guaranteed.

# OPTIONS

*-h, --help*
	Print the help message and exit

*-a, --anchor* <top/bottom>
	Anchor the bar to the top/bottom of the screen

*-f, --font* <fontname>
	Use the specified font

*--pbg* <#hex_value>
	Define background for the accented (primary) sections of the panel

*--pfg* <#hex_value>
	Define foreground for the accented (primary) sections of the panel

*--sbg* <#hex_value>
	Define background for the non-accented (secondary) sections of the panel

*--sfg* <#hex_value>
	Define foreground for the non-accented (secondary) sections of the panel

# SEE ALSO

labwc(1), labwc-config(1)