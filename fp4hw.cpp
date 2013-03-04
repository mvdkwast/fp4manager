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

#include "fp4hw.h"
#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <stdio.h>

/* Data structure to keep track of alsa ports */

AlsaClientInfo::AlsaClientInfo(int client, int port, const char* name) :
    m_client(client),
    m_port(port)
{
    m_name = strdup(name);
}

AlsaClientInfo::AlsaClientInfo(const AlsaClientInfo& other) {
    m_client = other.m_client;
    m_port = other.m_port;
    m_name = strdup(other.m_name);
}

AlsaClientInfo::~AlsaClientInfo() {
    delete m_name;
}

/*-------------------------------------------------------------------------------*/

FP4::FP4(const char* client_name) :
    m_autoreconnect(false),
    m_autoclient_id(0),
    m_autoclient(0),
    m_autoport(-1),
    m_outputEnabled(true),
    m_wasConnected(false),
    m_traceMode(0)
{
    m_client_name = strdup(client_name);

    clearKeyStateBuffer();

    openClient();
    openSystem();

    m_input_id = -1;
    m_input_port = 0;
}

FP4::~FP4() {
    closeClient();

    if (m_autoclient) {
        delete m_autoclient;
    }

    delete m_client_name;
}

bool FP4::open(int port) {
    return open(FP4_CLIENT_NAME, port);
}

bool FP4::open(const char *client_name, int port) {
    int client_id = resolveClientName(client_name, Readable);
    return open(client_id, port);
}

bool FP4::open(int client_id, int port) {
    if (!openInput(client_id, port)) {
        return false;
    }

    if (!openOutput(client_id, port)) {
        closeInput();
        return false;
    }

    if (!m_wasConnected) {
        onConnect();
    }
    else {
        onReconnect();
    }

    m_wasConnected = true;

    return true;
}

void FP4::close() {
    closeInput();
    closeOutput();
}

bool FP4::openSecondary(int client_id, int port, PortType dir) {
    if (dir == Readable) {
        return !snd_seq_connect_from( m_seq, m_hin, client_id, port);
    }
    else {
        return !snd_seq_connect_to(m_seq, m_hout, client_id, port);
    }
}

void FP4::closeSecondary(int client_id, int port, PortType dir) {
    if (dir == Readable) {
        snd_seq_disconnect_from(m_seq, m_hin, client_id, port);
    }
    else {
        snd_seq_disconnect_to(m_seq, m_hout, client_id, port);
    }
}

bool FP4::openInput(int client_id, int port) {
    closeInput();

    if (snd_seq_connect_from( m_seq, m_hin, client_id, port )) {
        return false;
    }

    m_input_id = client_id;
    m_input_port = port;

    return true;
}

bool FP4::openInput(const char* client_name, int port) {
    int client_id = resolveClientName(client_name, Readable);
    // cout << "client name \"" << client_name << "\" resolves to " << client_id << endl;
    if (client_id < 0) {
        return false;
    }

    return openInput( client_id, port );
}

void FP4::closeInput() {
    if (m_input_id >= 0) {
        snd_seq_disconnect_from(m_seq, m_hin, m_input_id, m_input_port);
        m_input_id = -1;
        m_input_port = 0;
    }
}

bool FP4::openOutput(int client_id, int port) {
    closeOutput();

    // 1 is the output port
    if (snd_seq_connect_to(m_seq, m_hout, client_id, port)) {
        return false;
    }

    m_output_id = client_id;
    m_output_port = port;
    return true;
}

bool FP4::openOutput(const char* client_name, int port) {
    int client_id = resolveClientName(client_name, Writable);
    if (client_id < 0) {
        return false;
    }

    return openOutput(client_id, port);
}

void FP4::closeOutput() {
    if (m_output_id >= 0) {
        snd_seq_disconnect_from(m_seq, m_hout, m_output_id, m_output_port);
        m_output_id = -1;
        m_output_port = 0;
    }
}

void FP4::trace(FP4::TraceCategory category, const char *format, ...) {
    if (category & m_traceMode) {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        fprintf(stderr, "\n");
    }
}

// return -1 if not found
int FP4::resolveClientName(const char* client_name, PortType type) {
    vector< AlsaClientInfo > clients = getPortList(type);

    for (vector< AlsaClientInfo >::const_iterator client=clients.begin();
         client != clients.end();
         ++client )
    {
        if (!strcmp(client->name(), client_name)) {
            return client->client();
        }
    }

    return -1;
}

vector< AlsaClientInfo > FP4::getPortList(PortType type) {
    vector< AlsaClientInfo > clients;

    uint perm = 0;
    switch ( type ) {
    case Writable:
        perm = SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE;
        break;

    case Readable:
    default:
        perm = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
        break;
    }

    snd_seq_client_info_t *cinfo;
    snd_seq_port_info_t *pinfo;

    snd_seq_client_info_alloca( &cinfo );
    snd_seq_port_info_alloca( &pinfo );
    snd_seq_client_info_set_client( cinfo, -1 );

    while (snd_seq_query_next_client( m_seq, cinfo ) >= 0 ) {
        snd_seq_port_info_set_client( pinfo, snd_seq_client_info_get_client( cinfo ) );
        snd_seq_port_info_set_port( pinfo, -1 );

        while ( snd_seq_query_next_port( m_seq, pinfo ) >= 0 ) {

            // ignore ports we opened
            if ( snd_seq_client_info_get_client( cinfo ) == m_client_id ) {
                continue;
            }

            // ignore system and bogus ports
            if ( snd_seq_client_info_get_client(cinfo) <= 0 ) {
                continue;
            }

            if ( ( ( snd_seq_port_info_get_capability( pinfo ) & perm ) != perm ) ||
                 ( snd_seq_port_info_get_capability( pinfo ) & SND_SEQ_PORT_CAP_NO_EXPORT ) )
            {
                continue;
            }

            clients.push_back(AlsaClientInfo(
                                  snd_seq_client_info_get_client( cinfo ),
                                  snd_seq_port_info_get_port( pinfo ),
                                  snd_seq_client_info_get_name( cinfo ) )
                              );
        }
    }

    return clients;
}

void FP4::dumpPortList(PortType type) {
    vector< AlsaClientInfo > clients = getPortList(type);

    for (vector< AlsaClientInfo >::const_iterator client=clients.begin();
         client != clients.end();
         ++client )
    {
        cout << client->client() << ":" << client->port() << " (" << client->name() << ")" << endl;
    }
}

void FP4::enableAutoReconnect(int client, int port) {
    if (m_autoclient) {
        delete m_autoclient;
        m_autoclient = 0;
    }

    m_autoreconnect = true;
    m_autoclient_id = client;
    m_autoport = port;
}

void FP4::enableAutoReconnect(const char* client_name, int port) {
    m_autoreconnect = true;
    m_autoport = port;

    if (m_autoclient) delete m_autoclient;
    m_autoclient = strdup(client_name);
}

void FP4::disableAutoReconnect( void ) {
    m_autoreconnect = false;
}

void FP4::openClient() {
    int err;

    // create sequencer
    err = snd_seq_open( &m_seq, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK );
    if (err < 0) {
        fprintf( stderr, "Cannot open ALSA sequencer" );
        exit( 0 );
    }

    snd_seq_set_client_name( m_seq, m_client_name );

    m_client_id = snd_seq_client_id( m_seq );

    // create ports
    m_hin = snd_seq_create_simple_port(
                m_seq,
                "In",
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                SND_SEQ_PORT_TYPE_APPLICATION );

    m_hout = snd_seq_create_simple_port(
                m_seq,
                "Out",
                SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_APPLICATION );
}

void FP4::closeClient() {
    if ( m_seq ) {
        snd_seq_close( m_seq );
    }

    m_seq = 0;
}

// This creates a port that listens to system messages like subscriptions/unsubscriptions.
// It will not appear in qjackctl or aconnect
// This allows the connection to be restored automatically (setAutoReconnect)
void FP4::openSystem() {
    snd_seq_port_subscribe_t *subs;
    snd_seq_addr_t tmp_addr;

    int portid = snd_seq_create_simple_port(
                m_seq,
                "System update monitor",
                SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
                SND_SEQ_PORT_TYPE_APPLICATION );

    if (portid < 0) {
        return;
    }

    snd_seq_port_subscribe_alloca( &subs );
    tmp_addr.client = SND_SEQ_CLIENT_SYSTEM;
    tmp_addr.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
    snd_seq_port_subscribe_set_sender( subs, &tmp_addr );
    tmp_addr.client = snd_seq_client_id( m_seq );
    tmp_addr.port = portid;
    snd_seq_port_subscribe_set_dest( subs, &tmp_addr );
    snd_seq_subscribe_port( m_seq, subs );
}

void FP4::processEvents() {
    snd_seq_event_t* ev_in;

    do {
        int err=snd_seq_event_input( m_seq, &ev_in );
        if (err == -EAGAIN) {
            // nothing to read from input fifo
            return;
        }

        if (err == -ENOSPC) {
            // fifo is full
            cerr << "FP4: seq FIFO input buffer full. Events are lost." << endl;
        }

        // if (ev_in->type != SND_SEQ_EVENT_CLOCK)
        //     cout << "<- event " << (int)ev_in->type << endl;

        switch ( ev_in->type ) {
        case SND_SEQ_EVENT_CONTROLLER: {
            int cc_channel = ev_in->data.control.channel;
            int cc_in = ev_in->data.control.param;
            int cc_value = ev_in->data.control.value;
            onController(cc_channel, cc_in, cc_value);
            break;
        }

        case SND_SEQ_EVENT_NOTEON: {
            int note_channel = ev_in->data.note.channel;
            int note_value = ev_in->data.note.note;
            int note_velocity = ev_in->data.note.velocity;
            onNoteOn(note_channel, note_value, note_velocity);
            break;
        }

        case SND_SEQ_EVENT_NOTEOFF: {
            int note_channel = ev_in->data.note.channel;
            int note_value = ev_in->data.note.note;
            onNoteOff(note_channel, note_value);
            break;
        }

        case SND_SEQ_EVENT_PGMCHANGE:
            onProgramChange(ev_in->data.control.channel, ev_in->data.control.value);
            break;

        case SND_SEQ_EVENT_PORT_SUBSCRIBED: {
            snd_seq_connect_t c = ev_in->data.connect;
            cerr << "FP4: " << (int)c.sender.client << ":" << (int)c.sender.port
                 << " connected to " << (int)c.dest.client << ":" << (int)c.dest.port << endl;
            break;
        }

        case SND_SEQ_EVENT_PORT_UNSUBSCRIBED: {
            snd_seq_connect_t c = ev_in->data.connect;
            cerr << "FP4: " << (int)c.sender.client << ":" << (int)c.sender.port
                 << " disconnected from " << (int)c.dest.client << ":" << (int)c.dest.port << endl;
            break;
        }

        case SND_SEQ_EVENT_PORT_START: {
            snd_seq_addr_t a = ev_in->data.addr;

            if (m_autoreconnect && a.port == m_autoport) {
                if (m_autoclient) {
                    snd_seq_client_info_t* cinfo;
                    snd_seq_client_info_alloca(&cinfo);

                    snd_seq_get_any_client_info(m_seq, a.client, cinfo);
                    const char* client_name = snd_seq_client_info_get_name(cinfo);

                    if (!strcmp(client_name, m_autoclient)) {
                        cerr << "FP4: reconnected." << endl;
                        open(a.client, a.port);
                    }
                }
                else {
                    if (a.client == m_autoclient_id && a.port == m_autoport) {
                        cerr << "FP4: reconnected." << endl;
                        open(a.client, a.port);
                    }
                }
            }

            onClientConnect(a.client, a.port);

            break;
        }

        case SND_SEQ_EVENT_PORT_EXIT: {
            snd_seq_addr_t a = ev_in->data.addr;
            if (m_autoreconnect && a.client == m_input_id && a.port == m_input_port) {
                cerr << "FP4: Main input disconnected." << endl;
                m_input_id = -1;
                m_input_port = -1;
                onDisconnect();
            }

            onClientDisconnect(a.client, a.port);
            break;
        }

        case SND_SEQ_EVENT_CLIENT_START: {
            snd_seq_addr_t a = ev_in->data.addr;
            cerr << "FP4: " << (int)a.client << ":" << (int)a.port << " Client start" << endl;
            break;
        }

        case SND_SEQ_EVENT_CLIENT_EXIT: {
            snd_seq_addr_t a = ev_in->data.addr;
            cerr << "FP4: " << (int)a.client << ":" << (int)a.port << " Client exit" << endl;
            break;
        }

        case SND_SEQ_EVENT_CLIENT_CHANGE: {
            snd_seq_addr_t a = ev_in->data.addr;
            cerr << "FP4: " << (int)a.client << ":" << (int)a.port << " Client change" << endl;
            break;
        }

        case SND_SEQ_EVENT_CLOCK:
            // ignore clock events
            break;

        case SND_SEQ_EVENT_SYSEX:
            onSysEx((unsigned char*)ev_in->data.ext.ptr, ev_in->data.ext.len);
            break;

        default:
            cerr << "FP4: Unknown ALSA event: " << (int)ev_in->type << endl;
            break;
        }

        snd_seq_free_event(ev_in);

    } while (snd_seq_event_input_pending(m_seq, 0) > 0);
} 

void FP4::sendProgramChange(int channel, int program) {
    if (m_outputEnabled) {
        trace(TraceProgramChanges, ">> PGM CHANGE channel: %i program: %i", channel, program);
        snd_seq_event_t ev;
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_pgmchange(&ev, channel, program);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

void FP4::sendBankChange(int channel, int msb, int lsb) {
    if (m_outputEnabled) {
        trace(TraceProgramChanges, ">> BANK CHANGE channel: %i bank: %i %i", channel, msb, lsb);
        snd_seq_event_t ev;
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_controller(&ev, channel, 0, msb);
        snd_seq_event_output_direct(m_seq, &ev);

        snd_seq_ev_set_controller(&ev, channel, 32, lsb);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

void FP4::sendNoteOn(int channel, int note, int velocity) {
    if (m_outputEnabled) {
        trace(TraceNotes, ">> NOTE ON channel: %i note: %i velocity: %i", channel, note, velocity);

        if (isKeyPressed(channel, note)) {
            cerr << "Duplicate note on." << endl;
            sendNoteOff(channel, note);
            //        return;
        }

        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_noteon(&ev, channel, note, velocity);
        snd_seq_event_output(m_seq, &ev);
        snd_seq_drain_output(m_seq);

        registerKeyPress(channel, note);
    }
}

void FP4::sendNoteOff(int channel, int note) {
    if (m_outputEnabled) {
        trace(TraceNotes, ">> NOTE OFF channel: %i note: %i", channel, note);

        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_noteoff(&ev, channel, note, 0);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);

        registerKeyRelease(channel, note);
    }
}

void FP4::sendController(int channel, int cc, int value) {
    if (m_outputEnabled) {
        trace(TraceNotes, ">> CTL channel: %i cc: %i value: %i", channel, cc, value);

        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_controller(&ev, channel, cc, value);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

void FP4::sendControllerHires(int channel, int cc, int value) {
    if (m_outputEnabled) {
        int lsb = value & 0x7f;
        int msb = (value>>7) & 0x7f;
        sendController(channel, cc, msb);
        sendController(channel, cc+0x20, lsb);
    }
}

// pitch is between -8192 and +8192
void FP4::sendPitchChange(int channel, int pitch) {
    if (m_outputEnabled) {
        trace(TracePitchBends, ">> PITCH BEND channel=%i pitch=%i", channel, pitch);

        if (pitch < -8192) pitch=-8192;
        if (pitch > 8191) pitch=8191;

        // Doesn't seem to work
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_pitchbend(&ev, channel, pitch);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

// change pitch bend range (0-24 semitones)
void FP4::sendPitchRange(int channel, int range) {
    if (!m_outputEnabled)
        return;

    trace(TracePitchBends, ">> PITCH BEND RANGE channel: %i range: %i", channel, range);

    if (range < 0) range=0;
    if (range > 24) range=24;

    uint8_t data[] = {
        (uint8_t)(0xb0 + channel),  // controller change
        0x64, 00,                   // rpn low
        0x65, 00,                   // rpn high
        0x06, (uint8_t)range,       // data entry1
        0x26, 0,                    // data entry2
        0x64, 0x7f,                 // rpn low reset
        0x65, 0x7f                  // rpn high reset
    };

    if (m_outputEnabled) {
        sendBytes(data, sizeof(data));
    }
}

// send a channel pressure message to channel
void FP4::sendChannelPressure(int channel, int pressure) {
    if (m_outputEnabled) {
        trace(TraceChannelPressure, ">> CHANNEL PRESSURE channel: %i pressure: %i", channel, pressure);
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_chanpress(&ev, channel, pressure);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

// notes after portamento control will be glided to.
void FP4::sendPortamentoControl(int channel, int note) {
    if (m_outputEnabled) {
        trace(TracePortamento, ">> PORTAMENTO CONTROL channel: %i note: %i", channel, note);
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_controller(&ev, channel, 84, note);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

// all currently sounding notes are turned off
void FP4::sendAllSoundsOff(int channel) {
    if (m_outputEnabled) {
        trace(TraceChannelControl, ">> SOUNDS OFF channel: %i", channel);
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_controller(&ev, channel, 120, 0);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

// all playing notes will be turned off, except those held with
// hold or sostenuto
void FP4::sendAllNotesOff(int channel) {
    if (m_outputEnabled) {
        trace(TraceChannelControl, ">> NOTES OFF channel: %i", channel);
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_controller(&ev, channel, 120, 0);
        snd_seq_event_output_direct(m_seq, &ev);
        snd_seq_drain_output(m_seq);
    }
}

// send raw midi bands to hardware
void FP4::sendBytes(unsigned char data[], unsigned int length) {
    if (m_outputEnabled) {
        trace(TraceSysex, ">> SYSEX bytes: %i", length);
        snd_seq_event_t ev;
        snd_seq_ev_set_source(&ev, m_input_port);
        snd_seq_ev_set_dest(&ev, m_output_id, m_output_port);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_sysex(&ev, length, data);
        snd_seq_event_output_direct(m_seq, &ev);
    }
}

void FP4::sendRPN(int channel, int msb, int lsb, int value) {
    if (m_outputEnabled) {
        sendController(channel, 100, lsb);
        sendController(channel, 101, msb);
        sendController(channel, 6, value);
    }
}

void FP4::sendNRPN(int channel, int msb, int lsb, int value) {
    if (m_outputEnabled) {
        sendController(channel, 98, lsb);
        sendController(channel, 99, msb);
        sendController(channel, 6, value);
    }
}

void FP4::sendRPNHires(int channel, int msb, int lsb, int value) {
    if (m_outputEnabled) {
        sendController(channel, 100, lsb);
        sendController(channel, 101, msb);
        sendController(channel, 6, (value>>7)&0x7f);
        sendController(channel, 38, value&0x7f);
    }
}

void FP4::sendNRPNHires(int channel, int msb, int lsb, int value) {
    if (m_outputEnabled) {
        sendController(channel, 98, lsb);
        sendController(channel, 99, msb);
        sendController(channel, 6, (value>>7)&0x7f);
        sendController(channel, 38, value&0x7f);
    }
}

void FP4::sendLocalControl(int channel, bool on) {
    if (m_outputEnabled) {
        trace(TraceChannelControl, ">> LOCAL CONTROL channel: %i value: %i", channel, on);
        sendController(channel, 122, on);
    }
}

void FP4::sendIdentityRequest() {
    if (m_outputEnabled) {
        unsigned char idR[] = {
            0xf0,
            0x7e,  // universal non realtime
            0x7f,  // disregard channel
            0x06,  // general information
            0x01,  // identity request
            0xf7 };
        sendBytes(idR, 6);
    }
}

void FP4::sendData(unsigned char MSB, unsigned char FSB, unsigned char LSB, unsigned char data[], unsigned int length) {
    if (!m_outputEnabled) {
        return;
    }

    trace(TraceSystem, ">> DT1 address: %02x %02x %02x bytes: %i", MSB, FSB, LSB, length);

    unsigned char ss[] = {
        0xf0,
        0x41, // roland
        0x10, // device id
        0x42, // model (GS)
        0x12, // command (data transfer)
    };

    unsigned int bufLen = 8 + length + 1 + 1; // length of total sysex message
    unsigned char* buf = new unsigned char[bufLen];
    memcpy(buf, ss, 5);
    buf[5] = MSB;
    buf[6] = FSB;
    buf[7] = LSB;

    memcpy(&buf[8], data, length);

    unsigned int checksum = 0;
    checksum += MSB + FSB + LSB;
    for (unsigned i=0; i<length; ++i) {
        checksum += data[i];
    }
    checksum = 128 - (checksum % 128);

    buf[bufLen-2] = checksum;
    buf[bufLen-1] = 0xf7;

    sendBytes(buf, bufLen);

//    dumpSysEx(buf, bufLen);

    delete[] buf;
}

void FP4::sendGM1On() {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> GM1 On");
    unsigned char ss[] = { 0xf0, 0x7e, 0x7f, 0x09, 0x01, 0xf7 };
    sendBytes(ss, 6);
}

void FP4::sendGM2On() {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> GM2 On");
    unsigned char ss[] = { 0xf0, 0x7e, 0x7f, 0x09, 0x03, 0xf7 };
    sendBytes(ss, 6);
}

void FP4::sendGMOff() {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> GM Off");
    unsigned char ss[] = { 0xf0, 0x7e, 0x7f, 0x09, 0x02, 0xf7 };
    sendBytes(ss, 6);
}

void FP4::sendGSReset() {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> GS Reset");
    unsigned char ss[] = {
        0xf0,
        0x41,			// roland
        0x10,			// device id
        0x42,			// model (GS)
        0x12,			// command (DT1)
        0x40, 0x00, 0x7f,	// address
        0x00,			// data
        0x41,			// checksum
        0xf7 };
    sendBytes(ss, 11);
}

void FP4::sendMasterVolume(int volume) {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> MASTER VOLUME volume: %i", volume);
    unsigned char ss[] = { 0xf0, 0x7f, 0x7f, 0x04, 0x01, 0x00, /* vol msb */ 0x00, 0xf7 };
    if (volume < 0) volume = 0;
    if (volume > 127) volume = 127;

    ss[6] = (unsigned char)volume;
    sendBytes(ss, 8);
}

// This needs GM2 to be enabled. Use sendSystemReverb* when possible.
void FP4::sendGlobalReverb(GM2ReverbType type, int time) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB (GM2) type: %i time: %i", type, time);
    unsigned char ss[] = {
        0xf0,
        0x7f,
        0x7f,
        0x04, // device control
        0x05, // global parameter control
        0x01, // slot path length
        0x01, // parameter id width
        0x01, // value width
        0x01, // slot path msb
        0x01, // slot path lsb (effect: reverb)
        0x00, // (param num) type
        0x00, // (param value) time
        0xf7 };

    ss[10] = type;
    ss[11] = time;
    sendBytes(ss, 13);
}

void FP4::sendSystemPanning(int panning) {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> MASTER PANNING panning: %i", panning);
    unsigned char data = (unsigned char)(panning + 63 + 1);
    sendData(0x40, 0x00, 0x06, &data, 1);
}

void FP4::sendSystemKeyShift(int tuning) {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> MASTER PANNING key shift: %i", tuning);
    unsigned char data = (unsigned char)(tuning + 24);
    sendData(0x40, 0x00, 0x05, &data, 1);
}

void FP4::sendMasterCoarseTuning(int semiTones) {
    if (!m_outputEnabled)
        return;

    trace(TraceSystem, ">> MASTER COARSE TUNING semitones: %i", semiTones);
    unsigned char sx[] = { 0xf0,
                           0x7f, // universal realtime
                           0x7f, // device id (broadcast)
                           0x04, // sub id#1 (device control)
                           0x04, // sub id#2 (device control)
                           0x00, // lsb param
                           0x00, // msb param
                           0xf7 };

    semiTones += 24 + 0x28;
    sx[6] = semiTones;
    sendBytes(sx, 8);
}

void FP4::sendSystemReverbMacro(ReverbType type) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> SYSTEM REVERB MACRO type: %i", type);
    unsigned char t = (unsigned  char)type;
    sendData(0x40, 0x01, 0x30, &t, 1);
}

void FP4::sendSystemReverb(ReverbType type, int preLPF, int level, int time, int delay) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> SYSTEM REVERB type: %i preLPF: %i level: %i time: %i delay: %i",
          type, preLPF, level, time, delay);

    unsigned char data[5];
    data[0] = (unsigned char)type;
    data[1] = (unsigned char)preLPF;
    data[2] = (unsigned char)level;
    data[3] = (unsigned char)time;
    data[4] = (unsigned char)delay;
    sendData(0x40, 0x01, 0x31, data, 5);
}

void FP4::sendSystemReverbCharacter(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB CHARACTER value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x31, &data, 1);
}

void FP4::sendSystemReverbPreLPF(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB PRE-LPF value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x32, &data, 1);
}

void FP4::sendSystemReverbLevel(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB LEVEL value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x33, &data, 1);
}

void FP4::sendSystemReverbTime(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB TIME value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x34, &data, 1);
}

void FP4::sendSystemReverbFeedback(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> REVERB FEEDBACK value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x35, &data, 1);
}

void FP4::sendSystemChorusMacro(ChorusType type) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS MACRO type: %i", type);
    unsigned char t = (unsigned char)type;
    sendData(0x40, 0x01, 0x38, &t, 1);
}

void FP4::sendSystemChorus(int preLPF, int level, int feedback, int delay, int rate, int depth, int sendToReverb) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS prelpf: %i level: %i feedback: %i, delay: %i, rate: %i, depth: %i, sendToReverb: %i",
          preLPF, level, feedback, delay, rate, depth, sendToReverb);
    unsigned char data[7];
    data[0] = (unsigned char)preLPF;
    data[1] = (unsigned char)level;
    data[2] = (unsigned char)feedback;
    data[3] = (unsigned char)delay;
    data[4] = (unsigned char)rate;
    data[5] = (unsigned char)depth;
    data[6] = (unsigned char)sendToReverb;
    sendData(0x40, 0x01, 0x39, data, 7);
}

void FP4::sendSystemChorusPreLPF(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS PRE-LPF value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x39, &data, 1);
}

void FP4::sendSystemChorusLevel(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS LEVEL value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3a, &data, 1);
}

void FP4::sendSystemChorusFeedBack(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS FEEDBACK value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3b, &data, 1);
}

void FP4::sendSystemChorusDelay(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS DELAY value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3c, &data, 1);
}

void FP4::sendSystemChorusRate(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS RATE value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3d, &data, 1);
}

void FP4::sendSystemChorusDepth(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS DEPTH value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3e, &data, 1);
}

void FP4::sendSystemChorusToReverbLevel(int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> CHORUS TO REVERB LEVEL value: %i", value);
    unsigned char data = value;
    sendData(0x40, 0x01, 0x3f, &data, 1);
}

void FP4::sendEffectEnabled(int channel, bool enabled, int msb, int lsb, int control1, int control2) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT ENABLED channel: %i enabled: %i effect msb: %i effect lsb: %i ctl1: %i ctl2: %i",
          channel, enabled, msb, lsb, control1, control2);

    if (enabled) {
        uint8_t data2[6] = { (uint8_t)msb, (uint8_t)lsb, 0x40, (uint8_t)127, (uint8_t)control1, (uint8_t)control2 };
        sendData(0x40, 0x41+channel, 0x23, data2, 6);
    }
    else {
        uint8_t data2[6] = { 0, 0, 0, 0, 0, 0 };
        sendData(0x40, 0x41+channel, 0x23, data2, 6);
    }
}

void FP4::sendEffectParameters(uint8_t *data, int parameterCount) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT PARAMETERS");
    sendData(0x40, 0x03, 0x00, data, 3+parameterCount);
}

void FP4::sendEffectParameter(int index, int value) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT SINGLE PARAMETER index: %i value: %i", index, value);
    uint8_t data = (uint8_t)value;
    sendData(0x40, 0x03, 0x03 + index, &data, 1);
}

void FP4::sendEffectParameters(int msb, int lsb, int *values, int parameterCount) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT PARAMETERS msb: %i lsb: %i parameterCount: %i", msb, lsb, parameterCount);

    uint8_t data[23];
    if (parameterCount > 20) parameterCount=20;

    uint8_t* d = data;
    *d++ = (uint8_t)msb;
    *d++ = (uint8_t)lsb;
    *d++ = (uint8_t)0;

    for (int i=0; i<parameterCount; ++i) {
        *d++ = data[i];
    }

    sendEffectParameters(data, parameterCount);
}

void FP4::sendEffectToReverbLevel(int level) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT TO REVERB level: %i", level);
    uint8_t data = (uint8_t)level;
    sendData(0x40, 0x03, 0x17, &data, 1);
}

void FP4::sendEffectToChorusLevel(int level) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT TO CHORUS level: %i", level);
    uint8_t data = (uint8_t)level;
    sendData(0x40, 0x03, 0x18, &data, 1);
}

void FP4::sendEffectWetLevel(int level) {
    if (!m_outputEnabled)
        return;

    trace(TraceEffects, ">> EFFECT WET level: %i", level);
    uint8_t data = (uint8_t)level;
    sendData(0x40, 0x03, 0x1a, &data, 1);
}

/* Pressed notes are stored in FP4::m_notes because the FP4 seems to send duplicate noteon events
(or maybe it's alsa?) resulting in stuck notes. It is also used in the case multiple keyboards send
notes on the same channel. If a note is marked as played, don't resend it until a noteoff is sent */

void FP4::registerKeyPress(int channel, int note) {
    m_notes[channel*(128/8) + (note>>3)] |= (uint8_t)(1<<(note&0b111));
}

void FP4::registerKeyRelease(int channel, int note) {
    m_notes[channel*(128/8) + (note>>3)] &= ~(uint8_t)(1<<(note&0b111));
}

void FP4::clearKeyStateBuffer() {
    memset(m_notes, 0, sizeof(m_notes));
}

bool FP4::isKeyPressed(int channel, int note) {
    return (m_notes[channel*(128/8) + (note>>3)] & (1<<(note&0b111))) == (1<<(note&0b111));
}

/* Virtual methods to react to incoming MIDI events. */

void FP4::onNoteOn(int channel, int note, int velocity) {
    (void)velocity;
    (void)note;
    (void)channel;
}

void FP4::onNoteOff(int channel, int note) {
    (void)channel;
    (void)note;
}

void FP4::onProgramChange(int channel, int pgm) {
    (void)channel;
    (void)pgm;
}

void FP4::onController(int channel, int controller, int value) {
#if 0
    if (controller == 0) {
        snd_seq_event_t* ev;

        int err=snd_seq_event_input(seq, &ev);
        if (err == -EAGAIN || err==-ENOSPC) {
            cerr << "FP4: incomplete bank change" << endl;
            return;
        }

        if (ev->type != SND_SEQ_EVENT_CONTROLLER) {
            cerr << "FP4: expected another control change after control change with control==0" << endl;
            cerr << "     Missed one event." << endl;
            return;
        }

        cout << "FP4: bank change on channel " << channel << ": " << ((value << 7) + ev->data.control.value) << " (" <<
                hex << value << ", " << ev->data.control.value << ")" << dec << endl;

    }
    else {
        cout << "FP4: Controller on channel " << channel << ": " << controller << "=" << value << endl;
    }
#endif
    (void)channel;
    (void)controller;
    (void)value;
}

void FP4::onSysEx(const unsigned char* data, int length) {
    if (data[0] != 0xf0) {
        cerr << "FP4: recieved invalid sysex message." << endl;
        return;
    }

    switch(data[1]) {
    case 0x7e:
        if (data[3] == 0x06 && data[4] == 0x02) {
            onIdentityResponse(data, length);
        }
        break;
    default:
        cerr << "FP4: received unknown sysex message:" << endl << "    ";
        dumpSysEx(data, length);
        break;
    }
}

void FP4::onIdentityResponse(const unsigned char* data, int length) {
    cout << hex << setw(2) << setfill('0');

    cout << "FP4: identity response." <<
            " Manufacturer: " << (int)data[5] <<
            " Family: " << *(uint16_t *)(&data[6]) <<
            " Model: " << *(uint16_t *)(&data[8]) <<
            " Version: " << *(uint32_t *)(&data[10]) << endl;

    cout << dec;
}

void FP4::onConnect() {
}

void FP4::onReconnect() {
}

void FP4::onDisconnect() {
}

void FP4::onClientConnect(int client_id, int port) {
    (void)client_id;
    (void)port;
}

void FP4::onClientDisconnect(int client_id, int port) {
    (void)client_id;
    (void)port;
}

void FP4::dumpSysEx(const unsigned char* data, int length) {
    cerr << hex << setw(2) << setfill('0');

    for (int i=0; i<length; ++i) {
        cerr << setw(2) << (int)data[i] << " ";
    }

    cerr << endl;
    cerr << dec;
}
