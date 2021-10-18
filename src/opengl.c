#include <stdio.h>
#include <string.h>
#include "opengl.h"

#ifdef WIN32
#define GETPROCADDRESS(s)	wglGetProcAddress(s)
#else
#include <GL/glx.h>

#define GETPROCADDRESS(s)	glXGetProcAddress((unsigned char*)s)
#endif

#define LOADPROC(type, func) \
	do { \
		if(!(func = (type)GETPROCADDRESS(#func))) { \
			fputs("failed to load entry point: " #func, stderr); \
			return -1; \
		} \
	} while(0)

int init_opengl(void)
{
	const char *glext, *glver;

	if(!(glver = (const char*)glGetString(GL_VERSION))) {
		fprintf(stderr, "failed to retrieve OpenGL version string\n");
		return -1;
	}
	glcaps.ver_major = 1;
	glcaps.ver_minor = 0;
	sscanf(glver, "%d.%d", &glcaps.ver_major, &glcaps.ver_minor);

	if(!(glext = (const char*)glGetString(GL_EXTENSIONS))) {
		fprintf(stderr, "failed to retrieve OpenGL extensions string\n");
		return -1;
	}

	if(glcaps.ver_major >= 2 || glcaps.ver_minor >= 5 || strstr(glext, "GL_ARB_vertex_buffer_object")) {
		glcaps.vbo = 1;
	}

	if(glcaps.ver_major >= 2 || (strstr(glext, "GL_ARB_vertex_shader") && strstr(glext, "GL_ARB_fragment_shader"))) {
		glcaps.sdr = 1;
	}

	if(glcaps.ver_major >= 3 || (strstr(glext, "GL_EXT_framebuffer_object") || strstr(glext, "GL_ARB_framebuffer_object"))) {
		glcaps.fbo = 1;
	}

	if(glcaps.vbo) {
		LOADPROC(PFNGLGENBUFFERSPROC, glGenBuffers);
		LOADPROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
		LOADPROC(PFNGLBINDBUFFERPROC, glBindBuffer);
		LOADPROC(PFNGLBUFFERDATAPROC, glBufferData);
	}

	if(glcaps.sdr) {
		LOADPROC(PFNGLCREATEPROGRAMPROC, glCreateProgram);
		LOADPROC(PFNGLDELETEPROGRAMPROC, glDeleteProgram);
		LOADPROC(PFNGLATTACHSHADERPROC, glAttachShader);
		LOADPROC(PFNGLLINKPROGRAMPROC, glLinkProgram);
		LOADPROC(PFNGLGETPROGRAMIVPROC, glGetProgramiv);
		LOADPROC(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
		LOADPROC(PFNGLUSEPROGRAMPROC, glUseProgram);
		LOADPROC(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
		LOADPROC(PFNGLUNIFORM1IPROC, glUniform1i);
		LOADPROC(PFNGLUNIFORM1FPROC, glUniform1f);
		LOADPROC(PFNGLUNIFORM2FPROC, glUniform2f);
		LOADPROC(PFNGLUNIFORM3FPROC, glUniform3f);
		LOADPROC(PFNGLUNIFORM4FPROC, glUniform4f);
		LOADPROC(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv);
		LOADPROC(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation);
		LOADPROC(PFNGLVERTEXATTRIB3FPROC, glVertexAttrib3f);
		LOADPROC(PFNGLCREATESHADERPROC, glCreateShader);
		LOADPROC(PFNGLDELETESHADERPROC, glDeleteShader);
		LOADPROC(PFNGLSHADERSOURCEPROC, glShaderSource);
		LOADPROC(PFNGLCOMPILESHADERPROC, glCompileShader);
		LOADPROC(PFNGLGETSHADERIVPROC, glGetShaderiv);
		LOADPROC(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog);
		LOADPROC(PFNGLBINDATTRIBLOCATIONPROC, glBindAttribLocation);
		LOADPROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
		LOADPROC(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray);
		LOADPROC(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
	}

	if(glcaps.fbo) {
		LOADPROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
		LOADPROC(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
		LOADPROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
		LOADPROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
		LOADPROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);
		LOADPROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
		LOADPROC(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
		LOADPROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);
		LOADPROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);
	}

	return 0;
}

int nextpow2(int x)
{
	if(--x < 0) return 0;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}
