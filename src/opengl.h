#ifndef OPENGL_H_
#define OPENGL_H_

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

enum {
	GLCAPS_SDR		= 0x0001,
	GLCAPS_VBO		= 0x0002,
	GLCAPS_FBO		= 0x0004,
	GLCAPS_FB_SRGB	= 0x0008,
	GLCAPS_DEBUG	= 0x8000
};

struct glcaps {
	int ver_major, ver_minor;
	unsigned int caps;
} glcaps;

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

int init_opengl(void);

int nextpow2(int x);

#endif	/* OPENGL_H_ */
