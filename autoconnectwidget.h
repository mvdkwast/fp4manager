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

/* This class handles configuration of which devices automatically
   connect, and handles their actual connection. */

#ifndef AUTOCONNECTWIDGET_H
#define AUTOCONNECTWIDGET_H

#include <QWidget>
#include <vector>
#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include "window.h"
#include "fp4hw.h"

class FP4Qt;
class QTableView;

struct ClientInfo {
    ClientInfo(const QString& clientName, int port) :
        clientName(clientName), port(port) {}

    bool operator<(const ClientInfo& other) const {
        return (port==other.port) ? clientName < other.clientName : port < other.port;
    }

    QString clientName;
    int port;

    static const ClientInfo Invalid;
};

/* Client status */
struct AutoConnectInfo {
    AutoConnectInfo(const QString& clientName, int port) :
        clientName(clientName), clientId(-1), port(port), connected(false), autoConnect(false), opened(false) { }

    bool operator==(const AutoConnectInfo& other) const {
        return other.port == port && other.clientName == clientName;
    }

    bool operator!=(const AutoConnectInfo& other) const {
        return other.port != port || other.clientName != clientName;
    }

    QString clientName;     // alsa name
    int clientId;           // alsa client id
    int port;               // alsa port number
    bool connected;         // device is currently available in the alsa device list
    bool autoConnect;       // connect on startup/device connection
    bool opened;            // this app is connected to this device
};

class AutoConnectTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    AutoConnectTableModel(FP4Qt* fp4, QObject* parent=0);

    int rowCount(const QModelIndex & parent = QModelIndex() ) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

public slots:
    void loadClients(QSettings& settings);
    void saveClients(QSettings& settings) const;
    void connectAll();

    void onConnect(int clientId, int port);
    void onDisconnect(int clientId, int port);

protected:
    void loadFromSettings(QSettings& settings);
    void loadConnected();
    QString lookupClientName(int clientId, int port) const;

private:
    FP4Qt* m_fp4;

    QList<AutoConnectInfo> m_autoConnectList;
    QMap<ClientInfo, AutoConnectInfo*> m_autoConnectMap;
    QMap<int, QString> m_clientNames;

    QStringList m_headers;
};

class AutoConnectWindow : public Window
{
    Q_OBJECT
public:
    explicit AutoConnectWindow(FP4Qt* fp4, QWidget *parent = 0);

public slots:
    void connectAll();

    void loadClients(QSettings& settings);
    void saveClients(QSettings& settings) const;

private:
    FP4Qt* m_fp4;

    AutoConnectTableModel* m_clients;
    QTableView* m_clientsTableWidget;
};

#endif // AUTOCONNECTWIDGET_H
