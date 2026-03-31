#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QImage>
#include <QColor>
#include <QtConcurrent/QtConcurrent>

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

    cam.sample_count = 20;

    connect(&cam, &camera::statusMessage, this, [this](QString y) {
        ui->status->setText(y);
    });

    connect(&watcher, &QFutureWatcher<QImage>::finished, this, [this]() {
        QImage result = watcher.result();
        ui->canvas->setFixedSize(result.width(), result.height());
        ui->canvas->setPixmap(QPixmap::fromImage(result));
        ui->status->setText("Done!");
    });

    QFuture<QImage> future = QtConcurrent::run([this, world]() {
        return cam.render(world);
    });
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}