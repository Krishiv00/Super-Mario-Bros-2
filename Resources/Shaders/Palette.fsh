uniform sampler2D texture;
uniform sampler2D palette;

uniform float subPalette = 0.0;

void main() {
    vec2 pixel = texture2D(texture, gl_TexCoord[0].xy).ra;

    if (pixel.y == 0) {
        discard;
    }

    gl_FragColor = texture2D(palette, vec2(pixel.x, subPalette)) * gl_Color;
}