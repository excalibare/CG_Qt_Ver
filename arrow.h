#ifndef ARROW_H
#define ARROW_H
#include <QLine>
#include <QColor>
#include <QPainter>
#include <vector>
#include <QFile>
#include <QDataStream>
#include <QColor>
#include <QDebug>
#include <QPoint>

struct Arrow {
    QLine line;
    int width;
    QColor color;
    int way;
    Arrow(QPoint p1, QPoint p2, int w, QColor c,int way) : line(p1, p2), width(w), color(c), way(way) {}
    Arrow(): line(), width(), color(), way(){}
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

// 保存 arrow 到文本文件
void saveArrowsToTextFile(const QVector<Arrow>& arrows, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行写入:" << filename;
        return;
    }

    QTextStream out(&file);
    out << arrows.size() << "\n"; // 写入箭头数量

    for (const auto& arrow : arrows) {
        out << arrow.line.p1().x() << " "
            << arrow.line.p1().y() << " "
            << arrow.line.p2().x() << " "
            << arrow.line.p2().y() << " "
            << arrow.width << " "
            << arrow.way << " ";
        out << arrow.color.red() << " "
            << arrow.color.green() << " "
            << arrow.color.blue() << " "
            << arrow.color.alpha() << "\n"; // 写入颜色
    }



    file.close();
}

// 从文本文件加载 arrow
QVector<Arrow> loadArrowsFromTextFile(const QString& filename) {
    QVector<Arrow> arrows;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行读取:" << filename;
        return arrows; // 返回空 QVector
    }

    QTextStream in(&file);
    int numArrows;
    in >> numArrows; // 读取多边形数量

    for (int i = 0; i < numArrows; ++i) {
        Arrow arrow;

        int p1_x, p1_y, p2_x, p2_y, wi, wa;
        int r, g, b, a;
        in >> p1_x >> p1_y >> p2_x >> p2_y; // 读取line
        in >> wi >> wa; // 读取width&way
        in >> r >> g >> b >> a;

        arrow.line = QLine(QPoint(p1_x,p1_y),QPoint(p2_x,p2_y));
        arrow.width = wi;
        arrow.way = wa;
        arrow.color = QColor(r, g, b, a);

        arrows.append(arrow);
    }

    file.close();
    return arrows;
}

#endif // ARROW_H
