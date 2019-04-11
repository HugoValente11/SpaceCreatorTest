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

#include "baseitems/common/objectanchor.h"
#include "baseitems/interactiveobject.h"
#include "instanceitem.h"

#include <QPointer>
#include <mscmessage.h>

namespace msc {
class LabeledArrowItem;

class MessageItem : public InteractiveObject
{
    Q_OBJECT
public:
    struct GeometryNotificationBlocker {
        GeometryNotificationBlocker(MessageItem *target);
        ~GeometryNotificationBlocker();

    private:
        const QPointer<MessageItem> m_target;
        const bool m_storedPositionChangeIgnored;

        GeometryNotificationBlocker() = delete;
        GeometryNotificationBlocker(const GeometryNotificationBlocker &other) = delete;
        GeometryNotificationBlocker &operator=(const GeometryNotificationBlocker &) = delete;
    };

    enum class CifUpdatePolicy
    {
        DontChange,
        ForceCreate,
        UpdateIfExists
    };

    explicit MessageItem(MscMessage *message, InstanceItem *source = nullptr, InstanceItem *target = nullptr,
                         QGraphicsItem *parent = nullptr);

    MscMessage *modelItem() const;

    void setInstances(InstanceItem *sourceInstance, InstanceItem *targetInstance);
    bool setSourceInstanceItem(InstanceItem *sourceInstance);
    bool setTargetInstanceItem(InstanceItem *targetInstance);

    QString name() const;

    QPainterPath shape() const override;
    QRectF boundingRect() const override;
    QPair<QPointF, bool> commentPoint() const override;

    void postCreatePolishing() override;

    QPointF head() const;
    void setHead(const QPointF &head, ObjectAnchor::Snap snap);
    QPointF tail() const;
    void setTail(const QPointF &tail, ObjectAnchor::Snap snap);

    bool isAutoResizable() const;
    void setAutoResizable(bool resizable);

    static MessageItem *createDefaultItem(MscMessage *message, const QPointF &pos);

    void performSnap();

    void onSourceInstanceMoved(const QPointF &from, const QPointF &to);
    void onTargetInstanceMoved(const QPointF &from, const QPointF &to);
    bool isCreator() const;

    void addMessagePoint(const QPointF &scenePoint);
    QVector<QPointF> messagePoints() const;
    void setMessagePoints(const QVector<QPointF> &scenePoints, MessageItem::CifUpdatePolicy cifUpdate);

    void applyCif() override;

    QString displayedText() const;

Q_SIGNALS:
    void retargeted(MessageItem *, const QPointF &pos, msc::MscMessage::EndType endType);

public Q_SLOTS:
    void setName(const QString &name);
    void setPositionChangeIgnored(bool ignored);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    void onMoveRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;
    void onResizeRequested(GripPoint *gp, const QPointF &from, const QPointF &to) override;
    void updateGripPoints() override;

    void prepareHoverMark() override;
    cif::CifLine::CifType mainCifType() const override;
    void updateCif() override;
    void moveSilentlyBy(const QPointF &shift) override;

private Q_SLOTS:
    void onTextChanged();
    void showMessageDialog();
    void rebuildLayout() override;
    void commitGeometryChange();
    void onRenamed(const QString &title);
    void onManualGeometryChangeFinished(GripPoint::Location pos, const QPointF &from, const QPointF &to);
    void updateDisplayText();
    void onChartBoxChanged();

private:
    QPointer<msc::MscMessage> m_message = nullptr;
    LabeledArrowItem *m_arrowItem = nullptr;
    QPointer<InstanceItem> m_sourceInstance = nullptr;
    QPointer<InstanceItem> m_targetInstance = nullptr;
    bool m_posChangeIgnored = false;
    bool m_autoResize = true;
    bool m_preventRecursion = false;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    QString displayTextFromModel() const;

    bool updateSource(const QPointF &to, ObjectAnchor::Snap snap, InstanceItem *keepInstance = nullptr);
    bool updateTarget(const QPointF &to, ObjectAnchor::Snap snap, InstanceItem *keepInstance = nullptr);
    bool updateSourceAndTarget(const QPointF &shift);
    void updateTooltip();

    bool ignorePositionChange() const;
    bool proceedPositionChange() const;

    InstanceItem *hoveredItem(const QPointF &hoverPoint) const;

    void updateMessagePoints();

    cif::CifBlockShared cifMessage() const;
    cif::CifBlockShared cifPosition() const;

    void extendGlobalMessage();
    bool setMessagePointsNoCif(const QVector<QPointF> &scenePoints);
};

} // namespace mas
