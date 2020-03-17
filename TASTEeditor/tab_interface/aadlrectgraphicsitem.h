/*
 Copyright (C) 2018-2020 European Space Agency - <maxime.perrotin@esa.int>

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

#include "baseitems/interactiveobject.h"

#include <QSet>
namespace taste3 {
namespace aadl {

class AADLRectGraphicsItem : public InteractiveObject
{
    Q_OBJECT
public:
    explicit AADLRectGraphicsItem(AADLObject *entity, QGraphicsItem *parentItem = nullptr);
    virtual QSizeF minimalSize() const;

    void setRect(const QRectF &geometry);

    void updateFromEntity() override;
    QList<QVariantList> prepareChangeCoordinatesCommandParams() const override;

    enum MoveStep
    {
        Left = Qt::Key_Left,
        Right = Qt::Key_Right,
        Up = Qt::Key_Up,
        Down = Qt::Key_Down
    };
    void singleStepMove(MoveStep direction);

private Q_SLOTS:
    void onGeometryChanged();

protected:
    void rebuildLayout() override;
    void initGripPoints() override;
    void onManualMoveProgress(GripPoint *grip, const QPointF &from, const QPointF &to) override;
    void onManualResizeProgress(GripPoint *grip, const QPointF &from, const QPointF &to) override;
    void onManualResizeFinish(GripPoint *grip, const QPointF &pressedAt, const QPointF &releasedAt) override;
    void onManualMoveFinish(GripPoint *grip, const QPointF &pressedAt, const QPointF &releasedAt) override;

    QRectF adjustRectToParent(GripPoint *grip, const QPointF &from, const QPointF &to);
    virtual bool allowGeometryChange(const QPointF &from, const QPointF &to);
    void shiftBy(const QPointF &shift);

private:
    QSet<InteractiveObject *> m_collidedItems;
};

} // namespace aadl
} // namespace taste3
