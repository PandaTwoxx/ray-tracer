#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QImage>
#include <QColor>

#include "ray.h"
#include "vec3.h"

bool hit_sphere(const vec3& center, double radius, const ray& r){
    vec3 c_to_r = center - r.origin();

    auto a = dot(r.direction(), r.direction());
    auto b = -2.0*dot(r.direction(), c_to_r);
    auto c = dot(c_to_r, c_to_r) - radius*radius;

    auto discriminant = b*b - 4*a*c;
    return (discriminant >= 0);
}

color ray_color(const ray& r){
    // sphere check
    if(hit_sphere(point3(1, 0, -2), 0.5, r)) return color(1, 0, 0);

    vec3 unit_dir = unit_vector(r.direction());
    auto a = 0.5*(unit_dir.y() + 1.0);
    return (1.0 - a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}

QRgb color_to_q(const color& c){
    // Scale 0.0-1.0 to 0-255
    int r = static_cast<int>(255.999 * c.x());
    int g = static_cast<int>(255.999 * c.y());
    int b = static_cast<int>(255.999 * c.z());

    return qRgb(r, g, b);
}

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
    ui->canvas->setText("Rendering...");

    // calculate canvas size
    auto aspect_ratio = 16.0 / 9.0;
    int width = 720;
    int height = int(width / aspect_ratio);
    height = (height < 1) ? 1 : height;

    ui->canvas->setFixedSize(width, height);

    // calculate viewport size and camera
    auto focal_length = 1.0;
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(width)/height);
    auto camera_center = vec3(0, 0, 0);

    // calculate vectors for viewport edges
    auto viewport_u = vec3(viewport_width, 0, 0);
    auto viewport_v = vec3(0, -viewport_height, 0);

    // calculate delta vectors for pixel to pixel
    auto pixel_delta_u = viewport_u / width;
    auto pixel_delta_v = viewport_v / height;

    // calculate topleft pixel location
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
    auto fpixel_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);


    QImage image(width, height, QImage::Format_RGB32);

    for (int y = 0; y < image.height(); ++y) {
        // Get a pointer to the start of this row for speed
        ui->canvas->setText(QString::number(image.height() - y));
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));

        for (int x = 0; x < image.width(); ++x) {
            // calculate pixel location and the ray from the camera to the pixel
            auto pixel = fpixel_loc + (x*pixel_delta_u) + (y*pixel_delta_v);
            auto ray_dir = pixel - camera_center;
            ray r(camera_center, ray_dir);

            // calculate the final rgb value
            color pixel_color = ray_color(r);

            // Assign the RGB value to the pixel
            line[x] = color_to_q(pixel_color);
        }
    }

    ui->canvas->setText("");
    ui->canvas->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}