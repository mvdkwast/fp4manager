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

/* Wrap the FP4 class to send Qt signals on midi events and
   override default behaviour to send incoming midi events back
   to the FP4 hardware. */

#ifndef FP4QT_H
#define FP4QT_H

#include <QObject>
#include <QMap>
#include <inttypes.h>
#include <QStringList>
#include "fp4hw.h"

class QWidget;
class QSettings;
class FP4Qt;
class ChannelTransform;

// map a channel + range to a new channel + transpose
struct ChannelMapping {
    ChannelMapping();

    int keyLow;
    int keyHigh;
    bool active;
    int octaveShift;
    int transformMode;
};

// identify a controller
struct ControllerInfo {
    ControllerInfo() : channel(-1), cc(-1) {}
    ControllerInfo(int channel, int cc) : channel(channel), cc(cc) {}

    int channel;
    int cc;

    static const ControllerInfo Invalid;

    bool operator<(const ControllerInfo& other) const {
        return ( channel == other.channel )
                ? ( cc < other.cc )
                : ( channel < other.channel );
    }

    bool operator==(const ControllerInfo& other) const {
        return cc == other.cc && channel == other.channel;
    }

    bool operator!=(const ControllerInfo& other) const {
        return cc != other.cc || channel != other.channel;
    }
};

// binding info to map controller values to widgets values
struct BindingInfo {
    BindingInfo() : group(""), name(""), minValue(-1), maxValue(-1), reversed(false) {}
    BindingInfo(const QString& group, const QString& name, int min=0, int max=127, bool reversed=false) :
        group(group), name(name), minValue(min), maxValue(max), reversed(reversed)
    {}

    QString group;
    QString name;
    int minValue;
    int maxValue;
    bool reversed;

    static const BindingInfo Invalid;
};

// Controller to Widget. This is used to send incoming CC events to widgets
typedef QMap< ControllerInfo, QWidget* > ControllerBindingMap;

// Widget identification (cc_group, cc_name) to Widget. This keeps track of all widgets that
// are bindable as they are created and destroyed, and is is used to find widgets when
// a new preset is loaded.
typedef QMap< QString, QMap< QString, QWidget* > > BindableWidgetsMap;

// Controller to Binding information (bound widget, mapping range). This is the binding
// configuration as saved.
typedef QMap< ControllerInfo, BindingInfo > BindingConfigMap;

class FP4Qt : public QObject, public FP4
{
    Q_OBJECT
public:
    explicit FP4Qt(const char* clientName=ALSA_CLIENT_NAME, QObject *parent = 0);

    QWidget* controlledWidget(int channel, int cc);
    ControllerInfo controlledWidgetInfo(QWidget* widget);

    const BindingConfigMap& bindingConfigMap() const { return m_bindingConfigMap; }

    void loadDefaultMappings();
    ChannelMapping* channelMapping(int inChannel, int outChannel);

    void enableChannelMappings(bool enable);
    bool channelMappingsEnabled() const;

    const QStringList& channelTransformNames() const { return m_channelTransformNames; }
    QList<ChannelTransform*> channelTransforms() { return m_channelTransforms; }

signals:
    void noteOnReceived(int channel, int note, int velocity);
    void noteOffReceived(int channel, int note);
    void bankChangeReceived(int channel, int msb, int lsb);
    void programChangeReceived(int channel, int pgm);
    void ccReceived(int channel, int cc, int value);
    void sysexReceived(const unsigned char* data, int length);
    void connected();
    void reconnected();
    void disconnected();
    void clientConnected(int m_client_id, int port);
    void clientDisconnected(int m_client_id, int port);

    void bindingsCleared();
    void bindingAdded(const ControllerInfo& controller, const BindingInfo& binding);
    void bindingRemoved(const ControllerInfo& controller, const BindingInfo& binding);
    void bindingUpdated(const ControllerInfo& controller, const BindingInfo& binding);

public slots:
    void clearBindings();

    void onNoteOn(int channel, int note, int velocity);
    void onNoteOff(int channel, int note);
    void onProgramChange(int channel, int pgm);
    void onController(int channel, int controller, int value);
    void onSysEx(const unsigned char* data, int len);
    void onIdentityResponse(const unsigned char* data, int len);

    // main client (FP4) connections
    void onConnect();
    void onReconnect();
    void onDisconnect();

    // other clients connections
    void onClientConnect(int m_client_id, int port);
    void onClientDisconnect(int m_client_id, int port);

    void addControllerBinding(QWidget* widget, int channel, int cc);
    void addControllerBinding(const ControllerInfo& controller, const BindingInfo& binding);
    void updateControllerBinding(QWidget* widget, int channel, int cc);
    void deleteControllerBinding(QWidget* widget);
    void deleteControllerBinding(int channel, int cc);

    void updateBoundWidget(QWidget* widget, int value);
    void onWidgetDeleted(QObject* obj);

    void registerBindableWidget(QWidget* widget);
    void unregisterBindableWidget(QObject* obj);

    void updateBinding(const ControllerInfo& controller, const BindingInfo& binding);

protected:
    void handleMappedNoteOn(int channel, int note, int velocity);
    void handleMappedNoteOff(int channel, int note);

public:
    void mappedNotesOff(int channelIn, int channelOut);
    bool isMappedNoteOn(int channelIn, int channelOut, int note);
    void registerMappedNote(int channelIn, int channelOut, int note);
    void unregisterMappedNote(int channelIn, int channelOut, int note);

private:
    bool m_channelMappingsEnabled;

    ControllerBindingMap m_ccBindings;
    BindableWidgetsMap m_bindableWidgets;
    BindingConfigMap m_bindingConfigMap;

    ChannelMapping m_mappings[16][16];

    uint8_t m_mappedNotes[16][16][128/8];

    QList<ChannelTransform*> m_channelTransforms;
    QStringList m_channelTransformNames;
};

#endif // FP4QT_H
