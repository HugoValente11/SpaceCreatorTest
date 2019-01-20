TEMPLATE = lib
TARGET = mscwidgets

include(../../esa.pri)

CONFIG += static

QT += core gui widgets

HEADERS += \
    baseitems/common/abstractinteractiveobject.h \
    baseitems/common/arrowsign.h \
    baseitems/common/drawrectinfo.h \
    baseitems/common/highlightrectitem.h \
    baseitems/common/objectanchor.h \
    baseitems/common/objectslink.h \
    baseitems/common/utils.h \
    baseitems/arrowitem.h \
    baseitems/grippoint.h \
    baseitems/grippointshandler.h \
    baseitems/instanceenditem.h \
    baseitems/interactiveobject.h \
    baseitems/labeledarrowitem.h \
    baseitems/objectslinkitem.h \
    baseitems/textitem.h \
    commands/common/commandids.h \
    commands/common/commandsfactory.h \
    commands/common/commandsstack.h \
    commands/basecommand.h \
    commands/cmdactioninformaltext.h \
    commands/cmdactionitemmove.h \
    commands/cmdconditionitemmove.h \
    commands/cmdconditionitemresize.h \
    commands/cmdentitynamechange.h \
    commands/cmdinstanceitemcreate.h \
    commands/cmdinstanceitemmove.h \
    commands/cmdinstanceitemresize.h \
    commands/cmdinstancekindchange.h \
    commands/cmdmessageitemmove.h \
    commands/cmdmessageitemcreate.h \
    commands/cmdmessageitemresize.h \
    tools/basecreatortool.h \
    tools/basetool.h \
    tools/hierarchycreatortool.h \
    tools/instancecreatortool.h \
    tools/messagecreatortool.h \
    tools/pointertool.h \
    tools/tooltypes.h \
    actionitem.h \
    chartviewmodel.h \
    conditionitem.h \
    documentitem.h \
    documentitemmodel.h \
    graphicsview.h \
    hierarchyviewmodel.h \
    instanceitem.h \
    messageitem.h \

SOURCES += \
    baseitems/common/arrowsign.cpp \
    baseitems/common/drawrectinfo.cpp \
    baseitems/common/highlightrectitem.cpp \
    baseitems/common/objectanchor.cpp \
    baseitems/common/objectslink.cpp \
    baseitems/common/utils.cpp \
    baseitems/arrowitem.cpp \
    baseitems/grippoint.cpp \
    baseitems/grippointshandler.cpp \
    baseitems/instanceenditem.cpp \
    baseitems/interactiveobject.cpp \
    baseitems/labeledarrowitem.cpp \
    baseitems/objectslinkitem.cpp \
    baseitems/textitem.cpp \
    commands/common/commandsfactory.cpp \
    commands/common/commandsstack.cpp \
    commands/basecommand.cpp \
    commands/cmdactioninformaltext.cpp \
    commands/cmdactionitemmove.cpp \
    commands/cmdconditionitemmove.cpp \
    commands/cmdconditionitemresize.cpp \
    commands/cmdentitynamechange.cpp \
    commands/cmdinstanceitemcreate.cpp \
    commands/cmdinstanceitemmove.cpp \
    commands/cmdinstanceitemresize.cpp \
    commands/cmdinstancekindchange.cpp \
    commands/cmdmessageitemcreate.cpp \
    commands/cmdmessageitemmove.cpp \
    commands/cmdmessageitemresize.cpp \
    tools/basecreatortool.cpp \
    tools/basetool.cpp \
    tools/instancecreatortool.cpp \
    tools/hierarchycreatortool.cpp \
    tools/messagecreatortool.cpp \
    tools/pointertool.cpp \
    chartviewmodel.cpp \
    actionitem.cpp \
    conditionitem.cpp \
    documentitem.cpp \
    documentitemmodel.cpp \
    graphicsview.cpp \
    hierarchyviewmodel.cpp \
    instanceitem.cpp \
    messageitem.cpp \

BuildType =release
CONFIG(debug, debug|release) {
    BuildType =debug
} # Profiling, etc (if any) are defaulted to Release

win32 {
    LIBS += -L$$OUT_PWD/../msclibrary/$$BuildType/ -lmsclibrary
    MSC_LIB_NAME=msclibrary.lib
    win32-g++: MSC_LIB_NAME=libmsclibrary.a
    PRE_TARGETDEPS += $$OUT_PWD/../msclibrary/$$BuildType/$$MSC_LIB_NAME
} else:unix {
    LIBS += -L$$OUT_PWD/../msclibrary/ -lmsclibrary
    PRE_TARGETDEPS += $$OUT_PWD/../msclibrary/libmsclibrary.a
}

INCLUDEPATH += $$PWD/../msclibrary
DEPENDPATH += $$PWD/../msclibrary

RESOURCES += \
    resources.qrc
