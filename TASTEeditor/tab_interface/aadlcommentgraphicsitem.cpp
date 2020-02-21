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

#include "aadlcommentgraphicsitem.h"

#include "aadlfunctiongraphicsitem.h"
#include "baseitems/common/utils.h"
#include "baseitems/textgraphicsitem.h"
#include "colors/colormanager.h"
#include "commands/commandids.h"
#include "commands/commandsfactory.h"

#include <QApplication>
#include <QGraphicsScene>
#include <QPainter>
#include <QTextDocument>
#include <QtDebug>
#include <app/commandsstack.h>
#include <baseitems/grippointshandler.h>

static const qreal kBorderWidth = 2;
static const qreal kMargins = 14 + kBorderWidth;

namespace taste3 {
namespace aadl {

AADLCommentGraphicsItem::AADLCommentGraphicsItem(AADLObjectComment *comment, QGraphicsItem *parent)
    : AADLRectGraphicsItem(comment, parent)
    , m_textItem(new TextGraphicsItem(this))
{
    setFlag(QGraphicsItem::ItemIsSelectable);

    m_textItem->setPlainText(comment->title());
    m_textItem->setFramed(false);
    m_textItem->setEditable(true);
    m_textItem->setBackgroundColor(Qt::transparent);
    m_textItem->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_textItem->setTextWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_textItem->setFlag(QGraphicsItem::ItemIsSelectable);

    connect(m_textItem, &TextGraphicsItem::edited, this, &AADLCommentGraphicsItem::textEdited);
    connect(m_textItem, &TextGraphicsItem::textChanged, this, &AADLCommentGraphicsItem::textChanged);

    setFont(QFont(qApp->font()));
    m_textItem->setFont(font());

    setHighlightable(false);
    colorSchemeUpdated();
}

void AADLCommentGraphicsItem::updateFromEntity()
{
    AADLRectGraphicsItem::updateFromEntity();
    setText(entity()->title());
}

void AADLCommentGraphicsItem::textEdited(const QString &text)
{
    if (entity()->title() == m_textItem->toPlainText())
        return;

    taste3::cmd::CommandsStack::current()->beginMacro(tr("Change comment"));

    const QRectF geometry = sceneBoundingRect();
    const QVector<QPointF> points { geometry.topLeft(), geometry.bottomRight() };
    const QVariantList geometryParams { QVariant::fromValue(entity()), QVariant::fromValue(points) };
    const auto geometryCmd = cmd::CommandsFactory::create(cmd::ChangeEntityGeometry, geometryParams);
    taste3::cmd::CommandsStack::current()->push(geometryCmd);

    const QVariantList commentTextParams { QVariant::fromValue(entity()), QVariant::fromValue(text) };
    const auto commentTextCmd = cmd::CommandsFactory::create(cmd::ChangeCommentText, commentTextParams);
    taste3::cmd::CommandsStack::current()->push(commentTextCmd);

    taste3::cmd::CommandsStack::current()->endMacro();
}

void AADLCommentGraphicsItem::textChanged()
{
    prepareGeometryChange();
    m_boundingRect = m_textItem->boundingRect();
    updateGripPoints();
}

void AADLCommentGraphicsItem::setText(const QString &text)
{
    if (m_textItem->toPlainText() == text)
        return;

    m_textItem->setTextWidth(150);
    m_textItem->setPlainText(text);
    m_textItem->setTextWidth(m_textItem->idealWidth());

    instantLayoutUpdate();
}

QString AADLCommentGraphicsItem::text() const
{
    return m_textItem->toPlainText();
}

AADLObjectComment *AADLCommentGraphicsItem::entity() const
{
    return qobject_cast<aadl::AADLObjectComment *>(aadlObject());
}

void AADLCommentGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen());
    painter->setBrush(brush());

    const QRectF br = mapRectFromItem(m_textItem, m_textItem->boundingRect());
    auto preparePolygon = [](const QRectF &rect) {
        return QVector<QPointF> { rect.topRight() - QPointF(kMargins, 0),
                                  rect.topLeft(),
                                  rect.bottomLeft(),
                                  rect.bottomRight(),
                                  rect.topRight() + QPointF(0, kMargins),
                                  rect.topRight() - QPointF(kMargins, 0) };
    };
    painter->drawPolygon(preparePolygon(br));

    auto preparePolyline = [](const QRectF &rect) {
        return QVector<QPointF> { rect.topRight() + QPointF(0, kMargins),
                                  rect.topRight() - QPointF(kMargins, -kMargins),
                                  rect.topRight() - QPointF(kMargins, 0) };
    };
    painter->drawPolyline(preparePolyline(br));

    painter->restore();
    AADLRectGraphicsItem::paint(painter, option, widget);
}

void AADLCommentGraphicsItem::rebuildLayout()
{
    AADLRectGraphicsItem::rebuildLayout();
    m_textItem->setExplicitSize(m_boundingRect.size());
}

QSizeF AADLCommentGraphicsItem::minimalSize() const
{
    return utils::DefaultGraphicsItemSize;
}

ColorManager::HandledColors AADLCommentGraphicsItem::handledColorType() const
{
    return ColorManager::HandledColors::Comment;
}

void AADLCommentGraphicsItem::colorSchemeUpdated()
{
    const ColorHandler &h = colorHandler();
    QPen pen = h.pen();
    pen.setCapStyle(Qt::FlatCap);
    pen.setStyle(Qt::SolidLine);
    setPen(pen);
    setBrush(h.brush());
    update();
}

} // namespace aadl
} // namespace taste3
