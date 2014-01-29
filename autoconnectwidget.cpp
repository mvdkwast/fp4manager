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

#include "autoconnectwidget.h"
#include "fp4qt.h"
#include "string.h"
#include "config.h"
#include "themeicon.h"
#include <QDebug>
#include <QtWidgets>
#include <algorithm>

const ClientInfo ClientInfo::Invalid(QString(), -1);

AutoConnectTableModel::AutoConnectTableModel(FP4Qt *fp4, QObject *parent) :
    QAbstractTableModel(parent),
    m_fp4(fp4)
{
    m_headers << "Availability" << "Connected" << "Autoconnect" << "Device / Application"
              << "Alsa client ID" << "Alsa client port";

    connect(m_fp4, SIGNAL(clientConnected(int,int)), SLOT(onConnect(int,int)));
    connect(m_fp4, SIGNAL(clientDisconnected(int,int)), SLOT(onDisconnect(int,int)));
}

int AutoConnectTableModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_autoConnectList.count();
}

int AutoConnectTableModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_headers.count();
}

QVariant AutoConnectTableModel::data(const QModelIndex &index, int role) const {
    if (index.row() > m_autoConnectList.count()) {
        return QVariant::Invalid;
    }

    const AutoConnectInfo& client = m_autoConnectList.at(index.row());

    switch(role) {
    case Qt::DisplayRole:
        switch(index.column()) {
        case 0:
            return client.connected
                    ? "Online"
                    : "Offline";
        case 1:
            return client.opened;
        case 2:
            return client.autoConnect;
        case 3:
            return client.clientName;
        case 4:
            return client.connected
                    ? QString::number(client.clientId)
                    : QString("");
        case 5:
            return client.port;
        default:
            return QVariant::Invalid;
        }

    case Qt::CheckStateRole:
        switch (index.column()) {
        case 1:
            return client.opened ? Qt::Checked : Qt::Unchecked;
        case 2:
            return client.autoConnect ? Qt::Checked : Qt::Unchecked;
        default:
            return QVariant::Invalid;
        }

    case Qt::ToolTipRole:
        switch(index.column()) {
        case 1:
            if (client.connected) {
                if (client.opened) {
                    return QString("Disconnect from this client now.");
                }
                else {
                    return QString("Connect to this client now.");
                }
            }
            return QVariant::Invalid;

        case 2:
            return QString("<p>Automatically connect to this client either when the program starts or when the devices connects.</p>");
        }

    case Qt::ForegroundRole:
        if (client.connected) {
            return QPalette().color(QPalette::Active, QPalette::Text);
        }
        else {
            return QPalette().color(QPalette::Disabled, QPalette::Text);
        }

    default:
        return QVariant::Invalid;
    }
}

QVariant AutoConnectTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return QVariant::Invalid;
    }

    if (role != Qt::DisplayRole) {
        return QVariant::Invalid;
    }

    if (section > m_headers.count()) {
        return QVariant::Invalid;
    }

    return m_headers.at(section);
}

Qt::ItemFlags AutoConnectTableModel::flags(const QModelIndex &index) const {
    switch(index.column()) {
    case 0:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    case 1:
        return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    case 2:
        return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    default:
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
}

bool AutoConnectTableModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
    if (idx.column() != 1 && idx.column() != 2) {
        return false;
    }

    AutoConnectInfo info = m_autoConnectList.at(idx.row());
    bool enable = value.value<bool>();

    if (idx.column() == 1) {
        if (!info.connected) {
            return false;
        }
        if (enable) {
            bool ok = m_fp4->openSecondary(info.clientId, info.port, FP4::Readable);
            if (!ok) {
                return false;
            }
            m_autoConnectList[idx.row()].opened = true;
        }
        else {
            m_fp4->closeSecondary(info.clientId, info.port, FP4::Readable);
            m_autoConnectList[idx.row()].opened = false;
        }

        QModelIndex topLeft = index(idx.row(), 0);
        QModelIndex bottomRight = index(idx.row(), m_headers.count());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }

    if (idx.column() == 2) {
        m_autoConnectList[idx.row()].autoConnect = enable;
        QModelIndex topLeft = index(idx.row(), 0);
        QModelIndex bottomRight = index(idx.row(), m_headers.count());
        emit dataChanged(topLeft, bottomRight);
        return true;
    }

    return false;
}

/* build a client list from settings and from Alsa */
void AutoConnectTableModel::loadClients(QSettings &settings) {
    beginResetModel();
    loadFromSettings(settings);
    loadConnected();
    endResetModel();
}

/* save connection info to settings if they should autoconnect next time. */
void AutoConnectTableModel::saveClients(QSettings &settings) const {
    settings.beginGroup("AutoConnect");
    settings.remove("");

    foreach(const AutoConnectInfo& client, m_autoConnectList) {
        if (client.autoConnect) {
            settings.setValue(QString("%1:%2").arg(client.clientName).arg(client.port), client.autoConnect);
        }
    }

    settings.endGroup();
}

/* connect to all know alsa clients that have the autoconnect status */
void AutoConnectTableModel::connectAll() {
    for (int i=0; i<m_autoConnectList.count(); ++i) {
        AutoConnectInfo& client = m_autoConnectList[i];
        if (client.connected && client.autoConnect) {
            bool ok = m_fp4->openSecondary(client.clientId, client.port, FP4::Readable);
            if (!ok) {
                qDebug() << "Connection to" << client.clientId << ":" << client.port << "failed.";
            }
            else {
                m_autoConnectList[i].opened = true;
            }
        }
    }
}

/* called when an Alsa client becomes available. If we know it from the settings,
   update its connection status and connect to it accordingly. */
void AutoConnectTableModel::onConnect(int clientId, int port) {
    QString clientName = lookupClientName(clientId, port);
    if (clientName.isEmpty()) {
        qDebug() << "Could not find a name fo client" << clientId << ":" << port;
        return;
    }

    // keep name for disconnect
    m_clientNames[clientId] = clientName;

    ClientInfo clientInfo(clientName, port);

    if (m_autoConnectMap.contains(clientInfo)) {
        // this client was seen before
        AutoConnectInfo* info = m_autoConnectMap.value(clientInfo);
        info->clientId = clientId;
        info->connected = true;

        if (info->autoConnect) {
            info->opened = m_fp4->openSecondary(clientId, port, FP4::Readable);
        }

        int row = m_autoConnectList.indexOf(*info);
        if (row >= 0) {
            QModelIndex topLeft = index(row, 0);
            QModelIndex bottomRight = index(row, m_headers.count());
            emit dataChanged(topLeft, bottomRight);
        }
    }
    else {
        // totally new and strange client, how exciting
        AutoConnectInfo info(clientName, port);
        info.clientId = clientId;
        info.connected = true;

        beginInsertRows(QModelIndex(), m_autoConnectList.count(), m_autoConnectList.count());
        m_autoConnectList << info;
        m_autoConnectMap[clientInfo] = &m_autoConnectList.last();
        endInsertRows();
    }
}

/* update connection status when an Alsa client leaves */
void AutoConnectTableModel::onDisconnect(int clientId, int port) {
    QString clientName = m_clientNames.value(clientId);
    if (clientName.isEmpty()) {
        qDebug() << "Client with unknown name disconnected.";
        return;
    }

    ClientInfo clientInfo(clientName, port);

    if (m_autoConnectMap.contains(clientInfo)) {
        AutoConnectInfo* info = m_autoConnectMap[clientInfo];
        info->clientId = -1;
        info->connected = false;
        info->opened = false;

        int row = m_autoConnectList.indexOf(*info);
        if (row >= 0) {
            QModelIndex topLeft = index(row, 0);
            QModelIndex bottomRight = index(row, m_headers.count());
            emit dataChanged(topLeft, bottomRight);
        }
    }
    else {
        qDebug() << "Unknown Alsa client" << clientId << ":" << port << "left.";
    }
}

/* Load autoconnection info from settings. This is called before
   loadConnected which loads a list of available alsa clients,
   so we don't care about updating client information */
void AutoConnectTableModel::loadFromSettings(QSettings& settings) {
    settings.beginGroup("AutoConnect");
    beginResetModel();

    m_autoConnectList.clear();

    foreach(QString entry, settings.childKeys()) {
        // we need to be greedier than split as usb names may contain ':'
        QRegExp rx("(.*):(\\d+)");
        if (rx.indexIn(entry) != -1) {
            QString name = rx.cap(1);
            int port = rx.cap(2).toInt();
            bool autoConnect = settings.value(entry, false).value<bool>();

            AutoConnectInfo info(name, port);
            info.autoConnect = autoConnect;

            m_autoConnectList << info;
            m_autoConnectMap[ClientInfo(name, port)] = &m_autoConnectList.last();
        }
    }

    endResetModel();
    settings.endGroup();
}

/* add all currently connected Alsa clients except for ourselves and the FP4. */
void AutoConnectTableModel::loadConnected() {
    auto clientList = m_fp4->getPortList(FP4::Readable);
    for (const AlsaClientInfo& client : clientList) {
        if (!strcmp(FP4_CLIENT_NAME, client.name())) {
            continue;
        }

        if (!strcmp(APP_TITLE, client.name())) {
            continue;
        }

        ClientInfo clientInfo(client.name(), client.port());

        if (m_autoConnectMap.contains(clientInfo)) {
            AutoConnectInfo* info = m_autoConnectMap.value(clientInfo);
            info->clientId = client.client();
            info->connected = true;
        }
        else {
            AutoConnectInfo info(client.name(), client.port());
            info.clientId = client.client();
            info.connected = true;
            m_autoConnectList << info;
            m_autoConnectMap[clientInfo] = &m_autoConnectList.last();
        }

        m_clientNames[client.client()] = client.name();
    }
}

/* lookup the human readable name of an Alsa client */
QString AutoConnectTableModel::lookupClientName(int clientId, int port) const {
    auto clientList = m_fp4->getPortList(FP4::Readable);

    auto it = std::find_if(clientList.begin(), clientList.end(), [&](const AlsaClientInfo& client) {
        return client.client() == clientId && client.port() == port;
    });

    return it == clientList.end()
        ? QString()
        : QString(it->name());
}

AutoConnectWindow::AutoConnectWindow(FP4Qt* fp4, QWidget *parent) :
    Window("autoconnect", parent),
    m_fp4(fp4)
{
    setTitle("Autoconnect devices");

    m_clients = new AutoConnectTableModel(m_fp4, this);

    QVBoxLayout* vbox = new QVBoxLayout;
    setLayout(vbox);

    m_clientsTableWidget = new QTableView;
    m_clientsTableWidget->setModel(m_clients);
    m_clientsTableWidget->horizontalHeader()->setStretchLastSection(true);
    m_clientsTableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    m_clientsTableWidget->verticalHeader()->hide();
    m_clientsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clientsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_clientsTableWidget->resizeColumnsToContents();
    m_clientsTableWidget->selectRow(0);
    vbox->addWidget(m_clientsTableWidget);
    \
    QDialogButtonBox* bbox = new QDialogButtonBox;
    vbox->addWidget(bbox);
    QPushButton* closeButton = bbox->addButton("&Close", QDialogButtonBox::AcceptRole);
    closeButton->setIcon(ThemeIcon::buttonIcon("window-close"));
    connect(closeButton, SIGNAL(clicked()), SLOT(close()));

    m_clientsTableWidget->setFocus();
}

void AutoConnectWindow::connectAll() {
    m_clients->connectAll();
}

void AutoConnectWindow::loadClients(QSettings &settings) {
    m_clients->loadClients(settings);
}

void AutoConnectWindow::saveClients(QSettings &settings) const {
    m_clients->saveClients(settings);
}
