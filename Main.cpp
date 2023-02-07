#include <Board.h>
#include <GLUtil.h>
#include <View.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace potato;
void glfw_error_cb(int error, const char* desc)
{
  gl::logger().error("GLFW Error {}: {}", error, desc);
}

int initGL(GLFWwindow*& window)
{
  glfwSetErrorCallback(glfw_error_cb);
  if (!glfwInit()) {
    gl::logger().error("Failed to initialize GLFW.");
    return 1;
  }
  gl::logger().info("Initialized GLFW.");
  // Window setup
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  std::string title = "Potato";
  window            = glfwCreateWindow(1024, 1024, title.c_str(), nullptr, nullptr);
  if (window == nullptr) {
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);
  // OpenGL bindings
  if (glewInit() != GLEW_OK) {
    gl::logger().error("Failed to initialize OpenGL bindings.");
    return 1;
  }
  gl::logger().info("OpenGL bindings are ready.");
  // TODO: Init shader.
  // TODO: Mouse support
  int W, H;
  GL_CALL(glfwGetFramebufferSize(window, &W, &H));
  GL_CALL(glViewport(0, 0, W, H));
  GL_CALL(glEnable(GL_DEPTH_TEST));
  GL_CALL(glEnable(GL_BLEND));
  GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_CALL(glEnable(GL_LINE_SMOOTH));
  GL_CALL(glEnable(GL_PROGRAM_POINT_SIZE));
  GL_CALL(glPointSize(3.0f));
  GL_CALL(glLineWidth(1.0f));
  GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
  return 0;
}

static std::vector<Board> makeBoards()
{
  std::vector<Board> boards(1, Board());
  Board().genMoves(boards, Piece::WHT);
  size_t end = boards.size();
  for (size_t i = 1; i < end; ++i) {
    boards[i].genMoves(boards, Piece::BLK);
  }
  return boards;
}

int main(int argc, char** argv)
{
  Board       b;
  int         err    = 0;
  GLFWwindow* window = nullptr;
  try {
    if ((err = initGL(window))) {
      gl::logger().error("Failed to initialize the viewer. Error code {}.", err);
      return err;
    }
    Shader::get().use();
    std::vector<Board> boards = makeBoards();
    BoardView          bview(boards[0]);
    size_t             bi = 0;
    // Render loop.
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      bview.draw();
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      bi = (bi + 1) % boards.size();
      bview.update(boards[bi]);
      glfwSwapBuffers(window);
      // TODO: Run queued state changes.
    }
  }
  catch (const std::exception& e) {
    gl::logger().critical("Fatal error: {}", e.what());
    err = -1;
  }
  return err;
}
