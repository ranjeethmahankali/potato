#pragma once

#include <Board.h>
#include <GLUtil.h>
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
static constexpr size_t BufferSize = (64 + 64) * 6;

class VertexBuffer : public std::array<Vertex, BufferSize>
{
public:
  VertexBuffer() = default;
  ~VertexBuffer();
  using std::array<Vertex, BufferSize>::size;
  using std::array<Vertex, BufferSize>::data;
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
  void bindVao() const;
  /**
   * @brief Binds the vertex buffer.
   */
  void bindVbo() const;
  /**
   * @brief Allocates the vertex array and vertex buffer on the GPU and copies the data.
   */
  void alloc();

  void free();

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
  explicit BoardView(const Board& b);
  void update(const Board& board);
  void draw() const;
  void free();

private:
  VertexBuffer mVBuf;
};

namespace view {

void start();
void update();
void stop();
void join();
bool closed();

}  // namespace view

}  // namespace potato
