set(LIB_NAME ivcore)

add_library(${LIB_NAME} STATIC)

target_sources(${LIB_NAME} PRIVATE
    ivcommonprops.cpp
    ivcommonprops.h
    ivconnectionchain.cpp
    ivconnectionchain.h
    ivinterfacechain.cpp
    ivinterfacechain.h
    ivnamevalidator.cpp
    ivnamevalidator.h
    ivobject.cpp
    ivobject.h
    ivcomment.cpp
    ivcomment.h
    ivmyfunction.h
    ivmyfunction.cpp
    ivconnection.cpp
    ivconnection.h
    ivconnectiongroup.cpp
    ivconnectiongroup.h
    ivfunction.cpp
    ivfunction.h
    ivfunctiontype.cpp
    ivfunctiontype.h
    ivinterface.cpp
    ivinterface.h
    ivinterfacegroup.cpp
    ivinterfacegroup.h
    ivmodel.cpp
    ivmodel.h
    parameter.cpp
    parameter.h
    ivtestutils.cpp
    ivtestutils.h
    ivxmlreader.cpp
    ivxmlreader.h
    connectioncreationvalidator.cpp
    connectioncreationvalidator.h
    propertytemplate.cpp
    propertytemplate.h
    propertytemplateconfig.cpp
    propertytemplateconfig.h
    ivlibrary.cpp
    ivlibrary.h
    ivcoreutils.cpp
    ivcoreutils.h
    ivresources.qrc
)

target_include_directories(${LIB_NAME} PUBLIC .)

target_link_libraries(${LIB_NAME}
    asn1editor
    asn1library
    shared
    ${QT_CORE}
    ${QT_XML}
)

source_group("Resource Files" REGULAR_EXPRESSION ".*.qrc$")
