/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 *
 * Copyright (C) 2016 DreamSourceLab <support@dreamsourcelab.com>
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


#include "dsmessagebox.h"
#include "shadow.h"

#include <QFrame>
#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QAbstractButton>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QStyle>
#include <QTimer>
#include "../dsvdef.h"
#include "../ui/langresource.h"
#include "../config/appconfig.h"
#include "../ui/fn.h"
#include "../ui/popupdlglist.h"

namespace pv {
namespace dialogs {

DSMessageBox::DSMessageBox(QWidget *parent,const QString title) :
#ifdef Q_OS_LINUX
    QDialog(NULL)  //enable the popup dialog draged.
#else
    QDialog(parent)
#endif
{
    (void)parent;
    _layout = NULL;
    _main_widget = NULL;
    _msg = NULL;
    _titlebar = NULL;
    _shadow = NULL;  
    _main_layout = NULL;

    _bClickYes = false;

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // Rounded card with the dialog content ("Dialogs" in theme.qss).
    _main_widget = new QFrame(this);
    _main_widget->setObjectName("dialog_frame");
    _main_layout = new QVBoxLayout(_main_widget);
    _main_widget->setLayout(_main_layout);  

    _shadow = new Shadow(this);
    _msg = new QMessageBox(this);
    _titlebar = new toolbars::TitleBar(false, this, NULL, false);
    _layout = new QVBoxLayout(this);
 
    _shadow->setBlurRadius(10.0);
    _shadow->setDistance(3.0);
    _shadow->setColor(QColor(0, 0, 0, 80));

    _main_widget->setAutoFillBackground(false);   // the stylesheet paints it, rounded
    this->setGraphicsEffect(_shadow);  

    _msg->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);   

    if (!title.isEmpty()){
        _titlebar->setTitle(title);
    }
    else{
        _titlebar->setTitle(L_S(STR_PAGE_MSG, S_ID(IDS_MSG_MESSAGE), "Message"));
    }
    
    _main_layout->addWidget(_titlebar);
    _main_layout->addWidget(_msg);   
    _layout->addWidget(_main_widget);

    setLayout(_layout); 

    connect(_msg, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(on_button(QAbstractButton*)));
}


DSMessageBox::~DSMessageBox()
{
    DESTROY_QT_OBJECT(_layout);
    DESTROY_QT_OBJECT(_main_widget);
    DESTROY_QT_OBJECT(_msg);
    DESTROY_QT_OBJECT(_titlebar);
    DESTROY_QT_OBJECT(_shadow);
    DESTROY_QT_OBJECT(_main_layout);

    PopupDlgList::RemoveDlgFromList(this);
}

void DSMessageBox::accept()
{
    using namespace Qt;

    QDialog::accept();
}

void DSMessageBox::reject()
{
    using namespace Qt;

    QDialog::reject();
}
  
QMessageBox* DSMessageBox::mBox()
{
    return _msg;
}
  
void DSMessageBox::on_button(QAbstractButton *btn)
{
    QMessageBox::ButtonRole role = _msg->buttonRole(btn);

    if (role == QMessageBox::AcceptRole || role == QMessageBox::YesRole){
        _bClickYes = true;
         accept();
    } 
    else
        reject();
}

int DSMessageBox::exec()
{
    prepare();

    PopupDlgList::AddDlgTolist(this);

    return QDialog::exec();
}

void DSMessageBox::prepare()
{
    QFont font = this->font();
    font.setPointSizeF(AppConfig::Instance().appOptions.fontSize);
    ui::set_form_font(this, font);
    // The box itself too: its labels fall back to its font when shown and
    // when the stylesheet styles them.
    _msg->setFont(font);

    // No stock icon: the text says what it is about.
    _msg->setIcon(QMessageBox::NoIcon);
    _msg->setText(_msg->text().trimmed());

    // With a detail text below it, the main text is the heading
    // ("Dialogs" in theme.qss).
    QLabel *heading = _msg->findChild<QLabel*>("qt_msgbox_label");
    if (heading != NULL){
        heading->setProperty("heading", !_msg->informativeText().isEmpty());
        heading->style()->unpolish(heading);
        heading->style()->polish(heading);
    }

    // Text aligned with the title, no empty icon column in front of it.
    QGridLayout *grid = qobject_cast<QGridLayout*>(_msg->layout());
    if (grid != NULL){
        // QMessageBox sets its margins again when it is shown.
        auto tidy = [grid]() {
            grid->setContentsMargins(5, 8, 0, 0);
            grid->setHorizontalSpacing(0);
            grid->setVerticalSpacing(10);
            // Without an icon Qt keeps a spacer in its place (row 0, column 0).
            for (int i = 0; i < grid->count(); i++){
                int r, c, rs, cs;
                grid->getItemPosition(i, &r, &c, &rs, &cs);
                QSpacerItem *sp = grid->itemAt(i)->spacerItem();
                if (sp != NULL && r == 0 && c == 0)
                    sp->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
            }
            grid->setColumnStretch(0, 0);
            grid->setColumnStretch(1, 1);
            grid->invalidate();
        };
        tidy();
        QTimer::singleShot(0, this, tidy);
    }
    if (QLabel *icon = _msg->findChild<QLabel*>("qt_msgboxex_icon_label"))
        icon->setFixedWidth(0);
    for (QLabel *label : _msg->findChildren<QLabel*>()){
        label->setIndent(0);
        label->setContentsMargins(0, 0, 0, 0);
    }

    // A comfortable minimum width, so short messages are not cramped.
    if (grid != NULL && property("min_width_added").isNull()){
        grid->addItem(new QSpacerItem(340, 0, QSizePolicy::Minimum, QSizePolicy::Fixed),
                      grid->rowCount(), 0, 1, grid->columnCount());
        setProperty("min_width_added", true);
    }

    // The default (blue) button is the one that does it (Yes / OK), not
    // whichever button happens to come first.
    if (_msg->defaultButton() == NULL){
        for (QAbstractButton *b : _msg->buttons()){
            QMessageBox::ButtonRole role = _msg->buttonRole(b);
            QPushButton *pb = qobject_cast<QPushButton*>(b);
            if (pb != NULL && (role == QMessageBox::YesRole || role == QMessageBox::AcceptRole)){
                _msg->setDefaultButton(pb);
                break;
            }
        }
    }
    // Only that button looks like the default one, even while another has
    // the keyboard focus; Enter always presses it.
    for (QAbstractButton *b : _msg->buttons()){
        QPushButton *pb = qobject_cast<QPushButton*>(b);
        if (pb != NULL)
            pb->setAutoDefault(false);
    }
    if (QPushButton *def = _msg->defaultButton()){
        def->setDefault(true);
        QTimer::singleShot(0, def, [def]() { def->setFocus(); });
    }

    if (_titlebar != NULL){
        _titlebar->update_font();
    }
}

} // namespace dialogs
} // namespace pv
