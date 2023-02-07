#version 330 core

out vec4 FragColor;

in vec3 ColorOrTex;

uniform sampler2D pieces;

void main()
{
  if (ColorOrTex.z == -1.f) {
    FragColor = vec4(0., 0., 0., 0.);
  } else if (ColorOrTex.z == 2.f) {
    FragColor = texture(pieces, ColorOrTex.xy);
  } else {
    FragColor = vec4(ColorOrTex.rgb, 1.0);
  }
}
