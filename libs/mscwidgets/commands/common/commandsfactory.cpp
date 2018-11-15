/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

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
#include "commandsfactory.h"
#include "commandsstack.h"

#include "commands/cmdmessageitemmove.h"
#include "commands/cmdmessageitemresize.h"
#include "commands/cmdinstanceitemmove.h"
#include "commands/cmdinstanceitemresize.h"

#include <messageitem.h>
#include <instanceitem.h>

#include <QGraphicsItem>
#include <QPointF>
#include <QRectF>

namespace msc {
namespace cmd {

QUndoCommand *CommandsFactory::create_messageItemMove(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (MessageItem *item = params.first().value<MessageItem *>()) {
        const QPointF &destination = params.last().toPointF();
        if (item->pos() != destination)
            return new CmdMessageItemMove(item, destination);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::create_messageItemResize(const QVariantList &params)
{
    Q_ASSERT(params.size() == 3);

    if (MessageItem *pItem = params.at(0).value<MessageItem *>()) {
        const QPointF &head(params.at(1).toPointF());
        const QPointF &tail(params.at(2).toPointF());
        return new CmdMessageItemResize(pItem, head, tail);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::create_instanceItemMove(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (InstanceItem *item = params.first().value<InstanceItem *>()) {
        const QPointF &destination = params.last().toPointF();
        if (item->pos() != destination)
            return new CmdInstanceItemMove(item, destination);
    }

    return nullptr;
}

QUndoCommand *CommandsFactory::create_instanceItemResize(const QVariantList &params)
{
    Q_ASSERT(params.size() == 2);

    if (InstanceItem *item = params.first().value<InstanceItem *>()) {
        const QRectF &newGeometry = params.last().toRectF();
        if (item->boundingRect() != newGeometry)
            return new CmdInstanceItemResize(item, newGeometry);
    }

    return nullptr;
}

} // ns cmd
} // ns msc
