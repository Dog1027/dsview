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

#include "topbar.h"

#include <QBoxLayout>
#include <QWidgetAction>
#include <QToolButton>
#include <QToolTip>
#include <QHelpEvent>
#include <QMap>
#include <QSet>
#include <QRegularExpression>

#include "toolbaritemsource.h"
#include "../log.h"

namespace pv {
namespace toolbars {

TopBar::TopBar(QWidget *parent, const QString &name, Qt::Orientation orientation, int iconSize)
    : QToolBar(name, parent)
{
    setObjectName(name);
    setOrientation(orientation);
    _icon_size = iconSize;
    setMovable(false);
    setFloatable(false);
    toggleViewAction()->setVisible(false);
    setContextMenuPolicy(Qt::PreventContextMenu);

    _row = new QWidget(this);
    _row->setObjectName("main_bar_row");
    _row_layout = new QBoxLayout(orientation == Qt::Horizontal
                                    ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom, _row);
    _row_layout->setContentsMargins(0, 0, 0, 0);
    _row_layout->setSpacing(0);
    addWidget(_row);

    ADD_UI(this);
}

TopBar::~TopBar()
{
    REMOVE_UI(this);
}

QToolBar* TopBar::add_group(const QString &name, bool fixed)
{
    QToolBar *group = new QToolBar(name, _row);
    group->setObjectName("group_" + name);
    group->setOrientation(orientation());
    group->setMovable(false);
    group->setFloatable(false);
    group->setIconSize(QSize(_icon_size, _icon_size));
    if (fixed)
        group->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _row_layout->addWidget(group);
    return group;
}

void TopBar::build(const QList<ToolbarItemSource*> &sources, const char *layout,
                   const QSet<QString> &taken, bool hide_rest)
{
    QMap<QString, ToolbarItem> items;
    QSet<QString> &placed = _placed;

    for (ToolbarItemSource *src : sources)
    {
        for (const ToolbarItem &item : src->items())
        {
            if (taken.contains(item.id))
                continue;
            if (items.contains(item.id))
                dsv_err("TopBar: duplicate item id \"%s\"", item.id.toUtf8().data());
            items[item.id] = item;
        }
        // The source toolbars only own the items; they are never shown.
        src->setVisible(false);
    }

    // Tokens: "[group flags]" or any non-space word; '#' starts a comment.
    QString text = QString::fromUtf8(layout);
    text.remove(QRegularExpression("#[^\n]*"));
    QRegularExpression token_re("\\[[^\\]]*\\]|\\S+");
    auto it = token_re.globalMatch(text);

    QToolBar *group = NULL;
    int auto_group = 0;

    while (it.hasNext())
    {
        QString tok = it.next().captured(0);

        if (tok.startsWith('['))
        {
            QStringList parts = tok.mid(1, tok.size() - 2).split(' ', Qt::SkipEmptyParts);
            QString name = parts.isEmpty() ? QString("g%1").arg(auto_group++) : parts.takeFirst();
            group = add_group(name, parts.contains("fixed"));
            continue;
        }
        if (tok == "->")
        {
            _row_layout->addStretch(1);
            group = NULL;
            continue;
        }
        if (group == NULL)
            group = add_group(QString("g%1").arg(auto_group++), false);

        if (tok == "|")
        {
            group->addSeparator();
            continue;
        }

        QString id = tok.section(':', 0, 0);
        QString style = tok.section(':', 1);

        if (!items.contains(id))
        {
            dsv_err("TopBar: unknown item \"%s\" in layout", id.toUtf8().data());
            continue;
        }
        if (placed.contains(id))
        {
            dsv_err("TopBar: item \"%s\" is placed twice", id.toUtf8().data());
            continue;
        }
        if (!style.isEmpty() && style != "icon" && style != "text")
            dsv_err("TopBar: unknown style \"%s\" for item \"%s\"", style.toUtf8().data(), id.toUtf8().data());

        const ToolbarItem &item = items[id];
        group->addAction(item.action);
        placed.insert(id);

        QToolButton *bt = qobject_cast<QToolButton*>(item.widget);
        if (bt != NULL)
        {
            if (style == "icon")
            {
                bt->setToolButtonStyle(Qt::ToolButtonIconOnly);
                bt->installEventFilter(this);   // tooltip from its text
                _icon_buttons.append(bt);
            }
            else
            {
                bt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                _text_buttons.append(bt);
            }
        }
    }

    // Items not mentioned in the layout go to a group that is never shown,
    // so the source toolbars can still toggle their actions safely.
    QToolBar *unused = NULL;
    for (const ToolbarItem &item : items)
    {
        if (!hide_rest || placed.contains(item.id))
            continue;
        if (unused == NULL)
        {
            unused = new QToolBar("unused", this);
            unused->setVisible(false);
        }
        unused->addAction(item.action);
    }

    equalize_widths();
}

void TopBar::equalize_widths()
{
    for (QList<QToolButton*> *list : {&_text_buttons, &_icon_buttons})
    {
        int width = 0;
        for (QToolButton *bt : *list)
        {
            bt->setMinimumWidth(0);
            width = qMax(width, bt->sizeHint().width());
        }
        for (QToolButton *bt : *list)
            bt->setMinimumWidth(width);
    }
}

void TopBar::UpdateLanguage()
{
    equalize_widths();
}

void TopBar::UpdateTheme()
{
    equalize_widths();
}

void TopBar::UpdateFont()
{
    equalize_widths();
}

bool TopBar::eventFilter(QObject *object, QEvent *event)
{
    // Icon-only items: show their current (translated) text as tooltip.
    if (event->type() == QEvent::ToolTip)
    {
        QToolButton *bt = qobject_cast<QToolButton*>(object);
        if (bt != NULL && bt->toolTip().isEmpty() && !bt->text().isEmpty())
        {
            QHelpEvent *he = static_cast<QHelpEvent*>(event);
            QToolTip::showText(he->globalPos(), bt->text().remove('&'), bt);
            return true;
        }
    }
    return QToolBar::eventFilter(object, event);
}

} // namespace toolbars
} // namespace pv
