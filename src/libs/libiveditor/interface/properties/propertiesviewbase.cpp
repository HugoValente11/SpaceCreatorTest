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

#include "propertiesviewbase.h"

#include "propertieslistmodel.h"
#include "aadlobjectfunction.h"
#include "aadlobjectiface.h"
#include "ui_propertiesviewbase.h"

#include <QDebug>
#include <QSortFilterProxyModel>

namespace aadlinterface {

PropertiesViewBase::PropertiesViewBase(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PropertiesViewBase)
{
    ui->setupUi(this);
    setButtonsDisabled();
}

PropertiesViewBase::~PropertiesViewBase()
{
    delete ui;
}

void PropertiesViewBase::setModel(PropertiesModelBase *model)
{
    if (model == m_model)
        return;

    if (tableView()->selectionModel())
        disconnect(tableView()->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                   &PropertiesViewBase::onCurrentRowChanged);

    m_model = model;
    tableView()->setModel(m_model);
    if (m_model->rowCount())
        tableView()->resizeColumnsToContents();

    if (tableView()->selectionModel())
        connect(tableView()->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
                &PropertiesViewBase::onCurrentRowChanged);

    setButtonsDisabled();
}

QTableView *PropertiesViewBase::tableView() const
{
    return ui->tableView;
}

void PropertiesViewBase::onCurrentRowChanged(const QModelIndex &current, const QModelIndex &)
{
    if (m_model && !setButtonsDisabled()) {
        ui->btnDel->setEnabled(current.isValid() && m_model->isProp(current));
    }
}

void PropertiesViewBase::on_btnAdd_clicked()
{
    if (m_model) {
        static const QString newNameTmp = tr("New property");
        QString newName(newNameTmp);
        int duplicateCounter(0);
        while (!m_model->findItems(newName).isEmpty())
            newName = QString("%1 #%2").arg(newNameTmp, QString::number(++duplicateCounter));

        if (m_model->createProperty(newName)) {
            auto updateColumnsWidth = [this]() { // #QTBUG-52307
                ui->tableView->horizontalHeader()->setStretchLastSection(false);
                ui->tableView->resizeColumnsToContents();
                ui->tableView->horizontalHeader()->setStretchLastSection(true);
            };
            const QModelIndex &added = m_model->index(m_model->rowCount() - 1, 0);
            updateColumnsWidth();
            ui->tableView->scrollToBottom();
            ui->tableView->edit(added);
        }
    }
}

void PropertiesViewBase::on_btnDel_clicked()
{
    if (m_model) {
        m_model->removeProperty(ui->tableView->currentIndex());
        ui->tableView->update();
    }
}

bool PropertiesViewBase::setButtonsDisabled()
{
    if (!m_model)
        return false;

    bool disabled = false;

    if (auto dataObject = m_model->dataObject()) {
        switch (dataObject->aadlType()) {
        case aadl::AADLObject::Type::Function: {
            if (auto fn = dataObject->as<const aadl::AADLObjectFunction *>())
                disabled = fn->inheritsFunctionType();
            break;
        }
        case aadl::AADLObject::Type::RequiredInterface:
        case aadl::AADLObject::Type::ProvidedInterface: {
            if (auto iface = dataObject->as<const aadl::AADLObjectIface *>()) {
                disabled = iface->isClone();
                if (!disabled && iface->isRequired()) {
                    if (auto ri = iface->as<const aadl::AADLObjectIfaceRequired *>())
                        disabled = ri->hasPrototypePi();
                }
            }

            break;
        }
        default:
            break;
        }
    }

    if (disabled) {
        ui->btnAdd->setDisabled(disabled);
        ui->btnDel->setDisabled(disabled);
    }

    return disabled;
}

}