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
#include "commandsstack.h"
#include "commandsfactory.h"

#include "commands/cmdmessageitemmove.h"
#include "commands/cmdmessageitemresize.h"
#include "commands/cmdinstanceitemmove.h"
#include "commands/cmdinstanceitemresize.h"

#include <QDebug>

namespace msc {
namespace cmd {

CommandsStack *CommandsStack::m_instance = nullptr;

CommandsStack *CommandsStack::instance()
{
    if (!m_instance)
        m_instance = new CommandsStack();

    return m_instance;
}

void CommandsStack::setCurrent(QUndoStack *stack)
{
    instance()->setCurrentStack(stack);
}

QUndoStack *CommandsStack::current()
{
    return instance()->currentStack();
}

bool CommandsStack::push(msc::cmd::Id id, const QVariantList &params)
{
    QUndoCommand *command(nullptr);

    switch (id) {
    case cmd::MoveMessage:
        command = cmd::CommandsFactory::createMessageItemMove(params);
        break;
    case cmd::RetargetMessage:
        command = cmd::CommandsFactory::createMessageItemResize(params);
        break;
    case cmd::CreateMessage:
        command = cmd::CommandsFactory::createMessageItemCreate(params);
        break;
    case cmd::MoveInstance:
        command = cmd::CommandsFactory::createInstanceItemMove(params);
        break;
    case cmd::ResizeInstance:
        command = cmd::CommandsFactory::createInstanceItemResize(params);
        break;
    default:
        qWarning() << "CommandsStack::push - command ignored" << id;
        break;
    }

    if (command) {
        CommandsStack::current()->push(command);
        return true;
    }

    return false;
}

CommandsStack::CommandsStack(QObject *parent)
    : QObject(parent)
{
}

void CommandsStack::setCurrentStack(QUndoStack *stack)
{
    if (m_current == stack)
        return;

    m_current = stack;
    Q_EMIT currentStackChanged(m_current);
}

QUndoStack *CommandsStack::currentStack() const
{
    return m_current;
}

} // namespace cmd
} // ns msc
