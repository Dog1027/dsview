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

#ifndef DSVIEW_PV_UI_APPSTATUSBAR_H
#define DSVIEW_PV_UI_APPSTATUSBAR_H

#include <QStatusBar>
#include <QTimer>

class QLabel;

namespace pv {

class SigSession;

namespace view {
class View;
}

namespace ui {

/*
 * The bar at the bottom of the main window.
 *
 *   left : device  |  mode  |  state (ready / capturing / waiting for trigger)
 *   right: time under the mouse  |  distance between cursors 1 and 2  |
 *          when the data on screen was captured, and what it is
 *          (samples, duration, rate)
 *
 * It only reads cheap, in-memory session state, refreshed by a timer, so it
 * needs no hooks in the capture code. Look: "Status bar" in theme.qss.
 */
class AppStatusBar : public QStatusBar
{
public:
    AppStatusBar(SigSession *session, view::View *view, QWidget *parent);

    void refresh();
    void refresh_pointer();

private:
    QLabel* add_label(const QString &name, bool permanent);
    static void set_text(QLabel *label, const QString &text);

private:
    SigSession *_session;
    view::View *_view;
    QTimer      _timer;

    QLabel *_device;
    QLabel *_mode;
    QLabel *_state;
    QLabel *_data;
    QLabel *_trigger;
    QLabel *_pointer;
    QLabel *_cursors;
};

} // namespace ui
} // namespace pv

#endif // DSVIEW_PV_UI_APPSTATUSBAR_H
