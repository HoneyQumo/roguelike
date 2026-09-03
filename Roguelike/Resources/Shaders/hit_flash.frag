// hit_flash.frag — белая вспышка попадания.
//
// Отдельными кадрами это делать нельзя: вспышка нужна на ЛЮБОМ кадре ЛЮБОЙ
// анимации, включая ходьбу и перезарядку. Кадров пришлось бы завести вдвое
// больше. Здесь один шейдер на весь проект.
//
//   sf::Shader hit; hit.loadFromFile("fx/hit_flash.frag", sf::Shader::Fragment);
//   hit.setUniform("texture", sf::Shader::CurrentTexture);
//   hit.setUniform("amount", flash);        // 1.0 в момент попадания
//   rt.draw(bodySprite, &hit);              // и тем же шейдером слой оружия
//
// flash гасить за 100-140 мс: flash = max(0.f, flash - dt / 0.12f);
uniform sampler2D texture;
uniform float amount;

void main()
{
    vec4 c = texture2D(texture, gl_TexCoord[0].xy);
    vec3 lit = mix(c.rgb, vec3(1.0), clamp(amount, 0.0, 1.0));
    gl_FragColor = vec4(lit, c.a) * gl_Color;
}
