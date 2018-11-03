/*
   Copyright (C) 2018 European Space Agency - <maxime.perrotin@esa.int>

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

#include <QMouseEvent>
#include <QGraphicsItem>

namespace msc {

/*!
  \class msc::GraphicsView
  \brief Basic view for an MSC diagram

  \inmodule MscWidgets

  Some documentation
*/

/*!
  Constructs a MSV view object with the parent \a parent.
*/
GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
}

void GraphicsView::setZoom(double percent)
{
    resetTransform();
    scale(percent / 100.0, percent / 100.0);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint &screenPos(event->pos());
    const QPointF &scenePos(mapToScene(screenPos));

    QPointF itemPos(-1, -1);
    if (QGraphicsItem *item = itemAt(screenPos))
        itemPos = item->mapFromScene(scenePos);

    Q_EMIT mouseMoved(screenPos, scenePos, itemPos);

    QGraphicsView::mouseMoveEvent(event);
}

} // namespace msc
