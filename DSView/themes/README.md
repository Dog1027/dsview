DSView themes
=============

The stylesheet started from BreezeStyleSheets (see LICENSE.md) and has been
reworked into one template plus a palette per theme.

| File            | What it is                                                    |
|-----------------|---------------------------------------------------------------|
| `theme.qss`     | The stylesheet template. Colors are `@name` tokens.          |
| `dark.palette`  | Values of the tokens for the dark theme.                      |
| `light.palette` | Values of the tokens for the light theme.                     |
| `dark/`, `light/` | Icons used by the stylesheet (`url(@icons/...)`).           |
| `breeze.qrc`    | Compiles all of the above into the executable.               |

Palette format: `@name: value;` — a value may refer to another entry
(`@divider: @hover;`). `/* comments */` are allowed. A token used in
`theme.qss` but missing from the palette is reported in the log.

Live editing
------------

Without live editing, a change needs `make` and a restart. To see changes
immediately, start DSView with the source directory of the themes:

```bash
DSVIEW_THEME_DIR=~/projects/dsview/DSView/themes ./build.dir/DSView
```

Every save of `theme.qss` or a palette is applied to the running program.
(The icons in `dark/` and `light/` are still read from the compiled
resources, so changes to them need a rebuild.)

Toolbar layout
--------------

Which buttons appear in the top toolbar and in the activity bar (the
vertical strip at the right edge that switches the side panels), their
order, grouping and whether they show text is defined in
`DSView/pv/toolbars/topbarlayout.h`. Their look is the "Main top bar" and
"Activity bar" sections at the end of `theme.qss`.

UI gallery
----------

All controls, the message boxes, a dialog, a menu, the palette colors and
every icon on one page, in the current theme:

```bash
./build.dir/DSView --gallery                 # open it next to the main window
./build.dir/DSView --gallery-shot out/       # save out/gallery-dark.png and
                                             # out/gallery-light.png, then quit
```

The Dark / Light buttons at the top switch the theme for the preview only
(closing the gallery restores the app's theme). With `DSVIEW_THEME_DIR`
the page is rebuilt on every save of a theme file. Compare two
`--gallery-shot` runs to see what a stylesheet change affected.
Code: `DSView/pv/dialogs/uigallery.cpp`.
