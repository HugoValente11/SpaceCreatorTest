/****************************************************************************
**
** Copyright (C) 2017 N7 Mobile sp. z o. o.
** Contact: http://n7mobile.pl/Space
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a programme and funded by
** European Space Agency.
**
** This Plugin is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This Plugin is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/
#include "modulemetadataparser.h"

#include <QJsonObject>
#include <QJsonArray>

using namespace Asn1Acn::Internal::Libraries;

ModuleMetadataParser::ModuleMetadataParser(const QByteArray &data)
    : m_document(QJsonDocument::fromJson(data))
{
}

namespace {

template <typename Func>
void foreachItem(const QJsonValue &value, Func f)
{
    for (const auto &item : value.toArray())
        f(item);
}

QString readName(const QJsonObject &object)
{
    const auto name = object["name"].toString();
    if (name.isEmpty())
        throw ModuleMetadataParser::Error("'name' missing or empty");
    return name;
}

QString readDescription(const QJsonObject &object)
{
    return object["description"].toString();
}

template <typename T>
std::unique_ptr<T> readNamedType(const QJsonObject &object)
{
    return std::make_unique<T>(readName(object), readDescription(object));
}

std::unique_ptr<Metadata::Element> readElement(const QJsonObject &object)
{
    auto element = readNamedType<Metadata::Element>(object);
    foreachItem(object["asn1Files"], [&element](const QJsonValue &value) {
        element->addAsn1File(value.toString());
    });
    foreachItem(object["conflicts"], [&element](const QJsonValue &value) {
        element->addConflict(value.toString());
    });
    foreachItem(object["requires"], [&element](const QJsonValue &value) {
        element->addRequirement(value.toString());
    });
    return element;
}

std::unique_ptr<Metadata::Submodule> readSubmodule(const QJsonObject &object)
{
    auto submodule = readNamedType<Metadata::Submodule>(object);
    foreachItem(object["elements"], [&submodule](const QJsonValue &value) {
        auto element = readElement(value.toObject());
        submodule->addElement(std::move(element));
    });
    return submodule;
}

std::unique_ptr<Metadata::Module> readModule(const QJsonObject &object)
{
    auto module = readNamedType<Metadata::Module>(object);
    foreachItem(object["submodules"], [&module](const QJsonValue &value) {
        auto element = readSubmodule(value.toObject());
        module->addSubmodule(std::move(element));
    });
    return module;
}

} // namespace

std::unique_ptr<Metadata::Module> ModuleMetadataParser::parse()
{
    if (m_document.isNull())
        throw Error("JSON malformed");
    return readModule(m_document.object());
}
