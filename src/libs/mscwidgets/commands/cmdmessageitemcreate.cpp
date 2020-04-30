/*
   Copyright (C) 2018-2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "cmdmessageitemcreate.h"

#include "chartviewmodel.h"
#include "cif/cifblockfactory.h"
#include "cif/ciflines.h"
#include "instanceitem.h"
#include "mscchart.h"
#include "mscinstance.h"
#include "mscmessage.h"

namespace msc {
namespace cmd {

CmdMessageItemCreate::InstanceGeometry CmdMessageItemCreate::initGeometryHolder(msc::InstanceItem *from)
{
    InstanceGeometry geometry;
    if (from) {
        geometry.m_pos = from->scenePos();
        geometry.m_cif = from->modelItem()->cifGeometry();
        geometry.m_axis = from->axisHeight();
    }

    return geometry;
}

CmdMessageItemCreate::CmdMessageItemCreate(msc::MscMessage *message, msc::ChartViewModel *model, int eventIndex,
                                           const QVector<QPoint> &points)
    : BaseCommand(message)
    , m_message(message)
    , m_viewModel(model)
    , m_chart(m_viewModel ? m_viewModel->currentChart() : nullptr)
    , m_eventIndex(eventIndex)
    , m_msgPoints(points)
    , m_sourceGeometryPrev(initGeometryHolder(
              (m_viewModel && m_message) ? m_viewModel->itemForInstance(m_message->sourceInstance()) : nullptr))
    , m_targetGeometryPrev(initGeometryHolder(
              (m_viewModel && m_message) ? m_viewModel->itemForInstance(m_message->targetInstance()) : nullptr))
{
    Q_ASSERT(m_chart.data());

    setText(QObject::tr("Add message"));
}

void CmdMessageItemCreate::redo()
{
    Q_ASSERT(m_chart.data());

    if (!m_message) {
        m_message = new MscMessage(QObject::tr("Message_%1").arg(m_chart->instanceEvents().size()));
        m_modelItem = m_message;
    }
    if (m_message->messageType() == MscMessage::MessageType::Create && m_message->targetInstance())
        m_message->targetInstance()->setExplicitCreator(m_message->sourceInstance());

    if (!m_msgPoints.isEmpty())
        if (cif::CifBlockShared msgCif = cif::CifBlockFactory::createBlockMessage()) {
            msgCif->setLines({ cif::CifLineShared(new cif::CifLineMessage) });
            msgCif->setPayload(QVariant::fromValue(m_msgPoints), cif::CifLine::CifType::Message);
            m_modelItem->setCifs({ msgCif });
        }

    // The chart takes over parent-/owner-ship
    m_chart->addInstanceEvent(m_message, m_eventIndex);
}

void CmdMessageItemCreate::undo()
{
    Q_ASSERT(m_chart.data());

    auto resotreInstanceGeometry = [this](MscInstance *instance, const InstanceGeometry &geometry) {
        if (!instance || !m_viewModel)
            return;

        if (InstanceItem *item = m_viewModel->itemForInstance(instance)) {
            QSignalBlocker silently(instance);
            if (!geometry.m_cif.isEmpty()) {
                instance->setCifGeometry(geometry.m_cif);
            } else {
                const QPointF &shift = geometry.m_pos - item->scenePos();
                item->moveSilentlyBy(shift);
                item->setAxisHeight(geometry.m_axis);
            }
        }
    };

    resotreInstanceGeometry(m_message->sourceInstance(), m_sourceGeometryPrev);
    resotreInstanceGeometry(m_message->targetInstance(), m_targetGeometryPrev);

    m_chart->removeInstanceEvent(m_message);

    // this command takes over ownership
    m_message->setParent(this);
    if (m_message->messageType() == MscMessage::MessageType::Create && m_message->targetInstance())
        m_message->targetInstance()->setExplicitCreator(nullptr);
}

bool CmdMessageItemCreate::mergeWith(const QUndoCommand *command)
{
    Q_UNUSED(command);
    return false;
}

int CmdMessageItemCreate::id() const
{
    return msc::cmd::Id::CreateMessage;
}

} // ns cmd
} // ns msc