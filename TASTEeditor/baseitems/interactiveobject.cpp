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
  along with this program. If not, see
  <https://www.gnu.org/licenses/lgpl-2.1.html>.
*/

#include "interactiveobject.h"

#include "baseitems/common/utils.h"
#include "common/highlightrectitem.h"
#include "grippointshandler.h"

#include <QBrush>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <functional>

namespace taste3 {

static const QMarginsF kMargins { 25, 25, 25, 25 };

InteractiveObject::InteractiveObject(QObject *entity, QGraphicsItem *parent)
    : ClickNotifierItem(entity, parent)
    , m_selectedPen(Qt::black, 2, Qt::DotLine)
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemSendsScenePositionChanges
             | QGraphicsItem::ItemIsSelectable);

    setCursor(Qt::ArrowCursor);
}

QObject *InteractiveObject::modelEntity() const
{
    return dataObject();
}

void InteractiveObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (isSelected()) {
        painter->save();
        painter->setPen(m_selectedPen);

        painter->drawRect(m_boundingRect);
        painter->restore();
    }
}

QRectF InteractiveObject::boundingRect() const
{
    return m_boundingRect;
}

void InteractiveObject::gripPointPressed(GripPoint::Location gp, const QPointF &at)
{
    if (m_gripPoints) {
        if (GripPoint *gripPnt = m_gripPoints->gripPoint(gp)) {
            if (gripPnt->isMover())
                onManualMoveStart(gripPnt->location(), at);
            else
                onManualResizeStart(gripPnt->location(), at);
        }
    }
}

void InteractiveObject::gripPointMoved(GripPoint::Location gripPos, const QPointF &from, const QPointF &to)
{
    if (m_gripPoints) {
        if (GripPoint *gripPnt = m_gripPoints->gripPoint(gripPos)) {
            if (gripPnt->isMover())
                onManualMoveProgress(gripPnt->location(), from, to);
            else
                onManualResizeProgress(gripPnt->location(), from, to);
        }
    }
}

void InteractiveObject::gripPointReleased(GripPoint::Location gp, const QPointF &pressedAt, const QPointF &releasedAt)
{
    if (m_gripPoints) {
        if (GripPoint *gripPnt = m_gripPoints->gripPoint(gp)) {
            if (gripPnt->isMover())
                onManualMoveFinish(gripPnt->location(), pressedAt, releasedAt);
            else
                onManualResizeFinish(gripPnt->location(), pressedAt, releasedAt);
        }
    }
}

void InteractiveObject::onSelectionChanged(bool isSelected)
{
    setZValue(isSelected ? 1 : 0);
    if (isSelected) {
        showGripPoints();
        updateGripPoints();
    } else {
        hideGripPoints();
    }
}

void InteractiveObject::createCommand() {}

bool InteractiveObject::handlePositionChanged(const QPointF &from, const QPointF &to)
{
    const QPointF delta { to - from };
    if (delta.isNull())
        return false;

    const QList<QGraphicsItem *> collidedItems = scene()->items(sceneBoundingRect().marginsAdded(kMargins));
    auto it = std::find_if(collidedItems.constBegin(), collidedItems.constEnd(), [this](const QGraphicsItem *item) {
        return dynamic_cast<const InteractiveObject *>(item) && item != this && item->parentItem() == parentItem();
    });
    // Fallback to previous geometry in case colliding with items at the same level
    if (it != collidedItems.constEnd()) {
        setPos(pos() - delta);
        rebuildLayout();
        updateGripPoints();
        return false;
    }

    return true;
}

bool InteractiveObject::handleGeometryChanged(GripPoint::Location grip, const QPointF &from, const QPointF &to)
{
    const QPointF delta { to - from };
    if (delta.isNull())
        return false;

    const QList<QGraphicsItem *> collidedItems = scene()->items(sceneBoundingRect().marginsAdded(kMargins));
    auto it = std::find_if(collidedItems.constBegin(), collidedItems.constEnd(), [this](const QGraphicsItem *item) {
        return dynamic_cast<const InteractiveObject *>(item) && item != this && item->parentItem() == parentItem();
    });
    // Fallback to previous geometry in case colliding with items at the same level
    if (it != collidedItems.constEnd()) {
        QRectF rect = mapRectToParent(boundingRect());
        switch (grip) {
        case GripPoint::Left: {
            rect.setLeft(rect.left() - delta.x());
        } break;
        case GripPoint::Top: {
            rect.setTop(rect.top() - delta.y());
        } break;
        case GripPoint::Right: {
            rect.setRight(rect.right() - delta.x());
        } break;
        case GripPoint::Bottom: {
            rect.setBottom(rect.bottom() - delta.y());
        } break;
        case GripPoint::TopLeft: {
            rect.setTopLeft(rect.topLeft() - delta);
        } break;
        case GripPoint::TopRight: {
            rect.setTopRight(rect.topRight() - delta);
        } break;
        case GripPoint::BottomLeft: {
            rect.setBottomLeft(rect.bottomLeft() - delta);
        } break;
        case GripPoint::BottomRight: {
            rect.setBottomRight(rect.bottomRight() - delta);
        } break;
        default:
            qWarning() << "Update grip point handling";
            break;
        }
        setRect(parentItem() ? parentItem()->mapRectToScene(rect) : rect);
        updateGripPoints();
        return false;
    }

    return true;
}

void InteractiveObject::rebuildLayout()
{
    for (auto item : childItems()) {
        if (auto iObj = dynamic_cast<InteractiveObject *>(item))
            iObj->instantLayoutUpdate();
    }
}

QFont InteractiveObject::font() const
{
    return m_font;
}

void InteractiveObject::setFont(const QFont &font)
{
    m_font = font;
}

QSizeF InteractiveObject::minimalSize() const
{
    return QSizeF();
}

QBrush InteractiveObject::brush() const
{
    return m_brush;
}

void InteractiveObject::setBrush(const QBrush &brush)
{
    m_brush = brush;
}

QPen InteractiveObject::pen() const
{
    return m_pen;
}

void InteractiveObject::setPen(const QPen &pen)
{
    m_pen = pen;
}

void InteractiveObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = true;
    m_storedZ = zValue();
    //    setZValue(m_storedZ + 1.);

    ClickNotifierItem::hoverEnterEvent(event);
}

void InteractiveObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hovered = false;
    setZValue(m_storedZ);

    ClickNotifierItem::hoverLeaveEvent(event);
}

void InteractiveObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    onManualMoveProgress(GripPoint::Center, event->lastScenePos(), event->scenePos());
    ClickNotifierItem::mouseMoveEvent(event);
}

void InteractiveObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    onManualMoveStart(GripPoint::Center, event->scenePos());
    ClickNotifierItem::mousePressEvent(event);
}

void InteractiveObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    onManualMoveFinish(GripPoint::Center, event->buttonDownScenePos(event->button()), event->scenePos());
    ClickNotifierItem::mouseReleaseEvent(event);
}

void InteractiveObject::onManualMoveStart(GripPoint::Location grip, const QPointF &at)
{
    if (grip == GripPoint::Center)
        m_clickPos = mapFromScene(at);
}

void InteractiveObject::onManualMoveProgress(GripPoint::Location grip, const QPointF &from, const QPointF &to)
{
    Q_UNUSED(grip);
    Q_UNUSED(from);
    Q_UNUSED(to);
}

void InteractiveObject::onManualMoveFinish(GripPoint::Location grip, const QPointF &pressedAt,
                                           const QPointF &releasedAt)
{
    Q_UNUSED(grip);
    Q_UNUSED(pressedAt);
    Q_UNUSED(releasedAt);

    m_clickPos = QPointF();
}

void InteractiveObject::onManualResizeStart(GripPoint::Location grip, const QPointF &at)
{
    Q_UNUSED(grip);
    Q_UNUSED(at);
}

void InteractiveObject::onManualResizeFinish(GripPoint::Location grip, const QPointF &pressedAt,
                                             const QPointF &releasedAt)
{
    Q_UNUSED(grip);
    Q_UNUSED(pressedAt);
    Q_UNUSED(releasedAt);
}

void InteractiveObject::onManualResizeProgress(GripPoint::Location grip, const QPointF &from, const QPointF &to)
{
    Q_UNUSED(grip);
    Q_UNUSED(from);
    Q_UNUSED(to);
}

void InteractiveObject::hideGripPoints()
{
    if (m_gripPoints)
        m_gripPoints->hideAnimated();
}

void InteractiveObject::showGripPoints()
{
    initGripPoints();
    m_gripPoints->showAnimated();
}

void InteractiveObject::initGripPoints()
{
    if (m_gripPoints)
        return;

    m_gripPoints = new GripPointsHandler(this);
    m_gripPoints->setZValue(0);

    connect(m_gripPoints, &GripPointsHandler::manualGeometryChangeStart, this, &InteractiveObject::gripPointPressed);
    connect(m_gripPoints, &GripPointsHandler::manualGeometryChangeProgress, this, &InteractiveObject::gripPointMoved);
    connect(m_gripPoints, &GripPointsHandler::manualGeometryChangeFinish, this, &InteractiveObject::gripPointReleased);

    connect(m_gripPoints, &GripPointsHandler::visibleChanged, this, [this]() {
        if (m_gripPoints && !m_gripPoints->isVisible())
            delete m_gripPoints; // it's not a thing directly added to the scene, so just delete is enough
    });
    if (GripPoint *gp = m_gripPoints->gripPoint(GripPoint::Location::Center))
        gp->setGripType(GripPoint::GripType::Mover);
}

void InteractiveObject::updateGripPoints()
{
    if (m_gripPoints)
        m_gripPoints->updateLayout();
}

QVariant InteractiveObject::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        onSelectionChanged(value.toBool());
        break;
    case QGraphicsItem::ItemPositionChange:
        m_prevPos = pos();
        break;
    case QGraphicsItem::ItemPositionHasChanged:
        Q_EMIT relocated(m_prevPos, pos());
        break;
    default:
        break;
    }

    return ClickNotifierItem::itemChange(change, value);
}

bool InteractiveObject::isHovered() const
{
    if (!isUnderMouse())
        return false;

    return m_gripPoints && m_gripPoints->isVisible();
}

HighlightRectItem *InteractiveObject::createHighlighter()
{
    if (m_highlighter)
        delete m_highlighter;
    m_highlighter = new HighlightRectItem(this);
    connect(m_highlighter, &HighlightRectItem::highlighted, m_highlighter, &QObject::deleteLater);
    connect(m_highlighter, &QObject::destroyed, this, [this]() { m_highlighter = nullptr; });

    m_highlighter->setRect(m_boundingRect);

    return m_highlighter;
}

bool InteractiveObject::isHighlightable() const
{
    return m_highlightable;
}

void InteractiveObject::setHighlightable(bool highlightable)
{
    m_highlightable = highlightable;
    clearHighlight();
}

void InteractiveObject::highlightConnected()
{
    doHighlighting(Qt::green, false);
}

void InteractiveObject::highlightDisconnected()
{
    doHighlighting(Qt::red, false);
}

void InteractiveObject::doHighlighting(const QColor &color, bool permanent)
{
    if (!m_highlightable) {
        return;
    }

    if (HighlightRectItem *highlighter = createHighlighter()) {
        QColor targetColor(color);
        QPen p(targetColor);
        p.setWidthF(1.);
        highlighter->setPen(p);
        targetColor.setAlphaF(0.25);
        highlighter->setBrush(targetColor);

        if (!permanent)
            highlighter->highlight();
    }
}

void InteractiveObject::clearHighlight()
{
    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }
}

bool InteractiveObject::isHighlighting() const
{
    return m_highlighter != nullptr;
}

QPointF InteractiveObject::centerInScene() const
{
    return sceneBoundingRect().center();
}

void InteractiveObject::scheduleLayoutUpdate()
{
    if (m_layoutDirty)
        return;

    m_layoutDirty = true;
    QMetaObject::invokeMethod(this, "instantLayoutUpdate", Qt::QueuedConnection);
}

void InteractiveObject::instantLayoutUpdate()
{
    const QRectF oldBounds = boundingRect();

    rebuildLayout();
    m_layoutDirty = false;

    if (oldBounds != boundingRect())
        Q_EMIT boundingBoxChanged();

    update();
}

void InteractiveObject::setRect(const QRectF &geometry)
{
    const QPointF geometryPos = parentItem() ? parentItem()->mapFromScene(geometry.topLeft()) : geometry.topLeft();
    setPos(geometryPos);
    prepareGeometryChange();
    m_boundingRect = { QPointF(0, 0), geometry.size() };
    instantLayoutUpdate();
    updateGripPoints();
}

} // namespace taste3
