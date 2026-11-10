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

#include "appstatusbar.h"

#include <QLabel>
#include <QDateTime>
#include <QStyle>
#include <cmath>

#include "../sigsession.h"
#include "../deviceagent.h"

namespace pv {
namespace ui {

namespace {

// 1234567 -> "1.23 MHz" (or "1.23M" with an empty unit): SI prefix and
// up to 3 significant digits.
QString si(double v, const QString &unit)
{
    static const char *prefixes[] = {"", "k", "M", "G", "T"};
    int i = 0;
    while (std::fabs(v) >= 1000.0 && i < 4){
        v /= 1000.0;
        i++;
    }
    if (unit.isEmpty())
        return QString::number(v, 'g', 3) + prefixes[i];
    return QString::number(v, 'g', 3) + " " + prefixes[i] + unit;
}

// Seconds -> "1.05 s", "250 ms", "12.5 us", ...
QString duration(double t)
{
    static const char *units[] = {"s", "ms", "us", "ns"};
    int i = 0;
    while (t != 0 && std::fabs(t) < 1.0 && i < 3){
        t *= 1000.0;
        i++;
    }
    return QString::number(t, 'g', 3) + " " + units[i];
}

} // namespace

AppStatusBar::AppStatusBar(SigSession *session, QWidget *parent)
    : QStatusBar(parent)
{
    _session = session;

    setObjectName("app_status_bar");
    setSizeGripEnabled(false);   // the frameless window resizes at its border

    _device  = add_label("status_device", false);
    _mode    = add_label("status_mode", false);
    _state   = add_label("status_state", false);
    _trigger = add_label("status_trigger", true);
    _data    = add_label("status_data", true);

    connect(&_timer, &QTimer::timeout, this, [this]() { refresh(); });
    _timer.start(300);
    refresh();
}

QLabel* AppStatusBar::add_label(const QString &name, bool permanent)
{
    QLabel *label = new QLabel(this);
    label->setObjectName(name);
    if (permanent)
        addPermanentWidget(label);
    else
        addWidget(label);
    return label;
}

void AppStatusBar::set_text(QLabel *label, const QString &text)
{
    if (label->text() != text)
        label->setText(text);
    label->setVisible(!text.isEmpty());
}

void AppStatusBar::refresh()
{
    DeviceAgent *dev = _session->get_device();

    if (dev == NULL || !dev->have_instance()){
        set_text(_device, tr("No device"));
        set_text(_mode, "");
        set_text(_state, "");
        set_text(_data, "");
        set_text(_trigger, "");
        return;
    }

    set_text(_device, dev->name());

    int mode = dev->get_work_mode();
    QString modeName = mode == DSO ? tr("Oscilloscope")
                     : mode == ANALOG ? tr("Data Acquisition")
                     : tr("Logic Analyzer");
    set_text(_mode, modeName);

    // State. is_triged() becomes true once data starts to arrive.
    QString state;
    if (_session->is_working()){
        if (_session->is_running_status() && !_session->is_triged()
            && !_session->is_instant() && mode != ANALOG)
            state = tr("Waiting for trigger...");
        else
            state = tr("Capturing...");
        if (_session->is_repeat_mode())
            state += " " + tr("(repeat)");
        else if (_session->is_loop_mode())
            state += " " + tr("(loop)");
    }
    else {
        state = tr("Ready");
    }
    set_text(_state, state);
    bool working = _session->is_working();
    if (_state->property("working").toBool() != working){
        _state->setProperty("working", working);
        _state->style()->unpolish(_state);   // re-apply the [working] rule
        _state->style()->polish(_state);
    }

    // The data on screen
    QString data;
    QString trigger;
    if (_session->have_view_data()){
        uint64_t rate = _session->cur_snap_samplerate();
        uint64_t samples = _session->cur_samplelimits();
        if (rate > 0 && samples > 0){
            data = si((double)samples, "") + " " + tr("samples") + "   "
                 + duration((double)samples / rate) + " @ " + si((double)rate, "Hz");
        }
        if (_session->is_triged() && _session->get_trig_time().isValid())
            trigger = tr("Captured at") + " " + _session->get_trig_time().toString("HH:mm:ss");
    }
    set_text(_data, data);
    set_text(_trigger, trigger);
}

} // namespace ui
} // namespace pv
