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

#ifndef DSVIEW_PV_UI_THEME_H
#define DSVIEW_PV_UI_THEME_H

#include <QObject>
#include <QString>
#include <QColor>
#include <QMap>

class QFileSystemWatcher;
class QTimer;

namespace pv {
namespace ui {

/*
 * Builds the application stylesheet from the template (theme.qss) and the
 * palette of a theme (<style>.palette), replacing every @name token with the
 * palette value.
 *
 * Files are read from the Qt resources, or, when the environment variable
 * DSVIEW_THEME_DIR points to a directory (normally <repo>/DSView/themes),
 * from that directory. In that case the files are watched and changed() is
 * emitted on every save, so the stylesheet can be edited live.
 */
class Theme : public QObject
{
    Q_OBJECT

public:
    static Theme& Instance();

    // Load the palette of `style` ("dark"/"light") and return the final QSS.
    QString build_style_sheet(const QString &style);

    // A palette color, e.g. color("accent"). Valid after build_style_sheet().
    QColor color(const QString &name, const QColor &fallback = QColor()) const;

    // True when files are loaded from DSVIEW_THEME_DIR (live edit mode).
    bool is_live() const;

signals:
    // Emitted (debounced) when a watched theme file changed on disk.
    void changed();

private:
    Theme();
    QString read_file(const QString &name) const;
    QMap<QString, QString> parse_palette(const QString &text) const;
    void watch(const QString &name);

private:
    QString _dir;                       // DSVIEW_THEME_DIR, empty = resources
    QMap<QString, QString> _vars;       // current palette
    QFileSystemWatcher *_watcher;
    QTimer *_debounce;
};

} // namespace ui
} // namespace pv

#endif // DSVIEW_PV_UI_THEME_H
