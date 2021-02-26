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

#include "cmdifacedatachangebase.h"

#include "aadlconnection.h"
#include "aadliface.h"
#include "aadlmodel.h"
#include "cmdentitiesremove.h"

namespace ive {
namespace cmd {

CmdIfaceDataChangeBase::CmdIfaceDataChangeBase(ivm::AADLIface *iface, const QString &targetName,
        const QVariant &targetValue, const QVariant &prevValue, QUndoCommand *parent)
    : shared::UndoCommand(parent)
    , m_iface(iface)
    , m_model(m_iface ? m_iface->objectsModel() : nullptr)
    , m_relatedConnections()
    , m_targetName(targetName)
    , m_targetToken(ivm::meta::Props::token(m_targetName))
    , m_oldValue(prevValue)
    , m_newValue(targetValue)
    , m_cmdRmConnection()
{
    setText(QObject::tr("Change %1's %2").arg(m_iface->titleUI(), m_targetName));
}

CmdIfaceDataChangeBase::~CmdIfaceDataChangeBase()
{
    qDeleteAll(m_cmdRmConnection);
}

QVector<QPointer<ivm::AADLIface>> CmdIfaceDataChangeBase::getRelatedIfaces()
{
    QVector<QPointer<ivm::AADLIface>> ifaces;

    if (m_iface) {
        ifaces.append(m_iface);
        for (const auto &clone : m_iface->clones())
            ifaces.append(clone);
    }

    return ifaces;
}

QVector<ivm::AADLConnection *> CmdIfaceDataChangeBase::getRelatedConnections()
{
    QVector<ivm::AADLConnection *> affected;

    if (m_iface && m_model)
        for (const auto &i : getRelatedIfaces())
            affected += m_model->getConnectionsForIface(i->id());

    return affected;
}

ivm::AADLIface *CmdIfaceDataChangeBase::interface() const
{
    return m_iface;
}

ivm::AADLIface *CmdIfaceDataChangeBase::getConnectionOtherSide(
        const ivm::AADLConnection *connection, ivm::AADLIface *changedIface)
{
    if (connection && changedIface) {
        switch (connection->connectionType()) {
        case ivm::AADLConnection::ConnectionType::RI2PI: {
            return changedIface->isRequired() ? connection->targetInterface() : changedIface;
        }
        case ivm::AADLConnection::ConnectionType::PI2RI: {
            return changedIface->isProvided() ? connection->targetInterface() : changedIface;
        }
        case ivm::AADLConnection::ConnectionType::PI2PI:
        case ivm::AADLConnection::ConnectionType::RI2RI: {
            if (connection->sourceInterface() == changedIface
                    || connection->sourceInterface()->cloneOf() == changedIface) {
                return connection->targetInterface();
            }

            if (connection->targetInterface() == changedIface
                    || connection->targetInterface()->cloneOf() == changedIface) {
                return connection->sourceInterface();
            }

            break;
        }
        default:
            break;
        }
    }

    Q_UNREACHABLE();

    return nullptr;
}

void CmdIfaceDataChangeBase::prepareRemoveConnectionCommands()
{
    QList<QPointer<ivm::AADLObject>> entities;
    for (const auto &connection : m_relatedConnections) {
        if (connectionMustDie(connection)) {
            entities.append(connection);
        }
    }
    if (!entities.isEmpty()) {
        auto cmdRm = new cmd::CmdEntitiesRemove(entities, m_model);
        m_cmdRmConnection.append(cmdRm);
    }
}

}
}
