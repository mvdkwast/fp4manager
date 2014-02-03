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

#include "chordselecterdialog.h"
#include "keyboardwidget.h"
#include "fp4managerapplication.h"
#include <QtWidgets>

ChordSelecterDialog::ChordSelecterDialog(QWidget *parent) :
    QDialog(parent)
{
    loadPresets();
    buildUI();
}

QList<int> ChordSelecterDialog::selectedNotes() const {
    return m_keyboardWidget->pressedNotes();
}

QString ChordSelecterDialog::selectedPreset() const {
    return m_presetCombo->currentText();
}

bool ChordSelecterDialog::presetIsModified() const {
    return m_modified;
}

void ChordSelecterDialog::setPreset(const QString &preset) {
    if (m_chordPresets.contains(preset)) {
        loadPreset(preset);
    }
    else {
        qWarning() << "Preset not found";
        m_keyboardWidget->releaseAllNotes();
    }
    m_lastNotes = m_keyboardWidget->pressedNotes();
}

void ChordSelecterDialog::setNotes(const QString &name, const QList<int> &notes) {
    loadModifiedPreset(name, notes);
    m_lastNotes = notes;
}

QVBoxLayout *ChordSelecterDialog::layout() {
    return m_layout;
}

void ChordSelecterDialog::loadPresets() {
    m_chordPresets.clear();

    QSettings settings(FP4App()->chordsFile(), QSettings::IniFormat);

    foreach(const QString& presetName, settings.childKeys()) {
        QStringList noteList = settings.value(presetName).toStringList();
        QList<int> notes;
        foreach (const QString& noteName, noteList) {
            notes << noteName.toInt();
        }
        m_chordPresets[presetName] = notes;

    }
}

void ChordSelecterDialog::buildUI() {
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QWidget* body = new QWidget;
    m_layout = new QVBoxLayout;
    m_layout->setMargin(0);
    m_layout->addWidget(createPresetBar());
    m_layout->addWidget(createSelecter());
    body->setLayout(m_layout);

    layout->addWidget(body);
    layout->addStretch();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(buttonBox);
}

QWidget *ChordSelecterDialog::createPresetBar() {
    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    hbox->setMargin(0);
    widget->setLayout(hbox);

    m_presetCombo = new QComboBox();
    m_presetCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    hbox->addWidget(m_presetCombo, 1);

    foreach (const QString& preset, m_chordPresets.keys()) {
        m_presetCombo->addItem(preset);
    }

    QPushButton* saveButton = new QPushButton("&Save");
    hbox->addWidget(saveButton);

    m_deleteButton = new QPushButton("&Delete");
    m_deleteButton->setDisabled(m_chordPresets.isEmpty());
    hbox->addWidget(m_deleteButton);

    connect(m_presetCombo, SIGNAL(currentIndexChanged(QString)), SLOT(presetChanged(QString)));
    connect(saveButton, SIGNAL(pressed()), SLOT(savePresetPressed()));
    connect(m_deleteButton, SIGNAL(pressed()), SLOT(deletePresetPressed()));

    return widget;
}

QWidget *ChordSelecterDialog::createSelecter() {
    m_keyboardWidget = new KeyboardWidget();

    connect(m_keyboardWidget, SIGNAL(notePressed(int)), SLOT(notePressed(int)));
    connect(m_keyboardWidget, SIGNAL(noteReleased(int)), SLOT(noteReleased(int)));

    return m_keyboardWidget;
}

void ChordSelecterDialog::presetChanged(const QString &name) {
    if (m_chordPresets.contains(name)) {
        loadPreset(name);
    }
    else {
        m_keyboardWidget->releaseAllNotes();
        m_keyboardWidget->setPressedNotes(m_lastNotes);
        m_modified = true;
    }
}

void ChordSelecterDialog::savePreset(const QString& presetName) {
    QSettings settings(FP4App()->chordsFile(), QSettings::IniFormat);

    QStringList noteList;
    foreach(int note, selectedNotes()) {
        noteList << QString::number(note);
    }

    settings.setValue(presetName, noteList);

    m_presetCombo->blockSignals(true);
    if (m_presetCombo->findText(presetName) < 0) {
        int i=0;
        for (; i<=m_presetCombo->count(); ++i) {
            if (m_presetCombo->itemText(i) > presetName) {
                break;
            }
        }
        m_presetCombo->insertItem(i, presetName);
    }
    m_presetCombo->setCurrentIndex(m_presetCombo->findText(presetName));
    m_presetCombo->blockSignals(false);

    m_chordPresets[presetName] = selectedNotes();

    m_deleteButton->setDisabled(false);
    m_modified = false;
}

void ChordSelecterDialog::loadPreset(const QString &preset) {
    Q_ASSERT(m_chordPresets.contains(preset));
    m_keyboardWidget->releaseAllNotes();
    m_keyboardWidget->setPressedNotes(m_chordPresets.value(preset));
    m_modified = false;
}

void ChordSelecterDialog::loadModifiedPreset(const QString &name, const QList<int> &notes) {
    m_presetCombo->blockSignals(true);
    m_presetCombo->insertItem(0, name);
    m_presetCombo->setCurrentIndex(0);
    m_presetCombo->blockSignals(false);

    m_keyboardWidget->releaseAllNotes();
    m_keyboardWidget->setPressedNotes(notes);

    m_modified = true;
}

void ChordSelecterDialog::deletePreset(const QString& preset) {
    QSettings settings(FP4App()->chordsFile(), QSettings::IniFormat);
    settings.remove(preset);
    int index=m_presetCombo->findText(preset);
    if (index >= 0) {
        m_presetCombo->removeItem(index);
    }
    m_deleteButton->setDisabled(m_chordPresets.isEmpty());
}

void ChordSelecterDialog::savePresetPressed() {
    bool ok;
    QString presetName;
    QString defaultName = m_presetCombo->currentText();

    forever {
        presetName = QInputDialog::getText(this, "Save preset",
            "Enter preset name:", QLineEdit::Normal, defaultName, &ok);

        if (!ok || presetName.trimmed().isEmpty()) {
            return;
        }

        presetName = presetName.trimmed();

        if (m_presetCombo->findText(presetName) >= 0) {
            QMessageBox::StandardButton reply = QMessageBox::warning(this, "Overwrite Preset ?",
                QString("Do you want to overwrite the \"%1\" preset ?").arg(presetName),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
            if (reply == QMessageBox::Cancel) {
                return;
            }
            if (reply == QMessageBox::No) {
                continue;
            }
        }

        break;
    }

    savePreset(presetName);
}

void ChordSelecterDialog::deletePresetPressed() {
    deletePreset(m_presetCombo->currentText());
}

void ChordSelecterDialog::notePressed(int note) {
    m_modified = true;
}

void ChordSelecterDialog::noteReleased(int note) {
    m_modified = true;
}
