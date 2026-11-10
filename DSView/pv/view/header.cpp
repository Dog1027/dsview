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

#include "header.h" 
  
#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QStyleOption>
#include <QApplication>
#include <assert.h>
#include <algorithm>
#include <QFont>

#include "view.h"
#include "trace.h"
#include "dsosignal.h"
#include "logicsignal.h"
#include "analogsignal.h"
#include "groupsignal.h"
#include "decodetrace.h"
#include "../sigsession.h"
#include "../dsvdef.h"
#include "../ui/langresource.h"
#include "../appcontrol.h"
#include "../config/appconfig.h"
#include "../ui/fn.h"
#include "../log.h"
#include "../ui/theme.h"
 
using namespace std;

namespace pv {
namespace view {

Header::Header(View &parent) :
	QWidget(&parent),
    _view(parent)
{
    _moveFlag = false;
    _colorFlag = false;
    _nameFlag = false;
    _context_trace = NULL;
    _mouse_is_down = false;
    
    nameEdit = new PopupLineEdit(this);
    nameEdit->setFixedWidth(100);
    nameEdit->hide();
    nameEdit->set_instant_mode(true);

	setMouseTracking(true);

    connect(nameEdit, SIGNAL(editingFinished()),
            this, SLOT(on_action_set_name_triggered()));


    ADD_UI(this);
}

Header::~Header()
{
    REMOVE_UI(this);
}

void Header::retranslateUi()
{
    update();
}


int Header::get_nameEditWidth()
{ 
    if (nameEdit->hasFocus())
        return nameEdit->width();
    else
        return 0;
}

pv::view::Trace* Header::get_mTrace(int &action, const QPoint &pt)
{
    const int w = width();
    std::vector<Trace*> traces;
    _view.get_traces(ALL_VIEW, traces);

    for(auto t : traces)
    {
        if ((action = t->pt_in_rect(t->get_y(), w, pt)))
            return t;
    }

    return NULL;
}

void Header::paintEvent(QPaintEvent*)
{ 
    using pv::view::Trace;

    QStyleOption o;
    o.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &o, &painter, this);

	const int w = width();
    std::vector<Trace*> traces;
    _view.get_traces(ALL_VIEW, traces);

    const bool dragging = !_drag_traces.empty();
    QColor fore(QWidget::palette().color(QWidget::foregroundRole()));
    fore.setAlpha(View::ForeAlpha);
 
    QFont font(painter.font());
    float fSize = AppConfig::Instance().appOptions.fontSize;
    if (fSize > 10)
        fSize = 10;
    font.setPointSizeF(fSize);
    painter.setFont(font);

    const bool logic = _view.session().get_device()->get_work_mode() == LOGIC;
    if (logic)
        paint_lanes(painter, traces, w);

    for(auto t : traces)
	{
        t->paint_label(painter, w, dragging ? QPoint(-1, -1) : _mouse_point, fore);
	}

    if (logic && !dragging && _view.hover_wave_x() >= 0)
        paint_values(painter, traces, w);

	painter.end();
}

// The row under the mouse (the same band as Viewport::paintHoverRow, which
// it joins at the right) and, for logic channels, the lane line of the
// waves running in under the name, fading in.
void Header::paint_lanes(QPainter &p, std::vector<Trace*> &traces, int w)
{
    const ui::Theme &theme = ui::Theme::Instance();
    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);

    const int hy = _view.hover_point().y();
    if (_view.hover_point().x() >= 0 && hy >= 0) {
        const int pad = std::max(0, (_view.get_spanY() - _view.get_signalHeight()) / 2);
        for (auto t : traces) {
            if (!t->enabled() || t->rows_size() == 0)
                continue;
            const int top = t->get_y() - t->get_totalHeight() / 2 - pad;
            const int bottom = t->get_y() + t->get_totalHeight() / 2 + pad;
            if (hy < top || hy >= bottom)
                continue;
            QColor band = theme.color("hover", QColor(128, 128, 128));
            band.setAlpha(150);
            p.setPen(Qt::NoPen);
            p.setBrush(band);
            p.drawRoundedRect(QRectF(2, top + 1, w + 6, bottom - top - 2), 4, 4);
            break;
        }
    }

    QColor fore(QWidget::palette().color(QWidget::foregroundRole()));
    p.setRenderHint(QPainter::Antialiasing, false);
    for (auto t : traces) {
        LogicSignal *s = dynamic_cast<LogicSignal*>(t);
        if (s == NULL)
            continue;
        const QRectF name_rect = t->get_rect("name", t->get_y(), w);
        const int low = t->get_y() + t->get_totalHeight() * 0.5 + 0.5f;
        QColor c = s->wave_colour(fore);
        c.setAlpha(t->enabled() ? 70 : 30);
        QColor c0 = c;
        c0.setAlpha(0);
        QLinearGradient g(name_rect.left(), 0, w, 0);
        g.setColorAt(0, c0);
        g.setColorAt(1, c);
        p.setPen(QPen(QBrush(g), 1));
        p.drawLine(QPointF(name_rect.left(), low), QPointF(w, low));
    }
    p.restore();
}

// Values at the mouse x over the waves, left of the number tag: the level
// of each logic channel, the annotation of each decoder row.
void Header::paint_values(QPainter &p, std::vector<Trace*> &traces, int w)
{
    const ui::Theme &theme = ui::Theme::Instance();
    const int x = _view.hover_wave_x();
    QColor fore(QWidget::palette().color(QWidget::foregroundRole()));

    const QFontMetrics name_fm(p.font());
    p.save();
    p.setFont(::ui::data_font(p.font()));
    const QFontMetrics fm(p.font());
    p.setRenderHint(QPainter::Antialiasing, true);

    for (auto t : traces) {
        if (!t->enabled())
            continue;
        const QRectF label_rect = t->get_rect("label", t->get_zero_vpos(), w);
        const QRectF name_rect = t->get_rect("name", t->get_y(), w);
        const qreal right = label_rect.left() - 6;
        const int row_h = std::max(10, std::min(fm.height(), t->get_totalHeight() - 2));

        if (LogicSignal *s = dynamic_cast<LogicSignal*>(t)) {
            bool level;
            if (!s->level_at(x, level))
                continue;
            const QString text = level ? "1" : "0";
            const qreal pw = fm.horizontalAdvance(text) + 8;
            const QRectF r(right - pw, t->get_y() - row_h / 2.0, pw, row_h);
            const QColor wave = s->wave_colour(fore);
            if (level) {
                QColor fill = wave;
                fill.setAlpha(70);
                p.setPen(Qt::NoPen);
                p.setBrush(fill);
                p.drawRoundedRect(r, 3, 3);
                p.setPen(wave);
            }
            else {
                p.setPen(QPen(theme.color("border", QColor(80, 80, 80)), 1));
                p.setBrush(Qt::NoBrush);
                p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
                p.setPen(theme.color("text-muted", QColor(140, 140, 140)));
            }
            p.drawText(r, Qt::AlignCenter, text);
        }
        else if (DecodeTrace *d = dynamic_cast<DecodeTrace*>(t)) {
            std::vector<std::pair<int, std::vector<QString>>> values;
            d->values_at(x, values);
            const qreal left = name_rect.left() + name_fm.horizontalAdvance(t->get_name()) + 8;
            const qreal avail = right - left - 8;
            if (avail < 12)
                continue;
            for (auto &v : values) {
                // The longest text that fits, else the shortest, elided.
                QString text;
                for (const QString &a : v.second) {
                    if (fm.horizontalAdvance(a) <= avail) {
                        text = a;
                        break;
                    }
                }
                if (text.isEmpty())
                    text = fm.elidedText(v.second.back(), Qt::ElideRight, avail);
                const qreal pw = fm.horizontalAdvance(text) + 8;
                const QRectF r(right - pw, v.first - row_h / 2.0, pw, row_h);
                p.setPen(QPen(theme.color("border", QColor(80, 80, 80)), 1));
                p.setBrush(theme.color("surface", QColor(50, 50, 50)));
                p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);
                p.setPen(theme.color("text", fore));
                p.drawText(r, Qt::AlignCenter, text);
            }
        }
    }
    p.restore();
}

void Header::mouseDoubleClickEvent(QMouseEvent *event)
{
    assert(event);

    std::vector<Trace*> traces;

    _view.get_traces(ALL_VIEW, traces);

    if (event->button() & Qt::LeftButton) {
        _mouse_down_point = event->pos();

        // Save the offsets of any Traces which will be dragged
        for(auto t : traces){
            if (t->selected())
                _drag_traces.push_back(
                    make_pair(t, t->get_v_offset()));
        }

        // Select the Trace if it has been clicked
        for(auto t : traces){
            if (t->mouse_double_click(width(), event->pos()))
                break;
        }
    }

}

void Header::mousePressEvent(QMouseEvent *event)
{
	assert(event);

    _mouse_is_down = true;

    std::vector<Trace*> traces;
    _view.get_traces(ALL_VIEW, traces);
    int action;

    const bool instant = _view.session().is_instant();
    if (instant && _view.session().is_running_status()) {
        return;
    }

	if (event->button() & Qt::LeftButton) {
		_mouse_down_point = event->pos();

        // Save the offsets of any Traces which will be dragged
        for(auto t : traces){
            if (t->selected())
                _drag_traces.push_back(
                    make_pair(t, t->get_v_offset()));
        }

        // Select the Trace if it has been clicked
        const auto mTrace = get_mTrace(action, event->pos());
        if (action == Trace::COLOR && mTrace) {
            _colorFlag = true;
        }
        else if (action == Trace::NAME && mTrace) {
            _nameFlag = true;
        }
        else if (action == Trace::LABEL && mTrace) {
            mTrace->select(true);

            if (~QApplication::keyboardModifiers() & Qt::ControlModifier)
                _drag_traces.clear();
            
            _drag_traces.push_back(make_pair(mTrace, mTrace->get_zero_vpos()));
            mTrace->set_old_v_offset(mTrace->get_v_offset());
        }

        for(auto t : traces){
            if (t->signal_type() == SR_CHANNEL_LOGIC && _view.session().is_working()){
                // Disable set trigger from left pannel when capturing.
                break;
            }
            if (t->mouse_press(width(), event->pos()))
                break;
        }

        if (~QApplication::keyboardModifiers() & Qt::ControlModifier) {
            // Unselect all other Traces because the Ctrl is not
            // pressed
            for(auto t : traces){
                if (t != mTrace)
                    t->select(false);
            }
        }
        update();
    }
}

void Header::mouseReleaseEvent(QMouseEvent *event)
{
	assert(event);

    _mouse_is_down = false;

    // judge for color / name / trigger / move
    int action;
    const auto mTrace = get_mTrace(action, event->pos());

    if (mTrace){
        if (action == Trace::COLOR && _colorFlag) {
            _context_trace = mTrace;
            changeColor(event);
            _view.set_all_update(true);
        }
        else if (action == Trace::NAME && _nameFlag) {
            _context_trace = mTrace;
            changeName(event);
        }
    }

    // Make view index by Y value;
    int mode = _view.session().get_device()->get_work_mode();    
    if (_moveFlag && mode == LOGIC)
    {
        std::vector<Trace*> traces;

        for (auto s : _view.session().get_decode_signals()){
            traces.push_back(s);
        }

        for (auto s : _view.session().get_signals()){
            traces.push_back(s);
        }
    
        sort(traces.begin(), traces.end(), View::compare_trace_y);

        int index = 0;
        for (auto t : traces){
            t->set_view_index(index++);
        }
    }

    if (_moveFlag) {
        _drag_traces.clear();
        _view.signals_changed(mTrace);
        _view.set_all_update(true);

        std::vector<Trace*> traces;
        _view.get_traces(ALL_VIEW, traces);

        for(auto t : traces){
            t->select(false);
        }            
    }

    _colorFlag = false;
    _nameFlag = false;
    _moveFlag = false;

    _view.normalize_layout();
}

void Header::wheelEvent(QWheelEvent *event)
{
    assert(event);

    int x = 0;
    int y = 0;
    int delta = 0;
    bool isVertical = true;
    QPoint pos;
    (void)x;
    (void)y;
     
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    x = (int)event->position().x();
    y = (int)event->position().y();    
    int anglex = event->angleDelta().x();
    int angley = event->angleDelta().y();

    pos.setX(x);
    pos.setY(y);

    if (anglex == 0 || ABS_VAL(angley) >= ABS_VAL(anglex)){
        delta = angley;
        isVertical = true;
    }
    else{
        delta = anglex;
        isVertical = false; //hori direction
    }
#else
    x = event->x();
    delta = event->delta();
    isVertical = event->orientation() == Qt::Vertical;
    pos = event->pos(); 
#endif

    if (isVertical && (event->modifiers() & Qt::ControlModifier))
    {
        _view.v_zoom(delta > 0 ? 1.25 : 0.8, pos.y());
    }
    else if (isVertical)
    {
        std::vector<Trace*> traces;
        _view.get_traces(ALL_VIEW, traces);
        // Vertical scrolling
        double shift = 0;

#ifdef Q_OS_DARWIN
        static bool active = true;
        static int64_t last_time;
        if (event->source() == Qt::MouseEventSynthesizedBySystem)
        {
            if (active)
            {
                last_time = QDateTime::currentMSecsSinceEpoch();
                shift = delta > 1.5 ? -1 : delta < -1.5 ? 1 : 0;
            }
            int64_t cur_time = QDateTime::currentMSecsSinceEpoch();
            if (cur_time - last_time > 100)
                active = true;
            else
                active = false;
        }
        else
        {
            shift = -delta / 80.0;
        }
#else
        shift = delta / 80.0;
#endif

        bool used = false;
        for (auto t : traces)
        {
            if (t->mouse_wheel(width(), pos, shift)){
                used = true;
                break;
            }
        }

        // Nothing under the mouse takes the wheel: scroll the channels.
        if (!used && _view.v_scrollable())
            _view.v_scroll_by(-delta * _view.get_signalHeight() / 120);

        update();
    }
}

void Header::changeName(QMouseEvent *event)
{
    if (_context_trace != NULL
        && _context_trace->get_type() != SR_CHANNEL_DSO
        && event->button() == Qt::LeftButton) 
    {
        header_resize();
        QFont font = this->font();
        float fsize = AppConfig::Instance().appOptions.fontSize;
        font.setPointSizeF(fsize <= 10 ? fsize: 10);
        nameEdit->setFont(font);

        nameEdit->setText(_context_trace->get_name());
        nameEdit->selectAll();
        nameEdit->setFocus();
        nameEdit->show();
        header_updated();
    }
}

void Header::changeColor(QMouseEvent *event)
{
     if (_view.session().is_working() && _view.session().get_device()->get_work_mode() == ANALOG){
        //Disable to select color when working on analog mode.
        return;
    }

    if ((event->button() == Qt::LeftButton)) {
        const QColor new_color = QColorDialog::getColor(_context_trace->get_colour(), this, L_S(STR_PAGE_DLG, S_ID(IDS_DLG_SET_CHANNEL_COLOUR), "Set Channel Colour"));
        if (new_color.isValid())
            _context_trace->set_colour(new_color);
    }
}

void Header::mouseMoveEvent(QMouseEvent *event)
{
	assert(event);

    if (_view.session().is_working() && _view.session().get_device()->get_work_mode() == LOGIC){
        //Disable the hover status of trig button on left pannel.
        return;
    }

	_mouse_point = event->pos();

    // Move the Traces if we are dragging
    if (!_drag_traces.empty()) {
		const int delta = event->pos().y() - _mouse_down_point.y();

        for (auto i = _drag_traces.begin(); i != _drag_traces.end(); i++) {
            const auto t = (*i).first;
			if (t) {
                int y = (*i).second + delta;
                if (t->get_type() == SR_CHANNEL_DSO) {
                    DsoSignal *dsoSig = NULL;
                    if ((dsoSig = dynamic_cast<DsoSignal*>(t))) {
                        dsoSig->set_zero_vpos(y);
                        _moveFlag = true;
                        traces_moved();
                    }
                } else if (t->get_type() == SR_CHANNEL_MATH) {
                    MathTrace *mathTrace = NULL;
                    if ((mathTrace = dynamic_cast<MathTrace*>(t))) {
                       mathTrace->set_zero_vpos(y);
                       _moveFlag = true;
                       traces_moved();
                    }
                 } else if (t->get_type() == SR_CHANNEL_ANALOG) {
                    AnalogSignal *analogSig = NULL;
                    if ((analogSig = dynamic_cast<AnalogSignal*>(t))) {
                        analogSig->set_zero_vpos(y);
                        _moveFlag = true;
                        traces_moved();
                    }
                } else {
                    if (~QApplication::keyboardModifiers() & Qt::ControlModifier) {
                        const int y_snap =
                            ((y + View::SignalSnapGridSize / 2) /
                                View::SignalSnapGridSize) *
                                View::SignalSnapGridSize;
                        if (y_snap != t->get_v_offset()) {
                            _moveFlag = true;
                            t->set_v_offset(y_snap);
                        }
                    }
                }
			}
		}
	}
	update();
}

void Header::leaveEvent(QEvent*)
{
	_mouse_point = QPoint(-1, -1);
	update();
}

void Header::contextMenuEvent(QContextMenuEvent *event)
{
    (void)event;

    int action;

    const auto t = get_mTrace(action, _mouse_point);

    if (!t || !t->selected() || action != Trace::LABEL)
        return;
}

void Header::on_action_set_name_triggered()
{
    auto context_Trace = _context_trace;
    if (!context_Trace)
		return;

    if (nameEdit->isModified()) {
        QString v = nameEdit->text().trimmed();
        if (v == "")
            v = QString::number(context_Trace->get_index());
        
        _view.session().set_trace_name(context_Trace, v);
    }

    nameEdit->hide();
    header_updated();
}

void Header::header_resize()
{
    if (_context_trace) {
        const int y = _context_trace->get_y();
        nameEdit->move(QPoint(_context_trace->get_leftWidth(), y - nameEdit->height() / 2));
    }
}

void Header::UpdateLanguage()
{
    retranslateUi();
}

void Header::UpdateTheme()
{
    retranslateUi();
}

void Header::UpdateFont()
{
}


} // namespace view
} // namespace pv
