#ifndef POLYGON_H
#define POLYGON_H
#include <QColor>
#include <QDebug>
#include <QPoint>
#include "point.h"
#include <vector>
#include <QFile>
#include <QDataStream>
using namespace std;

// 将Point类型的点转换为QPoint
QPoint P2QP(Point p)
{
    QPoint res;
    res.setX(p.x);
    res.setY(p.y);
    return res;
}

// 将Point类型的点转换为QPoint
Point QP2P(QPoint p)
{
    Point res;
    res.x = p.x();
    res.y = p.y();
    return res;
}

// 结构体来存储多边形每个边的信息，为多边形绘制服务
struct Edge
{
    int yMax;
    float xMin, slopeReciprocal;
};

struct PolygonShape
{
public:
    int width;
    int height;

    PolygonShape(int w = 0, int h = 0)
        : width(w)
        , height(h)
    {}

    void Set(int w, int h)
    {
        width = w;
        height = h;
    }
};

// 多边形
class Polygon
{
public:
    vector<Point> points; // 储存多边形的顶点
    QColor color;
    Polygon() {}

    void addPoint(const Point& point) { points.push_back(point); }

    void addPoint(const Polygon& polygon)
    {
        for (int i = 0; i < polygon.points.size(); ++i) {
            points.push_back(polygon.points[i]);
        }
    }

    bool isClosed() const
    {
        return points.size() > 2 && points.front().x == points.back().x
               && points.front().y == points.back().y;
    }

    void closePolygon()
    {
        if (points.size() > 2 && !isClosed()) {
            points.push_back(points.front()); // 封闭多边形
        }
    }

    Polygon operator=(const Polygon& poly)
    {
        this->points = poly.points;
        this->color = poly.color;
        return *this;
    }

    int length() { return points.size(); }

    void clear()
    {
        vector<Point> tem;
        points = tem;
        color = Qt::black;
    }

    void remove(int i) { points.erase(points.begin() + i); }

    void print()
    {
        qDebug() << "The shape is: ";
        for (int j = 0; j < points.size(); ++j) {
            qDebug() << points[j].Getx() << " & " << points[j].Gety() << " || ";
        }
        qDebug() << "\n";
    }
};

// 将自定义Polygon类型的Polygon转为QVector(QPoint)
QVector<QPoint> P2QV(Polygon p)
{
    QVector<QPoint> res;
    for (int i = 0; i < p.points.size() - 1; i++) {
        res.push_back(P2QP(p.points[i]));
    }
    return res;
}

// 将QVector(QPoint)转为自定义Polygon类型的Polygon
Polygon QV2P(QVector<QPoint> q)
{
    Polygon res;
    for (int i = 0; i < q.size(); i++) {
        res.points.push_back(QP2P(q[i]));
    }
    res.points.push_back(res.points[0]);
    return res;
}

// 保存 Polygon 向量到文本文件
void savePolygonsToTextFile(const QVector<Polygon>& polygons, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行写入:" << filename;
        return;
    }

    QTextStream out(&file);
    out << polygons.size() << "\n"; // 写入多边形数量

    for (const auto& polygon : polygons) {
        out << polygon.points.size() << "\n"; // 写入顶点数量
        for (const auto& point : polygon.points) {
            out << point.x << " " << point.y << "\n"; // 写入每个顶点
        }
        out << polygon.color.red() << " "
            << polygon.color.green() << " "
            << polygon.color.blue() << " "
            << polygon.color.alpha() << "\n"; // 写入颜色
    }

    file.close();
}

// 从文本文件加载 Polygon 向量
QVector<Polygon> loadPolygonsFromTextFile(const QString& filename) {
    QVector<Polygon> polygons;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行读取:" << filename;
        return polygons; // 返回空 QVector
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
    }

    file.close();
    return polygons;
}

#endif // !POLYGON_H
