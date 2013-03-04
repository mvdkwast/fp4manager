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

#ifndef SHOWWIDGET_H
#define SHOWWIDGET_H

#include <QWidget>
#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QTableView>
#include <QListView>
#include <QtCore>
#include "window.h"

class FP4Win;
class QPushButton;
class QLineEdit;
class QComboBox;
class QSplitter;
class QCheckBox;
class ConfigurationsWindow;

/* About show and show file -- mainly for future extension (artist, name, desc, version, date) */

struct ShowMetaInfo {
    ShowMetaInfo();
    void fillApplicationData();

    QString generatorName;
    QString generatorVersion;
    QString showName;

    void save(QSettings& settings);

    static ShowMetaInfo fromSettings(QSettings& settings);

    static ShowMetaInfo Invalid;
};

/* Basic list of configuration names */

struct ConfigurationItem {
    ConfigurationItem(const QString& name, const QString& desc) : name(name), description(desc) { }

    QString name;
    QString description;
};

class ConfigurationListModel : public QAbstractListModel {
    Q_OBJECT

public:
    ConfigurationListModel(QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

    int count() const;
    ConfigurationItem configurationAt(int index) const;

public slots:
    void onConfigurationAdded(const QString& name);
    void onConfigurationDeleted(const QString& name);
    void onConfigurationUpdated(const QString& name);

private:
    void populate();

    QList<ConfigurationItem> m_configurations;
};

class ConfigurationListView : public QListView {
    Q_OBJECT

public:
    ConfigurationListView(QWidget *parent=0);

signals:
    void performanceEventDropped(int index);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *e);
};

/* List of songs */

struct SongItem {
    SongItem(const QString& name, const QColor& color) : name(name), color(color) { }

    QString name;
    QColor color;
};

class SongListModel : public QAbstractListModel {
    Q_OBJECT

public:
    SongListModel(QObject* parent=0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    int count() const;
    const SongItem& songAt(int index) const;
    void setSong(int index, const SongItem& song);
    const QList<SongItem>& songs() const { return m_songs; }
    int indexOf(const QString& name) const;
    bool hasSong(const QString& name) const;

    void appendSong(const QString& songName, const QColor& color);
    void removeSong(int fromRow);
    void clear();

    void loadSongList(QSettings* settings);
    void saveSongList(QSettings* settings);

signals:
    void songAppended(const SongItem& song);
    void songRemoved(const SongItem& song);
    void songRemoved(int index);
    void songMoved(const SongItem&, int to);
    void songChanged(int index);
    void reset();

private:
    QList<SongItem> m_songs;
};

class SongListView : public QListView {
    Q_OBJECT

public:
    SongListView(QWidget *parent=0);

signals:
    void editPressed();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void keyPressEvent(QKeyEvent *);
};

class SongColorWidget : public QWidget {
    Q_OBJECT

public:
    SongColorWidget(QWidget* parent=0);

    QColor color() const { return m_color; }
    void setColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent *);

private:
    QColor m_color;
};

struct SongListWidget : public QWidget {
    Q_OBJECT

public:
    SongListWidget(SongListModel* model, QWidget* parent=0);

    QColor colorForIndex(int index);

protected slots:
    void onAddPressed();
    void onEditPressed();
    void onDeletePressed();
    void nameEditChanged(const QString& text);
    void onSongDoubleClicked(const QModelIndex& index);
    void onModelReset();

protected:
    void editSong(int idx);

private:
    SongListModel* m_songsModel;
    SongColorWidget* m_songColorWidget;
    QPushButton* m_addButton;
    QPushButton* m_deleteButton;
    QPushButton* m_editButton;
    QLineEdit* m_nameEdit;
    SongListView* m_songsView;
};


/* A orderable list of frames with some meta info */

struct PerformanceFrame {
    PerformanceFrame(const QString& name, const SongItem& song) : configurationName(name), song(song) { }
    PerformanceFrame(const QString& configurationName);
    PerformanceFrame(const SongItem& song);

    QString configurationName;
    SongItem song;
};

class TimeLineModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TimeLineModel(SongListModel* songListModel, QObject* parent=0);

    void appendFrame(const QString& configurationName);
    void insertFrame(const QString& configurationName, int column);
    void moveFrame(int fromColumn, int toColumn);
    void copyFrame(int fromColumn, int toColumn);
    void removeFrame(int fromColumn);
    void clear();

    int currentFrame() const;
    QString currentConfigurationName() const;
    QString currentSongName() const;

    int count() const;
    bool isEmpty() const;
    const PerformanceFrame& frameAt(int idx) const;

    void load(QSettings* settings);
    void save(QSettings* settings);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

signals:
    void currentFrameChanged(int from, int to);
    void currentFrameChanged(int idx);

public slots:
    void setPerformanceMode(bool performanceMode);
    void setCurrentFrame(int idx, bool forceUpdate=false);
    void rewind();
    void nextFrame();
    void previousFrame();
    void nextFrameInSong();
    void previousFrameInSong();
    void nextSong();
    void previousSong();

    void deleteFrame(int idx);
    void deleteConfigurationFrames();

protected slots:
    void onSongAdded(const SongItem& song);
    void onSongRemoved(const SongItem& song);
    void onSongChanged(int idx);
    void onSongListReset();
    void onSongMoved(const SongItem& song, int row);
    void onCurrentFrameChanged(int from, int to);

protected:
    bool internalDragMove(int from, int to);
    bool internalDragCopy(int from, int to);

private:
    QList<PerformanceFrame> m_frames;
    SongListModel* m_songListModel;

    int m_currentFrame;
};

/* A view to order configurations */

class TimelineView : public QTableView {
    Q_OBJECT

public:
    TimelineView(QWidget* parent=0);

protected slots:
    void onDoubleClick(const QModelIndex& index);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void keyPressEvent(QKeyEvent *ev);
};

/* A window to manage shows, ie. ordered lists of configurations */

class PerformanceWindow : public Window
{
    Q_OBJECT

public:
    explicit PerformanceWindow(FP4Win* fp4Win, QWidget *parent = 0);
    
signals:
    void performanceModeChanged(bool performanceMode);
    
public slots:
    void setPerformanceMode(bool performanceMode);
    void rewind();
    void nextFrame();
    void previousFrame();
    void nextFrameInSong();
    void previousFrameInSong();
    void nextSong();
    void previousSong();

protected slots:
    void closeEvent(QCloseEvent *);

    void onNewShowPressed();
    void onSaveShowPressed();
    void onDeleteShowPressed();
    void onShowChanged(const QString& showName);
    void onPerformanceModeChanged(bool isPerformanceMode);
    void onDeleteFramePressed();
    void deleteFrame(int idx);

    void onCurrentFrameChanged(int idx);

protected:
    void populateShowCombo();
    void loadShow(const QString& showName);
    void saveShow(const QString& showName);
    void deleteShow(const QString& showName);

    QString askShowName(QString defaultName="");
    void maybeSaveShow();

private:
    QWidget* buildLoadBar();
    QWidget* buildConfigurationWidget();
    QWidget* buildSongWidget();
    QWidget* buildTimeLineWidget();

private:
    FP4Win* m_fp4Win;
    SongListModel* m_songs;
    ConfigurationListModel* m_configurations;
    TimeLineModel* m_timeline;

    TimelineView* m_timeLineView;
    QComboBox* m_showCombo;
    QCheckBox* m_performanceModeCheckBox;
    QPushButton* m_saveShowButton;
    QPushButton* m_deleteShowButton;
    ConfigurationListView* m_configurationListView;

    QSplitter* m_splitter;

    QString m_currentShow;
};

#endif // SHOWWIDGET_H
