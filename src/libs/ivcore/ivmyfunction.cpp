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

#include "ivmyfunction.h"

#include "ivcommonprops.h"
#include "ivinterface.h"
#include "ivmodel.h"
#include "ivobject.h"

#include <QDebug>
#include <QScopedPointer>
#include <QUndoCommand>

namespace ivm {

struct IVMyFunctionPrivate {
    QPointer<IVFunctionType> m_fnType;
};

IVMyFunction::IVMyFunction(const QString &title, QObject *parent, const shared::Id &id)
    : IVFunction(title, parent, id, IVObject::Type::MyFunction)
    , d(new IVMyFunctionPrivate)
{
}

IVMyFunction::~IVMyFunction() { }

bool IVMyFunction::postInit(QString *warning)
{
    Q_UNUSED(warning);

    if (auto objModel = model()) {
        const QString typeName =
                entityAttributeValue(meta::Props::token(meta::Props::Token::instance_of)).value<QString>();
        if (!typeName.isEmpty()) {
            const QHash<QString, IVFunctionType *> types = objModel->getAvailableFunctionTypes(this);
            if (auto typeObj = types.value(typeName)) {
                setInstanceOf(typeObj);
            } else {
                return false;
            }
        }
    }

    return IVObject::postInit(warning);
}

void IVMyFunction::setInstanceOf(IVFunctionType *fnType)
{
    if (d->m_fnType != fnType) {
        if (d->m_fnType) {
            disconnect(d->m_fnType, &IVMyFunction::contextParamsChanged, this, &IVMyFunction::reflectContextParam);
            disconnect(d->m_fnType, &IVMyFunction::attributeChanged, this, &IVMyFunction::reflectAttr);

            restoreInternals();
        }

        d->m_fnType = fnType;

        if (d->m_fnType) {
            cloneInternals();

            connect(d->m_fnType, &IVMyFunction::attributeChanged, this, &IVMyFunction::reflectAttr);
            connect(d->m_fnType, &IVMyFunction::contextParamsChanged, this, &IVMyFunction::reflectContextParam);
        }
    }
}

void IVMyFunction::cloneInternals()
{
    m_originalFields.collect(this);

    if (d->m_fnType) {
        d->m_fnType->rememberInstance(this);

        reflectAttrs(d->m_fnType->entityAttributes());
        reflectContextParams(d->m_fnType->contextParams());
    }
}

void IVMyFunction::restoreInternals()
{
    if (d->m_fnType)
        d->m_fnType->forgetInstance(this);

    if (m_originalFields.collected()) {
        reflectAttrs(m_originalFields.attrs);
        reflectContextParams(m_originalFields.params);
    }
}

const IVFunctionType *IVMyFunction::instanceOf() const
{
    return d->m_fnType;
}

bool IVMyFunction::inheritsFunctionType() const
{
    return instanceOf();
}

void IVMyFunction::reflectAttrs(const EntityAttributes &attributes)
{
    EntityAttributes prepared { attributes };
    static const QList<meta::Props::Token> excludeTokens = { meta::Props::Token::is_type,
        meta::Props::Token::instance_of, meta::Props::Token::name, meta::Props::Token::InnerCoordinates,
        meta::Props::Token::coordinates };
    for (meta::Props::Token t : excludeTokens) {
        const QString name = meta::Props::token(t);
        prepared[name] = entityAttribute(name);
    }

    setEntityAttributes(prepared);
}

void IVMyFunction::reflectAttr(const QString &attrName)

{
    if (!d->m_fnType) {
        if (const IVFunctionType *fnType = dynamic_cast<const IVFunctionType *>(sender()))
            qWarning() << QString("The Function type \"%1\" (%2) has not been disconnected, it seems")
                                  .arg(fnType->title(), fnType->id().toString());
        return;
    }
    const meta::Props::Token attr = meta::Props::token(attrName);

    switch (attr) {
    case meta::Props::Token::is_type:
    case meta::Props::Token::instance_of:
    case meta::Props::Token::name:
    case meta::Props::Token::RootCoordinates:
    case meta::Props::Token::InnerCoordinates:
    case meta::Props::Token::coordinates:
        break;
    case meta::Props::Token::Unknown: {
        reflectAttrs(d->m_fnType->entityAttributes());
        break;
    }
    default: {
        setEntityAttribute(d->m_fnType->entityAttribute(attrName));
        break;
    }
    }
}

void IVMyFunction::reflectContextParam()
{
    if (!d->m_fnType) {
        if (const IVFunctionType *fnType = dynamic_cast<const IVFunctionType *>(sender()))
            qWarning() << QString("The Function type \"%1\" (%2) has not been disconnected, it seems")
                                  .arg(fnType->title(), fnType->id().toString());
        return;
    }

    reflectContextParams(d->m_fnType->contextParams());
}

void IVMyFunction::reflectContextParams(const QVector<ContextParameter> &params)
{
    setContextParams(params);
}

}
