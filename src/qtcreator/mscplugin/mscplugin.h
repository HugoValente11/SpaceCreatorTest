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

#include "mscplugin_global.h"

#include <QSharedPointer>
#include <extensionsystem/iplugin.h>

class QAction;

namespace aadlinterface {
class IVEditorCore;
}

namespace MscPlugin {

class AadlModelStorage;
class MscEditorFactory;
class MscModelStorage;

class MSCPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "MscPlugin.json")

public:
    MSCPlugin();
    ~MSCPlugin();

    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;

public Q_SLOTS:
    void showMessageDeclarations();
    void checkInstances();
    void setMinimapVisible(bool visible);

private:
    ExtensionSystem::IPlugin *aadlPlugin() const;
    QSharedPointer<aadlinterface::IVEditorCore> ivPlugin() const;

    MscEditorFactory *m_factory = nullptr;
    QAction *m_messageDeclarationAction = nullptr;
    AadlModelStorage *m_aadlStorage = nullptr;
    MscModelStorage *m_mscStorage = nullptr;
};

}
