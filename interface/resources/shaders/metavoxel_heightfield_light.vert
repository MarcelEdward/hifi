#version 120

//
//  metavoxel_heighfield_light.vert
//  vertex shader
//
//  Created by Andrzej Kapolka on 8/20/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// the height texture
uniform sampler2D heightMap;

// the distance between height points in texture space
uniform float heightScale;

// the interpolated position
varying vec4 position;

// the interpolated normal
varying vec4 normal;

void main(void) {
    // transform and store the normal for interpolation
    vec2 heightCoord = gl_MultiTexCoord0.st;
    float deltaX = texture2D(heightMap, heightCoord - vec2(heightScale, 0.0)).r -
        texture2D(heightMap, heightCoord + vec2(heightScale, 0.0)).r;
    float deltaZ = texture2D(heightMap, heightCoord - vec2(0.0, heightScale)).r -
        texture2D(heightMap, heightCoord + vec2(0.0, heightScale)).r;
    normal = normalize(gl_ModelViewMatrix * vec4(deltaX, heightScale, deltaZ, 0.0));
    
    // add the height to the position
    float height = texture2D(heightMap, heightCoord).r;
    position = gl_ModelViewMatrix * (gl_Vertex + vec4(0.0, height, 0.0, 0.0));
    gl_Position = gl_ProjectionMatrix * position;
    
    // the zero height should be invisible
    gl_FrontColor = vec4(1.0, 1.0, 1.0, step(height, 0.0));
    
    // and the shadow texture coordinates
    gl_TexCoord[1] = vec4(dot(gl_EyePlaneS[0], position), dot(gl_EyePlaneT[0], position), dot(gl_EyePlaneR[0], position), 1.0);
}
