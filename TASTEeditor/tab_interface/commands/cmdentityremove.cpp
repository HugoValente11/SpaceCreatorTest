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

#include "cmdentityremove.h"

#include "commandids.h"

#include <tab_aadl/aadlobject.h>
#include <tab_aadl/aadlobjectconnection.h>
#include <tab_aadl/aadlobjectiface.h>
#include <tab_aadl/aadlobjectsmodel.h>

namespace taste3 {
namespace aadl {
namespace cmd {

CmdEntityRemove::CmdEntityRemove(AADLObject *entity, AADLObjectsModel *model)
    : QUndoCommand()
    , m_model(model)
    , m_entity(entity)
{
    setText(QObject::tr("Remove"));

    collectRelatedItems(m_entity);
}

void CmdEntityRemove::redo()
{
    if (!m_model)
        return;

    auto removeAadlObjects = [this](const QVector<QPointer<AADLObject>> &collection) {
        for (auto it = collection.cbegin(); it != collection.cend(); ++it)
            m_model->removeObject(*it);
    };

    removeAadlObjects(m_relatedIfaces);
    removeAadlObjects(m_relatedConnections);
    removeAadlObjects(m_relatedEntities);

    if (m_entity)
        m_model->removeObject(m_entity);

    for (auto function : m_typedFunctions.keys())
        function->setInstanceOf(nullptr, AADLObjectFunction::ClonedIfacesPolicy::Keep);
}

void CmdEntityRemove::undo()
{
    if (!m_model)
        return;

    if (m_entity)
        m_model->addObject(m_entity);

    auto restoreAadlObjects = [this](const QVector<QPointer<AADLObject>> &collection) {
        for (auto it = collection.crbegin(); it != collection.crend(); ++it)
            m_model->addObject(*it);
    };

    restoreAadlObjects(m_relatedEntities);
    restoreAadlObjects(m_relatedIfaces);
    restoreAadlObjects(m_relatedConnections);

    static const QString instanceOfAttr = meta::Props::token(meta::Props::Token::instance_of);
    for (auto function : m_typedFunctions.keys())
        function->setInstanceOf(m_typedFunctions.value(function), AADLObjectFunction::ClonedIfacesPolicy::Keep);
}

bool CmdEntityRemove::mergeWith(const QUndoCommand *command)
{
    Q_UNUSED(command);
    return false;
}

int CmdEntityRemove::id() const
{
    return RemoveEntity;
}

void CmdEntityRemove::collectRelatedItems(AADLObject *toBeRemoved)
{
    if (!toBeRemoved)
        return;

    switch (toBeRemoved->aadlType()) {
    case AADLObject::AADLObjectType::AADLIface: {
        if (AADLObjectIface *iface = qobject_cast<AADLObjectIface *>(toBeRemoved)) {
            for (auto clone : iface->clones())
                collectRelatedItems(clone);
            if (auto connection = m_model->getConnectionForIface(iface->id()))
                storeLinkedEntity(connection);
        }
        break;
    }
    case AADLObject::AADLObjectType::AADLFunction:
    case AADLObject::AADLObjectType::AADLFunctionType: {
        if (AADLObjectFunctionType *fnType = qobject_cast<AADLObjectFunctionType *>(toBeRemoved)) {
            for (auto iface : fnType->interfaces())
                collectRelatedItems(iface);

            for (auto child : toBeRemoved->findChildren<AADLObjectFunction *>(QString(), Qt::FindDirectChildrenOnly))
                collectRelatedItems(child);

            if (fnType->aadlType() == AADLObject::AADLObjectType::AADLFunctionType) {
                for (AADLObjectFunction *function : fnType->instances())
                    m_typedFunctions.insert(function, fnType);
            }
        }
        break;
    }
    default: {
        for (auto child : toBeRemoved->findChildren<AADLObject *>(QString(), Qt::FindDirectChildrenOnly))
            collectRelatedItems(child);
        break;
    }
    }

    storeLinkedEntity(toBeRemoved);
}

void CmdEntityRemove::storeLinkedEntity(AADLObject *linkedEntity)
{
    auto storeObject = [](AADLObject *obj, QVector<QPointer<AADLObject>> *collection) {
        if (obj && !collection->contains(obj))
            collection->append(obj);
    };

    QVector<QPointer<AADLObject>> *pCollection { nullptr };
    switch (linkedEntity->aadlType()) {
    case AADLObject::AADLObjectType::AADLConnection:
        pCollection = &m_relatedConnections;
        break;
    case AADLObject::AADLObjectType::AADLIface:
        pCollection = &m_relatedIfaces;
        break;
    default:
        pCollection = &m_relatedEntities;
        break;
    }
    storeObject(linkedEntity, pCollection);
}

} // namespace cmd
} // namespace aadl
} // namespace taste3
