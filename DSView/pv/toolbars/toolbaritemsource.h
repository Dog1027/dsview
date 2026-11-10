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

#ifndef DSVIEW_PV_TOOLBARS_TOOLBARITEMSOURCE_H
#define DSVIEW_PV_TOOLBARS_TOOLBARITEMSOURCE_H

#include <QToolBar>
#include <QString>
#include <QList>

class QWidgetAction;

namespace pv {
namespace toolbars {

struct ToolbarItem
{
    QString         id;       // name used in topbarlayout.h
    QWidget         *widget;
    QWidgetAction   *action;  // shows/hides the item wherever it is placed
};

/*
 * Base of the functional toolbars (SamplingBar, TrigBar, FileBar, LogoBar).
 *
 * A source toolbar owns its widgets and their logic, but does not decide
 * where they appear: it registers them with add_item(), and TopBar places
 * them according to TOP_BAR_LAYOUT (topbarlayout.h). The source toolbar
 * itself is never shown.
 */
class ToolbarItemSource : public QToolBar
{
public:
    ToolbarItemSource(const QString &title, QWidget *parent);

    const QList<ToolbarItem>& items() const { return _items; }

protected:
    // Register `widget` under `id`. The returned action controls the
    // item's visibility (use it instead of the QAction from addWidget()).
    QAction* add_item(const QString &id, QWidget *widget);

    // Apply `font` to this toolbar's menus/actions and to all its items,
    // wherever they are placed.
    void apply_font(const QFont &font);

private:
    QList<ToolbarItem> _items;
};

} // namespace toolbars
} // namespace pv

#endif // DSVIEW_PV_TOOLBARS_TOOLBARITEMSOURCE_H
