#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray{
public:
    ray(){}

    ray(const vec3& origin, const vec3& direction, double time) : orig(origin), dir(direction), tm(time){}
    ray(const vec3& origin, const vec3& direction) : orig(origin), dir(direction){}

    const vec3& origin() const { return orig; }
    const vec3& direction() const { return dir; }

    double time() const{
        return tm;
    }

    point3 at(double t) const{
        return orig + t*dir;
    }

private:
    vec3 orig;
    vec3 dir;
    double tm;
};

#endif // RAY_H
