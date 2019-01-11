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

#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

class QUndoStack;
namespace msc {

class GraphicsView : public QGraphicsView
{
    Q_OBJECT

    Q_PROPERTY(qreal minZoomPercent READ minZoomPercent WRITE setMinZoomPercent)
    Q_PROPERTY(qreal maxZoomPercent READ maxZoomPercent WRITE setMaxZoomPercent)
    Q_PROPERTY(qreal zoomStepPercent READ zoomStepPercent WRITE setZoomStepPercent)

public:
    explicit GraphicsView(QWidget *parent = nullptr);

    void setZoom(double percent);

    QUndoStack *undoStack() const;

Q_SIGNALS:
    void mouseMoved(const QPoint &screenPos, const QPointF &scenePos, const QPointF &itemPos) const;
    void zoomChanged(qreal percent);

public Q_SLOTS:
    qreal minZoomPercent() const;
    void setMinZoomPercent(qreal percent);

    qreal maxZoomPercent() const;
    void setMaxZoomPercent(qreal percent);

    qreal zoomStepPercent() const;
    void setZoomStepPercent(qreal percent);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    QUndoStack *m_undoStack = nullptr;

private:
    qreal m_zoomPercent = 100.0;

    qreal m_minZoomPercent = 50.0;
    qreal m_zoomStepPercent = 25.0;
    qreal m_maxZoomPercent = 400.0;
};

} // namespace msc

#endif // GRAPHICSVIEW_H
