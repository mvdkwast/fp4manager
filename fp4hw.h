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

/*
 * FP4 - Process MIDI events from an ALSA MIDI source
 *
 * SYNOPSIS
 *   #include <unistd.h>
 *   #include "fp4.h"
 *
 *   class MidiTest : public FP4 {
 *   public:
 *       void onNoteOn(int channel, int note, int velocity) {
 *           // do something with this data
 *       }
 *   };
 *
 *   int main() {
 *       MidiTest midi;
 *       midi.open(24, 0);
 *
 *       while(true) {
 *           midi.processEvents();
 *           usleep(30000);
 *       }
 *   }
 *
 * USAGE
 *
 * Use FP4 as a base class, and override the following methods:
 * - onNoteOn(int channel, int note, int velocity)
 * - onNoteOff(int channel, int note)
 * - onController(int channel, int ctl, int value)
 *
 * First connect to a MIDI source using open(client_id, client_port)
 * or by name using open(client_name, client_port).
 *
 * Call processEvents regulary to get MIDI events (for instance in an idle function)
 *
 * FP4 can automatically reconnect to a MIDI source if this 
 * is enabled with setAutoReconnect(client_id, client_port) or 
 * setAutoReconnect(client_name, client_port). Note that autoreconnection
 * by client_id instead of name may not be reliable since ALSA does not 
 * always affect the same client_id to a device.
 *
 * To display a list of ports FP4 can connect to, you can use
 * dumpPortList().
 *
 * Finally a debugging class that displays incoming events is provided
 * as FP4Debug.
 *
 */

#ifndef __STILGAR_ALSA_H
#define __STILGAR_ALSA_H

#include <alsa/asoundlib.h>
#include <string.h>
#include <vector>
#include <inttypes.h>

#define ALSA_CLIENT_NAME "Stilgar Midi In"

#define FP4_CLIENT_NAME "Roland FP Series"

// GM2 reverb control
enum GM2ReverbType {
    GM2ReverbSmallRoom,
    GM2ReverbMediumRoom,
    GM2ReverbLargeRoom,
    GM2ReverbMediumHall,
    GM2ReverbLargeHall,
    GM2ReverbPlate = 8
};

// GM2 chorus control
enum GM2ChorusType {
    GM2Chorus1,
    GM2Chorus2,
    GM2Chorus3,
    GM2Chorus4,
    GM2FBChorus,
    GM2ChorusFlanger
};

// GM2 controllers
enum GM2Controller {
    GM2PitchControl,
    GM2FilterCutoffControl,
    GM2AmplitudeControl,
    GM2LFOPitchDepthControl,
    GM2LFOFilterDepthControl,
    GM2LFOAmplitudeDepthControl
};
    
// memory map reverb settings
enum ReverbType {
    ReverbRoom,
    ReverbRoom2,
    ReverbRoom3,
    ReverbHall1,
    ReverbHall2,
    ReverbPlate,
    ReverbDelay,
    ReverbPanningDelay
};

// memory mp chorus settings
enum ChorusType {
    Chorus1,
    Chorus2,
    Chorus3,
    Chorus4,
    ChorusFeedback,
    ChorusFlanger,
    ChorusShortDelay,
    ChorusShortDelayFB
};

using namespace std;

class AlsaClientInfo {
public:
    AlsaClientInfo(int client, int port, const char* name);
    AlsaClientInfo(const AlsaClientInfo& other);
    ~AlsaClientInfo();

    int client() const { return m_client; }
    int port() const { return m_port; }
    const char* name() const { return m_name; }

private:
    int m_client;
    int m_port;
    char* m_name;
};

class FP4 {
public:
    enum PortType {
        Readable,
        Writable
    };

    enum TraceCategory {
        TraceNotes = 1,
        TraceControllers = 2,
        TraceProgramChanges = 4,
        TracePitchBends = 8,
        TraceChannelPressure = 16,
        TraceChannelControl = 32,
        TracePortamento = 64,
        TraceEffects = 256,
        TraceSysex = 512,
        TraceSystem = 1024,
        TraceConnections = 4096,

        TraceAll = -1
    };

    FP4(const char* client_name = ALSA_CLIENT_NAME);
    ~FP4();

    // primary connection. handles autoconnect.
    bool open(const char* client_name, int port=0);
    bool open(int m_client_id, int port=0);
    bool open(int port=0);
    void close();

    // secondary connections
    bool openSecondary(int m_client_id, int port, PortType dir);
    void closeSecondary(int m_client_id, int port, PortType dir);

    void processEvents();

    void enableOutput() { m_outputEnabled=true; }
    void disableOutput() { m_outputEnabled=false; }

    int resolveClientName(const char* client_name, PortType type);

    vector< AlsaClientInfo > getPortList(PortType type=Readable);
    void dumpPortList(PortType type=Readable);

    int inputId(void) const { return m_input_id; }
    int inputPort(void) const { return m_input_port; }

    void enableAutoReconnect(int client, int port);
    void enableAutoReconnect(const char* client_name, int port);
    void disableAutoReconnect(void);

    void dumpSysEx(const unsigned char* data, int len);

    // send to FP4
    void sendProgramChange(int channel, int program);
    void sendBankChange(int channel, int msb, int lsb);
    void sendNoteOn(int channel, int note, int velocity);
    void sendNoteOff(int channel, int note);
    void sendController(int channel, int cc, int value);
    void sendControllerHires(int channel, int cc, int value);
    void sendPitchChange(int channel, int pitch);
    void sendPitchRange(int channel, int range);
    void sendChannelPressure(int channel, int pressure);
    void sendPortamentoControl(int channel, int note);
    void sendAllSoundsOff(int channel);
    void sendAllNotesOff(int channel);
    void sendBytes(unsigned char data[], unsigned int length);
    void sendRPN(int channel, int msb, int lsb, int value);
    void sendNRPN(int channel, int msb, int lsb, int value);
    void sendRPNHires(int channel, int msb, int lsb, int value);
    void sendNRPNHires(int channel, int msb, int lsb, int value);

    void sendIdentityRequest();

    // channel mode messages
    //void sendAllSoundsOff(int channel);
    //void sendResetAllControllers(int channel);
    void sendLocalControl(int channel, bool localOn);
    //void sendAllNotesOff(int channel);
    //void sendOmniOff(int channel);
    //void sendOmniOn(int channel);
    //void sendMonophonic(int channel);
    //void sendPolyphonic(int channel);

    // -- FP4 specifics --
    void sendData(unsigned char addrMSB, unsigned char addr, unsigned char addLSB, unsigned char data[], unsigned int length);

    void sendGM1On();
    void sendGM2On();
    void sendGMOff();
    void sendGSReset();

    void sendMasterVolume(int volume); // 0-127
    // void sendMasterFineTuning(int tuning);
    void sendMasterCoarseTuning(int semiTones); // between -24 and +24

    // only in GM2 mode
    void sendGlobalReverb(GM2ReverbType type, int time /* 0-127 */);
    // void sendGlobalChorus(GM2ChorusType, int modRate, int modDepth, int feedback, int sendToReverb); // all 0-127
    // void sendGlobalChannelPressure(GM2Controller control, int range); // all 0-127 except control==Pitch: 0x28-0x58
    // void sendGlobalController
    // void sendGlobalTuningAdjust
    // void sendGlobalKeyBasedInstrumentController
    
    // system memory map access
    void sendSystemPanning(int value); // -63 - 0 - 63
    void sendSystemKeyShift(int value); // -24 - 0 - +24

    void sendSystemReverbMacro(ReverbType type);
    void sendSystemReverb(ReverbType type, int preLPF /* (4) 0-7 */, int level, int time, int delay);
    void sendSystemReverbCharacter(int value);
    void sendSystemReverbPreLPF(int value);
    void sendSystemReverbLevel(int value);
    void sendSystemReverbTime(int value);
    void sendSystemReverbFeedback(int value);

    void sendSystemChorusMacro(ChorusType type);
    void sendSystemChorus(int preLPF /* (0) 0-7 */, int level /* 0-12 */, int feedback, int delay, int rate, int depth, int sendToReverb);
    void sendSystemChorusPreLPF(int value);
    void sendSystemChorusLevel(int value);
    void sendSystemChorusFeedBack(int value);
    void sendSystemChorusDelay(int value);
    void sendSystemChorusRate(int value);
    void sendSystemChorusDepth(int value);
    void sendSystemChorusToReverbLevel(int value);

    void sendEffectEnabled(int channel, bool enabled, int msb, int lsb, int control1=1, int control2=0x7f);
    void sendEffectParameters(uint8_t* data, int parameterCount);
    void sendEffectParameter(int index, int value);
    void sendEffectParameters(int msb, int lsb, int* values, int parameterCount);
    void sendEffectToReverbLevel(int level);
    void sendEffectToChorusLevel(int level);
    void sendEffectWetLevel(int level);

    // keep track of currently played notes
    void registerKeyPress(int channel, int note);
    void registerKeyRelease(int channel, int note);
    void clearKeyStateBuffer();
    bool isKeyPressed(int channel, int note);

    // set trace level
    void setTraceMode(int mode) { m_traceMode = mode; }

    // react to messages from FP4
    virtual void onNoteOn(int channel, int note, int velocity);
    virtual void onNoteOff(int channel, int note);
    virtual void onProgramChange(int channel, int pgm);
    virtual void onController(int channel, int controller, int value);
    virtual void onSysEx(const unsigned char* data, int len);
    virtual void onIdentityResponse(const unsigned char* data, int len);
    virtual void onConnect();
    virtual void onReconnect();
    virtual void onDisconnect();

    // any client connections
    virtual void onClientConnect(int m_client_id, int port);
    virtual void onClientDisconnect(int m_client_id, int port);

    // accessors
    snd_seq_t* getSequencer() const { return m_seq; }

protected:
    bool openInput(int m_client_id, int port);
    bool openInput(const char* client_name, int port);
    void closeInput();

    bool openOutput(int m_client_id, int port);
    bool openOutput(const char* client_name, int port);
    void closeOutput();

    void trace(TraceCategory category, const char* format, ...);

private:
    void openClient(void);
    void closeClient(void);
    void openSystem(void);

protected:
    snd_seq_t* m_seq;

private:
    int	m_hin;
    int m_hout;

    int m_input_id;
    int m_input_port;

    int m_output_id;
    int m_output_port;
	
    int m_client_id;
    char* m_client_name;

    bool m_autoreconnect;
    int m_autoclient_id;
    const char* m_autoclient;
    int m_autoport;

    bool m_outputEnabled;
    bool m_wasConnected;

private:
    uint8_t m_notes[16 * 128/8];

    int m_traceMode;
};

#endif
