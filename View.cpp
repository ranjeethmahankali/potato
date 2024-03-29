#include <Command.h>
#include <Position.h>
#include <Util.h>
#include <View.h>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>

namespace potato {
static constexpr uint32_t AtlasWidth  = 768;
static constexpr uint32_t AtlasHeight = 256;

using Texture = std::array<uint32_t, AtlasHeight * AtlasWidth>;

static Texture loadTexture()
{
  std::ifstream f(absPath("pieces.dat"), std::ios::binary);
  Texture       tx;
  f.read(reinterpret_cast<char*>(&tx), sizeof(tx));
  return tx;
}

static Texture& texture()
{
  static Texture sTexture = loadTexture();
  return sTexture;
}

static std::array<glm::vec4, NUniquePieces> generateTextureCoords()
{
  std::array<glm::vec4, NUniquePieces> coords;
  for (int i = 0; i < NUniquePieces; ++i) {
    Piece      pc     = Piece(i);
    glm::ivec2 pos    = {-1, -1};
    PieceType  ptype  = type(pc);
    Color      pcolor = color(pc);
    switch (ptype) {
    case PieceType::PWN:
      pos.x = 0;
      break;
    case PieceType::HRS:
      pos.x = 1;
      break;
    case PieceType::BSH:
      pos.x = 2;
      break;
    case PieceType::ROK:
      pos.x = 3;
      break;
    case PieceType::QEN:
      pos.x = 4;
      break;
    case PieceType::KNG:
      pos.x = 5;
      break;
    }
    switch (color(pc)) {
    case Color::WHT:
      pos.y = 0;
      break;
    case Color::BLK:
      pos.y = 1;
      break;
    }
    if (pos[0] == -1 || pos[1] == -1) {
      coords[i] = {-1.f, -1.f, -1.f, -1.f};
    }
    else {
      int x1    = pos.x * 128;
      int y1    = pos.y * 128;
      int x2    = x1 + 128;
      int y2    = y1 + 128;
      coords[i] = {
        float(x1) / 768.f, float(y1) / 256.f, float(x2) / 768.f, float(y2) / 256.f};
    }
  }
  return coords;
}

void Vertex::initAttributes()
{
  static constexpr size_t stride        = sizeof(Vertex);
  static const void*      posOffset     = (void*)(&(((Vertex*)nullptr)->mPosition));
  static const void*      txColorOffset = (void*)(&(((Vertex*)nullptr)->mTxOrColor));
  // Vertex position attribute.
  GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, posOffset));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, txColorOffset));
  GL_CALL(glEnableVertexAttribArray(1));
}

Atlas& Atlas::get()
{
  static Atlas sAtlas;
  return sAtlas;
}

glm::vec4 Atlas::textureCoords(uint8_t piece) const
{
  static const std::array<glm::vec4, NUniquePieces> sCoords = generateTextureCoords();
  return sCoords[piece];
}

Atlas::Atlas()
{
  initGLTexture();
  // Bind texture.
  GL_CALL(glBindTexture(GL_TEXTURE_2D, mGLTextureId));
}

Atlas::~Atlas()
{
  free();
}

void Atlas::free()
{
  if (mGLTextureId) {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    // Unbind texture.
    deleteGLTexture();
    mGLTextureId = 0;
  }
}

void Atlas::initGLTexture()
{
  GL_CALL(glGenTextures(1, &mGLTextureId));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, mGLTextureId));
  GL_CALL(glTexImage2D(GL_TEXTURE_2D,
                       0,
                       GL_RGBA,
                       GLsizei(Width),
                       GLsizei(Height),
                       0,
                       GL_RGBA,
                       GL_UNSIGNED_BYTE,
                       texture().data()));
  // Set texture options.
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Atlas::deleteGLTexture()
{
  if (mGLTextureId) {
    glDeleteTextures(1, &mGLTextureId);
    mGLTextureId = 0;
  }
}

static void checkShaderCompilation(uint32_t id, uint32_t type)
{
  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    std::string shaderType = "unknown";
    switch (type) {
    case GL_VERTEX_SHADER:
      shaderType = "vertex";
      break;
    case GL_FRAGMENT_SHADER:
      shaderType = "fragment";
      break;
    case GL_COMPUTE_SHADER:
      shaderType = "compute";
      break;
    default:
      break;
    }
    std::string message(length + 1, '\0');
    glGetShaderInfoLog(id, length, &length, message.data());
    gl::logger().error("Failed to compile {} shader:\n{}", shaderType, message);
    GL_CALL(glDeleteShader(id));
  }
};

static void checkShaderLinking(uint32_t progId)
{
  int success;
  glGetProgramiv(progId, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[1024];
    glGetProgramInfoLog(progId, 1024, NULL, infoLog);
    gl::logger().error("Error linking shader program:\n{}", infoLog);
  }
}

void Shader::init()
{
  uint32_t vsId = 0;
  uint32_t fsId = 0;
  // Compile vertex shader.
  {
    std::string vsrc = textFromFile(absPath("vshader.glsl"));
    vsId             = glCreateShader(GL_VERTEX_SHADER);
    const char* cstr = vsrc.c_str();
    GL_CALL(glShaderSource(vsId, 1, &cstr, nullptr));
    GL_CALL(glCompileShader(vsId));
    checkShaderCompilation(vsId, GL_VERTEX_SHADER);
  }
  // Compile fragment shader.
  {
    std::string fsrc = textFromFile(absPath("fshader.glsl"));
    fsId             = glCreateShader(GL_FRAGMENT_SHADER);
    const char* cstr = fsrc.c_str();
    GL_CALL(glShaderSource(fsId, 1, &cstr, nullptr));
    GL_CALL(glCompileShader(fsId));
    checkShaderCompilation(fsId, GL_FRAGMENT_SHADER);
  }
  // Link
  mProgramId = glCreateProgram();
  GL_CALL(glAttachShader(mProgramId, vsId));
  GL_CALL(glAttachShader(mProgramId, fsId));
  GL_CALL(glLinkProgram(mProgramId));
  checkShaderLinking(mProgramId);
  GL_CALL(glDeleteShader(vsId));
  GL_CALL(glDeleteShader(fsId));
}

void Shader::use() const
{
  GL_CALL(glUseProgram(mProgramId));
}

void Shader::free()
{
  if (mProgramId) {
    GL_CALL(glDeleteProgram(mProgramId));
    mProgramId = 0;
  }
}

Shader::~Shader()
{
  free();
}

static bool sFlipBoard = false;

static glm::vec2 quadCenter(glm::ivec2 qpos)
{
  qpos.y = 7 - qpos.y;
  if (sFlipBoard) {
    qpos = glm::ivec2(7, 7) - qpos;
  }
  return (glm::vec2(0.125f, 0.125f) + glm::vec2(qpos) / 4.f) - glm::vec2 {1.f, 1.f};
}

static glm::vec2 quadVertex(glm::ivec2 qpos, int vertexIdx)
{
  static constexpr std::array<glm::vec2, 4> sCorners = {
    {{-0.125f, -0.125f}, {0.125f, -0.125f}, {0.125f, 0.125f}, {-0.125f, 0.125f}}};
  // Flip along the y axis.
  return quadCenter(qpos) + sCorners[vertexIdx];
}

BoardView::BoardView(const Position& b)
{
  static constexpr glm::vec3 Black      = glm::vec3(0.15f, 0.35f, 0.15f);
  static constexpr glm::vec3 White      = glm::vec3(0.75f);
  static constexpr float     BoardDepth = 0.9f;
  auto                       dst        = mVBuf.data();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      glm::vec3             color = ((x + y) % 2) ? Black : White;
      std::array<Vertex, 4> quad;
      for (int vi = 0; vi < 4; ++vi) {
        glm::vec3 pos(quadVertex({x, y}, vi), BoardDepth);
        quad[vi] = Vertex {pos, color};
      }
      *(dst++) = quad[0];
      *(dst++) = quad[1];
      *(dst++) = quad[2];
      *(dst++) = quad[0];
      *(dst++) = quad[2];
      *(dst++) = quad[3];
    }
  }
  update(b);
}

BoardView::~BoardView()
{
  free();
}

void BoardView::update(const Position& b)
{
  static constexpr size_t PieceOffset = 64 * 6;
  auto                    dst         = mVBuf.data() + PieceOffset;
  static constexpr float  PieceDepth  = 0.f;
  glm::ivec2              pos;
  for (pos.y = 0; pos.y < 8; ++pos.y) {
    for (pos.x = 0; pos.x < 8; ++pos.x) {
      Piece                 pc        = b.piece(pos);
      glm::vec4             tc        = Atlas::get().textureCoords(pc);
      float                 colorFlag = pc ? 2.f : -1.f;
      std::array<Vertex, 4> quad      = {{
             {glm::vec3(quadVertex(pos, 0), PieceDepth), {tc[0], tc[1], colorFlag}},
             {glm::vec3(quadVertex(pos, 1), PieceDepth), {tc[2], tc[1], colorFlag}},
             {glm::vec3(quadVertex(pos, 2), PieceDepth), {tc[2], tc[3], colorFlag}},
             {glm::vec3(quadVertex(pos, 3), PieceDepth), {tc[0], tc[3], colorFlag}},
      }};
      *(dst++)                        = quad[0];
      *(dst++)                        = quad[1];
      *(dst++)                        = quad[2];
      *(dst++)                        = quad[0];
      *(dst++)                        = quad[2];
      *(dst++)                        = quad[3];
    }
  }
  mVBuf.alloc();
}

void BoardView::draw() const
{
  mVBuf.bindVao();
  mVBuf.bindVbo();
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
}

void BoardView::free()
{
  mVBuf.free();
}

MoveView::MoveView()
{
  std::fill(mVBuf.begin(), mVBuf.end(), Vertex {glm::vec3(0.), glm::vec3(0.)});
}

MoveView::~MoveView()
{
  free();
}

MoveView::MoveView(Move m)
{
  update(m);
}

void MoveView::update(Move m)
{
  static constexpr glm::vec3 sMoveColor = {2.f, 0.f, 1.f};
  static constexpr float     MoveDepth  = -0.2f;
  static constexpr float     sTheta     = float(M_PI) / float(CircleSubDiv);
  static constexpr float     sThickness = 0.01f;
  int                        from       = m.from();
  int                        to         = m.to();
  auto                       c1 = glm::vec3(quadCenter({from % 8, from / 8}), MoveDepth);
  auto                       c2 = glm::vec3(quadCenter({to % 8, to / 8}), MoveDepth);
  auto                       x  = glm::normalize(c2 - c1);
  auto                       y  = glm::cross(glm::vec3(0.f, 0.f, 1.f), x);
  x *= sThickness;
  y *= sThickness;
  float ang = float(1.5 * M_PI);
  auto  dst = mVBuf.begin();
  for (size_t i = 0; i < CircleSubDiv; ++i) {
    *(dst++) = Vertex {c1 + std::sin(ang) * y + std::cos(ang) * x, sMoveColor};
    ang -= sTheta;
    *(dst++) = Vertex {c1 + std::sin(ang) * y + std::cos(ang) * x, sMoveColor};
    *(dst++) = Vertex {c1, sMoveColor};
  }
  ang = float(0.5 * M_PI);
  for (size_t i = 0; i < CircleSubDiv; ++i) {
    *(dst++) = Vertex {c2 + std::sin(ang) * y + std::cos(ang) * x, sMoveColor};
    ang -= sTheta;
    *(dst++) = Vertex {c2 + std::sin(ang) * y + std::cos(ang) * x, sMoveColor};
    *(dst++) = Vertex {c2, sMoveColor};
  }
  *(dst++) = Vertex {c1 + y, sMoveColor};
  *(dst++) = Vertex {c2 + y, sMoveColor};
  *(dst++) = Vertex {c1 - y, sMoveColor};
  *(dst++) = Vertex {c2 + y, sMoveColor};
  *(dst++) = Vertex {c2 - y, sMoveColor};
  *(dst++) = Vertex {c1 - y, sMoveColor};

  mVBuf.alloc();
}

void MoveView::draw() const
{
  mVBuf.bindVao();
  mVBuf.bindVbo();
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
}

void MoveView::free()
{
  mVBuf.free();
}

SuggestionView::SuggestionView()
{
  clear();
}

SuggestionView::~SuggestionView()
{
  free();
}

SuggestionView::SuggestionView(int from)
{
  update(from);
}

void SuggestionView::update(int from)
{
  static constexpr std::array<glm::vec2, 4> sCorners = {
    {{-0.025f, -0.025f}, {0.025f, -0.025f}, {0.025f, 0.025f}, {-0.025f, 0.025f}}};
  static constexpr glm::vec3 SuggestionColor = {1.f, 0.05, -2.f};
  static constexpr float     SuggestionDepth = -0.1f;
  clear();
  MoveList legal;
  generateMoves(currentPosition(), legal);
  auto end = std::remove_if(
    legal.begin(), legal.end(), [from](Move m) { return m.from() != from; });
  auto begin = legal.begin();
  auto dst   = mVBuf.begin();
  while (begin != end) {
    std::array<Vertex, 4> quad;
    int                   to  = (begin++)->to();
    glm::ivec2            pos = {to % 8, to / 8};
    for (int vi = 0; vi < 4; ++vi) {
      quad[vi] = Vertex {glm::vec3(quadCenter(pos) + sCorners[vi], SuggestionDepth),
                         SuggestionColor};
    }
    *(dst++) = quad[0];
    *(dst++) = quad[1];
    *(dst++) = quad[2];
    *(dst++) = quad[0];
    *(dst++) = quad[2];
    *(dst++) = quad[3];
  }
  mVBuf.alloc();
}

void SuggestionView::draw() const
{
  mVBuf.bindVao();
  mVBuf.bindVbo();
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
}

void SuggestionView::free()
{
  mVBuf.free();
}

void SuggestionView::clear(bool realloc)
{
  mVBuf.fill(Vertex {glm::vec3 {0., 0., 0.}, glm::vec3 {0., 0., 0.}});
  if (realloc) {
    mVBuf.alloc();
  }
}

namespace view {

static std::unique_ptr<BoardView>      sView;
static std::unique_ptr<MoveView>       sMoveView;
static std::unique_ptr<SuggestionView> sSuggestionView;

static void glfw_error_cb(int error, const char* desc)
{
  gl::logger().error("GLFW Error {}: {}", error, desc);
}

static bool sMyTurn = false;

static void onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
  static std::array<int, 2> sMove = {{-1, -1}};
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    int& target = sMove[0] == -1 ? sMove[0] : sMove[1];
    if (action == GLFW_PRESS) {
      glm::dvec2 pos;
      GL_CALL(glfwGetCursorPos(window, &pos.x, &pos.y));
      glm::ivec2 ipos = glm::ivec2(glm::floor(pos / 128.0));
      if (sFlipBoard) {
        ipos = glm::ivec2(7, 7) - ipos;
      }
      target = ipos.y * 8 + ipos.x;
    }
    else if (action == GLFW_RELEASE) {
      glm::dvec2 pos;
      GL_CALL(glfwGetCursorPos(window, &pos.x, &pos.y));
      glm::ivec2 ipos = glm::ivec2(glm::floor(pos / 128.0));
      if (sFlipBoard) {
        ipos = glm::ivec2(7, 7) - ipos;
      }
      int t2 = ipos.y * 8 + ipos.x;
      if (t2 != target) {
        target = -1;
      }
      if (sMove[0] != -1 && sMove[1] != -1) {
        std::string mv = std::string(SquareCoord[sMove[0]]);
        mv += SquareCoord[sMove[1]];
        sSuggestionView->clear(true);
        bool success = doMove(mv);
        if (success) {
          std::cout << "You: " << mv << std::endl;
          sMove   = {{-1, -1}};
          sMyTurn = true;
        }
        else {
          // Not a legal move.
          // Likely the user just wants to change their piece selection.
          sMove[0] = std::exchange(sMove[1], -1);
          sSuggestionView->update(sMove[0]);
        }
      }
      else if (sMove[0] != -1) {
        // Selected a piece, show possible moves.
        sSuggestionView->update(sMove[0]);
      }
    }
  }
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
  // OpenGL bindings
  int err = GLEW_OK;
  if ((err = glewInit()) != GLEW_OK) {
    gl::logger().error("Failed to initialize OpenGL bindings: {}", err);
    return 1;
  }
  gl::logger().info("OpenGL bindings are ready.");
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
  glfwSetMouseButtonCallback(window, &onMouseButton);
  return 0;
}

void game(bool asBlack, bool flipBoard)
{
  if (asBlack) {
    sMyTurn = true;
  }
  sFlipBoard = asBlack != flipBoard;
  using namespace std::chrono_literals;
  GLFWwindow* window = nullptr;
  try {
    int err = 0;
    if ((err = initGL(window))) {
      gl::logger().error("Failed to initialize the viewer. Error code {}.", err);
      return;
    }
    sView           = std::make_unique<BoardView>(currentPosition());
    sMoveView       = std::make_unique<MoveView>();
    sSuggestionView = std::make_unique<SuggestionView>();
    Shader shader;
    shader.init();
    shader.use();
    // Render loop.
    while (!glfwWindowShouldClose(window)) {
      glfwPollEvents();
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      sView->draw();
      sMoveView->draw();
      sSuggestionView->draw();
      glfwSwapBuffers(window);
      if (sMyTurn) {
        Response response;
        {
          Timer timer("Thinking time: ");
          response = bestMove(currentPosition());
        }
        if (response.mMove) {  // Pototo's move responding to the user's move.
          std::this_thread::sleep_for(300ms);
          std::cout << " Me: " << *(response.mMove) << std::endl;
          response.mMove->commit(currentPosition());
          update(*(response.mMove));
          response = Response::none();
          currentPosition().freezeState();
        }
        else if (response.mConclusion == Conclusion::CHECKMATE) {
          std::cout << "It's a checkmate!\n";
          response = Response::none();
          glfwSetMouseButtonCallback(window, nullptr);
        }
        else if (response.mConclusion == Conclusion::STALEMATE) {
          std::cout << "It's a stalemate!\n";
          response = Response::none();
          glfwSetMouseButtonCallback(window, nullptr);
        }
        sMyTurn = false;
      }
    }
    gl::logger().info("Closing window...\n");
    glfwDestroyWindow(window);
    shader.free();
    Atlas::get().free();
    sView->free();
    glfwTerminate();
  }
  catch (const std::exception& e) {
    gl::logger().critical("Fatal error: {}", e.what());
    return;
  }
}

void update()
{
  sView->update(currentPosition());
}

void update(Move m)
{
  sView->update(currentPosition());
  sMoveView->update(m);
}

}  // namespace view
}  // namespace potato
