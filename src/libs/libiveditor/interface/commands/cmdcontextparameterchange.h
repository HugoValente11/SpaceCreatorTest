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

#pragma once

#include "aadlobject.h"
#include "aadlobjectfunctiontype.h"

#include <QPointer>
#include <QUndoCommand>

namespace aadl {
class AADLObject;
}

namespace aadlinterface {
namespace cmd {

class CmdContextParameterChange : public QUndoCommand
{
public:
    explicit CmdContextParameterChange(aadl::AADLObjectFunctionType *entity, const aadl::ContextParameter &oldParam,
                                       const aadl::ContextParameter &newParam);

    void redo() override;
    void undo() override;
    bool mergeWith(const QUndoCommand *command) override;
    int id() const override;

private:
    QPointer<aadl::AADLObjectFunctionType> m_entity;
    aadl::ContextParameter m_newParam;
    const aadl::ContextParameter m_oldParam;

    void swapParam(const aadl::ContextParameter &from, const aadl::ContextParameter &to);
};

}
}