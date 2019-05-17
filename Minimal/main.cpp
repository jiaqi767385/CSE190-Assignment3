/************************************************************************************

Authors     :   Bradley Austin Davis <bdavis@saintandreas.org>
Copyright   :   Copyright Brad Davis. All Rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>

#include <Windows.h>

#define __STDC_FORMAT_MACROS 1

#define FAIL(X) throw std::runtime_error(X)

///////////////////////////////////////////////////////////////////////////////
//
// GLM is a C++ math library meant to mirror the syntax of GLSL 
//

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// Import the most commonly used types into the default namespace
using glm::ivec3;
using glm::ivec2;
using glm::uvec2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::quat;

///////////////////////////////////////////////////////////////////////////////
//
// GLEW gives cross platform access to OpenGL 3.x+ functionality.  
//

#include <GL/glew.h>

bool checkFramebufferStatus(GLenum target = GL_FRAMEBUFFER) {
	GLuint status = glCheckFramebufferStatus(target);
	switch (status) {
	case GL_FRAMEBUFFER_COMPLETE:
		return true;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		std::cerr << "framebuffer incomplete attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		std::cerr << "framebuffer missing attachment" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		std::cerr << "framebuffer incomplete draw buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		std::cerr << "framebuffer incomplete read buffer" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		std::cerr << "framebuffer incomplete multisample" << std::endl;
		break;

	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
		std::cerr << "framebuffer incomplete layer targets" << std::endl;
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		std::cerr << "framebuffer unsupported internal format or image" << std::endl;
		break;

	default:
		std::cerr << "other framebuffer error" << std::endl;
		break;
	}

	return false;
}

bool checkGlError() {
	GLenum error = glGetError();
	if (!error) {
		return false;
	}
	else {
		switch (error) {
		case GL_INVALID_ENUM:
			std::cerr << ": An unacceptable value is specified for an enumerated argument.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			std::cerr << ": A numeric argument is out of range.The offending command is ignored and has no other side effect than to set the error flag";
			break;
		case GL_INVALID_OPERATION:
			std::cerr << ": The specified operation is not allowed in the current state.The offending command is ignored and has no other side effect than to set the error flag..";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			std::cerr << ": The framebuffer object is not complete.The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			std::cerr << ": There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_STACK_UNDERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to underflow.";
			break;
		case GL_STACK_OVERFLOW:
			std::cerr << ": An attempt has been made to perform an operation that would cause an internal stack to overflow.";
			break;
		}
		return true;
	}
}

void glDebugCallbackHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid* data) {
	OutputDebugStringA(msg);
	std::cout << "debug call: " << msg << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// GLFW provides cross platform window creation
//

#include <GLFW/glfw3.h>

namespace glfw {
	inline GLFWwindow * createWindow(const uvec2 & size, const ivec2 & position = ivec2(INT_MIN)) {
		GLFWwindow * window = glfwCreateWindow(size.x, size.y, "glfw", nullptr, nullptr);
		if (!window) {
			FAIL("Unable to create rendering window");
		}
		if ((position.x > INT_MIN) && (position.y > INT_MIN)) {
			glfwSetWindowPos(window, position.x, position.y);
		}
		return window;
	}
}

// A class to encapsulate using GLFW to handle input and render a scene
class GlfwApp {

protected:
	uvec2 windowSize;
	ivec2 windowPosition;
	GLFWwindow * window{ nullptr };
	unsigned int frame{ 0 };

public:
	GlfwApp() {
		// Initialize the GLFW system for creating and positioning windows
		if (!glfwInit()) {
			FAIL("Failed to initialize GLFW");
		}
		glfwSetErrorCallback(ErrorCallback);
	}

	virtual ~GlfwApp() {
		if (nullptr != window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}

	virtual int run() {
		preCreate();

		window = createRenderingTarget(windowSize, windowPosition);

		if (!window) {
			std::cout << "Unable to create OpenGL window" << std::endl;
			return -1;
		}

		postCreate();

		initGl();

		while (!glfwWindowShouldClose(window)) {
			++frame;
			glfwPollEvents();
			update();
			draw();
			finishFrame();
		}

		shutdownGl();

		return 0;
	}


protected:
	virtual GLFWwindow * createRenderingTarget(uvec2 & size, ivec2 & pos) = 0;

	virtual void draw() = 0;

	void preCreate() {
		glfwWindowHint(GLFW_DEPTH_BITS, 16);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
	}


	void postCreate() {
		glfwSetWindowUserPointer(window, this);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwMakeContextCurrent(window);

		// Initialize the OpenGL bindings
		// For some reason we have to set this experminetal flag to properly
		// init GLEW if we use a core context.
		glewExperimental = GL_TRUE;
		if (0 != glewInit()) {
			FAIL("Failed to initialize GLEW");
		}
		glGetError();

		if (GLEW_KHR_debug) {
			GLint v;
			glGetIntegerv(GL_CONTEXT_FLAGS, &v);
			if (v & GL_CONTEXT_FLAG_DEBUG_BIT) {
				//glDebugMessageCallback(glDebugCallbackHandler, this);
			}
		}
	}

	virtual void initGl() {
	}

	virtual void shutdownGl() {
	}

	virtual void finishFrame() {
		glfwSwapBuffers(window);
	}

	virtual void destroyWindow() {
		glfwSetKeyCallback(window, nullptr);
		glfwSetMouseButtonCallback(window, nullptr);
		glfwDestroyWindow(window);
	}

	virtual void onKey(int key, int scancode, int action, int mods) {
		if (GLFW_PRESS != action) {
			return;
		}

		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, 1);
			return;
		}
	}

	virtual void update() {}

	virtual void onMouseButton(int button, int action, int mods) {}

protected:
	virtual void viewport(const ivec2 & pos, const uvec2 & size) {
		glViewport(pos.x, pos.y, size.x, size.y);
	}

private:

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onKey(key, scancode, action, mods);
	}

	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		GlfwApp * instance = (GlfwApp *)glfwGetWindowUserPointer(window);
		instance->onMouseButton(button, action, mods);
	}

	static void ErrorCallback(int error, const char* description) {
		FAIL(description);
	}
};

//////////////////////////////////////////////////////////////////////
//
// The Oculus VR C API provides access to information about the HMD
//

#include <OVR_CAPI.h>
#include <OVR_CAPI_GL.h>

namespace ovr {

	// Convenience method for looping over each eye with a lambda
	template <typename Function>
	inline void for_each_eye(Function function) {
		for (ovrEyeType eye = ovrEyeType::ovrEye_Left;
			eye < ovrEyeType::ovrEye_Count;
			eye = static_cast<ovrEyeType>(eye + 1)) {
			function(eye);
		}
	}

	inline mat4 toGlm(const ovrMatrix4f & om) {
		return glm::transpose(glm::make_mat4(&om.M[0][0]));
	}

	inline mat4 toGlm(const ovrFovPort & fovport, float nearPlane = 0.01f, float farPlane = 10000.0f) {
		return toGlm(ovrMatrix4f_Projection(fovport, nearPlane, farPlane, true));
	}

	inline vec3 toGlm(const ovrVector3f & ov) {
		return glm::make_vec3(&ov.x);
	}

	inline vec2 toGlm(const ovrVector2f & ov) {
		return glm::make_vec2(&ov.x);
	}

	inline uvec2 toGlm(const ovrSizei & ov) {
		return uvec2(ov.w, ov.h);
	}

	inline quat toGlm(const ovrQuatf & oq) {
		return glm::make_quat(&oq.x);
	}

	inline mat4 toGlm(const ovrPosef & op) {
		mat4 orientation = glm::mat4_cast(toGlm(op.Orientation));
		mat4 translation = glm::translate(mat4(), ovr::toGlm(op.Position));
		return translation * orientation;
	}

	inline ovrMatrix4f fromGlm(const mat4 & m) {
		ovrMatrix4f result;
		mat4 transposed(glm::transpose(m));
		memcpy(result.M, &(transposed[0][0]), sizeof(float) * 16);
		return result;
	}

	inline ovrVector3f fromGlm(const vec3 & v) {
		ovrVector3f result;
		result.x = v.x;
		result.y = v.y;
		result.z = v.z;
		return result;
	}

	inline ovrVector2f fromGlm(const vec2 & v) {
		ovrVector2f result;
		result.x = v.x;
		result.y = v.y;
		return result;
	}

	inline ovrSizei fromGlm(const uvec2 & v) {
		ovrSizei result;
		result.w = v.x;
		result.h = v.y;
		return result;
	}

	inline ovrQuatf fromGlm(const quat & q) {
		ovrQuatf result;
		result.x = q.x;
		result.y = q.y;
		result.z = q.z;
		result.w = q.w;
		return result;
	}
}

class RiftManagerApp {
protected:
	ovrSession _session;
	ovrHmdDesc _hmdDesc;
	ovrGraphicsLuid _luid;

public:
	RiftManagerApp() {
		if (!OVR_SUCCESS(ovr_Create(&_session, &_luid))) {
			FAIL("Unable to create HMD session");
		}

		_hmdDesc = ovr_GetHmdDesc(_session);
	}

	~RiftManagerApp() {
		ovr_Destroy(_session);
		_session = nullptr;
	}
};

class RiftApp : public GlfwApp, public RiftManagerApp {
public:

private:
	GLuint _fbo{ 0 };
	GLuint _depthBuffer{ 0 };
	ovrTextureSwapChain _eyeTexture;

	GLuint _mirrorFbo{ 0 };
	ovrMirrorTexture _mirrorTexture;

	ovrEyeRenderDesc _eyeRenderDescs[2];

	mat4 _eyeProjections[2];

	ovrLayerEyeFov _sceneLayer;
	ovrViewScaleDesc _viewScaleDesc;

	uvec2 _renderTargetSize;
	uvec2 _mirrorSize;

	// Default Eye Offset
	float defaultHmdToEyeOffset[2]; 

public:

	RiftApp() {
		using namespace ovr;
		_viewScaleDesc.HmdSpaceToWorldScaleInMeters = 1.0f;

		memset(&_sceneLayer, 0, sizeof(ovrLayerEyeFov));
		_sceneLayer.Header.Type = ovrLayerType_EyeFov;
		_sceneLayer.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;

		ovr::for_each_eye([&](ovrEyeType eye) {
			ovrEyeRenderDesc& erd = _eyeRenderDescs[eye] = ovr_GetRenderDesc(_session, eye, _hmdDesc.DefaultEyeFov[eye]);
			ovrMatrix4f ovrPerspectiveProjection =
				ovrMatrix4f_Projection(erd.Fov, 0.01f, 1000.0f, ovrProjection_ClipRangeOpenGL);
			_eyeProjections[eye] = ovr::toGlm(ovrPerspectiveProjection);
			_viewScaleDesc.HmdToEyeOffset[eye] = erd.HmdToEyeOffset;

			// Default Eye Offset
			defaultHmdToEyeOffset[eye] = _viewScaleDesc.HmdToEyeOffset[eye].x;

			ovrFovPort & fov = _sceneLayer.Fov[eye] = _eyeRenderDescs[eye].Fov;
			auto eyeSize = ovr_GetFovTextureSize(_session, eye, fov, 1.0f);
			_sceneLayer.Viewport[eye].Size = eyeSize;
			_sceneLayer.Viewport[eye].Pos = { (int)_renderTargetSize.x, 0 };

			_renderTargetSize.y = std::max(_renderTargetSize.y, (uint32_t)eyeSize.h);
			_renderTargetSize.x += eyeSize.w;
		});
		// Make the on screen window 1/4 the resolution of the render target
		_mirrorSize = _renderTargetSize;
		_mirrorSize /= 4;
	}

protected:
	GLFWwindow * createRenderingTarget(uvec2 & outSize, ivec2 & outPosition) override {
		return glfw::createWindow(_mirrorSize);
	}

	void initGl() override {
		GlfwApp::initGl();

		// Disable the v-sync for buffer swap
		glfwSwapInterval(0);

		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = _renderTargetSize.x;
		desc.Height = _renderTargetSize.y;
		desc.MipLevels = 1;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.SampleCount = 1;
		desc.StaticImage = ovrFalse;
		ovrResult result = ovr_CreateTextureSwapChainGL(_session, &desc, &_eyeTexture);
		_sceneLayer.ColorTexture[0] = _eyeTexture;
		if (!OVR_SUCCESS(result)) {
			FAIL("Failed to create swap textures");
		}

		int length = 0;
		result = ovr_GetTextureSwapChainLength(_session, _eyeTexture, &length);
		if (!OVR_SUCCESS(result) || !length) {
			FAIL("Unable to count swap chain textures");
		}
		for (int i = 0; i < length; ++i) {
			GLuint chainTexId;
			ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, i, &chainTexId);
			glBindTexture(GL_TEXTURE_2D, chainTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		// Set up the framebuffer object
		glGenFramebuffers(1, &_fbo);
		glGenRenderbuffers(1, &_depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _renderTargetSize.x, _renderTargetSize.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		ovrMirrorTextureDesc mirrorDesc;
		memset(&mirrorDesc, 0, sizeof(mirrorDesc));
		mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		mirrorDesc.Width = _mirrorSize.x;
		mirrorDesc.Height = _mirrorSize.y;
		if (!OVR_SUCCESS(ovr_CreateMirrorTextureGL(_session, &mirrorDesc, &_mirrorTexture))) {
			FAIL("Could not create mirror texture");
		}
		glGenFramebuffers(1, &_mirrorFbo);
	}

	void onKey(int key, int scancode, int action, int mods) override {
		if (GLFW_PRESS == action) switch (key) {
		case GLFW_KEY_R:
			ovr_RecenterTrackingOrigin(_session);
			return;
		}

		GlfwApp::onKey(key, scancode, action, mods);
	}

	ovrPosef prevEye[2], currEye[2];
	bool initEye[2] = {false, false};

	void draw() final override {

		ovrPosef eyePoses[2];
		ovr_GetEyePoses(_session, frame, true, _viewScaleDesc.HmdToEyeOffset, eyePoses, &_sceneLayer.SensorSampleTime);
		int curIndex;
		ovr_GetTextureSwapChainCurrentIndex(_session, _eyeTexture, &curIndex);
		GLuint curTexId;
		ovr_GetTextureSwapChainBufferGL(_session, _eyeTexture, curIndex, &curTexId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ovr::for_each_eye([&](ovrEyeType eye) {
	
			// Init Eye
			if (!initEye[eye]) {
				prevEye[eye] = eyePoses[eye];
				initEye[eye] = true;
			}

			// 0 for non-Freeze and 1 for Freeze Mode
			currEye[eye] = prevEye[eye];
			
			if (FreezeMode() == 0) {
				currEye[eye].Position = eyePoses[eye].Position;
			}

			prevEye[eye] = currEye[eye];
			
			// Current Eye
			currentEye(eye);

			const auto& vp = _sceneLayer.Viewport[eye];
			glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
			_sceneLayer.RenderPose[eye] = eyePoses[eye];

			glm::vec3 eyePos = glm::vec3(currEye[eye].Position.x, currEye[eye].Position.y, currEye[eye].Position.z);
			offscreenRender(_eyeProjections[eye], ovr::toGlm(currEye[eye]), _fbo, vp, eyePos);
			glm::vec3 origEyePos = glm::vec3(eyePoses[eye].Position.x, eyePoses[eye].Position.y, eyePoses[eye].Position.z);
			// Render scene
			renderScene(_eyeProjections[eye], ovr::toGlm(eyePoses[eye]), origEyePos);
			
		});
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		ovr_CommitTextureSwapChain(_session, _eyeTexture);
		ovrLayerHeader* headerList = &_sceneLayer.Header;
		ovr_SubmitFrame(_session, frame, &_viewScaleDesc, &headerList, 1);

		GLuint mirrorTextureId;
		ovr_GetMirrorTextureBufferGL(_session, _mirrorTexture, &mirrorTextureId);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _mirrorFbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrorTextureId, 0);
		glBlitFramebuffer(0, 0, _mirrorSize.x, _mirrorSize.y, 0, _mirrorSize.y, _mirrorSize.x, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}

	// Get Default Eye Index
	float getDefaultIOD(int eyeIdx) { 

		return defaultHmdToEyeOffset[eyeIdx]; 
	}

	virtual void offscreenRender(const glm::mat4 & projection, const glm::mat4 & headPose, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) = 0;

	virtual void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & eyePos) = 0;

	virtual void currentEye(ovrEyeType eye) = 0;

	virtual int FreezeMode() = 0;
	
};

//////////////////////////////////////////////////////////////////////
//
// The remainder of this code is specific to the scene we want to 
// render.  I use oglplus to render an array of cubes, but your 
// application would perform whatever rendering you want
//


#include <time.h>
#include "Shader.h"
#include "Cube.h"
#include "TexturedCube.h"
#include "Skybox.h"
#include "Cave.h"
#include "Line.h"
#include <vector>
#include "Model.h"
#include "Mesh.h"

class Cursor {

	// Shader ID
	GLuint shaderID;

	// Cursor
	std::unique_ptr<Model> cursor;

public:

	// User's Dominant Hand's Controller Position 
	glm::vec3 position;

	Cursor() {
		shaderID = LoadShaders("cursor.vert", "cursor.frag");
		cursor = std::make_unique<Model>("webtrcc.obj");
	}

	/* Render sphere at User's Dominant Hand's Controller Position */
	void render(const glm::mat4& projection, const glm::mat4& view) {
		glm::mat4 toWorld = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), glm::vec3(0.005f));
		cursor->Draw(shaderID, projection, view, toWorld);
	}

};

class Scene {
	
	// Cave
	std::unique_ptr<Cave> cave;
	
	// Skybox
	Skybox * skybox; // current skybox
	Skybox * lefteye_skybox; // skybox for left eye
	Skybox * righteye_skybox; // skybox for right eye
	std::unique_ptr<Skybox> self_skybox; // customized skybox

	// Lines
	std::vector<Line* > LLines;
	std::vector<Line* > RLines;

	// Dots
	std::unique_ptr<Cursor> LeftEyeCursor;
	std::unique_ptr<Cursor> RightEyeCursor;
	std::unique_ptr<Cursor> ControllerCursor;
	
	// ShaderID
	GLint shaderID, skyboxShaderID, lineShaderID;
	
public:

	// Buttons
	bool buttonAPressed, buttonBPressed, buttonXPressed;

	// HandTrigger
	bool RHTriggerPressed;
	int buttonB, buttonX;

	// Cube
	std::unique_ptr<TexturedCube> cube;
	// Cube Size and Position
	float cubeSize;
	glm::vec3 cubePos;

	// Currenr Eye Index : 0 for LEFT eye, 1 for RIGHT eye
	int curEyeIdx;

	// Frame Buffers
	GLuint lFBO, lrenderedTexture, lRBO; // LEFT
	GLuint rFBO, rrenderedTexture, rRBO; // RIGHT
	GLuint bFBO, brenderedTexture, bRBO; // BUTTOM

	// EXTRA CREDIT 1
	int randNum;
	bool randNumGenerated;

	Scene() {

		srand(time(0));

		// Buttons
		buttonAPressed = false;
		buttonBPressed = false;
		buttonXPressed = false;

		// Hand Trigger
		RHTriggerPressed = false;

		// 0 for Non-Freeze Mode and 1 for Freeze Mode
		buttonB = 0;

		// 0 for Normal mode and 1 for disabling a random projector
		buttonX = 0;

		// EXTRA CREDIT 1
		randNum = rand() % 6;
		randNumGenerated = false;

		// Cursors
		LeftEyeCursor = std::unique_ptr<Cursor>(new Cursor());
		RightEyeCursor = std::unique_ptr<Cursor>(new Cursor());
		ControllerCursor = std::unique_ptr<Cursor>(new Cursor());

		// ShaderID
		shaderID = LoadShaders("shader.vert", "shader.frag");
		skyboxShaderID = LoadShaders("skybox.vert", "skybox.frag");
		lineShaderID = LoadShaders("line.vert", "line.frag");

		// LEFT Texture Mapping

		// Frame buffer
		glGenFramebuffers(1, &lFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, lFBO);
		// Bind texture to render
		glGenTextures(1, &lrenderedTexture); 
		glBindTexture(GL_TEXTURE_2D, lrenderedTexture);
		// Give an empty image to OpenGL
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		// Poor filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Set renderedTexture as color attachment
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lrenderedTexture, 0);
		// Depth buffer
		glGenRenderbuffers(1, &lRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, lRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, // 1. fbo target: GL_FRAMEBUFFER
			GL_DEPTH_ATTACHMENT, // 2. attachment point
			GL_RENDERBUFFER, // 3. rbo target: GL_RENDERBUFFER
			lRBO); // 4. rbo ID

		// RIGHT Texture Mapping
		glGenFramebuffers(1, &rFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, rFBO);

		glGenTextures(1, &rrenderedTexture);
		glBindTexture(GL_TEXTURE_2D, rrenderedTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rrenderedTexture, 0);

		glGenRenderbuffers(1, &rRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, rRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rRBO); 

		// BOTTOM Texture Mapping
		glGenFramebuffers(1, &bFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);

		glGenTextures(1, &brenderedTexture);
		glBindTexture(GL_TEXTURE_2D, brenderedTexture);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brenderedTexture, 0);

		glGenRenderbuffers(1, &bRBO);
		glBindRenderbuffer(GL_RENDERBUFFER, bRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 2048, 2048);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bRBO); 

		// Cave
		cave = std::make_unique<Cave>();
		cave->toWorld = glm::rotate(glm::mat4(1.0f), -glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// Skybox
		// Skybox for right eye
		righteye_skybox = new Skybox("skybox_righteye");
		righteye_skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		// Skybox for left eye
		lefteye_skybox = new Skybox("skybox_lefteye");
		lefteye_skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));
		// Customized Skybox 
		self_skybox = std::make_unique<Skybox>("skybox_customized_1");
		self_skybox->toWorld = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f));	

		// Cube
		cube = std::make_unique<TexturedCube>("cube");
		cubeSize = 0.1f; //20 cm
		cubePos = glm::vec3(0.0f, 0.0f, -0.15f); // Origin Position
		cube->toWorld = glm::translate(glm::mat4(1.0f), cubePos) * glm::scale(glm::mat4(1.0f), glm::vec3(cubeSize));
		

		// Lines
		for (unsigned int i = 0; i < 7; i++) {
			LLines.push_back(new Line());
			RLines.push_back(new Line());
		}
	}

	

	void preRender(const glm::mat4 & projection, const glm::mat4 & modelview, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) {

		// Extra Credit
		if (buttonX == 1 && randNumGenerated == false) {
			randNum = rand() % 6;
			randNumGenerated = true;
			//std::cout << randNum << std::endl; // Testing
		}

		float nearPlane = 0.01f, farPlane = 1000.0f;


		// Render scene to texture LEFT
		glBindFramebuffer(GL_FRAMEBUFFER, lFBO);
		glViewport(0, 0, 2048, 2048);
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vec3 pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, 2.0f, 1.0f));
		vec3 pb = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));
		vec3 pc = glm::vec3(cave->toWorld * vec4(-2.0f, 2.0f, 2.0f, 1.0f));

		if (buttonX == 0 || curEyeIdx * 3 != randNum) {
			glUseProgram(skyboxShaderID);
			skybox->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			cube->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}
		
		
		// Update Lines
		if (curEyeIdx == 0) {

			LLines[0]->update(pc, eyePos, false);
			LLines[1]->update(pa, eyePos, false); 
			LeftEyeCursor->position = eyePos;
		}
		else {
			
			RLines[0]->update(pc, eyePos, true);
			RLines[1]->update(pa, eyePos, true);
			RightEyeCursor->position = eyePos;
		}

		// Render scene to texture RIGHT
		glBindFramebuffer(GL_FRAMEBUFFER, rFBO);
		glViewport(0, 0, 2048, 2048);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));
		pb = glm::vec3(cave->toWorld * vec4(2.0f, -2.0f, -2.0f, 1.0f));
		pc = glm::vec3(cave->toWorld * vec4(-2.0f, 2.0f, -2.0f, 1.0f));

		if (buttonX == 0 || curEyeIdx * 3 + 1 != randNum) {
			glUseProgram(skyboxShaderID);
			skybox->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			cube->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}

		
		// Update Lines
		if (curEyeIdx == 0) {

			LLines[2]->update(pc, eyePos, false);
			LLines[3]->update(pa, eyePos, false);
			LLines[4]->update(pb + (pc - pa), eyePos, false);
			LLines[5]->update(pb, eyePos, false);
		}
		else {


			RLines[2]->update(pc, eyePos, true);
			RLines[3]->update(pa, eyePos, true);
			RLines[4]->update(pb + (pc - pa), eyePos, true);
			RLines[5]->update(pb, eyePos, true);
		}

		// Render scene to texture BOTTOM
		glBindFramebuffer(GL_FRAMEBUFFER, bFBO);
		glViewport(0, 0, 2048, 2048);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pa = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, 2.0f, 1.0f));
		pb = glm::vec3(cave->toWorld * vec4(2.0f, -2.0f, 2.0f, 1.0f));
		pc = glm::vec3(cave->toWorld * vec4(-2.0f, -2.0f, -2.0f, 1.0f));

		if (buttonX == 0 || (curEyeIdx * 3 + 2) != randNum) {
			glUseProgram(skyboxShaderID);
			skybox->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
			cube->draw(skyboxShaderID, getProjection(eyePos, pa, pb, pc, nearPlane, farPlane), modelview);
		}
		

		// Update Line
		if (curEyeIdx == 0) {
			
			LLines[6]->update(pb, eyePos, false);
		}
		else {
			
			RLines[6]->update(pb, eyePos, true);
		}

		// Restore FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
		glViewport(vp.Pos.x, vp.Pos.y, vp.Size.w, vp.Size.h);
	}

	glm::mat4 getProjection(glm::vec3 eyePos, glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, float n, float f) {

		vec3 vr = glm::normalize(pb - pa);
		vec3 vu = glm::normalize(pc - pa);
		vec3 vn = glm::normalize(glm::cross(vr, vu));
		vec3 va = pa - eyePos;
		vec3 vb = pb - eyePos;
		vec3 vc = pc - eyePos;

		float d = -glm::dot(vn, va);
		float l = glm::dot(vr, va) * n / d;
		float r = glm::dot(vr, vb) * n / d;
		float b = glm::dot(vu, va) * n / d;
		float t = glm::dot(vu, vc) * n / d;

		glm::mat4 P = glm::frustum(l, r, b, t, n, f);

		glm::mat4 M = glm::mat4(vr.x, vr.y, vr.z, 0.0f,
								vu.x, vu.y, vu.z, 0.0f, 
								vn.x, vn.y, vn.z, 0.0f, 
								0.0f, 0.0f, 0.0f, 1.0f);

		glm::mat4 T = glm::translate(glm::vec3(-eyePos.x, -eyePos.y, -eyePos.z));

		return P * glm::transpose(M) * T;
	}

	void render(const mat4 & projection, const mat4 & modelview, const glm::vec3 & eyePos) {

		// Customized Skybox
		glUseProgram(skyboxShaderID);
		self_skybox->draw(skyboxShaderID, projection, modelview);
		// Cave
		glUseProgram(shaderID);
		cave->draw(shaderID, projection, modelview, lrenderedTexture, rrenderedTexture, brenderedTexture);
		
		// Render Lines
		if (buttonAPressed == true) {
			glUseProgram(lineShaderID);
		
			for (unsigned i = 0; i < 7; i++) {
				LLines[i]->draw(lineShaderID, projection, modelview);
				RLines[i]->draw(lineShaderID, projection, modelview);
			}

			// Cursor
			LeftEyeCursor->render(projection, modelview);
			RightEyeCursor->render(projection, modelview);
		}

		
	}

	void currentEye(int eyeIdx) {
		curEyeIdx = eyeIdx;
		if (eyeIdx == 0) {
			skybox = lefteye_skybox;
		}
		else {
			skybox = righteye_skybox;
		}
		
	}

};



class ExampleApp : public RiftApp {

	std::shared_ptr<Scene> scene;

public:
	ExampleApp() {}
	glm::mat4 lastHeadPose;
	
	// Cursor
	std::unique_ptr<Cursor> cursor;

	ovrInputState inputState;
	double displayMidpointSeconds;
	ovrTrackingState trackState;

	// Hand Pose and Position
	glm::mat4 prevRightHandPose;
	glm::mat4 RightHandPose;
	glm::vec3 RHPosition;
	glm::mat4 LeftHandPose;
	glm::vec3 LHPosition;

protected:

	void initGl() override {
		RiftApp::initGl();

		// Enable depth buffering
		glEnable(GL_DEPTH_TEST);
		// Related to shaders and z value comparisons for the depth buffer
		glDepthFunc(GL_LEQUAL);
		// Set polygon drawing mode to fill front and back of each polygon
		// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Disable backface culling to render both sides of polygons
		glDisable(GL_CULL_FACE);
		// Set clear color
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		ovr_RecenterTrackingOrigin(_session);

		// Scene
		scene = std::shared_ptr<Scene>(new Scene());

		// Cursor
		cursor = std::unique_ptr<Cursor>(new Cursor());
	}

	void shutdownGl() override {}

	void update() override {

		displayMidpointSeconds = ovr_GetPredictedDisplayTime(_session, frame);
		trackState = ovr_GetTrackingState(_session, displayMidpointSeconds, ovrTrue);
		ovrPosef RHPose = trackState.HandPoses[ovrHand_Right].ThePose; // Right Hand
		RightHandPose = ovr::toGlm(RHPose);
		RHPosition = glm::vec3(RHPose.Position.x, RHPose.Position.y, RHPose.Position.z);

		//std::cout << RHPosition.x << " " << RHPosition.y << " " << RHPosition.z << std::endl; // Testing
		cursor->position = RHPosition;

		if (OVR_SUCCESS(ovr_GetInputState(_session, ovrControllerType_Touch, &inputState))) {
			if (inputState.Buttons & ovrButton_A) {
				scene->buttonAPressed = true;
			}
			else if (scene->buttonAPressed) {
				scene->buttonAPressed = false;
			}

			// Freeze Mode
			if (inputState.Buttons & ovrButton_B) {
				scene->buttonBPressed = true;
			}
			else if (scene->buttonBPressed) {
				scene->buttonB = (scene->buttonB + 1) % 2; // Toggle between Freeze mode and Non-Freeze mode
				scene->buttonBPressed = false; 
			}

			// Disable one random projector
			if (inputState.Buttons & ovrButton_X) {
				scene->buttonXPressed = true;
			}
			else if (scene->buttonXPressed) {
				scene->buttonX = (scene->buttonX + 1) % 2;
				scene->buttonXPressed = false;
				scene->randNumGenerated = false;
			}
		
			// Ability to switch viewpoint to controller with the trigger
			if (inputState.HandTrigger[ovrHand_Right] > 0.5f) {
				scene->RHTriggerPressed = true;
			}
			else {
				scene->RHTriggerPressed = false;
			}


			// Cube Size
			if (inputState.Buttons & ovrButton_LThumb) {
				scene->cubeSize = 0.1f;
			}
			else {
				if (scene->cubeSize < 0.25f && inputState.Thumbstick[ovrHand_Left].x > 0.5f) {
					scene->cubeSize += 0.001f;
				}
				else if (scene->cubeSize > 0.005f && inputState.Thumbstick[ovrHand_Left].x < -0.5f) {
					scene->cubeSize -= 0.001f;
				}
			}
			// Cube Motion
			if (inputState.Buttons & ovrButton_RThumb) {
				scene->cubePos = glm::vec3(0.0f, 0.0f, -0.15f);
			}
			else {
				// x-axis
				if (scene->cubeSize < 0.25f && inputState.Thumbstick[ovrHand_Right].x > 0.5f) {
					scene->cubePos.x += 0.001f;
				}
				else if (scene->cubeSize > 0.005f && inputState.Thumbstick[ovrHand_Right].x < -0.5f) {
					scene->cubePos.x -= 0.001f;
				}
				// z-axis
				if (scene->cubeSize < 0.25f && inputState.Thumbstick[ovrHand_Right].y > 0.5f) {
					scene->cubePos.z += 0.001f;
				}
				else if (scene->cubeSize > 0.005f && inputState.Thumbstick[ovrHand_Right].y < -0.5f) {
					scene->cubePos.z -= 0.001f;
				}
			}			
		}

		// Update Cube
		scene->cube->toWorld = glm::translate(glm::mat4(1.0f), scene->cubePos) * glm::scale(glm::mat4(1.0f), glm::vec3(scene->cubeSize));

	}

	// Off-Screen Rendering
	void offscreenRender(const glm::mat4 & projection, const glm::mat4 & headPose, GLuint _fbo, const ovrRecti & vp, const glm::vec3 & eyePos) {

		// Head-in-Hand Mode
		if (scene->RHTriggerPressed) { // Switch to Right Hand Controller Position

			glm::mat4 no_rotation = glm::mat4(1.0f);

			if (FreezeMode() == 0) {
				no_rotation[3] = RightHandPose[3];
				prevRightHandPose = RightHandPose;
			}
			else {
				no_rotation[3] = prevRightHandPose[3]; // Freeze
			}

			/* Note that because we're rendering in stereo, you'll need to create two camera (=eye) positions at the controller, one offset to the left, 
			one to the right by half the average human eye distance, which is 65 millimeters. When switching the viewpoint, this should only affect the
			viewpoint used for rendering on the CAVE displays. */
			glm::vec3 adjustedEyePose = glm::vec3(no_rotation[3][0], no_rotation[3][1], no_rotation[3][2]);

			adjustedEyePose.x += getDefaultIOD(scene->curEyeIdx); // ADD Default IOD

			scene->preRender(projection, glm::inverse(no_rotation), _fbo, vp, adjustedEyePose);
		}
		else { // Switch Back to Head Position
			scene->preRender(projection, glm::inverse(headPose), _fbo, vp, eyePos);
		}
	}

	void renderScene(const glm::mat4 & projection, const glm::mat4 & headPose, const glm::vec3 & eyePos) override {

		//std::cerr << RHPosition.x << " " << RHPosition.y << " " << RHPosition.z << std::endl;

		// Render Scene
		scene->render(projection, glm::inverse(headPose), eyePos);
		// Update Cursor
		cursor->render(projection, glm::inverse(headPose));
	}

	void currentEye(ovrEyeType eye) {
		if (eye == ovrEye_Left) {
			scene->currentEye(0);
		}
		else {
			scene->currentEye(1);
		}
	}

	int FreezeMode() { return scene->buttonB; }


};

// Execute our example class
int main(int argc, char** argv)
{
	int result = -1;

	if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
	{
		FAIL("Failed to initialize the Oculus SDK");
	}
	result = ExampleApp().run();

	ovr_Shutdown();
	return result;
}