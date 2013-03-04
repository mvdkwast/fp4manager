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

#include "fp4qt.h"
#include "controllerbinding.h"
#include "channeltransform.h"
#include "fp4constants.h"
#include <QtGui>
#include <QDebug>

/* Macros to keep track of played notes in mapped channels. This is so to
   make it possible to have multiple input ranges map to the same output
   range. This array is shared between the ChannelTransform classes.
*/
#define REGISTER_MAPPED_NOTE(channelIn, channelOut, note) \
    m_mappedNotes[channelIn][channelOut][(note>>3)] |= (1<<(note&0b111))

#define UNREGISTER_MAPPED_NOTE(channelIn, channelOut, note) \
    m_mappedNotes[channelIn][channelOut][(note>>3)] &= ~(1<<(note&0b111))

#define MAPPED_NOTE_IS_ON(channelIn, channelOut, note) \
    ((m_mappedNotes[channelIn][channelOut][(note>>3)] & (1<<(note&0b111))) == (1<<(note&0b111)))


const ControllerInfo ControllerInfo::Invalid;
const BindingInfo BindingInfo::Invalid;

ChannelMapping::ChannelMapping() :
    keyLow(0),
    keyHigh(0),
    active(false),
    octaveShift(0),
    transformMode(0)
{
}

FP4Qt::FP4Qt(const char *clientName, QObject *parent) :
    QObject(parent),
    FP4(clientName),
    m_channelMappingsEnabled(false)
{
    memset(m_mappedNotes, 0, sizeof(m_mappedNotes));

    ChannelTransformFactory ctf(this);
    m_channelTransforms = ctf.channelTransforms();
    m_channelTransformNames = ctf.channelTransformNames();

    loadDefaultMappings();
}

/* return widget associated to a channel+cc */
QWidget *FP4Qt::controlledWidget(int channel, int cc) {
    return m_ccBindings.value(ControllerInfo(channel, cc));
}

/* return channel and cc for a known widget or {-1, -1} */
ControllerInfo FP4Qt::controlledWidgetInfo(QWidget *widget) {
    QMapIterator< ControllerInfo, QWidget* > it(m_ccBindings);
    while (it.hasNext()) {
        it.next();
        if (it.value() == widget) {
            return it.key();
        }
    }

    return ControllerInfo::Invalid;
}

/* load default channel mappings, ie: map whole range from incoming channel
   to the same outgoing channel */
void FP4Qt::loadDefaultMappings() {
    for (int inChannel=0; inChannel<16; ++inChannel) {
        for (int outChannel=0; outChannel<16; ++outChannel) {
            ChannelMapping* mapping = &m_mappings[inChannel][outChannel];
            mapping->octaveShift = 0;
            mapping->keyLow = FP4_LOWEST_KEY;
            mapping->keyHigh = FP4_HIGHEST_KEY;
            mapping->active = (inChannel == outChannel);
            mapping->transformMode = 0;
        }
    }
}

/* get mapping between inChannel and outChannel */
ChannelMapping *FP4Qt::channelMapping(int inChannel, int outChannel) {
    Q_ASSERT(inChannel >= 0 && inChannel < 16);
    Q_ASSERT(outChannel >= 0 && outChannel < 16);
    return &m_mappings[inChannel][outChannel];
}

/* if enabled, m_mappings will be used to route incoming note{on,off} messages */
void FP4Qt::enableChannelMappings(bool enable) {
    m_channelMappingsEnabled = enable;
}

/* return true if channel mappings are used */
bool FP4Qt::channelMappingsEnabled() const {
    return m_channelMappingsEnabled;
}

/* update mapping info for existing binding */
void FP4Qt::updateBinding(const ControllerInfo &controller, const BindingInfo &binding) {
    if (!m_bindingConfigMap.contains(controller)) {
        qDebug("UpdateBinding called on non-exising binding");
        return;
    }

    m_bindingConfigMap[controller] = binding;
    emit bindingUpdated(controller, binding);
}


/* when a widget is associated to a midibindingbutton, it is registered so
   bindings can be effectuated when settings are loaded */
void FP4Qt::registerBindableWidget(QWidget *widget) {
    Q_ASSERT(!widget->property("cc_group").isNull());
    Q_ASSERT(!widget->property("cc_name").isNull());

    QString group = widget->property("cc_group").value<QString>();
    QString name = widget->property("cc_name").value<QString>();

    m_bindableWidgets[group][name] = widget;
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterBindableWidget(QObject*)));

    // FIXME: we should really use a bidirectional map structure for efficiency. Then again,
    //        there will never be many bindings to search.
    for (auto it=m_bindingConfigMap.constBegin(); it!=m_bindingConfigMap.constEnd(); ++it) {
        if (it.value().group == group && it.value().name == name) {
            m_ccBindings.insert(it.key(), widget);
            connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetDeleted(QObject*)));
            break;
        }
    }
}

/* when a bindable widget is destroyed, be sure to ignore it later on. */
void FP4Qt::unregisterBindableWidget(QObject* obj) {
    QWidget* widget = qobject_cast<QWidget*>(obj);
    Q_ASSERT(widget);
    Q_ASSERT(!widget->property("cc_group").isNull());
    Q_ASSERT(!widget->property("cc_name").isNull());

    QString group = widget->property("cc_group").value<QString>();
    QString name = widget->property("cc_name").value<QString>();

    QMap<QString, QMap<QString, QWidget*> >::iterator it=m_bindableWidgets.find(group);
    if (it == m_bindableWidgets.end()) {
        return;
    }

    it->remove(name);
    if (it->size() == 0) {
        m_bindableWidgets.remove(group);
    }
}

/* Relay incoming note on events to FP4, and let other objects react to them. */
void FP4Qt::onNoteOn(int channel, int note, int velocity) {
    if (m_channelMappingsEnabled) {
        handleMappedNoteOn(channel, note, velocity);
    }
    else {
        emit noteOnReceived(channel, note, velocity);
        sendNoteOn(channel, note, velocity);
    }
}

/* Relay incoming note off events to FP4 and let other objects react to them.
   Notes are removed from the m_notes table, see onNoteOn comment. */
void FP4Qt::onNoteOff(int channel, int note) {
    if (m_channelMappingsEnabled) {
        handleMappedNoteOff(channel, note);
    }
    else {
        emit noteOffReceived(channel, note);
        sendNoteOff(channel, note);
    }
}

/* Let other objects react to program changes. */
void FP4Qt::onProgramChange(int channel, int pgm) {
//    qDebug() << "FP4: Program change on channel " << channel << ": " << hex << pgm << dec;
    emit programChangeReceived(channel, pgm);
}

/* handle controller events.
   Let other objects react to bank changes.
   Update widgets associated to controller events.
   Let other objects react to other controller events.
*/
void FP4Qt::onController(int channel, int cc, int value) {
    if (cc == 0) {
        snd_seq_event_t* ev;

        int err=snd_seq_event_input(m_seq, &ev);
        if (err == -EAGAIN || err==-ENOSPC) {
            qDebug() << "FP4: incomplete bank change";
            return;
        }

        if (ev->type != SND_SEQ_EVENT_CONTROLLER) {
            qDebug() << "FP4: expected another control change after control change with control==0";
            qDebug() << "     Missed one event.";
            return;
        }

        qDebug() << "FP4: bank change on channel " << channel << ": " << ((value << 7) + ev->data.control.value) << " (" <<
                hex << value << ", " << ev->data.control.value << ")" << dec;

        emit bankChangeReceived(channel, value, ev->data.control.value);
    }
    else {
//        qDebug() << "FP4: Controller on channel " << channel << ": " << controller << "=" << value;
        ControllerInfo controller(channel, cc);
        QWidget* widget = m_ccBindings.value(controller, 0);
        if (widget) {
            Q_ASSERT(m_bindingConfigMap.contains(controller));
            const BindingInfo& binding = m_bindingConfigMap.value(controller);

            value = (int)((float)value * (((float)binding.maxValue - (float)binding.minValue)/127.f));
            if (binding.reversed) {
                value = binding.maxValue - value;
            }
            else {
                value += binding.minValue;
            }

            updateBoundWidget(widget, value);

            // emit modified value
            emit ccReceived(channel, cc, value);
        }
        else {
            sendController(channel, cc, value);
            emit ccReceived(channel, cc, value);
        }

    }
}

/* Handle received sysexes.
   Let other objects react to identity responses and other events separately. */
void FP4Qt::onSysEx(const unsigned char* data, int length) {
    if (data[0] != 0xf0) {
        qDebug() << "FP4: received invalid sysex message.";
        return;
    }

    switch(data[1]) {
    case 0x7e:
        if (data[3] == 0x06 && data[4] == 0x02) {
            onIdentityResponse(data, length);
        }
        break;
    default:
        qDebug() << "FP4: received unknown sysex message:" << endl << "    ";
        dumpSysEx(data, length);
        emit sysexReceived(data, length);
        break;
    }
}

/* display identity response blurb */
void FP4Qt::onIdentityResponse(const unsigned char* data, int length) {
    Q_UNUSED(length);

    qDebug() << "FP4: identity response." <<
            " Manufacturer: " << QString("%1").arg((int)data[5], 2, 16, QChar('0')) <<
            " Family: " << QString("%1").arg((int)*(uint16_t *)(&data[6]), 4, 16, QChar('0')) <<
            " Model: " << QString("%1").arg((int)*(uint16_t *)(&data[8]), 4, 16, QChar('0')) <<
                " Version: " << QString("%1").arg((int)*(uint32_t *)(&data[10]), 8, 16, QChar('0'));
}

/* let other objects react to initial connection */
void FP4Qt::onConnect() {
    emit connected();
}

/* let other objects react to subsequent connections (after a disconnection) */
void FP4Qt::onReconnect() {
    emit reconnected();
}

/* let other objects react to disconnections */
void FP4Qt::onDisconnect() {
    emit disconnected();
}

void FP4Qt::onClientConnect(int client_id, int port) {
    emit clientConnected(client_id, port);
}

void FP4Qt::onClientDisconnect(int client_id, int port) {
    emit clientDisconnected(client_id, port);
}

/* register a controller binding: bind a controller message (channel+cc) to a widget
   that will be updated on incoming CC events. */
void FP4Qt::addControllerBinding(QWidget *widget, int channel, int cc) {
    QString group = widget->property("cc_group").value<QString>();
    Q_ASSERT(!group.isEmpty());
    QString name = widget->property("cc_name").value<QString>();
    Q_ASSERT(!name.isEmpty());

    m_ccBindings.insert(ControllerInfo(channel, cc), widget);

    BindingInfo oldBinding;
    ControllerInfo controller(channel, cc);
    bool replace = m_bindingConfigMap.contains(controller);
    if (replace) {
         oldBinding = m_bindingConfigMap.value(controller);
    }

    BindingInfo binding(group, name);
    m_bindingConfigMap.insert(controller, binding );
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetDeleted(QObject*)));

    if (replace) {
        emit bindingRemoved(controller, oldBinding);
    }

    emit bindingAdded(controller, binding);
}

void FP4Qt::addControllerBinding(const ControllerInfo& controller, const BindingInfo& binding) {
    Q_ASSERT(!binding.group.isEmpty());
    Q_ASSERT(!binding.name.isEmpty());

    BindingInfo oldBinding;
    bool replace = m_bindingConfigMap.contains(controller);
    if (replace) {
        oldBinding = m_bindingConfigMap.value(controller);
    }

    m_bindingConfigMap[controller] = binding;

    // bind to widget if present
    if (m_bindableWidgets.contains(binding.group) && m_bindableWidgets[binding.group].contains(binding.name)) {
        QWidget* widget = m_bindableWidgets[binding.group][binding.name];
        m_ccBindings.insert(controller, widget);
        connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetDeleted(QObject*)));
    }

    if (replace) {
        emit bindingRemoved(controller, oldBinding);
    }

    emit bindingAdded(controller, binding);
}


/* add or replace binding */
void FP4Qt::updateControllerBinding(QWidget *widget, int channel, int cc) {
    QString group = widget->property("cc_group").value<QString>();
    Q_ASSERT(!group.isEmpty());
    QString name = widget->property("cc_name").value<QString>();
    Q_ASSERT(!name.isEmpty());

    // this replaces widget for channel+cc
    m_ccBindings.insert(ControllerInfo(channel, cc), widget);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetDeleted(QObject*)));

    BindingInfo binding(group, name, 0, 127, false);
    BindingInfo oldBinding;
    ControllerInfo controller(channel, cc);
    bool replace = m_bindingConfigMap.contains(controller);
    if (replace) {
        oldBinding = m_bindingConfigMap.value(controller);
        binding.minValue = oldBinding.minValue;
        binding.maxValue = oldBinding.maxValue;
        binding.reversed = oldBinding.reversed;
    }

    m_bindingConfigMap.insert(controller, binding );

    if (replace) {
        emit bindingUpdated(controller, binding);
    }
    else {
        emit bindingAdded(controller, binding);
    }
}

/* delete a controller binding */
void FP4Qt::deleteControllerBinding(QWidget *widget) {
    QMutableMapIterator< ControllerInfo, QWidget* > it(m_ccBindings);
    while (it.hasNext()) {
        it.next();
        const ControllerInfo &controller = it.key();
        if (it.value() == widget) {
            QString group = it.value()->property("cc_group").value<QString>();
            QString name = it.value()->property("cc_name").value<QString>();
            it.remove();
            BindingInfo binding = m_bindingConfigMap.value(controller);
            m_bindingConfigMap.remove(controller);
            emit bindingRemoved(controller, binding);
            break;
        }
    }
}

/* delete a controller binding */
void FP4Qt::deleteControllerBinding(int channel, int cc) {
    ControllerInfo controller(channel, cc);
    m_ccBindings.remove(controller);

    BindingInfo binding = m_bindingConfigMap.value(controller);
    m_bindingConfigMap.remove(controller);

    emit bindingRemoved(controller, binding);
}

/* update a widget when an incoming cc event is received.
   QCheckBoxes, QComboBoxes, QSliders and QLabels can be updated.
   value is between 0 and 127
*/
void FP4Qt::updateBoundWidget(QWidget *widget, int value) {
    QCheckBox* cb = qobject_cast<QCheckBox*>(widget);
    if (cb) {
        cb->setChecked(value > 63);
        return;
    }

    QComboBox* combo = qobject_cast<QComboBox*>(widget);
    if (combo) {
        int idx = (float)value/127.0 * (combo->count()-1);
        combo->setCurrentIndex(idx);
        return;
    }

    QSlider* slider = qobject_cast<QSlider*>(widget);
    if (slider) {
        int idx = slider->minimum() + (float)value/127.0 * (slider->maximum()-slider->minimum());
        slider->setValue(idx);
        return;
    }

    QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget);
    if (spinBox) {
        int idx = slider->minimum() + (float)value/127.0 * (slider->maximum()-slider->minimum());
        slider->setValue(idx);
        return;
    }

    QPushButton* button = qobject_cast<QPushButton*>(widget);
    if (button) {
        // buttons are a bit more complicated as we don't want to
        // call click() everytime we get a controller message larger than 63.
        // So we only call click() when the controller value reaches a threshold
        // and wait for the value to go below a lower threshold before
        // we can activate the button again. Two thresholds are used for
        // debouncing.
        QVariant stateVariant = button->property("midi_push");
        if (!stateVariant.isValid() || !stateVariant.value<bool>()) {
            // was not pressed but is now
            if (value > 85) {
                button->click();
                button->setDown(true);
                button->setProperty("midi_push", true);
            }
        }
        else {
            // was pressed, but is not anymore
            if (value < 42) {
                button->setDown(false);
                button->setProperty("midi_push", false);
            }
        }

        return;
    }

    QLabel* label = qobject_cast<QLabel*>(widget);
    if (label) {
        label->setText(QString("%1").arg(value));
        return;
    }

    qDebug() << "Got controller message for unsupported widget type.";
}

/* When a widget is deleted, remove its cc binding */
void FP4Qt::onWidgetDeleted(QObject *obj) {
    QWidget* widget = qobject_cast<QWidget*>(obj);
    if (!obj) {
        return;
    }

    QMutableMapIterator< ControllerInfo, QWidget* > it(m_ccBindings);
    while (it.hasNext()) {
        it.next();
        if (it.value() == widget) {
            it.remove();
            break;
        }
    }
}

/* delete all bindings */
void FP4Qt::clearBindings() {
    m_ccBindings.clear();
    m_bindingConfigMap.clear();
    emit bindingsCleared();
}

/* Process incoming noteon if channel mappings are in use. Channel mapping
   transform modes that ignore noteoff events keep track of the played notes in
   m_mappedNotes. FP4::m_notes cannot be used, because multiple input ranges
   may map to the same output channel, with another channel mapping transform
   mode applied. */
void FP4Qt::handleMappedNoteOn(int channelIn, int note, int velocity) {
    for (int channelOut=0; channelOut<16; ++channelOut) {
        // check if there is a mapping from channelIn to channelOut, and if
        // played note falls in that range
        ChannelMapping* mapping = channelMapping(channelIn, channelOut);
        if (!mapping->active)
            continue;
        if (note < mapping->keyLow || note > mapping->keyHigh)
            continue;

        // emit note on for mapped note, ignoring octave shift
        emit noteOnReceived(channelOut, note, velocity);

        // is the note still valid after octaveShift ?
        note += 12 * mapping->octaveShift;
        if (note < 0)
            continue;
        if (note > 127)
            continue;

        Q_ASSERT(mapping->transformMode >= 0 && mapping->transformMode < m_channelTransforms.count());
        m_channelTransforms[mapping->transformMode]->handleNoteOn(mapping, channelIn, channelOut, note, velocity);
    }
}

/* handle noteoff event on a mapped channel */
void FP4Qt::handleMappedNoteOff(int channelIn, int note) {
    for (int channelOut=0; channelOut<16; ++channelOut) {
        // check if there is a mapping from channelIn to channelOut, and if
        // played note falls in that range
        ChannelMapping* mapping = channelMapping(channelIn, channelOut);
        if (!mapping->active)
            continue;
        if (note < mapping->keyLow || note > mapping->keyHigh)
            continue;

        // emit noteoff for mapped note, ignoring octave shift
        emit noteOffReceived(channelOut, note);

        // is the note still valid after octaveShift ?
        note += 12 * mapping->octaveShift;
        if (note < 0)
            continue;
        if (note > 127)
            continue;

        Q_ASSERT(mapping->transformMode >= 0 && mapping->transformMode < m_channelTransforms.count());
        m_channelTransforms[mapping->transformMode]->handleNoteOff(mapping, channelIn, channelOut, note);
    }
}

/* send noteoff messages to hardware for all the notes marked as being
   played in m_mappedNotes for the specified channels and unmark them. */
void FP4Qt::mappedNotesOff(int channelIn, int channelOut) {
    const ChannelMapping* mapping = channelMapping(channelIn, channelOut);

    int low = mapping->keyLow + 12 * mapping->octaveShift;
    int high = mapping->keyHigh + 12 * mapping->octaveShift;
    if (low < 0) low = 0;
    if (low > 127) low = 127;
    if (high < 0) high = 0;
    if (high > 127) high = 127;

    for (int i=low; i<=high; ++i) {
        if (MAPPED_NOTE_IS_ON(channelIn, channelOut, i)) {
            UNREGISTER_MAPPED_NOTE(channelIn, channelOut, i);
            sendNoteOff(channelOut, i);
        }
    }
}

bool FP4Qt::isMappedNoteOn(int channelIn, int channelOut, int note) {
    return MAPPED_NOTE_IS_ON(channelIn, channelOut, note);
}

void FP4Qt::registerMappedNote(int channelIn, int channelOut, int note) {
    REGISTER_MAPPED_NOTE(channelIn, channelOut, note);
}

void FP4Qt::unregisterMappedNote(int channelIn, int channelOut, int note) {
    UNREGISTER_MAPPED_NOTE(channelIn, channelOut, note);
}

