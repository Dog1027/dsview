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

#include "titlebar.h"
#include <QStyle>
#include <QLabel> 
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent> 
#include <QPainter>
#include <QStyleOption>
#include <assert.h>
#include <QTimer>
#include <QGuiApplication>
#include <QWindow>

#include "../config/appconfig.h"
#include "../appcontrol.h"
#include "../dsvdef.h"
#include "../ui/fn.h"
#include "../log.h"

namespace pv {
namespace toolbars {

TitleBar::TitleBar(bool top, QWidget *parent, ITitleParent *titleParent, bool hasClose) :
    QWidget(parent)
{ 
   _minimizeButton = NULL;
   _maximizeButton = NULL;
   _closeButton = NULL;
   _moving = false;
   _is_draging = false;
   _parent = parent;
   _isTop = top;
   _hasClose = hasClose;
   _title = NULL;
   _is_native = false;
   _titleParent = titleParent;
   _is_done_moved = false; 
   _is_able_drag = true;

    assert(parent);

    setObjectName("TitleBar");
    setContentsMargins(0,0,0,0);
    // Tall enough that the 28px hover circle of the window buttons keeps
    // clear of the window edge.
    const int barHeight = top ? 42 : 36;

    // Window buttons. On Windows: wide, full-height buttons (red close).
    // Elsewhere: just a thin glyph, a soft circle appears on hover.
    // Their look is in theme.qss ("Window title bar").
#ifdef _WIN32
    const QSize buttonSize(46, barHeight);
    const int buttonGap = 0;
    const int rightMargin = 0;
    const bool compactButtons = false;
#else
    const QSize buttonSize(28, 28);
    const int buttonGap = 6;
    // In a dialog the frame already has a margin.
    const int rightMargin = top ? 10 : 0;
    const bool compactButtons = true;
#endif

    QHBoxLayout *lay1 = new QHBoxLayout(this);
    lay1->setContentsMargins(0, 0, 0, 0);
    lay1->setSpacing(0);

    _title = new QLabel(this);

    QHBoxLayout *buttonLay = new QHBoxLayout();
    // Centre the buttons vertically (alignment alone leaves them at the top).
    const int vMargin = (barHeight - buttonSize.height()) / 2;
    buttonLay->setContentsMargins(0, vMargin, rightMargin, barHeight - buttonSize.height() - vMargin);
    buttonLay->setSpacing(buttonGap);

    auto setupButton = [&](QToolButton *bt, const char *name) {
        bt->setObjectName(name);
        bt->setProperty("compact", compactButtons);
        bt->setFixedSize(buttonSize);
        bt->setIconSize(QSize(16, 16));
        bt->setFocusPolicy(Qt::NoFocus);
        buttonLay->addWidget(bt);
    };

    if (_isTop) {
        _minimizeButton = new QToolButton(this);
        _maximizeButton = new QToolButton(this);
        setupButton(_minimizeButton, "MinimizeButton");
        setupButton(_maximizeButton, "MaximizeButton");

        connect(this, SIGNAL(normalShow()), parent, SLOT(showNormal()));
        connect(this, SIGNAL( maximizedShow()), parent, SLOT(showMaximized()));
        connect(_minimizeButton, SIGNAL( clicked()), parent, SLOT(showMinimized()));
        connect(_maximizeButton, SIGNAL( clicked()), this, SLOT(showMaxRestore()));
    }

    if (_isTop || _hasClose) {
        _closeButton= new QToolButton(this);
        setupButton(_closeButton, "CloseButton");
        connect(_closeButton, SIGNAL( clicked()), parent, SLOT(close()));
    }

    // Reserve on the left as much as the buttons take on the right, so the
    // title is centred on the window (and the logo has room).
    int buttons = (_minimizeButton ? 1 : 0) + (_maximizeButton ? 1 : 0) + (_closeButton ? 1 : 0);
    int buttonsWidth = buttons > 0
        ? buttons * buttonSize.width() + (buttons - 1) * buttonGap + rightMargin : 0;

    if (_isTop) {
        lay1->addSpacing(qMax(48, buttonsWidth));
        lay1->addStretch(500);
        lay1->addWidget(_title);
        lay1->addStretch(500);
        lay1->addSpacing(qMax(0, 48 - buttonsWidth));
    }
    else {
        // Dialogs: plain title on the left, lined up with the content.
        lay1->addSpacing(2);
        lay1->addWidget(_title);
        lay1->addStretch(1);
    }
    lay1->addLayout(buttonLay);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); 
    
    ADD_UI(this);
}

TitleBar::~TitleBar(){ 
    DESTROY_QT_OBJECT(_minimizeButton);
    DESTROY_QT_OBJECT(_maximizeButton);
    DESTROY_QT_OBJECT(_closeButton);

    REMOVE_UI(this);
}

void TitleBar::reStyle()
{
    QString iconPath = GetIconPath();

    if (_isTop) {
        _minimizeButton->setIcon(QIcon(iconPath+"/minimize.svg"));
        if (ParentIsMaxsized())
            _maximizeButton->setIcon(QIcon(iconPath+"/restore.svg"));
        else
            _maximizeButton->setIcon(QIcon(iconPath+"/maximize.svg"));
    }
    if (_isTop || _hasClose)
        _closeButton->setIcon(QIcon(iconPath+"/close.svg"));
}

bool TitleBar::ParentIsMaxsized()
{
    if (_titleParent != NULL){
        return _titleParent->ParentIsMaxsized();
    } 
    else{
        return parentWidget()->isMaximized();
    }
}

void TitleBar::paintEvent(QPaintEvent *event)
{ 
    //draw logo icon
    QStyleOption o;
    o.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &p, this);

    p.setRenderHint(QPainter::Antialiasing, true);

    // App icon, like other apps show in their title bar (not in dialogs).
    if (_isTop) {
        static const QIcon logo(":/icons/logo.svg");
        const int size = 18;
        logo.paint(&p, QRect(12, (height() - size) / 2, size, size));
    }

    QWidget::paintEvent(event);
}

void TitleBar::setTitle(QString title)
{
    if (!_is_native){
        _title->setText(title);
    }
    else if (_parent != NULL){
        _parent->setWindowTitle(title);
    }    
}
  
QString TitleBar::title()
{
    if (!_is_native){
        return _title->text();
    }
    else if (_parent != NULL){
        return _parent->windowTitle();
    }
    return "";
}

void TitleBar::showMaxRestore()
{
    QString iconPath = GetIconPath();
    if (ParentIsMaxsized()) {
        _maximizeButton->setIcon(QIcon(iconPath+"/maximize.svg"));
        normalShow();
    } else {
        _maximizeButton->setIcon(QIcon(iconPath+"/restore.svg"));
        maximizedShow();
    }   
}

void TitleBar::setRestoreButton(bool max)
{
    QString iconPath = GetIconPath();
    if (!max) {
        _maximizeButton->setIcon(QIcon(iconPath+"/maximize.svg"));
    } else {
        _maximizeButton->setIcon(QIcon(iconPath+"/restore.svg"));
    }
}
  
void TitleBar::mousePressEvent(QMouseEvent* event)
{ 
    bool ableMove = !ParentIsMaxsized();

    if(event->button() == Qt::LeftButton && ableMove && _is_able_drag) 
    {
        int x = event->pos().x();
        int y = event->pos().y(); 
        
        bool bTopWidow = AppControl::Instance()->GetTopWindow() == _parent;
        bool bClick = (x >= 6 && y >= 5 && x <= width() - 6);  //top window need resize hit check
 
        if (!bTopWidow || bClick ){
            // On Wayland an application can't position its own window,
            // so move() does nothing: let the compositor move it instead.
            // (On Linux _titleParent is the MainFrame itself, so don't test it.)
            if (QGuiApplication::platformName() == "wayland")
            {
                QWindow *win = _parent->window()->windowHandle();
                if (win != NULL && win->startSystemMove()){
                    event->accept();
                    return;
                }
                dsv_info("TitleBar: startSystemMove() failed, falling back to move().");
            }

            _is_draging = true;             

            _clickPos = event->globalPos(); 

            if (_titleParent != NULL){
                _oldPos = _titleParent->GetParentPos();
            }
            else{
                _oldPos = _parent->pos(); 
            }

            _is_done_moved = false;
                
            event->accept();
            return;
        } 
    }  
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{  
    if(_is_draging){ 

        int datX = 0;
        int datY = 0;

        datX = (event->globalPos().x() - _clickPos.x());
        datY = (event->globalPos().y() - _clickPos.y());

        int x = _oldPos.x() + datX;
        int y = _oldPos.y() + datY;

        if (!_moving){
            if (ABS_VAL(datX) >= 2 || ABS_VAL(datY) >= 2){
                _moving = true;
            }
            else{
                return;
            }
        }

        if (_titleParent != NULL){

            if (!_is_done_moved){
                _is_done_moved = true;
                _titleParent->MoveBegin();
            }

            _titleParent->MoveWindow(x, y);
        }
        else{

#ifdef _WIN32

        QRect screenRect = AppControl::Instance()->_screenRect;

        if (screenRect.width() > 0 && QGuiApplication::screens().size() > 1)
        {
            QRect rect = _parent->frameGeometry();

            if (x < screenRect.left()){
                x = screenRect.left();
            }
            if (x + _parent->frameGeometry().width() > screenRect.right())
            {
                x = screenRect.right() - _parent->frameGeometry().width();
            }
        }
#endif

            _parent->move(x, y);
        }
        
        event->accept();
        return;
    } 
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (_moving && _titleParent != NULL){
        _titleParent->MoveEnd();
    }
    _moving = false;
    _is_draging = false;
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{  
    QWidget::mouseDoubleClickEvent(event); 

    if (_isTop){ 

      QTimer::singleShot(200, this, [this](){
                showMaxRestore();
            });
    }
}

void TitleBar::UpdateLanguage()
{
    
}

void TitleBar::UpdateTheme()
{
    reStyle();
}

void TitleBar::UpdateFont()
{  
    QFont font = this->font();
    font.setPointSizeF(AppConfig::Instance().appOptions.fontSize);
    if (!_isTop)
        font.setWeight(QFont::DemiBold);
    _title->setFont(font);
}

void TitleBar::EnableAbleDrag(bool bEnabled)
{
    _is_able_drag = bEnabled;
}
 
} // namespace toolbars
} // namespace pv
