/****************************************************************************
**
** Copyright (C) 2017 N7 Mobile sp. z o. o.
** Contact: http://n7mobile.pl/Space
**
** This file is part of ASN.1/ACN Plugin for QtCreator.
**
** Plugin was developed under a programme and funded by
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
#include "project.h"

#include <utils/qtcassert.h>

#include "visitor.h"

#include "QDebug"

using namespace Asn1Acn::Internal::Data;

Project::Project(const QString &projectName)
    : Node({})
    , m_name(projectName)
{
}

Project::~Project()
{
}

void Project::accept(Visitor &visitor) const
{
    visitor.visit(*this);
}

void Project::add(std::unique_ptr<File> file)
{
    const QString path = file->location().path();

    remove(path);

    file->setParent(this);
    m_filesByPathMap[path] = file.get();
    m_files.push_back(std::move(file));
}

void Project::remove(const QString &path)
{
    const auto mapIt = m_filesByPathMap.find(path);
    if (mapIt != m_filesByPathMap.end())
        m_filesByPathMap.erase(mapIt);

    for (auto vecIt = m_files.begin(); vecIt != m_files.end(); vecIt++) {
        if ((*vecIt)->source().filePath() == path) {
            m_files.erase(vecIt);
            break;
        }
    }
}

const File *Project::file(const QString &path) const
{
    const auto it = m_filesByPathMap.find(path);

    return it != m_filesByPathMap.end() ? it->second : nullptr;
}
