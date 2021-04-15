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

#include "graphicsview.h"

#include "ivobject.h"
#include "baseitems/common/ivutils.h"
#include "baseitems/interactiveobject.h"
#include "interface/ivrectgraphicsitem.h"

#include <QGraphicsItem>
#include <QMimeData>
#include <QMouseEvent>
#include <QtDebug>

namespace ive {

GraphicsView::GraphicsView(QWidget *parent)
    : shared::ui::GraphicsViewBase(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    setAcceptDrops(true);
}

QList<QPair<QPointF, QString>> GraphicsView::mouseMoveCoordinates(
        QGraphicsScene *, const QPoint &screenPos, const QPointF &scenePos) const
{
    QList<QPair<QPointF, QString>> coords;
    QList<QGraphicsItem *> itemsUnderCursor = items(screenPos);
    for (QGraphicsItem *item : itemsUnderCursor) {
        if (auto iObj = qobject_cast<ive::InteractiveObject *>(item->toGraphicsObject())) {
            coords.push_back({ item->mapFromScene(scenePos),
                    iObj->entity() ? iObj->entity()->objectName() : QLatin1String("None") });
        }
    }
    return coords;
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    bool keyHandled(false);
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down: {
        for (QGraphicsItem *item : scene()->selectedItems()) {
            if (ive::IVRectGraphicsItem *rectItem =
                            // Not the qgraphicsitem_cast due its "successful" cast of IVFunctionName/Text Graphics
                            // Items
                    qobject_cast<ive::IVRectGraphicsItem *>(item->toGraphicsObject())) {
                rectItem->singleStepMove(ive::IVRectGraphicsItem::MoveStep(event->key()));
                keyHandled = true;
            }
        }
        break;
    }
    default:
        break;
    }

    if (keyHandled)
        event->accept();
    else
        GraphicsViewBase::keyPressEvent(event);
}

void GraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void GraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void GraphicsView::dropEvent(QDropEvent *event)
{
    auto mimeData = event->mimeData();
    const shared::Id id = QUuid::fromString(mimeData->text());
    if (event->dropAction() == Qt::DropAction::CopyAction) {
        Q_EMIT importEntity(id, mapToScene(event->pos()));
        event->accept();
    } else if (event->dropAction() == Qt::DropAction::LinkAction) {
        Q_EMIT instantiateEntity(id, mapToScene(event->pos()));
        event->accept();
    } else {
        event->ignore();
    }
}

}
