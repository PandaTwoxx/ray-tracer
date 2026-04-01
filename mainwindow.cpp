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
    });

    connect(&cam, &camera::updateImage, this, [this](QImage qi) {
        ui->canvas->setPixmap(QPixmap::fromImage(qi));
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
    ui->status->setText("Generating Scene");
    // world
    hittable_list world;

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    cam.aspect_ratio = 16.0 / 9.0;
    cam.width       = 720;
    cam.sample_count = 10;
    cam.max_bounces         = 50;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    // DO NOT CHANGE
    cam.calculateHeight();
    ui->canvas->setFixedSize(cam.width, cam.height);

    QFuture<QImage> future = QtConcurrent::run([this, world]() {
        return cam.render(world);
    });
    watcher.setFuture(future);
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}