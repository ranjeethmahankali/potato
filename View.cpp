#include <Util.h>
#include <View.h>
#include <fstream>

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

static std::array<glm::vec4, 256> generateTextureCoords()
{
  std::array<glm::vec4, 256> coords;
  for (int i = 0; i < 256; ++i) {
    uint8_t pc = uint8_t(i);
    pc &= ~(Piece::ENPASSANT | Piece::CASTLE);
    uint8_t    type  = Piece::type(pc);
    uint8_t    color = Piece::color(pc);
    glm::ivec2 pos   = {-1, -1};
    switch (type) {
    case Piece::PWN:
      pos.x = 0;
      break;
    case Piece::HRS:
      pos.x = 1;
      break;
    case Piece::BSH:
      pos.x = 2;
      break;
    case Piece::ROK:
      pos.x = 3;
      break;
    case Piece::QEN:
      pos.x = 4;
      break;
    case Piece::KNG:
      pos.x = 5;
      break;
    }

    switch (color) {
    case Piece::WHT:
      pos.y = 0;
      break;
    case Piece::BLK:
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

void VertexBuffer::free()
{
  if (mVAO) {
    GL_CALL(glDeleteVertexArrays(1, &mVAO));
    mVAO = 0;
  }
  if (mVBO) {
    GL_CALL(glDeleteBuffers(1, &mVBO));
    mVBO = 0;
  }
}

VertexBuffer::~VertexBuffer()
{
  free();
}

void VertexBuffer::bindVao() const
{
  GL_CALL(glBindVertexArray(mVAO));
}

void VertexBuffer::bindVbo() const
{
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
}

void VertexBuffer::alloc()
{
  free();  // Free if already bound.
  GL_CALL(glGenVertexArrays(1, &mVAO));
  GL_CALL(glGenBuffers(1, &mVBO));

  bindVao();
  bindVbo();
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, numbytes(), data(), GL_STATIC_DRAW));
  Vertex::initAttributes();
  // Unbind stuff.
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  GL_CALL(glBindVertexArray(0));
}

const Atlas& Atlas::get()
{
  static Atlas sAtlas;
  return sAtlas;
}

glm::vec4 Atlas::textureCoords(uint8_t piece) const
{
  static const std::array<glm::vec4, 256> sCoords = generateTextureCoords();
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
  GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
  // Unbind texture.
  deleteGLTexture();
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
};

Shader::Shader()
{
  // Compile vertex shader.
  {
    std::string vsrc = textFromFile(absPath("vshader.glsl"));
    mVertShaderId    = glCreateShader(GL_VERTEX_SHADER);
    const char* cstr = vsrc.c_str();
    GL_CALL(glShaderSource(mVertShaderId, 1, &cstr, nullptr));
    GL_CALL(glCompileShader(mVertShaderId));
    checkShaderCompilation(mVertShaderId, GL_VERTEX_SHADER);
  }
  // Compile fragment shader.
  {
    std::string fsrc = textFromFile(absPath("fshader.glsl"));
    mFragShaderId    = glCreateShader(GL_FRAGMENT_SHADER);
    const char* cstr = fsrc.c_str();
    GL_CALL(glShaderSource(mFragShaderId, 1, &cstr, nullptr));
    GL_CALL(glCompileShader(mFragShaderId));
    checkShaderCompilation(mFragShaderId, GL_FRAGMENT_SHADER);
  }
  // Link
  mProgramId = glCreateProgram();
  GL_CALL(glAttachShader(mProgramId, mVertShaderId));
  GL_CALL(glAttachShader(mProgramId, mFragShaderId));
  GL_CALL(glLinkProgram(mProgramId));
  checkShaderLinking(mProgramId);
  GL_CALL(glDeleteShader(mVertShaderId));
  GL_CALL(glDeleteShader(mFragShaderId));
}

const Shader& Shader::get()
{
  static Shader sShader = Shader();
  return sShader;
}

void Shader::use() const
{
  GL_CALL(glUseProgram(mProgramId));
}

Shader::~Shader()
{
  GL_CALL(glDeleteProgram(mProgramId));
}

static glm::vec2 quadVertex(glm::ivec2 qpos, int vertexIdx)
{
  static constexpr std::array<glm::vec2, 4> sCorners = {
    {{-0.0625f, -0.0625f}, {0.0625f, -0.0625f}, {0.0625f, 0.0625f}, {-0.0625f, 0.0625f}}};
  // Flip along the y axis.
  qpos.y = 7 - qpos.y;
  return 2.f * ((glm::vec2(0.0625, 0.0625f) + glm::vec2(qpos) / 8.f)  // center
                + sCorners[vertexIdx]) -
         glm::vec2 {1.f, 1.f};
}

BoardView::BoardView(const Board& b)
{
  static constexpr glm::vec3 Black      = glm::vec3(0.15f, 0.35f, 0.15f);
  static constexpr glm::vec3 White      = glm::vec3(0.75f);
  static constexpr float     BoardDepth = 0.9f;
  auto                       dst        = mVBuf.data();
  for (int y = 0; y < 8; ++y) {
    for (int x = 0; x < 8; ++x) {
      glm::vec3             color = ((x + y) % 2) ? White : Black;
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

void BoardView::update(const Board& b)
{
  static constexpr size_t PieceOffset = 64 * 6;
  auto                    dst         = mVBuf.data() + PieceOffset;
  static constexpr float  PieceDepth  = 0.f;
  glm::ivec2              pos;
  for (pos.y = 0; pos.y < 8; ++pos.y) {
    for (pos.x = 0; pos.x < 8; ++pos.x) {
      uint8_t               pc        = b.piece(pos);
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
  mVBuf.bindVao();
  mVBuf.bindVbo();
}

void BoardView::draw() const
{
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, mVBuf.size()));
}

}  // namespace potato
