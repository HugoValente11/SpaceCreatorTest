﻿/*
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

#include "mainwindow.h"

#include "aadlobjectfunctiontype.h"
#include "asn1dialog.h"
#include "baseitems/common/aadlutils.h"
#include "commandsstack.h"
#include "common.h"
#include "context/action/actionsmanager.h"
#include "endtoendview.h"
#include "interface/commands/commandsfactory.h"
#include "interface/interfacedocument.h"
#include "iveditorcore.h"
#include "minimap.h"
#include "reports/bugreportdialog.h"
#include "settings/appoptions.h"
#include "ui_mainwindow.h"
#include "xmldocexporter.h"
#include "zoomcontroller.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsView>
#include <QImageWriter>
#include <QMessageBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTreeView>
#include <QUndoCommand>
#include <QUndoGroup>
#include <QWindow>

namespace aadlinterface {

/*!
\class aadlinterface::MainWindow
\brief Main application window - the place to store and manage supported document types, import/export data,
process command line arguments and user actions.

\sa aadlinterface::InterfaceDocument, shared::CommandLineParser
*/

MainWindow::MainWindow(aadlinterface::IVEditorCore *core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_zoomCtrl(new ZoomController())
    , m_core(core)
{
    ui->setupUi(this);

    setCentralWidget(m_core->document()->view());

    statusBar()->addPermanentWidget(m_zoomCtrl);
    m_core->addToolBars(this);

    // Connect the actions
    connect(m_core->actionNewFile(), &QAction::triggered, this, &MainWindow::onCreateFileRequested);
    connect(m_core->actionOpenFile(), &QAction::triggered, this, &MainWindow::onOpenFileRequested);
    connect(m_core->actionSaveFile(), &QAction::triggered, this, [=]() { exportXml(); });
    connect(m_core->actionSaveFileAs(), &QAction::triggered, this, [=]() { exportXmlAs(); });
    connect(m_core->actionQuit(), &QAction::triggered, this, &MainWindow::onQuitRequested);
    connect(m_core->actionImport(), &QAction::triggered, this, &MainWindow::onImportRequested);
    connect(m_core->actionExportFunctions(), &QAction::triggered, this, &MainWindow::onExportFunctionsRequested);
    connect(m_core->actionExportType(), &QAction::triggered, this, &MainWindow::onExportTypeRequested);

    // Register the actions to the action manager
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionNewFile(), "Create file", "Create new empty file");
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionOpenFile(), "Open file", "Show Open File dialog");
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionQuit(), "Quit", "Quite the application");
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionUndo(), "Undo", "Undo the last operation");
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionRedo(), "Redo", "Redo the last undone operation");
    ActionsManager::registerAction(Q_FUNC_INFO, m_core->actionImport(), "Import", "Import all available AADL files");
    ActionsManager::registerAction(
            Q_FUNC_INFO, m_core->actionExportFunctions(), "Export Functions", "Export selected objects");
    ActionsManager::registerAction(
            Q_FUNC_INFO, m_core->actionExportType(), "Export Type", "Export selected component type");

    connect(m_core->document(), &InterfaceDocument::dirtyChanged, this, &MainWindow::onDocDirtyChanged);

    m_core->initMenus(this);

    m_core->document()->fillToolBar(m_core->docToolBar());
    if (auto view = m_core->chartView()) {
        m_zoomCtrl->setView(view);
        connect(view, &aadlinterface::GraphicsView::mouseMoved, this, &MainWindow::onGraphicsViewInfo,
                Qt::UniqueConnection);
    }

    aadlinterface::cmd::CommandsStack::setCurrent(m_core->document()->commandsStack());

    updateActions();

    initSettings();

    updateWindowTitle();
    connect(m_core->document(), &InterfaceDocument::titleChanged, this, &MainWindow::updateWindowTitle);

    m_core->setupMiniMap();

    connect(m_core->document(), &InterfaceDocument::asn1ParameterErrorDetected, this, &MainWindow::showAsn1Errors);

    // Create the E2E view and add the action
    auto endToEndView = new EndToEndView(m_core->document(), this);
    endToEndView->hide();
    connect(core->actionToggleE2EView(), &QAction::toggled, endToEndView, &QWidget::setVisible);
    connect(endToEndView, &EndToEndView::visibleChanged, core->actionToggleE2EView(), &QAction::setChecked);
    endToEndView->setVisible(core->actionToggleE2EView()->isChecked());
}

/*!
 * \brief D-tor clears the stuff generated by QDesigner.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * \brief perform check for unsaved changes.
 * \a e - the event to be bypassed to the basic implementation.
 */
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (prepareQuit())
        QMainWindow::closeEvent(e);
    else
        e->ignore();
}

/*!
 * \brief Handler for File->open action.
 */
void MainWindow::onOpenFileRequested()
{
    const QString prevPath(m_core->document()->path());
    const QString &fileName = QFileDialog::getOpenFileName(
            this, tr("Open file"), prevPath, m_core->document()->supportedFileExtensions());
    if (!fileName.isEmpty() && closeFile()) {
        m_core->document()->load(fileName);
    }
}

/*!
 * \brief Handler for File->create action.
 */
void MainWindow::onCreateFileRequested()
{
    if (closeFile()) {
        m_core->document()->create();
    }
}

/*!
 * \brief Return the list of image formats which the Qt is available to write.
 */
static QStringList supportedImgFileExtensions()
{
    QStringList extensions;
    const QList<QByteArray> &formats = QImageWriter::supportedImageFormats();
    for (const QByteArray &format : formats)
        extensions << ("*." + format.toLower());
    return extensions;
}

/*!
 * \brief Handler for render scene request.
 */
void MainWindow::onSaveRenderRequested()
{
    const QString defaultFileName = QString("%1.png").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy_HH-mm-ss"));
    const QStringList &extensions = supportedImgFileExtensions();
    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save screenshot..."), defaultFileName, extensions.join(";; "));

    if (!fileName.isEmpty()) {
        QFileInfo selectedFile(fileName);
        const QString usedExtension = "*." + selectedFile.suffix().toLower();
        if (!extensions.contains(usedExtension))
            fileName.append(".png");

        saveSceneRender(fileName);
    }
}

void MainWindow::onImportRequested()
{
    m_core->document()->loadAvailableComponents();
}

void MainWindow::onExportFunctionsRequested()
{
    m_core->document()->exportSelectedFunctions();
}

void MainWindow::onExportTypeRequested()
{
    m_core->document()->exportSelectedType();
}

/*!
 * \brief Exports the document document to the output file \a savePath using the template file \a templatePath.
 * Returns \c true on success.
 */
bool MainWindow::exportXml(const QString &savePath, const QString &templatePath)
{
    return XmlDocExporter::exportDocSilently(m_core->document(), savePath, templatePath);
}

/*!
 * \brief Shows the preview dialog for exporting the document to the output file \a savePath
 * using the template file \a templatePath.
 * If a \a savePath or \a templatePath is empty, user will be asked to select the file via QFileDialog.
 * Returns true if the preview dialog can not be shown.
 */
bool MainWindow::exportXmlAs(const QString &savePath, const QString &templatePath)
{
    return XmlDocExporter::exportDocInteractive(m_core->document(), this, savePath, templatePath);
}

void MainWindow::onQuitRequested()
{
    if (prepareQuit())
        qApp->quit();
}

void MainWindow::onReportRequested()
{
    QList<QPixmap> images;
    if (m_core->document() != nullptr) {
        if (QGraphicsScene *scene = m_core->document()->scene()) {
            const QSize sceneSize = scene->sceneRect().size().toSize();
            if (!sceneSize.isNull()) {
                QPixmap pix(sceneSize);
                pix.fill(Qt::transparent);

                QPainter p;
                p.begin(&pix);
                p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
                scene->render(&p);
                p.end();

                images.append(pix);
            }
        }
    }
    auto dialog = new reports::BugreportDialog(images, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

void MainWindow::initSettings()
{
    restoreGeometry(aadlinterface::AppOptions::MainWindow.Geometry.read().toByteArray());
    restoreState(aadlinterface::AppOptions::MainWindow.State.read().toByteArray());

    const bool showMinimap = false; // TODO: use a storable option
    m_core->actionToggleMinimap()->setChecked(showMinimap);
}

void MainWindow::updateActions()
{
    bool renderAvailable(false);
    if (QGraphicsScene *scene = m_core->document()->scene()) {
        renderAvailable = !scene->sceneRect().isEmpty() && !scene->items().isEmpty();
    }

    m_core->actionSaveFile()->setEnabled(m_core->document()->isDirty());
    m_core->actionSaveSceneRender()->setEnabled(renderAvailable);
}

/*!
 * \brief Handles startup options from command line
 * \a arg - the option to be processed,
 * \a value - its value.
 * Returns \c true if the option \a arg \a value is known and handled.
 */
bool MainWindow::processCommandLineArg(shared::CommandLineParser::Positional arg, const QString &value)
{
    switch (arg) {
    case shared::CommandLineParser::Positional::DropUnsavedChangesSilently: {
        m_dropUnsavedChangesSilently = true;
        return true;
    }
    case shared::CommandLineParser::Positional::OpenAADLXMLFile: {
        if (!value.isEmpty() && m_core->document() != nullptr) {
            return m_core->document()->load(value);
        };

        return false;
    }
    case shared::CommandLineParser::Positional::OpenStringTemplateFile:
        if (!value.isEmpty())
            return exportXmlAs(QString(), value);
        return false;
    case shared::CommandLineParser::Positional::ExportToFile:
        if (!value.isEmpty())
            return exportXml(value);
        return false;
    case shared::CommandLineParser::Positional::ListScriptableActions: {
        ActionsManager::listRegisteredActions();
        QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
        return true;
    }
    default:
        break;
    }
    qWarning() << Q_FUNC_INFO << "Unhandled option:" << arg << value;
    return false;
}

/*!
 * \brief Renders the whole scene and saves it to the \a filePath
 */
void MainWindow::saveSceneRender(const QString &filePath) const
{
    if (filePath.isEmpty())
        return;

    if (QGraphicsScene *scene = m_core->document()->scene()) {
        QImage img(scene->sceneRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        scene->render(&p);
        img.save(filePath);
    }
}

/*!
 * \brief Updates the title of a tab related to the updated doc.
 * (Adds or removes the "*" sign when doc is changed/saved).
 */
void MainWindow::onDocDirtyChanged(bool dirty)
{
    setWindowModified(dirty);
    updateActions();
}

/*!
 * \brief Shows the \a info message from GraphicsView in the status bar.
 */
void MainWindow::onGraphicsViewInfo(const QString &info)
{
    statusBar()->showMessage(info);
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(QString("Interface View Editor [%1][*]").arg(m_core->document()->title()));
}

/*!
   Show the dialog to set the ASN1 file
 */
void MainWindow::openAsn1Dialog()
{
    Asn1Dialog dialog;
    QFileInfo fi(m_core->document()->asn1FilePath());
    dialog.setFile(fi);
    dialog.show();
    int result = dialog.exec();
    if (result == QDialog::Accepted) {
        if (m_core->document()->asn1FileName() != dialog.fileName()) {
            QVariantList params { QVariant::fromValue(m_core->document()), QVariant::fromValue(dialog.fileName()) };
            QUndoCommand *command = cmd::CommandsFactory::create(cmd::ChangeAsn1File, params);
            if (command) {
                cmd::CommandsStack::push(command);
            }
        }
    }
}

void MainWindow::showAsn1Errors(const QStringList &faultyInterfaces)
{
    QMessageBox::warning(
            this, tr("ASN1 error"), tr("Following interfaces have ASN.1 errors:") + "\n" + faultyInterfaces.join("\n"));
}

/*!
 * Performs check for unsaved changes, asking the user to save, if necessary.
 * Returns true if the file is closed.
 */
bool MainWindow::closeFile()
{
    if (m_core->document()->isDirty() && !m_dropUnsavedChangesSilently) {
        const QMessageBox::StandardButtons btns(QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        auto btn = QMessageBox::question(this, tr("Document closing"),
                tr("There are unsaved changes.\nWould you like to save the document?"), btns);
        if (btn == QMessageBox::Save) {
            if (!exportXml()) {
                return false;
            }
        } else if (btn == QMessageBox::Cancel) {
            return false;
        }
    }

    m_core->document()->close();

    return true;
}

/*!
 * \brief Closes documents, attempting to save changes, if any.
 * If user decline request to save changes, returns \c false.
 */
bool MainWindow::prepareQuit()
{
    if (!closeFile()) {
        return false;
    }

    aadlinterface::AppOptions::MainWindow.State.write(saveState());
    aadlinterface::AppOptions::MainWindow.Geometry.write(saveGeometry());

    return true;
}

}
