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
    // 顶部工具栏
    QToolBar* tbar = ui->toolBar;
    tbar->setMovable(false);//工具栏不可移动
    tbar->setStyleSheet("background-color:rgb(243, 249, 255);");//背景色

    QPushButton *clearButton = ui->Clear;
    clearButton->setText("Clear");
    QPushButton *colorButton = ui->Color;
    colorButton->setText("SetColor");

    QAction* solidarrowAction = ui->SolidArrow;
    QAction* solid2arrowAction = ui->Solid2Arrow;
    QAction* dashedarrowAction = ui->DashedArrow;
    QAction* dashed2arrowAction = ui->Dashed2Arrow;

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

    // 连接实线单向箭头动作
    connect(solidarrowAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->setDrawMode(SolidArrow); // 设置为SolidArrow模式
        myshapedrawer->set_ways(1);
    });
    // 连接实线双向箭头动作
    connect(solid2arrowAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->setDrawMode(SolidArrow);
        myshapedrawer->set_ways(2);
    });
    // 连接虚线单向箭头动作
    connect(dashedarrowAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->setDrawMode(DashedArrow); // 设置为DashedArrow模式
        myshapedrawer->set_ways(1);
    });
    // 连接虚线双向箭头动作
    connect(dashed2arrowAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->setDrawMode(DashedArrow);
        myshapedrawer->set_ways(2);
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
