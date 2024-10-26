#include <QApplication>
#include <QWidget>
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "shapedrawer.h"
#include "stdafx.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ShapeDrawer *myshapedrawer = ui->widget;
    QPushButton *clearButton = ui->Clear;
    clearButton->setText("Clear");
    QPushButton *colorButton = ui->Color;
    colorButton->setText("SetColor");
    QPushButton *polygonButton = ui->CreatePoly;
    polygonButton->setText("DrawPolygon");
    QPushButton *circleButton = ui->CreateCir;
    circleButton->setText("DrawCircle");

    // 连接选择变化信号(槽函数)

    // 连接清空按钮
    QObject::connect(clearButton, &QPushButton::clicked, myshapedrawer, &ShapeDrawer::Clear);
    // 连接颜色选择按钮，打开颜色选择器
    QObject::connect(colorButton, &QPushButton::clicked, [=]() {
        QColor color = QColorDialog::getColor(Qt::black, this, "Choose Line Color");
        if (color.isValid()) {
            myshapedrawer->setCurrentLineColor(color);
        }
    });
    // 连接自定义矩形创建
    // 点击后输入宽和高，或者在第一个窗口输入负数来选择第n个自定义图形（-1即第一个图形）
    QObject::connect(polygonButton, &QPushButton::clicked, [=]() {
        myshapedrawer->setDrawMode(SpecialPolygonMode);
    });
    // 连接自定义圆形创建
    // 点击后输入半径，或者在第一个窗口输入负数来选择第n个自定义图形（-1即第一个图形）
    QObject::connect(circleButton, &QPushButton::clicked, [=]() {
        myshapedrawer->setDrawMode(SpecialCircleMode);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // MainWindow w;
    MainWindow *w = new MainWindow();
    w->setWindowIcon(QIcon("test_icon.png"));
    w->show();
    return a.exec();
}
