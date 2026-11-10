/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2013 DreamSourceLab <support@dreamsourcelab.com>
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


#include "about.h"

#include <QPixmap>
#include <QApplication>
#include <QTextBrowser>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QScrollBar>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../ui/theme.h"
  
#include "../config/appconfig.h"
#include "../dsvdef.h"
#include "../utility/encoding.h"
#include "../ui/langresource.h"

namespace pv {
namespace dialogs {

About::About(QWidget *parent) :
    DSDialog(parent, true)
{
    #if defined(__x86_64__) || defined(_M_X64)
        QString arch = "x64";
    #elif defined(__i386) || defined(_M_IX86)
        QString arch = "x86";
    #elif defined(__arm__) || defined(_M_ARM)
        QString arch = "arm";
    #elif defined(__aarch64__)
        QString arch = "arm64";
    #else
        QString arch = "other";
    #endif

    QString site_url = QApplication::organizationDomain();
    if (site_url.startsWith("http") == false){
        site_url = "https://" + site_url;
    }
    const QString github = "https://github.com/DreamSourceLab/DSView";

    // Header: logo, then name / version, links and copyright.
    QLabel *logo = new QLabel();
    logo->setPixmap(QIcon(":/icons/dsl_logo.svg").pixmap(QSize(240, 84)));

    QLabel *name = new QLabel(QString("DSView %1 (%2)").arg(QApplication::applicationVersion()).arg(arch));
    name->setObjectName("heading");

    // Link colour in the HTML: the stylesheet overrides a label's palette.
    const QString link = ui::Theme::Instance().color("link").name();
    QLabel *links = new QLabel(QString("<a href=\"%1\" style=\"color:%3\">%1</a><br/>"
                                       "<a href=\"%2\" style=\"color:%3\">%2</a>")
                               .arg(site_url).arg(github).arg(link));
    links->setOpenExternalLinks(true);
    links->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QLabel *copyright = new QLabel(tr("© DreamSourceLab. All rights reserved."));
    copyright->setObjectName("hint");

    QVBoxLayout *info = new QVBoxLayout();
    info->setSpacing(4);
    info->addWidget(name);
    info->addWidget(links);
    info->addWidget(copyright);

    QVBoxLayout *head = new QVBoxLayout();
    head->setSpacing(12);
    head->addWidget(logo, 0, Qt::AlignLeft);
    head->addLayout(info);

    // Changelog and credits in tabs; only these scroll.
    QString changelog;
    QDir dir(GetAppDataDir());
    int lan = AppConfig::Instance().frameOptions.language;
    QFile news(dir.absolutePath() + "/NEWS" + QString::number(lan));
    if (news.open(QIODevice::ReadOnly)) {
        QTextStream stream(&news);
        encoding::set_utf8(stream);
        changelog = stream.readAll();
    }
    news.close();

    auto browser = [link](QWidget *parent) {
        QTextBrowser *b = new QTextBrowser(parent);
        b->setOpenExternalLinks(true);
        b->setFrameStyle(QFrame::NoFrame);
        b->setFocusPolicy(Qt::NoFocus);
        b->document()->setDefaultStyleSheet(QString("a { color: %1; }").arg(link));
        b->document()->setDocumentMargin(8);
        return b;
    };

    QTabWidget *tabs = new QTabWidget();
    QTextBrowser *log = browser(tabs);
    log->setPlainText(changelog);
    tabs->addTab(log, tr("Changelogs"));

    QTextBrowser *thanks = browser(tabs);
    thanks->setHtml(QString("<a href=\"%1\">%2</a><br/><a href=\"%3\">%4</a><br/>%5")
                    .arg("https://www.kickstarter.com/projects/dreamsourcelab/dslogic-multifunction-instruments-for-everyone")
                    .arg(tr("All backers on kickstarter"))
                    .arg("http://sigrok.org/")
                    .arg(tr("All members of Sigrok project"))
                    .arg(tr("All contributors of all open-source projects")));
    tabs->addTab(thanks, tr("Special Thanks"));

    layout()->setSpacing(16);
    layout()->addLayout(head);
    layout()->addWidget(tabs, 1);

    setTitle(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_ABOUT), "About"));

    // Resizable; the changelog is long, so start with a reasonable size.
    resize(640, 560);
}

About::~About()
{
}

} // namespace dialogs
} // namespace pv
