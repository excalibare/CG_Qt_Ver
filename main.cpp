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
    QAction* createRectAction = ui->Createrect;
    QAction* createCirAction = ui->CreateCir;
    QAction* createWriteTextAction = ui->Write;
    QAction* cancelAction = ui->Cancel;
    QAction* saveAction = ui->savefile;
    QAction* loadAction = ui->loadfile;

    QSpinBox *spinBox = ui->FrontSize;
    spinBox->setRange(0, 100); // 设置范围为0到100
    spinBox->setValue(10); // 设置默认值

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
    QObject::connect(createRectAction, &QAction::triggered, [=]() {
        myshapedrawer->setDrawMode(SpecialPolygonMode);
    });
    // 连接自定义圆形创建
    // 点击后输入半径，或者在第一个窗口输入负数来选择第n个自定义图形（-1即第一个图形）
    QObject::connect(createCirAction, &QAction::triggered, [=]() {
        myshapedrawer->setDrawMode(SpecialCircleMode);
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
    connect(createWriteTextAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->setDrawMode(WriteText);
    });
    connect(cancelAction, &QAction::triggered, this, [myshapedrawer]() {
        myshapedrawer->cancel();
    });
    connect(saveAction, &QAction::triggered, this, [this, myshapedrawer]() {
        // 弹出输入对话框让用户输入文件名称
        bool ok;
        QString fileName = QInputDialog::getText(this, tr("输入文件名称"),
                                                 tr("文件名称:"), QLineEdit::Normal,
                                                 "temp.txt", &ok);

        // 检查用户是否点击了“确定”并输入了有效的文件名
        if (ok && !fileName.isEmpty()) {
            // 可以在这里设置文件保存的路径
            QString filePath = QFileDialog::getSaveFileName(this, tr("保存文件"), fileName, tr("所有文件 (*)"));

            // 检查文件路径是否有效
            if (!filePath.isEmpty()) {
                // 处理选择的文件，例如显示文件名
                QMessageBox::information(this, tr("保存成功！！！"), filePath);

                // 调用 myshapedrawer 的保存函数
                myshapedrawer->save(filePath);
            }
        }
    });
    connect(loadAction, &QAction::triggered, this, [this, myshapedrawer]() {
        // 打开文件选择对话框
        QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("所有文件 (*)"));

        // 检查文件名是否为空
        if (!fileName.isEmpty()) {
            // 处理选择的文件，例如显示文件名
            QMessageBox::information(this, tr("选择的文件"), fileName);
        }
        myshapedrawer->load(fileName);
    });


    // 连接字体大小选择输入
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [myshapedrawer](int value){
        myshapedrawer->frontsize = value;
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
