/*
  Copyright (C) 2020 European Space Agency - <maxime.perrotin@esa.int>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include "xmldocexporter.h"

#include "app/common.h"
#include "document/abstracttabdocument.h"
#include "tab_aadl/aadlobject.h"
#include "tab_aadl/aadlobjectcomment.h"
#include "tab_aadl/aadlobjectconnection.h"
#include "tab_aadl/aadlobjectfunction.h"
#include "tab_aadl/aadlobjectfunctiontype.h"
#include "tab_aadl/aadlobjectiface.h"
#include "tab_interface/interfacetabdocument.h"
#include "templating/stringtemplate.h"
#include "templating/templateeditor.h"
#include "templating/exportedaadlobject.h"

#include <QDebug>
#include <QFileDialog>
#include <QMetaEnum>
#include <QObject>
#include <QScopedPointer>
#include <QStandardPaths>

namespace taste3 {
namespace app {

static const QString TemplateFileExtension = QString("tmplt");

QString XmlDocExporter::templatesPath()
{
    static const QString path =
            QString("%1/export_templates").arg(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    return path;
}

QString XmlDocExporter::interfaceDefaultTemplate()
{
    static const QString interfaceTemplatePath =
            QString("%1/aadl_xml/interfaceview.%2").arg(templatesPath(), TemplateFileExtension);
    return interfaceTemplatePath;
}

void XmlDocExporter::ensureDefaultTemplatesDeployed_interface(RolloutDefaultsPolicy policy)
{
    static const QStringList fileNames { QStringLiteral("interfaceview"), QStringLiteral("interface"),
                                         QStringLiteral("function"), QStringLiteral("connection"),
                                         QStringLiteral("comment") };

    static const QString sourceFile(":/defaults/templating/xml_templates/%1.%2");
    static const QString targetFilePath = QFileInfo(interfaceDefaultTemplate()).path();

    const bool forcedOverwrite = policy == RolloutDefaultsPolicy::Overwrite;

    auto rollOutDefaultTemplate = [forcedOverwrite](const QString &fileName) {
        const QString fileFrom(sourceFile.arg(fileName, TemplateFileExtension));
        const QString fileTo(QString("%1/%2.%3").arg(targetFilePath, fileName, TemplateFileExtension));
        if (forcedOverwrite || !QFileInfo::exists(fileTo))
            common::copyResourceFile(fileFrom, fileTo);
    };

    for (int i = 0; i < fileNames.size(); ++i) {
        const QString &fileName(fileNames[i]);
        rollOutDefaultTemplate(fileName);
        if (i)
            rollOutDefaultTemplate(fileName + QStringLiteral("s"));
    }
}

XmlDocExporter::XmlDocExporter() {}

bool XmlDocExporter::canExportXml(document::AbstractTabDocument *doc)
{
    if (!doc)
        return false;

    if (const document::InterfaceTabDocument *ifaceDoc = qobject_cast<const document::InterfaceTabDocument *>(doc))
        return true;

    return false;
}

bool XmlDocExporter::exportDocSilently(document::AbstractTabDocument *doc, const QString &outPath,
                                       const QString &templatePath)
{
    return exportDoc(doc, nullptr, outPath, templatePath, InteractionPolicy::Silently);
}

bool XmlDocExporter::exportDocInteractive(document::AbstractTabDocument *doc, QWidget *root, const QString &outPath,
                                          const QString &templatePath)
{
    return exportDoc(doc, root, outPath, templatePath, InteractionPolicy::Interactive);
}

bool XmlDocExporter::exportDoc(document::AbstractTabDocument *doc, QWidget *root, const QString &outPath,
                               const QString &templatePath, InteractionPolicy interaction)
{
    if (!doc)
        return false;

    if (document::InterfaceTabDocument *ifaceDoc = qobject_cast<document::InterfaceTabDocument *>(doc)) {
        ensureDefaultTemplatesDeployed_interface();

        QString usedTemplate(templatePath);
        if (interaction == InteractionPolicy::Interactive) {
            if (usedTemplate.isEmpty())
                usedTemplate = QFileDialog::getOpenFileName(root, QObject::tr("Select a template for Interface doc"),
                                                            QFileInfo(interfaceDefaultTemplate()).path(),
                                                            QString("*.%1").arg(TemplateFileExtension));
            if (usedTemplate.isEmpty())
                return false;
        }

        return exportDocInterface(ifaceDoc, root, outPath, usedTemplate, interaction);
    }

    qWarning() << "Unsupported document skipped";
    return true; // Do not break MainWindow's loop through all docs if this one is not supported
}

bool XmlDocExporter::exportDocInterface(document::InterfaceTabDocument *doc, QWidget *parentWindow,
                                        const QString &outPath, const QString &templatePath,
                                        InteractionPolicy interaction)
{
    if (!doc)
        return false;

    QString savePath(outPath);
    if (savePath.isEmpty())
        savePath = doc->path();

    if (savePath.isEmpty())
        savePath = QFileDialog::getSaveFileName(parentWindow, QObject::tr("Export Interface to an XML file"),
                                                doc->path(), doc->supportedFileExtensions());

    if (savePath.isEmpty())
        return false;

    QString usedTemplatePath(templatePath);
    if (templatePath.isEmpty())
        usedTemplatePath = XmlDocExporter::interfaceDefaultTemplate();

    const QHash<QString, QVariantList> aadlObjects = collectInterfaceObjects(doc);

    if (InteractionPolicy::Silently == interaction)
        return runExportSilently(doc, aadlObjects, usedTemplatePath, savePath);
    else
        return showExportDialog(doc, parentWindow, aadlObjects, usedTemplatePath, savePath);
}

bool XmlDocExporter::runExportSilently(document::InterfaceTabDocument *doc, const QHash<QString, QVariantList> &content,
                                       const QString &templateFileName, const QString &outFileName)
{
    QScopedPointer<templating::StringTemplate> strTemplate(templating::StringTemplate::create());
    QFile out(outFileName);
    const bool saved = strTemplate->parseFile(content, templateFileName, &out);
    doc->onSavedExternally(outFileName, saved);

    return saved;
}

bool XmlDocExporter::showExportDialog(document::InterfaceTabDocument *doc, QWidget *parentWindow,
                                      const QHash<QString, QVariantList> &content, const QString &templateFileName,
                                      const QString &outFileName)
{
    templating::TemplateEditor *previewDialog = new templating::TemplateEditor(outFileName, parentWindow);
    previewDialog->setAttribute(Qt::WA_DeleteOnClose);

    const bool templateParsed = previewDialog->parseTemplate(content, templateFileName);
    if (doc && templateParsed) {
        QObject::connect(previewDialog, &templating::TemplateEditor::fileSaved, doc,
                         &document::AbstractTabDocument::onSavedExternally);
    }
    return templateParsed;
}

static const QVector<QString> availableAADLObjectTypeNames()
{
    QVector<QString> keys;

    const QMetaEnum &me = QMetaEnum::fromType<aadl::AADLObject::Type>();
    for (int i = 0; i < me.keyCount(); ++i) {

        QString key;
        switch (static_cast<aadl::AADLObject::Type>(me.value(i))) {

        // Despite the same key "Functions" shared between Function and FunctionType,
        // each should be stored separately to allow queries like
        // keys[AADLObject::Type::Function] and keys[AADLObject::Type::FunctionType]
        case aadl::AADLObject::Type::FunctionType:
        case aadl::AADLObject::Type::Function:
            key = QStringLiteral("Functions");
            break;

        default:
            key = me.key(i);
            key.append(QStringLiteral("s"));
            break;
        }
        keys.append(key);
    }

    return keys;
}

QHash<QString, QVariantList> XmlDocExporter::collectInterfaceObjects(document::InterfaceTabDocument *doc)
{
    static const QVector<QString> keys = availableAADLObjectTypeNames();

    QHash<QString, QVariantList> grouppedObjects;
    for (const auto aadlObject : doc->objects()) {
        const aadl::AADLObject::Type t = aadlObject->aadlType();
        switch (t) {
        case aadl::AADLObject::Type::FunctionType:
        case aadl::AADLObject::Type::Function:
        case aadl::AADLObject::Type::Comment:
        case aadl::AADLObject::Type::Connection:
        case aadl::AADLObject::Type::Unknown: {
            if (t == aadl::AADLObject::Type::Unknown || aadlObject->isNested())
                continue;
            break;
        }
        default:
            break;
        }
        const QString aadlGroupType = keys[static_cast<int>(t)];
        const QVariant &exportedObject = templating::ExportedAADLObject::createFrom(aadlObject);
        grouppedObjects[aadlGroupType] << exportedObject;
    }

    return grouppedObjects;
}

} // ns app
} // ns taste3
