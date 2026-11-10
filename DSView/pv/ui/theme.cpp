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

#include "theme.h"

#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QRegularExpression>
#include <QSet>

#include "../log.h"

namespace pv {
namespace ui {

namespace
{
    // @name tokens: a letter followed by letters, digits, '-' or '_'.
    const QRegularExpression TOKEN_RE("@([A-Za-z][A-Za-z0-9_-]*)");
}

Theme& Theme::Instance()
{
    static Theme *ins = new Theme();
    return *ins;
}

Theme::Theme()
{
    _watcher = NULL;
    _debounce = NULL;

    QString dir = qEnvironmentVariable("DSVIEW_THEME_DIR");
    if (!dir.isEmpty())
    {
        if (QDir(dir).exists("theme.qss"))
        {
            _dir = QDir(dir).absolutePath();
            _watcher = new QFileSystemWatcher(this);
            _debounce = new QTimer(this);
            _debounce->setSingleShot(true);
            _debounce->setInterval(150);

            connect(_debounce, &QTimer::timeout, this, &Theme::changed);
            connect(_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path)
            {
                // Editors often save by replacing the file, which drops the watch.
                if (!_watcher->files().contains(path) && QFile::exists(path))
                    _watcher->addPath(path);
                _debounce->start();
            });

            dsv_info("Theme: live edit enabled, reading \"%s\"", _dir.toUtf8().data());
        }
        else
        {
            dsv_err("Theme: DSVIEW_THEME_DIR=\"%s\" has no theme.qss, using built-in theme.",
                    dir.toUtf8().data());
        }
    }
}

bool Theme::is_live() const
{
    return !_dir.isEmpty();
}

QString Theme::read_file(const QString &name) const
{
    QString path = _dir.isEmpty() ? (":/" + name) : (_dir + "/" + name);
    QFile f(path);

    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        dsv_err("Theme: can't read \"%s\"", path.toUtf8().data());
        return QString();
    }
    return QString::fromUtf8(f.readAll());
}

void Theme::watch(const QString &name)
{
    if (_watcher == NULL)
        return;

    QString path = _dir + "/" + name;
    if (!_watcher->files().contains(path))
        _watcher->addPath(path);
}

QMap<QString, QString> Theme::parse_palette(const QString &text) const
{
    QMap<QString, QString> vars;

    // Strip /* comments */, then read "@name: value;" entries.
    QString src = text;
    src.remove(QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption));

    QRegularExpression entry("@([A-Za-z][A-Za-z0-9_-]*)\\s*:\\s*([^;]+);");
    auto it = entry.globalMatch(src);

    while (it.hasNext())
    {
        auto m = it.next();
        vars[m.captured(1)] = m.captured(2).trimmed();
    }

    // Let a value reference another entry (@a: @b;), a few levels deep.
    for (int pass = 0; pass < 4; pass++)
    {
        for (auto k = vars.begin(); k != vars.end(); ++k)
        {
            auto m = TOKEN_RE.match(k.value());
            if (m.hasMatch() && m.capturedStart() == 0 && vars.contains(m.captured(1)))
                k.value() = vars.value(m.captured(1));
        }
    }

    return vars;
}

QString Theme::build_style_sheet(const QString &style)
{
    QString palette_name = style + ".palette";
    QString tpl = read_file("theme.qss");
    _vars = parse_palette(read_file(palette_name));

    watch("theme.qss");
    watch(palette_name);

    // Tokens inside /* comments */ are left alone.
    QList<QPair<int, int>> comments;
    auto cit = QRegularExpression("/\\*.*?\\*/", QRegularExpression::DotMatchesEverythingOption).globalMatch(tpl);
    while (cit.hasNext())
    {
        auto c = cit.next();
        comments.append(qMakePair(c.capturedStart(), c.capturedEnd()));
    }
    auto in_comment = [&comments](int pos)
    {
        for (const auto &c : comments){
            if (pos >= c.first && pos < c.second)
                return true;
        }
        return false;
    };

    QString out;
    out.reserve(tpl.size() + 1024);
    QSet<QString> missing;
    int last = 0;
    auto it = TOKEN_RE.globalMatch(tpl);

    while (it.hasNext())
    {
        auto m = it.next();
        if (in_comment(m.capturedStart()))
            continue;

        out += QStringView(tpl).mid(last, m.capturedStart() - last);

        QString name = m.captured(1);
        if (_vars.contains(name)){
            out += _vars.value(name);
        }
        else{
            out += m.captured(0);
            missing.insert(name);
        }
        last = m.capturedEnd();
    }
    out += QStringView(tpl).mid(last);

    for (const QString &name : missing)
    {
        dsv_err("Theme: \"@%s\" is used in theme.qss but not defined in %s",
                name.toUtf8().data(), palette_name.toUtf8().data());
    }

    return out;
}

QColor Theme::color(const QString &name, const QColor &fallback) const
{
    QColor c(_vars.value(name));
    return c.isValid() ? c : fallback;
}

} // namespace ui
} // namespace pv

namespace pv {
namespace ui {

QStringList Theme::color_names() const
{
    QStringList names;
    for (auto it = _vars.begin(); it != _vars.end(); ++it){
        if (QColor(it.value()).isValid())
            names.append(it.key());
    }
    return names;
}

} // namespace ui
} // namespace pv
