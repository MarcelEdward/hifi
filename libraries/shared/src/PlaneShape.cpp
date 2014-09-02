//
//  PlaneShape.cpp
//  libraries/shared/src
//
//  Created by Andrzej Kapolka on 4/10/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "PlaneShape.h"
#include "SharedUtil.h"

const glm::vec3 UNROTATED_NORMAL(0.0f, 1.0f, 0.0f);

PlaneShape::PlaneShape(const glm::vec4& coefficients) :
    Shape(PLANE_SHAPE) {
    
    glm::vec3 normal = glm::vec3(coefficients);
    _translation = -normal * coefficients.w;
    
    float angle = acosf(glm::dot(normal, UNROTATED_NORMAL));
    if (angle > EPSILON) {
        if (angle > PI - EPSILON) {
            _rotation = glm::angleAxis(PI, glm::vec3(1.0f, 0.0f, 0.0f));       
        } else {
            _rotation = glm::angleAxis(angle, glm::normalize(glm::cross(UNROTATED_NORMAL, normal)));
        }
    }
}

glm::vec3 PlaneShape::getNormal() const {
    return _rotation * UNROTATED_NORMAL;
}

glm::vec4 PlaneShape::getCoefficients() const {
    glm::vec3 normal = _rotation * UNROTATED_NORMAL;
    return glm::vec4(normal.x, normal.y, normal.z, -glm::dot(normal, _translation));
}

bool PlaneShape::findRayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDirection, float& distance) const {
    glm::vec3 n = getNormal();
    float denominator = glm::dot(n, rayDirection);
    if (fabsf(denominator) < EPSILON) {
        // line is parallel to plane
        return glm::dot(_translation - rayStart, n) < EPSILON;
    } else {
        float d = glm::dot(_translation - rayStart, n) / denominator;
        if (d > 0.0f) {
            // ray points toward plane
            distance = d;
            return true;
        }
    }
    return false;
}
