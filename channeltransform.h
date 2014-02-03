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

/* These classes are used to change midi note events in the FP4Qt note handling
   functions for mapped channels.

   One instance handles every channel, ie. there is only one object of every type of
   ChannelTransform in FP4Qt.
*/

#ifndef CHANNELTRANSFORM_H
#define CHANNELTRANSFORM_H

#include <QObject>
#include <QWidget>

class FP4Qt;
class ChannelMapping;
class QSettings;

class ChannelTransform : public QObject {
    Q_OBJECT
public:
    explicit ChannelTransform(FP4Qt* fp4, QObject* parent=0);
    virtual void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity) = 0;
    virtual void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note) = 0;

protected:
    FP4Qt* m_fp4;
};

class ChannelTransformFactory {
public:
    ChannelTransformFactory(FP4Qt* fp4);

    QList<ChannelTransform*> channelTransforms() const;
    QStringList channelTransformNames() const;

private:
    QList<ChannelTransform*> m_transforms;
};

class ForwardChannelTransform : public ChannelTransform {
    Q_OBJECT
public:
    explicit ForwardChannelTransform(FP4Qt* fp4, QObject* parent=0);
    void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity);
    void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note);
};

class MonophonicRetriggerChannelTransform : public ChannelTransform {
    Q_OBJECT
public:
    explicit MonophonicRetriggerChannelTransform(FP4Qt* fp4, QObject* parent=0);
    void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity);
    void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note);
};

class MonophonicToggleChannelTransform : public ChannelTransform {
    Q_OBJECT
public:
    explicit MonophonicToggleChannelTransform(FP4Qt* fp4, QObject* parent=0);
    void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity);
    void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note);
};

class SmartChannelTransform : public ChannelTransform {
    Q_OBJECT
public:
    explicit SmartChannelTransform(FP4Qt* fp4, QObject* parent=0);
    void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity);
    void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note);

private:
    qint64 m_lastNotePress[16][16];
};

class PortamentoChannelTransform : public ChannelTransform {
    Q_OBJECT
public:
    explicit PortamentoChannelTransform(FP4Qt* fp4, QObject* parent=0);
    void handleNoteOn(ChannelMapping* mapping, int channelIn, int channelOut, int note, int velocity);
    void handleNoteOff(ChannelMapping* mapping, int channelIn, int channelOut, int note);
};

#endif // CHANNELTRANSFORM_H
