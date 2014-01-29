SOURCES += \
    main.cpp \
    fp4win.cpp \
    fp4effect.cpp \
    fp4instr.cpp \
    fp4hw.cpp \
    instrumentwidget.cpp \
    controllerwidget.cpp \
    fp4qt.cpp \
    fp4controller.cpp \
    midibindbutton.cpp \
    controllerbinding.cpp \
    autoconnectwidget.cpp \
    gssendwidget.cpp \
    bindingmanagerwidget.cpp \
    splitswindow.cpp \
    keyboardwidget.cpp \
    keyboardrangewidget.cpp \
    musictheory.cpp \
    channeltransform.cpp \
    channelswindow.cpp \
    controllergeneratorwindow.cpp \
    fp4managerapplication.cpp \
    performancewindow.cpp \
    keyboardrangeeditdialog.cpp \
    instrumentselectdialog.cpp \
    window.cpp \
    preferences.cpp \
    preferenceswindow.cpp \
    themeicon.cpp \
    resetbutton.cpp \
    reverbwidget.cpp \
    parameterswidget.cpp \
    choruswidget.cpp \
    parameterwidgetbuilder.cpp \
    effectwidget.cpp \
    masterwidget.cpp \
    abstractcontrollerswidget.cpp \
    pedalswidget.cpp \
    soundparameterswidget.cpp \
    vibratowidget.cpp \
    portamentowidget.cpp \
    sendswidget.cpp \
    pitchwidget.cpp \
    channelpressuregenerator.cpp \
    controllergenerator.cpp \
    controllerkeysgenerator.cpp \
    keytimegenerator.cpp \
    voicinggenerator.cpp \
    chordselecterdialog.cpp

HEADERS += \
    fp4win.h \
    fp4effect.h \
    fp4instr.h \
    fp4fxlist.h \
    fp4hw.h \
    config.h \
    instrumentwidget.h \
    controllerwidget.h \
    fp4qt.h \
    fp4controller.h \
    midibindbutton.h \
    controllerbinding.h \
    autoconnectwidget.h \
    gssendwidget.h \
    bindingmanagerwidget.h \
    splitswindow.h \
    keyboardwidget.h \
    keyboardrangewidget.h \
    musictheory.h \
    channeltransform.h \
    channelswindow.h \
    controllergeneratorwindow.h \
    fp4managerapplication.h \
    fp4constants.h \
    performancewindow.h \
    keyboardrangeeditdialog.h \
    instrumentselectdialog.h \
    window.h \
    preferences.h \
    preferenceswindow.h \
    themeicon.h \
    resetbutton.h \
    reverbwidget.h \
    parameterswidget.h \
    choruswidget.h \
    parameterwidgetbuilder.h \
    effectwidget.h \
    masterwidget.h \
    abstractcontrollerswidget.h \
    pedalswidget.h \
    soundparameterswidget.h \
    vibratowidget.h \
    portamentowidget.h \
    sendswidget.h \
    pitchwidget.h \
    channelpressuregenerator.h \
    controllergenerator.h \
    controllerkeysgenerator.h \
    keytimegenerator.h \
    voicinggenerator.h \
    chordselecterdialog.h

QMAKE_CXXFLAGS += -std=c++0x
LIBS += -lasound
QT += widgets

OTHER_FILES += \
    NOTES.txt \
    files.txt

RESOURCES += \
    resources.qrc

