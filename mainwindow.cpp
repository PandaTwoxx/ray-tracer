#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QImage>
#include <QColor>

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

    // calculate viewport size
    auto viewport_height = 2.0;
    auto viewport_width = viewport_height * (double(width)/height);


    QImage image(width, height, QImage::Format_RGB32);

    for (int y = 0; y < image.height(); ++y) {
        // Get a pointer to the start of this row for speed
        ui->canvas->setText(QString::number(image.height() - y));
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));

        for (int x = 0; x < image.width(); ++x) {
            // --- YOUR CUSTOM RGB LOGIC HERE ---
            int r = (x * 255) / image.width();
            int g = (y * 255) / image.height();
            int b = 150;

            // Assign the RGB value to the pixel
            line[x] = qRgb(r, g, b);
        }
    }

    ui->canvas->setText("");
    ui->canvas->setPixmap(QPixmap::fromImage(image));
}

void MainWindow::on_pushButton_clicked()
{
    renderCanvas();
}