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

#ifndef DSVIEW_PV_DIALOGS_UIGALLERY_H
#define DSVIEW_PV_DIALOGS_UIGALLERY_H

#include <QWidget>
#include <QString>

class QScrollArea;
class QPushButton;

namespace pv {
namespace dialogs {

/*
 * UI gallery: every kind of control, the message boxes, a dialog, a menu,
 * the palette colors and all icons on one page, in the current theme.
 * For checking a change to theme.qss / the palettes at a glance.
 *
 *   DSView --gallery              open it next to the main window
 *   DSView --gallery-shot <dir>   save <dir>/gallery-dark.png and
 *                                 gallery-light.png, then quit
 *
 * Together with DSVIEW_THEME_DIR (live editing, see themes/README.md) the
 * page is rebuilt on every save of a theme file.
 */
class UiGallery : public QWidget
{
    Q_OBJECT

public:
    explicit UiGallery(QWidget *parent = NULL);
    ~UiGallery();

    static void Open();
    static void Shoot(const QString &dir);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void set_style(const QString &style);
    void rebuild();
    QWidget* build_content();

private:
    QScrollArea *_scroll;
    QPushButton *_dark_button;
    QPushButton *_light_button;
    QString      _app_style;     // theme of the app when the gallery opened
    QString      _style;         // theme shown in the gallery
};

} // namespace dialogs
} // namespace pv

#endif // DSVIEW_PV_DIALOGS_UIGALLERY_H
