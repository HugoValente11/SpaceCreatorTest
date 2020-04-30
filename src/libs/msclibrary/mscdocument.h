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

#pragma once

#include "mscentity.h"

#include <QObject>
#include <QString>
#include <QVector>

namespace msc {
class MscChart;
class MscMessageDeclarationList;

class MscDocument : public MscEntity
{
    Q_OBJECT
public:
    enum HierarchyType
    {
        HierarchyLeaf,
        HierarchyIs,
        HierarchyAnd,
        HierarchyOr,
        HierarchyParallel,
        HierarchyRepeat,
        HierarchyException,
        HierarchyUnkown /// Only used when loaded from a file
    };

    explicit MscDocument(QObject *parent = nullptr);
    MscDocument(const QString &name, QObject *parent = nullptr);
    ~MscDocument() override;

    const QVector<MscDocument *> &documents() const;
    bool addDocument(MscDocument *document, int index = -1);
    bool addDocument(MscDocument *document, bool checkType, int index = -1);
    void removeDocument(MscDocument *document, bool clear = true);

    const QVector<MscChart *> &charts() const;
    void addChart(MscChart *chart);

    MscMessageDeclarationList *messageDeclarations() const;
    MscMessageDeclarationList *messageDeclarations();

    void clear();
    void clearCharts();

    MscDocument *parentDocument() const;

    MscEntity::EntityType entityType() const override;

    HierarchyType hierarchyType() const;
    void setHierarchyType(HierarchyType type);

    bool isSingleChildDocument() const;
    bool isAddChildEnable() const;

    int maxInstanceNameNumber() const;
    int setInstanceNameNumbers(int nextNumber);

Q_SIGNALS:
    void documentAdded(MscDocument *document);
    void documentRemoved(MscDocument *document);
    void chartAdded(MscChart *chart);
    void cleared();
    void hierarchyTypeChanged(HierarchyType type);

private:
    QVector<MscDocument *> m_documents;
    QVector<MscChart *> m_charts;
    MscMessageDeclarationList *m_messageDeclarations = nullptr;

    HierarchyType m_hierarchyType = HierarchyAnd;
};

} // namespace msc