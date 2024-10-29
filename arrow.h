#ifndef ARROW_H
#define ARROW_H
#include <QLine>
#include <QColor>
#include <QPainter>

struct Arrow {
    QLine line;
    int width;
    QColor colour;
    int way;
    Arrow(QPoint p1, QPoint p2, int w, QColor c,int way) : line(p1, p2), width(w), colour(c), way(way) {}
};

// 箭头绘制
void drawArrowHead(QPainter& painter, QPoint p1, QPoint p2) {
    // 箭头的长度和角度
    const double arrowSize = 10.0; // 箭头大小
    double angle = atan2(p2.y() - p1.y(), p2.x() - p1.x());

    // 箭头的两个边
    QPointF arrowP1 = p2 - QPointF(arrowSize * cos(angle - M_PI / 6), arrowSize * sin(angle - M_PI / 6));
    QPointF arrowP2 = p2 - QPointF(arrowSize * cos(angle + M_PI / 6), arrowSize * sin(angle + M_PI / 6));

    // 绘制箭头
    painter.drawLine(p2, arrowP1);
    painter.drawLine(p2, arrowP2);
    painter.drawLine(arrowP1, arrowP2);
}

#endif // ARROW_H
