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

#ifndef DSVIEW_PV_UI_PANELCARD_H
#define DSVIEW_PV_UI_PANELCARD_H

#include <QFrame>

class QDockWidget;
class QLabel;

namespace pv {
namespace ui {

/*
 * Turns a side panel (dock widget) into a card: one rounded frame holding
 * a header with the dock's title and the panel content. The dock's own
 * title bar is hidden; the header follows the dock's windowTitle, so
 * retranslating the dock title keeps working. Look: "Cards" in theme.qss.
 */
class PanelCard : public QFrame
{
public:
    // Puts the content into a new card and makes the card the dock's widget.
    static PanelCard* wrap(QDockWidget *dock, QWidget *content);

private:
    PanelCard(QDockWidget *dock, QWidget *content);
    void update_title(const QString &title);

private:
    QLabel *_header;
};

} // namespace ui
} // namespace pv

#endif // DSVIEW_PV_UI_PANELCARD_H
