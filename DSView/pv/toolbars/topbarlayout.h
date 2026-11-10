/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef DSVIEW_PV_TOOLBARS_TOPBARLAYOUT_H
#define DSVIEW_PV_TOOLBARS_TOPBARLAYOUT_H

/*
 * ===================================================================
 *  Layout of the top toolbar. Edit this to rearrange the toolbar.
 * ===================================================================
 *
 *  [name]        start a group. Each group shrinks into its own » menu
 *                when the window is too narrow.
 *  [name fixed]  a group that never shrinks (always fully visible).
 *  id            place an item, shown as icon + text.
 *  id:icon       place an item, icon only (its text becomes the tooltip).
 *  |             separator line inside a group.
 *  ->            everything after this is pushed to the right edge.
 *  # ...         comment.
 *
 *  Available items (any item can go in any group; items left out are hidden):
 *
 *    device_type    device kind indicator (USB 2.0 / Demo / File)
 *    device         device selector
 *    options        device options dialog
 *    sample_count   capture duration / depth
 *    at             the "@" between duration and rate
 *    sample_rate    sample rate
 *    mode           single / repetitive / loop
 *    start          start / stop capture
 *    instant        instant capture
 *    trigger        trigger dock
 *    decode         protocol decoder dock
 *    measure        measurement dock
 *    search         search bar
 *    function       FFT / math (oscilloscope mode only)
 *    display        display options, theme
 *    file           file menu
 *    help           language, about, manual, log
 *
 *  Look of the groups/items: DSView/themes/theme.qss ("Main top bar").
 */
static const char *const TOP_BAR_LAYOUT = R"LAYOUT(

    [capture fixed]
        device_type:icon  device  options:icon
        |
        sample_count  at  sample_rate
        |
        mode:icon  start  instant

    [tools]
        trigger  decode  measure  search  function:icon
        |
        display:icon

    ->

    [app fixed]
        file:icon  help:icon

)LAYOUT";

#endif // DSVIEW_PV_TOOLBARS_TOPBARLAYOUT_H
