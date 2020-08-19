#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glut.h"

#include "gli/gli.hpp"
#include "gli/convert.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "IMAGE.H"

#define PI 3.1415926535897932384626433832795f


static BOOL GLCreateTexture2D(const gli::texture2d &texture, GLuint &tex)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	return TRUE;
}

static BOOL GLCreateTextureCube(const gli::texture_cube &texture, GLuint &tex)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format format = GL.translate(texture.format(), texture.swizzles());

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 0, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 1, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 2, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 3, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 4, 0));
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format.Internal, texture.extent().x, texture.extent().y, 0, format.External, format.Type, texture.data(0, 5, 0));
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return TRUE;
}

static void GLDestroyTexture(GLuint tex)
{
	glDeleteTextures(1, &tex);
}


static GLuint rbo = 0;
static GLuint fbo = 0;
static GLuint fboTexture = 0;
static GLuint fboTextureWidth = 0;
static GLuint fboTextureHeight = 0;

static BOOL GLCreateFBO(int width, int height, gli::format format)
{
	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(format);

	fboTextureWidth = width;
	fboTextureHeight = height;

	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, glFormat.Internal, fboTextureWidth, fboTextureHeight, 0, glFormat.External, glFormat.Type, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, fboTextureWidth, fboTextureHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ? TRUE : FALSE;
}

static void GLDestroyFBO(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);

	rbo = 0;
	fbo = 0;
	fboTexture = 0;
	fboTextureWidth = 0;
	fboTextureHeight = 0;
}


static GLuint program = 0;
static GLuint attribLocationPosition = 0;
static GLuint attribLocationTexcoord = 0;
static GLuint uniformLocationTexture = 0;
static GLuint uniformLocationTexcoordMatrix = 0;
static GLuint uniformLocationModelViewProjectionMatrix = 0;
static GLuint uniformLocationSHRed = 0;
static GLuint uniformLocationSHGrn = 0;
static GLuint uniformLocationSHBlu = 0;
static GLuint uniformLocationSamples = 0;
static GLuint uniformLocationRoughness = 0;
static GLuint uniformLocationEnvmap = 0;
static GLuint uniformLocationCubemap = 0;

static BOOL GLCreateProgram(const char *szShaderVertexCode, const char *szShaderFragmentCode)
{
	GLint linked;
	GLint compiled;
	GLuint vertex;
	GLuint fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &szShaderVertexCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint len;
		GLchar szError[4 * 1024];
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &len);
		glGetShaderInfoLog(vertex, sizeof(szError), &len, szError);
		glDeleteShader(vertex);
		printf("Vertex Error: %s\n", szError);
		vertex = 0;
		return FALSE;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &szShaderFragmentCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		GLint len;
		GLchar szError[4 * 1024];
		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &len);
		glGetShaderInfoLog(fragment, sizeof(szError), &len, szError);
		glDeleteShader(fragment);
		printf("Fragment Error: %s\n", szError);
		fragment = 0;
		return FALSE;
	}

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE) return FALSE;

	attribLocationPosition = glGetAttribLocation(program, "_position");
	attribLocationTexcoord = glGetAttribLocation(program, "_texcoord");
	uniformLocationTexture = glGetUniformLocation(program, "_texture");
	uniformLocationTexcoordMatrix = glGetUniformLocation(program, "_texcoordMatrix");
	uniformLocationModelViewProjectionMatrix = glGetUniformLocation(program, "_modelViewProjectionMatrix");
	uniformLocationSHRed = glGetUniformLocation(program, "_sh_red");
	uniformLocationSHGrn = glGetUniformLocation(program, "_sh_grn");
	uniformLocationSHBlu = glGetUniformLocation(program, "_sh_blu");
	uniformLocationSamples = glGetUniformLocation(program, "_samples");
	uniformLocationRoughness = glGetUniformLocation(program, "_roughness");
	uniformLocationEnvmap = glGetUniformLocation(program, "_envmap");
	uniformLocationCubemap = glGetUniformLocation(program, "_cubemap");

	return TRUE;
}

static void GLDestroyProgram(void)
{
	glUseProgram(0);
	glDeleteProgram(program);

	program = 0;
	attribLocationPosition = 0;
	attribLocationTexcoord = 0;
	uniformLocationTexture = 0;
	uniformLocationTexcoordMatrix = 0;
	uniformLocationModelViewProjectionMatrix = 0;
	uniformLocationSHRed = 0;
	uniformLocationSHGrn = 0;
	uniformLocationSHBlu = 0;
}


static GLuint ibo = 0;
static GLuint vbo = 0;
typedef struct vertex { float position[3]; float texcoord[2]; } vertex;

static BOOL GLCreateVBO(const vertex *vertices, int numVertices, const unsigned short *indices, int numIndices)
{
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * numVertices, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * numIndices, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return TRUE;
}

static void GLDestroyVBO(void)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);

	ibo = 0;
	vbo = 0;
}

static BOOL RenderIrradianceMap4(gli::texture_cube &texture, float *sh_red, float *sh_grn, float *sh_blu)
{
	static const GLchar *szShaderVertexCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform mat4 _modelViewProjectionMatrix;                                                \n\
																									\n\
			attribute vec3 _position;                                                               \n\
			attribute vec4 _texcoord;                                                               \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				gl_Position = _modelViewProjectionMatrix*vec4(_position, 1.0);                      \n\
				texcoord = _texcoord;                                                               \n\
			}                                                                                       \n\
		";

	static const GLchar *szShaderFragmentCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform float _sh_red[4];                                                               \n\
			uniform float _sh_grn[4];                                                               \n\
			uniform float _sh_blu[4];                                                               \n\
																									\n\
			uniform mat4 _texcoordMatrix;                                                           \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			vec3 SH(vec3 direction)                                                                 \n\
			{                                                                                       \n\
				float basis[4];                                                                     \n\
																									\n\
				float x = direction.x;                                                              \n\
				float y = direction.y;                                                              \n\
				float z = direction.z;                                                              \n\
																									\n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				basis[0] = 1.0f;                                                                    \n\
																									\n\
				basis[1] = y;                                                                       \n\
				basis[2] = z;                                                                       \n\
				basis[3] = x;                                                                       \n\
																									\n\
				for (int index = 0; index < 4; index++)                                             \n\
				{                                                                                   \n\
					color.r += _sh_red[index] * basis[index];                                       \n\
					color.g += _sh_grn[index] * basis[index];                                       \n\
					color.b += _sh_blu[index] * basis[index];                                       \n\
				}                                                                                   \n\
																									\n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec4 direction = _texcoordMatrix * vec4(texcoord.x, texcoord.y, 1.0f, 0.0f);        \n\
				direction.xyz = normalize(direction.xyz);                                           \n\
																									\n\
				gl_FragColor.rgb = SH(direction.xyz);                                               \n\
				gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(2.2f));                               \n\
			}                                                                                       \n\
		";

	static const vertex vertices[4] = {
		{ { -1.0f, -1.0f, 0.0f },{ -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, 0.0f },{  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, 0.0f },{  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ -1.0f,  1.0f } },
	};
	static const unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	BOOL rcode = TRUE;

	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(texture.format());

	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateFBO(texture.extent().x, texture.extent().y, texture.format()) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		glm::mat4 matModeViewProjection = matProjection * matModeView;
		glm::mat4 matTexcoords[6] = {
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::mat4(),
			glm::rotate(glm::mat4(),  PI, glm::vec3(0.0f, 1.0f, 0.0f)),
		};

		glViewport(0, 0, texture.extent().x, texture.extent().y);
		glUseProgram(program);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glEnableVertexAttribArray(attribLocationPosition);
		glEnableVertexAttribArray(attribLocationTexcoord);
		{
			glVertexAttribPointer(attribLocationPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
			glVertexAttribPointer(attribLocationTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)12);

			glUniformMatrix4fv(uniformLocationModelViewProjectionMatrix, 1, GL_FALSE, (const float *)&matModeViewProjection);
			glUniform1fv(uniformLocationSHRed, 4, sh_red);
			glUniform1fv(uniformLocationSHGrn, 4, sh_grn);
			glUniform1fv(uniformLocationSHBlu, 4, sh_blu);

			for (int index = 0; index < 6; index++)
			{
				glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
				glReadPixels(0, 0, texture.extent().x, texture.extent().y, glFormat.External, glFormat.Type, texture.data(0, index, 0));
			}
		}
		glDisableVertexAttribArray(attribLocationPosition);
		glDisableVertexAttribArray(attribLocationTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	texture = gli::flip(texture);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();

	return rcode;
}

static BOOL RenderIrradianceMap9(gli::texture_cube &texture, float *sh_red, float *sh_grn, float *sh_blu)
{
	static const GLchar *szShaderVertexCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform mat4 _modelViewProjectionMatrix;                                                \n\
																									\n\
			attribute vec3 _position;                                                               \n\
			attribute vec4 _texcoord;                                                               \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				gl_Position = _modelViewProjectionMatrix*vec4(_position, 1.0);                      \n\
				texcoord = _texcoord;                                                               \n\
			}                                                                                       \n\
		";

	static const GLchar *szShaderFragmentCode =
		"                                                                                           \n\
			#version 330                                                                            \n\
																									\n\
			uniform float _sh_red[9];                                                               \n\
			uniform float _sh_grn[9];                                                               \n\
			uniform float _sh_blu[9];                                                               \n\
																									\n\
			uniform mat4 _texcoordMatrix;                                                           \n\
																									\n\
			varying vec4 texcoord;                                                                  \n\
																									\n\
			vec3 SH(vec3 direction)                                                                 \n\
			{                                                                                       \n\
				float basis[9];                                                                     \n\
																									\n\
				float x = direction.x;                                                              \n\
				float y = direction.y;                                                              \n\
				float z = direction.z;                                                              \n\
																									\n\
				vec3 color = vec3(0.0f, 0.0f, 0.0f);                                                \n\
																									\n\
				basis[0] = 1.0f;                                                                    \n\
																									\n\
				basis[1] = y;                                                                       \n\
				basis[2] = z;                                                                       \n\
				basis[3] = x;                                                                       \n\
																									\n\
				basis[4] = (x * y);                                         						\n\
				basis[5] = (y * z);                                                                 \n\
				basis[6] = (z * z * 3.0f - 1.0f);                                                   \n\
				basis[7] = (x * z);                                                                 \n\
				basis[8] = (x * x - y * y);                                                         \n\
																									\n\
				for (int index = 0; index < 9; index++)                                             \n\
				{                                                                                   \n\
					color.r += _sh_red[index] * basis[index];                                       \n\
					color.g += _sh_grn[index] * basis[index];                                       \n\
					color.b += _sh_blu[index] * basis[index];                                       \n\
				}                                                                                   \n\
																									\n\
				return color;                                                                       \n\
			}                                                                                       \n\
																									\n\
			void main()                                                                             \n\
			{                                                                                       \n\
				vec4 direction = _texcoordMatrix * vec4(texcoord.x, texcoord.y, 1.0f, 0.0f);        \n\
				direction.xyz = normalize(direction.xyz);                                           \n\
																									\n\
				gl_FragColor.rgb = SH(direction.xyz);                                               \n\
				gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(2.2f));                               \n\
			}                                                                                       \n\
		";

	static const vertex vertices[4] = {
		{ { -1.0f, -1.0f, 0.0f },{ -1.0f, -1.0f } },
		{ {  1.0f, -1.0f, 0.0f },{  1.0f, -1.0f } },
		{ {  1.0f,  1.0f, 0.0f },{  1.0f,  1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ -1.0f,  1.0f } },
	};
	static const unsigned short indices[6] = { 0, 1, 2, 2, 3, 0 };

	BOOL rcode = TRUE;

	gli::gl GL(gli::gl::PROFILE_ES30);
	gli::gl::format glFormat = GL.translate(texture.format());

	if (GLCreateVBO(vertices, 4, indices, 6) == FALSE) goto ERR;
	if (GLCreateFBO(texture.extent().x, texture.extent().y, texture.format()) == FALSE) goto ERR;
	if (GLCreateProgram(szShaderVertexCode, szShaderFragmentCode) == FALSE) goto ERR;
	{
		glm::mat4 matModeView = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 matProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
		glm::mat4 matModeViewProjection = matProjection * matModeView;
		glm::mat4 matTexcoords[6] = {
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)),
			glm::rotate(glm::mat4(), -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::rotate(glm::mat4(),  PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::mat4(),
			glm::rotate(glm::mat4(),  PI, glm::vec3(0.0f, 1.0f, 0.0f)),
		};

		glViewport(0, 0, texture.extent().x, texture.extent().y);
		glUseProgram(program);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glEnableVertexAttribArray(attribLocationPosition);
		glEnableVertexAttribArray(attribLocationTexcoord);
		{
			glVertexAttribPointer(attribLocationPosition, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
			glVertexAttribPointer(attribLocationTexcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)12);

			glUniformMatrix4fv(uniformLocationModelViewProjectionMatrix, 1, GL_FALSE, (const float *)&matModeViewProjection);
			glUniform1fv(uniformLocationSHRed, 9, sh_red);
			glUniform1fv(uniformLocationSHGrn, 9, sh_grn);
			glUniform1fv(uniformLocationSHBlu, 9, sh_blu);

			for (int index = 0; index < 6; index++)
			{
				glUniformMatrix4fv(uniformLocationTexcoordMatrix, 1, GL_FALSE, (const float *)&matTexcoords[index]);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
				glReadPixels(0, 0, texture.extent().x, texture.extent().y, glFormat.External, glFormat.Type, texture.data(0, index, 0));
			}
		}
		glDisableVertexAttribArray(attribLocationPosition);
		glDisableVertexAttribArray(attribLocationTexcoord);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	texture = gli::flip(texture);

	goto RET;
ERR:
	rcode = FALSE;
RET:
	GLDestroyVBO();
	GLDestroyFBO();
	GLDestroyProgram();

	return rcode;
}

static glm::f32vec3 GetTexturePixelColor(const gli::texture_cube &texture, int x, int y, int face)
{
	return texture.load<glm::f32vec3>(gli::texture_cube::extent_type(x, y), face, 0);
}

static glm::f32vec3 GetTexturePixelColor(const gli::texture2d &texture, int x, int y)
{
	return texture.load<glm::f32vec3>(gli::texture2d::extent_type(x, y), 0);
}

static void SetTexturePixelColor(gli::texture2d &texture, int x, int y, int level, const glm::f32vec3 &color)
{
	texture.store(gli::extent2d(x, y), level, color);
}

static gli::texture2d Preview(const gli::texture_cube &cube)
{
	#define TEXTURE_CUBE_MAP_POSITIVE_X 0
	#define TEXTURE_CUBE_MAP_NEGATIVE_X 1
	#define TEXTURE_CUBE_MAP_POSITIVE_Y 2
	#define TEXTURE_CUBE_MAP_NEGATIVE_Y 3
	#define TEXTURE_CUBE_MAP_POSITIVE_Z 4
	#define TEXTURE_CUBE_MAP_NEGATIVE_Z 5

	//     +Y
	// -X  +Z  +X  -Z
	//     -Y

	gli::texture2d texture(cube.format(), gli::extent2d(cube.extent().x * 4, cube.extent().y * 3));

	for (int y = 0; y < cube.extent().y; y++) {
		for (int x = 0; x < cube.extent().x; x++) {
			// TEXTURE_CUBE_MAP_POSITIVE_X
			{
				int xx = x + cube.extent().x * 2;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_X);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_X
			{
				int xx = x + cube.extent().x * 0;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_X);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_POSITIVE_Y
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 0;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_Y);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_Y
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 2;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_Y);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_POSITIVE_Z
			{
				int xx = x + cube.extent().x * 1;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_POSITIVE_Z);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
			// TEXTURE_CUBE_MAP_NEGATIVE_Z
			{
				int xx = x + cube.extent().x * 3;
				int yy = y + cube.extent().y * 1;
				gli::f32vec3 color = GetTexturePixelColor(cube, x, y, TEXTURE_CUBE_MAP_NEGATIVE_Z);
				SetTexturePixelColor(texture, xx, yy, 0, color);
			}
		}
	}

	return texture;
}

void Preview4(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu)
{
	gli::texture_cube texture(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(64, 64));
	RenderIrradianceMap4(texture, sh_red, sh_grn, sh_blu);
	gli::texture2d preview = Preview(texture);

	IMAGE image;
	IMAGE_ZeroImage(&image);
	IMAGE_AllocImage(&image, preview.extent().x, preview.extent().y, 24);
	for (int y = 0; y < IMAGE_HEIGHT(&image); y++) {
		for (int x = 0; x < IMAGE_WIDTH(&image); x++) {
			glm::f32vec3 color = GetTexturePixelColor(preview, x, y);

			int r = (int)(color.r * 255 + 0.5f);
			int g = (int)(color.g * 255 + 0.5f);
			int b = (int)(color.b * 255 + 0.5f);
			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			IMAGE_SetPixelColor(&image, x, y, RGB(r, g, b));
		}
	}
	IMAGE_SaveJpg((char * const)szFileName, &image, 80);
	IMAGE_FreeImage(&image);
}

void Preview9(const char *szFileName, float *sh_red, float *sh_grn, float *sh_blu)
{
	gli::texture_cube texture(gli::FORMAT_RGB32_SFLOAT_PACK32, gli::extent2d(64, 64));
	RenderIrradianceMap9(texture, sh_red, sh_grn, sh_blu);
	gli::texture2d preview = Preview(texture);

	IMAGE image;
	IMAGE_ZeroImage(&image);
	IMAGE_AllocImage(&image, preview.extent().x, preview.extent().y, 24);
	for (int y = 0; y < IMAGE_HEIGHT(&image); y++) {
		for (int x = 0; x < IMAGE_WIDTH(&image); x++) {
			glm::f32vec3 color = GetTexturePixelColor(preview, x, y);

			int r = (int)(color.r * 255 + 0.5f);
			int g = (int)(color.g * 255 + 0.5f);
			int b = (int)(color.b * 255 + 0.5f);
			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			IMAGE_SetPixelColor(&image, x, y, RGB(r, g, b));
		}
	}
	IMAGE_SaveJpg((char * const)szFileName, &image, 80);
	IMAGE_FreeImage(&image);
}
