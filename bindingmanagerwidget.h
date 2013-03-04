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

/* These classes allow to user to manage controller bindings
   that are used by FP4Qt.
 */

#ifndef BINDINGMANAGERWIDGET_H
#define BINDINGMANAGERWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QList>
#include <QDialog>
#include "fp4qt.h"
#include "window.h"

class FP4Qt;
class QComboBox;
class QPushButton;
class QTableView;
class QSlider;
class QCheckBox;

struct MidiControllerBinding {
    MidiControllerBinding(const ControllerInfo& controller, const BindingInfo& binding) :
        controller(controller), binding(binding)
    {}

    ControllerInfo controller;
    BindingInfo binding;
};

class MidiBindingTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    MidiBindingTableModel(FP4Qt* fp4, QObject* parent=0);

    int rowCount(const QModelIndex & parent = QModelIndex() ) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    int findBinding(const ControllerInfo& controller);

public slots:
    void populate();
    void clear();
    void addBinding(const ControllerInfo& controller, const BindingInfo& binding);
    void removeBinding(const ControllerInfo& controller, const BindingInfo& binding);
    void updateBinding(const ControllerInfo& controller, const BindingInfo& binding);

protected:
    FP4Qt* m_fp4;
    QStringList m_headers;
    QList<MidiControllerBinding> m_bindings;
};

class BindingEditorDialog : public QDialog {
    Q_OBJECT

public:
    BindingEditorDialog(const MidiControllerBinding& binding, QWidget* parent=0);

    const BindingInfo& bindingInfo() { return m_bindingInfo.binding; }

signals:
    void bindingChanged(const ControllerInfo& controller, const BindingInfo& binding);

protected slots:
    void onMinValueChanged(int value);
    void onMaxValueChanged(int value);
    void onReversedChanged(bool value);

protected:
    void notifyBindingChanged();

private:
    MidiControllerBinding m_bindingInfo;

    QSlider* m_minSlider;
    QSlider* m_maxSlider;
    QCheckBox* m_reversedCb;
};

class BindingManagerWindow : public Window
{
    Q_OBJECT
public:
    explicit BindingManagerWindow(FP4Qt* fp4, QWidget *parent = 0);
    void showBindingEditor(const ControllerInfo& controller);

    void loadDefaultBindings();

    void restoreSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

    void savePreset(const QString& preset);
    void restorePreset(const QString& preset);

signals:
    
protected slots:
    void populatePresetCombo();
    void appendPreset(const QString& preset);
    void deletePreset(const QString& preset);
    void presetChanged(const QString& preset);
    void onLoad();
    void onAdd();
    void onSave();
    void onDelete();
    void onRowDoubleClicked(const QModelIndex& index);

    void editCurrentBinding();
    void deleteCurrentBinding();

protected:
    void saveBindings(QSettings& settings);
    void restoreBindings(QSettings& settings);

    void buildWidgets();
    QWidget* buildPresetSelecter();
    QWidget* buildActionBar();
    
private:
    FP4Qt* m_fp4;
    MidiBindingTableModel* m_bindingTable;

    QComboBox* m_presetCombo;
    QPushButton* m_deleteButton;
    QTableView* m_bindingView;

    QSettings* m_settings;
};

#endif // BINDINGMANAGERWIDGET_H
