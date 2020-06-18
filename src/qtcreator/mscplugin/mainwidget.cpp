/*
   Copyright (C) 2018 - 2019 European Space Agency - <maxime.perrotin@esa.int>

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

#include "mainwidget.h"

#include "asn1fileview.h"
#include "chartlayoutmanager.h"
#include "documentitemmodel.h"
#include "documenttreeview.h"
#include "graphicsview.h"
#include "mainmodel.h"
#include "mscchart.h"
#include "mscdocument.h"
#include "mscmodel.h"
#include "mscplugin.h"
#include "tools/entitydeletetool.h"

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QImage>
#include <QSplitter>
#include <QStackedWidget>
#include <QUndoStack>
#include <QtWidgets/QHeaderView>
#include <coreplugin/minisplitter.h>

namespace MscPlugin {

/*!
   \brief MscPlugin::MainWidget::MainWidget Is the main widget for the whole MSC plugin in QtCreator
 */

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , m_plugin(new msc::MSCPlugin(this))
{
    initUi();

    initActions();
    m_plugin->createActionCopy(nullptr);
    m_plugin->createActionPaste(nullptr);
    m_plugin->initChartTools();
    m_plugin->initHierarchyViewActions();
    initConnections();

    m_plugin->showDocumentView(true);
}

MainWidget::~MainWidget()
{
    if (m_documentTree->model()) {
        disconnect(m_documentTree->model(), nullptr, this, nullptr);
    }
    disconnect(&(m_plugin->mainModel()->chartViewModel()), nullptr, this, nullptr);
    disconnect(m_plugin->mainModel(), nullptr, this, nullptr);
    disconnect(m_plugin->mainModel()->undoStack(), nullptr, this, nullptr);
}

bool MainWidget::load(const QString &filename)
{
    m_plugin->mainModel()->chartViewModel().setPreferredChartBoxSize(QSizeF());

    const bool ok = m_plugin->mainModel()->loadFile(filename);
    if (ok) {
        m_plugin->chartView()->setZoom(100);
    }

    return ok;
}

bool MainWidget::save()
{
    return m_plugin->mainModel()->saveMsc(m_plugin->mainModel()->currentFilePath());
}

QString MainWidget::errorMessage() const
{
    return m_plugin->mainModel()->mscErrorMessages().join("\n");
}

void MainWidget::setFileName(const QString &filename)
{
    m_plugin->mainModel()->setCurrentFilePath(filename);
}

bool MainWidget::isDirty() const
{
    return m_plugin->mainModel()->needSave();
}

QUndoStack *MainWidget::undoStack()
{
    return m_plugin->mainModel()->undoStack();
}

QString MainWidget::textContents() const
{
    return m_plugin->mainModel()->modelText();
}

QAction *MainWidget::actionCopy() const
{
    return m_plugin->actionCopy();
}

QAction *MainWidget::actionPaste() const
{
    return m_plugin->actionPaste();
}

QAction *MainWidget::actionToolDelete() const
{
    return m_plugin->deleteTool()->action();
}

QVector<QAction *> MainWidget::toolActions() const
{
    return m_plugin->chartActions();
}

msc::MSCPlugin *MainWidget::mscPlugin() const
{
    return m_plugin.get();
}

void MainWidget::showChart(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    auto *obj = static_cast<QObject *>(index.internalPointer());
    if (obj == nullptr) {
        return;
    }

    if (auto document = dynamic_cast<msc::MscDocument *>(obj)) {
        if (!document->charts().empty()) {
            m_plugin->mainModel()->chartViewModel().setCurrentChart(document->charts()[0]);
            m_plugin->showDocumentView(true);
        }
    }
}

void MainWidget::showSelection(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (!current.isValid()) {
        return;
    }

    auto *obj = static_cast<QObject *>(current.internalPointer());
    if (obj == nullptr) {
        m_plugin->mainModel()->setSelectedDocument(nullptr);
        return;
    }

    auto chart = dynamic_cast<msc::MscChart *>(obj);
    if (!chart) {
        if (auto document = dynamic_cast<msc::MscDocument *>(obj)) {
            if (document && document->hierarchyType() == msc::MscDocument::HierarchyLeaf) {
                if (!document->charts().isEmpty())
                    chart = document->charts().at(0);
            }
        }
    }
    if (chart) {
        m_plugin->mainModel()->chartViewModel().setCurrentChart(chart);
        m_plugin->showDocumentView(true);
    } else {
        m_plugin->showHierarchyView(true);

        if (auto document = dynamic_cast<msc::MscDocument *>(obj)) {
            bool canNewChild = true;
            if (document->hierarchyType() == msc::MscDocument::HierarchyType::HierarchyLeaf)
                canNewChild = false;
            if (document->documents().size() > 0
                    && (document->hierarchyType() == msc::MscDocument::HierarchyType::HierarchyRepeat
                            || document->hierarchyType() == msc::MscDocument::HierarchyType::HierarchyIs
                            || document->hierarchyType() == msc::MscDocument::HierarchyType::HierarchyException)) {
                canNewChild = false;
            }
            for (QAction *action : m_plugin->hierarchyActions()) {
                action->setEnabled(canNewChild);
            }

            m_plugin->mainModel()->setSelectedDocument(document);
        }
    }
}

void MainWidget::initUi()
{
    auto centerView = new QStackedWidget(this);
    auto graphicsView = new msc::GraphicsView(this);
    centerView->addWidget(graphicsView);
    auto hierarchyView = new msc::GraphicsView(this);
    centerView->addWidget(hierarchyView);

    m_plugin->setViews(centerView, graphicsView, hierarchyView);

    m_documentTree = new msc::DocumentTreeView(this);
    m_documentTree->header()->setVisible(true);

    m_asn1Widget = new msc::ASN1FileView(this);

    m_leftVerticalSplitter = new Core::MiniSplitter(Qt::Vertical);
    m_leftVerticalSplitter->addWidget(m_documentTree);
    m_leftVerticalSplitter->addWidget(m_asn1Widget);
    m_leftVerticalSplitter->setStretchFactor(0, 1);
    m_leftVerticalSplitter->setStretchFactor(1, 0);

    m_horizontalSplitter = new Core::MiniSplitter(Qt::Horizontal);
    m_horizontalSplitter->addWidget(m_leftVerticalSplitter);
    m_horizontalSplitter->addWidget(centerView);
    m_horizontalSplitter->setStretchFactor(0, 0);
    m_horizontalSplitter->setStretchFactor(1, 1);

    setLayout(new QVBoxLayout);
    layout()->addWidget(m_horizontalSplitter);
    layout()->setMargin(0);

    m_documentTree->setModel(m_plugin->mainModel()->documentItemModel());
    m_asn1Widget->setModel(m_plugin->mainModel()->mscModel());
}

void MainWidget::initConnections()
{
    connect(m_documentTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWidget::showSelection);
    connect(m_documentTree, &QTreeView::doubleClicked, this, &MainWidget::showChart);

    connect(m_plugin->mainModel(), &msc::MainModel::selectedDocumentChanged, m_documentTree,
            &msc::DocumentTreeView::setSelectedDocument);

    connect(m_plugin->mainModel(), &msc::MainModel::modelUpdated, m_asn1Widget, &msc::ASN1FileView::setModel);

    connect(m_plugin->mainModel()->documentItemModel(), &msc::DocumentItemModel::dataChanged, this,
            &MainWidget::showSelection);

    connect(m_plugin->mainModel()->undoStack(), &QUndoStack::indexChanged, this,
            [&]() { Q_EMIT dirtyChanged(isDirty()); });
    connect(m_plugin->mainModel(), &msc::MainModel::lasteSaveUndoChange, this,
            [&]() { Q_EMIT dirtyChanged(isDirty()); });

    connect(m_plugin->mainModel(), &msc::MainModel::currentFilePathChanged, this, [&](const QString &filename) {
        QFileInfo fileInfo(filename);
        m_asn1Widget->setCurrentDirectory(fileInfo.absolutePath());
    });
}

}
