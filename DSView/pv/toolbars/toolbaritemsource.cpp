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

#include "toolbaritemsource.h"

#include <QWidgetAction>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

#include "../ui/fn.h"

namespace pv {
namespace toolbars {

ToolbarItemSource::ToolbarItemSource(const QString &title, QWidget *parent)
    : QToolBar(title, parent)
{
    setMovable(false);
    setFloatable(false);
}

QAction* ToolbarItemSource::add_item(const QString &id, QWidget *widget)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);

    ToolbarItem item;
    item.id = id;
    item.widget = widget;
    item.action = action;
    _items.append(item);

    return action;
}

void ToolbarItemSource::apply_font(const QFont &font)
{
    // Menus and actions still belong to this toolbar.
    ui::set_toolbar_font(this, font);

    // The item widgets live in TopBar's groups now.
    for (const ToolbarItem &item : _items)
    {
        item.widget->setFont(font);

        for (auto *o : item.widget->findChildren<QToolButton*>())
            o->setFont(font);
        for (auto *o : item.widget->findChildren<QPushButton*>())
            o->setFont(font);
        for (auto *o : item.widget->findChildren<QComboBox*>())
            o->setFont(font);
        for (auto *o : item.widget->findChildren<QLabel*>())
            o->setFont(font);
    }
}

} // namespace toolbars
} // namespace pv
