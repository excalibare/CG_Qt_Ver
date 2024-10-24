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
    clearButton->setText("Clear!");
    QObject::connect(clearButton, &QPushButton::clicked, myshapedrawer, &ShapeDrawer::Clear);

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
