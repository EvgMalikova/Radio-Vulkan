// Copyright 2020 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0
#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_scalar_block_layout : require
#include "../common.h"
#include "shaderCommon.h"
// The payload:
layout(location = 0) rayPayloadInEXT PassableInfo pld;
hitAttributeEXT vec2 attributes;
//#include "closestHitCommon.h"

void main()
{
  //HitInfo hitInfo = getObjectHitInfo();
 
  
  vec3  origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * (gl_HitTEXT+0.3);
    vec3  rayDir = gl_WorldRayDirectionEXT;
    uint  flags  =  gl_RayFlagsOpaqueEXT ;

  
  pld.color        = vec3(0.7);
  pld.rayOrigin    = origin;//offsetPositionAlongNormal(hitInfo.worldPosition, hitInfo.worldNormal);
  pld.rayDirection = rayDir;//diffuseReflection(hitInfo.worldNormal, pld.rngState);
  pld.rayHitSky    = false;
}
