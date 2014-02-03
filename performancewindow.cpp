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

#include "performancewindow.h"
#include "fp4win.h"
#include "midibindbutton.h"
#include "config.h"
#include "fp4managerapplication.h"
#include "themeicon.h"
#include <QtWidgets>
#include <QListView>
#include <algorithm>

#define FP4_SHOW_EVENT_MIME     "application/fp4-showevent"
#define FP4_SONG_INDEX_MIME     "application/fp4-songindex"
#define FP4_PRESET_NAME_MIME    "application/fp4-presetname"

ShowMetaInfo ShowMetaInfo::Invalid;

ShowMetaInfo::ShowMetaInfo() { }

void ShowMetaInfo::fillApplicationData() {
    generatorName = APP_TITLE;
    generatorVersion = APP_VERSION;
}

ShowMetaInfo ShowMetaInfo::fromSettings(QSettings &settings) {
    settings.beginGroup("About");
    if (settings.value("Type") != "TimeLine") {
        settings.endGroup();
        return ShowMetaInfo();
    }

    ShowMetaInfo info;
    info.generatorName = settings.value("Generator").toString();
    info.generatorVersion = settings.value("GeneratorVersion").toString();
    info.showName = settings.value("showName").toString();
    settings.endGroup();

    return info;
}

void ShowMetaInfo::save(QSettings& settings) {
    settings.beginGroup("About");
    settings.setValue("Type", "Timeline");
    settings.setValue("Generator", APP_TITLE);
    settings.setValue("GeneratorVersion", APP_VERSION);
    settings.setValue("ShowName", showName);
    settings.endGroup();
}

class PresetDelegate : public QStyledItemDelegate {
public:
    PresetDelegate(QObject* parent=0);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

PresetDelegate::PresetDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize PresetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 30);
}

void PresetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 options = option;

    const ConfigurationListModel* model = qobject_cast<const ConfigurationListModel*>(index.model());
    Q_ASSERT(model);

    painter->save();

    QRect textRect = options.rect;
    textRect.setX(textRect.x() + 20);
    textRect.setWidth(textRect.width() - 40);

    QTextDocument description;
    description.setHtml(QString("%1<br/>%2").arg(
                            model->configurationAt(index.row()).name,
                            model->configurationAt(index.row()).description));

    options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);

    painter->translate(options.rect.left(), options.rect.top());
    QRect clipRect(0, 0, options.rect.width(), options.rect.height());
    description.drawContents(painter, clipRect);

    painter->restore();
}

class SongNameDelegate : public QStyledItemDelegate {
public:
    SongNameDelegate(QObject* parent=0);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

SongNameDelegate::SongNameDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize SongNameDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(200, 30);
}

void SongNameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const SongListModel* model = qobject_cast<const SongListModel*>(index.model());
    Q_ASSERT(model);

    painter->save();

    QRect textRect = option.rect;
    textRect.setX(textRect.x() + 20);
    textRect.setWidth(textRect.width() - 40);
    QString name = QString("%1. %2").arg(index.row()+1).arg(model->songAt(index.row()).name);

    QPalette palette = QApplication::style()->standardPalette();

    if (option.state & QStyle::State_Selected) {
        QColor fillColor = model->songAt(index.row()).color;
        fillColor.setHsv(fillColor.hsvHue(), 200, 255);
        painter->fillRect(option.rect, fillColor);
        QFont font;
        font.setBold(true);
        painter->setFont(font);
    }
    else {
        painter->fillRect(option.rect, model->songAt(index.row()).color);
        painter->setPen(palette.text().color());
    }

    painter->drawText(textRect, Qt::AlignVCenter, name);

    painter->restore();
}

class ShowEventNameDelegate : public QItemDelegate {
public:
    ShowEventNameDelegate(QObject* parent=0);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

ShowEventNameDelegate::ShowEventNameDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QSize ShowEventNameDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    qDebug() << "sizehint called";
    return QSize(150, 150);
}

void ShowEventNameDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItemV4 options = option;
    const TimeLineModel* show = qobject_cast<const TimeLineModel*>(index.model());
    Q_ASSERT(show);

    if (!index.isValid()) {
        return;
    }

    PerformanceFrame preset = show->frameAt(index.column());

    painter->save();
    painter->fillRect(options.rect, preset.song.color);

    if (show->currentFrame() == index.column()) {
        QColor color = preset.song.color;
        color.setHsv(color.hsvHue(), 200, 150);
        painter->setPen(QPen(color, 3));
        painter->drawRect(options.rect.adjusted(1, 1, -2, -2));
        QFont bold;
        bold.setBold(true);
        painter->setFont(bold);
    }

    if (options.state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::black, 1));
        painter->drawRect(options.rect.adjusted(1, 1, -1, -1));
    }

    painter->drawText(options.rect.adjusted(5, 5, -5, -5), Qt::AlignVCenter, preset.configurationName);

    painter->setPen(options.palette.text().color());

    if (index.column() == 0 || show->frameAt(index.column()-1).song.name != preset.song.name) {
        painter->drawText(options.rect.x(), options.rect.y() + 40, preset.song.name);
    }

    painter->restore();
}

ConfigurationListModel::ConfigurationListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    populate();
}

int ConfigurationListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_configurations.count();
}

QVariant ConfigurationListModel::data(const QModelIndex &index, int role) const {
    if (index.row() >= m_configurations.count()) {
        return QVariant::Invalid;
    }

    if (role == Qt::DisplayRole) {
        return m_configurations.at(index.row()).name;
    }
    else if (role == Qt::ToolTipRole) {
        return m_configurations.at(index.row()).description;
    }

    return QVariant::Invalid;
}

Qt::DropActions ConfigurationListModel::supportedDropActions() const {
    return Qt::MoveAction;
}

Qt::ItemFlags ConfigurationListModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }

    return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
}

QStringList ConfigurationListModel::mimeTypes() const {
    QStringList types;
    types << FP4_PRESET_NAME_MIME << FP4_SHOW_EVENT_MIME;
    return types;
}

QMimeData *ConfigurationListModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream str(&encodedData, QIODevice::WriteOnly);

    // only handle single selection
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            str << data(index, Qt::DisplayRole).toString();
            break;
        }
    }

    mimeData->setData(FP4_PRESET_NAME_MIME, encodedData);
    return mimeData;
}

int ConfigurationListModel::count() const {
    return m_configurations.count();
}

ConfigurationItem ConfigurationListModel::configurationAt(int index) const {
    Q_ASSERT(index < m_configurations.count());
    return m_configurations.at(index);
}

void ConfigurationListModel::onConfigurationAdded(const QString &name) {
    beginInsertRows(QModelIndex(), m_configurations.count(), m_configurations.count());
    m_configurations << ConfigurationItem(name, "(description)");
    endInsertRows();
}

void ConfigurationListModel::onConfigurationDeleted(const QString &name) {
    auto it = std::find_if(m_configurations.constBegin(), m_configurations.constEnd(),
                           [&](const ConfigurationItem& conf) { return conf.name == name; });

    Q_ASSERT(it != m_configurations.constEnd());
    int idx = it - m_configurations.constBegin();
    beginRemoveRows(QModelIndex(), idx, idx);
    m_configurations.removeAt(idx);
    endRemoveRows();
}

void ConfigurationListModel::onConfigurationUpdated(const QString &name) {
    auto it = std::find_if(m_configurations.constBegin(), m_configurations.constEnd(),
                           [&](const ConfigurationItem& conf) { return conf.name == name; });

    Q_ASSERT(it != m_configurations.constEnd());
    int idx = it - m_configurations.constBegin();

    // TODO
//    m_configurations[idx] =  ConfigurationItem(name, m_configurationManager->shortDescription(name));
    emit dataChanged(index(idx, 0), index(idx, 0));
}

void ConfigurationListModel::populate() {
    beginResetModel();

    QDir dir(FP4App()->configurationsPath());
    QStringList files = dir.entryList(
                QStringList(QString("*.%1").arg(CONFIG_FILE_EXTENSION)),
                QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                QDir::Name);

    m_configurations.clear();
    foreach (QString file, files) {
        ConfigurationItem item(file, "(description)");
        m_configurations << item;
    }

    endResetModel();
}

ConfigurationListView::ConfigurationListView(QWidget *parent) :
    QListView(parent)
{
    setItemDelegate(new PresetDelegate(this));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
}

void ConfigurationListView::dragEnterEvent(QDragEnterEvent *event) {
    qDebug() << "drag enter into configurationlistview" << event->mimeData()->formats();
    if (event->mimeData()->hasFormat(FP4_SHOW_EVENT_MIME)) {
        event->acceptProposedAction();
        qDebug() << "accepted !";
    }
}

void ConfigurationListView::dropEvent(QDropEvent *e) {
    Q_ASSERT(e->mimeData()->hasFormat(FP4_SHOW_EVENT_MIME));

    QByteArray data = e->mimeData()->data(FP4_SHOW_EVENT_MIME);
    QDataStream str(&data, QIODevice::ReadOnly);

    int column;
    str >> column;

    emit performanceEventDropped(column);
}

/* Song list */

SongListModel::SongListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int SongListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_songs.count();
}

QVariant SongListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant::Invalid;
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_songs[index.row()].name;

    case Qt::BackgroundColorRole:
        return m_songs[index.row()].color;

    default:
        return QVariant::Invalid;
    }
}

Qt::DropActions SongListModel::supportedDropActions() const {
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags SongListModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;
    }
    else {
        return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
}

QStringList SongListModel::mimeTypes() const {
    QStringList mimes;
    mimes << FP4_SONG_INDEX_MIME;
    return mimes;
}

QMimeData *SongListModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);

    // we only handle single selection
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            str << m_songs.at(index.row()).name;
            str << m_songs.at(index.row()).color;
            str << index.row();
            break;
        }
    }

    mimeData->setData(FP4_SONG_INDEX_MIME, data);
    return mimeData;
}

bool SongListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    Q_UNUSED(column);

    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat(FP4_SONG_INDEX_MIME)) {
        return false;
    }

    if (row < 0) {
        if (parent.isValid()) {
            row = parent.row();
        }
        else {
            row = m_songs.count();
        }
    }

    QByteArray encodedData = data->data(FP4_SONG_INDEX_MIME);
    QDataStream ds(&encodedData, QIODevice::ReadOnly);

    QString songName;
    QColor color;
    int fromRow;

    ds >> songName;
    ds >> color;
    ds >> fromRow;

    qDebug() << "drop in song..." << songName << color << fromRow;

    if (row == m_songs.count()) {
        beginRemoveRows(QModelIndex(), fromRow, fromRow);
        SongItem song = m_songs.takeAt(fromRow);
        endRemoveRows();
        beginInsertRows(QModelIndex(), m_songs.count()-1, m_songs.count()-1);
        m_songs.append(song);
        endInsertRows();
        emit songMoved(song, m_songs.count()-1);
    }
    else {
        SongItem song = m_songs.at(fromRow);
        if (fromRow == row) {
            // drag to self does nothing
        }
        else if (fromRow+1 == row) {
            // drag on the next item will swap
            SongItem fs = m_songs.at(row);
            m_songs[row] = m_songs[fromRow];
            m_songs[fromRow] = fs;
            QModelIndex tl = index(fromRow);
            QModelIndex br = index(row);
            emit dataChanged(tl, br);
            emit songMoved(song, row);
        }
        else if (fromRow < row) {
            // drag on other items will move to before
            beginMoveRows(QModelIndex(), row, row, QModelIndex(), fromRow);
            m_songs.move(fromRow, row-1);
            endMoveRows();
            emit songMoved(song, row-1);
        }
        else {
            // drag on other items will move to before
            beginMoveRows(QModelIndex(), fromRow, fromRow, QModelIndex(), row);
            m_songs.move(fromRow, row);
            endMoveRows();
            emit songMoved(song, row);
        }
    }

    return true;
}

int SongListModel::count() const {
    return m_songs.count();
}

const SongItem &SongListModel::songAt(int index) const {
    Q_ASSERT(index < m_songs.count());
    return m_songs.at(index);
}

void SongListModel::setSong(int idx, const SongItem &song) {
    Q_ASSERT(idx < m_songs.count());
    m_songs[idx] = song;
    emit dataChanged(index(0, idx), index(0, idx));
    emit songChanged(idx);
}

int SongListModel::indexOf(const QString &name) const {
    auto it = std::find_if(m_songs.constBegin(), m_songs.constEnd(),
                           [&](const SongItem& song) {
                                return song.name == name;
                           });

    return (it == m_songs.constEnd())
        ? -1
        : it - m_songs.constBegin();
}

bool SongListModel::hasSong(const QString &name) const {
    return indexOf(name) != -1;
}

void SongListModel::appendSong(const QString &songName, const QColor& color) {
    SongItem song(songName, color);
    beginInsertRows(QModelIndex(), m_songs.count(), m_songs.count());
    m_songs.append(song);
    endInsertRows();
    emit songAppended(song);
}

void SongListModel::removeSong(int fromRow) {
    Q_ASSERT(fromRow < m_songs.count());
    beginRemoveRows(QModelIndex(), fromRow, fromRow);
    SongItem song = m_songs.at(fromRow);
    m_songs.removeAt(fromRow);
    endRemoveRows();
    emit songRemoved(fromRow);
    emit songRemoved(song);
}

void SongListModel::clear() {
    beginResetModel();
    m_songs.clear();
    endResetModel();
    emit reset();
}

void SongListModel::loadSongList(QSettings *settings) {
    beginResetModel();
    m_songs.clear();
    for (int i=0; i<settings->childGroups().count(); ++i) {
        settings->beginGroup(QString("song%1").arg(i));
        QString songName = settings->value("name").value<QString>();
        QColor color = settings->value("color").value<QColor>();
        settings->endGroup();
        m_songs.append(SongItem(songName, color));
    }
    endResetModel();
    emit reset();
}

void SongListModel::saveSongList(QSettings *settings) {
    for (int i=0; i<m_songs.count(); ++i) {
        settings->beginGroup(QString("song%1").arg(i));
        settings->setValue("name", m_songs[i].name);
        settings->setValue("color", m_songs[i].color);
        settings->endGroup();
    }
}

SongListView::SongListView(QWidget *parent) :
    QListView(parent)
{
    setItemDelegate(new SongNameDelegate(this));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
}

void SongListView::dragEnterEvent(QDragEnterEvent *ev) {
    if (ev->mimeData()->hasFormat(FP4_SONG_INDEX_MIME)) {
        ev->acceptProposedAction();
    }
}

void SongListView::keyPressEvent(QKeyEvent *ev) {
    if (ev->key() == Qt::Key_Delete) {
        if (!currentIndex().isValid()) {
            return;
        }
        SongListModel* songs = qobject_cast<SongListModel*>(model());
        Q_ASSERT(songs);
        songs->removeSong(currentIndex().row());
        ev->accept();
    }
    else if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
        emit editPressed();
        ev->accept();
    }
    else {
        QListView::keyPressEvent(ev);
    }
}

SongColorWidget::SongColorWidget(QWidget *parent) :
    QWidget(parent)
{
    m_color = QColor(0xff, 0xff, 0xff);
}

void SongColorWidget::setColor(const QColor &color) {
    m_color=color;
    update();
}

void SongColorWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);
    painter.setBrush(m_color);

    QRect rect(4, 4, width()-8, height()-8);
    painter.drawRoundedRect(rect, 4, 4);
}

SongListWidget::SongListWidget(SongListModel *model, QWidget *parent) :
    QWidget(parent),
    m_songsModel(model)
{
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    setLayout(vbox);

    QLabel* descLabel = new QLabel("Manage the &songs used in the current show.");
    descLabel->setWordWrap(true);
    vbox->addWidget(descLabel);

    // topBar: enter new song
    QWidget* topBar = new QWidget;
    vbox->addWidget(topBar, 0);
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->setMargin(0);
    topLayout->setSpacing(0);
    topBar->setLayout(topLayout);

    m_nameEdit = new QLineEdit;
    m_nameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    descLabel->setBuddy(m_nameEdit);
    topLayout->addWidget(m_nameEdit);

    m_songColorWidget = new SongColorWidget;
    m_songColorWidget->setFixedWidth(40);
    m_songColorWidget->setColor(colorForIndex(0));
    m_songColorWidget->setToolTip("Next Color");
    topLayout->addWidget(m_songColorWidget);

    m_addButton = new QPushButton(ThemeIcon::buttonIcon("list-add"), "Add");
    topLayout->addWidget(m_addButton);

    // middle part: songs list
    m_songsView = new SongListView;
    m_songsView->setModel(model);
    vbox->addWidget(m_songsView, 1);

    // bottom part: edit / delete buttons
    QWidget* bottomBar = new QWidget;
    vbox->addWidget(bottomBar, 0);
    QHBoxLayout* bottomLayout = new QHBoxLayout;
    bottomLayout->setMargin(0);
    bottomLayout->setSpacing(0);
    bottomBar->setLayout(bottomLayout);

    bottomLayout->addStretch(1);
    m_editButton = new QPushButton(ThemeIcon::buttonIcon("document-properties"), "Edit");
    bottomLayout->addWidget(m_editButton);
    m_deleteButton = new QPushButton(ThemeIcon::buttonIcon("list-remove"), "Delete");
    bottomLayout->addWidget(m_deleteButton);

    connect(m_nameEdit, SIGNAL(textChanged(QString)), SLOT(nameEditChanged(QString)));
    connect(m_nameEdit, SIGNAL(returnPressed()), SLOT(onAddPressed()));
    connect(m_addButton, SIGNAL(clicked()), SLOT(onAddPressed()));
    connect(m_editButton, SIGNAL(clicked()), SLOT(onEditPressed()));
    connect(m_deleteButton, SIGNAL(clicked()), SLOT(onDeletePressed()));
    connect(m_songsView, SIGNAL(doubleClicked(QModelIndex)), SLOT(onSongDoubleClicked(QModelIndex)));
    connect(m_songsView, SIGNAL(editPressed()), SLOT(onEditPressed()));

    connect(model, SIGNAL(reset()), SLOT(onModelReset()));
}

QColor SongListWidget::colorForIndex(int index) {
    QColor color;
    int hue = index * (360.0 * 2.0 / 13.0 );
    hue %= 360;
    color.setHsv(hue, 120, 255);
    return color;
}

void SongListWidget::onAddPressed() {
    QString text = m_nameEdit->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    if (m_songsModel->hasSong(text)) {
        QMessageBox::warning(this, "Duplicate song", "There is already a song with this name.", QMessageBox::Ok);
        m_nameEdit->selectAll();
        return;
    }

    m_songsModel->appendSong(text, m_songColorWidget->color());
    m_songColorWidget->setColor(colorForIndex(m_songsModel->rowCount(QModelIndex())));
    m_nameEdit->clear();
    m_nameEdit->setFocus();
    m_addButton->setDisabled(true);
}

void SongListWidget::onEditPressed() {
    QModelIndex idx = m_songsView->currentIndex();
    if (!idx.isValid()) {
        qDebug() << "Trying to edit invalid index";
        return;
    }

    editSong(idx.row());
}

void SongListWidget::onDeletePressed() {
    if (m_songsModel->rowCount(QModelIndex()) == 0) {
        return;
    }

    QModelIndex index = m_songsView->currentIndex();
    if (index.isValid()) {
        m_songsModel->removeSong(index.row());
        m_songColorWidget->setColor(colorForIndex(m_songsModel->rowCount(QModelIndex())));
    }
}

void SongListWidget::nameEditChanged(const QString &text) {
    m_addButton->setEnabled(!text.trimmed().isEmpty());
}

void SongListWidget::onSongDoubleClicked(const QModelIndex &idx) {
    if (!idx.isValid()) {
        qDebug() << "Trying to edit invalid index";
        return;
    }

    editSong(idx.row());
}

void SongListWidget::onModelReset() {
    m_songColorWidget->setColor(colorForIndex(m_songsModel->count()));
}

void SongListWidget::editSong(int idx) {
    SongItem song = m_songsModel->songAt(idx);
    bool ok;
    bool done=false;
    QString newName = song.name;

    while (!done) {
        newName = QInputDialog::getText(this, "Rename Song",
            "Enter song name:", QLineEdit::Normal, newName, &ok);

        if (!ok) {
            qDebug() << "Cancelled";
            return;
        }

        newName = newName.trimmed();
        if (newName.isEmpty() || newName == song.name) {
            qDebug() << "Aborted";
            return;
        }

        if (m_songsModel->hasSong(newName)) {
            int answer = QMessageBox::question(this, "Duplicate song", "There is already a song with this name. "
                                               "Press OK to enter another name or Cancel to abort.",
                                               QMessageBox::Ok, QMessageBox::Cancel);
            if (answer == QMessageBox::Cancel) {
                return;
            }
        }
        else {
            done = true;
        }
    }

    song.name = newName;
    m_songsModel->setSong(idx, song);
}

/* ShowPreset list */

PerformanceFrame::PerformanceFrame(const QString &presetName) :
    configurationName(presetName),
    song(QString(), Qt::white)
{
}

PerformanceFrame::PerformanceFrame(const SongItem &song) :
    song(song)
{
}

TimeLineModel::TimeLineModel(SongListModel *songListModel, QObject *parent) :
    QAbstractTableModel(parent),
    m_songListModel(songListModel),
    m_currentFrame(0)
{
    connect(m_songListModel, SIGNAL(songAppended(SongItem)), SLOT(onSongAdded(SongItem)));
    connect(m_songListModel, SIGNAL(songRemoved(SongItem)), SLOT(onSongRemoved(SongItem)));
    connect(m_songListModel, SIGNAL(reset()), SLOT(onSongListReset()));
    connect(m_songListModel, SIGNAL(songMoved(SongItem,int)), SLOT(onSongMoved(SongItem,int)));
    connect(m_songListModel, SIGNAL(songChanged(int)), SLOT(onSongChanged(int)));

    connect(this, SIGNAL(currentFrameChanged(int,int)), SLOT(onCurrentFrameChanged(int,int)));
}

int TimeLineModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

int TimeLineModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_frames.count();
}

QVariant TimeLineModel::data(const QModelIndex &index, int role) const {
    if (index.column() >= m_frames.count()) {
        return QVariant();
    }

    switch(role) {
    case Qt::DisplayRole:
        switch(index.row()) {
        case 0:
            return m_frames.at(index.column()).configurationName;
        default:
            return QVariant();
        }

    default:
        return QVariant();
    }
}

Qt::DropActions TimeLineModel::supportedDropActions() const {
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags TimeLineModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return QAbstractTableModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        Qt::ItemFlags flags = QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;
        if (!m_frames.at(index.column()).configurationName.isEmpty()) {
            flags |= Qt::ItemIsDragEnabled;
        }
        return flags;
    }

    return QAbstractTableModel::flags(index);
}

QStringList TimeLineModel::mimeTypes() const {
    QStringList types;
    types << FP4_PRESET_NAME_MIME;
    types << FP4_SHOW_EVENT_MIME;
    return types;
}

QMimeData *TimeLineModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray data;
    QDataStream str(&data, QIODevice::WriteOnly);

    // we only handle single selection
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            str << index.column();
            break;
        }
    }

    mimeData->setData(FP4_SHOW_EVENT_MIME, data);
    return mimeData;
}

bool TimeLineModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int toColumn, const QModelIndex &parent) {
    Q_UNUSED(row);

    if (action == Qt::IgnoreAction)
          return true;

    if (toColumn < 0) {
        if (parent.isValid()) {
            toColumn = parent.column();
        }
        else {
            toColumn = m_frames.count() - 1;
        }
    }

    if (data->hasFormat(FP4_PRESET_NAME_MIME)) {
        // drag effect from effect list to timeline

        if (action != Qt::MoveAction) {
            qDebug() << "Got preset with invalid action";
            return false;
        }

        QByteArray encodedData = data->data(FP4_PRESET_NAME_MIME);
        QDataStream ds(&encodedData, QIODevice::ReadOnly);
        QString presetName;
        ds >> presetName;

        qDebug() << "Inserting" << presetName << "at column" << toColumn;
        insertFrame(presetName, toColumn);

        return true;
    }
    else if (data->hasFormat(FP4_SHOW_EVENT_MIME)) {
        // internal drag around
        // effects can be dragged, but song order must be respected, and
        // an empty effect square must be left at the right of each song span

        QByteArray encodedData = data->data(FP4_SHOW_EVENT_MIME);
        QDataStream ds(&encodedData, QIODevice::ReadOnly);

        if (toColumn == m_frames.count()) {
            qDebug() << "Cannot move past last";
            return false;
        }

        int sourceColumn;
        ds >> sourceColumn;

        if (sourceColumn == toColumn) {
            // cannot copy/move to self
            return false;
        }

        if (action == Qt::MoveAction) {
            return internalDragMove(sourceColumn, toColumn);
        }
        else if (action == Qt::CopyAction) {
            return internalDragCopy(sourceColumn, toColumn);
        }
        else {
            qDebug() << "Invalid action";
            return false;
        }

        return true;
    }

    qDebug() << "Unknown mime formats" << data->formats();
    return false;
}

void TimeLineModel::appendFrame(const QString &configurationName) {
    beginInsertColumns(QModelIndex(), m_frames.count(), m_frames.count());
    m_frames.append(PerformanceFrame(configurationName));
    endInsertColumns();
}

void TimeLineModel::insertFrame(const QString &configurationName, int column) {
    PerformanceFrame sp = frameAt(column);
    PerformanceFrame np = sp;
    np.configurationName = configurationName;

    beginInsertColumns(QModelIndex(), column, column);
    m_frames.insert(column, np);
    endInsertColumns();
}

void TimeLineModel::moveFrame(int fromColumn, int toColumn) {
    if (fromColumn + 1 == toColumn) {
        // i'm kinda confused by these beginMoveXxxx methods' indexes
        PerformanceFrame sp = frameAt(fromColumn);
        m_frames[fromColumn] = m_frames[toColumn];
        m_frames[toColumn] = sp;
        QModelIndex tl = index(0, fromColumn);
        QModelIndex br = index(0, toColumn);
        emit dataChanged(tl, br);
    }
    else {
        PerformanceFrame np = frameAt(fromColumn);
        PerformanceFrame sp = frameAt(toColumn);
        np.song = sp.song;

        if (fromColumn < toColumn) {
            beginMoveColumns(QModelIndex(), fromColumn, fromColumn, QModelIndex(), toColumn);
            m_frames.removeAt(fromColumn);
            m_frames.insert(toColumn-1, np);
            endMoveColumns();
        }
        else {
            beginMoveColumns(QModelIndex(), fromColumn, fromColumn, QModelIndex(), toColumn);
            m_frames.removeAt(fromColumn);
            m_frames.insert(toColumn, np);
            endMoveColumns();
        }
    }

}

void TimeLineModel::copyFrame(int fromColumn, int toColumn) {
    Q_ASSERT(fromColumn < m_frames.count());
    Q_ASSERT(toColumn < m_frames.count());

    PerformanceFrame preset = m_frames.at(fromColumn);
    SongItem song = m_frames.at(toColumn).song;
    preset.song = song;

    beginInsertColumns(QModelIndex(), toColumn, toColumn);
    m_frames.insert(toColumn, preset);
    endInsertColumns();
}

void TimeLineModel::removeFrame(int fromColumn) {
    if (m_frames.count() >= fromColumn) {
        return;
    }

    beginRemoveColumns(QModelIndex(), fromColumn, fromColumn);
    m_frames.removeAt(fromColumn);
    endRemoveColumns();
}

// delete all frames, even empty song frames
void TimeLineModel::clear() {
    beginResetModel();
    m_frames.clear();
    m_currentFrame = 0;
    endResetModel();
}

int TimeLineModel::currentFrame() const {
    return m_currentFrame;
}

QString TimeLineModel::currentConfigurationName() const {
    if (m_currentFrame < m_frames.count()) {
        return m_frames.at(m_currentFrame).configurationName;
    }
    else {
        return QString();
    }
}

QString TimeLineModel::currentSongName() const {
    if (m_currentFrame < m_frames.count()) {
        return m_frames.at(m_currentFrame).song.name;
    }
    else {
        return QString();
    }
}

void TimeLineModel::setCurrentFrame(int idx, bool forceUpdate) {
    if (idx >= m_frames.count()) idx=m_frames.count()-1;
    if (idx < 0) idx = 0;
    if (forceUpdate || m_currentFrame != idx) {
        int old = m_currentFrame;
        m_currentFrame = idx;
        emit currentFrameChanged(old, idx);
        emit currentFrameChanged(idx);
    }
}

void TimeLineModel::rewind() {
    // move to first non-empty frame
    for (int i=0; i<m_frames.count(); ++i) {
        if (!m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            return;
        }
    }

    setCurrentFrame(0);
}

int TimeLineModel::count() const {
    return m_frames.count();
}

bool TimeLineModel::isEmpty() const {
    return m_frames.isEmpty();
}

const PerformanceFrame &TimeLineModel::frameAt(int idx) const {
    Q_ASSERT(idx < m_frames.count());
    return m_frames.at(idx);
}

void TimeLineModel::load(QSettings *settings) {
    beginResetModel();
    m_frames.clear();

    m_currentFrame = settings->value("Current", 0).toInt();

    for (int i=0; i<settings->childGroups().count(); ++i) {
        QString key = QString("preset%1").arg(i);
        settings->beginGroup(key);
        QString presetName = settings->value("preset").toString();
        QString songName = settings->value("song").toString();
        QColor songColor = Qt::white;
        int songNumber = m_songListModel->indexOf(songName);
        if (songNumber >= 0) {
            songColor = m_songListModel->songAt(songNumber).color;
        }
        m_frames << PerformanceFrame(presetName, SongItem(songName, songColor));
        settings->endGroup();
    }

    endResetModel();
}

void TimeLineModel::save(QSettings *settings) {
    settings->remove("");

    settings->setValue("Current", m_currentFrame);

    for (int i=0; i<m_frames.count(); ++i) {
        const PerformanceFrame& preset = m_frames.at(i);
        settings->beginGroup(QString("preset%1").arg(i));
        settings->setValue("preset", preset.configurationName);
        settings->setValue("song", preset.song.name);
        settings->endGroup();
    }
}

void TimeLineModel::onSongAdded(const SongItem &song) {
    beginInsertColumns(QModelIndex(), m_frames.count(), m_frames.count());
    m_frames.append(PerformanceFrame(song));
    endInsertColumns();
}

void TimeLineModel::onSongRemoved(const SongItem &song) {
    beginResetModel();

    auto it = std::remove_if(m_frames.begin(), m_frames.end(), [&](const PerformanceFrame& preset)
        { return preset.song.name == song.name; });

    m_frames.erase(it, m_frames.end());

    endResetModel();
}

void TimeLineModel::onSongChanged(int songNumber) {
    Q_ASSERT(songNumber < m_frames.count());
    SongItem song = m_songListModel->songAt(songNumber);

    int songCount = -1;
    QString currentSong;

    for (int presetIdx = 0; presetIdx < m_frames.count(); ++presetIdx) {
        const PerformanceFrame& preset = m_frames.at(presetIdx);
        if (currentSong != preset.song.name) {
            songCount++;
            currentSong = preset.song.name;
        }
        if (songCount == songNumber) {
            PerformanceFrame newPreset = preset;
            newPreset.song = song;
            m_frames[presetIdx] = newPreset;
            emit dataChanged(index(0, presetIdx), index(0, presetIdx));
        }

        if (songCount > songNumber) {
            break;
        }
    }
}

void TimeLineModel::onSongListReset() {
    beginResetModel();
    m_frames.clear();
    for (int i=0; i<m_songListModel->count(); ++i) {
        m_frames << PerformanceFrame(m_songListModel->songAt(i));
    }
    endResetModel();
}

void TimeLineModel::onSongMoved(const SongItem &song, int row) {
    qDebug() << "Song moved: " << song.name << "to" << row;

    // 1. find where this song was
    int first=-1;
    int last=-1;
    for (int i=0; i<m_frames.count(); ++i) {
        if (m_frames.at(i).song.name == song.name) {
            if (first == -1) {
                first = i;
            }
            last = i;
        }
    }
    Q_ASSERT(first != -1);


    // 2. find to which column the row corresponds and move

    // FIXME: using reset is less efficient than move (but easier (; )
    beginResetModel();

    QList<PerformanceFrame> temp;
    for (int i=first; i<=last; ++i) {
        temp << m_frames.takeAt(first);
    }

    if (row == m_songListModel->count()-1) {
        qDebug() << "move to end: " << first << "-" << last << "to" << m_frames.count();
        m_frames << temp;
    }
    else {
        QString songName;
        int toColumn = 0;
        int songCount = -1;
        for (int i=0; i<m_frames.count(); ++i) {
            if (m_frames.at(i).song.name != songName) {
                songCount++;
                songName = m_frames.at(i).song.name;
            }
            if (songCount == row) {
                toColumn = i;
                break;
            }
        }
        qDebug() << "move inside: " << first << "-" << last << "to" << toColumn;

        while (!temp.isEmpty()) {
            m_frames.insert(toColumn, temp.takeLast());
        }
        endResetModel();
    }

    endResetModel();
}

void TimeLineModel::onCurrentFrameChanged(int from, int to) {
    // datachanged emitted because cells are repainted when cell contents
    // change, but also when active state changes. (ie. current frame
    // changes)
    QModelIndex fromIdx = index(0, from);
    QModelIndex toIdx = index(0, to);
    emit dataChanged(fromIdx, fromIdx);
    emit dataChanged(toIdx, toIdx);
}

bool TimeLineModel::internalDragMove(int from, int to) {
    if (from == m_frames.count()) {
        // last col is virtual
        return false;
    }

    PerformanceFrame sp = frameAt(from);
    PerformanceFrame np = frameAt(to);
//    if (sp.song.name == np.song.name && np.presetName.isEmpty()) {
//        qDebug() << "Cannot move existing preset to existing square";
//        return false;
//    }

    if (sp.configurationName.isEmpty()) {
        qDebug() << "Cannot move empty cells";
        return false;
    }

    qDebug() << "Moving data from column" << from << "to column" << to;
    moveFrame(from, to);
    return true;
}

bool TimeLineModel::internalDragCopy(int from, int to) {
    if (from == m_frames.count()) {
        // last col is virtual
        return false;
    }

    PerformanceFrame sp = frameAt(from);

    if (sp.configurationName.isEmpty()) {
        qDebug() << "Cannot copy empty cells";
        return false;
    }

    qDebug() << "Copying data from column" << from << "to column" << to;
    copyFrame(from, to);
    return true;
}


void TimeLineModel::setPerformanceMode(bool performanceMode) {
}

void TimeLineModel::nextFrame() {
    if (m_frames.isEmpty()) {
        return;
    }

    for (int i=currentFrame()+1; i<m_frames.count(); ++i) {
        if (!m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            break;
        }
    }
}

void TimeLineModel::previousFrame() {
    if (m_frames.isEmpty()) {
        return;
    }

    for (int i=currentFrame()-1; i>=0; --i) {
        if (!m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            break;
        }
    }
}

void TimeLineModel::nextFrameInSong() {
    if (m_frames.isEmpty()) {
        return;
    }

    QString songName = currentSongName();
    for (int i=currentFrame()+1; i<m_frames.count(); ++i) {
        if (m_frames.at(i).song.name != songName) {
            break;
        }
        if (!m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            break;
        }
    }
}

void TimeLineModel::previousFrameInSong() {
    if (m_frames.isEmpty()) {
        return;
    }

    QString songName = currentSongName();
    for (int i=currentFrame()-1; i>=0; --i) {
        if (m_frames.at(i).song.name != songName) {
            break;
        }
        if (!m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            break;
        }
    }
}

void TimeLineModel::nextSong() {
    if (m_frames.isEmpty()) {
        return;
    }

    QString songName = currentSongName();
    for (int i=currentFrame()+1; i<m_frames.count(); ++i) {
        if (m_frames.at(i).song.name != songName && !m_frames.at(i).configurationName.isEmpty()) {
            setCurrentFrame(i);
            break;
        }
    }
}

void TimeLineModel::previousSong() {
    if (m_frames.isEmpty()) {
        return;
    }

    QString songName = currentSongName();
    int i = currentFrame()-1;
    bool found = false;

    // find the first frame in another song that has a presetname
    for (; i>=0; --i) {
        if (m_frames.at(i).song.name != songName && !m_frames.at(i).configurationName.isEmpty()) {
            songName = m_frames.at(i).song.name;
            found = true;
            break;
        }
    }

    if (!found) {
        return;
    }

    // find the first frame in that song
    for (; i>=1; --i) {
        if (m_frames.at(i-1).song.name != songName) {
            break;
        }
    }

    setCurrentFrame(i);
}

void TimeLineModel::deleteFrame(int idx) {
    if (m_frames.isEmpty()) {
        return;
    }

    if (idx >= m_frames.count()) {
        qDebug() << "Trying to delete past end";
        return;
    }

    if (m_frames.at(idx).configurationName.isEmpty()) {
        qDebug() << "Cannot remove empty preset";
        return;
    }

    // FIXME: newly selected cell must be redrawn too, how to find out
    //        which on it is ? For now reset entire model instead of
    //        using {begin,end}RemoveColumns + emit dataChanged

    beginResetModel();

//    beginRemoveColumns(QModelIndex(), idx, idx);
    m_frames.removeAt(idx);
//    endRemoveColumns();

    endResetModel();
}

// delete all frames but leave empty song frames
void TimeLineModel::deleteConfigurationFrames() {
    auto it = std::remove_if(m_frames.begin(), m_frames.end(),
        [](const PerformanceFrame& frame) {
            return !frame.configurationName.isEmpty();
        });

    beginResetModel();
    m_frames.erase(it, m_frames.end());
    endResetModel();
}

/* The ShowPreset organizer */

TimelineView::TimelineView(QWidget *parent) :
    QTableView(parent)
{
    setItemDelegateForRow(0, new ShowEventNameDelegate(this));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    resizeColumnsToContents();
    resizeRowsToContents();

    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClick(QModelIndex)));
}

void TimelineView::onDoubleClick(const QModelIndex &index) {
    TimeLineModel* show = qobject_cast<TimeLineModel*>(model());
    Q_ASSERT(show);
    int idx = index.column();
    show->setCurrentFrame(idx, true);
}

void TimelineView::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat(FP4_PRESET_NAME_MIME)) {
        event->acceptProposedAction();
    }

    if (event->mimeData()->hasFormat(FP4_SHOW_EVENT_MIME)) {
        event->acceptProposedAction();
    }

    if (event->mimeData()->hasFormat(FP4_SONG_INDEX_MIME)) {
        event->acceptProposedAction();
    }
}

void TimelineView::keyPressEvent(QKeyEvent *ev) {
    if (ev->key() == Qt::Key_Delete) {
        if (!currentIndex().isValid()) {
            return;
        }
        TimeLineModel* sm = qobject_cast<TimeLineModel*>(model());
        Q_ASSERT(sm);
        sm->deleteFrame(currentIndex().column());
        ev->accept();
    }
    else if (ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return) {
        if (!currentIndex().isValid()) {
            return;
        }
        TimeLineModel* sm = qobject_cast<TimeLineModel*>(model());
        Q_ASSERT(sm);
        sm->setCurrentFrame(currentIndex().column());
        ev->accept();
    }
    else {
        QTableView::keyPressEvent(ev);
    }
}

/* A widget to create/delete and edit shows */

PerformanceWindow::PerformanceWindow(FP4Win *fp4Win, QWidget *parent) :
    Window("performance", parent),
    m_fp4Win(fp4Win)
{
    setTitle("Performance");

    m_songs = new SongListModel(); // this);

    m_configurations = new ConfigurationListModel;
//    connect(fp4Win->m_globalConfigurationWidget, SIGNAL(configurationAdded(QString)),
//            m_configurations, SLOT(onConfigurationAdded(QString)));
//    connect(fp4Win->m_globalConfigurationWidget, SIGNAL(configurationDeleted(QString)),
//            m_configurations, SLOT(onConfigurationDeleted(QString)));

    m_timeline = new TimeLineModel(m_songs); // , this);

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    layout->addWidget(buildLoadBar(), 0);

    m_splitter = new QSplitter(Qt::Vertical);
    layout->addWidget(m_splitter, 1);

    QWidget* topPart = new QWidget;
    m_splitter->addWidget(topPart);
    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->setMargin(0);
    topLayout->setSpacing(0);
    topPart->setLayout(topLayout);

    topLayout->addWidget(buildConfigurationWidget());
    topLayout->addWidget(buildSongWidget());

    m_splitter->addWidget(buildTimeLineWidget());

    populateShowCombo();

    if (!m_currentShow.isEmpty()) {
        loadShow(m_currentShow);
    }

    connect(m_showCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onShowChanged(QString)));
}

void PerformanceWindow::setPerformanceMode(bool performanceMode) {
    if (m_performanceModeCheckBox->isChecked() != performanceMode) {
        m_performanceModeCheckBox->setChecked(performanceMode);
    }
}

void PerformanceWindow::rewind() {
    m_timeline->rewind();
}

void PerformanceWindow::nextFrame() {
    m_timeline->nextFrame();
}

void PerformanceWindow::previousFrame() {
    m_timeline->previousFrame();
}

void PerformanceWindow::nextFrameInSong() {
    m_timeline->nextFrameInSong();
}

void PerformanceWindow::previousFrameInSong() {
    m_timeline->previousFrameInSong();
}

void PerformanceWindow::nextSong() {
    m_timeline->nextSong();
}

void PerformanceWindow::previousSong() {
    m_timeline->previousSong();
}

void PerformanceWindow::closeEvent(QCloseEvent *) {
    maybeSaveShow();

    QSettings settings;
    settings.beginGroup("Shows");
    settings.setValue("Current", m_currentShow);
    settings.endGroup();
}

void PerformanceWindow::onNewShowPressed() {
    QString showName = askShowName();
    if (showName.isEmpty()) {
        return;
    }

    maybeSaveShow();

    disconnect(m_showCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onShowChanged(QString)));
    if (m_showCombo->findText(showName) >= 0) {
        m_showCombo->setCurrentIndex(m_showCombo->findText(showName));
    }
    else {
        m_showCombo->addItem(showName);
        m_showCombo->setCurrentIndex(m_showCombo->findText(showName));
    }
    connect(m_showCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onShowChanged(QString)));

    m_songs->clear();

    m_currentShow = showName;
}

void PerformanceWindow::onSaveShowPressed() {
    QString defaultName = m_showCombo->currentText();
    QString showName = askShowName(defaultName);
    if (showName.isEmpty()) {
        return;
    }

    saveShow(showName);

    disconnect(m_showCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onShowChanged(QString)));
    populateShowCombo();
    m_showCombo->setCurrentIndex(m_showCombo->findText(showName));
    connect(m_showCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onShowChanged(QString)));
}

void PerformanceWindow::onDeleteShowPressed() {
    int ret = QMessageBox::warning(this, "Delete Show", "This will delete the timeline and the songs in the current show",
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Cancel) {
        return;
    }

    QString showName = m_showCombo->currentText();
    deleteShow(showName);
    m_showCombo->removeItem(m_showCombo->findText(showName));
}

void PerformanceWindow::onShowChanged(const QString &showName) {
    maybeSaveShow();
    loadShow(showName);
}

void PerformanceWindow::onPerformanceModeChanged(bool isPerformanceMode) {
    if (isPerformanceMode) {
        m_timeline->setCurrentFrame(m_timeline->currentFrame(), true);
    }
    emit performanceModeChanged(isPerformanceMode);
}

void PerformanceWindow::onDeleteFramePressed() {
    QModelIndex idx = m_timeLineView->currentIndex();
    if (!idx.isValid()) {
        qDebug() << "Trying to delete at invalid index";
        return;
    }

    m_timeline->deleteFrame(idx.column());
}

void PerformanceWindow::deleteFrame(int idx) {
    m_timeline->deleteFrame(idx);
}

void PerformanceWindow::onCurrentFrameChanged(int idx) {
    m_timeLineView->scrollTo(m_timeline->index(0, idx), QAbstractItemView::EnsureVisible);

    if (m_performanceModeCheckBox->isChecked()) {
        QString presetName = m_timeline->frameAt(idx).configurationName;
        if (presetName.isEmpty()) {
            return;
        }

        qDebug() << "load " << presetName;
//        m_fp4Win->m_globalConfigurationWidget->loadConfiguration(presetName);
    }
}

void PerformanceWindow::populateShowCombo() {
    QDir dir(FP4App()->timeLinesPath());
    QStringList files = dir.entryList(
                QStringList(QString("*.%1").arg(TIMELINE_FILE_EXTENSION)),
                QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                QDir::Name);

    m_showCombo->clear();
    m_showCombo->addItems(files);
    if (!m_currentShow.isEmpty()) {
        m_showCombo->setCurrentIndex(m_showCombo->findText(m_currentShow));
    }
}

void PerformanceWindow::loadShow(const QString &showName) {
    FP4ManagerApplication* app = FP4App();
    QString fileName = app->timeLinesPath() + QDir::separator() + showName;
    QSettings settings(fileName, QSettings::IniFormat);

    ShowMetaInfo meta = ShowMetaInfo::fromSettings(settings);

    settings.beginGroup("Songs");
    m_songs->loadSongList(&settings);
    settings.endGroup();

    settings.beginGroup("TimeLine");
    m_timeline->load(&settings);
    settings.endGroup();

    m_currentShow = showName;
}

void PerformanceWindow::saveShow(const QString &showName) {
    Q_ASSERT(!showName.isNull());

    // FIXME: showName must be proven to be a valid filename before arriving here

    FP4ManagerApplication* app = FP4App();
    QString fileName = app->timeLinesPath() + QDir::separator() + showName;
    QSettings settings(fileName, QSettings::IniFormat);

    settings.remove("");

    ShowMetaInfo meta;
    meta.fillApplicationData();
    meta.showName = showName;
    meta.save(settings);

    settings.beginGroup("Songs");
    m_songs->saveSongList(&settings);
    settings.endGroup();

    settings.beginGroup("TimeLine");
    settings.remove("");
    m_timeline->save(&settings);
    settings.endGroup();
}

void PerformanceWindow::deleteShow(const QString &showName) {
    FP4ManagerApplication* app = FP4App();
    QString fileName = app->timeLinesPath() + QDir::separator() + showName;
    QFile::remove(fileName);
}

QString PerformanceWindow::askShowName(QString defaultName) {
    bool ok;
    bool again = true;
    QString showName;

    while (again) {
        showName = QInputDialog::getText(this, "Save Show",
            "Enter show name:", QLineEdit::Normal, defaultName, &ok);

        if (!ok || showName.trimmed().isEmpty()) {
            return QString();
        }

        showName = showName.trimmed();
        if (showName == "Current") {
            QMessageBox::warning(this, "System name",
                "This show name is reserved. Please choose another one.",
                QMessageBox::Ok);
        }
        else {
            again = false;
        }
    }

    QStringList shows;
    for (int i=0; i<m_showCombo->count(); ++i) {
        shows << m_showCombo->itemText(i);
    }

    if (shows.contains(showName)) {
        QMessageBox::StandardButton reply = QMessageBox::warning(this, "Overwrite Show ?",
            QString("Do you want to overwrite the \"%1\" show ?").arg(showName),
            QMessageBox::Ok | QMessageBox::Cancel );
        if (reply == QMessageBox::Cancel) {
            return QString();
        }
    }

    return showName;
}

void PerformanceWindow::maybeSaveShow() {
    bool doSave = true;
    if (m_currentShow.isEmpty()) {
        if (m_timeline->isEmpty()) {
            doSave = false;
        }
        else {
            QMessageBox::StandardButton answer =
                    QMessageBox::question(this, "Save timeline ?",
                                          "Do you want to save the current timeline ?",
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            switch(answer) {
                case QMessageBox::Yes: {
                    m_currentShow = askShowName();
                    doSave = !m_currentShow.isEmpty();
                    break;
                }

                case QMessageBox::No:
                    doSave = false;
                    break;

                case QMessageBox::Cancel:
                default:
                    return;
            }
        }
    }

    if (doSave) {
        saveShow(m_currentShow);
    }
}

QWidget *PerformanceWindow::buildLoadBar() {
    QWidget* widget = new QWidget;
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    widget->setLayout(hbox);

    m_showCombo = new QComboBox;
    m_showCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(m_showCombo, 1);

    QPushButton* newShow = new QPushButton(ThemeIcon::buttonIcon("document-new"), "New");
    hbox->addWidget(newShow);

    m_saveShowButton = new QPushButton(ThemeIcon::buttonIcon("document-save-as"), "Save As");
    hbox->addWidget(m_saveShowButton);

    m_deleteShowButton = new QPushButton(ThemeIcon::buttonIcon("edit-delete"), "Delete");
    hbox->addWidget(m_deleteShowButton);
    connect(newShow, SIGNAL(clicked()), SLOT(onNewShowPressed()));
    connect(m_saveShowButton, SIGNAL(clicked()), SLOT(onSaveShowPressed()));
    connect(m_deleteShowButton, SIGNAL(clicked()), SLOT(onDeleteShowPressed()));

    return widget;
}

QWidget *PerformanceWindow::buildConfigurationWidget() {
    QWidget* widget = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    widget->setLayout(vbox);

    QLabel* presetLabel = new QLabel("&Configurations");
    vbox->addWidget(presetLabel);

    m_configurationListView = new ConfigurationListView;
    m_configurationListView->setModel(m_configurations);
    vbox->addWidget(m_configurationListView, 0);
    presetLabel->setBuddy(m_configurationListView);

    connect(m_configurationListView, SIGNAL(performanceEventDropped(int)), SLOT(deleteFrame(int)));

    return widget;
}

QWidget *PerformanceWindow::buildSongWidget() {
    SongListWidget* widget = new SongListWidget(m_songs);
    return widget;
}

QWidget *PerformanceWindow::buildTimeLineWidget() {
    QWidget* widget = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->setMargin(0);
    vbox->setSpacing(0);
    widget->setLayout(vbox);

    QWidget* toolBar = new QWidget;
    vbox->addWidget(toolBar);
    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    toolBar->setLayout(hbox);

    m_performanceModeCheckBox = new QCheckBox("&Activate Show Mode");
    hbox->addWidget(m_performanceModeCheckBox);

    QPushButton* rewindButton = new QPushButton("&Rewind");
    rewindButton->setProperty("cc_group", "Performance");
    rewindButton->setProperty("cc_name", "Rewind");
    MidiBindButton* rewindMidi = new MidiBindButton(m_fp4Win->fp4(), rewindButton);
    hbox->addWidget(rewindMidi);
    hbox->addWidget(rewindButton);

    QPushButton* prevButton = new QPushButton("&Previous");
    prevButton->setProperty("cc_group", "Performance");
    prevButton->setProperty("cc_name", "Previous");
    MidiBindButton* prevMidi = new MidiBindButton(m_fp4Win->fp4(), prevButton);
    hbox->addWidget(prevMidi);
    hbox->addWidget(prevButton);

    QPushButton* nextButton = new QPushButton("&Next");
    nextButton->setProperty("cc_group", "Performance");
    nextButton->setProperty("cc_name", "Next");
    MidiBindButton* nextMidi = new MidiBindButton(m_fp4Win->fp4(), nextButton);
    hbox->addWidget(nextMidi);
    hbox->addWidget(nextButton);

    QPushButton* prevInSongButton = new QPushButton("Previous in song");
    prevInSongButton->setProperty("cc_group", "Performance");
    prevInSongButton->setProperty("cc_name", "Previous in song");
    MidiBindButton* prevInSongMidi = new MidiBindButton(m_fp4Win->fp4(), prevInSongButton);
    hbox->addWidget(prevInSongMidi);
    hbox->addWidget(prevInSongButton);

    QPushButton* nextInSongButton = new QPushButton("Next in song");
    nextInSongButton->setProperty("cc_group", "Performance");
    nextInSongButton->setProperty("cc_name", "Next in song");
    MidiBindButton* nextInSongMidi = new MidiBindButton(m_fp4Win->fp4(), nextInSongButton);
    hbox->addWidget(nextInSongMidi);
    hbox->addWidget(nextInSongButton);

    QPushButton* prevSongButton = new QPushButton("Previous Song");
    prevSongButton->setProperty("cc_group", "Performance");
    prevSongButton->setProperty("cc_name", "Previous Song");
    MidiBindButton* prevSongMidi = new MidiBindButton(m_fp4Win->fp4(), prevSongButton);
    hbox->addWidget(prevSongMidi);
    hbox->addWidget(prevSongButton);

    QPushButton* nextSongButton = new QPushButton("Next Song");
    nextSongButton->setProperty("cc_group", "Performance");
    nextSongButton->setProperty("cc_name", "Next Song");
    MidiBindButton* nextSongMidi = new MidiBindButton(m_fp4Win->fp4(), nextSongButton);
    hbox->addWidget(nextSongMidi);
    hbox->addWidget(nextSongButton);

    m_timeLineView = new TimelineView;
    m_timeLineView->setModel(m_timeline);
    vbox->addWidget(m_timeLineView);

    QWidget* editBar = new QWidget;
    vbox->addWidget(editBar);
    QHBoxLayout* editLayout = new QHBoxLayout;
    editLayout->setMargin(0);
    editLayout->setSpacing(0);
    editBar->setLayout(editLayout);

    editLayout->addStretch(1);

    QPushButton* deleteButton = new QPushButton(ThemeIcon::buttonIcon("list-remove"), "Delete");
    editLayout->addWidget(deleteButton);

    QPushButton* clearButton = new QPushButton(ThemeIcon::buttonIcon("edit-clear"), "Clear");
    editLayout->addWidget(clearButton);

    connect(m_performanceModeCheckBox, SIGNAL(clicked(bool)), m_timeline, SLOT(setPerformanceMode(bool)));
    connect(rewindButton, SIGNAL(clicked(bool)), m_timeline, SLOT(rewind()));
    connect(prevButton, SIGNAL(clicked()), m_timeline, SLOT(previousFrame()));
    connect(nextButton, SIGNAL(clicked()), m_timeline, SLOT(nextFrame()));
    connect(prevInSongButton, SIGNAL(clicked()), m_timeline, SLOT(previousFrameInSong()));
    connect(nextInSongButton, SIGNAL(clicked()), m_timeline, SLOT(nextFrameInSong()));
    connect(prevSongButton, SIGNAL(clicked()), m_timeline, SLOT(previousSong()));
    connect(nextSongButton, SIGNAL(clicked()), m_timeline, SLOT(nextSong()));

    connect(m_timeline, SIGNAL(currentFrameChanged(int)), SLOT(onCurrentFrameChanged(int)));

    connect(m_performanceModeCheckBox, SIGNAL(clicked(bool)), SLOT(onPerformanceModeChanged(bool)));

    connect(deleteButton, SIGNAL(clicked()), SLOT(onDeleteFramePressed()));
    connect(clearButton, SIGNAL(clicked()), m_timeline, SLOT(deleteConfigurationFrames()));

    return widget;
}
