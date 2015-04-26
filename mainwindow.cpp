#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QFFmpegGLWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QFFmpegPlayer::Init();

    player=new QFFmpegPlayer();
    player->play();

    QFFmpegGLWidget *gl=new QFFmpegGLWidget();
    player->setGLWidget(gl);
    this->setCentralWidget(gl);

}

MainWindow::~MainWindow()
{
    delete ui;

    player->destroy();

}

