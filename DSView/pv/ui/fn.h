/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2023 DreamSourceLab <support@dreamsourcelab.com>
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

#ifndef _UI_FN_H
#define _UI_FN_H

#include <QFont>
#include <QSize>
#include <QLabel>

struct FontParam;

class QToolBar;
class QWidget;

namespace ui
{   

    enum AdjustSizeAction
    {
        ADJUST_WIDTH = 0,
        ADJUST_HEIGHT = 1,
        ADJUST_ALL = 2,
    };

    void set_font_param(QFont &font, struct FontParam &param);

    void set_toolbar_font(QToolBar *bar, QFont font);

    void set_form_font(QWidget *wid, QFont font);

    QSize measure_string(QFont font, QString str);

    void adjust_label_size(QLabel *ctrl, AdjustSizeAction action);

    // Data values (numbers, hex, decoded text) in the waveform area: the
    // system monospace font at the size of `base`, so digits keep their
    // place when the value changes and hex columns line up.
    QFont data_font(const QFont &base);

    // `base` with tabular (equal width) digits, where Qt supports it
    // (Qt 6.7+). For numbers among labels, e.g. the time ruler.
    QFont tabular_font(const QFont &base);

    // A form label from a language string that ends in ": " (or the
    // full-width colon): the text without it. Labels have no colon.
    QString label_text(const QString &s);

} // namespace ui

#endif