#pragma once

namespace rgg {

inline constexpr const char* kVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 matrix;
  uniform vec4 color;
  out vec4 color_out;
  void main() {
    color_out = color;
    gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

inline constexpr const char* kFragmentShader = R"(
  #version 410
  in vec4 color_out;
	out vec4 frag_color;
  void main() {
   frag_color = color_out;
  }
)";

inline constexpr const char* kCircleVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 model;
  uniform mat4 view_projection;
  uniform vec4 color;
  out vec4 color_out;
  out vec3 center_out;
  out vec3 position_out;
  void main() {
    color_out = color;
    center_out = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    position_out = (model * vec4(vertex_position, 1.0)).xyz;
    gl_Position = view_projection * model * vec4(vertex_position, 1.0);
  }
)";

inline constexpr const char* kCircleFragmentShader = R"(
  #version 410
  in vec4 color_out;
  in vec3 center_out;
  in vec3 position_out;
	out vec4 frag_color;
  void main() {
    float dist = length(position_out - center_out);
    float delta = 3;
    // TODO(anthony): Replace 25 with a radius.
    float alpha = smoothstep(25 - delta, 25, dist);
    frag_color = vec4(color_out.xyz, 1.0 - alpha);
  }
)";


inline constexpr const char* kFontVertexShader = R"(
  #version 410
  layout (location = 0) in vec4 text_pos;
  uniform mat4 matrix;
  out vec2 texture_coordinates;
  void main() {
  	texture_coordinates = text_pos.zw;
  	gl_Position = matrix * vec4(text_pos.xy, 0.0 , 1.0);
  }
)";

inline constexpr const char* kFontFragmentShader = R"(
  #version 410
  in vec2 texture_coordinates;
  uniform sampler2D basic_texture;
  out vec4 frag_color;
  void main() {
  	vec4 texel = texture(basic_texture, texture_coordinates);
  	frag_color = vec4(texel.x, texel.x, texel.x, 1.0);
  }
)";

inline constexpr const char* kTextureVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  layout (location = 1) in vec2 uv_position;
  uniform mat4 matrix;
  out vec2 texture_coordinates;
  void main() {
  	texture_coordinates = uv_position;
  	gl_Position = matrix * vec4(vertex_position, 1.0);
  }
)";

inline constexpr const char* kTextureFragmentShader = R"(
  #version 410
  in vec2 texture_coordinates;
  uniform sampler2D basic_texture;
  layout(location = 0) out vec4 frag_color;
  void main() {
  	frag_color = texture(basic_texture, texture_coordinates);
  }
)";


}
