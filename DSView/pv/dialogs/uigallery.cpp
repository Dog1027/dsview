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

#include "uigallery.h"

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QEventLoop>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>

#include "dsdialog.h"
#include "dsmessagebox.h"
#include "../config/appconfig.h"
#include "../ui/dscombobox.h"
#include "../ui/fn.h"
#include "../ui/theme.h"
#include "../ui/uimanager.h"
#include "../log.h"

namespace pv {
namespace dialogs {

namespace {

UiGallery *g_gallery = NULL;

// Run the event loop for a while, so deferred layout / polish work is done.
void settle(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

// A titled card; the page is made of these.
QGroupBox* card(const QString &title, QBoxLayout **lay)
{
    QGroupBox *box = new QGroupBox(title);
    QVBoxLayout *v = new QVBoxLayout(box);
    v->setContentsMargins(12, 10, 12, 12);
    v->setSpacing(10);
    *lay = v;
    return box;
}

QHBoxLayout* row(QBoxLayout *parent)
{
    QHBoxLayout *h = new QHBoxLayout();
    h->setSpacing(8);
    parent->addLayout(h);
    return h;
}

// A real dialog, shown inside the page instead of as a window.
void embed(QWidget *dialog, QBoxLayout *lay)
{
    dialog->setParent(NULL);
    dialog->setWindowFlags(Qt::Widget);
    lay->addWidget(dialog, 0, Qt::AlignLeft);
    dialog->show();
}

} // namespace

UiGallery::UiGallery(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("DSView UI gallery");
    setObjectName("ui_gallery");
    setAttribute(Qt::WA_StyledBackground, true);

    _app_style = AppConfig::Instance().frameOptions.style;
    if (_app_style.isEmpty())
        _app_style = THEME_STYLE_DARK;
    _style = _app_style;

    QLabel *title = new QLabel("UI gallery");
    QFont tf = title->font();
    tf.setPointSizeF(tf.pointSizeF() + 3);
    tf.setWeight(QFont::DemiBold);
    title->setFont(tf);

    _dark_button = new QPushButton("Dark");
    _light_button = new QPushButton("Light");
    QButtonGroup *group = new QButtonGroup(this);
    for (QPushButton *b : {_dark_button, _light_button}){
        b->setCheckable(true);
        group->addButton(b);
    }
    (_style == THEME_STYLE_LIGHT ? _light_button : _dark_button)->setChecked(true);
    connect(_dark_button, &QPushButton::clicked, this, [this]() { set_style(THEME_STYLE_DARK); });
    connect(_light_button, &QPushButton::clicked, this, [this]() { set_style(THEME_STYLE_LIGHT); });

    QLabel *note = new QLabel(ui::Theme::Instance().is_live()
                              ? "Live: rebuilt on every save of a theme file"
                              : "Start with DSVIEW_THEME_DIR for live editing");
    note->setObjectName("hint");

    QHBoxLayout *head = new QHBoxLayout();
    head->setContentsMargins(16, 12, 16, 8);
    head->setSpacing(8);
    head->addWidget(title);
    head->addSpacing(16);
    head->addWidget(_dark_button);
    head->addWidget(_light_button);
    head->addStretch(1);
    head->addWidget(note);

    _scroll = new QScrollArea();
    _scroll->setWidgetResizable(true);
    _scroll->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addLayout(head);
    lay->addWidget(_scroll, 1);

    // Live editing: the main window applies the new stylesheet, then the
    // page is built again (embedded dialogs read fonts / sizes once).
    connect(&ui::Theme::Instance(), &ui::Theme::changed, this, [this]() {
        QTimer::singleShot(150, this, [this]() { rebuild(); });
    });

    rebuild();
}

UiGallery::~UiGallery()
{
    if (g_gallery == this)
        g_gallery = NULL;
}

void UiGallery::Open()
{
    if (g_gallery == NULL){
        g_gallery = new UiGallery();
        g_gallery->setAttribute(Qt::WA_DeleteOnClose);
        g_gallery->resize(1040, 820);
    }
    g_gallery->show();
    g_gallery->raise();
    g_gallery->activateWindow();
}

void UiGallery::Shoot(const QString &dir)
{
    QDir().mkpath(dir);

    UiGallery *g = new UiGallery();
    g->resize(1040, 820);
    g->show();

    for (const QString style : {QString(THEME_STYLE_DARK), QString(THEME_STYLE_LIGHT)}){
        g->set_style(style);
        settle(400);
        QString path = QDir(dir).absoluteFilePath("gallery-" + style + ".png");
        if (g->_scroll->widget()->grab().save(path)){
            dsv_info("UI gallery: saved %s", path.toUtf8().data());
            printf("%s\n", path.toUtf8().data());
        }
        else{
            dsv_err("UI gallery: can't save %s", path.toUtf8().data());
        }
    }

    g->close();
    delete g;
    QTimer::singleShot(0, qApp, &QApplication::quit);
}

void UiGallery::closeEvent(QCloseEvent *event)
{
    // Back to the theme the app had; the gallery only previews.
    if (_style != _app_style)
        set_style(_app_style);
    QWidget::closeEvent(event);
}

void UiGallery::set_style(const QString &style)
{
    _style = style;
    (_style == THEME_STYLE_LIGHT ? _light_button : _dark_button)->setChecked(true);

    // Not saved: AppConfig only tells the icon lookups which set to use.
    AppConfig::Instance().frameOptions.style = style;
    qApp->setStyleSheet(ui::Theme::Instance().build_style_sheet(style));
    UiManager::Instance()->Update(UI_UPDATE_ACTION_THEME);
    UiManager::Instance()->Update(UI_UPDATE_ACTION_FONT);

    rebuild();
}

void UiGallery::rebuild()
{
    QWidget *old = _scroll->takeWidget();
    if (old != NULL)
        old->deleteLater();

    QWidget *content = build_content();

    QFont font = this->font();
    font.setPointSizeF(AppConfig::Instance().appOptions.fontSize);
    content->setFont(font);
    ::ui::set_form_font(content, font);

    _scroll->setWidget(content);
}

QWidget* UiGallery::build_content()
{
    const QString icons = GetIconPath();
    QWidget *page = new QWidget();
    page->setObjectName("ui_gallery_page");

    QGridLayout *grid = new QGridLayout(page);
    grid->setContentsMargins(16, 8, 16, 16);
    grid->setSpacing(16);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);

    QBoxLayout *lay;
    QHBoxLayout *h;

    // ---- Buttons
    QGroupBox *buttons = card("Buttons", &lay);
    h = row(lay);
    h->addWidget(new QPushButton("Button"));
    QPushButton *def = new QPushButton("Default");
    def->setDefault(true);
    h->addWidget(def);
    QPushButton *off = new QPushButton("Disabled");
    off->setEnabled(false);
    h->addWidget(off);
    QPushButton *chk = new QPushButton("Checked");
    chk->setCheckable(true);
    chk->setChecked(true);
    h->addWidget(chk);
    h->addStretch(1);

    h = row(lay);
    for (const char *name : {"params.svg", "search.svg", "del.svg", "add.svg"}){
        QToolButton *tb = new QToolButton();
        tb->setIcon(QIcon(icons + "/" + name));
        tb->setToolTip(name);
        h->addWidget(tb);
    }
    QToolButton *withText = new QToolButton();
    withText->setIcon(QIcon(icons + "/file.svg"));
    withText->setText("Tool button");
    withText->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    h->addWidget(withText);
    h->addStretch(1);

    lay->addWidget(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    lay->addWidget(new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Help));
    grid->addWidget(buttons, 0, 0);

    // ---- Inputs
    QGroupBox *inputs = card("Inputs", &lay);
    QFormLayout *form = new QFormLayout();
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    lay->addLayout(form);
    form->addRow("Line edit", new QLineEdit("Some text"));
    QLineEdit *ph = new QLineEdit();
    ph->setPlaceholderText("Placeholder");
    form->addRow("Placeholder", ph);
    QLineEdit *dis = new QLineEdit("Disabled");
    dis->setEnabled(false);
    form->addRow("Disabled", dis);
    DsComboBox *combo = new DsComboBox();
    combo->addItems({"1 MHz", "10 MHz", "100 MHz"});
    form->addRow("Combo box", combo);
    DsComboBox *combo2 = new DsComboBox();
    combo2->addItem("Disabled");
    combo2->setEnabled(false);
    form->addRow("Disabled combo", combo2);
    QSpinBox *spin = new QSpinBox();
    spin->setRange(0, 100);
    spin->setValue(42);
    form->addRow("Spin box", spin);
    QDoubleSpinBox *dspin = new QDoubleSpinBox();
    dspin->setSuffix(" V");
    dspin->setValue(1.8);
    form->addRow("Double spin", dspin);
    grid->addWidget(inputs, 0, 1);

    // ---- Choices
    QGroupBox *choices = card("Choices", &lay);
    h = row(lay);
    QCheckBox *c1 = new QCheckBox("Checked");
    c1->setChecked(true);
    h->addWidget(c1);
    h->addWidget(new QCheckBox("Unchecked"));
    QCheckBox *c3 = new QCheckBox("Partly");
    c3->setTristate(true);
    c3->setCheckState(Qt::PartiallyChecked);
    h->addWidget(c3);
    QCheckBox *c4 = new QCheckBox("Disabled");
    c4->setEnabled(false);
    h->addWidget(c4);
    h->addStretch(1);
    h = row(lay);
    QRadioButton *r1 = new QRadioButton("Option A");
    r1->setChecked(true);
    h->addWidget(r1);
    h->addWidget(new QRadioButton("Option B"));
    QRadioButton *r3 = new QRadioButton("Disabled");
    r3->setEnabled(false);
    h->addWidget(r3);
    h->addStretch(1);
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setValue(30);
    lay->addWidget(slider);
    QProgressBar *progress = new QProgressBar();
    progress->setValue(60);
    lay->addWidget(progress);
    grid->addWidget(choices, 1, 0);

    // ---- Text
    QGroupBox *text = card("Text", &lay);
    QLabel *heading = new QLabel("Heading");
    QFont hf = heading->font();
    hf.setWeight(QFont::DemiBold);
    heading->setFont(hf);
    lay->addWidget(heading);
    lay->addWidget(new QLabel("Normal label text"));
    QLabel *muted = new QLabel("Muted hint text (QLabel#hint)");
    muted->setObjectName("hint");
    lay->addWidget(muted);
    QLabel *disabled = new QLabel("Disabled label");
    disabled->setEnabled(false);
    lay->addWidget(disabled);
    QTextEdit *edit = new QTextEdit("Text edit\nwith two lines");
    edit->setFixedHeight(64);
    lay->addWidget(edit);
    grid->addWidget(text, 1, 1);

    // ---- Tabs, table, list
    QGroupBox *views = card("Tabs, table, list", &lay);
    QTabWidget *tabs = new QTabWidget();
    QTableWidget *table = new QTableWidget(4, 3);
    table->setHorizontalHeaderLabels({"Time", "Type", "Data"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    const char *rows[][3] = {{"0.000 ms", "Start", ""}, {"0.012 ms", "Address", "0x50"},
                             {"0.104 ms", "Data", "0xA5"}, {"0.196 ms", "Stop", ""}};
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 3; c++)
            table->setItem(r, c, new QTableWidgetItem(rows[r][c]));
    table->selectRow(1);
    table->setFixedHeight(150);
    tabs->addTab(table, "Table");
    QListWidget *list = new QListWidget();
    list->addItems({"I²C", "SPI", "UART", "CAN"});
    list->setCurrentRow(2);
    tabs->addTab(list, "List");
    QWidget *disabledTab = new QWidget();
    tabs->addTab(disabledTab, "Disabled");
    tabs->setTabEnabled(2, false);
    lay->addWidget(tabs);
    grid->addWidget(views, 2, 0);

    // ---- Menu (a picture of it: a real menu is a popup window)
    QGroupBox *menus = card("Menu", &lay);
    QMenu menu;
    menu.addAction(QIcon(icons + "/open.svg"), "Open...");
    menu.addAction(QIcon(icons + "/save.svg"), "Save");
    menu.addSeparator();
    QAction *checked = menu.addAction("Checked item");
    checked->setCheckable(true);
    checked->setChecked(true);
    QAction *grey = menu.addAction("Disabled item");
    grey->setEnabled(false);
    menu.addMenu("Submenu")->addAction("Item");
    menu.ensurePolished();
    menu.adjustSize();
    QLabel *menuPicture = new QLabel();
    menuPicture->setPixmap(menu.grab());
    lay->addWidget(menuPicture, 0, Qt::AlignLeft);
    lay->addStretch(1);
    grid->addWidget(menus, 2, 1);

    // ---- Message boxes and a dialog: the real classes, embedded
    QGroupBox *dialogs = card("Message boxes and dialog", &lay);
    QHBoxLayout *dh = row(lay);
    QVBoxLayout *dcol = new QVBoxLayout();
    dcol->setSpacing(8);
    dh->addLayout(dcol);

    DSMessageBox *confirm = new DSMessageBox(NULL, "Confirm");
    confirm->mBox()->setText("Save captured data?");
    confirm->mBox()->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirm->prepare();
    embed(confirm, dcol);

    DSMessageBox *error = new DSMessageBox(NULL, "Error");
    error->mBox()->setText("Failed to start capture.");
    error->mBox()->setInformativeText("The device did not answer. Check the USB cable and try again.");
    error->mBox()->setStandardButtons(QMessageBox::Ok);
    error->prepare();
    embed(error, dcol);
    dcol->addStretch(1);

    DSDialog *dialog = new DSDialog(NULL, true, false);
    dialog->setTitle("Dialog");
    QGroupBox *group = new QGroupBox("Group");
    QFormLayout *gform = new QFormLayout(group);
    gform->setContentsMargins(12, 10, 12, 10);
    gform->setHorizontalSpacing(12);
    gform->setVerticalSpacing(8);
    gform->addRow("Name", new QLineEdit("Channel 0"));
    DsComboBox *gcombo = new DsComboBox();
    gcombo->addItems({"Rising edge", "Falling edge"});
    gform->addRow("Edge", gcombo);
    gform->addRow("", new QCheckBox("Enabled"));
    dialog->layout()->addWidget(group);
    dialog->layout()->addWidget(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
    dialog->update_font();
    QVBoxLayout *dcol2 = new QVBoxLayout();
    dh->addLayout(dcol2);
    embed(dialog, dcol2);
    dcol2->addStretch(1);
    dh->addStretch(1);
    grid->addWidget(dialogs, 3, 0, 1, 2);

    // ---- Palette
    QGroupBox *palette = card("Palette (" + _style + ".palette)", &lay);
    QGridLayout *pg = new QGridLayout();
    pg->setHorizontalSpacing(12);
    pg->setVerticalSpacing(6);
    lay->addLayout(pg);
    const QStringList names = ui::Theme::Instance().color_names();
    const int perColumn = (names.size() + 2) / 3;
    for (int i = 0; i < names.size(); i++){
        QColor c = ui::Theme::Instance().color(names[i]);
        QLabel *swatch = new QLabel();
        swatch->setFixedSize(36, 20);
        swatch->setStyleSheet(QString("background-color:%1; border:1px solid rgba(128,128,128,0.5); border-radius:4px;")
                              .arg(c.name(QColor::HexArgb)));
        QLabel *name = new QLabel("@" + names[i]);
        QLabel *value = new QLabel(c.name());
        value->setObjectName("hint");
        const int r = i % perColumn, col = (i / perColumn) * 4;
        pg->addWidget(swatch, r, col);
        pg->addWidget(name, r, col + 1);
        pg->addWidget(value, r, col + 2);
        pg->setColumnMinimumWidth(col + 3, 16);
    }
    pg->setColumnStretch(11, 1);
    grid->addWidget(palette, 4, 0, 1, 2);

    // ---- Icons
    QStringList files = QDir(icons).entryList({"*.svg", "*.png", "*.gif"}, QDir::Files, QDir::Name);
    QGroupBox *iconCard = card(QString("Icons (%1, %2)").arg(icons).arg(files.size()), &lay);
    QGridLayout *ig = new QGridLayout();
    ig->setHorizontalSpacing(4);
    ig->setVerticalSpacing(10);
    lay->addLayout(ig);
    const int perRow = 10;
    for (int i = 0; i < files.size(); i++){
        QVBoxLayout *cell = new QVBoxLayout();
        cell->setSpacing(2);
        QLabel *pic = new QLabel();
        pic->setPixmap(QIcon(icons + "/" + files[i]).pixmap(24, 24));
        pic->setAlignment(Qt::AlignHCenter);
        QLabel *name = new QLabel(QFileInfo(files[i]).completeBaseName());
        name->setObjectName("hint");
        name->setAlignment(Qt::AlignHCenter);
        cell->addWidget(pic);
        cell->addWidget(name);
        ig->addLayout(cell, i / perRow, i % perRow);
    }
    for (int c = 0; c < perRow; c++)
        ig->setColumnStretch(c, 1);
    grid->addWidget(iconCard, 5, 0, 1, 2);

    grid->setRowStretch(6, 1);
    return page;
}

} // namespace dialogs
} // namespace pv
