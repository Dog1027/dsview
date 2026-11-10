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

#include "../ui/uimanager.h"

class QHBoxLayout;
class QToolButton;

namespace pv {
namespace toolbars {

class ToolbarItemSource;

/*
 * The single, full-width toolbar at the top of the main window.
 * It collects the items registered by the source toolbars and arranges
 * them into groups as described by a layout text (see topbarlayout.h).
 *
 * All icon+text buttons get the same width, and so do all icon-only
 * buttons; recomputed whenever language, theme or font size change.
 */
class TopBar : public QToolBar, public IUiWindow
{
public:
    explicit TopBar(QWidget *parent);
    ~TopBar();

    // Arrange the items of `sources` according to `layout`.
    void build(const QList<ToolbarItemSource*> &sources, const char *layout);

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
    QHBoxLayout *_row_layout;
    QList<QToolButton*> _text_buttons;
    QList<QToolButton*> _icon_buttons;
};

} // namespace toolbars
} // namespace pv

#endif // DSVIEW_PV_TOOLBARS_TOPBAR_H
