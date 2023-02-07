// Copyright 2020 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

// Common file to make closest-hit GLSL shaders shorter to write.
// At the moment, each .glsl file can only have a single entry point, even
// though SPIR-V supports multiple entry points per module - this is why
// we have many small .rchit.glsl files.
#ifndef VK_MINI_PATH_TRACER_CLOSEST_HIT_COMMON_H
#define VK_MINI_PATH_TRACER_CLOSEST_HIT_COMMON_H

#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#include "../common.h"
#include "shaderCommon.h"

// This will store two of the barycentric coordinates of the intersection when
// closest-hit shaders are called:
hitAttributeEXT vec2 attributes;

// These shaders can access the vertex and index buffers:
// The scalar layout qualifier here means to align types according to the alignment
// of their scalar components, instead of e.g. padding them to std140 rules.
layout(binding = BINDING_VERTICES, set = 0, scalar) buffer Vertices
{
  vec3 vertices[];
};
layout(binding = BINDING_INDICES, set = 0, scalar) buffer Indices
{
  uint indices[];
};

// The payload:
layout(location = 0) rayPayloadInEXT PassableInfo pld;

struct HitInfo
{
  vec3 objectPosition;
  vec3 worldPosition;
  vec3 worldNormal;
  vec3 color;
};

// Gets hit info about the object at the intersection. This uses GLSL variables
// defined in closest hit stages instead of ray queries.
HitInfo getObjectHitInfo()
{
  HitInfo result;
  // Get the ID of the triangle
  const int primitiveID = gl_PrimitiveID;

  // Get the indices of the vertices of the triangle
  const uint i0 = indices[3 * primitiveID + 0];
  const uint i1 = indices[3 * primitiveID + 1];
  const uint i2 = indices[3 * primitiveID + 2];

  // Get the vertices of the triangle
  const vec3 v0 = vertices[i0];
  const vec3 v1 = vertices[i1];
  const vec3 v2 = vertices[i2];


  // Get the barycentric coordinates of the intersection
  vec3 barycentrics = vec3(0.0, attributes.x, attributes.y);
  barycentrics.x    = 1.0 - barycentrics.y - barycentrics.z;

  // Compute the coordinates of the intersection
  result.objectPosition = v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
  // Transform from object space to world space:
  result.worldPosition = gl_ObjectToWorldEXT * vec4(result.objectPosition, 1.0f);


  // Compute the normal of the triangle in object space, using the right-hand rule:
  //    v2      .
  //    |\      .
  //    | \     .
  //    |/ \    .
  //    /   \   .
  //   /|    \  .
  //  L v0---v1 .
  // n
  const vec3 objectNormal = cross(v1 - v0, v2 - v0);
  // Transform normals from object space to world space. These use the transpose of the inverse matrix,
  // because they're directions of normals, not positions:
  result.worldNormal = normalize((objectNormal * gl_WorldToObjectEXT).xyz);

  // Flip the normal so it points against the ray direction:
  const vec3 rayDirection = gl_WorldRayDirectionEXT;
  result.worldNormal      = faceforward(result.worldNormal, rayDirection, result.worldNormal);

  return result;
}

// offsetPositionAlongNormal shifts a point on a triangle surface so that a
// ray bouncing off the surface with tMin = 0.0 is no longer treated as
// intersecting the surface it originated from.
//
// Here's the old implementation of it we used in earlier chapters:
// vec3 offsetPositionAlongNormal(vec3 worldPosition, vec3 normal)
// {
//   return worldPosition + 0.0001 * normal;
// }
//
// However, this code uses an improved technique by Carsten W
