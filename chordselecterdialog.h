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

#ifndef CHORDSELECTERDIALOG_H
#define CHORDSELECTERDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>

class QWidget;
class QComboBox;
class KeyboardWidget;
class QVBoxLayout;

class ChordSelecterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChordSelecterDialog(QWidget *parent = 0);

    void setPreset(const QString& preset);
    void setNotes(const QString& name, const QList<int>& notes);

    QVBoxLayout* layout();

    QList<int> selectedNotes() const;
    QString selectedPreset() const;
    bool presetIsModified() const;

    void loadPresets();

signals:
    
public slots:
    void loadPreset(const QString& preset);
    void loadModifiedPreset(const QString& name, const QList<int>& notes);
    void deletePreset(const QString& preset);
    void savePreset(const QString& preset);

protected:
    void buildUI();
    QWidget* createPresetBar();
    QWidget* createSelecter();

protected slots:
    void presetChanged(const QString& name);
    void savePresetPressed();
    void deletePresetPressed();

    void notePressed(int note);
    void noteReleased(int note);

protected:
    QMap<QString, QList<int> > m_chordPresets;

    QComboBox* m_presetCombo;
    QPushButton* m_deleteButton;
    KeyboardWidget* m_keyboardWidget;
    QVBoxLayout* m_layout;

    bool m_modified;

    QList<int> m_lastNotes;
};

#endif // CHORDSELECTERDIALOG_H
