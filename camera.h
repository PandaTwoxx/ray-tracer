#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include <QImage>
#include <QLabel>
#include <QObject>

class camera : public QObject{
    Q_OBJECT
public:
    double aspect_ratio = 16.0 / 9.0;
    int width = 720;
    int height = 405;
    int sample_count = 10;
    int max_bounces = 10;

    camera(double ar, int w): aspect_ratio(ar), width(w) {}
    camera(){}

signals:
    void statusMessage(QString msg);

public:

    QImage render(const hittable& world){
        initialize();

        emit statusMessage("Rendering...");

        QImage image(width, height, QImage::Format_RGB32);

        for (int y = 0; y < image.height(); ++y) {
            emit statusMessage("Rendering line " + QString::number(y) + " out of " + QString::number(image.height()));
            // Get a pointer to the start of this row for speed
            QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));

            for (int x = 0; x < image.width(); ++x) {
                color pixel_color(0,0,0);
                for(int sample = 0; sample < sample_count; sample++){
                    ray r = get_ray(x, y);
                    pixel_color += ray_color(r, max_bounces, world);
                }
                line[x] = color_to_q(pixel_color * pixel_ss);
            }
        }

        emit statusMessage("Done.");

        return image;
    }
private:
    point3 fpixel_loc;
    point3 camera_center;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    double pixel_ss;

    void initialize(){
        // calculate canvas size
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        pixel_ss = 1.0 / sample_count;

        // calculate viewport size and camera
        auto focal_length = 1.0;
        auto viewport_height = 2.0;
        auto viewport_width = viewport_height * (double(width)/height);
        camera_center = vec3(0, 0, 0);

        // calculate vectors for viewport edges
        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        // calculate delta vectors for pixel to pixel
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // calculate topleft pixel location
        auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) - viewport_u/2 - viewport_v/2;
        fpixel_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world){
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;
        if(world.hit(r, interval(0, infinity), rec)){
            vec3 direction = random_on_hemisphere(rec.normal);
            return 0.5 * ray_color(ray(rec.p, direction), depth-1, world);
        }
        vec3 unit_dir = unit_vector(r.direction());
        auto a = 0.5*(unit_dir.y() + 1.0);
        return (1.0 - a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

    QRgb color_to_q(const color& c){
        // Scale 0.0-1.0 to 0-255
        static const interval intensity(0.000, 0.999);
        int r = int(256 * intensity.clamp(c.x()));
        int g = int(256 * intensity.clamp(c.y()));
        int b = int(256 * intensity.clamp(c.z()));

        return qRgb(r, g, b);
    }

    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = fpixel_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
        auto ray_origin = camera_center;
        auto ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

};

#endif // CAMERA_H
