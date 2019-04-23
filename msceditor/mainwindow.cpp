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

#include "mainwindow.h"

#include "baseitems/common/coordinatesconverter.h"
#include "chartviewmodel.h"
#include "commandlineparser.h"
#include "commands/common/commandsstack.h"
#include "documentitemmodel.h"
#include "graphicsview.h"
#include "mainmodel.h"
#include "messagedeclarationsdialog.h"
#include "mscaction.h"
#include "mscchart.h"
#include "msccondition.h"
#include "msccreate.h"
#include "mscdocument.h"
#include "mscmessage.h"
#include "mscmessagedeclarationlist.h"
#include "mscmodel.h"
#include "msctimer.h"
#include "remotecontrolhandler.h"
#include "remotecontrolwebserver.h"
#include "settings/appoptions.h"
#include "textview.h"
#include "tools/actioncreatortool.h"
#include "tools/commentcreatortool.h"
#include "tools/conditioncreatortool.h"
#include "tools/coregioncreatortool.h"
#include "tools/entitydeletetool.h"
#include "tools/hierarchycreatortool.h"
#include "tools/instancecreatortool.h"
#include "tools/instancestoptool.h"
#include "tools/messagecreatortool.h"
#include "tools/pointertool.h"
#include "tools/timercreatortool.h"
#include "ui_mainwindow.h"

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMessageBox>
#include <QMetaEnum>
#include <QMimeData>
#include <QToolBar>
#include <QUndoGroup>
#include <QUndoStack>
#include <QVector>

const QByteArray HIERARCHY_TYPE_TAG = "hierarchyTag";

const QLatin1String MainWindow::DotMscFileExtensionLow = QLatin1String(".msc");

struct MainWindowPrivate {
    explicit MainWindowPrivate(MainWindow *mainWindow)
        : ui(new Ui::MainWindow)
        , m_model(new MainModel(mainWindow))
        , m_mscToolBar(new QToolBar(QObject::tr("MSC"), mainWindow))
        , m_hierarchyToolBar(new QToolBar(QObject::tr("Hierarchy"), mainWindow))
        , m_undoGroup(new QUndoGroup(mainWindow))
    {
        m_mscToolBar->setObjectName("mscTools");
        m_mscToolBar->setAllowedAreas(Qt::AllToolBarAreas);
        mainWindow->addToolBar(Qt::LeftToolBarArea, m_mscToolBar);

        m_hierarchyToolBar->setObjectName("hierarchyTools");
        m_hierarchyToolBar->setAllowedAreas(Qt::AllToolBarAreas);
        mainWindow->addToolBar(Qt::LeftToolBarArea, m_hierarchyToolBar);
    }

    ~MainWindowPrivate() { delete ui; }

    Ui::MainWindow *ui = nullptr;

    QComboBox *m_zoomBox = nullptr;

    MainModel *m_model = nullptr;
    QToolBar *m_mscToolBar = nullptr;
    QToolBar *m_hierarchyToolBar = nullptr;
    QUndoGroup *m_undoGroup = nullptr;

    QMenu *m_menuFile = nullptr;
    QAction *m_actNewFile = nullptr;
    QAction *m_actOpenFile = nullptr;
    QAction *m_actSaveFile = nullptr;
    QAction *m_actSaveFileAs = nullptr;
    QAction *m_actQuit = nullptr;

    QMenu *m_menuEdit = nullptr;
    QAction *m_actUndo = nullptr;
    QAction *m_actRedo = nullptr;

    QAction *m_actCopy = nullptr;
    QAction *m_actPaste = nullptr;

    QMenu *m_menuView = nullptr;
    QAction *m_actShowDocument = nullptr;
    QAction *m_actShowHierarchy = nullptr;

    QMenu *m_menuViewWindows = nullptr;
    QAction *m_actToggleErrorView = nullptr;
    QAction *m_actToggleHierarchyView = nullptr;
    QAction *m_actToggleMscTextView = nullptr;
    QAction *m_actToggleAsn1View = nullptr;

    QMenu *m_menuHelp = nullptr;
    QAction *m_actAboutQt = nullptr;

    QVector<msc::BaseTool *> m_tools;
    QAction *m_defaultToolAction = nullptr;
    msc::InstanceStopTool *m_instanceStopTool = nullptr;
    msc::InstanceCreatorTool *m_instanceCreatorTool = nullptr;
    msc::EntityDeleteTool *m_deleteTool = nullptr;
    msc::MessageCreatorTool *m_messageCreateTool = nullptr;

    RemoteControlWebServer *m_remoteControlWebServer = nullptr;
    RemoteControlHandler *m_remoteControlHandler = nullptr;

    bool m_dropUnsavedChangesSilently = false;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    setupUi();
    d->ui->hierarchyView->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing
                                         | QPainter::SmoothPixmapTransform);

    initConnections();

    selectCurrentChart();

    loadSettings();

    d->m_mscToolBar->setVisible(d->ui->centerView->currentWidget() == d->ui->graphicsView);
    d->m_hierarchyToolBar->setVisible(d->ui->centerView->currentWidget() == d->ui->hierarchyView);
}

MainWindow::~MainWindow()
{
    if (d->ui->documentTreeView->model()) {
        disconnect(d->ui->documentTreeView->model(), nullptr, this, nullptr);
    }
    disconnect(&(d->m_model->chartViewModel()), nullptr, this, nullptr);

    // Had this connection not dropped, the currentUndoStack() would need check
    // for nullptr d, d->ui, d->ui->graphicsView
    disconnect(d->m_model->undoStack(), &QUndoStack::indexChanged, this, &MainWindow::updateTitles);
}

QGraphicsView *MainWindow::currentView() const
{
    return d->ui->graphicsView;
}

void MainWindow::createNewDocument()
{
    if (!saveDocument()) {
        return;
    }

    d->m_model->chartViewModel().setPreferredChartBoxSize(prepareChartBoxSize());
    d->m_model->initialModel();

    d->ui->graphicsView->setZoom(100);
}

void MainWindow::selectAndOpenFile()
{
    if (!saveDocument()) {
        return;
    }

    static const QStringList suffixes = { QString("MSC files (%1)").arg(mscFileFilters().join(" ")),
                                          QString("All files (*.*)") };

    const QString path = QFileInfo(d->m_model->currentFilePath()).absoluteFilePath();

    const QString filename = QFileDialog::getOpenFileName(this, tr("MSC"), path, suffixes.join(";;"));
    if (!filename.isEmpty()) {
        d->ui->errorTextEdit->clear();
        openFileMsc(filename);
    }
}

bool MainWindow::openFileMsc(const QString &file)
{
    d->ui->errorTextEdit->appendPlainText(tr("Opening file: %1").arg(file));

    QFileInfo fileInfo(file);
    if (!fileInfo.exists()) {
        d->ui->errorTextEdit->appendPlainText(tr("File not exists."));
        return false;
    }

    d->m_model->chartViewModel().setPreferredChartBoxSize(QSizeF());

    const bool ok = d->m_model->loadFile(file);
    if (ok) {
        d->ui->graphicsView->setZoom(100);
    }

    if (!d->m_model->mscErrorMessages().isEmpty())
        showErrorView();

    d->ui->errorTextEdit->appendHtml(d->m_model->mscErrorMessages().join("\n"));
    QString loadStatus = tr("success");
    QString statusColor = "black";
    if (!d->m_model->mscErrorMessages().isEmpty()) {
        loadStatus = tr("warnings");
        statusColor = "orange";
    }
    if (!ok) {
        loadStatus = tr("failed");
        statusColor = "red";
    }
    d->ui->errorTextEdit->appendHtml(
            tr("Model loading: <b><font color=%2>%1</font></b><br>").arg(loadStatus, statusColor));

    return ok;
}

void MainWindow::updateTitles()
{
    static const QString title = tr("%1 [%2]%3");

    const QString dirtyMarker(d->m_model->needSave() ? "*" : "");
    const QString &filename = d->m_model->currentFilePath();
    const QString mscFileName(filename.isEmpty() ? tr("Untitled") : QFileInfo(filename).fileName());
    setWindowTitle(title.arg(qApp->applicationName(), mscFileName, dirtyMarker));

    d->m_actSaveFile->setText(tr("&Save \"%1\"").arg(mscFileName));
}

bool MainWindow::openMscChain(const QString &dirPath)
{
    if (dirPath.isEmpty())
        return false;

    QDir dir(dirPath);
    if (!dir.exists() || !dir.isReadable())
        return false;

    for (const QFileInfo &file : dir.entryInfoList(mscFileFilters())) {
        openFileMsc(file.absoluteFilePath());
    }
    return true;
}

void MainWindow::activateDefaultTool()
{
    Q_ASSERT(d->m_defaultToolAction);
    d->m_defaultToolAction->setChecked(true);
}

void MainWindow::updateTextView()
{
    if (!d->ui->dockWidgetMscText->isVisible()) {
        return;
    }
    d->ui->mscTextBrowser->updateView();
}

void MainWindow::saveMsc()
{
    const QString &filename = d->m_model->currentFilePath();
    if (filename.isEmpty()) {
        saveAsMsc();
    } else {
        d->m_model->saveMsc(filename);
    }
}

void MainWindow::saveAsMsc()
{
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save as..."), QFileInfo(d->m_model->currentFilePath()).path(),
                                         tr("MSC files (%1);;All files (*.*)").arg(mscFileFilters().join(" ")));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(DotMscFileExtensionLow))
            fileName.append(DotMscFileExtensionLow);
        d->m_model->setCurrentFilePath(fileName);

        saveMsc();
    }
}

void MainWindow::showDocumentView(bool show)
{
    if (show) {
        d->ui->centerView->setCurrentWidget(d->ui->graphicsView);

        d->m_hierarchyToolBar->hide();
        d->m_mscToolBar->show();

        d->m_actCopy->setEnabled(true);
        msc::MscChart *chart = d->m_model->chartViewModel().currentChart();
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        const bool clipBoardHasMscChart = mimeData ? mimeData->hasFormat(MainModel::MscChartMimeType) : false;
        d->m_actPaste->setEnabled(clipBoardHasMscChart && chart && chart->instances().isEmpty());

        d->m_deleteTool->setView(d->ui->graphicsView);
        d->m_deleteTool->setCurrentChart(chart);

        updateZoomBox(d->ui->graphicsView->zoom());
    }
}

void MainWindow::showHierarchyView(bool show)
{
    if (show) {
        d->ui->centerView->setCurrentWidget(d->ui->hierarchyView);

        d->m_hierarchyToolBar->show();
        d->m_mscToolBar->hide();

        d->m_actCopy->setEnabled(false);
        d->m_actPaste->setEnabled(false);

        d->m_deleteTool->setView(d->ui->hierarchyView);
        d->m_deleteTool->setCurrentChart(nullptr);

        updateZoomBox(d->ui->hierarchyView->zoom());
    }
}

void MainWindow::showErrorView()
{
    if (!d->ui->dockWidgetErrors->isVisible()) {
        d->m_actToggleErrorView->trigger();
    }
}

void MainWindow::selectCurrentChart()
{
    msc::MscChart *chart = d->m_model->chartViewModel().currentChart();

    if (chart != nullptr) {
        if (QUndoStack *currentStack = d->m_model->undoStack()) {
            if (!d->m_undoGroup->stacks().contains(currentStack))
                d->m_undoGroup->addStack(currentStack);
            d->m_undoGroup->setActiveStack(currentStack);
        } else {
            d->m_undoGroup->removeStack(d->m_undoGroup->activeStack());
        }
        msc::cmd::CommandsStack::setCurrent(d->m_undoGroup->activeStack());

        // TODO: add support for dedicated stacks for each tab
    }

    if (d->m_deleteTool) {
        d->m_deleteTool->setCurrentChart(chart);
    }
}

void MainWindow::showChart(const QModelIndex &index)
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
            d->m_model->chartViewModel().fillView(document->charts()[0]);
            showDocumentView(true);
        }
    }
}

void MainWindow::showSelection(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (!current.isValid()) {
        return;
    }

    auto *obj = static_cast<QObject *>(current.internalPointer());
    if (obj == nullptr) {
        d->m_model->setSelectedDocument(nullptr);
        return;
    }

    if (auto chart = dynamic_cast<msc::MscChart *>(obj)) {
        d->m_model->chartViewModel().fillView(chart);
        showDocumentView(true);
    } else {
        showHierarchyView(true);

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
            for (auto action : d->m_hierarchyToolBar->actions()) {
                action->setEnabled(canNewChild);
            }

            d->m_model->setSelectedDocument(document);
            d->m_actPaste->setEnabled(QApplication::clipboard()->mimeData()->hasFormat(MainModel::MscChartMimeType)
                                      && d->m_model->selectedDocument()->isAddChildEnable());
        }
    }
}

void MainWindow::setupUi()
{
    d->ui->setupUi(this);

    d->ui->graphicsView->setScene(d->m_model->graphicsScene());
    //    d->ui->graphicsView->setAlignment(Qt::AlignTop | Qt::AlignLeft); // make scene's origin be great^W in topLeft
    d->ui->documentTreeView->setModel(d->m_model->documentItemModel());

    d->ui->hierarchyView->setBackgroundBrush(QImage(":/resources/resources/texture.png"));
    d->ui->hierarchyView->setScene(d->m_model->hierarchyScene());

    d->ui->mscTextBrowser->setModel(d->m_model->mscModel());
    d->ui->asn1Widget->setModel(d->m_model->mscModel());

    initActions();
    initMenus();
    initTools();
    initMainToolbar();
    initDocumentViewActions();

    // status bar
    d->m_zoomBox = new QComboBox(d->ui->statusBar);
    for (auto x = d->ui->graphicsView->minZoomPercent(); x <= d->ui->graphicsView->maxZoomPercent();
         x += d->ui->graphicsView->zoomStepPercent())
        d->m_zoomBox->addItem(QString("%1 %").arg(x), x);

    d->m_zoomBox->setCurrentIndex(d->m_zoomBox->findData(100));

    connect(d->m_zoomBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
        if (auto graphicsView = dynamic_cast<msc::GraphicsView *>(d->ui->centerView->currentWidget()))
            graphicsView->setZoom(qobject_cast<QComboBox *>(sender())->itemData(index).toDouble());
    });

    connect(d->ui->graphicsView, QOverload<double>::of(&msc::GraphicsView::zoomChanged), this,
            &MainWindow::updateZoomBox);
    connect(d->ui->hierarchyView, QOverload<double>::of(&msc::GraphicsView::zoomChanged), this,
            &MainWindow::updateZoomBox);

    statusBar()->addPermanentWidget(d->m_zoomBox);
    statusBar()->show();
}

void MainWindow::initActions()
{
    d->m_actUndo = d->m_undoGroup->createUndoAction(this, tr("Undo:"));
    d->m_actUndo->setShortcut(QKeySequence::Undo);
    d->m_actUndo->setIcon(QIcon::fromTheme("edit-undo", QIcon(":/icons/toolbar/undo.svg")));

    d->m_actRedo = d->m_undoGroup->createRedoAction(this, tr("Redo:"));
    d->m_actRedo->setShortcut(QKeySequence::Redo);
    d->m_actRedo->setIcon(QIcon::fromTheme("edit-redo", QIcon(":/icons/toolbar/redo.svg")));

    d->m_actCopy = new QAction(tr("Copy:"), this);
    d->m_actCopy->setIcon(QIcon::fromTheme("edit-copy"));
    d->m_actCopy->setMenu(new QMenu(this));
    d->m_actCopy->menu()->addAction(tr("Copy Diagram"), d->m_model, &MainModel::copyCurrentChart, QKeySequence::Copy);
    d->m_actCopy->menu()->addAction(tr("Copy as Picture"), d->m_model, &MainModel::copyCurrentChartAsPicture);

    d->m_actPaste = new QAction(tr("Paste:"), this);
    d->m_actPaste->setShortcut(QKeySequence::Paste);
    d->m_actPaste->setIcon(QIcon::fromTheme("edit-paste"));
    connect(d->m_actPaste, &QAction::triggered, d->m_model, &MainModel::pasteChart);

    d->m_deleteTool = new msc::EntityDeleteTool(&(d->m_model->chartViewModel()), d->ui->graphicsView, this);
    d->m_deleteTool->setCurrentChart(d->m_model->chartViewModel().currentChart());
}

void MainWindow::initMenus()
{
    initMenuFile();
    initMenuEdit();
    initMenuView();
    initMenuHelp();
}

void MainWindow::initMenuFile()
{
    d->m_menuFile = menuBar()->addMenu(tr("File"));

    d->m_actNewFile = d->m_menuFile->addAction(style()->standardIcon(QStyle::SP_FileIcon), tr("New File"), this,
                                               &MainWindow::createNewDocument, QKeySequence::New);

    d->m_actOpenFile = d->m_menuFile->addAction(style()->standardIcon(QStyle::SP_DirOpenIcon), tr("&Open File"), this,
                                                &MainWindow::selectAndOpenFile, QKeySequence::Open);

    d->m_actSaveFile = d->m_menuFile->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("&Save"), this,
                                                &MainWindow::saveMsc, QKeySequence::Save);

    d->m_actSaveFileAs = d->m_menuFile->addAction(style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save As..."),
                                                  this, &MainWindow::saveAsMsc, QKeySequence::SaveAs);

    d->m_menuFile->addSeparator();

    d->m_actQuit = d->m_menuFile->addAction(
            tr("&Quit"), this,
            [&]() {
                if (this->saveDocument()) {
                    this->saveSettings();
                    QApplication::quit();
                }
            },
            QKeySequence::Quit);
}

void MainWindow::initMenuEdit()
{
    d->m_menuEdit = menuBar()->addMenu(tr("Edit"));
    d->m_menuEdit->addAction(d->m_actUndo);
    d->m_menuEdit->addAction(d->m_actRedo);
    d->m_menuEdit->addSeparator();
    d->m_menuEdit->addAction(d->m_deleteTool->action());
    d->m_menuEdit->addSeparator();
    d->m_menuEdit->addAction(d->m_actCopy);
    d->m_menuEdit->addAction(d->m_actPaste);
}

void MainWindow::initMenuView()
{
    d->m_menuView = menuBar()->addMenu(tr("&View"));
    d->m_actShowDocument =
            d->m_menuView->addAction(tr("Show &Document"), this, &MainWindow::showDocumentView, tr("F8"));
    d->m_actShowHierarchy =
            d->m_menuView->addAction(tr("Show &Hierarchy"), this, &MainWindow::showHierarchyView, tr("F9"));

    d->m_actShowDocument->setCheckable(true);
    d->m_actShowHierarchy->setCheckable(true);
    auto group = new QActionGroup(d->m_menuView);
    group->addAction(d->m_actShowDocument);
    group->addAction(d->m_actShowHierarchy);
    d->m_actShowDocument->setChecked(true);

    d->m_menuView->addSeparator();
    d->m_menuView->addAction(tr("Show messages ..."), this, &MainWindow::openMessageDeclarationEditor);

    initMenuViewWindows();
}

void MainWindow::initDocumentViewActions()
{
    auto addAction = [&](msc::MscDocument::HierarchyType type) {
        // create tool
        auto tool = new msc::HierarchyCreatorTool(type, &(d->m_model->hierarchyViewModel()), nullptr, this);
        auto action = d->m_hierarchyToolBar->addAction(tool->title());
        action->setProperty(HIERARCHY_TYPE_TAG, type);
        action->setCheckable(true);
        action->setIcon(tool->icon());
        action->setToolTip(tr("%1: %2").arg(tool->title(), tool->description()));
        tool->setView(d->ui->hierarchyView);

        connect(tool, &msc::HierarchyCreatorTool::documentCreated, this, [&](msc::MscDocument *document) {
            activateDefaultTool();
            d->m_model->setSelectedDocument(document);
        });

        connect(action, &QAction::toggled, tool, &msc::BaseTool::setActive);
        connect(action, &QAction::toggled, this, [this]() {
            for (QAction *action : d->m_hierarchyToolBar->actions())
                action->setChecked(false);
        });
    };

    addAction(msc::MscDocument::HierarchyAnd);
    addAction(msc::MscDocument::HierarchyOr);
    addAction(msc::MscDocument::HierarchyParallel);
    addAction(msc::MscDocument::HierarchyIs);
    addAction(msc::MscDocument::HierarchyRepeat);
    addAction(msc::MscDocument::HierarchyException);
    addAction(msc::MscDocument::HierarchyLeaf);
}

void MainWindow::initMenuViewWindows()
{
    d->m_menuView->addSeparator();
    d->m_menuViewWindows = d->m_menuView->addMenu("Windows");

    d->m_actToggleErrorView = d->ui->dockWidgetErrors->toggleViewAction();
    d->m_menuViewWindows->addAction(d->m_actToggleErrorView);

    d->m_actToggleHierarchyView = d->ui->dockWidgetDocument->toggleViewAction();
    d->m_menuViewWindows->addAction(d->m_actToggleHierarchyView);

    d->m_actToggleMscTextView = d->ui->dockWidgetMscText->toggleViewAction();
    d->m_menuViewWindows->addAction(d->m_actToggleMscTextView);

    d->m_actToggleAsn1View = d->ui->dockWidgetAsn1->toggleViewAction();
    d->m_menuViewWindows->addAction(d->m_actToggleAsn1View);
}

void MainWindow::initMenuHelp()
{
    d->m_menuHelp = menuBar()->addMenu(tr("Help"));
    d->m_actAboutQt = d->m_menuHelp->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);
}

void MainWindow::onCreateMessageToolRequested()
{
    d->m_messageCreateTool->activate();
}

bool MainWindow::startRemoteControl(quint16 port)
{
    if (!d->m_remoteControlWebServer) {
        d->m_remoteControlWebServer = new RemoteControlWebServer(this);
        d->m_remoteControlHandler = new RemoteControlHandler(this);
        d->m_remoteControlHandler->setModel(d->m_model);

        connect(d->m_remoteControlWebServer, &RemoteControlWebServer::executeCommand, d->m_remoteControlHandler,
                &RemoteControlHandler::handleRemoteCommand);
        connect(d->m_remoteControlHandler, &RemoteControlHandler::commandDone, d->m_remoteControlWebServer,
                &RemoteControlWebServer::commandDone);
    }
    if (d->m_remoteControlWebServer->start(port))
        return true;

    d->m_remoteControlWebServer->deleteLater();
    d->m_remoteControlWebServer = nullptr;
    d->m_remoteControlHandler->deleteLater();
    d->m_remoteControlHandler = nullptr;
    qWarning() << "Continue app running without remote control enabled";
    return false;
}

void MainWindow::initTools()
{
    auto pointerTool = new msc::PointerTool(nullptr, this);
    d->m_tools.append(pointerTool);

    d->m_instanceCreatorTool = new msc::InstanceCreatorTool(&(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(d->m_instanceCreatorTool);

    d->m_instanceStopTool = new msc::InstanceStopTool(nullptr, this);
    d->m_tools.append(d->m_instanceStopTool);

    d->m_messageCreateTool = new msc::MessageCreatorTool(msc::MscMessage::MessageType::Message,
                                                         &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(d->m_messageCreateTool);

    auto messageCreateTool = new msc::MessageCreatorTool(msc::MscMessage::MessageType::Create,
                                                         &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(messageCreateTool);

    auto commentCreateTool = new msc::CommentCreatorTool(false, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(commentCreateTool);

    auto globalCommentCreateTool = new msc::CommentCreatorTool(true, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(globalCommentCreateTool);

    auto coregionCreateTool = new msc::CoregionCreatorTool(&(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(coregionCreateTool);

    auto actionCreateTool = new msc::ActionCreatorTool(&(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(actionCreateTool);

    auto conditionCreateTool = new msc::ConditionCreatorTool(false, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(conditionCreateTool);

    auto sharedConditionCreateTool =
            new msc::ConditionCreatorTool(true, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(sharedConditionCreateTool);

    auto startTimerCreateTool =
            new msc::TimerCreatorTool(msc::MscTimer::TimerType::Start, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(startTimerCreateTool);

    auto stopTimerCreateTool =
            new msc::TimerCreatorTool(msc::MscTimer::TimerType::Stop, &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(stopTimerCreateTool);

    auto timeoutCreateTool = new msc::TimerCreatorTool(msc::MscTimer::TimerType::Timeout,
                                                       &(d->m_model->chartViewModel()), nullptr, this);
    d->m_tools.append(timeoutCreateTool);

    QActionGroup *toolsActions = new QActionGroup(this);
    toolsActions->setExclusive(false);
    for (msc::BaseTool *tool : d->m_tools) {
        QAction *toolAction = d->m_mscToolBar->addAction(tool->title());
        toolAction->setCheckable(true);
        toolAction->setIcon(tool->icon());
        toolAction->setToolTip(tr("%1: %2").arg(tool->title(), tool->description()));
        toolAction->setData(QVariant::fromValue<msc::BaseTool::ToolType>(tool->toolType()));
        tool->setView(currentView());
        if (tool == d->m_instanceStopTool)
            d->m_instanceStopTool->setAction(toolAction);
        else if (tool == d->m_instanceCreatorTool)
            d->m_instanceCreatorTool->setAction(toolAction);

        connect(this, &MainWindow::currentGraphicsViewChanged, tool, &msc::BaseTool::setView);
        connect(tool, &msc::BaseTool::activeChanged, toolAction, &QAction::setChecked);
        if (msc::BaseCreatorTool *creatorTool = qobject_cast<msc::BaseCreatorTool *>(tool))
            connect(creatorTool, &msc::BaseCreatorTool::created, this, &MainWindow::activateDefaultTool);

        toolsActions->addAction(toolAction);
        connect(toolAction, &QAction::toggled, tool, &msc::BaseTool::setActive);
        connect(toolAction, &QAction::toggled, this, &MainWindow::updateMscToolbarActionsChecked);
    }

    d->m_defaultToolAction = toolsActions->actions().first();
    d->m_defaultToolAction->setVisible(false);
    activateDefaultTool();
}

void MainWindow::initMainToolbar()
{
    d->ui->mainToolBar->addAction(d->m_actNewFile);
    d->ui->mainToolBar->addAction(d->m_actOpenFile);
    d->ui->mainToolBar->addAction(d->m_actSaveFile);

    d->ui->mainToolBar->addSeparator();
    d->ui->mainToolBar->addAction(d->m_actUndo);
    d->ui->mainToolBar->addAction(d->m_actRedo);

    d->ui->mainToolBar->addSeparator();
    d->ui->mainToolBar->addAction(d->m_deleteTool->action());

    d->ui->mainToolBar->addSeparator();
    d->ui->mainToolBar->addAction(d->m_actCopy);
    d->ui->mainToolBar->addAction(d->m_actPaste);
}

void MainWindow::initConnections()
{
    connect(d->ui->documentTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            &MainWindow::showSelection);
    connect(d->ui->documentTreeView, &QTreeView::doubleClicked, this, &MainWindow::showChart);

    connect(&(d->m_model->chartViewModel()), &msc::ChartViewModel::currentChartChanged, this,
            &MainWindow::selectCurrentChart);

    connect(d->m_model, &MainModel::showChartVew, this, [this]() { showDocumentView(true); });

    connect(d->m_model, &MainModel::selectedDocumentChanged, d->ui->documentTreeView,
            &DocumentTreeView::setSelectedDocument);

    connect(d->ui->graphicsView, &msc::GraphicsView::mouseMoved, this, &MainWindow::showCoordinatesInfo);

    connect(d->ui->graphicsView, &msc::GraphicsView::createMessageToolRequested, this,
            &MainWindow::onCreateMessageToolRequested);

    connect(d->m_model, &MainModel::modelDataChanged, this, &MainWindow::updateModel);
    connect(d->m_model, &MainModel::modelUpdated, this, &MainWindow::updateModel);
    connect(d->m_model, &MainModel::modelUpdated, d->ui->asn1Widget, &ASN1FileView::setModel);

    connect(d->m_actToggleMscTextView, &QAction::toggled, this, [this](bool on) {
        if (on) {
            QMetaObject::invokeMethod(d->ui->mscTextBrowser, "updateView", Qt::QueuedConnection);
        }
    });

    connect(d->m_model->documentItemModel(), &msc::DocumentItemModel::dataChanged, this, &MainWindow::showSelection);

    connect(d->m_model->undoStack(), &QUndoStack::indexChanged, this, &MainWindow::updateTitles);
    connect(d->m_model, &MainModel::lasteSaveUndoChange, this, &MainWindow::updateTitles);

    connect(d->m_model, &MainModel::currentFilePathChanged, this, [&](const QString &filename) {
        QFileInfo fileInfo(filename);
        d->ui->asn1Widget->setCurrentDirectory(fileInfo.absolutePath());
    });
}

bool MainWindow::processCommandLineArg(CommandLineParser::Positional arg, const QString &value)
{
    switch (arg) {
    case CommandLineParser::Positional::OpenFileMsc: {
        return openFileMsc(value);
    }
    case CommandLineParser::Positional::DbgOpenMscExamplesChain: {
        return openMscChain(value);
    }
    case CommandLineParser::Positional::StartRemoteControl: {
        if (startRemoteControl(value.toUShort())) {
            menuBar()->setVisible(false);

            d->m_mscToolBar->setVisible(false);
            d->m_hierarchyToolBar->setVisible(false);
            d->ui->mainToolBar->setVisible(false);

            d->ui->dockWidgetErrors->hide();
            d->ui->dockWidgetMscText->hide();
            d->ui->dockWidgetDocument->hide();
            d->ui->dockWidgetErrorsContents->hide();
            d->ui->dockWidgetDocumenetContents->hide();
            d->ui->dockWidgetAsn1->hide();

            statusBar()->hide();
            return true;
        }
    } break;
    case CommandLineParser::Positional::DropUnsavedChangesSilently: {
        d->m_dropUnsavedChangesSilently = true;
        break;
    }
    default:
        qWarning() << Q_FUNC_INFO << "Unhandled option:" << arg << value;
        break;
    }
    return false;
}

msc::BaseTool *MainWindow::activeTool() const
{
    for (auto tool : d->m_tools)
        if (tool->isActive())
            return tool;

    return nullptr;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    QMainWindow::keyPressEvent(e);

    switch (e->key()) {
    case Qt::Key_Escape: {
        if (!e->isAutoRepeat()) {
            if (QAction *pointerToolAction = d->m_mscToolBar->actions().first())
                if (!pointerToolAction->isChecked())
                    pointerToolAction->setChecked(true);
        }
        break;
    }
#ifdef QT_DEBUG
    case Qt::Key_R: {
        if (!e->isAutoRepeat() && (e->modifiers() == Qt::ControlModifier)) {
            openFileMsc(d->m_model->currentFilePath());
        }
        break;
    }
    case Qt::Key_M: {
        if (!e->isAutoRepeat() && e->modifiers().testFlag(Qt::AltModifier)
            && e->modifiers().testFlag(Qt::ControlModifier) && e->modifiers().testFlag(Qt::ShiftModifier)) {
            showMousePositioner();
        }
        break;
    }
    default: {
        if (msc::BaseTool *tool = activeTool())
            tool->processKeyPress(e);
        break;
    }
#endif
    }
}

void MainWindow::loadSettings()
{
    // as default hide the msc text view
    d->ui->dockWidgetMscText->setVisible(false);

    restoreGeometry(AppOptions::MainWindow.Geometry->read().toByteArray());
    restoreState(AppOptions::MainWindow.State->read().toByteArray());
    d->m_model->setCurrentFilePath(AppOptions::MainWindow.LastFilePath->read().toString());

    // the toolbar might be hidden from a streaming tool session
    d->ui->mainToolBar->show();

    const bool isDocViewMode = 0 == AppOptions::MainWindow.DocOrHierarchyViewMode->read().toInt();
    if (isDocViewMode)
        showDocumentView(true);
    else
        showHierarchyView(true);
    QAction *changeViewModeAction = isDocViewMode ? d->m_actShowDocument : d->m_actShowHierarchy;
    changeViewModeAction->setChecked(true);
}

void MainWindow::saveSettings()
{
    AppOptions::MainWindow.Geometry->write(saveGeometry());
    AppOptions::MainWindow.State->write(saveState());
    AppOptions::MainWindow.LastFilePath->write(d->m_model->currentFilePath());
    AppOptions::MainWindow.DocOrHierarchyViewMode->write(d->ui->centerView->currentIndex());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (saveDocument()) {
        saveSettings();
        QMainWindow::closeEvent(e);
    } else {
        e->ignore();
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);

    if (e->type() == QEvent::ActivationChange) {

        // In order to automatically create on startup a default doc with a ChartItem
        // box sized accordingly to this->centralWidget(), we have to wait while
        // processing of restoreState(...) & restoreGeometry(...) will be finished.
        // The final geometry (which is stored in a QByteArray and not in a QRect)
        // is applyed after QEvent::Show, so QEvent::Resize handler seems to be
        // a proper entry point, but actually it's not:
        // in case the saved window state is "normal", the resizeEvent(...) will
        // be called once, but for "maximized" there would be one more call, and
        // there is no easy way to detect without additional routines is it necessary
        // to wait for the second call or not.
        // That's why it's performed here and in such way.
        // (But now I suspect writing those additional routines would take about
        // same time as writing this comment did)

        static bool isFirstCall(true);
        if (isFirstCall) {
            isFirstCall = false;
            onGeometryRestored();
        }
    }
}

void MainWindow::onGeometryRestored()
{
    // A new document should be created only if no files were opened by command line args
    if (d->m_model->currentFilePath().isEmpty()) {
        QMetaObject::invokeMethod(this, "createNewDocument", Qt::QueuedConnection);
    }
}

QSizeF MainWindow::prepareChartBoxSize() const
{
    static constexpr qreal padding = 100.;
    if (centralWidget()) {
        return centralWidget()->geometry().size() - QSizeF(padding, padding);
    }
    return QSizeF();
}

void MainWindow::updateMscToolbarActionsEnablement()
{
    auto chart = d->m_model->chartViewModel().currentChart();
    const bool hasInstance = chart && !chart->instances().isEmpty();

    bool forceDefault(false);
    for (QAction *act : d->m_mscToolBar->actions()) {
        const msc::BaseTool::ToolType toolType(act->data().value<msc::BaseTool::ToolType>());
        switch (toolType) {
        case msc::BaseTool::ToolType::ActionCreator:
        case msc::BaseTool::ToolType::ConditionCreator:
        case msc::BaseTool::ToolType::MessageCreator:
        case msc::BaseTool::ToolType::CommentCreator:
        case msc::BaseTool::ToolType::CoregionCreator:
        case msc::BaseTool::ToolType::EntityDeleter:
        case msc::BaseTool::ToolType::InstanceStopper:
        case msc::BaseTool::ToolType::TimerCreator: {
            const bool changed = act->isEnabled() && !hasInstance;
            forceDefault = forceDefault || changed;
            act->setEnabled(hasInstance);
            break;
        }
        case msc::BaseTool::ToolType::Pointer:
        case msc::BaseTool::ToolType::InstanceCreator:
        default: {
            act->setEnabled(true);
            break;
        }
        }
    }

    if (forceDefault)
        activateDefaultTool();
}

void MainWindow::updateMscToolbarActionsChecked()
{
    if (QAction *senderAction = qobject_cast<QAction *>(sender()))
        if (senderAction->isChecked()) {
            for (QAction *action : d->m_mscToolBar->actions())
                if (action != senderAction)
                    action->setChecked(false);
        }
}

QStringList MainWindow::mscFileFilters()
{
    static QStringList filters;
    if (filters.isEmpty()) {
        const QString asterisk("*%1");
        filters << asterisk.arg(DotMscFileExtensionLow) << asterisk.arg(DotMscFileExtensionLow).toUpper();
    }
    return filters;
}

bool MainWindow::saveDocument()
{
    if (!d->m_dropUnsavedChangesSilently && d->m_model->needSave()) {
        auto result = QMessageBox::warning(
                this, windowTitle(), tr("You have unsaved data. Do you want to save the MSC document?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        if (result == QMessageBox::Cancel) {
            return false;
        } else if (result == QMessageBox::Yes) {
            saveMsc();
        }
    }

    return true;
}

void MainWindow::updateModel()
{
    d->ui->mscTextBrowser->setModel(d->m_model->mscModel());
    updateMscToolbarActionsEnablement();
}

QPlainTextEdit *MainWindow::textOutputPane() const
{
    return d->ui->errorTextEdit;
}

#ifdef QT_DEBUG
#include <QInputDialog>

// A way to precisiousely move mouse pointer to scene coordinates without pixel hunting.
// Invoked by CTRL+ALT+SHIFT+M
void MainWindow::showMousePositioner()
{
    const bool isItemMove = currentView()->scene()->selectedItems().size() == 1;
    bool gotText(false);
    QString input = QInputDialog::getText(this, isItemMove ? "Move selected item to" : "Move mouse to",
                                          "x y:", QLineEdit::Normal, "0 0", &gotText)
                            .trimmed();
    if (!gotText || input.size() < 3) // minimal valid input is "0 0"
        return;

    bool isCif = input.at(0).toUpper() == 'C';
    if (isCif)
        input.remove(0, 1);

    static const QRegularExpression rxPoint("(-?\\d+\\.?\\d*),?\\s?(-?\\d+\\.?\\d*)");
    QRegularExpressionMatch m = rxPoint.match(input);
    const QStringList &coords = m.capturedTexts();
    if (coords.size() == 3) {
        bool xOk(false), yOk(false);
        const QPointF &inputPos = { coords.at(1).toDouble(&xOk), coords.at(2).toDouble(&yOk) };
        if (xOk && yOk) {
            QPointF scenePos(inputPos);
            if (isCif)
                scenePos = msc::utils::CoordinatesConverter::cifToScene(scenePos.toPoint());

            const QPoint &localPos = currentView()->mapFromScene(scenePos);
            const QPoint &globalPos = currentView()->mapToGlobal(localPos);

            if (isItemMove) {
                if (QGraphicsItem *item = currentView()->scene()->selectedItems().first())
                    item->setPos(scenePos);
            } else {
                QCursor::setPos(globalPos);

                // Update the status bar info:
                QMouseEvent event(QEvent::MouseMove, localPos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
                QApplication::sendEvent(currentView()->viewport(), &event);
            }
        }
    }
}
#endif

void MainWindow::showCoordinatesInfo(const QString &info)
{
    statusBar()->showMessage(info);
}

void MainWindow::openMessageDeclarationEditor()
{
    msc::MscModel *model = d->m_model->mscModel();
    if (!model)
        return;

    QVector<msc::MscDocument *> docs = model->documents();
    if (docs.isEmpty()) {
        return;
    }

    MessageDeclarationsDialog dialog(docs.at(0)->messageDeclarations(), model->asn1TypesData(), this);
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        msc::cmd::CommandsStack::current()->beginMacro("Edit message declarations");
        const QVariantList cmdParams = { QVariant::fromValue<msc::MscDocument *>(docs.at(0)),
                                         QVariant::fromValue<msc::MscMessageDeclarationList *>(dialog.declarations()) };
        msc::cmd::CommandsStack::push(msc::cmd::Id::SetMessageDeclarations, cmdParams);
        const QVariantList params { QVariant::fromValue(d->m_model->mscModel()), dialog.fileName(), "ASN.1" };
        msc::cmd::CommandsStack::push(msc::cmd::Id::SetAsn1File, params);
        d->m_model->mscModel()->setAsn1TypesData(dialog.asn1Types());
        msc::cmd::CommandsStack::current()->endMacro();
    }
}

void MainWindow::updateZoomBox(double percent)
{
    const QSignalBlocker blocker(d->m_zoomBox);

    d->m_zoomBox->setCurrentIndex(d->m_zoomBox->findData(percent));
}
