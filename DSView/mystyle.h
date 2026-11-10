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

#ifndef MYSTYLE_H
#define MYSTYLE_H

#include <QProxyStyle>
#include <QMenu>
#include <QAbstractButton>
#include <QComboBox>
#include <QTabBar>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QFocusEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QAbstractItemView>
#include <QScreen>
#include <QScrollBar>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include "pv/ui/theme.h"

// Keyboard focus ring for check boxes and radio buttons. It is a separate
// widget on top, so it can also go around the name in front of the box
// (the form label or buddy label), which QSS cannot reach.
class FocusRing : public QWidget
{
public:
    explicit FocusRing(QWidget *parent) : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPen pen(pv::ui::Theme::Instance().color("accent", QColor(0x00, 0x78, 0xd4)));
        pen.setWidth(2);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(QRectF(rect()).adjusted(1, 1, -1, -1), 4, 4);
    }
};

class MyStyle : public QProxyStyle
{
    Q_OBJECT
    public:
    int pixelMetric(PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) {
        int s = QProxyStyle::pixelMetric(metric, option, widget);
        if (metric == QStyle::PM_SmallIconSize) {
            s = 24;
        }
        return s;
    }

    // No underlined mnemonic letters (Alt shortcuts still work).
    int styleHint(StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr, QStyleHintReturn *ret = nullptr) const override {
        if (hint == QStyle::SH_UnderlineShortcut)
            return 0;
        // OK / Cancel buttons: text only.
        if (hint == QStyle::SH_DialogButtonBox_ButtonsHaveIcons)
            return 0;
#ifdef Q_OS_LINUX
        // GNOME order: Cancel / No on the left, the button that does it
        // (OK / Yes / Save) at the right end.
        if (hint == QStyle::SH_DialogButtonLayout)
            return QDialogButtonBox::GnomeLayout;
#endif
        return QProxyStyle::styleHint(hint, option, widget, ret);
    }

    // Menus get a transparent window, so the rounded corners of the
    // stylesheet (QMenu in theme.qss) are not painted over.
    void polish(QWidget *widget) override {
        if (qobject_cast<QMenu*>(widget) != nullptr && !widget->testAttribute(Qt::WA_TranslucentBackground)) {
            widget->setAttribute(Qt::WA_TranslucentBackground);
            widget->setWindowFlag(Qt::FramelessWindowHint);
            widget->setWindowFlag(Qt::NoDropShadowWindowHint);
        }

        // Anything clickable shows the pointing hand.
        if (qobject_cast<QAbstractButton*>(widget) != nullptr
            || qobject_cast<QComboBox*>(widget) != nullptr
            || qobject_cast<QTabBar*>(widget) != nullptr) {
            widget->setCursor(Qt::PointingHandCursor);
        }

        // Combo box lists as wide as their longest item (see eventFilter).
        if (QComboBox *cb = qobject_cast<QComboBox*>(widget)) {
            if (QWidget *popup = cb->view()->window())
                popup->installEventFilter(this);
        }

        // Dialog footer: lets theme.qss draw the divider above the buttons.
        // QSS padding does not reach its layout, so the 12 px from the
        // line to the buttons is a contents margin.
        if (qobject_cast<QDialogButtonBox*>(widget) != nullptr) {
            widget->setAttribute(Qt::WA_StyledBackground, true);
            widget->setContentsMargins(0, 12, 0, 0);
            widget->installEventFilter(this);   // equal button widths
        }

        // Check boxes and radio buttons: same keyboard-only focus ring
        // (FocusRing, around the box, its text and the name in front).
        if (qobject_cast<QCheckBox*>(widget) != nullptr
            || qobject_cast<QRadioButton*>(widget) != nullptr) {
            widget->installEventFilter(this);
        }

        if (QPushButton *pb = qobject_cast<QPushButton*>(widget)) {
            // A mouse click does not leave focus on a push button; only
            // the keyboard (Tab) shows the focus ring. Otherwise a clicked
            // button keeps a highlight and looks stuck.
            if (pb->focusPolicy() == Qt::StrongFocus)
                pb->setFocusPolicy(Qt::TabFocus);

            // No auto-default: in a QDialog the default (blue) look would
            // follow the focused button. In a button box the action button
            // (OK / Yes / Save) is the default, and stays it.
            pb->setAutoDefault(false);

            // Focus ring only for keyboard focus ([kbfocus="true"] in
            // theme.qss), not when a dialog puts the focus on its button.
            pb->installEventFilter(this);
            if (QDialogButtonBox *box = qobject_cast<QDialogButtonBox*>(pb->parentWidget())) {
                const QDialogButtonBox::ButtonRole role = box->buttonRole(pb);
                if (role == QDialogButtonBox::AcceptRole || role == QDialogButtonBox::YesRole) {
                    bool other = false;
                    for (QAbstractButton *b : box->buttons()) {
                        QPushButton *o = qobject_cast<QPushButton*>(b);
                        if (o && o != pb && o->isDefault())
                            other = true;
                    }
                    if (!other)
                        pb->setDefault(true);
                }
                // Cancel / No: red (QPushButton[role="reject"] in theme.qss).
                const bool reject = role == QDialogButtonBox::RejectRole
                                 || role == QDialogButtonBox::NoRole;
                pb->setProperty("role", reject ? "reject" : "");
            }
        }

        QProxyStyle::polish(widget);
    }
    using QProxyStyle::polish;

    bool eventFilter(QObject *obj, QEvent *event) override {
        // Button box: the buttons at the right (OK, Cancel, Yes, No, ...)
        // get one width, the widest one's. Help / Reset at the left keep
        // their own.
        if (event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest) {
            if (QDialogButtonBox *box = qobject_cast<QDialogButtonBox*>(obj))
                equalize_buttons(box);
        }

        // A combo box list opens as wide as the combo box only, so long
        // items (and the scroll bar) got cut at the right. Widen it to its
        // content and keep it on the screen.
        if (event->type() == QEvent::Show) {
            QWidget *popup = qobject_cast<QWidget*>(obj);
            QComboBox *cb = popup ? qobject_cast<QComboBox*>(popup->parentWidget()) : nullptr;
            if (cb && popup->isWindow())
                fit_combo_popup(cb, popup);
        }

        QWidget *w = qobject_cast<QWidget*>(obj);
        const bool choice = qobject_cast<QCheckBox*>(obj) != nullptr
                         || qobject_cast<QRadioButton*>(obj) != nullptr;

        if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
            bool kb = false;
            if (event->type() == QEvent::FocusIn) {
                const Qt::FocusReason r = static_cast<QFocusEvent*>(event)->reason();
                kb = r == Qt::TabFocusReason || r == Qt::BacktabFocusReason;
            }
            if (w && choice) {
                if (kb)
                    show_ring(w);
                else if (_ring_target == w)
                    hide_ring();
            }
            else if (w && w->property("kbfocus").toBool() != kb) {
                w->setProperty("kbfocus", kb);
                w->style()->unpolish(w);
                w->style()->polish(w);
                w->update();
            }
        }
        else if (w && w == _ring_target) {
            if (event->type() == QEvent::Move || event->type() == QEvent::Resize)
                show_ring(w);
            else if (event->type() == QEvent::Hide)
                hide_ring();
        }
        return QProxyStyle::eventFilter(obj, event);
    }

private:
    static void equalize_buttons(QDialogButtonBox *box) {
        QList<QPushButton*> same;
        int w = 0;
        for (QAbstractButton *b : box->buttons()) {
            QPushButton *pb = qobject_cast<QPushButton*>(b);
            if (!pb || pb->isHidden())
                continue;
            const QDialogButtonBox::ButtonRole role = box->buttonRole(pb);
            if (role == QDialogButtonBox::HelpRole || role == QDialogButtonBox::ResetRole)
                continue;
            same.append(pb);
            // The width it wants without our minimum.
            const int keep = pb->minimumWidth();
            pb->setMinimumWidth(0);
            w = std::max(w, pb->sizeHint().width());
            pb->setMinimumWidth(keep);
        }
        for (QPushButton *pb : same) {
            if (pb->minimumWidth() != w)
                pb->setMinimumWidth(w);
        }
    }

    static void fit_combo_popup(QComboBox *cb, QWidget *popup) {
        QAbstractItemView *view = cb->view();
        const int text_w = view->sizeHintForColumn(0);
        if (text_w <= 0 || cb->count() == 0)
            return;

        // Frame, QSS padding and border of the list (without a scroll bar).
        QScrollBar *sb = view->verticalScrollBar();
        int chrome = view->width() - view->viewport()->width();
        if (sb->isVisible())
            chrome -= sb->width();

        // A scroll bar is coming when the rows do not fit the height.
        const int rows_h = view->sizeHintForRow(0) * cb->count();
        const bool scrolls = rows_h > view->viewport()->height() + 1;
        const int sb_w = scrolls ? sb->sizeHint().width() : 0;

        const int need = text_w + chrome + sb_w + 4
                       + (popup->width() - view->width());
        if (popup->width() >= need)
            return;

        QRect r = popup->geometry();
        r.setWidth(need);
        if (QScreen *screen = cb->screen()) {
            const QRect avail = screen->availableGeometry();
            if (r.right() > avail.right())
                r.moveRight(avail.right());
            if (r.left() < avail.left())
                r.moveLeft(avail.left());
        }
        popup->setGeometry(r);
    }

    // The label that names `w`: its QFormLayout label or a buddy label.
    static QLabel* name_label(QWidget *w) {
        QWidget *p = w->parentWidget();
        if (!p)
            return nullptr;
        for (QFormLayout *form : p->findChildren<QFormLayout*>()) {
            if (form->indexOf(w) >= 0) {
                if (QLabel *l = qobject_cast<QLabel*>(form->labelForField(w)))
                    return l;
            }
        }
        // A grid row: the label in the cell to the left.
        for (QGridLayout *grid : p->findChildren<QGridLayout*>()) {
            const int i = grid->indexOf(w);
            if (i < 0)
                continue;
            int row, col, rs, cs;
            grid->getItemPosition(i, &row, &col, &rs, &cs);
            if (col > 0) {
                QLayoutItem *it = grid->itemAtPosition(row, col - 1);
                if (QLabel *l = it ? qobject_cast<QLabel*>(it->widget()) : nullptr)
                    return l;
            }
        }
        for (QLabel *l : p->findChildren<QLabel*>()) {
            if (l->buddy() == w)
                return l;
        }
        return nullptr;
    }

    // Box and its text, without the empty part of a stretched widget.
    static QRect content_rect(QWidget *w) {
        const QSize hint = w->sizeHint();
        QRect r(QPoint(0, 0), QSize(qMin(hint.width(), w->width()), w->height()));
        if (w->layoutDirection() == Qt::RightToLeft)
            r.moveRight(w->width() - 1);
        return r;
    }

    void show_ring(QWidget *w) {
        QWidget *p = w->parentWidget();
        if (!p)
            return;
        QRect r = content_rect(w).translated(w->pos());

        QLabel *l = name_label(w);
        if (l && l->isVisible() && !l->text().trimmed().isEmpty() && p->isAncestorOf(l)) {
            QString text = l->text();
            text.remove('&');
            const QSize ts(l->fontMetrics().horizontalAdvance(text), l->fontMetrics().height());
            QRect lr = QStyle::alignedRect(l->layoutDirection(), l->alignment(), ts, l->rect());
            lr.moveTopLeft(l->mapTo(p, lr.topLeft()));
            r = r.united(lr);
        }

        r = r.adjusted(-5, -3, 5, 3).intersected(p->rect());

        if (!_ring || _ring->parentWidget() != p) {
            delete _ring;
            _ring = new FocusRing(p);
        }
        _ring_target = w;
        _ring->setGeometry(r);
        _ring->raise();
        _ring->show();
        _ring->update();
    }

    void hide_ring() {
        if (_ring)
            _ring->hide();
        _ring_target = nullptr;
    }

    QPointer<FocusRing> _ring;
    QPointer<QWidget> _ring_target;
};

#endif // MYSTYLE_H
