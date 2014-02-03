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
FP4 Manager. If not, see http://www.gnu.org/licenses/.

******************************************************************************/

/* Select an instrument.

   Two display modes are possible: by FP4 bank button or by GM2 category.

   Favourites are shared and synchronized between instances.

   Use InstrumentSelectDialog for a standalone dialog and InstrumentWidget
   if the instrument selecter needs to be embedded.

*/

#ifndef INSTRUMENTWIDGET_H
#define INSTRUMENTWIDGET_H

#include <QWidget>
#include <QPushButton>

class InstrumentWidget;
class FP4Instrument;
class FP4Qt;
class QLabel;
class QCheckBox;
class QSignalMapper;
class FavouritesButton;
class QSettings;

/* A widget to select instruments and activate them on the FP4 */
class InstrumentWidget : public QWidget
{
    Q_OBJECT
public:
    enum DisplayMode {
        FP4StyleDisplay,
        GM2StyleDisplay
    };

    explicit InstrumentWidget(QWidget *parent = 0);

    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const { return m_displayMode; }

    int currentInstrumentId() const;
    const FP4Instrument* currentInstrument() const;

    void disableShortcuts();

    static int findInstrumentId(const FP4Instrument* instr);

    static const FP4Instrument* instrument(int instrumentId);

    static bool isFavourite(int instrumentId);
    static bool isValidInstrumentId(int instrumentId);

    static void restoreFavourites(QSettings& settings);
    static void saveFavourites(QSettings& settings);

signals:
    void instrumentChanged(unsigned instrumentId);

public slots:
    void setInstrument(unsigned instrumentId);
    void updateInstrument(unsigned instrumentId);

    void nextInstrument();
    void previousInstrument();

    void nextInstrumentInBank();
    void previousInstrumentInBank();

protected slots:
    void onSwitchDisplayMode();
    void onFavouriteToggled(bool);

private:
    QWidget* buildGM2StyleWidgets();
    QWidget* buildFP4StyleWidgets();

signals:
    
private:
    DisplayMode m_displayMode;
    int m_instrumentId;

    QWidget* m_GM2Widget;
    QWidget* m_FP4Widget;
    QPushButton* m_switchDisplayButton;

    QLabel* m_currentInstrumentLabel;
    QCheckBox* m_favouriteCheckBox;
    FavouritesButton* m_favouritesButton;

    QPushButton* m_prevButton;
    QPushButton* m_nextButton;
    QPushButton* m_prevInBankButton;
    QPushButton* m_nextInBankButton;

    static QList<int> s_favourites;
    static QList<InstrumentWidget*> s_instrumentWidgets;

    friend class FavouritesButton;
};

/* a push button with a menu to select instruments from a bank / category */
class AbstractBankButton : public QPushButton {
    Q_OBJECT

public:
    AbstractBankButton(unsigned bank, InstrumentWidget* instrumentWidget, QWidget* parent=0);
    QSize sizeHint() const;

protected slots:
    void onInstrumentChanged(unsigned int instrumentId);
    void instrumentSelected(int instrumentId);

protected:
    virtual bool instrumentMatchesBank(unsigned instrumentId, unsigned bank) const = 0;
    virtual QString bankName() const = 0;
    virtual QString instrumentName(unsigned instrumentId) const;

    void buildWidget();

protected:
    InstrumentWidget* m_instrumentWidget;
    unsigned m_bank;
};

/* A button with a menu to elect an instrument from a GM2 style bank. */
class GM2CategoryButton : public AbstractBankButton {
    Q_OBJECT

public:
    GM2CategoryButton(unsigned bank, InstrumentWidget* instrumentWidget, QWidget* parent=0);

protected:
    bool instrumentMatchesBank(unsigned instrumentId, unsigned bank) const;
    QString bankName() const;
    QString instrumentName(unsigned instrumentId) const;
};

/* a button with a menu to select an instrument from a FP4 bank (physical
   buttons on the FP4 */
class FP4BankButton : public AbstractBankButton {
    Q_OBJECT

public:
    FP4BankButton(unsigned bank, InstrumentWidget* instrumentWidget, QWidget* parent=0);

protected:
    bool instrumentMatchesBank(unsigned instrumentId, unsigned bank) const;
    QString bankName() const;
};

/* A button with a menu to select an instrument from favourites */
class FavouritesButton : public QPushButton {
    Q_OBJECT

public:
    FavouritesButton(InstrumentWidget *instrumentWidget, QWidget* parent=0);
    void updateFavourites();

protected slots:
    void instrumentSelected(int instrumentId);

private:
    InstrumentWidget* m_instrumentWidget;
    QSignalMapper* m_signalMapper;
};

#endif // INSTRUMENTWIDGET_H
