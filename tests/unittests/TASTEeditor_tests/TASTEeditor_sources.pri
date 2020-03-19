include(tests_common.pri)

HEADERS += \
        $$TASTE_PRO_ROOT/app/commandlineparser.h \
        $$TASTE_PRO_ROOT/app/commandsstack.h \
        $$TASTE_PRO_ROOT/app/common.h \
        $$TASTE_PRO_ROOT/app/context/action/action.h \
        $$TASTE_PRO_ROOT/app/context/action/actionsmanager.h \
        $$TASTE_PRO_ROOT/app/context/action/attrhandler.h \
        $$TASTE_PRO_ROOT/app/context/action/condition.h \
        $$TASTE_PRO_ROOT/app/context/action/editor/actionsmodel.h \
        $$TASTE_PRO_ROOT/app/context/action/editor/attributesmodel.h \
        $$TASTE_PRO_ROOT/app/context/action/editor/conditiondialog.h \
        $$TASTE_PRO_ROOT/app/context/action/editor/conditionsmodel.h \
        $$TASTE_PRO_ROOT/app/context/action/editor/dynactioneditor.h \
        $$TASTE_PRO_ROOT/app/context/action/extprocmonitor.h \
        $$TASTE_PRO_ROOT/app/datatypes/basicdatatype.h \
        $$TASTE_PRO_ROOT/app/datatypes/datatypesstorage.h \
#        $$TASTE_PRO_ROOT/app/mainwindow.h \
        $$TASTE_PRO_ROOT/app/xmldocexporter.h \
        $$TASTE_PRO_ROOT/app/zoomcontroller.h \
        $$TASTE_PRO_ROOT/baseitems/common/abstractinteractiveobject.h \
        $$TASTE_PRO_ROOT/baseitems/common/drawrectinfo.h \
        $$TASTE_PRO_ROOT/baseitems/common/highlightrectitem.h \
        $$TASTE_PRO_ROOT/baseitems/common/utils.h \
        $$TASTE_PRO_ROOT/baseitems/graphicsview.h \
        $$TASTE_PRO_ROOT/baseitems/grippoint.h \
        $$TASTE_PRO_ROOT/baseitems/grippointshandler.h \
        $$TASTE_PRO_ROOT/baseitems/interactiveobject.h \
        $$TASTE_PRO_ROOT/baseitems/textgraphicsitem.h \
        $$TASTE_PRO_ROOT/document/abstracttabdocument.h \
        $$TASTE_PRO_ROOT/document/documentsmanager.h \
        $$TASTE_PRO_ROOT/document/tabdocumentfactory.h \
        $$TASTE_PRO_ROOT/dummy/dummytabdocument.h \
#        $$TASTE_PRO_ROOT/logging/logcategory.h \
#        $$TASTE_PRO_ROOT/logging/loghandler.h \
        $$TASTE_PRO_ROOT/templating/abstracttemplatedobject.h \
        $$TASTE_PRO_ROOT/templating/stringtemplate.h \
        $$TASTE_PRO_ROOT/templating/exportedaadlconnection.h \
        $$TASTE_PRO_ROOT/templating/exportedaadlfunction.h \
        $$TASTE_PRO_ROOT/templating/exportedaadliface.h \
        $$TASTE_PRO_ROOT/templating/exportedaadlobject.h \
        $$TASTE_PRO_ROOT/templating/exportedaadlproperty.h \
        $$TASTE_PRO_ROOT/templating/templateeditor.h \
        $$TASTE_PRO_ROOT/templating/templatehighlighter.h \
        $$TASTE_PRO_ROOT/templating/xmlhighlighter.h \
        $$TASTE_PRO_ROOT/templating/templatesyntaxhelpdialog.h \
#        $$TASTE_PRO_ROOT/reports/bugreportdialog.h \
#        $$TASTE_PRO_ROOT/reports/bugreporthandler.h \
        $$TASTE_PRO_ROOT/settings/appoptions.h \
        $$TASTE_PRO_ROOT/settings/settingsmanager.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlcommonprops.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlnamevalidator.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobject.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectcomment.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectconnection.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectfunction.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectfunctiontype.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectiface.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectsmodel.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlparameter.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadltabdocument.h \
        $$TASTE_PRO_ROOT/tab_aadl/aadlxmlreader.h \
        $$TASTE_PRO_ROOT/tab_concurrency/concurrencytabdocument.h \
        $$TASTE_PRO_ROOT/tab_data/datatabdocument.h \
        $$TASTE_PRO_ROOT/tab_deployment/deploymenttabdocument.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlcommentgraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlconnectiongraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctiongraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctionnamegraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctiontypegraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlinterfacegraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/aadlrectgraphicsitem.h \
        $$TASTE_PRO_ROOT/tab_interface/commenttextdialog.h \
        $$TASTE_PRO_ROOT/tab_interface/colors/colorhandlereditor.h \
        $$TASTE_PRO_ROOT/tab_interface/colors/colormanager.h \
        $$TASTE_PRO_ROOT/tab_interface/colors/colormanagerdialog.h \
        $$TASTE_PRO_ROOT/tab_interface/colors/colorselectorbutton.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcommentitemcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdconnectionitemcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparameterchange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparametercreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparameterremove.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityautolayout.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityattributechange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitygeometrychange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertychange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertycreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertyremove.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertyrename.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityremove.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdfunctionitemcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdfunctiontypeitemcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifacedatachangebase.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparambase.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamremove.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamchange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceattrchange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdinterfaceitemcreate.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdrequiredifacepropertychange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdrootentitychange.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/commandids.h \
        $$TASTE_PRO_ROOT/tab_interface/commands/commandsfactory.h \
        $$TASTE_PRO_ROOT/tab_interface/connectioncreationvalidator.h \
        $$TASTE_PRO_ROOT/tab_interface/creatortool.h \
        $$TASTE_PRO_ROOT/tab_interface/graphicsitemhelpers.h \
        $$TASTE_PRO_ROOT/tab_interface/interfacetabdocument.h \
        $$TASTE_PRO_ROOT/tab_interface/interfacetabgraphicsscene.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/adddynamicpropertydialog.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/contextparametersmodel.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/comboboxdelegate.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/functionattrdelegate.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/interfaceattrdelegate.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/propertytypedelegate.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicproperty.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicpropertyconfig.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicpropertymanager.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/ifaceparametersmodel.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesdialog.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertieslistmodel.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesmodelbase.h \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesviewbase.h \
        $$TASTE_PRO_ROOT/tab_msc/msctabdocument.h

SOURCES += \
        $$TASTE_PRO_ROOT/app/commandlineparser.cpp \
        $$TASTE_PRO_ROOT/app/commandsstack.cpp \
        $$TASTE_PRO_ROOT/app/common.cpp \
        $$TASTE_PRO_ROOT/app/context/action/action.cpp \
        $$TASTE_PRO_ROOT/app/context/action/actionsmanager.cpp \
        $$TASTE_PRO_ROOT/app/context/action/attrhandler.cpp \
        $$TASTE_PRO_ROOT/app/context/action/condition.cpp \
        $$TASTE_PRO_ROOT/app/context/action/editor/actionsmodel.cpp \
        $$TASTE_PRO_ROOT/app/context/action/editor/attributesmodel.cpp \
        $$TASTE_PRO_ROOT/app/context/action/editor/conditiondialog.cpp \
        $$TASTE_PRO_ROOT/app/context/action/editor/conditionsmodel.cpp \
        $$TASTE_PRO_ROOT/app/context/action/editor/dynactioneditor.cpp \
        $$TASTE_PRO_ROOT/app/context/action/extprocmonitor.cpp \
        $$TASTE_PRO_ROOT/app/datatypes/basicdatatype.cpp \
        $$TASTE_PRO_ROOT/app/datatypes/datatypesstorage.cpp \
#        $$TASTE_PRO_ROOT/app/mainwindow.cpp \
        $$TASTE_PRO_ROOT/app/xmldocexporter.cpp \
        $$TASTE_PRO_ROOT/app/zoomcontroller.cpp \
        $$TASTE_PRO_ROOT/baseitems/common/drawrectinfo.cpp \
        $$TASTE_PRO_ROOT/baseitems/common/highlightrectitem.cpp \
        $$TASTE_PRO_ROOT/baseitems/common/utils.cpp \
        $$TASTE_PRO_ROOT/baseitems/graphicsview.cpp \
        $$TASTE_PRO_ROOT/baseitems/grippoint.cpp \
        $$TASTE_PRO_ROOT/baseitems/grippointshandler.cpp \
        $$TASTE_PRO_ROOT/baseitems/interactiveobject.cpp \
        $$TASTE_PRO_ROOT/baseitems/textgraphicsitem.cpp \
        $$TASTE_PRO_ROOT/document/abstracttabdocument.cpp \
        $$TASTE_PRO_ROOT/document/documentsmanager.cpp \
        $$TASTE_PRO_ROOT/document/tabdocumentfactory.cpp \
        $$TASTE_PRO_ROOT/dummy/dummytabdocument.cpp \
#        $$TASTE_PRO_ROOT/logging/logcategory.cpp \
#        $$TASTE_PRO_ROOT/logging/loghandler.cpp \
        $$TASTE_PRO_ROOT/templating/stringtemplate.cpp \
        $$TASTE_PRO_ROOT/templating/exportedaadlconnection.cpp \
        $$TASTE_PRO_ROOT/templating/exportedaadlfunction.cpp \
        $$TASTE_PRO_ROOT/templating/exportedaadliface.cpp \
        $$TASTE_PRO_ROOT/templating/exportedaadlobject.cpp \
        $$TASTE_PRO_ROOT/templating/templateeditor.cpp \
        $$TASTE_PRO_ROOT/templating/templatehighlighter.cpp \
        $$TASTE_PRO_ROOT/templating/xmlhighlighter.cpp \
        $$TASTE_PRO_ROOT/templating/templatesyntaxhelpdialog.cpp \
#        $$TASTE_PRO_ROOT/reports/bugreportdialog.cpp \
#        $$TASTE_PRO_ROOT/reports/bugreporthandler.cpp \
        $$TASTE_PRO_ROOT/settings/appoptions.cpp \
        $$TASTE_PRO_ROOT/settings/settingsmanager.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlcommonprops.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlnamevalidator.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobject.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectcomment.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectconnection.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectfunction.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectfunctiontype.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectiface.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlobjectsmodel.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlparameter.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadltabdocument.cpp \
        $$TASTE_PRO_ROOT/tab_aadl/aadlxmlreader.cpp \
        $$TASTE_PRO_ROOT/tab_concurrency/concurrencytabdocument.cpp \
        $$TASTE_PRO_ROOT/tab_data/datatabdocument.cpp \
        $$TASTE_PRO_ROOT/tab_deployment/deploymenttabdocument.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlcommentgraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlconnectiongraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctiongraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctionnamegraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlfunctiontypegraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlinterfacegraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/aadlrectgraphicsitem.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commenttextdialog.cpp \
        $$TASTE_PRO_ROOT/tab_interface/colors/colorhandlereditor.cpp \
        $$TASTE_PRO_ROOT/tab_interface/colors/colormanager.cpp \
        $$TASTE_PRO_ROOT/tab_interface/colors/colormanagerdialog.cpp \
        $$TASTE_PRO_ROOT/tab_interface/colors/colorselectorbutton.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcommentitemcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdconnectionitemcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparameterchange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparametercreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdcontextparameterremove.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityautolayout.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityattributechange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitygeometrychange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertychange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertycreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertyremove.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentitypropertyrename.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdentityremove.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdfunctionitemcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdfunctiontypeitemcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifacedatachangebase.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparambase.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamremove.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceparamchange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdifaceattrchange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdinterfaceitemcreate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdrequiredifacepropertychange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/cmdrootentitychange.cpp \
        $$TASTE_PRO_ROOT/tab_interface/commands/commandsfactory.cpp \
        $$TASTE_PRO_ROOT/tab_interface/connectioncreationvalidator.cpp \
        $$TASTE_PRO_ROOT/tab_interface/creatortool.cpp \
        $$TASTE_PRO_ROOT/tab_interface/graphicsitemhelpers.cpp \
        $$TASTE_PRO_ROOT/tab_interface/interfacetabdocument.cpp \
        $$TASTE_PRO_ROOT/tab_interface/interfacetabgraphicsscene.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/adddynamicropertydialog.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/contextparametersmodel.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/comboboxdelegate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/functionattrdelegate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/interfaceattrdelegate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/delegates/propertytypedelegate.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicproperty.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicpropertyconfig.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicpropertymanager.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/ifaceparametersmodel.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesdialog.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertieslistmodel.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesmodelbase.cpp \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesviewbase.cpp \
        $$TASTE_PRO_ROOT/tab_msc/msctabdocument.cpp

FORMS += \
        $$TASTE_PRO_ROOT/app/context/action/editor/conditiondialog.ui \
        $$TASTE_PRO_ROOT/app/context/action/editor/dynactioneditor.ui \
#        $$TASTE_PRO_ROOT/app/mainwindow.ui \
#        $$TASTE_PRO_ROOT/reports/bugreportdialog.ui \
        $$TASTE_PRO_ROOT/tab_interface/commenttextdialog.ui \
        $$TASTE_PRO_ROOT/tab_interface/colors/colorhandlereditor.ui \
        $$TASTE_PRO_ROOT/tab_interface/colors/colormanagerdialog.ui \
        $$TASTE_PRO_ROOT/tab_interface/properties/adddynamicpropertydialog.ui \
        $$TASTE_PRO_ROOT/tab_interface/properties/dynamicpropertymanager.ui \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesdialog.ui \
        $$TASTE_PRO_ROOT/tab_interface/properties/propertiesviewbase.ui
