#include "math/mat.h"
#include "math/quat.h"
#include "math/vec.h"

#include "platform/platform.cc"

// rgg for render go-go!

namespace rgg {

struct Tag {
  // TODO(abrunasso): Support custom shaders.
  GLuint vao_reference;
  GLuint vert_count;
  GLenum mode;
};

bool Initialize();

Tag CreateRenderable(int vert_count, GLfloat* verts, GLenum mode);

void RenderTag(const Tag& tag,
               const math::Vec3f& position,
               const math::Vec3f& scale,
               const math::Quatf& orientation,
               const math::Vec4f& color);

void RenderTriangle(const math::Vec3f& position,
                    const math::Vec3f& scale,
                    const math::Quatf& orientation,
                    const math::Vec4f& color);

void RenderRectangle(const math::Vec3f& position,
                     const math::Vec3f& scale,
                     const math::Quatf& orientation,
                     const math::Vec4f& color);

void RenderCircle(const math::Vec3f& position,
                  const math::Vec3f& scale,
                  const math::Quatf& orientation,
                  const math::Vec4f& color);

void RenderLine(const math::Vec3f& start,
                const math::Vec3f& end,
                const math::Vec4f& color);

void RenderGrid(float width, float height, const math::Vec4f& color);

}
