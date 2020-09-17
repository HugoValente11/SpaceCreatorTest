/*
  Copyright (C) 2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "dynamicproperty.h"

#include <QMetaEnum>

namespace aadlinterface {

const QString kTagName = QLatin1String("Attr");

struct DynamicProperty::DynamicPropertyPrivate {
    QString m_name;
    DynamicProperty::Info m_info;
    DynamicProperty::Type m_type;
    DynamicProperty::Scopes m_scope;
    QList<QVariant> m_vals;
    QString m_rxValueValidatorPattern;
    QMap<DynamicProperty::Scope, QPair<QString, QString>> m_rxAttrValidatorPattern;
    bool m_isVisible = true;
};

DynamicProperty::DynamicProperty()
    : d(new DynamicPropertyPrivate)
{
}

DynamicProperty::~DynamicProperty() { }

QString DynamicProperty::name() const
{
    return d->m_name;
}
void DynamicProperty::setName(const QString &name)
{
    d->m_name = name;
}

DynamicProperty::Info DynamicProperty::info() const
{
    return d->m_info;
}

void DynamicProperty::setInfo(DynamicProperty::Info info)
{
    d->m_info = info;
}

DynamicProperty::Type DynamicProperty::type() const
{
    return d->m_type;
}

void DynamicProperty::setType(DynamicProperty::Type t)
{
    d->m_type = t;
}

DynamicProperty::Scopes DynamicProperty::scope() const
{
    return d->m_scope;
}

void DynamicProperty::setScope(const DynamicProperty::Scopes &s)
{
    d->m_scope = s;
}

bool DynamicProperty::isVisible() const
{
    return d->m_isVisible;
}

void DynamicProperty::setVisible(bool value)
{
    d->m_isVisible = value;
}

QList<QVariant> DynamicProperty::valuesList() const
{
    return d->m_vals;
}

void DynamicProperty::setValuesList(const QList<QVariant> &range)
{
    d->m_vals = range;
}

QString DynamicProperty::valueValidatorPattern() const
{
    return d->m_rxValueValidatorPattern;
}

void aadlinterface::DynamicProperty::setValueValidatorPattern(const QString &pattern)
{
    d->m_rxValueValidatorPattern = pattern;
}

QMap<DynamicProperty::Scope, QPair<QString, QString>> DynamicProperty::attrValidatorPatterns() const
{
    return d->m_rxAttrValidatorPattern;
}

void DynamicProperty::setAttrValidatorPattern(const QMap<Scope, QPair<QString, QString>> &pattern)
{
    d->m_rxAttrValidatorPattern = pattern;
}

QDomElement DynamicProperty::toXml(QDomDocument *doc) const
{
    QDomElement attrElement = doc->createElement(kTagName);
    attrElement.setAttribute(QLatin1String("name"), d->m_name);
    if (!d->m_isVisible) {
        attrElement.setAttribute(QLatin1String("visible"), QLatin1String("false"));
    }

    static const QMetaEnum &infoMeta = QMetaEnum::fromType<DynamicProperty::Info>();
    if (d->m_info == DynamicProperty::Info::Property) {
        attrElement.setAttribute(QLatin1String("type"), infoMeta.valueToKey(static_cast<int>(d->m_info)));
    }

    static const QMetaEnum &typeMeta = QMetaEnum::fromType<DynamicProperty::Type>();
    QDomElement typeElement = doc->createElement(typeMeta.name());
    const QString typeString = typeMeta.valueToKey(static_cast<int>(d->m_type));
    QDomElement typeSubElement = doc->createElement(typeString);
    if (!d->m_rxValueValidatorPattern.isEmpty()) {
        typeSubElement.setAttribute(QLatin1String("validator"), d->m_rxValueValidatorPattern);
    }
    if (d->m_type == DynamicProperty::Type::Enumeration) {
        for (auto entry : d->m_vals) {
            QDomElement entryElement = doc->createElement(QLatin1String("Entry"));
            entryElement.setAttribute(QLatin1String("value"), entry.toString());
            typeSubElement.appendChild(entryElement);
        }
    }
    typeElement.appendChild(typeSubElement);
    attrElement.appendChild(typeElement);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    static const QMetaEnum &scopeMeta = QMetaEnum::fromType<DynamicProperty::Scopes>();
    static const QString scopesElementTag = scopeMeta.name();
#else
    static const QMetaEnum &scopeMeta = QMetaEnum::fromType<DynamicProperty::Scope>();
    static const QString scopesElementTag = QLatin1String("Scopes");
#endif
    QDomElement scopeElement = doc->createElement(scopesElementTag);
    auto scopesToString = [](const DynamicProperty::Scopes s) -> QStringList {
        QStringList result;
        for (int idx = 0; idx < scopeMeta.keyCount(); ++idx) {
            if (s.testFlag(static_cast<DynamicProperty::Scope>(scopeMeta.value(idx)))) {
                result.append(QString::fromLatin1(scopeMeta.key(idx)));
            }
        }
        if (result.isEmpty()) {
            return { scopeMeta.valueToKeys(static_cast<int>(DynamicProperty::Scope::None)) };
        }
        return result;
    };
    for (const auto scope : scopesToString(d->m_scope)) {
        QDomElement scopeSubElement = doc->createElement(scope);
        for (auto it = d->m_rxAttrValidatorPattern.constBegin(); it != d->m_rxAttrValidatorPattern.constEnd(); ++it) {
            QDomElement attrValidatorElement = doc->createElement(QLatin1String("AttrValidator"));
            attrValidatorElement.setAttribute(QLatin1String("name"), it.value().first);
            attrValidatorElement.setAttribute(QLatin1String("value"), it.value().second);
            scopeSubElement.appendChild(attrValidatorElement);
        }
        scopeElement.appendChild(scopeSubElement);
    }
    attrElement.appendChild(scopeElement);

    return attrElement;
}

DynamicProperty *DynamicProperty::fromXml(const QDomElement &element)
{
    if (element.isNull() || element.tagName() != kTagName || !element.hasAttribute(QLatin1String("name"))
            || !element.hasChildNodes()) {
        return nullptr;
    }
    bool ok;

    const QString attrName = element.attribute(QLatin1String("name"));
    const QString attrType = element.attribute(QLatin1String("type"), QLatin1String("Attribute"));
    const bool isVisible =
            QString::compare(element.attribute(QLatin1String("visible")), QLatin1String("false"), Qt::CaseInsensitive)
            != 0;
    static const QMetaEnum &infoMeta = QMetaEnum::fromType<DynamicProperty::Info>();
    const int infoInt = infoMeta.keyToValue(attrType.toUtf8().data(), &ok);
    const DynamicProperty::Info i = ok ? static_cast<DynamicProperty::Info>(infoInt) : DynamicProperty::Info::Property;

    static const QMetaEnum &typeMeta = QMetaEnum::fromType<DynamicProperty::Type>();
    const QDomElement typeElement = element.firstChildElement(typeMeta.name());
    DynamicProperty::Type t = DynamicProperty::Type::Unknown;

    QList<QVariant> enumVals;
    QString typeValidator;
    if (!typeElement.isNull()) {
        const QDomElement typeSubElement = typeElement.firstChildElement();
        if (!typeSubElement.isNull()) {
            typeValidator = typeSubElement.attribute(QLatin1String("validator"));
            const int typeInt = typeMeta.keyToValue(typeSubElement.tagName().toUtf8().data(), &ok);
            if (ok && typeInt != -1) {
                t = static_cast<DynamicProperty::Type>(typeInt);
            }
            if (t == DynamicProperty::Type::Enumeration) {
                QDomElement typeEntryElement = typeSubElement.firstChildElement(QLatin1String("Entry"));
                while (!typeEntryElement.isNull()) {
                    const QVariant value = typeEntryElement.attribute(QLatin1String("value"));
                    enumVals.append(value);
                    typeEntryElement = typeEntryElement.nextSiblingElement(typeEntryElement.tagName());
                }
            }
        }
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    static const QMetaEnum &scopeMeta = QMetaEnum::fromType<DynamicProperty::Scopes>();
    static const QString scopesElementTag = scopeMeta.name();
#else
    static const QMetaEnum &scopeMeta = QMetaEnum::fromType<DynamicProperty::Scope>();
    static const QString scopesElementTag = QLatin1String("Scopes");
#endif
    const QDomElement scopeElement = element.firstChildElement(scopesElementTag);
    DynamicProperty::Scopes s = DynamicProperty::Scope::None;
    QMap<DynamicProperty::Scope, QPair<QString, QString>> attrValidators;
    if (scopeElement.isNull() || !scopeElement.hasChildNodes()) {
        s = DynamicProperty::Scope::All;
    } else {
        QDomElement scopeSubElement = scopeElement.firstChildElement();
        while (!scopeSubElement.isNull()) {
            const QString scopeName = scopeSubElement.tagName();
            const int scopeInt = scopeMeta.keyToValue(scopeName.toUtf8().data(), &ok);
            if (!ok || scopeInt == -1) {
                continue;
            }
            const DynamicProperty::Scope scope = static_cast<DynamicProperty::Scope>(scopeInt);
            s.setFlag(scope);
            QDomElement attrValidatorElement = scopeSubElement.firstChildElement(QLatin1String("AttrValidator"));
            while (!attrValidatorElement.isNull()) {
                const QString attrValidatorName = attrValidatorElement.attribute(QLatin1String("name"));
                const QString attrValidatorPattern = attrValidatorElement.attribute(QLatin1String("value"));
                if (!attrValidatorName.isEmpty() && !attrValidatorPattern.isNull()) {
                    attrValidators.insert(scope, qMakePair(attrValidatorName, attrValidatorPattern));
                }
                attrValidatorElement = attrValidatorElement.nextSiblingElement(attrValidatorElement.tagName());
            }
            scopeSubElement = scopeSubElement.nextSiblingElement();
        }
    }

    auto dynamicProperty = new DynamicProperty;
    dynamicProperty->setName(attrName);
    dynamicProperty->setInfo(i);
    dynamicProperty->setType(t);
    dynamicProperty->setScope(s);
    dynamicProperty->setValuesList(enumVals);
    dynamicProperty->setAttrValidatorPattern(attrValidators);
    dynamicProperty->setValueValidatorPattern(typeValidator);
    dynamicProperty->setVisible(isVisible);
    return dynamicProperty;
}

QString DynamicProperty::tagName()
{
    return kTagName;
}

} // namespace aadlinterface
