#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QImage>
#include <QColor>

#include "vec3.h"
#include "rt.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::renderCanvas(){
    // world
    hittable_list world;

    world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
    world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

    camera cam;
    cam.sample_count = 20;

    QImage render = cam.render(world, ui->status);

    ui->canvas->setFixedSize(cam.width, cam.height);
    ui->canvas->setPixmap(QPixmap::fromImage(render));
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}