#!/usr/bin/env python3
"""Generate DSView's themed line icons (24x24, 1.5px stroke, round caps).

Usage: genicons.py <icons dir>   (writes <dir>/dark/*.svg and <dir>/light/*.svg)
"""
import os, sys

THEMES = {"dark": "#cccccc", "light": "#3b3b3b"}

# Colour accents that stay the same in both themes (adjusted per theme below).
ACCENT = {
    "dark":  {"green": "#3fb950", "red": "#f14c4c", "blue": "#3794ff"},
    "light": {"green": "#2ea043", "red": "#d1242f", "blue": "#0066cc"},
}

# Inner markup. "S" = stroked group with the theme colour; plain elements use
# fill="C" where they must be filled. {green}/{red}/{blue} are accents.
ICONS = {
    # ---- app / menus ----
    "about":   '<circle cx="12" cy="12" r="8.5"/><path d="M12 11v5.5"/><circle cx="12" cy="7.8" r="0.4" fill="C"/>',
    "help":    '<circle cx="12" cy="12" r="8.5"/><path d="M9.6 9.6a2.4 2.4 0 1 1 3.3 2.2c-.6.3-.9.8-.9 1.4v.6"/><circle cx="12" cy="16.6" r="0.4" fill="C"/>',
    "bug":     '<rect x="7.5" y="8" width="9" height="11.5" rx="4.5"/><path d="M9.5 8.3V7a2.5 2.5 0 0 1 5 0v1.3M12 11v8.3M7.5 12.5H4.5M16.5 12.5h3M7.8 9.5 5.5 7.5M16.2 9.5l2.3-2M7.8 16.5 5.5 18.5M16.2 16.5l2.3 2"/>',
    "manual":  '<path d="M12 6.5C10.3 5.2 7.7 4.8 4.5 5v13c3.2-.2 5.8.2 7.5 1.5 1.7-1.3 4.3-1.7 7.5-1.5V5c-3.2-.2-5.8.2-7.5 1.5zM12 6.5v13"/>',
    "log":     '<path d="M6 3.5h8l4 4v13H6z"/><path d="M14 3.5v4h4M9 12h6M9 15h6M9 18h4"/>',
    "update":  '<path d="M19.5 12a7.5 7.5 0 1 1-2.2-5.3M19.5 4.5v3.7h-3.7"/>',
    "folder":  '<path d="M3.5 6.5a1.5 1.5 0 0 1 1.5-1.5h4.4l2 2H19a1.5 1.5 0 0 1 1.5 1.5v9a1.5 1.5 0 0 1-1.5 1.5H5a1.5 1.5 0 0 1-1.5-1.5z"/>',
    "open":    '<path d="M3.5 17.5v-11A1.5 1.5 0 0 1 5 5h4.4l2 2H17a1.5 1.5 0 0 1 1.5 1.5V10"/><path d="M3.5 17.5 6.2 11h14.3l-2.7 6.5a1.5 1.5 0 0 1-1.4 1H5a1.5 1.5 0 0 1-1.5-1z"/>',
    "file":    '<path d="M6 3.5h8l4 4v13H6z"/><path d="M14 3.5v4h4"/>',
    "save":    '<path d="M4.5 6A1.5 1.5 0 0 1 6 4.5h10.5l3 3V18a1.5 1.5 0 0 1-1.5 1.5H6A1.5 1.5 0 0 1 4.5 18z"/><path d="M8 4.5v4h7v-4M7.5 19.5v-6h9v6"/>',
    "export":  '<path d="M11 4.5H6A1.5 1.5 0 0 0 4.5 6v12A1.5 1.5 0 0 0 6 19.5h12a1.5 1.5 0 0 0 1.5-1.5v-5M14 4.5h5.5V10M19.5 4.5 11 13"/>',
    "capture": '<rect x="3.5" y="6.5" width="17" height="12.5" rx="2"/><path d="M8.5 6.5 10 4.5h4l1.5 2"/><circle cx="12" cy="12.7" r="3.2"/>',
    "gear":    '<circle cx="12" cy="12" r="2.8"/><path d="M10.3 3.5h3.4l.5 2.3 1.6.9 2.2-.8 1.7 2.9-1.7 1.6v1.9l1.7 1.6-1.7 2.9-2.2-.8-1.6.9-.5 2.3h-3.4l-.5-2.3-1.6-.9-2.2.8-1.7-2.9 1.7-1.6v-1.9L4.3 8.8 6 5.9l2.2.8 1.6-.9z"/>',
    "display": '<rect x="3.5" y="4.5" width="17" height="12" rx="1.5"/><path d="M9 20h6M12 16.5V20"/>',
    "dark":    '<path d="M19.5 14.5A8 8 0 0 1 9.5 4.5a8 8 0 1 0 10 10z"/>',
    "light":   '<circle cx="12" cy="12" r="3.8"/><path d="M12 3v2M12 19v2M3 12h2M19 12h2M5.6 5.6 7 7M17 17l1.4 1.4M5.6 18.4 7 17M17 7l1.4-1.4"/>',

    # ---- capture ----
    "params":  '<path d="M4.5 7h9M17.5 7h2M4.5 12h3M11.5 12h8M4.5 17h7M15.5 17h4"/><circle cx="15.5" cy="7" r="2"/><circle cx="9.5" cy="12" r="2"/><circle cx="13.5" cy="17" r="2"/>',
    "once":    '<path d="M4 12h12M12 8l4 4-4 4M19.5 6v12"/>',
    "repeat":  '<path d="M4.5 11V9.5A2.5 2.5 0 0 1 7 7h12M16 4l3 3-3 3M19.5 13v1.5A2.5 2.5 0 0 1 17 17H5M8 20l-3-3 3-3"/>',
    "loop":    '<path d="M12 12c-1.8-2.4-3.3-3.5-5-3.5a3.5 3.5 0 0 0 0 7c1.7 0 3.2-1.1 5-3.5s3.3-3.5 5-3.5a3.5 3.5 0 0 1 0 7c-1.7 0-3.2-1.1-5-3.5z"/>',

    # ---- panels / activity bar ----
    "trigger": '<path d="M3.5 18.5h7v-13h10M7.8 13.2l2.7-2.7 2.7 2.7"/>',
    "protocol":'<path d="M2.5 12 5 7.5h4.5L12 12l-2.5 4.5H5zM12 12l2.5-4.5H19l2.5 4.5-2.5 4.5h-4.5z"/>',
    "measure": '<path d="M3.5 16 16 3.5l4.5 4.5L8 20.5z"/><path d="M7 12.5l2 2M9.5 10l1.3 1.3M12 7.5l2 2M14.5 5l1.3 1.3"/>',
    "search":  '<circle cx="10.5" cy="10.5" r="6"/><path d="M15 15l5 5"/>',
    "search-bar": '<circle cx="10" cy="9.5" r="5"/><path d="M13.7 13.2 17 16.5M4 20h16"/>',
    "nav":     '<circle cx="12" cy="12" r="7.5"/><circle cx="12" cy="12" r="2.5"/><path d="M12 2.5v3M12 18.5v3M2.5 12h3M18.5 12h3"/>',
    "pre":     '<path d="M14.5 6 8.5 12l6 6"/>',
    "next":    '<path d="M9.5 6l6 6-6 6"/>',
    "add":     '<path d="M12 5v14M5 12h14"/>',
    "del":     '<path d="M4.5 7h15M9.5 7V5h5v2M6.5 7l1 12.5h9l1-12.5M10.2 10.5v6M13.8 10.5v6"/>',
    "shown":   '<path d="M2.5 12S6 5.5 12 5.5 21.5 12 21.5 12 18 18.5 12 18.5 2.5 12 2.5 12z"/><circle cx="12" cy="12" r="3"/>',
    "hidden":  '<path d="M9.9 6.7A9 9 0 0 1 12 6.5c6 0 9.5 5.5 9.5 5.5a15 15 0 0 1-2.5 3.1M14.1 14.1a3 3 0 0 1-4.2-4.2M6.6 7.6C4 9.2 2.5 12 2.5 12s3.5 6.5 9.5 6.5c1.8 0 3.3-.5 4.6-1.2M3.5 3.5l17 17"/>',

    # ---- oscilloscope functions ----
    "function": '<path d="M14.5 4.5c-2.5 0-3 1.5-3.3 3.5L9.8 16c-.3 2-.8 3.5-3.3 3.5M7.5 10h7.5"/><path d="M15.5 13.5l4 5M19.5 13.5l-4 5"/>',
    "fft":     '<path d="M3.5 20.5h17M5.5 18v-3M8.5 18V9M11.5 18V5M14.5 18v-6M17.5 18v-3.5"/>',
    "math":    '<rect x="3.5" y="3.5" width="17" height="17" rx="2"/><path d="M12 3.5v17M3.5 12h17M6 7.7h3.5M7.75 6v3.5M14.5 7.7h3.5M6.4 14.7l2.5 2.5M8.9 14.7l-2.5 2.5M14.5 15h3.5M14.5 17.2h3.5"/>',
    "lissajous": '<path d="M12 3.5c4 0 7 3.8 7 8.5s-3 8.5-7 8.5-7-3.8-7-8.5 3-8.5 7-8.5zM5.3 7.5c2.8 1.3 10.6 7.7 13.4 9M5.3 16.5c2.8-1.3 10.6-7.7 13.4-9"/>',

    # ---- device type ----
    "usb2":    '<path d="M12 3.5v14.5M9.5 6 12 3.5 14.5 6M12 13 7.5 10.5V8.5M12 15.5l4.5-2.5V11"/><circle cx="12" cy="19.2" r="1.3"/><circle cx="7.5" cy="7.3" r="1.2"/><rect x="15.5" y="9" width="2" height="2"/>',
    "demo":    '<rect x="4" y="5" width="16" height="11" rx="1.5"/><path d="M10.5 8.5v4l3.5-2z"/><path d="M8 19.5h8"/>',
    "data":    '<ellipse cx="12" cy="6.5" rx="7" ry="2.5"/><path d="M5 6.5v11c0 1.4 3.1 2.5 7 2.5s7-1.1 7-2.5v-11M5 12c0 1.4 3.1 2.5 7 2.5s7-1.1 7-2.5"/>',
}

# usb3: usb2 with an accent dot, so SuperSpeed is recognisable.
ICONS["usb3"] = ICONS["usb2"] + '<circle cx="18.5" cy="18.5" r="2.5" fill="{blue}" stroke="none"/>'

# Filled, coloured (same shapes as before).
FILLED = {
    "start":  '<path d="M8 5.2v13.6a1 1 0 0 0 1.5.9l10.6-6.8a1 1 0 0 0 0-1.7L9.5 4.3A1 1 0 0 0 8 5.2z" fill="{green}"/>',
    "stop":   '<rect x="5.5" y="5.5" width="13" height="13" rx="2.5" fill="{red}"/>',
    "single": '<path d="M13.5 2.5 5.5 13.5h6l-1 8 8-11h-6z" fill="{blue}" stroke="{blue}" stroke-width="1" stroke-linejoin="round"/>',
}

# Device modes: wide (2:1) for the mode button, and square for the menu.
WAVES = {
    "la":  'M2 11h4V3h5v8h5V3h5v8h4',
    "daq": 'M2 9c2-4 4-5 6-2.5s3 5 5 3.5 2-6 5-6.5 3 3 4 5 2 1 3 .5',
    "osc": 'M2 7c2.5-5.5 5-5.5 7.5 0s5 5.5 7.5 0 5-5.5 7.5 0',
}
SQUARE_WAVES = {
    "la":  'M6 15.5h2.5v-7h3.5v7h3.5v-7h2.5',
    "daq": 'M6 13c1.2-3 2.4-3.5 3.5-1.8s2 3.5 3 2.3 1.2-4.2 3-4.6 1.5 2 2.5 2.6',
    "osc": 'M5.5 12c1.6-4.2 3.3-4.2 4.9 0s3.3 4.2 4.9 0 2.4-3 3.2-2',
}

HEAD = '<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}">'
STROKE = '<g fill="none" stroke="{c}" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round">'


def write(path, text):
    with open(path, "w") as f:
        f.write(text + "\n")


def main(root):
    for theme, col in THEMES.items():
        acc = ACCENT[theme]
        out = os.path.join(root, theme)
        for name, body in ICONS.items():
            body = body.replace('fill="C"', 'fill="%s" stroke="none"' % col).format(**acc)
            write(os.path.join(out, name + ".svg"),
                  HEAD.format(w=24, h=24) + STROKE.format(c=col) + body + "</g></svg>")
        for name, body in FILLED.items():
            write(os.path.join(out, name + ".svg"),
                  HEAD.format(w=24, h=24) + body.format(**acc) + "</svg>")
        for name, d in WAVES.items():
            write(os.path.join(out, name + ".svg"),
                  HEAD.format(w=28, h=14) + STROKE.format(c=col)
                  + '<path d="%s"/></g></svg>' % d)
        for name, d in SQUARE_WAVES.items():
            write(os.path.join(out, "square-" + name + ".svg"),
                  HEAD.format(w=24, h=24) + STROKE.format(c=col)
                  + '<rect x="3.5" y="4.5" width="17" height="15" rx="2.5"/>'
                  + '<path d="%s"/></g></svg>' % d)


if __name__ == "__main__":
    main(sys.argv[1])
