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

#include "bindingmanagerwidget.h"
#include "fp4qt.h"
#include "fp4managerapplication.h"
#include "themeicon.h"
#include <QtGui>
#include <algorithm>

MidiBindingTableModel::MidiBindingTableModel(FP4Qt *fp4, QObject *parent) :
    QAbstractTableModel(parent),
    m_fp4(fp4)
{
    m_headers << "Channel" << "Controller" << "Controller Group" << "Controller Name"
              << "Min Value" << "Max Value" << "Reversed";
    populate();
}

int MidiBindingTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_fp4->bindingConfigMap().size();
}

int MidiBindingTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_headers.count();
}

QVariant MidiBindingTableModel::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant::Invalid;
    }

    if (index.row() > m_bindings.count()) {
        return QVariant::Invalid;
    }

    switch (index.column()) {
    case 0:
        return m_bindings[index.row()].controller.channel + 1;
    case 1:
        return m_bindings[index.row()].controller.cc;
    case 2:
        return m_bindings[index.row()].binding.group;
    case 3:
        return m_bindings[index.row()].binding.name;
    case 4:
        return m_bindings[index.row()].binding.minValue;
    case 5:
        return m_bindings[index.row()].binding.maxValue;
    case 6:
        return m_bindings[index.row()].binding.reversed;
    default:
        return QVariant::Invalid;
    }
}

QVariant MidiBindingTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    if (section < m_headers.count()) {
        return m_headers.at(section);
    }
    else {
        return QVariant::Invalid;
    }
}

Qt::ItemFlags MidiBindingTableModel::flags(const QModelIndex &index) const {
    Q_UNUSED(index);

    // prevent selection of individual items (row only)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

// return index of binding that has this controller, or -1 if not found
int MidiBindingTableModel::findBinding(const ControllerInfo &controller) {
    auto it = std::find_if(m_bindings.constBegin(), m_bindings.constEnd(), [&](const MidiControllerBinding& binding) {
        return binding.controller == controller; });

    return it == m_bindings.constEnd()
        ? -1
        : it - m_bindings.constBegin();
}

void MidiBindingTableModel::populate() {
    const BindingConfigMap& bindings = m_fp4->bindingConfigMap();

    beginResetModel();
    m_bindings.clear();

    for (auto it = bindings.constBegin(); it != bindings.constEnd(); ++it) {
        m_bindings << MidiControllerBinding(it.key(), it.value());
    }

    endResetModel();
}

void MidiBindingTableModel::clear() {
    beginResetModel();
    m_bindings.clear();
    endResetModel();
}

void MidiBindingTableModel::addBinding(const ControllerInfo &controller, const BindingInfo &binding) {
    beginInsertRows(QModelIndex(), m_bindings.count(), m_bindings.count());
    m_bindings << MidiControllerBinding(controller, binding);
    endInsertRows();
}

void MidiBindingTableModel::removeBinding(const ControllerInfo &controller, const BindingInfo &bindingInfo) {
    Q_UNUSED(bindingInfo);

    auto it = std::find_if(m_bindings.begin(), m_bindings.end(), [&](MidiControllerBinding& binding) {
        return binding.controller == controller; });

    if (it != m_bindings.end()) {
        beginRemoveRows(QModelIndex(), it - m_bindings.begin(), it - m_bindings.begin());
        m_bindings.erase(it);
        endRemoveRows();
    }
}

void MidiBindingTableModel::updateBinding(const ControllerInfo &controller, const BindingInfo &bindingInfo) {
    auto it = std::find_if(m_bindings.begin(), m_bindings.end(), [&](MidiControllerBinding& binding) {
        return binding.controller == controller; });

    if (it == m_bindings.end()) {
        qDebug() << "updateBinding called on non-existing binding";
        return;
    }

    int row = it - m_bindings.begin();
    m_bindings[row].binding = bindingInfo;

    emit dataChanged(index(row, 0), index(row, columnCount(QModelIndex())));
}

BindingManagerWindow::BindingManagerWindow(FP4Qt *fp4, QWidget *parent) :
    Window("bindingmanager", parent),
    m_fp4(fp4)
{
    setTitle("Binding Manager");

    m_settings = new QSettings(FP4App()->bindingsFile(), QSettings::IniFormat, this);

    m_bindingTable = new MidiBindingTableModel(m_fp4);
//    connect(m_fp4, SIGNAL(bindingSetChanged()),
//            m_bindingTable, SLOT(populate()));
    connect(m_fp4, SIGNAL(bindingAdded(ControllerInfo,BindingInfo)),
            m_bindingTable, SLOT(addBinding(ControllerInfo,BindingInfo)));
    connect(m_fp4, SIGNAL(bindingRemoved(ControllerInfo,BindingInfo)),
            m_bindingTable, SLOT(removeBinding(ControllerInfo,BindingInfo)));
    connect(m_fp4, SIGNAL(bindingUpdated(ControllerInfo,BindingInfo)),
            m_bindingTable, SLOT(updateBinding(ControllerInfo,BindingInfo)));
    connect(m_fp4, SIGNAL(bindingsCleared()),
            m_bindingTable, SLOT(populate()));

    buildWidgets();
    populatePresetCombo();
}

void BindingManagerWindow::showBindingEditor(const ControllerInfo &controller) {
    int idx = m_bindingTable->findBinding(controller);
    if (idx == -1) {
        qDebug() << "Binding not found. This should never happen.";
        // but assert would crash the app although the situation is not dangerous !
        return;
    }

    m_bindingView->selectRow(idx);
    editCurrentBinding();
}

// assign the 3 pedal inputs on the FP-4 to their normal usage.
void BindingManagerWindow::loadDefaultBindings() {
    m_fp4->addControllerBinding(ControllerInfo(0, 64), BindingInfo("Controllers 0", "Sustain"));
    m_fp4->addControllerBinding(ControllerInfo(0, 66), BindingInfo("Controllers 0", "Sostenuto"));
    m_fp4->addControllerBinding(ControllerInfo(0, 67), BindingInfo("Controllers 0", "Soft"));
}

void BindingManagerWindow::restoreSettings(QSettings &settings) {
    settings.beginGroup("Bindings");
    restoreBindings(settings);
    settings.endGroup();
}

void BindingManagerWindow::saveSettings(QSettings &settings) {
    settings.beginGroup("Bindings");
    saveBindings(settings);
    settings.endGroup();
}

void BindingManagerWindow::buildWidgets() {
    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QWidget* presetBar = buildPresetSelecter();
    vbox->addWidget(presetBar, 0);

    m_bindingView = new QTableView;
    m_bindingView->setModel(m_bindingTable);
    m_bindingView->horizontalHeader()->setStretchLastSection(true);
    m_bindingView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_bindingView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_bindingView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_bindingView->selectRow(0);
    m_bindingView->resizeColumnsToContents();
    vbox->addWidget(m_bindingView, 1);

    QWidget* actionBar = buildActionBar();
    vbox->addWidget(actionBar, 0);

    connect(m_bindingView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onRowDoubleClicked(QModelIndex)));
}

QWidget *BindingManagerWindow::buildPresetSelecter() {
    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    widget->setLayout(hbox);

    QLabel* comboLabel = new QLabel("&Preset:");
    hbox->addWidget(comboLabel);
    m_presetCombo = new QComboBox;
    m_presetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(m_presetCombo);
    comboLabel->setBuddy(m_presetCombo);

    QPushButton* loadButton = new QPushButton("&Load");
    loadButton->setIcon(ThemeIcon::buttonIcon("document-open"));
    loadButton->setToolTip("<p>Replace the current bindings with the selected preset.</p>");
    hbox->addWidget(loadButton);
    connect(loadButton, SIGNAL(clicked()), SLOT(onLoad()));

    QPushButton* addButton = new QPushButton("A&dd");
    addButton->setIcon(ThemeIcon::buttonIcon("list-add"));
    addButton->setToolTip("<p>Add the selected preset to the current bindings.</p>");
    hbox->addWidget(addButton);
    connect(addButton, SIGNAL(clicked()), SLOT(onAdd()));

    QPushButton* saveButton = new QPushButton("Save &As");
    saveButton->setIcon(ThemeIcon::buttonIcon("document-save-as"));
    saveButton->setToolTip("<p>Save the current bindings as a preset.</p>");
    hbox->addWidget(saveButton);
    connect(saveButton, SIGNAL(clicked()), SLOT(onSave()));

    m_deleteButton = new QPushButton("De&lete");
    m_deleteButton->setIcon(ThemeIcon::buttonIcon("edit-delete"));
    m_deleteButton->setToolTip("<p>Delete the selected preset.</p>");
    m_deleteButton->setDisabled(true);
    hbox->addWidget(m_deleteButton);
    connect(m_deleteButton, SIGNAL(clicked()), SLOT(onDelete()));

    return widget;
}

QWidget *BindingManagerWindow::buildActionBar() {
    QDialogButtonBox* box = new QDialogButtonBox(Qt::Horizontal);
    QPushButton* editButton = box->addButton("&Edit", QDialogButtonBox::ActionRole);
    editButton->setIcon(ThemeIcon::buttonIcon("document-properties"));
    connect(editButton, SIGNAL(clicked()), SLOT(editCurrentBinding()));

    QPushButton* deleteButton = box->addButton("&Delete", QDialogButtonBox::ActionRole);
    deleteButton->setIcon(ThemeIcon::buttonIcon("edit-delete"));
    connect(deleteButton, SIGNAL(clicked()), SLOT(deleteCurrentBinding()));

    QPushButton* closeButton = box->addButton("&Close", QDialogButtonBox::AcceptRole);
    closeButton->setIcon(ThemeIcon::buttonIcon("window-close"));
    connect(closeButton, SIGNAL(clicked()), SLOT(close()));

    return box;
}

void BindingManagerWindow::populatePresetCombo() {
    m_presetCombo->clear();
    m_presetCombo->addItem("Default");
    m_presetCombo->addItems(m_settings->childGroups());
}

void BindingManagerWindow::appendPreset(const QString &preset) {
    if (preset == "Default") {
        loadDefaultBindings();
    }
    else {
        if (!m_settings->childGroups().contains(preset)) {
                QMessageBox::warning(this, "Load preset", "This preset cannot be loaded. Maybe the configuration file is corrupt");
                return;
        }

        m_settings->beginGroup(preset);
        restoreBindings(*m_settings);
        m_settings->endGroup();
    }

    m_bindingView->selectRow(0);
}

void BindingManagerWindow::deletePreset(const QString &preset) {
    if (preset == "Default" || preset == "Last") {
        QMessageBox::warning(this, "Delete Preset", "This system preset cannot be deleted.");
        return;
    }
    else {
        m_settings->beginGroup(preset);
        m_settings->remove("");
        m_settings->endGroup();
    }
}

void BindingManagerWindow::savePreset(const QString &preset) {
    m_settings->beginGroup(preset);
    saveBindings(*m_settings);
    m_settings->endGroup();
}

void BindingManagerWindow::restorePreset(const QString &preset) {
    m_fp4->clearBindings();

    if (preset == "Default") {
        loadDefaultBindings();
    }
    else {
        if (!m_settings->childGroups().contains(preset)) {
            if (preset == "Last") {
                loadDefaultBindings();
            }
            else {
                QMessageBox::warning(this, "Load preset", "This preset cannot be loaded. Maybe the configuration "
                                     "file is corrupt");
            }
            return;
        }

        m_settings->beginGroup(preset);
        restoreBindings(*m_settings);
        m_settings->endGroup();

        int idx = m_presetCombo->findText(preset);
        m_presetCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    }

    m_bindingView->selectRow(0);
}

void BindingManagerWindow::presetChanged(const QString &preset) {
    m_deleteButton->setDisabled(preset == "Last" || preset == "Default");
}

void BindingManagerWindow::onLoad() {
    QString presetName = m_presetCombo->currentText();
    restorePreset(presetName);
}

void BindingManagerWindow::onAdd() {
    QString presetName = m_presetCombo->currentText();
    appendPreset(presetName);
}

void BindingManagerWindow::onSave() {
    bool ok;
    bool again = true;
    QString presetName;

    while (again) {
        QString defaultName = m_presetCombo->currentText();
        if (defaultName == "Last" || defaultName == "Default") {
            defaultName.clear();
        }

        presetName = QInputDialog::getText(this, "Save preset",
            "Enter preset name:", QLineEdit::Normal, defaultName, &ok);

        if (!ok || presetName.trimmed().isEmpty()) {
            return;
        }

        presetName = presetName.trimmed();
        if (presetName == "Last" || presetName == "Default") {
            QMessageBox::warning(this, "System preset",
                "This preset name is reserved. Please choose another one.",
                QMessageBox::Ok);
        }
        else {
            again = false;
        }
    }

    QStringList presets;
    for (int i=0; i<m_presetCombo->count(); ++i) {
        presets << m_presetCombo->itemText(i);
    }

    if (presets.contains(presetName)) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, "Overwrite Preset ?",
            QString("Do you want to overwrite the \"%1\" preset ?").arg(presetName),
            QMessageBox::Ok | QMessageBox::Cancel );
        if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    savePreset(presetName);

    populatePresetCombo();
    m_presetCombo->setCurrentIndex(m_presetCombo->findText(presetName));
}

void BindingManagerWindow::onDelete() {
    QString presetName = m_presetCombo->currentText();
    if (presetName == "Last" || presetName == "Default") {
        QMessageBox::warning(this, "Cannot Delete",
            "This is a system preset. It can not be deleted.",
            QMessageBox::Ok);
        return;
    }

    deletePreset(presetName);

    int idx = m_presetCombo->currentIndex();
    populatePresetCombo();
    idx--;
    if (idx < 0) idx=0;
    m_presetCombo->setCurrentIndex(idx);
}

void BindingManagerWindow::onRowDoubleClicked(const QModelIndex &index) {
    if (index.isValid()) {
        m_bindingView->selectRow(index.row());
        editCurrentBinding();
    }
}

void BindingManagerWindow::editCurrentBinding() {
    QModelIndex idx = m_bindingView->currentIndex();
    if (idx == QModelIndex()) {
        return;
    }

    int row = idx.row();
    idx = m_bindingTable->index(row, 0);
    int channel = m_bindingTable->data(idx, Qt::DisplayRole).value<int>() - 1;
    idx = m_bindingTable->index(row, 1);
    int cc = m_bindingTable->data(idx, Qt::DisplayRole).value<int>();

    ControllerInfo controller(channel, cc);
    BindingInfo bindingInfo = m_fp4->bindingConfigMap().value(controller);
    MidiControllerBinding binding(controller, bindingInfo);

    BindingEditorDialog* dlg = new BindingEditorDialog(binding, this);
    connect(dlg, SIGNAL(bindingChanged(ControllerInfo,BindingInfo)),
            m_fp4, SLOT(updateBinding(ControllerInfo,BindingInfo)));

    int ret = dlg->exec();

    // Restore old value if dialog was cancelled. This is the opposite behaviour
    // of usual boxes, but it simplifies having the dialog preview the changed values.
    if (ret == QDialog::Rejected) {
        m_fp4->updateBinding(controller, bindingInfo);
    }
}

void BindingManagerWindow::deleteCurrentBinding() {
    QModelIndex idx = m_bindingView->currentIndex();
    if (idx == QModelIndex()) {
        return;
    }

    int row = idx.row();
    idx = m_bindingTable->index(row, 0);
    int channel = m_bindingTable->data(idx, Qt::DisplayRole).value<int>() - 1;
    idx = m_bindingTable->index(row, 1);
    int cc = m_bindingTable->data(idx, Qt::DisplayRole).value<int>();

    m_fp4->deleteControllerBinding(channel, cc);
}

void BindingManagerWindow::saveBindings(QSettings &settings) {
    settings.remove("");
    int i=0;
    for (auto it = m_fp4->bindingConfigMap().constBegin(); it != m_fp4->bindingConfigMap().constEnd(); ++it) {
        settings.beginGroup(QString("binding%1").arg(i++));
        settings.setValue("channel", it.key().channel);
        settings.setValue("cc", it.key().cc);
        settings.setValue("group", it.value().group);
        settings.setValue("name", it.value().name);
        settings.setValue("min", it.value().minValue);
        settings.setValue("max", it.value().maxValue);
        settings.setValue("reversed", it.value().reversed);
        settings.endGroup();
    }
}

void BindingManagerWindow::restoreBindings(QSettings &settings) {
    foreach(QString binding, settings.childGroups()) {
        settings.beginGroup(binding);

        int channel = settings.value("channel", 0).toInt();
        int cc = settings.value("cc", -1).toInt();
        QString group = settings.value("group", "").toString();
        QString name = settings.value("name", "").toString();

        if (cc < 0 || cc > 127 || group.isEmpty() || name.isEmpty()) {
            qDebug() << "Ignoring invalid binding in config file.";
            continue;
        }

        int minValue = settings.value("min", 0).toInt();
        int maxValue = settings.value("max", 0).toInt();
        bool reversed = settings.value("reversed", false).toBool();

        ControllerInfo controller(channel, cc);
        BindingInfo bindingInfo(group, name, minValue, maxValue, reversed);

        m_fp4->addControllerBinding(controller, bindingInfo);

        settings.endGroup();
    }
}

BindingEditorDialog::BindingEditorDialog(const MidiControllerBinding &binding, QWidget *parent) :
    QDialog(parent),
    m_bindingInfo(binding)
{
    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    QLabel* minLabel = new QLabel("M&in Value:");
    vbox->addWidget(minLabel);
    m_minSlider = new QSlider(Qt::Horizontal);
    m_minSlider->setRange(0, 127);
    m_minSlider->setValue(binding.binding.minValue);
    minLabel->setBuddy(m_minSlider);
    vbox->addWidget(m_minSlider);

    QLabel* maxLabel = new QLabel("M&ax Value:");
    vbox->addWidget(maxLabel);
    m_maxSlider = new QSlider(Qt::Horizontal);
    m_maxSlider->setRange(0, 127);
    m_maxSlider->setValue(binding.binding.maxValue);
    maxLabel->setBuddy(m_maxSlider);
    vbox->addWidget(m_maxSlider);

    m_reversedCb = new QCheckBox("Re&verse controller");
    m_reversedCb->setChecked(binding.binding.reversed);
    vbox->addWidget(m_reversedCb);

    vbox->addStretch(1);

    QDialogButtonBox* bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    vbox->addWidget(bbox);

    connect(m_minSlider, SIGNAL(valueChanged(int)), SLOT(onMinValueChanged(int)));
    connect(m_maxSlider, SIGNAL(valueChanged(int)), SLOT(onMaxValueChanged(int)));
    connect(m_reversedCb, SIGNAL(clicked(bool)), SLOT(onReversedChanged(bool)));

    connect(bbox, SIGNAL(accepted()), SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), SLOT(reject()));
}

void BindingEditorDialog::onMinValueChanged(int value) {
    if (m_maxSlider->value() < value) {
        m_maxSlider->setValue(value);
    }

    notifyBindingChanged();
}

void BindingEditorDialog::onMaxValueChanged(int value) {
    if (m_minSlider->value() > value) {
        m_minSlider->setValue(value);
    }

    notifyBindingChanged();
}

void BindingEditorDialog::onReversedChanged(bool value) {
    Q_UNUSED(value);
    notifyBindingChanged();
}

void BindingEditorDialog::notifyBindingChanged() {
    BindingInfo newBinding = m_bindingInfo.binding;
    newBinding.minValue = m_minSlider->value();
    newBinding.maxValue = m_maxSlider->value();
    newBinding.reversed = m_reversedCb->isChecked();
    emit bindingChanged(m_bindingInfo.controller, newBinding);
}
