#include <array>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl3.h>

#include "shader.hpp"

using namespace glm;
using namespace std;

const auto size = 40;

void addBlock(vector<float>& vertices, vector<int>& elements, array<float, 2> grid[][size], int x, int y, int z) {
  const auto a = 0.5f;
  array<array<float, 2>, 4> xz {grid[x][z+1], grid[x+1][z+1], grid[x+1][z], grid[x][z]};
  array<float, 8> b {a + xz[0][0], a - xz[1][0], a - xz[2][0], a + xz[3][0], a - xz[0][1], a - xz[1][1], a + xz[2][1], a + xz[3][1]};
  // vertices
  vector<vec3> v; v.reserve(8);
  for (auto x = -a; x <= a; x += 1.0f)
    for (auto y = -a; y <= a; y += 1.0f)
      for (auto z = -a; z <= a; z += 1.0f)
        v.push_back(vec3(x, y, z));
  array<vec3, 8> vs {
    vec3(-b[0], +a, +b[4]),
    vec3(+b[1], +a, +b[5]),
    vec3(+b[1], -a, +b[5]),
    vec3(-b[0], -a, +b[4]),
    vec3(+b[2], +a, -b[6]),
    vec3(-b[3], +a, -b[7]),
    vec3(-b[3], -a, -b[7]),
    vec3(+b[2], -a, -b[6])
  };
  array<vec3, 24> cube {
    v[0], v[1], v[2], v[3], // -x
    v[0], v[2], v[4], v[6], // -z
    v[0], v[1], v[2], v[3], // -x
    v[4], v[5], v[6], v[7], // +x
    v[0], v[1], v[2], v[3], // -x
    v[4], v[5], v[6], v[7], // +x
    //v[4], v[5], v[6], v[7], // +x
    //v[0], v[1], v[4], v[5], // -y
    //v[2], v[3], v[6], v[7], // +y
    //v[0], v[2], v[4], v[6], // -z
    //v[1], v[3], v[5], v[7]  // +z
  };
  array<vec3, 24> cubes {
    v[0], v[1], v[2], v[3], // -x
    v[0], v[1], v[2], v[3], // -x
    v[0], v[1], v[2], v[3], // -x
    v[0], v[1], v[2], v[3], // -x
    v[0], v[1], v[2], v[3], // -x
    v[0], v[1], v[2], v[3], // -x
    //v[4], v[5], v[6], v[7], // +x
    //v[1], v[4], v[7], v[2], // +x
    //v[5], v[0], v[3], v[6], // -x
    //v[5], v[4], v[1], v[0], // +y
    //v[2], v[7], v[6], v[3]  // -y
  };
  for (int i = 0; i < 6; i++) {
    int s = vertices.size() / 3;
    elements.insert(elements.end(), {s, s+1, s+2, s, s+2, s+3});
    for (int j = 0; j < 4; j++) {
      auto p = cube[i*4+j];
      vertices.insert(vertices.end(), {p[0] + (float)x, p[1] + (float)y, p[2] + (float)z});
    }
  }
}

int error(const char* msg) {
  cout << msg << "\n";
  glfwTerminate();
  return -1;
}

float randf(float a=0.0f, float b=1.0f) {
  return (b - a) * ((float)rand() / RAND_MAX) + a;
}

GLuint genBuffer() {
  GLuint buf;
  glGenBuffers(1, &buf);
  return buf;
}

auto pos = vec3(0.0f, 0.0f, 5.0f),
  direction = vec3(0.0f, 0.0f, -1.0f),
  up = vec3(0.0f, 1.0f, 0.0f);
auto pitch = 0.0f, yaw = -90.0f;
auto width = 0.0f, height = 0.0f;
auto fov = 45.0f;

bool cursor;

void mouseCallback(GLFWwindow* win, double x, double y) {
  if (cursor)
    return;
  auto sensitivity = 0.05f;
	yaw += sensitivity * (x - width);
	pitch += sensitivity * (height - y);
  pitch = pitch < -89.0f ? -89.0f :
    pitch > 89.0f ? 89.0f :
    pitch;

  width = x;
  height = y;
  direction = normalize(vec3(
    cos(radians(yaw)) * cos(radians(pitch)),
    sin(radians(pitch)),
    sin(radians(yaw)) * cos(radians(pitch))
  ));
}

bool keys[1024];

void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    cursor = !cursor;
    glfwSetInputMode(win, GLFW_CURSOR, cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
  }
  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS) keys[key] = true;
    if (action == GLFW_RELEASE) keys[key] = false;
  }
  ImGui_ImplGlfwGL3_KeyCallback(win, key, scancode, action, mode);
}

void update(float deltatime) {
  auto speed = 5.0f * deltatime;
	if (keys[GLFW_KEY_W]) pos += direction * speed;
	if (keys[GLFW_KEY_A]) pos -= normalize(cross(direction, up)) * speed;
	if (keys[GLFW_KEY_S]) pos -= direction * speed;
	if (keys[GLFW_KEY_D]) pos += normalize(cross(direction, up)) * speed;
}

int main() {
  srand(time(0));

  if (!glfwInit())
    return error("Failed to initialize GLFW");

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	auto win = glfwCreateWindow(1024, 768, "dog", nullptr, nullptr);
	if (win == nullptr)
    return error("Failed to open GLFW window");

	glfwMakeContextCurrent(win);
  glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
    return error("Failed to initialize GLEW");

	ImGui_ImplGlfwGL3_Init(win, false);

  glfwSetCharCallback(win, ImGui_ImplGlfwGL3_CharCallback);
  glfwSetCursorPosCallback(win, mouseCallback);
  glfwSetKeyCallback(win, keyCallback);
  glfwSetMouseButtonCallback(win, ImGui_ImplGlfwGL3_MouseButtonCallback);
  glfwSetScrollCallback(win, ImGui_ImplGlfwGL3_ScrollCallback);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	GLuint vertexArray;
  glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	auto program = LoadShaders("vertex.shader", "fragment.shader");
	auto color = glGetUniformLocation(program, "color"),
    mvp = glGetUniformLocation(program, "mvp");

  vector<float> vertices;
  vector<int> elements;

  const auto max = 0.15f;
  array<float, 2> grid[size][size];
  for (int x = 0; x < size; x++)
    for (int z = 0; z < size; z++)
      grid[x][z] = {randf(-max, max), randf(-max, max)};

  addBlock(vertices, elements, grid, 0, 0, 0);

  auto vertexBuf = genBuffer();
 	glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
  auto elementBuf = genBuffer();
 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(int), &elements[0], GL_STATIC_DRAW);

  array<GLfloat, 4> rgba {randf(), randf(), randf(), 1.0f};

  auto lastTime = 0.0, lastFPS = glfwGetTime();
  auto fps = 0, nFrames = 0;
	while (!glfwWindowShouldClose(win)) {
    auto now = glfwGetTime();
    auto dt = now - lastTime;
    lastTime = now;
    if (now - lastFPS >= 1.0f) {
      fps = nFrames;
      lastFPS += 1.0f;
      nFrames = 0;
    } else nFrames++;

		glfwPollEvents();
    update(dt);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);

		glUniform4fv(color, 1, rgba.data());
    auto mat = perspective(fov, 4.0f / 3.0f, 0.01f, 1000.0f) *
      lookAt(pos, pos + direction, up) *
      mat4(1.0f);
		glUniformMatrix4fv(mvp, 1, false, &mat[0][0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuf);
    glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
		glDisableVertexAttribArray(0);

    ImGui_ImplGlfwGL3_NewFrame();
    ImGui::Begin("dog");
    ImGui::SliderFloat("r", &rgba[0], 0.0f, 1.0f);
    ImGui::SliderFloat("g", &rgba[1], 0.0f, 1.0f);
    ImGui::SliderFloat("b", &rgba[2], 0.0f, 1.0f);
    ImGui::SliderFloat("a", &rgba[3], 0.0f, 1.0f);
    ImGui::Text("%d fps", fps);
    if (ImGui::Button("quit"))
      glfwSetWindowShouldClose(win, true);
    ImGui::End();
    ImGui::Render();

		glfwSwapBuffers(win);
	}

	glDeleteBuffers(1, &vertexBuf);
	glDeleteBuffers(1, &elementBuf);
  glDeleteProgram(program);
	glDeleteVertexArrays(1, &vertexArray);
  ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
