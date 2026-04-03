#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
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
    double vfov = 90;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat = point3(0, 0, -1);
    vec3 vup = vec3(0, 1, 0);
    color background;

    double defocus_angle = 0;
    double focus_dist = 10;

    camera(double ar, int w): aspect_ratio(ar), width(w) {}
    camera(){}

signals:
    void statusMessage(QString msg);
    void updateImage(QImage qi);

public:

    QImage render(const hittable& world){
        initialize();

        emit statusMessage("Rendering...");

        QImage image(width, height, QImage::Format_RGB32);

        for (int y = 0; y < image.height(); ++y) {
            if (y % 20 == 0 || y == height - 1) {
                emit statusMessage(QString("Rendering lines %1-%2/%3...").arg(y).arg(std::min(y+19, height)).arg(height));
            }
            // Get a pointer to the start of this row for speed
            QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(y));

            for (int x = 0; x < image.width(); ++x) {
                color pixel_color(0,0,0);
                for(int sample = 0; sample < sample_count; sample++){
                    ray r = get_ray(x, y);
                    pixel_color += ray_color(r, max_bounces, world);
                    rays_calculated++;
                }
                line[x] = color_to_q(pixel_color * pixel_ss);
            }
            emit updateImage(image);
        }

        emit statusMessage("Done. Rendered " + QString::number(rays_calculated) + " rays and " + QString::number(bounces) + " bounces.");

        return image;
    }

    void calculateHeight(){
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;
    }
private:
    point3 fpixel_loc;
    point3 camera_center;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    double pixel_ss;
    vec3 u, v, w;
    vec3 defocus_disk_u;
    vec3 defocus_disk_v;
    int rays_calculated = 0;
    int bounces = 0;

    void initialize(){
        // calculate canvas size
        height = int(width / aspect_ratio);
        height = (height < 1) ? 1 : height;

        pixel_ss = 1.0 / sample_count;
        camera_center = lookfrom;

        // calculate viewport size and camera
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(width)/height);

        //calculate u v w
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // calculate vectors for viewport edges
        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        // calculate delta vectors for pixel to pixel
        pixel_delta_u = viewport_u / width;
        pixel_delta_v = viewport_v / height;

        // calculate topleft pixel location
        auto viewport_upper_left = camera_center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        fpixel_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);

        //calculate camera defocus disk vectors
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle/2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    color ray_color(const ray& r, int depth, const hittable& world){
        bounces++;
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;

        if(!world.hit(r, interval(0.001, infinity), rec)){
            return background;
        }

        ray scattered;
        color attenuation;
        color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

        if (!rec.mat->scatter(r, rec, attenuation, scattered))
            return color_from_emission;

        color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

        return color_from_emission + color_from_scatter;
    }

    QRgb color_to_q(const color& c){
        // Scale 0.0-1.0 to 0-255
        static const interval intensity(0.000, 0.999);
        int r = int(256 * intensity.clamp(linear_to_gamma(c.x())));
        int g = int(256 * intensity.clamp(linear_to_gamma(c.y())));
        int b = int(256 * intensity.clamp(linear_to_gamma(c.z())));

        return qRgb(r, g, b);
    }
    double linear_to_gamma(double linear_component)
    {
        if (linear_component > 0)
            return std::sqrt(linear_component);

        return 0;
    }

    ray get_ray(int i, int j) const {
        auto offset = sample_square();
        auto pixel_sample = fpixel_loc + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);
        auto ray_origin = (defocus_angle <= 0) ? camera_center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }
};

#endif // CAMERA_H
