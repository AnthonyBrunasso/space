#pragma once

namespace gfx
{

struct Light {
  v3f kLightPosition;
  v3f kSpecularColor = v3f(1.f, 1.f, 1.f);
  v3f kDiffuseColor = v3f(.7f, .7f, .7f);
  v3f kAmbientColor = v3f(.2f, .2f, .2f);
};

// Assume a single light source for now.
static Light kLight;

// Reflectance properties of objects. Not sure where this should go.
struct SurfaceProperties {
  v3f specular_reflectance = v3f(1.f, 1.f, 1.f);
  v3f diffuse_reflectance = v3f(1.f, .5f, 0.f);
  v3f ambient_reflectance = v3f(1.f, 1.f, 1.f);
  float specular_exponent = 100.f;  // Power of specular highlights.
};

static SurfaceProperties kDefaultSurfaceProperties;

}  // namespace gfx
