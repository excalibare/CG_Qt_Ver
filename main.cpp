#include "mainwindow.h"
#include "./ui_mainwindow.h"
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
#include "shapedrawer.h"
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

//TODO:绘制弧线的 起始角 大于 结束角 功能未完善



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QLabel* test = ui->label;
    //test->setText("man what can i say!");
    ShapeDrawer* myshapedrawer = ui->widget;
    QPushButton* clearButton = ui->Clear;
    clearButton->setText("Clear!");
    QObject::connect(clearButton,&QPushButton::clicked,myshapedrawer,&ShapeDrawer::Clear);
}

MainWindow::~MainWindow()
{
    delete ui;
}


int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    // MainWindow w;
    MainWindow *w = new MainWindow();
    w->setWindowIcon(QIcon("test_icon.png"));
    w->show();
    return a.exec();
}
