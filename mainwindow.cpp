#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QColor>
#include <QtConcurrent/QtConcurrent>

#include "vec3.h"
#include "rt.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&cam, &camera::statusMessage, this, [this](QString y) {
        ui->status->setText(y);
        ui->canvas->update();
    });

    connect(&watcher, &QFutureWatcher<QImage>::finished, this, [this]() {
        QImage result = watcher.result();
        if (!result.isNull()) {
            ui->canvas->setFixedSize(result.width(), result.height());
            ui->canvas->setPixmap(QPixmap::fromImage(result));
            ui->canvas->update();
        }
        ui->status->setText("Done!");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::renderCanvas(){
    // world
    hittable_list world;

    auto material_ground = make_shared<lambertian>(color(0.1, 0.1, 0.1));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<metal>(color(0.8, 0.8, 0.8));
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2));

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0), 100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.2),   0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   0.5, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),   0.5, material_right));

    cam.sample_count = 50;
    cam.max_bounces = 50;

    QFuture<QImage> future = QtConcurrent::run([this, world]() {
        return cam.render(world);
    });
    watcher.setFuture(future);
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}