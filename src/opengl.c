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

#ifdef GLDEBUG
static void dbglog(unsigned int src, unsigned int type, unsigned int id,
		unsigned int severity, int length, const char *msg, const void *cls);
#endif

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
		glcaps.caps |= GLCAPS_VBO;

		LOADPROC(PFNGLGENBUFFERSPROC, glGenBuffers);
		LOADPROC(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
		LOADPROC(PFNGLBINDBUFFERPROC, glBindBuffer);
		LOADPROC(PFNGLBUFFERDATAPROC, glBufferData);
	}

	if(glcaps.ver_major >= 2 || (strstr(glext, "GL_ARB_vertex_shader") && strstr(glext, "GL_ARB_fragment_shader"))) {
		glcaps.caps |= GLCAPS_SDR;

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

	if(glcaps.ver_major >= 3 || strstr(glext, "GL_EXT_framebuffer_object") ||
			strstr(glext, "GL_ARB_framebuffer_object")) {
		glcaps.caps |= GLCAPS_FBO;

		LOADPROC(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
		LOADPROC(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers);
		LOADPROC(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer);
		LOADPROC(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D);
		LOADPROC(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer);
		LOADPROC(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers);
		LOADPROC(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers);
		LOADPROC(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer);
		LOADPROC(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage);
		LOADPROC(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus);
		LOADPROC(PFNGLDRAWBUFFERSPROC, glDrawBuffers);
	}

	if((glcaps.ver_major = 3 && glcaps.ver_minor >= 2) || glcaps.ver_major > 3 ||
			strstr(glext, "GL_ARB_framebuffer_sRGB") || strstr(glext, "GL_EXT_framebuffer_sRGB")) {
		glcaps.caps |= GLCAPS_FB_SRGB;
	}

	if((glcaps.ver_major == 2 && glcaps.ver_minor >= 1) || glcaps.ver_major >= 3 ||
			strstr(glext, "GL_EXT_texture_sRGB")) {
		glcaps.caps |= GLCAPS_TEX_SRGB;
	}

	if(glcaps.ver_major >= 3 || strstr(glext, "GL_ARB_texture_float")) {
		glcaps.caps |= GLCAPS_TEX_FLOAT;
	}

	if(strstr(glext, "GL_ARB_debug_output")) {
		glcaps.caps |= GLCAPS_DEBUG;

		LOADPROC(PFNGLDEBUGMESSAGECALLBACKPROC, glDebugMessageCallback);

#ifdef GLDEBUG
		glDebugMessageCallback(dbglog, 0);
		glEnable(GL_DEBUG_OUTPUT);
#endif
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

#ifdef GLDEBUG
static const char *gldebug_srcstr(unsigned int src)
{
	switch(src) {
	case GL_DEBUG_SOURCE_API:
		return "api";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "wsys";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "sdrc";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "3rdparty";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "app";
	case GL_DEBUG_SOURCE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}

static const char *gldebug_typestr(unsigned int type)
{
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		return "error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "deprecated";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "portability warning";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "performance warning";
	case GL_DEBUG_TYPE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}

static void dbglog(unsigned int src, unsigned int type, unsigned int id,
		unsigned int severity, int length, const char *msg, const void *cls)
{
	static const char *fmt = "[GLDEBUG] (%s) %s: %s\n";
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		fprintf(stderr, fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	case GL_DEBUG_TYPE_PORTABILITY:
	case GL_DEBUG_TYPE_PERFORMANCE:
		fprintf(stderr, fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

		/*
	default:
		fprintf(stderr, fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		*/
	}

}
#endif
