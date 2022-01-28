#include "stdafx.h"
#include "EditorMain.h"

#include <cmath>

/* TODO:
- Add Project and Unproject functions (https://www.khronos.org/opengl/wiki/GluProject_and_gluUnProject_code)
*/


/** Math functions **/
float degtorad(float deg)
{
    return deg * pi / 180.0f;
}

float lengthdir_x(float len,float yaw,float pitch)
{
    return(std::sinf(degtorad(yaw-90)) * cosf(degtorad(pitch)) * len);
}


float lengthdir_y(float len,float yaw,float pitch)
{
    return(std::cosf(degtorad(yaw-90)) * cosf(degtorad(pitch)) * len);
}


float lengthdir_z(float len, float pitch)
{
    return std::sinf(degtorad(pitch)) * len;
}


/* Collision/Intersection */

bool RayIntersectsPoint(glm::vec3 &Origin, glm::vec3 &Direction, const glm::vec3 &Point, float min_range = 0.f) {
    glm::vec3 P(Origin), D(Direction), V(Point);

    glm::vec3 vss = V - P;
    glm::normalize(vss);

    float test_dist = std::sqrtf( std::powf(D.x-vss.x, 2) + std::powf(D.z-vss.z, 2) + std::powf(D.z-vss.z, 2) );

    return (test_dist > (-min_range) && test_dist < min_range);
}

bool RayIntersectsTriangle(glm::vec3 &Origin, glm::vec3 &Direction, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3) {
    glm::vec3 p(Origin),d(Direction);
    glm::vec3 e1,e2,h,s,q;
    glm::vec3 V0(v1),V1(v2),V2(v3);
    float a,f,u,v,t;

    e1 = V1 - V0;
    e2 = V2 - V0;

    h = glm::cross(d, e2);
    a = glm::dot(e1, h);

    if (a > -0.00001f && a < 0.00001f) return(false);
    //if (a == 0.0f) return false;

    f = 1.0f / a;

    s = p - V0;

    u = f * (glm::dot(s, h));

    if (u < 0.0f || u > 1.0f) return false;

    q = glm::cross(s, e1);

    v = f * glm::dot(d, q);

    if (v < 0.0f || (u + v) > 1.0f) return false;

    // at this stage we can compute t to find out where
    // the intersection point is on the line

    t = f * glm::dot(e2, q);

    if (t > 0.00001f) 
        return true;
    else 
        return false;
}


void SubdivideFace(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
    /*
    http://www.glprogramming.com/red/chapter02.html#name8
    */
    glm::vec3 v12, v23, v31;
    int i;

    for (i = 0; i < 3; i++) {
        v12[i] = v1[i] + v2[i];
        v23[i] = v2[i] + v3[i];
        v31[i] = v3[i] + v1[i];
    }

    v12 = glm::normalize(v12);
    v23 = glm::normalize(v23);
    v31 = glm::normalize(v31);

    /*
    drawtriangle(v1, v12, v31);
    drawtriangle(v2, v23, v12);
    drawtriangle(v3, v31, v23);
    drawtriangle(v12, v23, v31);
    */
}
