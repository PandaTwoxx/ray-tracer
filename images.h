#ifndef IMAGES_H
#define IMAGES_H

#include <QImage>
#include <QString>
#include <iostream>

class rtw_image {
public:
    rtw_image() {}

    rtw_image(const char* image_filename) {
        auto filename = QString(image_filename);

        if (load(":/" + filename)) return;          // Qt resource: :/earth.jpeg
        if (load(":/images/" + filename)) return;   // Qt resource with subdir
        if (load("/images/" + filename)) return;    // filesystem fallback
        if (load(filename)) return;                 // bare path fallback

        std::cerr << "ERROR: Could not load image file '"
                  << image_filename << "'.\n";
    }

    bool load(const QString& path) {
        QImage tmp;
        if (!tmp.load(path)) return false;

        // Normalize to RGB888 so pixel_data always returns 3 bytes
        img = tmp.convertToFormat(QImage::Format_RGB888);
        return !img.isNull();
    }

    int width()  const { return img.isNull() ? 0 : img.width(); }
    int height() const { return img.isNull() ? 0 : img.height(); }

    const unsigned char* pixel_data(int x, int y) const {
        static unsigned char magenta[] = { 255, 0, 255 };
        if (img.isNull()) return magenta;

        x = clamp(x, 0, img.width());
        y = clamp(y, 0, img.height());

        return img.constScanLine(y) + x * bytes_per_pixel;
    }

private:
    static constexpr int bytes_per_pixel = 3;
    QImage img;

    static int clamp(int x, int low, int high) {
        if (x < low)  return low;
        if (x < high) return x;
        return high - 1;
    }
};

#endif // IMAGES_H
