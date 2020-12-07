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

#pragma once

#include "exportableaadlobject.h"

namespace aadl {
class AADLObjectConnectionGroup;
}

namespace aadlinterface {

class ExportableAADLConnectionGroup : public ExportableAADLObject
{
    Q_GADGET
    Q_PROPERTY(QString name READ connectionGroupName)
    Q_PROPERTY(QString source READ sourceName)
    Q_PROPERTY(QString target READ targetName)
    Q_PROPERTY(QVariantMap sifaces READ sourceInterfaces)
    Q_PROPERTY(QVariantMap tifaces READ targetInterfaces)

public:
    ExportableAADLConnectionGroup(const aadl::AADLObjectConnectionGroup *connection = nullptr);

    QString connectionGroupName() const;
    QString sourceName() const;
    QString targetName() const;
    QVariantMap sourceInterfaces() const;
    QVariantMap targetInterfaces() const;
};

} // namespace aadlinterface

Q_DECLARE_METATYPE(aadlinterface::ExportableAADLConnectionGroup)
Q_DECLARE_TYPEINFO(aadlinterface::ExportableAADLConnectionGroup, Q_MOVABLE_TYPE);
