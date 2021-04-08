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

#include "contextparametersmodel.h"

#include "aadlcommonprops.h"
#include "aadlfunction.h"
#include "aadlnamevalidator.h"
#include "aadlobject.h"
#include "asn1/file.h"
#include "commandsstack.h"
#include "interface/commands/cmdcontextparameterchange.h"
#include "interface/commands/cmdcontextparametercreate.h"
#include "interface/commands/cmdcontextparameterremove.h"
#include "propertytemplate.h"
#include "propertytemplateconfig.h"

#include <algorithm>

namespace ive {

ContextParametersModel::ContextParametersModel(cmd::CommandsStack::Macro *macro, QObject *parent)
    : PropertiesModelBase(parent)
    , m_cmdMacro(macro)
{
}

ContextParametersModel::~ContextParametersModel() { }

void ContextParametersModel::createNewRow(const ivm::ContextParameter &param, int row)
{
    QStandardItem *titleItem = new QStandardItem(param.name());
    QStandardItem *typeItem = new QStandardItem(param.paramTypeName());
    QStandardItem *valueItem = new QStandardItem();
    valueItem->setData(param.defaultValue(), Qt::EditRole);

    setItem(row, Column::Name, titleItem);
    setItem(row, Column::Type, typeItem);
    setItem(row, Column::Value, valueItem);

    m_params.insert(row, param);
}

void ContextParametersModel::setDataObject(ivm::AADLObject *obj)
{
    clear();
    m_params.clear();
    m_dataObject = obj;

    if (!m_dataObject)
        return;

    if (auto func = qobject_cast<ivm::AADLFunctionType *>(m_dataObject)) {
        const int paramsCount = func->contextParams().size();

        beginInsertRows(QModelIndex(), 0, paramsCount);

        for (int i = 0; i < paramsCount; ++i) {
            auto param = func->contextParams().at(i);
            createNewRow(param, i);
        }

        endInsertRows();
    }
}

const ivm::AADLObject *ContextParametersModel::dataObject() const
{
    return m_dataObject;
}

void ContextParametersModel::setDataTypes(const QSharedPointer<Asn1Acn::File> &dataTypes)
{
    m_dataTypes = dataTypes;
}

int ContextParametersModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_dataObject ? m_params.size() : 0;
}

int ContextParametersModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant ContextParametersModel::data(const QModelIndex &index, int role) const
{
    const QVariant &res = QStandardItemModel::data(index, role);
    if (!index.isValid())
        return res;

    auto param = m_params.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        switch (index.column()) {
        case Column::Type:
            return param.paramTypeName();
        case Column::Value:
            return param.defaultValue();
        default:
            return ivm::AADLNameValidator::decodeName(m_dataObject->aadlType(), param.name());
        }
    }
    }

    return res;
}

bool ContextParametersModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || data(index, role) == value)
        return false;

    if (role == Qt::EditRole) {
        auto paramOld = m_params.at(index.row());
        ivm::ContextParameter paramNew(paramOld);

        switch (index.column()) {
        case Column::Name: {
            if (!paramNew.setName(ivm::AADLNameValidator::encodeName(m_dataObject->aadlType(), value.toString())))
                return false;
            break;
        }
        case Column::Type: {
            if (!paramNew.setParamTypeName(value.toString()))
                return false;
            break;
        }
        case Column::Value: {
            if (!m_dataTypes.data()) {
                return true;
            }
            const Asn1Acn::Types::Type *basicDataType = m_dataTypes->typeFromName(paramNew.paramTypeName());
            if (!paramNew.setDefaultValue(basicDataType, value)) {
                return false;
            }
            break;
        }
        default:
            return false;
        }

        if (auto entity = qobject_cast<ivm::AADLFunctionType *>(m_dataObject)) {
            auto attributesCmd = new cmd::CmdContextParameterChange(entity, paramOld, paramNew);
            m_cmdMacro->push(attributesCmd);
            m_params.replace(index.row(), paramNew);
        }
    }

    QStandardItemModel::setData(index, value, role);
    Q_EMIT dataChanged(index, index, { role, Qt::DisplayRole });
    return true;
}

bool ContextParametersModel::createProperty(const QString &propName)
{
    bool res(false);

    ivm::ContextParameter param(propName);
    param.setParamType(ivm::BasicParameter::Type::Timer);

    if (auto entity = qobject_cast<ivm::AADLFunctionType *>(m_dataObject)) {
        auto propsCmd = new cmd::CmdContextParameterCreate(entity, param);
        const int rows = rowCount();
        beginInsertRows(QModelIndex(), rows, rows);

        m_cmdMacro->push(propsCmd);
        createNewRow(param, rows);
        res = true;

        endInsertRows();
    }

    return res;
}

bool ContextParametersModel::removeProperty(const QModelIndex &index)
{
    bool res(false);
    if (!index.isValid())
        return res;

    const int row(index.row());
    if (auto entity = qobject_cast<ivm::AADLFunctionType *>(m_dataObject)) {
        auto propsCmd = new cmd::CmdContextParameterRemove(entity, row);
        m_cmdMacro->push(propsCmd);
        removeRow(row);
        m_params.removeAt(row);

        res = true;
    }

    return res;
}

bool ContextParametersModel::isAttr(const QModelIndex & /*id*/) const
{
    return false;
}

bool ContextParametersModel::isProp(const QModelIndex & /*id*/) const
{
    return true;
}

Qt::ItemFlags ContextParametersModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QStandardItemModel::flags(index);
    if (index.column() == Column::Value && m_params.at(index.row()).paramType() != ivm::BasicParameter::Type::Other)
        flags = flags & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled;

    if (!m_dataObject)
        return flags;

    if (flags.testFlag(Qt::ItemIsEditable) || flags.testFlag(Qt::ItemIsEnabled)) {
        switch (m_dataObject->aadlType()) {
        case ivm::AADLObject::Type::Function: {
            if (auto fn = m_dataObject->as<const ivm::AADLFunction *>())
                if (fn->inheritsFunctionType())
                    flags = flags & ~Qt::ItemIsEditable & ~Qt::ItemIsEnabled;

            break;
        }
        default:
            break;
        }
    }

    return flags;
}

QVariant ContextParametersModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Column::Name:
            return tr("Name");
        case Column::Type:
            return tr("Type");
        case Column::Value:
            return tr("Value");
        }
    }
    return QVariant();
}

}
