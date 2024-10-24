// #include "mainwindow.h"
#include <QApplication>
#include <QWidget>
#include <cmath>
#include <vector>
#include "stdafx.h"
#include "point.h"
#include "bezier.h"
#include "tools.h"
#include "fill.h"
#include "line.h"
#include "polygon.h"
#include "arc.h"
#include "transmatrix.h"
//#include <QInputDialog>
//#include <QPainter>
//#include <QMouseEvent>
//#include <QComboBox>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
//#include <QSlider>
//#include <QLabel>
//#include <QPen>
using namespace std;

//TODO:���ƻ��ߵ� ��ʼ�� ���� ������ ����δ����

class ShapeDrawer : public QWidget {
    // Q_OBJECT;
private:
    // ȫ��
    DrawMode mode;              // ��ǰ����ģʽ��ֱ�� �� Բ��
    vector<vector<pointData>> MAP;
    int lineWidth = 5;          // �洢�������
    bool hasStartPoint = false; // �Ƿ������
    bool drawing = false;       // �Ƿ����ڻ��ƣ�������������ͷ�ʱ�Ķ�����
    QColor currentLineColor = Qt::black;    // ��ǰ������ɫ
    QVector<int> shape;          // ����ͼ�ε��ػ�˳�򣬷�ֹ˳����� 1.ֱ�� 2.Բ����Բ 3.����� 4.���ӵ� 5.Bezier 6.Bspline
    float XL = 0, XR = 800, YB = 0, YT = 550;
    Point _begin = Point(0, 0); // ��ק�Ĳο����꣬�������λ��
    bool ctr_or_not = false;	// Bezier��Bspline�Ƿ��ƶ����Ƶ�
    QLabel* coordLabel;			// ��갴��ʱ��ʾʵʱλ�õ����

    // ֱ�߶�
    QPoint startPoint;          // �߶ε����
    QPoint endPoint;            // �߶ε��յ�
    line_Algorithm line_algo = Midpoint;  // ��ǰѡ���ֱ�߶��㷨
    QVector<Line> lines;        // �洢�ѻ��Ƶ�ֱ�߶�

    // Բ��Բ��
    QPoint center;              // Բ�ģ�����Բ����
    int radius;                 // �뾶������Բ����
    int startAngle, endAngle;   // Բ������ʼ����ֹ�Ƕ�
    int counter = 0;            // �����������������0-2ѭ����
    QVector<Arc> arcs;          // �洢�ѻ��Ƶ�ֱ�߶�

    // �����
    QVector<Polygon> polygons; // �洢��������
    Polygon currentPolygon; // ��ǰ���ڻ��ƵĶ����
    vector<Fill> fills; // �洢��������
    QStack<Point> stack;
    Polygon* nowPolygon;

    // �ü�
    QPoint clipStartPoint;  // �ü����ڵ����
    QPoint clipEndPoint;    // �ü����ڵ��յ�
    clip_Algorithm  clip_algo = SutherlandTrim;// ��ǰѡ���ֱ�߶βü��㷨
    transMode trans_algo = MOVE;
    QVector<QPoint> _cropPolygon; // ���ж����

    // ���
    Fill nowFill;

    // ����
    bool isInTagRect = false; //�Ƿ��ڱ�־������
    bool isInPolygon = false;
    bool isInEllipse = false;
    bool isArrow = false;
    bool isInRect = false;
    bool isInFill = false;

    bool iscomfirm = false;
    bool isSpecificRefer = false; // �Ƿ���ָ���ı任��
    Polygon tempTransPoly;
    Point referancePoint;
    QRect* transRectTag = new QRect(100, 100, 20, 20);             //��ǩ����

    // Bezier����
    QVector<QVector<QPoint>> all_beziers;	// ���е�Bezier����
    QVector<QPoint> currentBezierControlPoints;	// �ݴ�ĵ�ǰBezier���߿��Ƶ�
    int isOnPoint1 = -1;	// ���ڸ��ٵ�ǰѡ�еĿ��Ƶ�
    int SelectedBezier = -1, SelectedPoint = -1;

    // Bspline����
    QVector<QVector<QPoint>> all_bsplines;	// ���е�Bspline����
    QVector<QPoint> currentBsplineControlPoints;	// ��ǰBspline���ߵĿ��Ƶ�����
    int last_k = 3;			// ��ǰBspline���ߵĽ���
    vector<int> k_steps;	// ÿ��Bspline���ߵĽ���
    int isOnPoint2 = -1;	// ���ڸ��ٵ�ǰѡ�еĿ��Ƶ�
    int SelectedBspline = -1;

protected:
    // ��д�����¼�
    void paintEvent(QPaintEvent* event) override
    {
        // ����ͼ�εļ����������ƴ�vector��ȡ����˳��
        int i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, i6 = 0;
        clearMAP(MAP);
        QPainter painter(this);

        // ÿ��ˢ�µ�ʱ�����»����Ѵ���ֱ��
        for (int c = 0; c < shape.size(); c++) {
            QPen pen;
            // �ػ�ֱ��
            if (shape.at(c) == 1 && lines.size() > i1) {
                const Line& line = lines.at(i1++);
                pen.setColor(line.colour);  // ����������ɫ(Ĭ�� black)
                pen.setWidth(line.width);   // ʹ�� width ����������ϸ
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
            // �ػ�Բ�� �� Բ
            else if (shape.at(c) == 2 && arcs.size() > i2) {
                const Arc& arc = arcs.at(i2++);
                pen.setColor(arc.colour);   // ����������ɫ(Ĭ�� black)
                pen.setWidth(arc.width);    // ʹ�� width ����������ϸ
                painter.setPen(pen);
                if (arc.startAngle <= arc.endAngle) {
                    drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, arc.endAngle);
                }
                else {
                    drawMidpointArc(painter, arc.center, arc.radius, arc.startAngle, 360);
                    drawMidpointArc(painter, arc.center, arc.radius, 0, arc.endAngle);
                }
            }
            // �ػ�����
            else if (shape.at(c) == 3 && polygons.size() > i3) {
                const Polygon& polygon = polygons.at(i3++);
                QPen pen;
                pen.setColor(polygon.color);  // ����������ɫ(Ĭ�� black)
                // pen.setWidth(lineWidth);		 // ʹ�� width ����������ϸ
                painter.setPen(pen);
                if (polygon.points.size() > 1) {
                    for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
                        drawDDALine(painter, QPoint(polygon.points[i].x, polygon.points[i].y), QPoint(polygon.points[i + 1].x, polygon.points[i + 1].y));
                    }
                }
                if (polygon.isClosed()) {
                    scanlineFill(painter, polygon);
                }
            }
            // �ػ��������
            else if (shape.at(c) == 4 && fills.size() > i4) {
                const Fill& fill = fills.at(i4++);
                QPen pen;
                pen.setColor(fill.color);  // ����������ɫ(Ĭ�� black)
                painter.setPen(pen);
                qDebug() << "In paintevent : " << fill.color << " " << Fill(fill).point.Getx() << " " << Fill(fill).point.Gety();
                fillShape(painter, fill.point, fill.color);
            }
            // �ػ�Bezier����
            else if (shape.at(c) == 5 && all_beziers.size() > i5) {
                painter.setPen(Qt::blue);
                const QVector<QPoint>& ControlPoints = all_beziers.at(i5++);
                // ����Beizer���Ƶ�
                if (ctr_or_not) {
                    for (const QPoint& point : ControlPoints) {
                        painter.drawEllipse(point, 5, 5);
                    }
                }
                // ����Bezier����
                if (ControlPoints.size() > 1) {
                    QPen bezierPen(currentLineColor, lineWidth);
                    Bezier bezier(1, painter, ControlPoints.toVector(), bezierPen);
                    bezier.drawBezier();
                }
            }
            // �ػ�Bspline����
            else if (shape.at(c) == 6 && all_bsplines.size() > i6) {
                int temp_k = k_steps[i6];
                painter.setPen(Qt::blue);
                const QVector<QPoint>& ControlPoints = all_bsplines.at(i6++);
                // ����Bspline���Ƶ�
                if (ctr_or_not) {
                    for (const QPoint& point : ControlPoints) {
                        painter.drawEllipse(point, 5, 5);
                    }
                    for (int j = 0; j < ControlPoints.size() - 1; ++j) {
                        // ����������ʽ
                        QPen dashedPen(Qt::blue, 1);
                        painter.setPen(dashedPen);
                        drawPreviewDash(painter, ControlPoints.at(j), ControlPoints.at(j + 1));
                    }
                }
                // ����Bspline����
                if (ControlPoints.size() > 1) {
                    QPen bsplinePen(currentLineColor, lineWidth);
                    Bspline bspline(painter, ControlPoints.toVector(), temp_k, bsplinePen);
                    bspline.drawBspline();
                }
            }
        }

        // ���Ƶ�ǰ���ڴ�����Bezier����
        if (mode == BezierMode && currentBezierControlPoints.size() > 1) {
            QPen bezierPen(currentLineColor, lineWidth);
            Bezier bezier(1, painter, currentBezierControlPoints.toVector(), bezierPen);
            bezier.drawBezier();
        }

        // ���Ƶ�ǰ���ڴ�����Beizer���Ƶ�
        if (mode == BezierMode) {
            painter.setPen(Qt::blue);
            for (const QPoint& point : currentBezierControlPoints) {
                painter.drawEllipse(point, 5, 5);
            }
        }

        // ���Ƶ�ǰ���ڴ�����Bspline����
        if (mode == BsplineMode && currentBsplineControlPoints.size() > 1) {
            QPen bsplinePen(currentLineColor, lineWidth);

            Bspline bspline(painter, currentBsplineControlPoints.toVector(), last_k, bsplinePen);
            bspline.drawBspline();
        }

        // ���Ƶ�ǰ���ڴ�����Bspline���Ƶ�
        if (mode == BsplineMode) {
            painter.setPen(Qt::blue);
            for (const QPoint& point : currentBsplineControlPoints) {
                painter.drawEllipse(point, 5, 5);
            }
        }

        // ���Ƶ�ǰ���ڴ����Ķ����
        if (currentPolygon.points.size() > 1) {
            for (size_t i = 0; i < currentPolygon.points.size() - 1; ++i) {
                painter.drawLine(currentPolygon.points[i].x, currentPolygon.points[i].y, currentPolygon.points[i + 1].x, currentPolygon.points[i + 1].y);
            }
        }

        // ���Ƶ�ǰ���ڴ����Ķ���βü�����
        if (_cropPolygon.size() > 1) {
            for (size_t i = 0; i < _cropPolygon.size() - 1; ++i) {
                painter.drawLine(_cropPolygon[i].x(), _cropPolygon[i].y(), _cropPolygon[i + 1].x(), _cropPolygon[i + 1].y());
            }
        }

        // �������㣬�����߶ε�Ԥ��
        if (hasStartPoint) {
            QPen pen;
            QColor temp_color = Qt::gray;   // ����Ԥ��������ɫ(��ɫ��
            pen.setColor(temp_color);
            pen.setWidth(lineWidth);        // ʹ�õ�ǰ���õ��������
            painter.setPen(pen);

            if (mode == LineMode) {
                if (line_algo == DashLine)
                    drawPreviewDash(painter, startPoint, endPoint);
                else
                    drawPreviewSolid(painter, startPoint, endPoint);
            }
            else if (mode == CircleMode) {
                drawMidpointArc(painter, center, radius, startAngle, endAngle);
            }
            else  if (mode == ArcMode) {
                if (startAngle <= endAngle)
                    drawMidpointArc(painter, center, radius, startAngle, endAngle);
                else {
                    drawMidpointArc(painter, center, radius, startAngle, 360);
                    drawMidpointArc(painter, center, radius, 0, endAngle);
                }
            }
            else if (mode == TrimMode && clip_algo != CropPolygon) {
                // ����Ԥ���û���
                pen.setWidth(2);
                painter.setPen(pen);
                // ��ȡԤ���ü����ο���ĸ�����
                QPoint top_left(min(clipStartPoint.x(), clipEndPoint.x()), max(clipStartPoint.y(), clipEndPoint.y()));
                QPoint top_right(max(clipStartPoint.x(), clipEndPoint.x()), max(clipStartPoint.y(), clipEndPoint.y()));
                QPoint bottom_left(min(clipStartPoint.x(), clipEndPoint.x()), min(clipStartPoint.y(), clipEndPoint.y()));
                QPoint bottom_right(max(clipStartPoint.x(), clipEndPoint.x()), min(clipStartPoint.y(), clipEndPoint.y()));
                // ����Ԥ�����ο�
                drawPreviewDash(painter, top_left, top_right);
                drawPreviewDash(painter, top_right, bottom_right);
                drawPreviewDash(painter, bottom_right, bottom_left);
                drawPreviewDash(painter, bottom_left, top_left);
            }
        }

        // ����������յ㣬����ʵ���߶�
        if (drawing) {
            QPen pen;
            pen.setColor(currentLineColor); // ʹ�õ�ǰ���õ���ɫ
            pen.setWidth(lineWidth);        // ʹ�õ�ǰ���õ��������
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
            }
            else if (mode == CircleMode) {
                drawMidpointArc(painter, center, radius, startAngle, endAngle);
            }
            else  if (mode == ArcMode) {
                if (startAngle <= endAngle)
                    drawMidpointArc(painter, center, radius, startAngle, endAngle);
                else {
                    drawMidpointArc(painter, center, radius, startAngle, 360);
                    drawMidpointArc(painter, center, radius, 0, endAngle);
                }
            }
        }
    }

    // �ж����Ƶ�ʱ���Ƿ���磨���MAP��
    // дfill��lyc��лл���ṩ���������
    // lyc�����ǣ����width��heightû��Ĭ�ϲ�����Ϊʲô���Լ�д�ж�
    // lty: �����Լ����ø�Ĭ�ϲ�������Լ����ַ�����ʳ������
    // lyc���������������Ϣ��
    // lyc�����Ҹ���
    bool checkLegalPos(int x, int y, int width = 800, int height = 550) {
        if (x >= 0 && x <= width && y >= 0 && y <= height) {
            return true;
        }
        else
            return false;
    }

    // �������ص��� MAP ��Ӧ����������
    void drawPixel(int x, int y, QPainter& painter) {
        //��ʹ�ñ�ˢ
        if (checkLegalPos(x, y, 800, 550)) {
            painter.drawPoint(x, y);
            // qDebug() << x << " " << y;
            MAP[x][y].setColor(painter.pen().color());
        }
    };

    // DDA �㷨ʵ��
    void drawDDALine(QPainter& painter, QPoint p1, QPoint p2) {
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

    // Bresenham �㷨ʵ��
    void drawBresenhamLine(QPainter& painter, QPoint p1, QPoint p2) {
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

    // �е��㷨ʵ��
    void drawMidpointLine(QPainter& painter, QPoint p1, QPoint p2) {
        // ��ȡ��Ϣ
        int x1 = p1.x();
        int y1 = p1.y();
        int x2 = p2.x();
        int y2 = p2.y();

        int x = x1, y = y1;	//����ʼ��
        int dy = y1 - y2, dx = x2 - x1;
        int delta_x = (dx >= 0 ? 1 : (dx = -dx, -1));	//��dx>0�򲽳�Ϊ1������Ϊ-1��ͬʱdx����
        int delta_y = (dy <= 0 ? 1 : (dy = -dy, -1));	//ע������dy<0,���ǻ�����y����������

        drawPixel(x, y, painter);		//����ʼ��

        int d, incrE, incrNE;
        if (-dy <= dx)		// б�ʾ���ֵ <= 1
        //����-dy�������е�dy
        {
            d = 2 * dy + dx;	//��ʼ���ж�ʽd
            incrE = 2 * dy;		//ȡ����Eʱ�б�ʽ����
            incrNE = 2 * (dy + dx);//NE
            while (x != x2)
            {
                if (d < 0)
                    y += delta_y, d += incrNE;
                else
                    d += incrE;
                x += delta_x;
                drawPixel(x, y, painter);
            }
        }
        else				// б�ʾ���ֵ > 1
        // x��y�������
        {
            d = 2 * dx + dy;
            incrE = 2 * dx;
            incrNE = 2 * (dy + dx);
            while (y != y2)
            {
                if (d < 0)	//ע��d�仯���
                    d += incrE;
                else
                    x += delta_x, d += incrNE;
                y += delta_y;
                drawPixel(x, y, painter);
            }
        }
    }

    // ���߻��� Bresenham �㷨ʵ��
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

        int dashLength = 8;   // ÿ�����ߵĳ���
        int gapLength = 8;    // ÿ�οհ׵ĳ���
        int totalLength = dashLength + gapLength;  // �����ڳ���
        int stepCount = 0;    // �������������ھ����Ƿ����

        while (x1 != x2 || y1 != y2) {
            // ֻ�����ߵĲ��ֻ��Ƶ�
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

            stepCount++;  // ���Ӳ���������
        }
    }

    // �е�Բ���㷨ʵ��
    void drawMidpointArc(QPainter& painter, QPoint center, int radius, int startAngle, int endAngle) {
        // ���Ƕ�ת��Ϊ����
        float startRad = startAngle * M_PI / 180.0;
        float endRad = endAngle * M_PI / 180.0;

        int x = radius;
        int y = 0;
        int d = 1 - radius;

        drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);

        while (x > y) {
            if (d < 0) {
                d += 2 * y + 3;
            }
            else {
                d += 2 * (y - x) + 5;
                --x;
            }
            ++y;
            drawSymmetricPointsArc(painter, center, x, y, startRad, endRad);
        }
    }

    // ���ƶԳƵ㣬ֻ�����ڽǶȷ�Χ�ڵĵ�
    void drawSymmetricPointsArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad) {
        drawPointInArc(painter, center, x, y, startRad, endRad);
        drawPointInArc(painter, center, -x, y, startRad, endRad);
        drawPointInArc(painter, center, x, -y, startRad, endRad);
        drawPointInArc(painter, center, -x, -y, startRad, endRad);
        drawPointInArc(painter, center, y, x, startRad, endRad);
        drawPointInArc(painter, center, -y, x, startRad, endRad);
        drawPointInArc(painter, center, y, -x, startRad, endRad);
        drawPointInArc(painter, center, -y, -x, startRad, endRad);
    }

    // �жϵ��Ƿ���Բ���Ƕȷ�Χ�ڲ�����
    void drawPointInArc(QPainter& painter, QPoint center, int x, int y, float startRad, float endRad) {
        float angle = atan2(y, x);
        if (angle < 0) angle += 2 * M_PI;
        if (angle >= startRad && angle <= endRad) {
            drawPixel(center.x() + x, center.y() + y, painter);
        }
    }

    // ɨ�����㷨
    void scanlineFill(QPainter& painter, const Polygon& polygon) {
        if (polygon.points.size() < 3) return;

        // ������С����� y ����
        int ymin = polygon.points[0].y, ymax = polygon.points[0].y;
        for (const Point& p : polygon.points) {
            ymin = min(ymin, p.y);
            ymax = max(ymax, p.y);
        }

        // ��ÿ��ɨ���ߴ� ymin �� ymax ���д���
        for (int y = ymin; y <= ymax; ++y) {
            vector<int> intersections;

            // ����ÿ�����뵱ǰɨ���ߵĽ���
            for (size_t i = 0; i < polygon.points.size() - 1; ++i) {
                const Point& p1 = polygon.points[i];
                const Point& p2 = polygon.points[i + 1];

                if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
                    int x = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                    intersections.push_back(x);
                }
            }

            // �Խ����������
            sort(intersections.begin(), intersections.end());

            // ��������֮����߶�(Ԥ���棬��ʹ��Qt�ĺ���)
            for (size_t i = 0; i < intersections.size(); i += 2) {
                painter.drawLine(intersections[i], y, intersections[i + 1], y);
            }
        }
    }

    // ���ӵ�����㷨
    void fillShape(QPainter& painter, Point point, QColor newColor) {
        //�����λ�õ���ɫ����Ϊ��Ҫ�滻����ɫ
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
                while (MAP[x][y].getColor() != oldColor && x < xr)x++;
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
                while (MAP[x][y].getColor() != oldColor && x < xr)x++;
            }
        }
    }

    // Sutherland �����
    OutCode computeOutCode(double x, double y, double xmin, double ymin, double xmax, double ymax) {
        OutCode code = INSIDE;

        if (x < xmin) code = OutCode(code | LEFT);
        else if (x > xmax) code = OutCode(code | RIGHT);
        if (y < ymin) code = OutCode(code | BOTTOM);
        else if (y > ymax) code = OutCode(code | TOP);

        return code;
    }

    // Sutherland �ü�ֱ�߶��㷨
    bool cohenSutherlandClip(QLineF& line, double xmin, double ymin, double xmax, double ymax) {
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
            }
            else if (outcode0 & outcode1) {
                // Both endpoints outside rectangle
                break;
            }
            else {
                // Some segment of the line lies within the rectangle
                double x, y;
                // Pick an endpoint that is outside the rectangle
                OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

                if (outcodeOut & TOP) {          // point above the clip rectangle
                    x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                    y = ymax;
                }
                else if (outcodeOut & BOTTOM) { // point below the clip rectangle
                    x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                    y = ymin;
                }
                else if (outcodeOut & RIGHT) {  // point to the right of the clip rectangle
                    y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                    x = xmax;
                }
                else if (outcodeOut & LEFT) {   // point to the left of the clip rectangle
                    y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                    x = xmin;
                }

                if (outcodeOut == outcode0) {
                    x0 = x;
                    y0 = y;
                    outcode0 = computeOutCode(x0, y0, xmin, ymin, xmax, ymax);
                }
                else {
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

    // �е��㷨�ü�ֱ�߶�
    bool liangBarskyClip(QLineF& line, double xmin, double ymin, double xmax, double ymax) {
        double x0 = line.x1(), y0 = line.y1();
        double x1 = line.x2(), y1 = line.y2();

        double dx = x1 - x0;
        double dy = y1 - y0;

        double t0 = 0.0, t1 = 1.0;
        double p[] = { -dx, dx, -dy, dy };
        double q[] = { x0 - xmin, xmax - x0, y0 - ymin, ymax - y0 };

        for (int i = 0; i < 4; i++) {
            if (p[i] == 0 && q[i] < 0) {
                // Line is parallel and outside the boundary
                return false;
            }

            double r = q[i] / p[i];
            if (p[i] < 0) {
                if (r > t1) return false;
                else if (r > t0) t0 = r;
            }
            else if (p[i] > 0) {
                if (r < t0) return false;
                else if (r < t1) t1 = r;
            }
        }

        if (t0 > t1) return false;

        line.setP1(QPointF(x0 + t0 * dx, y0 + t0 * dy));
        line.setP2(QPointF(x0 + t1 * dx, y0 + t1 * dy));
        return true;
    }

    // ���������ĵ㣬������ת
    Point getPolyCenter(vector<Point> Poly) {
        Point p0 = Poly[0];
        Point p1 = Poly[1];
        Point p2;
        int Center_X, Center_Y;
        double sumarea = 0, sumx = 0, sumy = 0;
        double area;
        for (int i = 2; i < Poly.size(); i++) {
            p2 = Poly[i];
            area = p0.Getx() * p1.Gety() + p1.Getx() * p2.Gety() + p2.Getx() * p0.Gety() - p1.Getx() * p0.Gety() - p2.Getx() * p1.Gety() - p0.Getx() * p2.Gety();
            area /= 2;
            sumarea += area;
            sumx += (p0.Getx() + p1.Getx() + p2.Getx()) * area; //���cx[i] * s[i]�͡�cy[i] * s[i]
            sumy += (p0.Gety() + p1.Gety() + p2.Gety()) * area;
            p1 = p2;//ת��Ϊ��һ�������Σ��������
        }
        Center_X = (int)(sumx / sumarea / 3.0);
        Center_Y = (int)(sumy / sumarea / 3.0);
        qDebug() << Center_X;
        qDebug() << Center_Y;
        return Point(Center_X, Center_Y);
    }

    // ����OnSegment����
    int crossProduct(Point A, Point B, Point C) {
        return (B.Getx() - A.Getx()) * (C.Gety() - A.Gety()) - (C.Getx() - A.Getx()) * (B.Gety() - A.Gety());
    }

    // ����OnSegment����
    int dotProduct(Point p1, Point p2) {
        return p1.Getx() * p2.Getx() + p1.Gety() * p2.Gety();
    }

    //�жϵ�Q�Ƿ���P1��P2���߶���
    bool OnSegment(Point P1, Point P2, Point Q) {
        //ǰһ���жϵ�Q��P1P2ֱ���� ��һ���ж���P1P2��Χ��
        //QP1 X QP2
        return crossProduct(Q, P1, P2) == 0 && dotProduct(P1 - Q, P2 - Q) <= 0;
    }

    // �����������һ��ԭ�㣬����������ԭ�㹹�ɵļн�
    double getAngle(QPoint origin, QPoint p1, QPoint p2) {
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

    // �����������һ��ԭ�㣬����������ԭ�㹹�ɵļн�
    double getAngle(Point origin, Point p1, Point p2) {
        int x1 = p1.Getx(), y1 = p1.Gety(), x2 = p2.Getx(), y2 = p2.Gety(), x3 = origin.Getx(), y3 = origin.Gety();
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

    // ����p���Ƿ��ڶ�����ڲ�
    bool polyContains(vector<Point> polygon, Point P) {
        bool flag = false; //�൱�ڼ���
        Point P1, P2; //�����һ���ߵ���������
        for (int i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
            //polygon[]�Ǹ�������εĶ���
            P1 = polygon[i];
            P2 = polygon[j];
            if (OnSegment(P1, P2, P)) return true; //���ڶ����һ������
            if (((P1.Gety() - P.Gety()) > 0 != (P2.Gety() - P.Gety()) > 0) &&
                (P.Getx() - (P.Gety() - P1.Gety()) * (P1.Getx() - P2.Getx()) / (P1.Gety() - P2.Gety()) - P1.Getx()) < 0)
                flag = !flag;
        }
        qDebug() << flag;
        return flag;
    }

    // ʹ�ñ任������ж���α任
    void polygonTrans(QMouseEvent* e) {
        transMatrix trM;
        double zoomPropor_X, zoomPropor_Y;  //�������ŵı���
        Point moveVector;              //�����ƶ�������
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
            moveVector = Point(e->pos()) - _begin;    //�����ƶ��������յ�����
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
                zoomPropor_X =
                    abs(e->pos().x() - referancePoint.Getx()) * 1.0 / abs(tempTransPoly.points[0].Getx() - referancePoint.Getx());
                zoomPropor_Y =
                    abs(e->pos().y() - referancePoint.Gety()) * 1.0 / abs(tempTransPoly.points[0].Gety() - referancePoint.Gety());
                trM.setZoomTrans(zoomPropor_X, zoomPropor_Y); //�������ű任����
                for (int i = 0; i < (*nowPolygon).points.size(); ++i) {
                    //(*nowPolygon)[i] = trM * tempTransPoly[i];
                    (*nowPolygon).points[i] = trM * tempTransPoly.points[i];
                }
                if (isInFill) {
                    trM.setMoveTrans(getPolyCenter((*nowPolygon).points) - nowFill.point);
                    nowFill.point = trM * nowFill.point;
                }
                trM.setMoveTrans((*nowPolygon).points[0] - transRectTag->center()); //�ñ�־�������ĵ�����ƶ����µ�
                QPoint leftTop = QPoint((trM * (transRectTag->topLeft())).Getx(), (trM * (transRectTag->topLeft())).Gety());
                transRectTag->setTopLeft(leftTop);
                //transRectTag->setBottomRight(trM * (transRectTag->bottomRight()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(), (trM * (transRectTag->bottomRight())).Gety()));
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
                if (angle > 30 || angle < -30)   /**��תʱ������ת����60���ʱ�����bug��Ŀǰԭ���в���ȷ��������Ϊÿ�γ���30��ʱ�����ݴ����Σ����´εĽǶȴ������**/
                    iscomfirm = true;     /**����ɾ��������������������������**/
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
                trM.setMoveTrans((*nowPolygon).points[0] - transRectTag->center()); //�ñ�־�������ĵ�����ƶ����µ�
                //transRectTag->setTopLeft(trM * (transRectTag->topLeft()));
                //transRectTag->setBottomRight(trM * (transRectTag->bottomRight()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->topLeft())).Getx(), (trM * (transRectTag->topLeft())).Gety()));
                transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(), (trM * (transRectTag->bottomRight())).Gety()));
                _begin = e->pos();
                update();
            }
        }
    }

    // �����Ƿ��ڶ������
    bool outsideOneEdgeOfPolygon(QVector<QPoint> polygon, QPoint p, int x) {
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
        }
        else {
            return false;
        }
    }

    // �жϵ�p�Ƿ�λ�ڲü���p1-p2���ڲࣨ���͹����Σ�
    bool isInside(QPoint p, QPoint p1, QPoint p2) {
        return (p2.x() - p1.x()) * (p.y() - p1.y()) - (p2.y() - p1.y()) * (p.x() - p1.x()) <= 0;
    }

    // ���������߶εĽ��㣬���ƽ���򷵻�QPoint(-1, -1)
    QPoint intersection(QPoint p1, QPoint p2, QPoint k1, QPoint k2) {
        double a1 = p2.y() - p1.y();
        double b1 = p1.x() - p2.x();
        double c1 = a1 * p1.x() + b1 * p1.y();

        double a2 = k2.y() - k1.y();
        double b2 = k1.x() - k2.x();
        double c2 = a2 * k1.x() + b2 * k1.y();

        double determinant = a1 * b2 - a2 * b1;

        if (std::abs(determinant) < 1e-10) {  // ����Ƿ�ƽ��
            return QPoint(-1, -1); // ƽ����û�н���
        }
        else {
            double x = (b2 * c1 - b1 * c2) / determinant;
            double y = (a1 * c2 - a2 * c1) / determinant;
            return QPoint(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)));
        }
    }

    // ���ݸ�������βü����ж����
    QVector<QPoint> cropPolygon(const QVector<QPoint>& subjectPolygon, const QVector<QPoint>& clipPolygon) {
        QVector<QPoint> output = subjectPolygon;

        for (int i = 0; i < clipPolygon.size(); ++i) {
            QVector<QPoint> input = output;
            output.clear();

            // ��ǰ�ü��ߵ���������
            QPoint clipEdgeStart = clipPolygon[i];
            QPoint clipEdgeEnd = clipPolygon[(i + 1) % clipPolygon.size()];

            for (int j = 0; j < input.size(); ++j) {
                // ���ü�����ε�ǰ�ߵ���������
                QPoint currentPoint = input[j];
                QPoint previousPoint = input[(j - 1 + input.size()) % input.size()];

                bool currentInside = isInside(currentPoint, clipEdgeStart, clipEdgeEnd);
                bool previousInside = isInside(previousPoint, clipEdgeStart, clipEdgeEnd);

                if (currentInside) {
                    if (!previousInside) {
                        // ������ⲿ���룬���㽻��
                        QPoint inter = intersection(previousPoint, currentPoint, clipEdgeStart, clipEdgeEnd);
                        output.append(inter); // ��ӽ���
                    }
                    // ��ǰ�����ڲ࣬�����õ�
                    output.append(currentPoint);
                }
                else if (previousInside) {
                    // ������ڲ��뿪�����㽻��
                    QPoint inter = intersection(previousPoint, currentPoint, clipEdgeStart, clipEdgeEnd);
                    output.append(inter); // ��ӽ���
                }
            }
        }

        return output;
    }

    // ��������ƶ��¼�
    void mouseMoveEvent(QMouseEvent* event) override {
        // ��ȡ���ľֲ����꣨����ڴ��ڣ�
        QPoint localPos = event->pos();

        // ���±�ǩ��ʾ���������
        coordLabel->setText(QString("(%1, %2)")
                                .arg(localPos.x())
                                .arg(localPos.y()));

        // ��λ��ǰ�����ָ�Ķ����
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
            setCursor(Qt::SizeAllCursor);//��קģʽ�£���������קͼ���У��������״��Ϊʮ��
            if (!(*nowPolygon).points.empty()) {
                polygonTrans(event);
                update();
            }
        }
        else {
            setCursor(Qt::ArrowCursor);//�ָ�ԭʼ�����״
        }

        if (isInPolygon) {
            transMatrix trM;
            QPoint tem = QPoint((*nowPolygon).points[0].Getx(), (*nowPolygon).points[0].Gety());
            trM.setMoveTrans(tem - transRectTag->center());
            transRectTag->setBottomRight(QPoint((trM * (transRectTag->topLeft())).Getx(), (trM * (transRectTag->topLeft())).Gety()));
            transRectTag->setBottomRight(QPoint((trM * (transRectTag->bottomRight())).Getx(), (trM * (transRectTag->bottomRight())).Gety()));
        }

        if (hasStartPoint)
        {
            if (mode == LineMode) {
                // �����յ�Ϊ��ǰ���λ��
                endPoint = event->pos();
            }
            else if (mode == CircleMode) {
                QPoint currentPos = event->pos();
                radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2) + std::pow(currentPos.y() - center.y(), 2));
            }
            else if (mode == ArcMode && counter == 0) {
                return;
            }
            else if (mode == ArcMode && counter == 1) {
                QPoint currentPos = event->pos();
                radius = std::sqrt(std::pow(currentPos.x() - center.x(), 2) + std::pow(currentPos.y() - center.y(), 2));

                // ���㵱ǰ���λ����Բ�ĵĽǶ�
                float angleRad = atan2(currentPos.y() - center.y(), currentPos.x() - center.x());
                endAngle = static_cast<int>(angleRad * 180.0 / M_PI); // ת��Ϊ�Ƕ�

                if (endAngle < 0) {
                    endAngle += 360;  // ȷ���Ƕ�Ϊ��
                }
            }
            else if (mode == TrimMode) {
                // ���else ifΪʲôҪ���� ArcMode ���棿����
                // �����浱Ȼ�ò����ػ���
                clipEndPoint = event->pos();
            }
            else if (mode == BezierMode && isOnPoint1 != -1 && !ctr_or_not) {
                currentBezierControlPoints[isOnPoint1] = event->pos(); // ���µ�ǰBezier���߿��Ƶ�λ��
            }
            else if (mode == BezierMode && SelectedBezier != -1 && SelectedPoint != -1 && ctr_or_not) {
                all_beziers[SelectedBezier][SelectedPoint] = event->pos();// ��������Bezier���߿��Ƶ�λ��
            }
            else if (mode == BsplineMode && isOnPoint2 != -1 && !ctr_or_not) {
                currentBsplineControlPoints[isOnPoint2] = event->pos(); // ���µ�ǰBezier���߿��Ƶ�λ��
            }
            else if (mode == BsplineMode && SelectedBspline != -1 && SelectedPoint != -1 && ctr_or_not) {
                all_bsplines[SelectedBspline][SelectedPoint] = event->pos();// ��������Bezier���߿��Ƶ�λ��
            }

            update(); // �����ػ�
        }
    }

    // ������갴���¼�
    void mousePressEvent(QMouseEvent* event) override {
        setCursor(Qt::ArrowCursor);//�ָ�ԭʼ�����״
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
                // ��һ�ε������¼���
                startPoint = event->pos();
                endPoint = startPoint;      // ��ʼ���յ�Ϊ���
                shape.push_back(1);
            }
            else if (mode == CircleMode) {
                center = event->pos();      // ��¼Բ��
                hasStartPoint = true;
                shape.push_back(2);
            }
            else if (mode == ArcMode && counter == 0) {
                center = event->pos();      // ��¼Բ��
                shape.push_back(2);
            }
            else if (mode == ArcMode && counter == 1) {
                // ���㵱ǰ���λ����Բ�ĵĽǶ�
                float angleRad = atan2(event->pos().y() - center.y(), event->pos().x() - center.x());
                startAngle = static_cast<int>(angleRad * 180.0 / M_PI); // ת��Ϊ�Ƕ�
                if (startAngle < 0) {
                    startAngle += 360;  // ȷ���Ƕ�Ϊ��
                }
            }
            else if (mode == PolygonMode) {
                // ��ȡ�������λ��
                int x = event->pos().x();
                int y = event->pos().y();
                currentPolygon.addPoint(Point(x, y)); // ��Ӷ���
                currentPolygon.color = currentLineColor; // �޸Ķ������ɫ

                update(); // ���������ػ�
            }
            else if (mode == FillMode) {
                fills.push_back(Fill(Point(event->pos().x(), event->pos().y()), currentLineColor));
                shape.push_back(4);
            }
            else if (mode == TrimMode && clip_algo != CropPolygon) {
                clipStartPoint = event->pos();  // ��¼��갴�µ�λ����Ϊ���
                clipEndPoint = clipStartPoint;
            }
            else if (mode == TrimMode && clip_algo == CropPolygon) {
                QPoint a;
                a.setX(event->pos().x());
                a.setY(event->pos().y());
                _cropPolygon.append(a);
                update();
            }
            else if (mode == BezierMode && !ctr_or_not) {
                QPoint pos = event->pos();
                // ����Ƿ��������еĿ��Ƶ㸽��
                bool pointSelected = false;
                for (int i = 0; i < currentBezierControlPoints.size(); ++i) {
                    if ((pos - currentBezierControlPoints[i]).manhattanLength() < 10) {
                        isOnPoint1 = i;  // ѡ�����еĵ�����϶�
                        pointSelected = true;
                        break;
                    }
                }
                if (!pointSelected) {
                    // ����µĿ��Ƶ�
                    currentBezierControlPoints.append(pos);
                    isOnPoint1 = -1; // ��ѡ���µ�
                }
                update(); // ���»�ͼ
            }
            else if (mode == BezierMode && ctr_or_not) {
                QPoint pos = event->pos();
                // ����Ƿ��������еĿ��Ƶ㸽��
                bool pointSelected = false;
                for (int i = 0; i < all_beziers.size(); ++i) {
                    for (int j = 0; j < all_beziers[i].size(); j++) {
                        if ((pos - all_beziers[i][j]).manhattanLength() < 10) {
                            SelectedBezier = i;  // ѡ�����еĵ�����϶�
                            SelectedPoint = j;
                            pointSelected = true;
                            break;
                        }
                    }
                }
                if (!pointSelected) {
                    SelectedBezier = -1; // ��ѡ���µ�
                    SelectedPoint = -1;
                }
            }
            else if (mode == BsplineMode && !ctr_or_not) {
                QPoint pos = event->pos();
                // ������л��Ƶ�
                bool pointSelected = false;
                for (int i = 0; i < currentBsplineControlPoints.size(); ++i) {
                    if ((pos - currentBsplineControlPoints[i]).manhattanLength() < 10) {
                        isOnPoint2 = i;  // ѡ�����еĵ�����϶�
                        pointSelected = true;
                        break;
                    }
                }
                if (!pointSelected) {
                    // ����µĿ��Ƶ�
                    currentBsplineControlPoints.append(pos);
                    qDebug() << "x:" << event->pos().x() << "y:" << event->pos().y() << "\n";
                    isOnPoint2 = -1; // ��ѡ���µ�
                }
                update(); // ���»�ͼ
            }
            else if (mode == BsplineMode && ctr_or_not) {
                QPoint pos = event->pos();
                // ����Ƿ��������еĿ��Ƶ㸽��
                bool pointSelected = false;
                for (int i = 0; i < all_bsplines.size(); ++i) {
                    for (int j = 0; j < all_bsplines[i].size(); j++) {
                        if ((pos - all_bsplines[i][j]).manhattanLength() < 10) {
                            SelectedBspline = i;  // ѡ�����еĵ�����϶�
                            SelectedPoint = j;
                            pointSelected = true;
                            break;
                        }
                    }
                }
                if (!pointSelected) {
                    SelectedBspline = -1; // ��ѡ���µ�
                    SelectedPoint = -1;
                }
            }

            hasStartPoint = true;
            drawing = false; // ��ʼ��Ϊ������ʵ��ֱ��
        }
    }

    // ��������ɿ��¼�
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (hasStartPoint)
        {
            if (mode == LineMode) {
                endPoint = event->pos();  // ֱ��ģʽ�£��ɿ��趨�յ�
                lines.append(Line(startPoint, endPoint, lineWidth, currentLineColor, line_algo));
            }
            else if (mode == CircleMode) {
                endPoint = event->pos();  // Բģʽ�£�����뾶
                radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
                arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
            }
            else if (mode == ArcMode && counter == 0) {
                counter += 1;
            }
            else if (mode == ArcMode && counter == 1) {
                endPoint = event->pos();  // Բ��ģʽ�£�����뾶
                radius = std::sqrt(std::pow(endPoint.x() - center.x(), 2) + std::pow(endPoint.y() - center.y(), 2));
                arcs.append(Arc(center, radius, startAngle, endAngle, lineWidth, currentLineColor));
                counter = 0;
            }
            else if (mode == TrimMode && clip_algo == SutherlandTrim || mode == TrimMode && clip_algo == MidTrim) {
                // drawingrect = false;
                qDebug() << "Trim the line.\n";
                clipEndPoint = event->pos();
                // ȷ���ü����ε����ϡ�������������
                double xmin = min(clipStartPoint.x(), clipEndPoint.x());
                double ymin = min(clipStartPoint.y(), clipEndPoint.y());
                double xmax = max(clipStartPoint.x(), clipEndPoint.x());
                double ymax = max(clipStartPoint.y(), clipEndPoint.y());

                // �����ڲü����ڵ�ֱ�ߵ���ʼ������˵�
                for (Line& line : lines) {
                    QLineF lineF(line.line);
                    if (clip_algo == SutherlandTrim) {
                        cohenSutherlandClip(lineF, xmin, ymin, xmax, ymax);
                    }
                    else if (clip_algo == MidTrim) {
                        liangBarskyClip(lineF, xmin, ymin, xmax, ymax);
                    }
                    line.line.setP1(lineF.p1().toPoint());
                    line.line.setP2(lineF.p2().toPoint());
                }
            }
            else if (mode == TransMode) {
                iscomfirm = true;
            }
            else if (mode == BezierMode) {
                isOnPoint1 = -1;  // Deselect control point
            }
            else if (mode == BsplineMode) {
                isOnPoint2 = -1;  // Deselect control point
            }

            drawing = true;         // �������
            update();               // �����ػ�
            hasStartPoint = false;  // ���ã������ٴλ����µ��߶�
            drawing = false;
        }
    }

    // ����˫������¼�
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        // ˫���¼���յ�ǰ�����
        if (mode == PolygonMode) {
            if (currentPolygon.points.size() > 2) {
                currentPolygon.closePolygon(); // ��յ�ǰ�����
                shape.push_back(3);
                polygons.push_back(currentPolygon); // ���浽������б�
                currentPolygon = Polygon(); // ���õ�ǰ������Կ�ʼ�µĻ���
                update();
            }
        }
        // ˫���¼���յ�ǰ����βü�����
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
                        // ������صĶ���εĳ��ȴ��ڵ���3����˵�����к�Ķ���β�Ϊ�գ�������׷�ӵ��µĶ����������
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

    // �������¼�
    void keyPressEvent(QKeyEvent* event) override {
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
                switch (shape.last())
                {
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
                }
                shape.pop_back();
            }
        }
        update();
    }

    void keyReleaseEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Control)
        {
            ctr_or_not = false;
            qDebug() << "ctr_or_not" << ctr_or_not << "\n";
        }
        update();
    }

public:
    ShapeDrawer(QWidget* parent = nullptr) : QWidget(parent), mode(LineMode), hasStartPoint(false), drawing(false),
        radius(0), startAngle(0), endAngle(360)
    {
        // ���ñ�����ɫ
        QPalette pal = this->palette();
        pal.setColor(QPalette::Window, Qt::white); // �������ñ�����ɫΪ��ɫ
        this->setAutoFillBackground(true);  // �����Զ���䱳��
        this->setPalette(pal);

        // ��ͼ���ڴ�С��
        setFixedSize(800, 550);
        initMAP(MAP);

        // Ϊ�ı߻�������ֱ�ߣ���ֹ������
        std::vector<vector<int>> tem = { {0,0},{799,0},{799,549}, {0,549},{0,0} };
        for (int i = 0; i < 4; ++i) {
            lines.append(Line(QPoint(tem[i][0], tem[i][1]), QPoint(tem[i + 1][0], tem[i + 1][1]), 1, Qt::black, Midpoint));
            shape.append(1);
        }

        // ȷ�����ڲ������Խ��ռ��̽���
        setFocusPolicy(Qt::StrongFocus);

        // ����һ����ǩ������ʾ�������

        coordLabel = new QLabel("(0, 0)", this);
        coordLabel->setMinimumWidth(250);
        QStatusBar* statusBar = new QStatusBar(this);
        statusBar->addWidget(coordLabel);
        statusBar->setGeometry(0, 525, 600, 20);
        statusBar->setStyleSheet("border: solid black 2px;");

        // �������׷�٣�����ֻ�ڰ��°�ťʱ�Ż�׷������ƶ���
        // setMouseTracking(true);
    }

    // ���õ�ǰ��ͼģʽ
    void setDrawMode(DrawMode newMode) {
        mode = newMode;
        update();  // ģʽ�л������»���
    }

    // ����������������ȵĺ���
    void setLineWidth(int width) {
        lineWidth = width;
        update();  // ÿ�θı���ʱ���������»���
    }

    //  ���������õ�ǰ������ɫ
    void setCurrentLineColor(QColor color) {
        currentLineColor = color;
        update();
    }

    //  ���������õ�ǰ�㷨
    void setAlgorithm(line_Algorithm algo)
    {
        line_algo = algo;
        // update(); // �ı��㷨�����»���
    }

    // �������òü�ֱ�߶��㷨
    void setclipAlgorithm(clip_Algorithm algo)
    {
        clip_algo = algo;
    }

    // �������ñ���ģʽ
    void setTransAlgorithm(transMode algo)
    {
        trans_algo = algo;
    }

    // ����������Բ������ʼ�ǶȺͽ����Ƕ�
    void setArcAngles(int start, int end) {
        startAngle = start;
        endAngle = end;
        update();  // �����ػ�
    }

    // ����������B�����߽���
    void setk_step(int k_value) {
        last_k = k_value;
        //k_steps.push_back(k_value);
        update();
    }

    // ��������ջ�������
    void Clear()
    {
        shape.clear();
        lines.clear();
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
};

class MainWindow : public QWidget {
    // Q_OBJECT

private:
    ShapeDrawer* shapeDrawer;       // ���������״������
    QComboBox* modeComboBox;        // ����ѡ�����ģʽ����Ͽ�
    QComboBox* line_algorithmComboBox;   // ������ֱ���㷨 ѡ��ť
    QComboBox* clip_algorithmComboBox;	// ������ֱ�߶βü��㷨 ѡ��ť
    QSlider* widthSlider;           // ����������������� ������
    QPushButton* colorButton;       // ��������ɫ ѡ��ť
    QComboBox* lineTypeComboBox;    // ���������� ѡ��ť
    QComboBox* transModeComboBox;    // ���������� ѡ��ť
    QPushButton* clearButton;		// �����������ť

public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
        shapeDrawer = new ShapeDrawer(this); // ������ͼ����

        // ����ѡ���ͼģʽ����Ͽ�
        modeComboBox = new QComboBox(this);
        modeComboBox->addItem("Line", LineMode);
        modeComboBox->addItem("Circle", CircleMode);
        modeComboBox->addItem("Arc", ArcMode);
        modeComboBox->addItem("Shape", PolygonMode);
        modeComboBox->addItem("Fill", FillMode);
        modeComboBox->addItem("Trim", TrimMode);
        modeComboBox->addItem("Transform", TransMode);
        modeComboBox->addItem("Bezier Curve", BezierMode);
        modeComboBox->addItem("Bspline Curve", BsplineMode);

        // ��������������㷨ѡ��
        line_algorithmComboBox = new QComboBox(this);
        line_algorithmComboBox->addItem("Midpoint", Midpoint);
        line_algorithmComboBox->addItem("Bresenham", Bresenham);
        line_algorithmComboBox->addItem("DDA", DDA);
        line_algorithmComboBox->addItem("DashLine", DashLine);

        // ������������Ӳü��㷨ѡ��
        clip_algorithmComboBox = new QComboBox(this);
        clip_algorithmComboBox->addItem("Cohen-Sutherland", SutherlandTrim);
        clip_algorithmComboBox->addItem("Midpoint Trim", MidTrim);
        clip_algorithmComboBox->addItem("Trim Polygon", CropPolygon);

        // ������������ӱ���ѡ��
        transModeComboBox = new QComboBox(this);
        transModeComboBox->addItem("Move", MOVE);
        transModeComboBox->addItem("Zoom", ZOOM);
        transModeComboBox->addItem("Rotate", ROTATE);

        // ���������������������������
        widthSlider = new QSlider(Qt::Horizontal, this);
        widthSlider->setRange(1, 15);  // ����������ȷ�ΧΪ 1 �� 15 ����
        widthSlider->setValue(5);      // ��ʼֵΪ 5 ����

        // ������ɫѡ��ť
        colorButton = new QPushButton("Choose Painter Color", this);

        // ������ջ�����ť
        clearButton = new QPushButton("Clear", this);

        // ˮƽ���ֹ��������˴���ʱֻ���Ҳࣩ
        QVBoxLayout* rightLayout = new QVBoxLayout();
        rightLayout->addWidget(new QLabel("Select Painting Mode:"));
        rightLayout->addWidget(modeComboBox);       // ����������ģʽѡ��
        rightLayout->addWidget(new QLabel("Select Line Mode:"));
        rightLayout->addWidget(line_algorithmComboBox);  // ������ֱ�߶��㷨ѡ��
        rightLayout->addWidget(new QLabel("Select Clip Mode:"));
        rightLayout->addWidget(clip_algorithmComboBox);  // ������ֱ�߶βü��㷨ѡ��
        rightLayout->addWidget(new QLabel("Select Transform Mode:"));
        rightLayout->addWidget(transModeComboBox);		// ������ͼ�α任ģʽѡ��
        rightLayout->addWidget(new QLabel("Select Line Width:"));
        rightLayout->addWidget(widthSlider);        // ����������������ӵ��Ҳ಼��
        rightLayout->addWidget(colorButton);        // ����������ɫ��ť��ӵ�����
        rightLayout->addStretch();
        rightLayout->addWidget(clearButton);

        // ��ֱ���ֹ�����
        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        mainLayout->addWidget(shapeDrawer);
        mainLayout->addLayout(rightLayout);

        // �����������ѡ��仯�ź�
        connect(line_algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
            shapeDrawer->setAlgorithm(static_cast<line_Algorithm>(line_algorithmComboBox->currentData().toInt()));
        });

        // �����������ѡ��仯�ź�
        connect(clip_algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
            shapeDrawer->setclipAlgorithm(static_cast<clip_Algorithm>(clip_algorithmComboBox->currentData().toInt()));
        });

        // �����������ѡ��仯�ź�
        connect(transModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
            shapeDrawer->setTransAlgorithm(static_cast<transMode>(transModeComboBox->currentData().toInt()));
        });

        // ���ӻ�������ֵ�仯�źŵ� shapeDrawer �� setLineWidth ����
        connect(widthSlider, &QSlider::valueChanged, shapeDrawer, &ShapeDrawer::setLineWidth);

        // ������ɫѡ��ť������ɫѡ����
        connect(colorButton, &QPushButton::clicked, [=]() {
            QColor color = QColorDialog::getColor(Qt::black, this, "Choose Line Color");
            if (color.isValid()) {
                shapeDrawer->setCurrentLineColor(color);
            }
        });

        // ������Ͽ��ѡ��仯�ź�
        // ��ѡ��ģʽ�ı�ʱ�����¼�
        connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            DrawMode selectedMode = static_cast<DrawMode>(modeComboBox->itemData(index).toInt());
            shapeDrawer->setDrawMode(selectedMode);
            if (selectedMode == CircleMode) {
                // �����Ի���������ʼ�ǶȺͽ����Ƕ�
                bool ok;
                int startAngle = QInputDialog::getInt(this, tr("Set Start Angle"), tr("Enter start angle (degrees):"), 0, 0, 360, 1, &ok);
                if (ok) {
                    int endAngle = QInputDialog::getInt(this, tr("Set End Angle"), tr("Enter end angle (degrees):"), 360, 0, 360, 1, &ok);
                    if (ok) {
                        // ���� ShapeDrawer �е���ʼ�ǶȺͽ����Ƕ�
                        shapeDrawer->setArcAngles(startAngle, endAngle);
                    }
                }
            }
        });

        connect(modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
            DrawMode selectedMode = static_cast<DrawMode>(modeComboBox->itemData(index).toInt());
            shapeDrawer->setDrawMode(selectedMode);
            if (selectedMode == BsplineMode) {
                // �����Ի�������Ҫ���ƵĽ���
                bool ok;
                int k_step = QInputDialog::getInt(this, tr("Set Bspline steps"), tr("Enter steps:"), 3, 1, 10, 1, &ok);
                if (ok)
                    shapeDrawer->setk_step(k_step);
            }
        });

        connect(clearButton, &QPushButton::clicked, shapeDrawer, &ShapeDrawer::Clear);

        setLayout(mainLayout);
        setWindowTitle("Drawing with Algorithms");
        // �����ڴ�С��
        setFixedSize(1000, 600);
    }
};

/*
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
*/


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // MainWindow w;
    MainWindow *w = new MainWindow();
    w->show();
    return a.exec();
}
