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

#ifndef CONFIG_H
#define CONFIG_H

#define APP_ORGANISATION "Stilgar's software"
#define APP_ORGANISATION_DOMAIN "vdkwast.com"
#define APP_TITLE "FP4 Manager"
#define APP_VERSION "0.0.1"
#define APP_ICON ":/icons/piano.png"

#define BINDING_PRESETS_FILE "bindings.ini"
#define FX_LIBRARY_FILE "effects.ini"
#define SPLIT_PRESETS_FILE "splits.ini"
#define REVERB_PRESETS_FILE "reverb.ini"
#define CHORUS_PRESETS_FILE "chorus.ini"
#define FILTER_PRESETS_FILE "filter.ini"
#define VIBRATO_PRESETS_FILE "vibrato.ini"
#define CHORD_PRESETS_FILE "chords.ini"
#define SCALE_PRESETS_FILE "scales.ini"

#define CONFIG_FILE_EXTENSION "fp4config"
#define TIMELINE_FILE_EXTENSION "fp4timeline"
#define DEFAULT_DATA_PATH ".fp4manager"

// time messages are displayed in statusbar in ms
#define STATUSBAR_TIMEOUT 2000

// minimum timer interval for dynamic effects in ms
#define MIN_TIMER_INTERVAL 20 

#endif // CONFIG_H
