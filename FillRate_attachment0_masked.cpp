#include <iostream>
#include <cassert>
#include <string>
#include <sstream>
#include <chrono>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <vector>

static std::string getErrorDescr(const GLenum errCode)
{
	// English descriptions are from
	// https://www.opengl.org/sdk/docs/man/docbook4/xhtml/glGetError.xml
	switch (errCode) {
		case GL_NO_ERROR: return "No error has been recorded. THIS message is the error itself.";
		case GL_INVALID_ENUM: return "An unacceptable value is specified for an enumerated argument.";
		case GL_INVALID_VALUE: return "A numeric argument is out of range.";
		case GL_INVALID_OPERATION: return "The specified operation is not allowed in the current state.";
		case GL_INVALID_FRAMEBUFFER_OPERATION: return "The framebuffer object is not complete.";
		case GL_OUT_OF_MEMORY: return "There is not enough memory left to execute the command.";
		case GL_STACK_UNDERFLOW: return "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
		case GL_STACK_OVERFLOW: return "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
		default:;
	}
	return "No description available.";
}

static std::string getErrorMessage()
{
	const GLenum error = glGetError();
	if (GL_NO_ERROR == error) return "";

	std::stringstream ss;
	ss << "OpenGL error: " << static_cast<int>(error) << std::endl;
	ss << "Error string: ";
	ss << getErrorDescr(error);
	ss << std::endl;
	return ss.str();
}

[[maybe_unused]] static bool error()
{
	const auto message = getErrorMessage();
	if (message.length() == 0) return false;
	std::cerr << message;
	return true;
}

static bool compileShader(const GLuint shader, const std::string& source)
{
	unsigned int linesCount = 0;
	for (const auto c: source) linesCount += static_cast<unsigned int>(c == '\n');
	const char** sourceLines = new const char*[linesCount];
	int* lengths = new int[linesCount];

	int idx = 0;
	const char* lineStart = source.data();
	int lineLength = 1;
	const auto len = source.length();
	for (unsigned int i = 0; i < len; ++i) {
		if (source[i] == '\n') {
			sourceLines[idx] = lineStart;
			lengths[idx] = lineLength;
			lineLength = 1;
			lineStart = source.data() + i + 1;
			++idx;
		}
		else ++lineLength;
	}

	glShaderSource(shader, linesCount, sourceLines, lengths);
	glCompileShader(shader);
	GLint logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		auto* const log = new GLchar[logLength + 1];
		glGetShaderInfoLog(shader, logLength, nullptr, log);
		std::cout << "Log: " << std::endl;
		std::cout << log;
		delete[] log;
	}

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	delete[] sourceLines;
	delete[] lengths;
	return bool(compileStatus);
}

static GLuint createProgram(const std::string& vertSource, const std::string& fragSource)
{
	const auto vs = glCreateShader(GL_VERTEX_SHADER);
	if (vs == 0) {
		std::cerr << "Error: vertex shader is 0." << std::endl;
		return 2;
	}
	const auto fs = glCreateShader(GL_FRAGMENT_SHADER);
	if (fs == 0) {
		std::cerr << "Error: fragment shader is 0." << std::endl;
		return 2;
	}

	// Compile shaders
	if (!compileShader(vs, vertSource)) {
		std::cerr << "Error: could not compile vertex shader." << std::endl;
		return 5;
	}
	if (!compileShader(fs, fragSource)) {
		std::cerr << "Error: could not compile fragment shader." << std::endl;
		return 5;
	}

	// Link program
	const auto program = glCreateProgram();
	if (program == 0) {
		std::cerr << "Error: program is 0." << std::endl;
		return 2;
	}
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	// Get log
	GLint logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 0) {
		auto* const log = new GLchar[logLength + 1];
		glGetProgramInfoLog(program, logLength, nullptr, log);
		std::cout << "Log: " << std::endl;
		std::cout << log;
		delete[] log;
	}
	GLint linkStatus = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (!linkStatus) {
		std::cerr << "Error: could not link." << std::endl;
		return 2;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
	return program;
}

static const std::string vertSource = R"(
#version 330

layout(location = 0) in vec2 v;

void main()
{
	gl_Position = vec4(v, 0.0, 1.0);
}
)";

static const std::string fragSource = R"(
#version 330

layout(location = 0) out vec4 outColor0;

void main()
{
	outColor0 = vec4(0.5, 0.5, 0.5, 1.0);
}
)";

int main()
{
	// Init
	if (!glfwInit()) {
		std::cerr << "Error: glfw init failed." << std::endl;
		return 3;
	}

	static const int width = 800;
	static const int height= 600;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = nullptr;
	window = glfwCreateWindow(width, height, "Shader test", nullptr, nullptr);
	if (window == nullptr) {
		std::cerr << "Error: window is null." << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cerr << "Error: glew not OK." << std::endl;
		glfwTerminate();
		return 2;
	}

	// Shader program
	const auto shaderProgram = createProgram(vertSource, fragSource);
	glUseProgram(shaderProgram);

	// Vertex buffer
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	float bufferData[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f
	};
	glBufferData(GL_ARRAY_BUFFER, std::size(bufferData) * sizeof(float), bufferData, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// Framebuffer
	GLuint fb, att[6];
	glGenTextures(6, att);
	glGenFramebuffers(1, &fb);

	glBindTexture(GL_TEXTURE_2D, att[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, att[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, att[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, att[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, att[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, att[5]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, att[0], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, att[1], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, att[2], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, att[3], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, att[4], 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, att[5], 0);

	GLuint dbs[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5};
	glDrawBuffers(6, dbs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
		std::cerr << "Error: framebuffer is incomplete." << std::endl;
		return 1;
	}
	if (error()) {
		std::cerr << "OpenGL error occured." << std::endl;
		return 2;
	}

	glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glColorMaski(1, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glColorMaski(2, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glColorMaski(3, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glColorMaski(4, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glColorMaski(5, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	// Fpsmeter
	static const uint32_t framesMax = 50;
	uint32_t framesCount = 0;
	auto start = std::chrono::steady_clock::now();

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClear(GL_COLOR_BUFFER_BIT);
		for (int i = 0; i < 100; ++i) glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (++framesCount == framesMax) {
			framesCount = 0;
			const auto now = std::chrono::steady_clock::now();
			const auto duration = now - start;
			start = now;
			const float secsPerFrame = (std::chrono::duration_cast<std::chrono::microseconds>(duration).count() / 1000000.0f) / framesMax;
			std::cout << "FPS: " << 1.0f / secsPerFrame << std::endl;
		}
	}

	// Shutdown
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(vao);
	glUseProgram(0);
	glDeleteProgram(shaderProgram);
	glDeleteBuffers(1, &buffer);
	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fb);
	glDeleteTextures(6, att);
	glfwMakeContextCurrent(nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
