#pragma once

#include <cstdint>

// https://www.khronos.org/registry/OpenGL/api/GL/glext.h
// https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h

// GL types.
typedef size_t GLsizeiptr;
typedef char GLchar;
typedef float GLfloat;
typedef uint32_t GLuint;
typedef int GLint;
typedef uint32_t GLenum;
typedef unsigned char GLubyte;
typedef intptr_t GLintptr;

#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_VERSION                        0x1F02
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_FUNC_ADD                       0x8006
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_VERTEX_ARRAY_BINDING           0x85B5
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_STREAM_DRAW                    0x88E0
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GLsizei uint32_t
#define GLboolean bool


// GL functions loaded using wglGetProcAddress.
typedef void glGenBuffers_Func(GLsizei, GLuint*);
static glGenBuffers_Func* glGenBuffers;
typedef void glBindBuffer_Func(GLenum, GLuint);
static glBindBuffer_Func* glBindBuffer;
typedef void glBufferData_Func(GLenum, GLsizeiptr, const void*, GLenum);
static glBufferData_Func* glBufferData;
typedef void glGenVertexArrays_Func(GLsizei, GLuint*);
static glGenVertexArrays_Func* glGenVertexArrays;
typedef void glBindVertexArray_Func(GLuint);
static glBindVertexArray_Func* glBindVertexArray;
typedef void glEnableVertexAttribArray_Func(GLuint);
static glEnableVertexAttribArray_Func* glEnableVertexAttribArray;
typedef void glVertexAttribPointer_Func(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
static glVertexAttribPointer_Func* glVertexAttribPointer;
typedef GLuint glCreateShader_Func(GLenum);
static glCreateShader_Func* glCreateShader;
typedef void glShaderSource_Func(GLuint, GLsizei, const GLchar* const*, const GLint*);
static glShaderSource_Func* glShaderSource;
typedef void glCompileShader_Func(GLuint);
static glCompileShader_Func* glCompileShader;
typedef GLuint glCreateProgram_Func(void);
static glCreateProgram_Func* glCreateProgram;
typedef void glAttachShader_Func(GLuint, GLuint);
static glAttachShader_Func* glAttachShader;
typedef void glLinkProgram_Func(GLuint);
static glLinkProgram_Func* glLinkProgram;
typedef void glUseProgram_Func(GLuint);
static glUseProgram_Func* glUseProgram;
typedef GLint glGetUniformLocation_Func(GLuint, const GLchar*);
static glGetUniformLocation_Func* glGetUniformLocation;
typedef void glUniformMatrix4fv_Func(GLint, GLsizei, GLboolean, const GLfloat*);
static glUniformMatrix4fv_Func* glUniformMatrix4fv;
typedef void glGetShaderInfoLog_Func(GLuint, GLsizei, GLsizei*, GLchar*);
static glGetShaderInfoLog_Func* glGetShaderInfoLog;
typedef void glGetProgramInfoLog_Func(GLuint, GLsizei, GLsizei*, GLchar*);
static glGetProgramInfoLog_Func* glGetProgramInfoLog;
typedef void glGetShaderiv_Func(GLuint, GLenum, GLint*);
static glGetShaderiv_Func* glGetShaderiv;
typedef void glGetProgramiv_Func(GLuint, GLenum, GLint*);
static glGetProgramiv_Func* glGetProgramiv;
typedef void glGetActiveAttrib_Func(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
static glGetActiveAttrib_Func* glGetActiveAttrib;
typedef GLint glGetAttribLocation_Func(GLuint, const GLchar*);
static glGetAttribLocation_Func* glGetAttribLocation;
typedef void glGetActiveUniform_Func(GLuint, GLuint, GLsizei, GLsizei*, GLint*, GLenum*, GLchar*);
static glGetActiveUniform_Func* glGetActiveUniform;
typedef void glUniform4f_Func(GLint, GLfloat, GLfloat, GLfloat, GLfloat);
static glUniform4f_Func* glUniform4f;
typedef void glUniform3f_Func(GLint, GLfloat, GLfloat, GLfloat);
static glUniform3f_Func* glUniform3f;
typedef void glActiveTexture_Func(GLenum);
static glActiveTexture_Func* glActiveTexture;
typedef void glUniform1i_Func(GLint, GLint);
static glUniform1i_Func* glUniform1i;
typedef void glDeleteShader_Func(GLuint);
static glDeleteShader_Func* glDeleteShader;
typedef void glGenFramebuffers_Func(GLsizei, GLuint*);
static glGenFramebuffers_Func* glGenFramebuffers;
typedef void glBindFramebuffer_Func(GLenum, GLuint);
static glBindFramebuffer_Func* glBindFramebuffer;
typedef void glFramebufferTexture_Func(GLenum, GLenum, GLuint, GLint);
static glFramebufferTexture_Func* glFramebufferTexture;
typedef void glDrawBuffers_Func(GLsizei, const GLenum*);
static glDrawBuffers_Func* glDrawBuffers;
typedef void glUniform1f_Func(GLint, GLfloat);
static glUniform1f_Func* glUniform1f;
typedef void glGenerateMipmap_Func(GLenum);
static glGenerateMipmap_Func* glGenerateMipmap;
typedef void glBlendFuncSeparate_Func(GLenum, GLenum, GLenum, GLenum);
static glBlendFuncSeparate_Func* glBlendFuncSeparate;
typedef void glBindTextures_Func(GLuint, GLsizei, const GLuint*);
static glBindTextures_Func* glBindTextures;
//typedef void glGetIntegerv_Func(GLenum, GLint*);
//static glGetIntegerv_Func* glGetIntegerv;
typedef const GLubyte glGetStringi_Func(GLenum, GLuint);
static glGetStringi_Func* glGetStringi;
typedef void glBlendEquation_Func(GLenum);
static glBlendEquation_Func* glBlendEquation;
typedef void glBufferSubData_Func(GLenum, GLintptr, GLsizeiptr, const void*);
static glBufferSubData_Func* glBufferSubData;
typedef void glDeleteVertexArrays_Func(GLsizei, const GLuint*);
static glDeleteVertexArrays_Func* glDeleteVertexArrays;
typedef void glBlendEquationSeparate_Func(GLenum, GLenum);
static glBlendEquationSeparate_Func* glBlendEquationSeparate;
typedef void glDetachShader_Func(GLuint, GLuint);
static glDetachShader_Func* glDetachShader;
typedef void glDeleteBuffers_Func(GLsizei, const GLuint*);
static glDeleteBuffers_Func* glDeleteBuffers;
typedef void glDeleteProgram_Func(GLuint);
static glDeleteProgram_Func* glDeleteProgram;

// STUPID... Fix this.
#undef GLsizei
#undef GLboolean
