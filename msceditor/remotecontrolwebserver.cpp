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

#include <QMetaEnum>

#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#include "remotecontrolwebserver.h"

static const int kPort = 34622;

static inline QByteArray generateResponse(bool result, const QString &errorString = QString()) {
    QJsonObject obj;
    obj.insert(QLatin1String("result"), result);
    if (!errorString.isEmpty())
        obj.insert(QLatin1String("errorString"), errorString);
    return QJsonDocument(obj).toJson();
}

/*!
   \class RemoteControlWebServer
   \brief Handles remote control commands and arguments compounded in json packet using websocket. JSON structure:
    \quotation
        {
            "CommandType": "command",
            "Parameters": {
                             "parameter1": "parameter1Value",
                             "parameter2": "parameter2Value",
                             "parameter3": "parameter3Value",
                             ...
                          }
        }
    \endquotation
    Implemented commands and parameters list:
    \list
    \li \c Instance - creating new Instance
        \list
        \li \c name - Instance's Name, command fails if CHart already has Instance with this name, optional
        \li \c exStop - Explicit stop {True, False=default}, optional
        \endlist
    \li \c Message - creating new Message, command fails if neither \c srcName nor \c dstName parameter set.
        \list
        \li \c name - Message's Name, optional
        \li \c srcName - source Instance's Name, command fails if Chart doesn't have Instance with this name
        \li \c dstName - target Instance's Name, command fails if Chart doesn't have Instance with this name
        \li \c MessageType - type of \c Message, optional
            \list
            \li \c Message - default
            \li \c Create
            \endlist
        \endlist
    \li \c Timer - creating new Timer
        \list
        \li \c name - Timer's Name, optional
        \li \c instanceName - linked Instance's Name, command fails if Chart doesn't have Instance with this name, mandatory
        \li \c TimerType - type of \c Timer, optional
            \list
            \li \c Start
            \li \c Stop
            \li \c Timeout
            \li \c Unknown - default
            \endlist
        \endlist
    \li \c Action - creating new Action
        \list
        \li \c name - Action's Name, optional
        \li \c instanceName - linked Instance's Name, command fails if Chart doesn't have Instance with this name, mandatory
        \endlist
    \li \c Condition - creating new Condition
        \list
        \li \c name - Condition's Name, optional
        \li \c instanceName - linked Instance's Name, command fails if Chart doesn't have Instance with this name, mandatory
        \endlist
    \endlist
    After command processing returns JSON packet:
    \quotation
        {
            "result": True,
            "errorString": "Short error description"
        }
    \endquotation
   \inmodule MscEditor
 */

RemoteControlWebServer::RemoteControlWebServer(QObject *parent)
    : QObject(parent)
    , m_webSocketServer(new QWebSocketServer(QLatin1String("Remote Control"),
                                             QWebSocketServer::NonSecureMode, this))
{
    if (m_webSocketServer->listen(QHostAddress::Any, kPort)) {
        qDebug() << "Echoserver listening on port" << kPort;
        connect(m_webSocketServer, &QWebSocketServer::newConnection,
                this, &RemoteControlWebServer::onNewConnection);
    }
}

RemoteControlWebServer::~RemoteControlWebServer()
{
    m_webSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void RemoteControlWebServer::onNewConnection()
{
    QWebSocket *pSocket = m_webSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &RemoteControlWebServer::processTextMessage);
    connect(pSocket, &QWebSocket::connected, this, &RemoteControlWebServer::socketConnected);
    connect(pSocket, &QWebSocket::disconnected, this, &RemoteControlWebServer::socketDisconnected);
    connect(pSocket, static_cast<void(QWebSocket::*)(QAbstractSocket::SocketError)>(&QWebSocket::error),
            this, &RemoteControlWebServer::error);

    m_clients << pSocket;
}

void RemoteControlWebServer::processTextMessage(const QString &message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Message received:" << message << pClient;

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (QJsonParseError::NoError != error.error) {
        qWarning() << "Json document parsing error:" << error.error << error.errorString();
        if (pClient) {
            qDebug() << "Informing remote about error:"
                    << pClient->sendTextMessage(generateResponse(false, error.errorString()));
        }
        return;
    }
    const QJsonObject obj = doc.object();
    const QMetaEnum qtEnum = QMetaEnum::fromType<RemoteControlWebServer::CommandType>();
    const QString commandTypeStr = obj.value(qtEnum.name()).toString();
    const int commandTypeInt = qtEnum.keyToValue(commandTypeStr.toLocal8Bit().constData());
    Q_EMIT executeCommand(static_cast<RemoteControlWebServer::CommandType>(commandTypeInt),
                          obj.value(QLatin1String("Parameters")).toObject().toVariantMap(),
                          pClient ? pClient->peerName() : QString());
}

void RemoteControlWebServer::socketConnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Socket Connected:" << pClient;
}

void RemoteControlWebServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Socket disconnected:" << pClient;
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void RemoteControlWebServer::error(QAbstractSocket::SocketError error)
{
    const QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qWarning() << "Socket error:" << error << (pClient ? pClient->errorString() : QString());
}

void RemoteControlWebServer::commandDone(RemoteControlWebServer::CommandType commandType, bool result,
                                         const QString &peerName, const QString &errorString)
{
    const QMetaEnum qtEnum = QMetaEnum::fromType<RemoteControlWebServer::CommandType>();
    const int commandTypeInt = static_cast<int>(commandType);
    const QString commandTypeStr = QLatin1String(qtEnum.valueToKey(commandTypeInt));

    auto it = std::find_if(m_clients.constBegin(), m_clients.constEnd(), [peerName](const QWebSocket *socket){
        return socket->peerName() == peerName;
    });
    if (it == m_clients.constEnd())
        return;

    (*it)->sendTextMessage(generateResponse(result, errorString));
}
