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

inline constexpr const char* kVertexShader3d = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  layout (location = 1) in vec3 vertex_normal;

  uniform mat4 projection;
  uniform mat4 view;
  uniform mat4 model;
  uniform vec4 color;
  uniform vec3 light_position_world;

  out vec3 eye_position;
  out vec3 eye_normal;

  void main() {
    eye_position = vec3(view * model * vec4(vertex_position, 1.0));
    eye_normal = vec3(view * model * vec4(vertex_normal, 0.0));
    gl_Position = projection * vec4(eye_position, 1.0);
  }
)";

inline constexpr const char* kFragmentShader3d = R"(
  #version 410
  in vec3 eye_position;
  in vec3 eye_normal;

  uniform mat4 view;
  uniform vec4 color;
  uniform vec3 light_position_world;

  // Default light properties.
  vec3 light_specular = vec3(1.0, 1.0, 1.0);
  vec3 light_diffuse = vec3(0.7, 0.7, 0.7);
  vec3 light_ambient = vec3(0.2, 0.2, 0.2);

  // Surface reflectance.
  vec3 surface_specular = vec3(1.0, 1.0, 1.0);
  vec3 surface_diffuse = vec3(0.7, 0.7, 0.8);
  vec3 surface_ambient = vec3(1.0, 1.0, 1.0);

  float specular_exponent = 100.0; 
  out vec4 frag_color;

  void main() {
    vec3 intensity_ambient = light_ambient * surface_ambient;
    vec3 eye_normal = normalize(eye_normal);
    
    vec3 eye_light_position = vec3(view * vec4(light_position_world, 1.0));
    vec3 eye_distance_to_light = eye_light_position - eye_position;
    vec3 eye_direction_to_light = normalize(eye_distance_to_light);
    float d = dot(eye_direction_to_light, eye_normal);
    d = max(d, 0.0);
    vec3 intensity_diffuse = light_diffuse * surface_diffuse * d;

    vec3 eye_surface_to_viewer = normalize(-eye_position);
    vec3 eye_half_way = normalize(eye_surface_to_viewer + eye_direction_to_light);
    float ds = max(dot(eye_half_way, eye_normal), 0.0);
    float specular_factor = pow(ds, specular_exponent);
    vec3 intensity_specular = light_specular * surface_diffuse * specular_factor;

    // TODO(abrunasso): How do I combine color and light?
    frag_color = color * vec4(intensity_ambient + intensity_diffuse + intensity_specular, 1.0);
  }
)";


inline constexpr const char* kSmoothRectangleVertexShader = R"(
#version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 model;
  uniform mat4 view_projection;
  uniform vec4 color;
  uniform float smoothing_radius;
  out vec4 color_out;
  out float smoothing_radius_out;
  out vec3 center_out;
  out vec3 position_out;
  void main() {
    smoothing_radius_out = smoothing_radius;
    color_out = color;
    center_out = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    position_out = (model * vec4(vertex_position, 1.0)).xyz;
    gl_Position = view_projection * vec4(position_out, 1.0);
  }
)";

inline constexpr const char* kSmoothRectangleFragmentShader = R"(
#version 410
  in vec4 color_out;
  in float smoothing_radius_out;
  in vec3 center_out;
  in vec3 position_out;
  out vec4 frag_color;
  void main() {
    float dist = length(position_out - center_out);
    if (dist > smoothing_radius_out) discard;
    float delta = 2;
    float alpha = smoothstep(smoothing_radius_out - delta, smoothing_radius_out,
                             dist);
    frag_color = vec4(color_out.xyz, color_out.w - alpha);
  }
)";


inline constexpr const char* kCircleVertexShader = R"(
  #version 410
  layout (location = 0) in vec3 vertex_position;
  uniform mat4 model;
  uniform mat4 view_projection;
  uniform vec4 color;
  uniform float inner_radius;
  uniform float outer_radius;
  out vec4 color_out;
  out vec3 center_out;
  out vec3 position_out;
  out float out_inner_radius;
  out float out_outer_radius;
  void main() {
    color_out = color;
    center_out = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    position_out = (model * vec4(vertex_position, 1.0)).xyz;
    out_inner_radius = inner_radius;
    out_outer_radius = outer_radius;
    gl_Position = view_projection * vec4(position_out, 1.0);
  }
)";

inline constexpr const char* kCircleFragmentShader = R"(
  #version 410
  in vec4 color_out;
  in vec3 center_out;
  in vec3 position_out;
  in float out_inner_radius;
  in float out_outer_radius;
	out vec4 frag_color;
  void main() {
    float dist = length(position_out - center_out);
    if (dist > out_outer_radius || dist < out_inner_radius) discard;
    float delta = 2;
    float alpha = smoothstep(out_outer_radius - delta, out_outer_radius, dist);
    frag_color = vec4(color_out.xyz, color_out.w - alpha);
  }
)";


inline constexpr const char* kFontVertexShader = R"(
  #version 410
  layout (location = 0) in vec4 text_pos;
  uniform mat4 matrix;
  uniform vec4 color;
  out vec4 color_out;
  out vec2 texture_coordinates;
  void main() {
    color_out = color;
    texture_coordinates = text_pos.zw;
    gl_Position = matrix * vec4(text_pos.xy, 0.0 , 1.0);
  }
)";

inline constexpr const char* kFontFragmentShader = R"(
  #version 410
  in vec2 texture_coordinates;
  in vec4 color_out;
  uniform sampler2D basic_texture;
  out vec4 frag_color;
  void main() {
    vec4 texel = texture(basic_texture, texture_coordinates);
    frag_color = vec4(color_out.xyz, texel.x);
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
