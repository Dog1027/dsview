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

#ifndef DSVIEW_PV_TOOLBARS_TOPBAR_H
#define DSVIEW_PV_TOOLBARS_TOPBAR_H

#include <QToolBar>
#include <QList>
#include <QSet>

#include "../ui/uimanager.h"

class QBoxLayout;
class QToolButton;

namespace pv {
namespace toolbars {

class ToolbarItemSource;

/*
 * A toolbar built from a layout text (see topbarlayout.h): it collects the
 * items registered by the source toolbars and arranges them into groups.
 * Used horizontally for the top bar and vertically for the activity bar.
 *
 * All icon+text buttons get the same width, and so do all icon-only
 * buttons; recomputed whenever language, theme or font size change.
 */
class TopBar : public QToolBar, public IUiWindow
{
public:
    TopBar(QWidget *parent, const QString &name = "main_bar",
           Qt::Orientation orientation = Qt::Horizontal, int iconSize = 20);
    ~TopBar();

    // Arrange the items of `sources` according to `layout`. Items whose id
    // is in `taken` belong to another bar and are left alone. With
    // `hide_rest`, items no layout mentions are parked in a hidden toolbar;
    // only the bar that is built last may do that.
    void build(const QList<ToolbarItemSource*> &sources, const char *layout,
               const QSet<QString> &taken = QSet<QString>(), bool hide_rest = true);

    // Ids of the items this bar shows.
    inline const QSet<QString>& placed(){
        return _placed;
    }

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

    // IUiWindow: called after the source toolbars updated their texts/fonts.
    void UpdateLanguage() override;
    void UpdateTheme() override;
    void UpdateFont() override;

private:
    QToolBar* add_group(const QString &name, bool fixed);
    void equalize_widths();

private:
    QWidget     *_row;
    QBoxLayout  *_row_layout;
    int          _icon_size;
    QSet<QString> _placed;
    QList<QToolButton*> _text_buttons;
    QList<QToolButton*> _icon_buttons;
};

} // namespace toolbars
} // namespace pv

#endif // DSVIEW_PV_TOOLBARS_TOPBAR_H
