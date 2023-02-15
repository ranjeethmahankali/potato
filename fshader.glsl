#version 330 core

out vec4 FragColor;

in vec3 ColorOrTex;

uniform sampler2D pieces;

void main()
{
  if (ColorOrTex.z == -1.f) {
    // This means this triangle represents a piece on a square that is empty.
    FragColor = vec4(0., 0., 0., 0.);
  } else if (ColorOrTex.z == 2.f) {
    // This means this triangle represents a piece.
    FragColor = texture(pieces, ColorOrTex.xy);
  } else {
    // This means this triangle represents a square on the board.
    FragColor = vec4(ColorOrTex.rgb, 1.0);
  }
}
