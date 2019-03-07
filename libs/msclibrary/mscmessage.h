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

#include "mscinstanceevent.h"

#include <QObject>
#include <QString>

namespace msc {
class MscInstance;

class MscMessage : public MscInstanceEvent
{
    Q_OBJECT
    Q_PROPERTY(MscInstance *sourceInstance READ sourceInstance WRITE setSourceInstance NOTIFY sourceChanged)
    Q_PROPERTY(MscInstance *targetInstance READ targetInstance WRITE setTargetInstance NOTIFY targetChanged)

public:
    struct Parameters {
        QString name;
        QString expression;
        QString pattern;
    };

    enum class MessageType
    {
        Message = 0,
        Create

        // TODO: add call/reply/method/etc?
    };

    enum class EndType
    {
        SOURCE_TAIL,
        TARGET_HEAD
    };
    Q_ENUM(EndType)

    explicit MscMessage(QObject *parent = nullptr);
    explicit MscMessage(const QString &name, QObject *parent = nullptr);

    MscInstance *sourceInstance() const;
    void setSourceInstance(MscInstance *source);

    MscInstance *targetInstance() const;
    void setTargetInstance(MscInstance *target);

    MscEntity::EntityType entityType() const override;

    const Parameters &parameters() const;
    void setParameters(const Parameters &parameters);

    MscMessage::MessageType messageType() const;

    bool relatesTo(const MscInstance *instance) const override;

    struct Flow {
        MscInstance *from = nullptr;
        MscInstance *to = nullptr;

        bool isComplete() const { return from && to; }
        bool operator==(const Flow &other) const { return from == other.from && to == other.to; }

        struct SourceLineInfo {
            size_t m_line = 0;
            size_t m_pos = 0;
        } m_sourceLineInfo;
    };

    Flow m_descrIn, m_descrOut;

    bool isGlobal() const;
    bool isOrphan() const;

Q_SIGNALS:
    void sourceChanged(MscInstance *source);
    void targetChanged(MscInstance *target);

protected:
    MscMessage::MessageType m_msgType = MscMessage::MessageType::Message;

private:
    MscInstance *m_source = nullptr;
    MscInstance *m_target = nullptr;
    Parameters m_parameters;
};
}
