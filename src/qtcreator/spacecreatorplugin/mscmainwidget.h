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

#pragma once

#include <QPointer>
#include <QSharedPointer>
#include <QStackedWidget>
#include <QWidget>
#include <memory>

class QAction;
class QGraphicsView;
class QSplitter;
class QUndoStack;
class QVBoxLayout;

namespace asn1 {
class ASN1FileView;
}

namespace msc {
class DocumentTreeView;
class GraphicsView;
class MSCEditorCore;
}

namespace shared {
class ActionsBar;
}

namespace spctr {
class MscModelStorage;

class MscMainWidget : public QWidget
{
    Q_OBJECT
public:
    MscMainWidget(MscModelStorage *mscStorage, QWidget *parent = nullptr);
    ~MscMainWidget();

    bool load(const QString &filename);
    bool save();
    QString errorMessage() const;

    void setFileName(const QString &filename);

    bool isDirty() const;
    QUndoStack *undoStack();

    QString textContents() const;

    QAction *actionCopy() const;
    QAction *actionPaste() const;
    QAction *actionToolDelete() const;

    QSharedPointer<msc::MSCEditorCore> mscCore() const;

Q_SIGNALS:
    void dirtyChanged(bool dirty);
    void asn1Selected(const QString &fileName);
    void mscDataLoaded(const QString &fileName, QSharedPointer<msc::MSCEditorCore> data);

private Q_SLOTS:
    void showChart(const QModelIndex &index);
    void showSelection(const QModelIndex &current, const QModelIndex &previous);
    void showAsn1Errors(const QStringList &faultyMessages);
    void onViewModeChanged();

private:
    void init();
    void initConnections();

    shared::ActionsBar *m_chartToolBar = nullptr;
    shared::ActionsBar *m_documentToolBar = nullptr;
    QWidget *m_leftArea = nullptr;
    QSplitter *m_horizontalSplitter = nullptr;
    QVBoxLayout *m_leftVerticalLayout = nullptr;

    msc::DocumentTreeView *m_documentTree = nullptr;
    asn1::ASN1FileView *m_asn1Widget = nullptr;

    QSharedPointer<msc::MSCEditorCore> m_plugin;
    QPointer<MscModelStorage> m_mscStorage;
};

}
