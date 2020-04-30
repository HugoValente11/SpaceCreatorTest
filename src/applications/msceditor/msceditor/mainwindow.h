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

#include "commandlineparser.h"
#include "remotecontrolwebserver.h"

#include <QMainWindow>
#include <QModelIndex>
#include <memory>

class QUndoStack;

namespace Ui {
class MainWindow;
}

namespace msc {
class MscDocument;
class BaseTool;
}

class MainModel;
class QUndoGroup;
class QGraphicsView;
class QPlainTextEdit;

struct MainWindowPrivate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QGraphicsView *currentView() const;

    bool processCommandLineArg(CommandLineParser::Positional arg, const QString &value);
    QPlainTextEdit *textOutputPane() const;

public Q_SLOTS:
    void createNewDocument();
    void selectAndOpenFile();
    void selectCurrentChart();

Q_SIGNALS:
    void currentGraphicsViewChanged(QGraphicsView *view);

private Q_SLOTS:
    void showChart(const QModelIndex &index);
    void showSelection(const QModelIndex &current, const QModelIndex &previous);
    bool openFileMsc(const QString &file);
    void saveMsc();
    void saveAsMsc();
    void showDocumentView(bool show);
    void showHierarchyView(bool show);
    bool openMscChain(const QString &dirPath);
    void activateDefaultTool();
    void onCreateMessageToolRequested();
    bool startRemoteControl(quint16 port);
    void updateTitles();
    void updateModel();
    void showCoordinatesInfo(const QString &info);
    void openMessageDeclarationEditor();
    void updateZoomBox(double percent);
    void saveScreenshot();

private:
    static const QLatin1String DotMscFileExtensionLow;
    std::unique_ptr<MainWindowPrivate> const d;

    void loadSettings();
    void saveSettings();

    void setupUi();
    void initActions();
    void initMenus();
    void initMenuFile();
    void initMenuEdit();
    void initMenuView();
    void initDocumentViewActions();
    void initMenuViewWindows();
    void initMenuHelp();
    void initTools();
    void initMainToolbar();
    void initConnections();

    void closeEvent(QCloseEvent *e) override;

    void keyPressEvent(QKeyEvent *e) override;

    void clearUndoStacks();

    void changeEvent(QEvent *event) override;
    void onGeometryRestored();
    QSizeF prepareChartBoxSize() const;

    void updateMscToolbarActionsEnablement();
    void updateMscToolbarActionsChecked();

    static QStringList mscFileFilters();

    bool saveDocument();

#ifdef QT_DEBUG
    void showMousePositioner();
#endif

    msc::BaseTool *activeTool() const;

    void saveSceneRender(const QString &filePath) const;
};