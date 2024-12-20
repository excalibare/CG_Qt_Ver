#ifndef SHAPEDRAWER_H
#define SHAPEDRAWER_H

#include <QApplication>
#include <QDebug>
#include <QLineEdit>
#include <QWidget>
#include "arc.h"
#include "arrow.h"
#include "bezier.h"
#include "fill.h"
#include "line.h"
#include "point.h"
#include "polygon.h"
#include "stdafx.h"
#include "tools.h"
#include "transmatrix.h"
#include <cmath>
#include <vector>

class ShapeDrawer : public QWidget
{
public:
    int frontsize = 10; // 字体大小
    // Q_OBJECT;
private:
    // 全局
    DrawMode mode; // 当前绘制模式：直线 或 圆弧
    vector<vector<pointData>> MAP;
    int lineWidth = 5;          // 存储线条宽度
    bool hasStartPoint = false; // 是否有起点
    bool drawing = false;       // 是否正在绘制（用来控制鼠标释放时的动作）
    QColor currentLineColor = Qt::black; // 当前线条颜色
    QVector<int>
        shape; // 控制图形的重绘顺序，防止顺序错乱 1.直线 2.圆弧或圆 3.多边形 4.种子点 5.Bezier 6.Bspline 114.实线箭头 514.虚线箭头
    float XL = 0, XR = 800, YB = 0, YT = 550;
    Point _begin = Point(0, 0); // 拖拽的参考坐标，方便计算位移
    bool ctr_or_not = false;    // Bezier与Bspline是否移动控制点
    QLabel* coordLabel;         // 鼠标按下时显示实时位置的组件
    string filename;            // 储存文件名称

    // 直线段
    QPoint startPoint;                   // 线段的起点
    QPoint endPoint;                     // 线段的终点
    line_Algorithm line_algo = Midpoint; // 当前选择的直线段算法
    QVector<Line> lines;                 // 存储已绘制的直线段
    int way = 0;
    QVector<Arrow> solid_arrows;
    QVector<Arrow> dashed_arrows;

    // 圆、圆弧
    QPoint center;             // 圆心（对于圆弧）
    int radius;                // 半径（对于圆弧）
    int startAngle, endAngle;  // 圆弧的起始和终止角度
    int counter = 0;           // 计数（点击鼠标次数，0-2循环）
    QVector<Arc> arcs;         // 存储已绘制的直线段
    QVector<int> CircleShapes; // 储存DIY圆形的数据

    // 多边形
    QVector<Polygon> polygons; // 存储多个多边形
    Polygon currentPolygon;    // 当前正在绘制的多边形
    vector<Fill> fills;        // 存储多个填充区域
    QStack<Point> stack;
    Polygon* nowPolygon;                 // 当前正在绘制的多边形
    QVector<PolygonShape> PolygonShapes; // 储存DIY多边形的数据

    // 裁剪
    QPoint clipStartPoint;                     // 裁剪窗口的起点
    QPoint clipEndPoint;                       // 裁剪窗口的终点
    clip_Algorithm clip_algo = SutherlandTrim; // 当前选择的直线段裁剪算法
    transMode trans_algo = MOVE;               // 当前选择的变形模式
    QVector<QPoint> _cropPolygon;              // 裁切多边形

    // 填充
    Fill nowFill; // 当前鼠标指向的填充区域

    // 变形
    bool isInTagRect = false; //是否在标志矩形内
    bool isInPolygon = false;
    bool isInEllipse = false;
    bool isArrow = false;
    bool isInRect = false;
    bool isInFill = false;

    bool iscomfirm = false;
    bool isSpecificRefer = false; // 是否有指定的变换点
    Polygon tempTransPoly;
    Point referancePoint;
    QRect* transRectTag = new QRect(100, 100, 20, 20); //标签矩形

    // Bezier曲线
    QVector<QVector<QPoint>> all_beziers;       // 所有的Bezier曲线
    QVector<QPoint> currentBezierControlPoints; // 暂存的当前Bezier曲线控制点
    int isOnPoint1 = -1;                        // 用于跟踪当前选中的控制点
    int SelectedBezier = -1, SelectedPoint = -1;

    // Bspline曲线
    QVector<QVector<QPoint>> all_bsplines;       // 所有的Bspline曲线
    QVector<QPoint> currentBsplineControlPoints; // 当前Bspline曲线的控制点容器
    int last_k = 3;                              // 当前Bspline曲线的阶数
    vector<int> k_steps;                         // 每条Bspline曲线的阶数
    int isOnPoint2 = -1;                         // 用于跟踪当前选中的控制点
    int SelectedBspline = -1;

protected:
    // 重写绘制事件
    void paintEvent(QPaintEvent* event) override
    {
        // 各类图形的计数器，控制从vector中取出的顺序
        int i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, i6 = 0, i114 = 0, i514 = 0;
        clearMAP(MAP);
        QPainter painter(this);

        // 每次刷新的时候重新绘制已存在直线
        for (int c = 0; c < shape.size(); c++) {
            QPen pen;
            // 重绘直线
            if (shape.at(c) == 1 && lines.size() > i1) {
                const Line& line = lines.at(i1++);
                pen.setColor(line.colour); // 设置线条颜色(默认 black)
                pen.setWidth(line.width);  // 使用 width 设置线条粗细
                painter.setPen(pen);
                switch (line.alg) {
                case DDA:
                    drawDDALine(painter, line.line.p1(), line.line.p2());
                    break;
                case Bresenham:
                    drawBresenhamLine(painter, line.line.p1(), line.line.p2());
                    break;
                case Midpoint:
                    drawMidpointLine(painter, line.line.p1(), line.line.p2());
                    break;
                case DashLine:
                    drawDashLine(painter, line.line.p1(), line.line.p2());
                    break;
                }
            }
            // 重绘圆弧 或 圆
            else if (shape.at(c) == 2 && arcs.size() > i2) {
                const Arc& arc = arcs.at(i2++);
                pen.setColor(arc.colour); // 设置线条颜色(默认 black)
                pen.setWidth(arc.width);  // 使用 width 设置线条粗细
                painter.setPen(pen);
                if (arc.startAngle <= arc.endAngle) {
                    drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, arc.endAngle);
                } else {
                    drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, 360);
                    drawMidpointArc(painter, arc.center, arc.radius, 0, arc.endAngle);
                }
            }
            // 重绘多边形
            else if (shape.at(c) == 3 && polygons.size() > i3) {
                const Polygon& polygon = polygons.at(i3++);
                QPen pen;
                pen.setColor(polygon.color); // 设置线条颜色(默认 black)
                // pen.setWidth(lineWidth);		 // 使用 width 设置线条粗细
                painter.setPen(pen);
                if (polygon.points.size() > 1) {
                    for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
                        drawDDALine(painter,
                                    QPoint(polygon.points[i].x, polygon.points[i].y),
                                    QPoint(polygon.points[i + 1].x, polygon.points[i + 1].y));
                    }
                }
                if (polygon.isClosed()) {
                    scanlineFill(painter, polygon);
                }
            }
            // 重绘填充区域
            else if (shape.at(c) == 4 && fills.size() > i4) {
                const Fill& fill = fills.at(i4++);
                QPen pen;
                pen.setColor(fill.color); // 设置线条颜色(默认 black)
                painter.setPen(pen);
                qDebug() << "In paintevent : " << fill.color << " " << Fill(fill).point.Getx()
                         << " " << Fill(fill).point.Gety();
                fillShape(painter, fill.point, fill.color);
            }
            // 重绘Bezier曲线
            else if (shape.at(c) == 5 && all_beziers.size() > i5) {
                painter.setPen(Qt::blue);
                const QVector<QPoint>& ControlPoints = all_beziers.at(i5++);
                // 绘制Beizer控制点
                if (ctr_or_not) {
                    for (const QPoint& point : ControlPoints) {
                        painter.drawEllipse(point, 5, 5);
                    }
                }
                // 绘制Bezier曲线
                if (ControlPoints.size() > 1) {
                    QPen bezierPen(currentLineColor, lineWidth);
                    Bezier bezier(1, painter, ControlPoints.toVector(), bezierPen);
                    bezier.drawBezier();
                }
            }
            // 重绘Bspline曲线
            else if (shape.at(c) == 6 && all_bsplines.size() > i6) {
                int temp_k = k_steps[i6];
                painter.setPen(Qt::blue);
                const QVector<QPoint>& ControlPoints = all_bsplines.at(i6++);
                // 绘制Bspline控制点
                if (ctr_or_not) {
                    for (const QPoint& point : ControlPoints) {
                        painter.drawEllipse(point, 5, 5);
                    }
                    for (int j = 0; j < ControlPoints.size() - 1; ++j) {
                        // 设置虚线样式
                        QPen dashedPen(Qt::blue, 1);
                        painter.setPen(dashedPen);
                        drawPreviewDash(painter, ControlPoints.at(j), ControlPoints.at(j + 1));
                    }
                }
                // 绘制Bspline曲线
                if (ControlPoints.size() > 1) {
                    QPen bsplinePen(currentLineColor, lineWidth);
                    Bspline bspline(painter, ControlPoints.toVector(), temp_k, bsplinePen);
                    bspline.drawBspline();
                }
            }
            // 重绘实线箭头
            else if (shape.at(c) == 114 && solid_arrows.size() > i114) {
                const Arrow& arrow = solid_arrows.at(i114++);
                pen.setColor(arrow.color);
                pen.setWidth(arrow.width);
                painter.setPen(pen);
                if (arrow.way == 1)
                    drawSolidArrow(painter, arrow.line.p1(), arrow.line.p2());
                else
                    draw2SolidArrow(painter, arrow.line.p1(), arrow.line.p2());
            }
            // 重绘虚线箭头
            else if (shape.at(c) == 514 && dashed_arrows.size() > i514) {
                const Arrow& arrow = dashed_arrows.at(i514++);
                pen.setColor(arrow.color);
                pen.setWidth(arrow.width);
                painter.setPen(pen);
                if (arrow.way == 1)
                    drawDashedArrow(painter, arrow.line.p1(), arrow.line.p2());
                else
                    draw2DashedArrow(painter, arrow.line.p1(), arrow.line.p2());
            }
        }

        // 绘制当前正在创建的Bezier曲线
        if (mode == BezierMode && currentBezierControlPoints.size() > 1) {
            QPen bezierPen(currentLineColor, lineWidth);
            Bezier bezier(1, painter, currentBezierControlPoints.toVector(), bezierPen);
            bezier.drawBezier();
        }

        // 绘制当前正在创建的Beizer控制点
        if (mode == BezierMode) {
            painter.setPen(Qt::blue);
            for (const QPoint& point : currentBezierControlPoints) {
                painter.drawEllipse(point, 5, 5);
            }
        }

        // 绘制当前正在创建的Bspline曲线
        if (mode == BsplineMode && currentBsplineControlPoints.size() > 1) {
            QPen bsplinePen(currentLineColor, lineWidth);

            Bspline bspline(painter, currentBsplineControlPoints.toVector(), last_k, bsplinePen);
            bspline.drawBspline();
        }

        // 绘制当前正在创建的Bspline控制点
        if (mode == BsplineMode) {
            painter.setPen(Qt::blue);
            for (const QPoint& point : currentBsplineControlPoints) {
                painter.drawEllipse(point, 5, 5);
            }
        }

        // 绘制当前正在创建的多边形
        if (currentPolygon.points.size() > 1) {
            for (size_t i = 0; i < currentPolygon.points.size() - 1; ++i) {
                painter.drawLine(currentPolygon.points[i].x,
                                 currentPolygon.points[i].y,
                                 currentPolygon.points[i + 1].x,
                                 currentPolygon.points[i + 1].y);
            }
        }

        // 绘制当前正在创建的多边形裁剪窗口
        if (_cropPolygon.size() > 1) {
            for (size_t i = 0; i < _cropPolygon.size() - 1; ++i) {
                painter.drawLine(_cropPolygon[i].x(),
                                 _cropPolygon[i].y(),
                                 _cropPolygon[i + 1].x(),
                                 _cropPolygon[i + 1].y());
            }
        }
        // 如果有起点，绘制线段的预览
        if (hasStartPoint) {
            QPen pen;
            QColor temp_color = Qt::gray; // 设置预览线条颜色(灰色）
            pen.setColor(temp_color);
            pen.setWidth(lineWidth); // 使用当前设置的线条宽度
            painter.setPen(pen);

            if (mode == LineMode) {
                if (line_algo == DashLine)
                    drawPreviewDash(painter, startPoint, endPoint);
                else
                    drawPreviewSolid(painter, startPoint, endPoint);
            } else if (mode == SolidArrow) {
                if (way == 1)
                    drawSolidArrow(painter, startPoint, endPoint);
                else
                    draw2SolidArrow(painter, startPoint, endPoint);
            } else if (mode == DashedArrow) {
                if (way == 1)
                    drawDashedArrow(painter, startPoint, endPoint);
                else
                    draw2DashedArrow(painter, startPoint, endPoint);
            } else if (mode == CircleMode) {
                drawMidpointArc(painter, center, radius, startAngle, endAngle);
            } else if (mode == ArcMode) {
                if (startAngle <= endAngle)
                    drawMidpointArc(painter, center, radius, startAngle, endAngle);
                else {
                    drawMidpointArc(painter, center, radius, startAngle, 360);
                    drawMidpointArc(painter, center, radius, 0, endAngle);
                }
            } else if (mode == TrimMode && clip_algo != CropPolygon) {
                // 更新预览用画笔
                pen.setWidth(2);
                painter.setPen(pen);
                // 获取预览裁剪矩形框的四个顶点
                QPoint top_left(min(clipStartPoint.x(), clipEndPoint.x()),
                                max(clipStartPoint.y(), clipEndPoint.y()));
                QPoint top_right(max(clipStartPoint.x(), clipEndPoint.x()),
                                 max(clipStartPoint.y(), clipEndPoint.y()));
                QPoint bottom_left(min(clipStartPoint.x(), clipEndPoint.x()),
                                   min(clipStartPoint.y(), clipEndPoint.y()));
                QPoint bottom_right(max(clipStartPoint.x(), clipEndPoint.x()),
                                    min(clipStartPoint.y(), clipEndPoint.y()));
                // 绘制预览矩形框
                drawPreviewDash(painter, top_left, top_right);
                drawPreviewDash(painter, top_right, bottom_right);
                drawPreviewDash(painter, bottom_right, bottom_left);
                drawPreviewDash(painter, bottom_left, top_left);
            }
        }

        // 如果有起点和终点，绘制实际线段
        if (drawing) {
            QPen pen;
            pen.setColor(currentLineColor); // 使用当前设置的颜色
            pen.setWidth(lineWidth);        // 使用当前设置的线条宽度
            painter.setPen(pen);

            if (mode == LineMode) {
                switch (line_algo) {
                case DDA:
                    drawDDALine(painter, startPoint, endPoint);
                    break;
                case Bresenham:
                    drawBresenhamLine(painter, startPoint, endPoint);
                    break;
                case Midpoint:
                    drawMidpointLine(painter, startPoint, endPoint);
                    break;
                case DashLine:
                    drawDashLine(painter, startPoint, endPoint);
                    break;
                }
            } else if (mode == CircleMode) {
                drawMidpointArc(painter, center, radius, startAngle, endAngle);
            } else if (mode == ArcMode) {
                if (startAngle <= endAngle)
                    drawMidpointArc(painter, center, radius, startAngle, endAngle);
                else {
                    drawMidpointArc(painter, center, radius, startAngle, 360);
                    drawMidpointArc(painter, center, radius, 0, endAngle);
                }
            }
        }
    }

    // 判定绘制的时候是否出界（针对MAP）
    // 写fill的lyc：谢谢你提供了这个东西
    // lyc：不是，如果width和height没有默认参数我为什么不自己写判断
    // lty: 那你自己设置个默认参数嘛，就自己动手丰衣足食，懂吧
    // lyc：不是你真回我消息了
    // lyc：那我改了
    bool checkLegalPos(int x, int y, int width = 800, int height = 550)
    {
        if (x >= 0 && x <= width && y >= 0 && y <= height) {
            return true;
        } else
            return false;
    }

    // 绘制像素点与 MAP 对应的像素数据
    void drawPixel(int x, int y, QPainter& painter)
    {
        //不使用笔刷
        if (checkLegalPos(x, y, 800, 550)) {
            painter.drawPoint(x, y);
            // qDebug() << x << " " << y;
            MAP[x][y].setColor(painter.pen().color());
        }
    };

    // DDA 算法实现
    void drawDDALine(QPainter& painter, QPoint p1, QPoint p2)
    {
        int dx = p2.x() - p1.x();
        int dy = p2.y() - p1.y();
        int steps = std::max(abs(dx), abs(dy));
        float xIncrement = dx / static_cast<float>(steps);
        float yIncrement = dy / static_cast<float>(steps);

        float x = p1.x();
        float y = p1.y();
        for (int i = 0; i <= steps; ++i) {
            drawPixel(static_cast<int>(x), static_cast<int>(y), painter);
            x += xIncrement;
            y += yIncrement;
        }
    }

    // Bresenham 算法实现
    void drawBresenhamLine(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        while (x1 != x2 || y1 != y2) {
            drawPixel(x1, y1, painter);
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }
    }

    // 实线箭头 算法实现
    void drawSolidArrow(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        // 绘制直线
        while (x1 != x2 || y1 != y2) {
            drawPixel(x1, y1, painter);
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }

        // 绘制箭头
        drawArrowHead(painter, p1, p2);
    }

    void draw2SolidArrow(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        // 绘制直线
        while (x1 != x2 || y1 != y2) {
            drawPixel(x1, y1, painter);
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }
        }

        // 绘制箭头
        drawArrowHead(painter, p1, p2);
        drawArrowHead(painter, p2, p1);
    }

    // 虚线箭头 算法实现
    void draw2DashedArrow(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        int dashLength = 8;                       // 每段虚线的长度
        int gapLength = 8;                        // 每段空白的长度
        int totalLength = dashLength + gapLength; // 总周期长度
        int stepCount = 0;                        // 计数步数，用于决定是否绘制

        while (x1 != x2 || y1 != y2) {
            // 只在虚线的部分绘制点
            if (stepCount % totalLength < dashLength) {
                drawPixel(x1, y1, painter);
            }

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }

            stepCount++; // 增加步数计数器
        }

        // 绘制箭头
        drawArrowHead(painter, p1, p2);
        drawArrowHead(painter, p2, p1);
    }

    void drawDashedArrow(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        int dashLength = 8;                       // 每段虚线的长度
        int gapLength = 8;                        // 每段空白的长度
        int totalLength = dashLength + gapLength; // 总周期长度
        int stepCount = 0;                        // 计数步数，用于决定是否绘制

        while (x1 != x2 || y1 != y2) {
            // 只在虚线的部分绘制点
            if (stepCount % totalLength < dashLength) {
                drawPixel(x1, y1, painter);
            }

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }

            stepCount++; // 增加步数计数器
        }

        // 绘制箭头
        drawArrowHead(painter, p1, p2);
    }

    // 中点算法实现
    void drawMidpointLine(QPainter& painter, QPoint p1, QPoint p2)
    {
        // 提取信息
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();

        int x = x1, y = y1; //赋初始点
        int dy = y1 - y2, dx = x2 - x1;
        int delta_x = (dx >= 0 ? 1 : (dx = -dx, -1)); //若dx>0则步长为1，否则为-1，同时dx变正
        int delta_y = (dy <= 0 ? 1 : (dy = -dy, -1)); //注意这里dy<0,才是画布中y的增长方向

        drawPixel(x, y, painter); //画起始点

        int d, incrE, incrNE;
        if (-dy <= dx) // 斜率绝对值 <= 1
        //这里-dy即画布中的dy
        {
            d = 2 * dy + dx;        //初始化判断式d
            incrE = 2 * dy;         //取像素E时判别式增量
            incrNE = 2 * (dy + dx); //NE
            while (x != x2) {
                if (d < 0)
                    y += delta_y, d += incrNE;
                else
                    d += incrE;
                x += delta_x;
                drawPixel(x, y, painter);
            }
        } else // 斜率绝对值 > 1
        // x和y情况互换
        {
            d = 2 * dx + dy;
            incrE = 2 * dx;
            incrNE = 2 * (dy + dx);
            while (y != y2) {
                if (d < 0) //注意d变化情况
                    d += incrE;
                else
                    x += delta_x, d += incrNE;
                y += delta_y;
                drawPixel(x, y, painter);
            }
        }
    }

    // 虚线绘制 Bresenham 算法实现
    void drawDashLine(QPainter& painter, QPoint p1, QPoint p2)
    {
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;

        int dashLength = 8;                       // 每段虚线的长度
        int gapLength = 8;                        // 每段空白的长度
        int totalLength = dashLength + gapLength; // 总周期长度
        int stepCount = 0;                        // 计数步数，用于决定是否绘制

        while (x1 != x2 || y1 != y2) {
            // 只在虚线的部分绘制点
            if (stepCount % totalLength < dashLength) {
                drawPixel(x1, y1, painter);
            }

            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x1 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y1 += sy;
            }

            stepCount++; // 增加步数计数器
        }
    }

    // 中点圆弧算法实现
    void drawMidpointArc(QPainter& painter, QPoint center, int radius, int startAngle, int endAngle)
    {
        // 将角度转换为弧度
        float startRad = startAngle * M_PI / 180.0;
        float endRad = endAngle * M_PI / 180.0;

        int x = radius;
        int y = 0;
        int d = 1 - radius;

        drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);

        while (x > y) {
            if (d < 0) {
                d += 2 * y + 3;
            } else {
                d += 2 * (y - x) + 5;
                --x;
            }
            ++y;
            drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);
        }
    }

    // 绘制对称点，只绘制在角度范围内的点
    void drawSymmetricPointsArc(
        QPainter& painter, QPoint center, int x, int y, float startRad, float endRad)
    {
        drawPointInArc(painter, center, x, y, startRad, endRad);
        drawPointInArc(painter, center, -x, y, startRad, endRad);
        drawPointInArc(painter, center, x, -y, startRad, endRad);
        drawPointInArc(painter, center, -x, -y, startRad, endRad);
        drawPointInArc(painter, center, y, x, startRad, endRad);
        drawPointInArc(painter, center, -y, x, startRad, endRad);
        drawPointInArc(painter, center, y, -x, startRad, endRad);
        drawPointInArc(painter, center, -y, -x, startRad, endRad);
    }

    // 判断点是否在圆弧角度范围内并绘制
    void drawPointInArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad)
    {
        float angle = atan2(y, x);
        if (angle < 0)
            angle += 2 * M_PI;
        if (angle >= startRad && angle <= endRad) {
            drawPixel(center.x() + x, center.y() + y, painter);
        }
    }

    // 扫描线算法
    void scanlineFill(QPainter& painter, const Polygon& polygon)
    {
        if (polygon.points.size() < 3)
            return;

        // 计算最小和最大 y 坐标
        int ymin = polygon.points[0].y, ymax = polygon.points[0].y;
        for (const Point& p : polygon.points) {
            ymin = min(ymin, p.y);
            ymax = max(ymax, p.y);
        }

        // 对每条扫描线从 ymin 到 ymax 进行处理
        for (int y = ymin; y <= ymax; ++y) {
            vector<int> intersections;

            // 计算每条边与当前扫描线的交点
            for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
                const Point& p1 = polygon.points[i];
                const Point& p2 = polygon.points[i + 1];

                if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
                    int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                    intersections.push_back(x);
                }
            }

            // 对交点进行排序
            sort(intersections.begin(), intersections.end());

            // 画出交点之间的线段(预览版，故使用Qt的函数)
            for (size_t i = 0; i < intersections.size(); i += 2) {
                painter.drawLine(intersections[i], y, intersections[i + 1], y);
            }
        }
    }

    void drawSpecialPolygon(Point point, PolygonShape Pshape)
    {
        Polygon Spolygon;
        int half_h = int(Pshape.height / 2);
        int half_w = int(Pshape.width / 2);
        Spolygon.points.push_back(Point(point.x - half_w, point.y - half_h));
        Spolygon.points.push_back(Point(point.x - half_w, point.y + half_h));
        Spolygon.points.push_back(Point(point.x + half_w, point.y + half_h));
        Spolygon.points.push_back(Point(point.x + half_w, point.y - half_h));

        Spolygon.closePolygon(); // 封闭当前多边形
        shape.push_back(3);
        polygons.push_back(Spolygon); // 保存到多边形列表
        update();
        return;
    }

    void setSpecialPolygon(Point p)
    {
        bool ok;
        int width
            = QInputDialog::getInt(this,
                                   tr("Set width"),
                                   tr("Enter width(enter negetive if you want the DIY polygon):"),
                                   100,
                                   -10,
                                   400,
                                   1,
                                   &ok);
        if (ok) {
            if (width < 0) {
                if (abs(width) > PolygonShapes.size()) {
                    qDebug() << "[setSpecialPolygon] : the PolygonShape is out of range, which is "
                             << width << "\n";
                    return;
                } else {
                    drawSpecialPolygon(p, PolygonShapes[abs(++width)]);
                }
            } else {
                int height = QInputDialog::getInt(this,
                                                  tr("Set height"),
                                                  tr("Enter height:"),
                                                  50,
                                                  0,
                                                  400,
                                                  1,
                                                  &ok);
                if (ok) {
                    PolygonShape* nowPolygonShape = new PolygonShape(width, height);
                    PolygonShapes.push_back(*nowPolygonShape);
                    drawSpecialPolygon(p, *nowPolygonShape);
                }
            }
        }
    }

    void drawSpecialCircle(QPoint point, int radius)
    {
        arcs.append(Arc(point, radius, 0, 360, lineWidth, currentLineColor));
        shape.push_back(2);
        update();
        return;
    }

    void setSpecialCircle(QPoint p)
    {
        bool ok;
        int radius
            = QInputDialog::getInt(this,
                                   tr("Set radius"),
                                   tr("Enter radius(enter negetive if you want the DIY circle):"),
                                   50,
                                   -10,
                                   400,
                                   1,
                                   &ok);
        if (ok) {
            if (radius < 0) {
                if (abs(radius) > CircleShapes.size()) {
                    qDebug() << "[setSpecialCircle] : the CircleShape is out of range, which is "
                             << radius << "\n";
                    return;
                } else {
                    drawSpecialCircle(p, CircleShapes[abs(++radius)]);
                }
            } else {
                CircleShapes.push_back(radius);
                drawSpecialCircle(p, radius);
            }
        }
    }

    // 种子点填充算法
    void fillShape(QPainter& painter, Point point, QColor newColor)
    {
        //将点击位置的颜色设置为需要替换的颜色
        QColor oldColor = MAP[point.Getx()][point.Gety()].getColor();
        if (oldColor == newColor)
            return;
        int xl, xr, i, x, y;
        x = point.Getx();
        y = point.Gety();
        bool spanNeedFill;
        Point pt(point);
        stack.push(pt);
        while (!stack.isEmpty()) {
            pt = stack.pop();
            y = pt.Gety();
            x = pt.Getx();
            //if (y >= 549 || y <= 0)continue;
            while (MAP[x][y].getColor() == oldColor && x < 799) {
                drawPixel(x, y, painter);
                x++;
            }
            xr = x - 1;
            x = pt.Getx() - 1;
            while (MAP[x][y].getColor() == oldColor && x > 0) {
                drawPixel(x, y, painter);
                x--;
            }
            xl = x + 1;
            x = xl;
            y = y + 1;
            while (x < xr) {
                spanNeedFill = false;
                while (MAP[x][y].getColor() == oldColor && x < 799) {
                    spanNeedFill = true;
                    x++;
                }
                if (spanNeedFill) {
                    pt.x = x - 1;
                    pt.y = y;
                    stack.push(pt);
                    spanNeedFill = false;
                }
                while (MAP[x][y].getColor() != oldColor && x < xr)
                    x++;
            }
            x = xl;
            y = y - 2;
            while (x < xr) {
                spanNeedFill = false;
                while (MAP[x][y].getColor() == oldColor && x < 599) {
                    spanNeedFill = true;
                    x++;
                }
                if (spanNeedFill) {
                    pt.x = x - 1;
                    pt.y = y;
                    stack.push(pt);
                    spanNeedFill = false;
                }
                while (MAP[x][y].getColor() != oldColor && x < xr)
                    x++;
            }
        }
    }

    // Sutherland 点编码
    OutCode computeOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax)
    {
        OutCode code = INSIDE;

        if (x < xmin)
            code = OutCode(code | LEFT);
        else if (x > xmax)
            code = OutCode(code | RIGHT);
        if (y < ymin)
            code = OutCode(code | BOTTOM);
        else if (y > ymax)
            code = OutCode(code | TOP);

        return code;
    }

    // Sutherland 裁剪直线段算法
    bool cohenSutherlandClip(QLineF& line, double xmin, double ymin, double xmax, double ymax)
    {
        double x0 = line.x1(), y0 = line.y1();
        double x1 = line.x2(), y1 = line.y2();

        OutCode outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
        OutCode outcode1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
        bool accept = false;

        while (true) {
            if (!(outcode0 | outcode1)) {
                // Both endpoints inside rectangle
                accept = true;
                break;
            } else if (outcode0 & outcode1) {
                // Both endpoints outside rectangle
                break;
            } else {
                // Some segment of the line lies within the rectangle
                double x, y;
                // Pick an endpoint that is outside the rectangle
                OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

                if (outcodeOut & TOP) { // point above the clip rectangle
                    x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                    y = ymax;
                } else if (outcodeOut & BOTTOM) { // point below the clip rectangle
                    x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                    y = ymin;
                } else if (outcodeOut & RIGHT) { // point to the right of the clip rectangle
                    y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                    x = xmax;
                } else if (outcodeOut & LEFT) { // point to the left of the clip rectangle
                    y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                    x = xmin;
                }

                if (outcodeOut == outcode0) {
                    x0 = x;
                    y0 = y;
                    outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
                } else {
                    x1 = x;
                    y1 = y;
                    outcode1 = computeOutCode(x1, y1, xmin, ymin, xmax, ymax);
                }
            }
        }
        if (accept) {
            line.setP1(QPointF(x0, y0));
            line.setP2(QPointF(x1, y1));
        }
        return accept;
    }

    // 中点算法裁剪直线段
    bool liangBarskyClip(QLineF& line, double xmin, double ymin, double xmax, double ymax)
    {
        double x0 = line.x1(), y0 = line.y1();
        double x1 = line.x2(), y1 = line.y2();

        double dx = x1 - x0;
        double dy = y1 - y0;

        double t0 = 0.0, t1 = 1.0;
        double p[] = {-dx, dx, -dy, dy};
        double q[] = {x0 - xmin, xmax - x0, y0 - ymin, ymax - y0};

        for (int i = 0; i < 4; i++) {
            if (p[i] == 0 && q[i] < 0) {
                // Line is parallel and outside the boundary
                return false;
            }

            double r = q[i] / p[i];
            if (p[i] < 0) {
                if (r > t1)
                    return false;
                else if (r > t0)
                    t0 = r;
            } else if (p[i] > 0) {
                if (r < t0)
                    return false;
                else if (r < t1)
                    t1 = r;
            }
        }

        if (t0 > t1)
            return false;

        line.setP1(QPointF(x0 + t0 * dx, y0 + t0 * dy));
        line.setP2(QPointF(x0 + t1 * dx, y0 + t1 * dy));
        return true;
    }

    // 求多边形中心点，用于旋转
    Point getPolyCenter(vector<Point> Poly)
    {
        Point p0 = Poly[0];
        Point p1 = Poly[1];
        Point p2;
        int Center_X, Center_Y;
        double sumarea = 0, sumx = 0, sumy = 0;
        double area;
        for (int i = 2; i < Poly.size(); i++) {
            p2 = Poly[i];
            area = p0.Getx() * p1.Gety() + p1.Getx() * p2.Gety() + p2.Getx() * p0.Gety()
                   - p1.Getx() * p0.Gety() - p2.Getx() * p1.Gety() - p0.Getx() * p2.Gety();
            area /= 2;
            sumarea += area;
            sumx += (p0.Getx() + p1.Getx() + p2.Getx()) * area; //求∑cx[i] * s[i]和∑cy[i] * s[i]
            sumy += (p0.Gety() + p1.Gety() + p2.Gety()) * area;
            p1 = p2; //转换为下一个三角形，求总面积
        }
        Center_X = (int) (sumx / sumarea / 3.0);
        Center_Y = (int) (sumy / sumarea / 3.0);
        qDebug() << Center_X;
        qDebug() << Center_Y;
        return Point(Center_X, Center_Y);
    }

    // 辅助OnSegment函数
    int crossProduct(Point A, Point B, Point C)
    {
        return (B.Getx() - A.Getx()) * (C.Gety() - A.Gety())
               - (C.Getx() - A.Getx()) * (B.Gety() - A.Gety());
    }

    // 辅助OnSegment函数
    int dotProduct(Point p1, Point p2) { return p1.Getx() * p2.Getx() + p1.Gety() * p2.Gety(); }

    //判断点Q是否在P1和P2的线段上
    bool OnSegment(Point P1, Point P2, Point Q)
    {
        //前一个判断点Q在P1P2直线上 后一个判断在P1P2范围上
        //QP1 X QP2
        return crossProduct(Q, P1, P2) == 0 && dotProduct(P1 - Q, P2 - Q) <= 0;
    }

    // 给出两个点和一个原点，计算两点与原点构成的夹角
    double getAngle(QPoint origin, QPoint p1, QPoint p2)
    {
        int x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y(), x3 = origin.x(), y3 = origin.y();
        double theta = atan2(x1 - x3, y1 - y3) - atan2(x2 - x3, y2 - y3);

        if (theta > M_PI)
            theta -= 2 * M_PI;
        if (theta < -M_PI)
            theta += 2 * M_PI;

        theta = abs(theta * 180.0 / M_PI);
        if ((x2 - x3) * (y2 - y1) - (y2 - y3) * (x2 - x1) < 0)
            theta *= -1;
        return theta;
    }

    // 给出两个点和一个原点，计算两点与原点构成的夹角
    double getAngle(Point origin, Point p1, Point p2)
    {
        int x1 = p1.Getx(), y1 = p1.Gety(), x2 = p2.Getx(), y2 = p2.Gety(), x3 = origin.Getx(),
            y3 = origin.Gety();
        double theta = atan2(x1 - x3, y1 - y3) - atan2(x2 - x3, y2 - y3);

        if (theta > M_PI)
            theta -= 2 * M_PI;
        if (theta < -M_PI)
            theta += 2 * M_PI;

        theta = abs(theta * 180.0 / M_PI);
        if ((x2 - x3) * (y2 - y1) - (y2 - y3) * (x2 - x1) < 0)
            theta *= -1;
        return theta;
    }

    // 测试p点是否在多边形内部
    bool polyContains(vector<Point> polygon, Point P)
    {
        bool flag = false; //相当于计数
        Point P1, P2;      //多边形一条边的两个顶点
        for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            //polygon[]是给出多边形的顶点
            P1 = polygon[i];
            P2 = polygon[j];
            if (OnSegment(P1, P2, P))
                return true; //点在多边形一条边上
            if (((P1.Gety() - P.Gety()) > 0 != (P2.Gety() - P.Gety()) > 0)
                && (P.Getx()
                    - (P.Gety() - P1.Gety()) * (P1.Getx() - P2.Getx()) / (P1.Gety() - P2.Gety())
                    - P1.Getx())
                       < 0)
                flag = !flag;
        }
        qDebug() << flag;
        return flag;
    }

    // 使用变换矩阵进行多边形变换
    void polygonTrans(QMouseEvent* e)
    {
        transMatrix trM;
        double zoomPropor_X, zoomPropor_Y; //进行缩放的比例
        Point moveVector;                  //进行移动的向量
        double angle;

        tempTransPoly = *nowPolygon;

        if (iscomfirm) {
            //tempTransPoly = *nowPolygon;
            iscomfirm = false;
        }

        if (isSpecificRefer)
            trM.setReference(referancePoint);
        else {
            referancePoint = getPolyCenter((*nowPolygon).points);
            trM.setReference(referancePoint);
        }

        switch (trans_algo) {
        case MOVE:
            moveVector = Point(e->pos()) - _begin; //计算移动向量，终点减起点
            trM.setMoveTrans(moveVector);
            for (int i = 0; i < (*nowPolygon).points.size(); ++i) {
                //vector<Point> tem((*nowPolygon).points.size());
                (*nowPolygon).points[i] = trM * ((*nowPolygon).points[i]);
                //tem[i] = trM * (*nowPolygon).points[i];
                //(*nowPolygon).points[i] = tem[i];
            }
            if (isInFill) {
                trM.setMoveTrans(getPolyCenter((*nowPolygon).points) - nowFill.point);
                nowFill.point = trM * nowFill.point;
            }
            _begin = Point(e->pos());
            update();
            break;
        case ZOOM:
            qDebug() << isInTagRect << "----\n";
            if (isInTagRect) {
                qDebug() << "in ZOOM!!!\n";
                zoomPropor_X = abs(e->pos().x() - referancePoint.Getx()) * 1.0
                               / abs(tempTransPoly.points[0].Getx() - referancePoint.Getx());
                zoomPropor_Y = abs(e->pos().y() - referancePoint.Gety()) * 1.0
                               / abs(tempTransPoly.points[0].Gety() - referancePoint.Gety());
                trM.setZoomTrans(zoomPropor_X, zoomPropor_Y); //生成缩放变换矩阵
                for (int i = 0; i < (*nowPolygon).points.size(); ++i) {
                    //(*nowPolygon)[i] = trM * tempTransPoly[i];
                    (*nowPolygon).points[i] = trM * tempTransPoly.points[i];
                }
                if (isInFill) {
                    trM.setMoveTrans(getPolyCenter((*nowPolygon).points) - nowFill.point);
                    nowFill.point = trM * nowFill.point;
                }
                trM.setMoveTrans((*nowPolygon).points[0]
                                 - transRectTag->center()); //让标志矩形中心点跟随移动到新点
                QPoint leftTop = QPoint((trM * (transRectTag->topLeft())).Getx(),
                                        (trM * (transRectTag->topLeft())).Gety());
                transRectTag->setTopLeft(leftTop);
                //transRectTag->setBottomRight(trM * (transRectTag->bottomRight()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(),
                                                    (trM * (transRectTag->bottomRight())).Gety()));
                update();
                //_begin = e->pos();
            }
            break;
        case ROTATE:
            qDebug() << isInTagRect << "----\n";
            if (isInTagRect) {
                qDebug() << "in ROTATE!!!\n";
                angle = getAngle(referancePoint, tempTransPoly.points[0], e->pos());
                trM.setRotateTrans(angle);
                if (angle > 30
                    || angle
                           < -30) /**旋转时连续旋转超过60多度时会出现bug，目前原因尚不明确，处理方法为每次超过30度时更新暂存多边形，则下次的角度从零计算**/
                    iscomfirm = true; /**切勿删除！！！！！！！！！！！！**/
                for (int i = 0; i < (*nowPolygon).points.size(); ++i) {
                    //(*nowPolygon)[i] = trM * tempTransPoly[i];
                    (*nowPolygon).points[i] = trM * tempTransPoly.points[i];
                }
                if (isInFill) {
                    //trM.setMoveTrans(getPolyCenter(*nowPolygon) - *nowFill);
                    trM.setMoveTrans(getPolyCenter((*nowPolygon).points) - nowFill.point);
                    //(*nowFill) = trM * (*nowFill);
                    nowFill.point = trM * nowFill.point;
                }
                trM.setMoveTrans((*nowPolygon).points[0]
                                 - transRectTag->center()); //让标志矩形中心点跟随移动到新点
                //transRectTag->setTopLeft(trM * (transRectTag->topLeft()));
                //transRectTag->setBottomRight(trM * (transRectTag->bottomRight()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->topLeft())).Getx(),
                                                    (trM * (transRectTag->topLeft())).Gety()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(),
                                                    (trM * (transRectTag->bottomRight())).Gety()));
                _begin = e->pos();
                update();
            }
        }
    }

    // 检测点是否在多边形外
    bool outsideOneEdgeOfPolygon(QVector<QPoint> polygon, QPoint p, int x)
    {
        QPoint p1 = polygon[x];
        QPoint p2 = polygon[(x + 1) % int(polygon.length())];
        QPoint p3 = polygon[(x + 2) % int(polygon.length())];
        int a = p2.y() - p1.y();
        int b = p1.x() - p2.x();
        int c = p2.x() * p1.y() - p1.x() * p2.y();
        if (a < 0) {
            a = -a;
            b = -b;
            c = -c;
        }
        int pointD = a * p.x() + b * p.y() + c;
        int polyNextPointD = a * p3.x() + b * p3.y() + c;
        if (pointD * polyNextPointD >= 0) {
            return true;
        } else {
            return false;
        }
    }

    // 判断点p是否位于裁剪边p1-p2的内侧（针对凸多边形）
    bool isInside(QPoint p, QPoint p1, QPoint p2)
    {
        return (p2.x() - p1.x()) * (p.y() - p1.y()) - (p2.y() - p1.y()) * (p.x() - p1.x()) <= 0;
    }

    // 计算两条线段的交点，如果平行则返回QPoint(-1, -1)
    QPoint intersection(QPoint p1, QPoint p2, QPoint k1, QPoint k2)
    {
        double a1 = p2.y() - p1.y();
        double b1 = p1.x() - p2.x();
        double c1 = a1 * p1.x() + b1 * p1.y();

        double a2 = k2.y() - k1.y();
        double b2 = k1.x() - k2.x();
        double c2 = a2 * k1.x() + b2 * k1.y();

        double determinant = a1 * b2 - a2 * b1;

        if (std::abs(determinant) < 1e-10) { // 检查是否平行
            return QPoint(-1, -1);           // 平行线没有交点
        } else {
            double x = (b2 * c1 - b1 * c2) / determinant;
            double y = (a1 * c2 - a2 * c1) / determinant;
            return QPoint(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)));
        }
    }

    // 根据给定多边形裁剪现有多边形
    QVector<QPoint> cropPolygon(const QVector<QPoint>& subjectPolygon,
                                const QVector<QPoint>& clipPolygon)
    {
        QVector<QPoint> output = subjectPolygon;

        for (int i = 0; i < clipPolygon.size(); ++i) {
            QVector<QPoint> input = output;
            output.clear();

            // 当前裁剪边的两个顶点
            QPoint clipEdgeStart = clipPolygon[i];
            QPoint clipEdgeEnd = clipPolygon[(i + 1) % clipPolygon.size()];

            for (int j = 0; j < input.size(); ++j) {
                // 被裁剪多边形当前边的两个顶点
                QPoint currentPoint = input[j];
                QPoint previousPoint = input[(j - 1 + input.size()) % input.size()];

                bool currentInside = isInside(currentPoint, clipEdgeStart, clipEdgeEnd);
                bool previousInside = isInside(previousPoint, clipEdgeStart, clipEdgeEnd);

                if (currentInside) {
                    if (!previousInside) {
                        // 如果从外部进入，计算交点
                        QPoint inter = intersection(previousPoint,
                                                    currentPoint,
                                                    clipEdgeStart,
                                                    clipEdgeEnd);
                        output.append(inter); // 添加交点
                    }
                    // 当前点在内侧，保留该点
                    output.append(currentPoint);
                } else if (previousInside) {
                    // 如果从内部离开，计算交点
                    QPoint inter = intersection(previousPoint,
                                                currentPoint,
                                                clipEdgeStart,
                                                clipEdgeEnd);
                    output.append(inter); // 添加交点
                }
            }
        }

        return output;
    }

    // 处理鼠标移动事件
    void mouseMoveEvent(QMouseEvent* event) override
    {
        // 获取鼠标的局部坐标（相对于窗口）
        QPoint localPos = event->pos();

        // 更新标签显示的鼠标坐标
        coordLabel->setText(QString("(%1, %2)").arg(localPos.x()).arg(localPos.y()));

        // 定位当前鼠标所指的多边形
        if (polygons.size() > 0) {
            for (int i = 0; i < polygons.size(); i++) {
                if (polyContains(polygons[i].points, Point(event->pos().x(), event->pos().y()))) {
                    nowPolygon = &polygons[i];
                    qDebug() << "nowPolygon" << (*nowPolygon).points[0].Getx() << "\n";
                    isInPolygon = 1;
                    isInEllipse = 0;
                    isInRect = 0;
                    isArrow = 0;
                    for (int j = 0; j < fills.size(); j++) {
                        if (polyContains(polygons[i].points, fills.at(j).point)) {
                            nowFill = fills[j];
                            isInFill = 1;
                        }
                    }
                }
            }
        }

        update();

        if (transRectTag->contains(event->pos())) {
            isInTagRect = true;
            isArrow = 0;
        }

        update();

        if (!isArrow && mode == TransMode) {
            setCursor(Qt::SizeAllCursor); //拖拽模式下，并且在拖拽图形中，将光标形状改为十字
            if (!(*nowPolygon).points.empty()) {
                polygonTrans(event);
                update();
            }
        } else {
            setCursor(Qt::ArrowCursor); //恢复原始光标形状
        }

        if (isInPolygon) {
            transMatrix trM;
            QPoint tem = QPoint((*nowPolygon).points[0].Getx(), (*nowPolygon).points[0].Gety());
            trM.setMoveTrans(tem - transRectTag->center());
            transRectTag->setBottomRight(QPoint((trM * (transRectTag->topLeft())).Getx(),
                                                (trM * (transRectTag->topLeft())).Gety()));
            transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(),
                                                (trM * (transRectTag->bottomRight())).Gety()));
        }

        if (hasStartPoint) {
            if (mode == LineMode || mode == SolidArrow || mode == DashedArrow) {
                // 更新终点为当前鼠标位置
                endPoint = event->pos();
            } else if (mode == CircleMode) {
                QPoint currentPos = event->pos();
                radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2)
                                   + std::pow(currentPos.y() - center.y(), 2));
            } else if (mode == ArcMode && counter == 0) {
                return;
            } else if (mode == ArcMode && counter == 1) {
                QPoint currentPos = event->pos();
                radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2)
                                   + std::pow(currentPos.y() - center.y(), 2));

                // 计算当前鼠标位置与圆心的角度
                float angleRad = atan2(currentPos.y() - center.y(), currentPos.x() - center.x());
                endAngle = static_cast<int>(angleRad * 180.0 / M_PI); // 转换为角度

                if (endAngle < 0) {
                    endAngle += 360; // 确保角度为正
                }
            } else if (mode == TrimMode) {
                // 这个else if为什么要放在 ArcMode 里面？？？
                // 放里面当然用不了重绘啦
                clipEndPoint = event->pos();
            } else if (mode == BezierMode && isOnPoint1 != -1 && !ctr_or_not) {
                currentBezierControlPoints[isOnPoint1] = event->pos(); // 更新当前Bezier曲线控制点位置
            } else if (mode == BezierMode && SelectedBezier != -1 && SelectedPoint != -1
                       && ctr_or_not) {
                all_beziers[SelectedBezier][SelectedPoint]
                    = event->pos(); // 更新所有Bezier曲线控制点位置
            } else if (mode == BsplineMode && isOnPoint2 != -1 && !ctr_or_not) {
                currentBsplineControlPoints[isOnPoint2] = event->pos(); // 更新当前Bezier曲线控制点位置
            } else if (mode == BsplineMode && SelectedBspline != -1 && SelectedPoint != -1
                       && ctr_or_not) {
                all_bsplines[SelectedBspline][SelectedPoint]
                    = event->pos(); // 更新所有Bezier曲线控制点位置
            }

            update(); // 触发重绘
        }
    }

    // 处理鼠标按下事件
    void mousePressEvent(QMouseEvent* event) override
    {
        setCursor(Qt::ArrowCursor); //恢复原始光标形状
        if (event->button() == Qt::MiddleButton) {
            if (mode == TransMode) {
                referancePoint = event->pos();
                isSpecificRefer = true;
                update();
            }
        }

        if (isInPolygon) {
            _begin = event->pos();
            qDebug() << "found the polygon";
        }

        if (!hasStartPoint) {
            if (mode == LineMode) {
                // 第一次点击：记录起点
                startPoint = event->pos();
                endPoint = startPoint; // 初始化终点为起点
                shape.push_back(1);
            } else if (mode == SolidArrow) {
                startPoint = event->pos();
                endPoint = startPoint; // 初始化终点为起点
                shape.push_back(114);
            } else if (mode == DashedArrow) {
                startPoint = event->pos();
                endPoint = startPoint; // 初始化终点为起点
                shape.push_back(514);
            } else if (mode == CircleMode) {
                center = event->pos(); // 记录圆心
                hasStartPoint = true;
                shape.push_back(2);
            } else if (mode == ArcMode && counter == 0) {
                center = event->pos(); // 记录圆心
                shape.push_back(2);
            } else if (mode == ArcMode && counter == 1) {
                // 计算当前鼠标位置与圆心的角度
                float angleRad = atan2(event->pos().y() - center.y(), event->pos().x() - center.x());
                startAngle = static_cast<int>(angleRad * 180.0 / M_PI); // 转换为角度
                if (startAngle < 0) {
                    startAngle += 360; // 确保角度为正
                }
            } else if (mode == PolygonMode) {
                // 获取鼠标点击的位置
                int x = event->pos().x();
                int y = event->pos().y();
                currentPolygon.addPoint(Point(x, y));    // 添加顶点
                currentPolygon.color = currentLineColor; // 修改多边形颜色

                update(); // 触发界面重绘
            } else if (mode == FillMode) {
                fills.push_back(Fill(Point(event->pos().x(), event->pos().y()), currentLineColor));
                shape.push_back(4);
            } else if (mode == TrimMode && clip_algo != CropPolygon) {
                clipStartPoint = event->pos(); // 记录鼠标按下的位置作为起点
                clipEndPoint = clipStartPoint;
            } else if (mode == TrimMode && clip_algo == CropPolygon) {
                QPoint a;
                a.setX(event->pos().x());
                a.setY(event->pos().y());
                _cropPolygon.append(a);
                update();
            } else if (mode == BezierMode && !ctr_or_not) {
                QPoint pos = event->pos();
                // 检查是否点击在已有的控制点附近
                bool pointSelected = false;
                for (int i = 0; i < currentBezierControlPoints.size(); ++i) {
                    if ((pos - currentBezierControlPoints[i]).manhattanLength() < 10) {
                        isOnPoint1 = i; // 选择已有的点进行拖动
                        pointSelected = true;
                        break;
                    }
                }
                if (!pointSelected) {
                    // 添加新的控制点
                    currentBezierControlPoints.append(pos);
                    isOnPoint1 = -1; // 不选中新点
                }
                update(); // 更新绘图
            } else if (mode == BezierMode && ctr_or_not) {
                QPoint pos = event->pos();
                // 检查是否点击在已有的控制点附近
                bool pointSelected = false;
                for (int i = 0; i < all_beziers.size(); ++i) {
                    for (int j = 0; j < all_beziers[i].size(); j++) {
                        if ((pos - all_beziers[i][j]).manhattanLength() < 10) {
                            SelectedBezier = i; // 选择已有的点进行拖动
                            SelectedPoint = j;
                            pointSelected = true;
                            break;
                        }
                    }
                }
                if (!pointSelected) {
                    SelectedBezier = -1; // 不选中新点
                    SelectedPoint = -1;
                }
            } else if (mode == BsplineMode && !ctr_or_not) {
                QPoint pos = event->pos();
                // 检查已有绘制点
                bool pointSelected = false;
                for (int i = 0; i < currentBsplineControlPoints.size(); ++i) {
                    if ((pos - currentBsplineControlPoints[i]).manhattanLength() < 10) {
                        isOnPoint2 = i; // 选择已有的点进行拖动
                        pointSelected = true;
                        break;
                    }
                }
                if (!pointSelected) {
                    // 添加新的控制点
                    currentBsplineControlPoints.append(pos);
                    qDebug() << "x:" << event->pos().x() << "y:" << event->pos().y() << "\n";
                    isOnPoint2 = -1; // 不选中新点
                }
                update(); // 更新绘图
            } else if (mode == BsplineMode && ctr_or_not) {
                QPoint pos = event->pos();
                // 检查是否点击在已有的控制点附近
                bool pointSelected = false;
                for (int i = 0; i < all_bsplines.size(); ++i) {
                    for (int j = 0; j < all_bsplines[i].size(); j++) {
                        if ((pos - all_bsplines[i][j]).manhattanLength() < 10) {
                            SelectedBspline = i; // 选择已有的点进行拖动
                            SelectedPoint = j;
                            pointSelected = true;
                            break;
                        }
                    }
                }
                if (!pointSelected) {
                    SelectedBspline = -1; // 不选中新点
                    SelectedPoint = -1;
                }
            }

            hasStartPoint = true;
            drawing = false; // 初始化为不绘制实际直线
        }

        if (mode == SpecialPolygonMode) {
            setSpecialPolygon(Point(event->pos()));
            hasStartPoint = false;
        } else if (mode == SpecialCircleMode) {
            setSpecialCircle(event->pos());
            hasStartPoint = false;
        } else if (mode == WriteText) {
            // 创建文本框
            QLineEdit* lineEdit = new QLineEdit(this);
            lineEdit->setGeometry(event->x(), event->y(), 90, 40); // 设置位置和大小
            // 设置透明背景
            lineEdit->setStyleSheet(
                QString("QLineEdit { background: transparent; color: %1; font-size: %2px; }")
                    .arg("white")
                    .arg(frontsize));
            lineEdit->show();     // 显示文本框
            lineEdit->setFocus(); // 设置焦点
            hasStartPoint = false;
        }
    }

    // 处理鼠标松开事件
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (hasStartPoint) {
            if (mode == LineMode) {
                endPoint = event->pos(); // 直线模式下，松开设定终点
                lines.append(Line(startPoint, endPoint, lineWidth, currentLineColor, line_algo));
            } else if (mode == SolidArrow) {
                endPoint = event->pos();
                solid_arrows.append(Arrow(startPoint, endPoint, lineWidth, currentLineColor, way));
            } else if (mode == DashedArrow) {
                endPoint = event->pos();
                dashed_arrows.append(Arrow(startPoint, endPoint, lineWidth, currentLineColor, way));
            } else if (mode == CircleMode) {
                endPoint = event->pos(); // 圆模式下，计算半径
                radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2)
                                   + std::pow(endPoint.y() - center.y(), 2));
                arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
            } else if (mode == ArcMode && counter == 0) {
                counter += 1;
            } else if (mode == ArcMode && counter == 1) {
                endPoint = event->pos(); // 圆弧模式下，计算半径
                radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2)
                                   + std::pow(endPoint.y() - center.y(), 2));
                arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
                counter = 0;
            } else if (mode == TrimMode && clip_algo == SutherlandTrim
                       || mode == TrimMode && clip_algo == MidTrim) {
                // drawingrect = false;
                qDebug() << "Trim the line.\n";
                clipEndPoint = event->pos();
                // 确定裁剪矩形的左上、右下两个顶点
                double xmin = min(clipStartPoint.x(), clipEndPoint.x());
                double ymin = min(clipStartPoint.y(), clipEndPoint.y());
                double xmax = max(clipStartPoint.x(), clipEndPoint.x());
                double ymax = max(clipStartPoint.y(), clipEndPoint.y());

                // 更新在裁剪框内的直线的起始与结束端点
                for (Line& line : lines) {
                    QLineF lineF(line.line);
                    if (clip_algo == SutherlandTrim) {
                        cohenSutherlandClip(lineF, xmin, ymin, xmax, ymax);
                    } else if (clip_algo == MidTrim) {
                        liangBarskyClip(lineF, xmin, ymin, xmax, ymax);
                    }
                    line.line.setP1(lineF.p1().toPoint());
                    line.line.setP2(lineF.p2().toPoint());
                }
            } else if (mode == TransMode) {
                iscomfirm = true;
            } else if (mode == BezierMode) {
                isOnPoint1 = -1; // Deselect control point
            } else if (mode == BsplineMode) {
                isOnPoint2 = -1; // Deselect control point
            }

            drawing = true;        // 绘制完成
            update();              // 触发重绘
            hasStartPoint = false; // 重置，允许再次绘制新的线段
            drawing = false;
        }
    }

    // 处理双击鼠标事件
    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        // 双击事件封闭当前多边形
        if (mode == PolygonMode) {
            if (currentPolygon.points.size() > 2) {
                currentPolygon.closePolygon(); // 封闭当前多边形
                shape.push_back(3);
                polygons.push_back(currentPolygon); // 保存到多边形列表
                currentPolygon = Polygon();         // 重置当前多边形以开始新的绘制
                update();
            }
        }
        // 双击事件封闭当前多边形裁剪区域
        if (mode == TrimMode && clip_algo == CropPolygon) {
            update();
            QVector<QVector<QPoint>> newPolygon;
            vector<int> deleteIndex;
            int k = 0;
            for (int i = 0; i < shape.length(); i++) {
                if (shape.at(i) == 3) {
                    qDebug() << "has Polygon\n";
                    // QPen pen = _brush.at(i);
                    QVector<QPoint> polygon = cropPolygon(P2QV(polygons.at(k++)), _cropPolygon);
                    qDebug() << "now the p has ::" << polygon.size() << "<<\n";
                    if (polygon.length() >= 3) {
                        qDebug() << "has new Polygon!\n";
                        // 如果返回的多边形的长度大于等于3，则说明裁切后的多边形不为空，将他们追加到新的多边形数组中
                        newPolygon.append(polygon);
                    }
                    deleteIndex.push_back(i);
                }
            }
            sort(deleteIndex.rbegin(), deleteIndex.rend());
            for (int i = 0; i < deleteIndex.size(); ++i) {
                polygons.remove(deleteIndex.size() - i - 1);
                shape.remove(deleteIndex[i]);
            }
            _cropPolygon.clear();
            for (int i = 0; i < newPolygon.length(); ++i) {
                polygons.append(QV2P(newPolygon.at(i)));
                shape.append(3);
                update();
            }
        }
    }

    // 处理按键事件
    void keyPressEvent(QKeyEvent* event) override
    {
        if (mode == BezierMode && event->key() == Qt::Key_Return) {
            all_beziers.append(currentBezierControlPoints);
            // qDebug() << "all_beziers's size: " << all_beziers.size() << "<<\n";
            shape.append(5);
            // qDebug() << "shape's size: " << shape.size() << "<<\n";
            currentBezierControlPoints.clear();
        }
        if (mode == BsplineMode && event->key() == Qt::Key_Return) {
            all_bsplines.append(currentBsplineControlPoints);
            // qDebug() << "all_bsplines's size: " << all_bsplines.size() << "<<\n";
            shape.append(6);
            // qDebug() << "shape's size: " << shape.size() << "<<\n";
            k_steps.push_back(last_k);
            currentBsplineControlPoints.clear();
        }
        if (event->key() == Qt::Key_Control) {
            ctr_or_not = true;
            qDebug() << "ctr_or_not" << ctr_or_not << "\n";
        }
        if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier)) {
            if (shape.size() > 0) {
                switch (shape.last()) {
                case 1:
                    lines.pop_back();
                    qDebug() << "Cancel a line.\n";
                    break;
                case 2:
                    arcs.pop_back();
                    qDebug() << "Cancel an arc.\n";
                    break;
                case 3:
                    polygons.pop_back();
                    qDebug() << "Cancel a polygon.\n";
                    break;
                case 4:
                    fills.pop_back();
                    qDebug() << "Cancel a fill area.\n";
                    break;
                case 5:
                    all_beziers.pop_back();
                    qDebug() << "Cancel a Bezier line.\n";
                    break;
                case 6:
                    all_bsplines.pop_back();
                    qDebug() << "Cancel a B-Spline.\n";
                    break;
                case 114:
                    solid_arrows.pop_back();
                    qDebug() << "Cancel a Solid Arrow.\n";
                    break;
                case 514:
                    dashed_arrows.pop_back();
                    qDebug() << "Cancel a Dashed Arrow.\n";
                    break;
                }
                shape.pop_back();
            }
        }
        update();
    }

    void keyReleaseEvent(QKeyEvent* event) override
    {
        if (event->key() == Qt::Key_Control) {
            ctr_or_not = false;
            qDebug() << "ctr_or_not" << ctr_or_not << "\n";
        }
        update();
    }

public:
    ShapeDrawer(QWidget* parent = nullptr)
        : QWidget(parent)
        , mode(LineMode)
        , hasStartPoint(false)
        , drawing(false)
        , radius(0)
        , startAngle(0)
        , endAngle(360)
    {
        // 设置背景颜色
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, Qt::white); // 这里设置背景颜色为白色
        this->setAutoFillBackground(true);         // 启用自动填充背景
        this->setPalette(pal);

        // 绘图窗口大小↓
        //setFixedSize(800, 550);
        initMAP(MAP);

        // 为四边绘制四条直线，防止填充溢出
        std::vector<vector<int>> tem = {{0, 0}, {799, 0}, {799, 549}, {0, 549}, {0, 0}};
        for (int i = 0; i < 4; ++i) {
            lines.append(Line(QPoint(tem[i][0], tem[i][1]),
                              QPoint(tem[i + 1][0], tem[i + 1][1]),
                              1,
                              Qt::black,
                              Midpoint));
            shape.append(1);
        }

        // 确保窗口部件可以接收键盘焦点
        setFocusPolicy(Qt::StrongFocus);

        // 创建一个标签用于显示鼠标坐标

        coordLabel = new QLabel("(0, 0)", this);
        coordLabel->setMinimumWidth(250);
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->addWidget(coordLabel);
        statusBar->setGeometry(0, 525, 600, 20);
        statusBar->setStyleSheet("border: solid black 2px;");

        // 启用鼠标追踪（否则只在按下按钮时才会追踪鼠标移动）
        // setMouseTracking(true);
    }

    // 设置当前绘图模式
    void setDrawMode(DrawMode newMode)
    {
        mode = newMode;
        update(); // 模式切换后重新绘制
    }

    // 新增：设置线条宽度的函数
    void setLineWidth(int width)
    {
        lineWidth = width;
        update(); // 每次改变宽度时，触发重新绘制
    }

    //  新增：设置当前线条颜色
    void setCurrentLineColor(QColor color)
    {
        currentLineColor = color;
        update();
    }

    //  新增：设置当前算法
    void setAlgorithm(line_Algorithm algo)
    {
        line_algo = algo;
        // update(); // 改变算法后重新绘制
    }

    // 新增设置裁剪直线段算法
    void setclipAlgorithm(clip_Algorithm algo) { clip_algo = algo; }

    // 新增设置变形模式
    void setTransAlgorithm(transMode algo) { trans_algo = algo; }

    // 新增：设置圆弧的起始角度和结束角度
    void setArcAngles(int start, int end)
    {
        startAngle = start;
        endAngle = end;
        update(); // 触发重绘
    }

    void cancel()
    {
        if (shape.size() > 0) {
            switch (shape.last()) {
            case 1:
                lines.pop_back();
                qDebug() << "Cancel a line.\n";
                break;
            case 2:
                arcs.pop_back();
                qDebug() << "Cancel an arc.\n";
                break;
            case 3:
                polygons.pop_back();
                qDebug() << "Cancel a polygon.\n";
                break;
            case 4:
                fills.pop_back();
                qDebug() << "Cancel a fill area.\n";
                break;
            case 5:
                all_beziers.pop_back();
                qDebug() << "Cancel a Bezier line.\n";
                break;
            case 6:
                all_bsplines.pop_back();
                qDebug() << "Cancel a B-Spline.\n";
                break;
            case 114:
                solid_arrows.pop_back();
                qDebug() << "Cancel a Solid Arrow.\n";
                break;
            case 514:
                dashed_arrows.pop_back();
                qDebug() << "Cancel a Dashed Arrow.\n";
                break;
            }
            shape.pop_back();
        }
        update();
    }

    // 新增：设置B样曲线阶数
    void setk_step(int k_value)
    {
        last_k = k_value;
        //k_steps.push_back(k_value);
        update();
    }

    // 设置单向、双向箭头
    void set_ways(int k) { way = k; }

    // 新增：清空画布功能
    void Clear()
    {
        shape.clear();
        lines.clear();
        solid_arrows.clear();
        dashed_arrows.clear();
        arcs.clear();
        polygons.clear();
        fills.clear();
        stack.clear();
        _cropPolygon.clear();
        all_beziers.clear();
        currentBezierControlPoints.clear();
        all_bsplines.clear();
        currentBsplineControlPoints.clear();
        k_steps.clear();
        clearMAP(MAP);
        update();
    }

    // 保存文件
    void save(QString filename)
    {
        qDebug() << "Start to save.\n";
        savePolygonsToTextFile(polygons, filename);
        saveArrowsToTextFile(solid_arrows,filename);
        saveArrowsToTextFile(dashed_arrows,filename);
    }

    // 读取文件
    void load(QString filename)
    {
        loadfromfile(filename);
        update();
    }
    void loadfromfile(const QString& filename){
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "无法打开文件进行读取:" << filename;
            return; // 直接返回
        }

        QTextStream in(&file);
        int numPolygons;
        in >> numPolygons; // 读取多边形数量

        for (int i = 0; i < numPolygons; ++i) {
            Polygon polygon;
            int numPoints;
            in >> numPoints; // 读取顶点数量

            for (int j = 0; j < numPoints; ++j) {
                float x, y;
                in >> x >> y; // 读取每个顶点
                polygon.addPoint(Point(x, y));
            }

            int r, g, b, a;
            in >> r >> g >> b >> a; // 读取颜色
            polygon.color = QColor(r, g, b, a);

            polygons.append(polygon);
            shape.append(3);
        }

        int numArrows;
        in >> numArrows; // 读取多边形数量
        for (int i = 0; i < numArrows; ++i) {
            Arrow arrow;

            int p1_x, p1_y, p2_x, p2_y, wi, wa;
            int r, g, b, a;
            in >> p1_x >> p1_y >> p2_x >> p2_y; // 读取line
            qDebug() << p1_x << " " << p1_y << " " << p2_x << " " << p2_y << "\n";
            in >> wi >> wa; // 读取width&way
            qDebug() << wi << " " << wa << "\n";
            in >> r >> g >> b >> a;
            qDebug() << r << " " << g << " " << b << " " << a << "\n";

            arrow.line = QLine(QPoint(p1_x,p1_y),QPoint(p2_x,p2_y));
            arrow.width = wi;
            arrow.way = wa;
            arrow.color = QColor(r, g, b, a);

            solid_arrows.append(arrow);
            shape.append(114);
        }

        in >> numArrows; // 读取多边形数量
        for (int i = 0; i < numArrows; ++i) {
            Arrow arrow;

            int p1_x, p1_y, p2_x, p2_y, wi, wa;
            int r, g, b, a;
            in >> p1_x >> p1_y >> p2_x >> p2_y; // 读取line
            qDebug() << p1_x << " " << p1_y << " " << p2_x << " " << p2_y << "\n";
            in >> wi >> wa; // 读取width&way
            qDebug() << wi << " " << wa << "\n";
            in >> r >> g >> b >> a;
            qDebug() << r << " " << g << " " << b << " " << a << "\n";

            arrow.line = QLine(QPoint(p1_x,p1_y),QPoint(p2_x,p2_y));
            arrow.width = wi;
            arrow.way = wa;
            arrow.color = QColor(r, g, b, a);

            dashed_arrows.append(arrow);
            shape.append(514);
        }

        file.close();
    }
};

#endif // SHAPEDRAWER_H
