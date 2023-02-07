#pragma once

#include <Board.h>
#include <GLUtil.h>
#include <condition_variable>
#include <glm/glm.hpp>
#include <mutex>
#include <thread>

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

private:
  uint32_t mVAO = 0;
  uint32_t mVBO = 0;

  void free();
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
  static const Atlas& get();
  glm::vec4           textureCoords(uint8_t piece) const;

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
  static const Shader& get();
  void                 use() const;
  ~Shader();
  Shader(const Shader&) = delete;
  Shader(Shader&&)      = delete;

private:
  Shader();
  uint32_t mVertShaderId = 0;
  uint32_t mFragShaderId = 0;
  uint32_t mProgramId    = 0;
};

struct BoardView
{
  explicit BoardView(const Board& b);
  void update(const Board& board);
  void draw() const;

private:
  VertexBuffer mVBuf;
};

struct RenderLoop
{
  static RenderLoop& start();
  void               set(const Board& b);
  ~RenderLoop();

private:
  RenderLoop();
  RenderLoop(const RenderLoop&)                  = delete;
  RenderLoop(RenderLoop&&)                       = delete;
  const RenderLoop& operator=(const RenderLoop&) = delete;
  const RenderLoop& operator=(RenderLoop&&)      = delete;
  void              render();
  static void       wait();
  static void       loop();
  void              join();
  int               initGL();

private:
  static BoardView               sView;
  static GLFWwindow*             sWindow;
  static bool                    sPaused;
  static std::condition_variable sCV;
  static std::mutex              sMutex;
  std::thread                    mThread;
};

}  // namespace potato
