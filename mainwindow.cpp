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
    QImage image(720, 480, QImage::Format_RGB32);

    for (int y = 0; y < image.height(); ++y) {
        // Get a pointer to the start of this row for speed
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