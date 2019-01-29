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

#pragma once
#include <QGraphicsObject>

#include <memory>

class QGraphicsRectItem;

namespace msc {

class MscChart;
class TextItem;

class ChartItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit ChartItem(MscChart *chartEntity, QGraphicsItem *parent = nullptr);
    ~ChartItem() override;

    QRectF boundingRect() const override;

    // Get the size of the written text with this font
    QSizeF textSize(const QFont &font) const;

    QString chartName() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QRectF box() const;
    void setBox(const QRectF &r);

public Q_SLOTS:
    void setName(const QString &name);

private:
    QGraphicsRectItem *m_rectItem = nullptr;
    TextItem *m_textItem = nullptr;
    MscChart *m_chart = nullptr;
    QRectF m_box;

    void updateBox();
};

} // ns msc
