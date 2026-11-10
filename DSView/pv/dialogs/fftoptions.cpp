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

#include <QGroupBox>
#include "../ui/fn.h"
#include "fftoptions.h"
#include <QFormLayout>
#include <QListWidget>
#include "../sigsession.h"
#include "../data/spectrumstack.h"
#include "../view/trace.h"
#include "../view/dsosignal.h"
#include "../view/spectrumtrace.h"
#include "../dsvdef.h"
#include "../log.h"

#include "../ui/langresource.h"


using namespace boost;
using namespace std;
 
namespace pv {
namespace dialogs {

FftOptions::FftOptions(QWidget *parent, SigSession *session) :
    DSDialog(parent),
    _session(session),
    _button_box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this)
{
    _len_combobox = NULL;
    _interval_combobox = NULL;
    _en_checkbox = NULL;
    _ch_combobox = NULL;
    _window_combobox = NULL;
    _dc_checkbox = NULL;
    _view_combobox = NULL;
    _dbv_combobox = NULL;
    _hint_label = NULL;
    _glayout = NULL;
    _layout = NULL;

    _en_checkbox = new QCheckBox(this);
    _len_combobox = new DsComboBox(this);
    _interval_combobox = new DsComboBox(this);
    _ch_combobox = new DsComboBox(this);
    _window_combobox = new DsComboBox(this);
    _dc_checkbox = new QCheckBox(this);
    _dc_checkbox->setChecked(true);
    _view_combobox = new DsComboBox(this);
    _dbv_combobox = new DsComboBox(this);
 
    // setup _ch_combobox
    for(auto s : _session->get_signals()) {
        if (s->signal_type() == SR_CHANNEL_DSO) {
            view::DsoSignal *dsoSig = (view::DsoSignal*)s;
            _ch_combobox->addItem(dsoSig->get_name(), QVariant::fromValue(dsoSig->get_index()));
        }
    }

    // setup _window_combobox _len_combobox
    _sample_limit = 0;
    
    if (_session->get_device()->get_config_uint64(SR_CONF_MAX_DSO_SAMPLELIMITS, _sample_limit)) {
        _sample_limit = _sample_limit * 0.5;
    }
    else {
        dsv_err("ERROR: config_get SR_CONF_MAX_DSO_SAMPLELIMITS failed.");
    }

    std::vector<QString> windows;
    std::vector<uint64_t> length;
    std::vector<QString> view_modes;
    std::vector<int> dbv_ranges;

    for(auto t : _session->get_spectrum_traces()) {
        view::SpectrumTrace *spectrumTraces = NULL;
        if ((spectrumTraces = dynamic_cast<view::SpectrumTrace*>(t))) {
            windows = spectrumTraces->get_windows_support();
            length = spectrumTraces->get_length_support();
            view_modes = spectrumTraces->get_view_modes_support();
            dbv_ranges = spectrumTraces->get_dbv_ranges();
            break;
        }
    }

    assert(windows.size() > 0);
    assert(length.size() > 0);
    assert(view_modes.size() > 0);
    assert(dbv_ranges.size() > 0);

    for (unsigned int i = 0; i < windows.size(); i++)
    {
        _window_combobox->addItem(windows[i],
            QVariant::fromValue(i));
    }

    for (unsigned int i = 0; i < length.size(); i++)
    {
        if (length[i] < _sample_limit)
            _len_combobox->addItem(QString::number(length[i]),
                QVariant::fromValue(length[i]));
        else
            break;
    }

    assert(_len_combobox->count() > 0);
    _len_combobox->setCurrentIndex(_len_combobox->count()-1);

    const int max_interval = _sample_limit/_len_combobox->currentData().toLongLong();
    for (int i = 1; i <= max_interval; i*=2)
    {
        _interval_combobox->addItem(QString::number(i),
            QVariant::fromValue(i));
    }
    for (unsigned int i = 0; i < view_modes.size(); i++)
    {
        _view_combobox->addItem(view_modes[i],
            QVariant::fromValue(i));
    }
    assert(_view_combobox->count() > 0);
    _view_combobox->setCurrentIndex(_view_combobox->count()-1);
    for (unsigned int i = 0; i < dbv_ranges.size(); i++)
    {
        _dbv_combobox->addItem(QString::number(dbv_ranges[i]),
            QVariant::fromValue(dbv_ranges[i]));
    }

    // load current settings
    for(auto t : _session->get_spectrum_traces()) {
         view::SpectrumTrace *spectrumTraces = NULL;
        if ((spectrumTraces = dynamic_cast<view::SpectrumTrace*>(t))) {
            if (spectrumTraces->enabled()) {
                _en_checkbox->setChecked(true);
                for (int i = 0; i < _ch_combobox->count(); i++) {
                    if (spectrumTraces->get_index() == _ch_combobox->itemData(i).toInt()) {
                        _ch_combobox->setCurrentIndex(i);
                        break;
                    }
                }
                for (int i = 0; i < _len_combobox->count(); i++) {
                    if (spectrumTraces->get_spectrum_stack()->get_sample_num() == _len_combobox->itemData(i).toULongLong()) {
                        _len_combobox->setCurrentIndex(i);
                        break;
                    }
                }
                _interval_combobox->clear();
                const int max_interval = _sample_limit/_len_combobox->currentData().toLongLong();
                for (int i = 1; i <= max_interval; i*=2)
                {
                    _interval_combobox->addItem(QString::number(i),
                        QVariant::fromValue(i));
                }
                for (int i = 0; i < _interval_combobox->count(); i++) {
                    if (spectrumTraces->get_spectrum_stack()->get_sample_interval() == _interval_combobox->itemData(i).toInt()) {
                        _interval_combobox->setCurrentIndex(i);
                        break;
                    }
                }
                for (int i = 0; i < _dbv_combobox->count(); i++) {
                    if (spectrumTraces->dbv_range() == _dbv_combobox->itemData(i).toLongLong()) {
                        _dbv_combobox->setCurrentIndex(i);
                        break;
                    }
                }
                _window_combobox->setCurrentIndex(spectrumTraces->get_spectrum_stack()->get_windows_index());
                _dc_checkbox->setChecked(spectrumTraces->get_spectrum_stack()->dc_ignored());
                _view_combobox->setCurrentIndex(spectrumTraces->view_mode());
            }
        }
    }

    _hint_label = new QLabel(this);
    QString hint_pic= ":/icons/" + _window_combobox->currentText()+".png";
    QPixmap pixmap(hint_pic);
    _hint_label->setPixmap(pixmap);

    // Groups with a title, a form inside, as in the other dialogs:
    // Spectrum and Display on the left, Window (with its shape) right.
    auto label = [](const char *id, const char *def) {
        return ::ui::label_text(L_S(STR_PAGE_DLG, id, def));
    };
    auto group = [this](const QString &title) {
        QGroupBox *box = new QGroupBox(title, this);
        QFormLayout *form = new QFormLayout(box);
        form->setContentsMargins(12, 10, 12, 12);
        form->setHorizontalSpacing(12);
        form->setVerticalSpacing(8);
        form->setLabelAlignment(Qt::AlignLeft);
        return std::make_pair(box, form);
    };

    auto spectrum = group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_FFT_GROUP_SPECTRUM), "Spectrum"));
    spectrum.second->addRow(label(S_ID(IDS_DLG_FFT_ENABLE), "FFT Enable: "), _en_checkbox);
    spectrum.second->addRow(label(S_ID(IDS_DLG_FFT_SOURCE), "FFT Source: "), _ch_combobox);
    spectrum.second->addRow(label(S_ID(IDS_DLG_FFT_LENGTH), "FFT Length: "), _len_combobox);
    spectrum.second->addRow(label(S_ID(IDS_DLG_SAMPLE_INTERVAL), "Sample Interval: "), _interval_combobox);

    auto display = group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_FFT_GROUP_DISPLAY), "Display"));
    display.second->addRow(label(S_ID(IDS_DLG_Y-AXIS_MODE), "Y-axis Mode: "), _view_combobox);
    display.second->addRow(label(S_ID(IDS_DLG_DBV_RANGE), "DBV Range: "), _dbv_combobox);

    auto window = group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_FFT_GROUP_WINDOW), "Window"));
    window.second->addRow(label(S_ID(IDS_DLG_FFT_WINDOW), "FFT Window: "), _window_combobox);
    window.second->addRow(label(S_ID(IDS_DLG_DC_IGNORED), "DC Ignored: "), _dc_checkbox);
    _hint_label->setAlignment(Qt::AlignCenter);
    window.second->addRow(_hint_label);

    QVBoxLayout *left = new QVBoxLayout();
    left->setSpacing(12);
    left->addWidget(spectrum.first);
    left->addWidget(display.first);
    left->addStretch(1);

    QVBoxLayout *right = new QVBoxLayout();
    right->setSpacing(12);
    right->addWidget(window.first);
    right->addStretch(1);

    _glayout = new QGridLayout();
    _glayout->setContentsMargins(0, 0, 0, 0);
    _glayout->setHorizontalSpacing(12);
    _glayout->addLayout(left, 0, 0);
    _glayout->addLayout(right, 0, 1);

    _layout = new QVBoxLayout();
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setSpacing(12);
    _layout->addLayout(_glayout);
    _layout->addWidget(&_button_box);

    layout()->addLayout(_layout);
    setTitle(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_FFT_OPTIONS), "FFT Options"));

    connect(&_button_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(&_button_box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_window_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(window_changed(int)));
    connect(_len_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(len_changed(int)));
    connect(_session->device_event_object(), SIGNAL(device_updated()), this, SLOT(reject()));
}

FftOptions::~FftOptions(){

}

void FftOptions::accept()
{
    using namespace Qt;

    QDialog::accept();

   for(auto t : _session->get_spectrum_traces()) {
        view::SpectrumTrace *spectrumTraces = NULL;
        if ((spectrumTraces = dynamic_cast<view::SpectrumTrace*>(t))) {
            spectrumTraces->set_enable(false);
            if (spectrumTraces->get_index() == _ch_combobox->currentData().toInt()) {
                spectrumTraces->get_spectrum_stack()->set_dc_ignore(_dc_checkbox->isChecked());
                spectrumTraces->get_spectrum_stack()->set_sample_num(_len_combobox->currentData().toULongLong());
                spectrumTraces->get_spectrum_stack()->set_sample_interval(_interval_combobox->currentData().toInt());
                spectrumTraces->get_spectrum_stack()->set_windows_index(_window_combobox->currentData().toInt());
                spectrumTraces->set_view_mode(_view_combobox->currentData().toUInt());
                
                spectrumTraces->set_dbv_range(_dbv_combobox->currentData().toInt());
                spectrumTraces->set_enable(_en_checkbox->isChecked());

                if (_session->is_stopped_status() && spectrumTraces->enabled()){
                    spectrumTraces->get_spectrum_stack()->calc_fft();
                }
            }
        }
    }
    _session->spectrum_rebuild();
}

void FftOptions::reject()
{
    using namespace Qt;

    QDialog::reject();
}

void FftOptions::window_changed(int index)
{
    QString str = _window_combobox->itemText(index);
    QString hint_pic= ":/icons/" + str +".png";
    QPixmap pixmap(hint_pic);
    _hint_label->setPixmap(pixmap);
}

void FftOptions::len_changed(int index)
{
    int pre_index = _interval_combobox->currentIndex();
    _interval_combobox->clear();
    const int max_interval = _sample_limit/_len_combobox->itemData(index).toLongLong();
    for (int i = 1; i <= max_interval; i*=2)
    {
        _interval_combobox->addItem(QString::number(i),
            QVariant::fromValue(i));
    }
    
    if (pre_index >= _interval_combobox->count()){
        pre_index = 0;
    }
    _interval_combobox->setCurrentIndex(pre_index);
}

} // namespace dialogs
} // namespace pv
