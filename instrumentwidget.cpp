/******************************************************************************

Copyright 2011-2013 Martijn van der Kwast <martijn@vdkwast.com>

This file is part of FP4-Manager

FP4-Manager is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

FP4-Manager is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Foobar. If not, see http://www.gnu.org/licenses/.

******************************************************************************/

#include "instrumentwidget.h"
#include "fp4instr.h"
#include "fp4qt.h"
#include "preferences.h"
#include "fp4managerapplication.h"

#include <QtGui>

QList<int> InstrumentWidget::s_favourites;
QList< InstrumentWidget*> InstrumentWidget::s_instrumentWidgets;

AbstractBankButton::AbstractBankButton(unsigned bank, InstrumentWidget *instrumentWidget, QWidget *parent) :
    QPushButton(parent),
    m_instrumentWidget(instrumentWidget),
    m_bank(bank)
{
    // make button look "active" if current instrument in InstrumentWidget matches this bank.
    connect(m_instrumentWidget, SIGNAL(instrumentChanged(uint)), SLOT(onInstrumentChanged(uint)));
}

QSize AbstractBankButton::sizeHint() const {
    QFontMetrics metrics = fontMetrics();
    return QSize(10, metrics.height()*2);
}

// called when instrument changes in instrument widget
void AbstractBankButton::onInstrumentChanged(unsigned int instrument) {
    if (instrumentMatchesBank(instrument, m_bank)) {
        QFont font;
        font.setBold(true);
        setFont(font);
    }
    else {
        QFont font;
        font.setBold(false);
        setFont(font);
    }
}

// called when instrument is selected in menu
// instrumentId is int not uint because of QSignalMapper
void AbstractBankButton::instrumentSelected(int instrumentId) {
    m_instrumentWidget->setInstrument(instrumentId);
}

QString AbstractBankButton::instrumentName(unsigned instrumentId) const {
    Q_ASSERT(instrumentId < FP4InstrumentData::instruments().size());
    const FP4Instrument& instrument = FP4InstrumentData::instruments().at(instrumentId);
    return instrument.name;
}

void AbstractBankButton::buildWidget() {
    QSignalMapper *instrumentMapper = new QSignalMapper(this);
    connect(instrumentMapper, SIGNAL(mapped(int)), SLOT(instrumentSelected(int)));

    setText(bankName());

    QMenu* menu = new QMenu(this);

    for (unsigned i=0; i<FP4InstrumentData::instruments().size(); ++i) {
        if (instrumentMatchesBank(i, m_bank)) {
            QString name = instrumentName(i);
            QAction* action = menu->addAction(name);

            instrumentMapper->setMapping(action, i);
            connect(action, SIGNAL(triggered()), instrumentMapper, SLOT(map()));
        }
    }

    setMenu(menu);
}

GM2CategoryButton::GM2CategoryButton(unsigned bank, InstrumentWidget *instrumentWidget, QWidget *parent) :
    AbstractBankButton(bank, instrumentWidget, parent)
{
    buildWidget();
}

bool GM2CategoryButton::instrumentMatchesBank(unsigned instrumentId, unsigned bank) const {
    Q_ASSERT(instrumentId < FP4InstrumentData::instruments().size());
    const FP4Instrument& instrument = FP4InstrumentData::instruments().at(instrumentId);
    return (instrument.category == (int)bank);
}

QString GM2CategoryButton::bankName() const {
    return FP4InstrumentData::instrumentCategories().at(m_bank);
}

QString GM2CategoryButton::instrumentName(unsigned instrumentId) const {
    Q_ASSERT(instrumentId < FP4InstrumentData::instruments().size());
    const FP4Instrument& instrument = FP4InstrumentData::instruments().at(instrumentId);
    if (instrumentId < 83) {
        return (instrumentId < 86 ) ? QString("*%1").arg(instrument.name) : instrument.name;
    }
    else {
        return instrument.name;
    }
}


FP4BankButton::FP4BankButton(unsigned bank, InstrumentWidget *instrumentWidget, QWidget *parent) :
    AbstractBankButton(bank, instrumentWidget, parent)
{
    buildWidget();
}

bool FP4BankButton::instrumentMatchesBank(unsigned instrumentId, unsigned bank) const {
    Q_ASSERT(instrumentId < FP4InstrumentData::instruments().size());
    const FP4Instrument& instrument = FP4InstrumentData::instruments().at(instrumentId);
    return (instrument.fp4_bank == (int)bank);
}

QString FP4BankButton::bankName() const {
    return FP4InstrumentData::instrumentBanks().at(m_bank);
}


FavouritesButton::FavouritesButton(InstrumentWidget* instrumentWidget, QWidget *parent) :
    QPushButton(parent),
    m_instrumentWidget(instrumentWidget)
{
    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(int)), SLOT(instrumentSelected(int)));

    setDisabled(true);
    setText("Fa&vourites");
    setMenu(new QMenu());
}

/* Repopulate the favourites list and enable/disable the button if it's empty. */
void FavouritesButton::updateFavourites() {
    // QSignalMapper doesn't have a clear method
    delete m_signalMapper;
    m_signalMapper = new QSignalMapper(this);
    connect(m_signalMapper, SIGNAL(mapped(int)), SLOT(instrumentSelected(int)));

    menu()->clear();
    if (InstrumentWidget::s_favourites.isEmpty()) {
        setDisabled(true);
        return;
    }

    setDisabled(false);
    for (int i=0; i<InstrumentWidget::s_favourites.count(); ++i) {
        int instrumentId = InstrumentWidget::s_favourites.at(i);
        const FP4Instrument* instrument = InstrumentWidget::instrument(instrumentId);
        QAction* action = menu()->addAction(instrument->name);
        m_signalMapper->setMapping(action, instrumentId);
        connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    }
}

/* send selected instrument to parent instrumentwidget */
void FavouritesButton::instrumentSelected(int instrumentId) {
    m_instrumentWidget->setInstrument(instrumentId);
}

InstrumentWidget::InstrumentWidget(QWidget *parent) :
    QWidget(parent),
    m_displayMode(FP4StyleDisplay),
    m_instrumentId(0)
{
    s_instrumentWidgets << this;

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);

    // create buttons
    QWidget* buttonsWidget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    buttonsWidget->setLayout(hbox);

    m_GM2Widget = buildGM2StyleWidgets();
    hbox->addWidget(m_GM2Widget, 1);

    m_FP4Widget = buildFP4StyleWidgets();
    hbox->addWidget(m_FP4Widget, 1);

    QWidget *controlWidget = new QWidget;
    hbox->addWidget(controlWidget);
    QGridLayout* controlLayout = new QGridLayout;
    controlLayout->setMargin(0);
    controlLayout->setSpacing(0);
    controlWidget->setLayout(controlLayout);

    m_switchDisplayButton = new QPushButton;
    controlLayout->addWidget(m_switchDisplayButton, 0, 0, 1, 2);
    connect(m_switchDisplayButton, SIGNAL(clicked()), this, SLOT(onSwitchDisplayMode()));

    m_prevButton = new QPushButton("&Prev");
    m_prevButton->setToolTip("Select previous instrument <i>(P)</i>");
    m_prevButton->setShortcut(QKeySequence(Qt::Key_P));
    controlLayout->addWidget(m_prevButton, 1, 0);
    connect(m_prevButton, SIGNAL(clicked()), SLOT(previousInstrument()));

    m_nextButton = new QPushButton("&Next");
    m_nextButton->setToolTip("Select next instrument <i>(N)</i>");
    m_nextButton->setShortcut(QKeySequence(Qt::Key_N));
    controlLayout->addWidget(m_nextButton, 1, 1);
    connect(m_nextButton, SIGNAL(clicked()), SLOT(nextInstrument()));

    m_prevInBankButton = new QPushButton("Prev in Bank");
    m_prevInBankButton->setToolTip("Select previous instrument in current bank <i>(,)</i>");
    m_prevInBankButton->setShortcut(QKeySequence(Qt::Key_Comma));
    controlLayout->addWidget(m_prevInBankButton, 2, 0);
    connect(m_prevInBankButton, SIGNAL(clicked()), SLOT(previousInstrumentInBank()));

    m_nextInBankButton = new QPushButton("Next in Bank");
    m_nextInBankButton->setToolTip("Select next instrument in current bank <i>(.)</i>");
    m_nextInBankButton->setShortcut(QKeySequence(Qt::Key_Period));
    controlLayout->addWidget(m_nextInBankButton, 2, 1);
    connect(m_nextInBankButton, SIGNAL(clicked()), SLOT(nextInstrumentInBank()));

    // create current instrument / favourite bar
    QWidget* currentWidget = new QWidget(this);
    QHBoxLayout* hbox2 = new QHBoxLayout;
    currentWidget->setLayout(hbox2);
    m_currentInstrumentLabel = new QLabel;
    hbox2->addWidget(m_currentInstrumentLabel, 1);

    m_favouriteCheckBox = new QCheckBox(this);
    if (isFavourite(m_instrumentId)) {
        m_favouriteCheckBox->setChecked(true);
    }
    hbox2->addWidget(m_favouriteCheckBox, 0);
    connect(m_favouriteCheckBox, SIGNAL(toggled(bool)), this, SLOT(onFavouriteToggled(bool)));

    m_favouritesButton = new FavouritesButton(this);
    m_favouritesButton->updateFavourites();
    hbox2->addWidget(m_favouritesButton, 0);

    vbox->addWidget(buttonsWidget);
    vbox->addWidget(currentWidget);
    setLayout(vbox);

    Preferences* prefs = FP4App()->preferences();
    setDisplayMode((DisplayMode)prefs->useGM2Banks());
}

/* update interface when display mode is changed */
void InstrumentWidget::setDisplayMode(DisplayMode mode) {
    m_displayMode = mode;

    m_switchDisplayButton->setText(m_displayMode == FP4StyleDisplay  ? "GM2" : "FP4");

    if (m_displayMode == FP4StyleDisplay) {
        m_GM2Widget->hide();
        m_FP4Widget->show();
    }
    else {
        m_GM2Widget->show();
        m_FP4Widget->hide();
    }
}

// change current instrument and update display
void InstrumentWidget::setInstrument(unsigned instrumentId) {
    if (isValidInstrumentId(instrumentId)) {
        m_instrumentId = instrumentId;
    }
    else {
        m_instrumentId = 0;
    }

    updateInstrument(instrumentId);
    emit instrumentChanged(instrumentId);
}

// change instrument display (don't emit any signals)
void InstrumentWidget::updateInstrument(unsigned instrumentId) {
    if (isValidInstrumentId(instrumentId)) {
        m_instrumentId = instrumentId;
    }
    else {
        m_instrumentId = 0;
    }

    const FP4Instrument* instrument = &FP4InstrumentData::instruments().at(m_instrumentId);
    QString bankName = FP4InstrumentData::instrumentBanks().at(instrument->fp4_bank);
    m_currentInstrumentLabel->setText(QString("<b>%1</b> (%2 %3)")
                                      .arg(instrument->name, bankName)
                                      .arg(instrumentId - FP4InstrumentData::bankOffset(instrument->fp4_bank) + 1));
    m_favouriteCheckBox->setChecked(isFavourite(instrumentId));
}

// select next instrument by global instrument id. wrap around.
void InstrumentWidget::nextInstrument() {
    int id = m_instrumentId+1;
    if (!isValidInstrumentId(id)) {
        id = 0;
    }
    setInstrument(id);
}

// select previous instrument by global instrument. wrap around.
void InstrumentWidget::previousInstrument() {
    int id = m_instrumentId - 1;
    if (id <= 0) {
        id = (int)FP4InstrumentData::instruments().size() - 1;
    }
    else {
        id--;
    }
    setInstrument(id);
}

// select next instrument in current bank. wrap around.
void InstrumentWidget::nextInstrumentInBank() {
    int id = currentInstrumentId();

    if (m_displayMode == FP4StyleDisplay) {
        int bank = currentInstrument()->fp4_bank;
        if (isValidInstrumentId(id+1) && instrument(id+1)->fp4_bank==bank) {
            setInstrument(id+1);
        }
        else {
            setInstrument(FP4InstrumentData::bankOffset(bank));
        }
    }
    else {
        int category = currentInstrument()->category;
        do {
            if (!isValidInstrumentId(++id)) {
                id = 0;
            }
        } while (instrument(id)->category != category);
        setInstrument(id);
    }
}

// select previous instrument in current bank. wrap around.
void InstrumentWidget::previousInstrumentInBank() {
    int id = currentInstrumentId();

    if (m_displayMode == FP4StyleDisplay) {
        int bank = currentInstrument()->fp4_bank;
        if (isValidInstrumentId(id-1) && instrument(id-1)->fp4_bank==bank) {
            setInstrument(id-1);
        }
        else {
            setInstrument(FP4InstrumentData::bankOffset(bank+1) - 1);
        }
    }
    else {
        int category = currentInstrument()->category;
        do {
            if (!isValidInstrumentId(--id)) {
                id = FP4InstrumentData::instruments().size() - 1;
            }
        } while (instrument(id)->category != category);
        setInstrument(id);
    }
}

// load favourites from settings
void InstrumentWidget::restoreFavourites(QSettings &settings) {
    settings.beginGroup("FavouriteInstruments");
    s_favourites.clear();
    foreach(QString key, settings.childKeys()) {
        int instrumentId = settings.value(key, -1).value<int>();
        if (isValidInstrumentId(instrumentId)) {
            s_favourites.append(instrumentId);
        }
    }
    settings.endGroup();
}

// save favourites from settings
void InstrumentWidget::saveFavourites(QSettings &settings) {
    settings.beginGroup("FavouriteInstruments");
    settings.remove("");
    int i=0;
    foreach (int instrumentId, s_favourites) {
        settings.setValue(QString("fav%1").arg(i++), instrumentId);
    }
    settings.endGroup();
}

// build buttons to select an instrument by FP4 style banks
QWidget* InstrumentWidget::buildFP4StyleWidgets() {
    QWidget* widget = new QWidget(this);
    QHBoxLayout* hbox = new QHBoxLayout;

    for (unsigned i=0; i<FP4InstrumentData::instrumentBanks().size(); ++i) {
        FP4BankButton* button = new FP4BankButton(i, this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        hbox->addWidget(button);
    }

    widget->setLayout(hbox);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    return widget;
}

// build buttons to select an instrument by GM2 style bank
QWidget* InstrumentWidget::buildGM2StyleWidgets() {
    QWidget* widget = new QWidget(this);
    QGridLayout* grid = new QGridLayout;

    for (unsigned i=0; i<FP4InstrumentData::instrumentCategories().size(); ++i) {
        GM2CategoryButton* button = new GM2CategoryButton(i, this);
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        grid->addWidget(button, i/7, i%7);
    }

    widget->setLayout(grid);
    grid->setMargin(0);
    grid->setSpacing(0);
    return widget;
}

// return current instrument or 0 if current instrumentId is invalid
const FP4Instrument *InstrumentWidget::instrument(int instrumentId) {
    if (isValidInstrumentId(instrumentId)) {
        return &FP4InstrumentData::instruments().at(instrumentId);
    }
    else {
        return 0;
    }
}

// return instrument index of current instrumenet
int InstrumentWidget::currentInstrumentId() const {
    return m_instrumentId;
}

// return instrument that is current to this widget
const FP4Instrument *InstrumentWidget::currentInstrument() const {
    return instrument(m_instrumentId);
}

// Disable keyboard shortcuts. This is hackish, but allows to have menu shortcuts
// when the widget is embedded in the main window, and button shortcuts when the
// widget is not.
void InstrumentWidget::disableShortcuts() {
    m_prevButton->setShortcut(QKeySequence());
    m_nextButton->setShortcut(QKeySequence());
    m_prevInBankButton->setShortcut(QKeySequence());
    m_nextInBankButton->setShortcut(QKeySequence());
}

// return the index of an instrument in the instrument list or -1 if not found.
// FIXME: this function iterates over the whole instrument list to find an id.
//        should probably be optimized with a reverse mapping if used a lot.
int InstrumentWidget::findInstrumentId(const FP4Instrument *instr) {
    if (!instr) {
        return -1;
    }

    int i=0;
    for (const FP4Instrument& check : FP4InstrumentData::instruments()) {
        if (instr->lsb == check.lsb && instr->msb == check.msb && instr->program == check.program) {
            return i;
        }
        ++i;
    }

    return -1;
}

// return true if an instrument is in the favourite list
bool InstrumentWidget::isFavourite(int instrumentId) {
    return s_favourites.contains(instrumentId);
}

// return true if instrumentid is a valid instrument index
bool InstrumentWidget::isValidInstrumentId(int instrumentId) {
    return (instrumentId >= 0 && instrumentId < (int)FP4InstrumentData::instruments().size());
}

/* Toggle display mode if button is clicked */
void InstrumentWidget::onSwitchDisplayMode() {
    DisplayMode mode = (m_displayMode==FP4StyleDisplay) ? GM2StyleDisplay : FP4StyleDisplay;
    setDisplayMode(mode);
}

/* Add or remove favourite instruments if status toggled */
void InstrumentWidget::onFavouriteToggled(bool checked) {
    if (checked) {
        if (!s_favourites.contains(m_instrumentId)) {
            s_favourites.append(m_instrumentId);
        }
    }
    else {
        s_favourites.removeAll(m_instrumentId);
    }

    // tell all widgets to update
    foreach(InstrumentWidget* widget, s_instrumentWidgets) {
        widget->m_favouritesButton->updateFavourites();
    }
}

