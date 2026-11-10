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

#include "panelcard.h"

#include <QDockWidget>
#include <QLabel>
#include <QVBoxLayout>

namespace pv {
namespace ui {

PanelCard* PanelCard::wrap(QDockWidget *dock, QWidget *content)
{
    PanelCard *card = new PanelCard(dock, content);
    dock->setTitleBarWidget(new QWidget(dock));   // hide the built-in title bar
    dock->setWidget(card);
    return card;
}

PanelCard::PanelCard(QDockWidget *dock, QWidget *content)
    : QFrame(dock)
{
    setObjectName("panel_card");

    _header = new QLabel(this);
    _header->setObjectName("panel_card_header");

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(_header);
    lay->addWidget(content, 1);

    update_title(dock->windowTitle());
    connect(dock, &QDockWidget::windowTitleChanged, this,
            [this](const QString &title) { update_title(title); });
}

void PanelCard::update_title(const QString &title)
{
    // "Trigger Setting..." -> "Trigger Setting"
    QString text = title.trimmed();
    while (text.endsWith('.'))
        text.chop(1);
    _header->setText(text);
}

} // namespace ui
} // namespace pv
