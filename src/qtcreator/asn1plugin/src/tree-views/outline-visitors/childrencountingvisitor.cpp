/****************************************************************************
**
** Copyright (C) 2017-2019 N7 Space sp. z o. o.
** Contact: http://n7space.com
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a program and funded by
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
#include "childrencountingvisitor.h"

#include <asn1/definitions.h>
#include <asn1/file.h>
#include <asn1/project.h>
#include <asn1/root.h>

using namespace Asn1Acn::Internal::TreeViews::OutlineVisitors;

ChildrenCountingVisitor::~ChildrenCountingVisitor() {}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::Definitions &defs) const
{
    return static_cast<int>(defs.types().size()) + static_cast<int>(defs.values().size());
}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::File &file) const
{
    return static_cast<int>(file.definitionsList().size());
}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::Project &project) const
{
    return static_cast<int>(project.files().size());
}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::TypeAssignment &type) const
{
    Q_UNUSED(type);
    return 0;
}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::ValueAssignment &value) const
{
    Q_UNUSED(value);
    return 0;
}

int ChildrenCountingVisitor::valueFor(const Asn1Acn::Root &root) const
{
    return static_cast<int>(root.projects().size());
}
