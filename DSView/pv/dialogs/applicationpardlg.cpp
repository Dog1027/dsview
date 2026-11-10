/*
 * This file is part of the DSView project.
 * DSView is based on PulseView.
 * 
 * Copyright (C) 2021 DreamSourceLab <support@dreamsourcelab.com>
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

#include "applicationpardlg.h"
#include "dsdialog.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QString>
#include <QFontDatabase>
#include <QGroupBox>
#include <QLabel>
#include <vector>
#include <QGridLayout>

#include "../config/appconfig.h"
#include "../ui/langresource.h"
#include "../appcontrol.h"
#include "../sigsession.h"
#include "../ui/dscombobox.h"
#include "../log.h"

namespace pv
{
namespace dialogs
{

ApplicationParamDlg::ApplicationParamDlg()
{
   
}

ApplicationParamDlg::~ApplicationParamDlg()
{
}

void ApplicationParamDlg::bind_font_name_list(QComboBox *box, QString v)
{   
    int selDex = -1;

    QString defName(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_DEFAULT_FONT), "Default"));
    box->addItem(defName);

    if (_font_name_list.size() == 0)
    {
        QFontDatabase fDataBase;
        _font_name_list = fDataBase.families();
    }
   
    for (QString family : _font_name_list) {
        if (family.indexOf("[") == -1)
        {
            box->addItem(family);

            if (selDex == -1 && family == v){
                selDex = box->count() - 1;
            }
        }
    }

    if (selDex == -1)
        selDex = 0;

    box->setCurrentIndex(selDex);
}

void ApplicationParamDlg::bind_font_size_list(QComboBox *box, float size)
{   
    int selDex = -1;

    float minSize = 0;
    float maxSize = 0;

    AppConfig::GetFontSizeRange(&minSize, &maxSize);

    for(int i=minSize; i<=maxSize; i++)
    {
        box->addItem(QString::number(i));
        if (i == size){
            selDex = box->count() - 1;
        }
    }
    if (selDex == -1)
        selDex = 2;
    box->setCurrentIndex(selDex);
}

bool ApplicationParamDlg::ShowDlg(QWidget *parent)
{
    DSDialog dlg(parent, true, true);
    dlg.setTitle(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_DISPLAY_OPTIONS), "Display options"));
    dlg.setMinimumWidth(360);

    QVBoxLayout *lay = new QVBoxLayout();
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(12);

    //show config
    AppConfig &app = AppConfig::Instance(); 

    // Every option is a check box with its text on the right, so all the
    // boxes line up on the left edge.
    QCheckBox *ck_quickScroll = new QCheckBox(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_QUICK_SCROLL), "Quick scroll"));
    ck_quickScroll->setChecked(app.appOptions.quickScroll);

    QCheckBox *ck_trigInMid = new QCheckBox(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_TRIG_DISPLAY_MIDDLE), "Tig pos in middle"));
    ck_trigInMid->setChecked(app.appOptions.trigPosDisplayInMid);

    QCheckBox *ck_profileBar = new QCheckBox(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_DISPLAY_PROFILE_IN_BAR), "Profile in bar"));
    ck_profileBar->setChecked(app.appOptions.displayProfileInBar);

    QCheckBox *ck_abortData = new QCheckBox(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_USE_ABORT_DATA_REPEAT), "Used abort data"));
    ck_abortData->setChecked(app.appOptions.swapBackBufferAlways);

    QCheckBox *ck_autoScrollLatestData = new QCheckBox(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_AUTO_SCROLL_LATEAST_DATA), "Auto scoll latest"));
    ck_autoScrollLatestData->setChecked(app.appOptions.autoScrollLatestData);

    QComboBox *ftCbSize = new DsComboBox();
    ftCbSize->setMinimumWidth(70);
    bind_font_size_list(ftCbSize, app.appOptions.fontSize);

    auto make_group = [&lay](const QString &title) {
        QGroupBox *group = new QGroupBox(title);
        QVBoxLayout *gl = new QVBoxLayout(group);
        gl->setContentsMargins(12, 10, 12, 10);
        gl->setSpacing(8);
        lay->addWidget(group);
        return gl;
    };

    QVBoxLayout *logicLay = make_group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_GROUP_LOGIC), "Logic"));
    logicLay->addWidget(ck_quickScroll);
    logicLay->addWidget(ck_abortData);
    logicLay->addWidget(ck_autoScrollLatestData);

    QVBoxLayout *dsoLay = make_group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_GROUP_DSO), "Scope"));
    dsoLay->addWidget(ck_trigInMid);

    QVBoxLayout *uiLay = make_group(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_GROUP_UI), "UI"));
    uiLay->addWidget(ck_profileBar);
    QHBoxLayout *fontRow = new QHBoxLayout();
    fontRow->addWidget(new QLabel(L_S(STR_PAGE_DLG, S_ID(IDS_DLG_FONT_SIZE), "Font size")));
    fontRow->addStretch(1);
    fontRow->addWidget(ftCbSize);
    uiLay->addLayout(fontRow);

    dlg.layout()->addLayout(lay);      
    dlg.exec();
    bool ret = dlg.IsClickYes();

    //save config
    if (ret){

        bool bAppChanged = false;
        bool bFontChanged = false;
        float fSize = ftCbSize->currentText().toFloat();

        if (app.appOptions.quickScroll != ck_quickScroll->isChecked()){
            app.appOptions.quickScroll = ck_quickScroll->isChecked();
            bAppChanged = true;
        }       
        if (app.appOptions.trigPosDisplayInMid != ck_trigInMid->isChecked()){
            app.appOptions.trigPosDisplayInMid = ck_trigInMid->isChecked();
            bAppChanged = true;
        }
        if (app.appOptions.displayProfileInBar != ck_profileBar->isChecked()){
            app.appOptions.displayProfileInBar = ck_profileBar->isChecked();
            bAppChanged = true;
        }
        if (app.appOptions.swapBackBufferAlways != ck_abortData->isChecked()){
            app.appOptions.swapBackBufferAlways = ck_abortData->isChecked();
            bAppChanged = true;
        }        
        if (app.appOptions.fontSize != fSize){
            app.appOptions.fontSize = fSize;
            bFontChanged = true;
        }
        if (app.appOptions.autoScrollLatestData != ck_autoScrollLatestData->isChecked()){
            app.appOptions.autoScrollLatestData = ck_autoScrollLatestData->isChecked();
            bAppChanged = true;
        }
 
        if (bAppChanged){
            app.SaveApp();
            AppControl::Instance()->GetSession()->broadcast_msg(DSV_MSG_APP_OPTIONS_CHANGED);
        }
        
        if (bFontChanged){
            if (!bAppChanged){
                app.SaveApp();
            }
            AppControl::Instance()->GetSession()->broadcast_msg(DSV_MSG_FONT_OPTIONS_CHANGED);
        }
    }
   
   return ret;
}
 

} //
}//

