#pragma once

#include <GLUtil.h>
#include <Move.h>
#include <Position.h>
#include <glm/glm.hpp>

namespace potato {

struct Vertex
{
  glm::vec3 mPosition;
  glm::vec3 mTxOrColor;

  static void initAttributes();
};

// 64 squares for the board, 64 squares for the pieces and two triangles (6 vertices per
// square).
static constexpr size_t BoardBufferSize = (64 + 64) * 6;

template<size_t Size>
class VertexBuffer : public std::array<Vertex, Size>
{
public:
  VertexBuffer() = default;
  ~VertexBuffer() { free(); }
  using std::array<Vertex, Size>::size;
  using std::array<Vertex, Size>::data;
  // Disallow copying and moving.
  VertexBuffer(const VertexBuffer&)                   = delete;
  const VertexBuffer& operator=(const VertexBuffer&)  = delete;
  const VertexBuffer& operator=(VertexBuffer&& other) = delete;
  VertexBuffer(VertexBuffer&& other)                  = delete;
  /**
   * @brief Size of the buffer in bytes.
   *
   * @return size_t
   */
  constexpr size_t numbytes() const { return sizeof(Vertex) * size(); }
  /**
   * @brief Bind the vertex array.
   */
  void bindVao() const { GL_CALL(glBindVertexArray(mVAO)); }
  /**
   * @brief Binds the vertex buffer.
   */
  void bindVbo() const { GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVBO)); }
  /**
   * @brief Allocates the vertex array and vertex buffer on the GPU and copies the data.
   */
  void alloc()
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

  void free()
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

private:
  uint32_t mVAO = 0;
  uint32_t mVBO = 0;
};

void unbindTexture();

class Atlas
{
public:
  static constexpr uint32_t Width    = 768;
  static constexpr uint32_t Height   = 256;
  static constexpr uint32_t TileSize = 128;

  ~Atlas();
  // Forbid copy and move.
  Atlas(const Atlas&)                  = delete;
  const Atlas& operator=(const Atlas&) = delete;
  Atlas(Atlas&&)                       = delete;
  const Atlas& operator=(Atlas&&)      = delete;

  /**
   * @brief Get the singleton instance of the atlas.
   */
  static Atlas& get();
  glm::vec4     textureCoords(uint8_t piece) const;
  void          free();

private:
  Atlas();

  void initGLTexture();
  void deleteGLTexture();

private:
  uint32_t mGLTextureId = 0;
  // Atlas texture containing all pieces.
  std::array<uint32_t, Width * Height> mTexture;
};

class Shader
{
public:
  Shader() = default;
  ~Shader();
  void init();
  void use() const;
  void free();
  Shader(const Shader&) = delete;
  Shader(Shader&&)      = delete;

private:
  uint32_t mProgramId = 0;
};

struct BoardView
{
  explicit BoardView(const Position& b);
  ~BoardView();
  void update(const Position& board);
  void draw() const;
  void free();

private:
  VertexBuffer<BoardBufferSize> mVBuf;
};

struct MoveView
{
  MoveView();
  ~MoveView();
  explicit MoveView(Move m);
  void update(Move m);
  void draw() const;
  void free();

private:
  static constexpr size_t        CircleSubDiv = 60;
  VertexBuffer<(2 * 60 + 2) * 3> mVBuf;
};

struct SuggestionView
{
  SuggestionView();
  ~SuggestionView();
  explicit SuggestionView(int from);
  void update(int from);
  void draw() const;
  void free();
  void clear(bool realloc = false);

private:
  VertexBuffer<32 * 6> mVBuf;
};

namespace view {

void game();
void update();
void update(Move m);

}  // namespace view

}  // namespace potato
