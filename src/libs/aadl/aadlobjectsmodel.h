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
#include "aadlobjectiface.h"

#include <QAbstractItemModel>
#include <QVector>
#include <memory>

namespace aadl {

class AADLObjectComment;
class AADLObjectConnection;
class AADLObjectFunction;
class AADLObjectFunctionType;

struct AADLObjectsModelPrivate;
class AADLObjectsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum class AADLRoles
    {
        IdRole = Qt::UserRole,
        TypeRole
    };

    explicit AADLObjectsModel(QObject *parent = nullptr);
    ~AADLObjectsModel() override;

    bool addObject(AADLObject *obj);
    bool removeObject(AADLObject *obj);

    void setRootObject(shared::Id id);
    AADLObject *rootObject() const;
    shared::Id rootObjectId() const;

    AADLObject *getObject(const shared::Id &id) const;
    AADLObject *getObjectByName(const QString &name, AADLObject::Type type = AADLObject::Type::Unknown) const;
    AADLObjectIface *getIfaceByName(
            const QString &name, AADLObjectIface::IfaceType dir, const AADLObjectFunctionType *parent = nullptr) const;
    AADLObjectFunction *getFunction(const shared::Id &id) const;
    AADLObjectFunction *getFunction(const QString &name) const;
    AADLObjectFunctionType *getFunctionType(const shared::Id &id) const;
    QHash<QString, AADLObjectFunctionType *> getAvailableFunctionTypes(const AADLObjectFunction *fnObj) const;
    AADLObjectIface *getInterface(const shared::Id &id) const;
    AADLObjectIfaceRequired *getRequiredInterface(const shared::Id &id) const;
    AADLObjectIfaceProvided *getProvidedInterface(const shared::Id &id) const;
    AADLObjectConnection *getConnection(const shared::Id &id) const;
    AADLObjectComment *getCommentById(const shared::Id &id) const;
    AADLObjectConnection *getConnection(
            const QString &interfaceName, const QString &source, const QString &target) const;

    AADLObjectConnection *getConnectionForIface(const shared::Id &id) const;
    QVector<AADLObjectConnection *> getConnectionsForIface(const shared::Id &id) const;

    const QHash<shared::Id, AADLObject *> &objects() const;

    QList<AADLObject *> visibleObjects() const;
    QList<AADLObject *> visibleObjects(shared::Id rootId) const;

    void clear();

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex indexFromObject(AADLObject *object) const;
    AADLObject *objectFromIndex(const QModelIndex &index) const;

private:
    int rowInParent(AADLObject *obj) const;

Q_SIGNALS:
    void aadlObjectAdded(aadl::AADLObject *object);
    void aadlObjectRemoved(aadl::AADLObject *object);
    void modelReset();
    void rootObjectChanged(shared::Id rootId);

public Q_SLOTS:
    bool initFromObjects(const QVector<aadl::AADLObject *> &visibleObjects);

private:
    const std::unique_ptr<AADLObjectsModelPrivate> d;
};

}
