/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 * 
 * Copyright (C) 2016 DreamSourceLab <support@dreamsourcelab.com>
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

#ifndef MYSTYLE_H
#define MYSTYLE_H

#include <QProxyStyle>
#include <QMenu>
#include <QAbstractButton>
#include <QComboBox>
#include <QTabBar>

class MyStyle : public QProxyStyle
{
    Q_OBJECT
    public:
    int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) {
        int s = QProxyStyle::pixelMetric(metric, option, widget);
        if (metric == QStyle::PM_SmallIconSize) {
            s = 24;
        }
        return s;
    }

    // No underlined mnemonic letters (Alt shortcuts still work).
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr, QStyleHintReturn *ret = nullptr) const override {
        if (hint == QStyle::SH_UnderlineShortcut)
            return 0;
        // OK / Cancel buttons: text only.
        if (hint == QStyle::SH_DialogButtonBox_ButtonsHaveIcons)
            return 0;
        return QProxyStyle::styleHint(hint, option, widget, ret);
    }

    // Menus get a transparent window, so the rounded corners of the
    // stylesheet (QMenu in theme.qss) are not painted over.
    void polish(QWidget *widget) override {
        if (qobject_cast<QMenu*>(widget) != nullptr && !widget->testAttribute(Qt::WA_TranslucentBackground)) {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setWindowFlag(Qt::FramelessWindowHint);
            widget->setWindowFlag(Qt::NoDropShadowWindowHint);
        }

        // Anything clickable shows the pointing hand.
        if (qobject_cast<QAbstractButton*>(widget) != nullptr
            || qobject_cast<QComboBox*>(widget) != nullptr
            || qobject_cast<QTabBar*>(widget) != nullptr) {
            widget->setCursor(Qt::PointingHandCursor);
        }

        QProxyStyle::polish(widget);
    }
    using QProxyStyle::polish;
};

#endif // MYSTYLE_H
