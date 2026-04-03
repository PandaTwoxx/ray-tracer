#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include <QImage>
#include <QLabel>
#include <QObject>
#include <thread>
#include <vector>
#include <atomic>

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
    int thread_count = 1;

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
        image.fill(Qt::black);
        rays_calculated = 0;
        bounces = 0;

        int threads_to_use = thread_count > 0 ? thread_count : 1;
        std::vector<std::thread> threads;
        std::atomic<int> rows_completed(0);

        uchar* bits = image.bits();
        int bpl = image.bytesPerLine();

        auto renderRows = [&](int start_y, int end_y) {
            for (int y = start_y; y < end_y; ++y) {
                QRgb *line = reinterpret_cast<QRgb*>(bits + y * bpl);
                int local_rays = 0;

                for (int x = 0; x < width; ++x) {
                    color pixel_color(0,0,0);
                    for(int sample = 0; sample < sample_count; sample++){
                        ray r = get_ray(x, y);
                        pixel_color += ray_color(r, max_bounces, world);
                        local_rays++;
                    }
                    line[x] = color_to_q(pixel_color * pixel_ss);
                }
                
                rays_calculated += local_rays;
                rows_completed++;
                int finished = rows_completed.load();
                if (finished % 20 == 0 || finished == height) {
                    emit statusMessage(QString("Rendering... %1/%2 lines completed").arg(finished).arg(height));
                    emit updateImage(image.copy());
                }
            }
        };

        int rows_per_thread = height / threads_to_use;
        for (int i = 0; i < threads_to_use; ++i) {
            int start = i * rows_per_thread;
            int end = (i == threads_to_use - 1) ? height : start + rows_per_thread;
            threads.emplace_back(renderRows, start, end);
        }

        for (auto& t : threads) {
            t.join();
        }

        emit updateImage(image.copy());
        emit statusMessage("Done. Rendered " + QString::number(rays_calculated.load()) + " rays and " + QString::number(bounces.load()) + " bounces.");

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
    std::atomic<int> rays_calculated{0};
    std::atomic<int> bounces{0};

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
