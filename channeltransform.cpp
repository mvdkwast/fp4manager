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

#include "channeltransform.h"
#include "fp4qt.h"
#include <QDateTime>
#include <QtGui>

ChannelTransform::ChannelTransform(FP4Qt *fp4, QObject *parent) :
    QObject(parent),
    m_fp4(fp4)
{ }

/* Create an instance of every type of ChannelTransform. Transfer ownership to
   the FP4Qt object. */
ChannelTransformFactory::ChannelTransformFactory(FP4Qt *fp4) {
    m_transforms << new ForwardChannelTransform(fp4, fp4);
    m_transforms << new MonophonicRetriggerChannelTransform(fp4, fp4);
    m_transforms << new MonophonicToggleChannelTransform(fp4, fp4);
    m_transforms << new SmartChannelTransform(fp4, fp4);
    m_transforms << new PortamentoChannelTransform(fp4, fp4);
}

QList<ChannelTransform *> ChannelTransformFactory::channelTransforms() const {
    return m_transforms;
}

QStringList ChannelTransformFactory::channelTransformNames() const {
    QStringList names;
    names << "Forward";
    names << "Monophonic / Retrigger";
    names << "Monophonic / Toggle";
    names << "Smart";
    names << "Portamento";
    return names;
}

/* ChannelMapping::Normal: This forward the incoming note to the output. Same as not using mappings
   except fo the routing. */
ForwardChannelTransform::ForwardChannelTransform(FP4Qt *fp4, QObject *parent) :
    ChannelTransform(fp4, parent)
{ }

void ForwardChannelTransform::handleNoteOn(ChannelMapping *mapping, int channelIn, int channelOut, int note, int velocity) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    m_fp4->sendNoteOn(channelOut, note, velocity);
}

void ForwardChannelTransform::handleNoteOff(ChannelMapping *mapping, int channelIn, int channelOut, int note) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    m_fp4->sendNoteOff(channelOut, note);
}

/* ChannelMapping::MonophonicRetrigger: Allow one note at the time.
   If the note being currently played is played again, send a noteoff
   and a noteon message to retrigger the note. */
MonophonicRetriggerChannelTransform::MonophonicRetriggerChannelTransform(FP4Qt *fp4, QObject *parent) :
    ChannelTransform(fp4, parent)
{ }

void MonophonicRetriggerChannelTransform::handleNoteOn(ChannelMapping *mapping, int channelIn, int channelOut, int note, int velocity) {
    Q_UNUSED(mapping);
    if (m_fp4->isMappedNoteOn(channelIn, channelOut, note))
    {
        m_fp4->sendNoteOff(channelOut, note);
        m_fp4->sendNoteOn(channelOut, note, velocity);
    }
    else {
        m_fp4->mappedNotesOff(channelIn, channelOut);
        m_fp4->registerMappedNote(channelIn, channelOut, note);
        m_fp4->sendNoteOn(channelOut, note, velocity);
    }
}

void MonophonicRetriggerChannelTransform::handleNoteOff(ChannelMapping *mapping, int channelIn, int channelOut, int note) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    Q_UNUSED(channelOut);
    Q_UNUSED(note);

    // do nothing
}

/* ChannelMapping::MonophonicToggle: Allow one note at the time.
   If the note being currently played is played again, send a noteoff message. */
MonophonicToggleChannelTransform::MonophonicToggleChannelTransform(FP4Qt *fp4, QObject *parent) :
    ChannelTransform(fp4, parent)
{ }

void MonophonicToggleChannelTransform::handleNoteOn(ChannelMapping *mapping, int channelIn, int channelOut, int note, int velocity) {
    Q_UNUSED(mapping);

    if (m_fp4->isMappedNoteOn(channelIn, channelOut, note))
    {
        m_fp4->sendNoteOff(channelOut, note);
        m_fp4->unregisterMappedNote(channelIn, channelOut, note);
    }
    else {
        m_fp4->mappedNotesOff(channelIn, channelOut);
        m_fp4->registerMappedNote(channelIn, channelOut, note);
        m_fp4->sendNoteOn(channelOut, note, velocity);
    }
}

void MonophonicToggleChannelTransform::handleNoteOff(ChannelMapping *mapping, int channelIn, int channelOut, int note) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    Q_UNUSED(channelOut);
    Q_UNUSED(note);

    // do nothing
}

/* ChannelMapping::Smart: Note On events arriving shortly on after the other are played as chords. Notes arriving
   later send note off events for currently sounding notes. */

#define SMART_CHANNEL_TRANSFORM_DELAY 20

SmartChannelTransform::SmartChannelTransform(FP4Qt *fp4, QObject *parent) :
    ChannelTransform(fp4, parent)
{
    memset(m_lastNotePress, 0, sizeof(m_lastNotePress));
}

void SmartChannelTransform::handleNoteOn(ChannelMapping *mapping, int channelIn, int channelOut, int note, int velocity) {
    Q_UNUSED(mapping);

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now > m_lastNotePress[channelIn][channelOut] + SMART_CHANNEL_TRANSFORM_DELAY) {
        m_fp4->mappedNotesOff(channelIn, channelOut);
    }

    m_fp4->registerMappedNote(channelIn, channelOut, note);
    m_fp4->sendNoteOn(channelOut, note, velocity);

    m_lastNotePress[channelIn][channelOut] = now;
}

void SmartChannelTransform::handleNoteOff(ChannelMapping *mapping, int channelIn, int channelOut, int note) {
    Q_UNUSED(mapping);
    Q_UNUSED(note);

    m_lastNotePress[channelIn][channelOut] = QDateTime::currentMSecsSinceEpoch();
}

/* insert portamento messages after each note */
PortamentoChannelTransform::PortamentoChannelTransform(FP4Qt *fp4, QObject *parent) :
    ChannelTransform(fp4, parent)
{ }

void PortamentoChannelTransform::handleNoteOn(ChannelMapping *mapping, int channelIn, int channelOut, int note, int velocity) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    m_fp4->sendNoteOn(channelOut, note, velocity);
    m_fp4->sendPortamentoControl(channelOut, note);
}

void PortamentoChannelTransform::handleNoteOff(ChannelMapping *mapping, int channelIn, int channelOut, int note) {
    Q_UNUSED(mapping);
    Q_UNUSED(channelIn);
    m_fp4->sendNoteOff(channelOut, note);
}
