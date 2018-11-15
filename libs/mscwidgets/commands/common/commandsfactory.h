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

#pragma once

#include <QVariant>

class QGraphicsItem;
class QPointF;
class QRectF;
class QUndoCommand;

namespace msc {
namespace cmd {

class CommandsFactory
{
public:
    static QUndoCommand *create_messageItemMove(const QVariantList &params);
    static QUndoCommand *create_messageItemResize(const QVariantList &params);
    static QUndoCommand *create_instanceItemMove(const QVariantList &params);
    static QUndoCommand *create_instanceItemResize(const QVariantList &params);

private:
    CommandsFactory() = delete;
    CommandsFactory(const CommandsFactory &other) = delete;
};

} // ns cmd
} // ns msc
