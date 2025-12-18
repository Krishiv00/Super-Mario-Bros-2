uniform sampler2D texture;
uniform sampler2D pallete;

uniform float subPallete = 0.0;

void main() {
    // only store the red and alpha channel
    vec2 pixel = texture2D(texture, gl_TexCoord[0].xy).ra;

    if (pixel.y == 0) {
        discard;
    }

    gl_FragColor = texture2D(pallete, vec2(pixel.x, subPallete)) * gl_Color;
}