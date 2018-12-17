#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;

// Fractal ramp
uniform sampler2D fractal;

uniform int texID;
uniform int shaderMode;

const float ambient = .25;

// Fractal parameters
const int iter = 150;
const float scale = 3;


bool shouldColorCircle(float circleSize, float maxRadius) {
  return pow(mod(gl_FragCoord.x, maxRadius * 2) - maxRadius, 2) + pow(mod(gl_FragCoord.y, maxRadius * 2) - maxRadius, 2) < circleSize;
}

float aastep(float threshold, float value) {
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));
  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main() {
  if (shaderMode == 4) {
    vec3 normal = normalize(vertNormal);
    float depth = (gl_FragCoord.z / gl_FragCoord.w) * 2;
    if (depth > 100) depth = 100;
    outColor = (vec4(pos.x / 15, pos.y / 15, pos.z / 2, 1) / depth) + vec4(normal.xyz / 20, 1);
    return;
  }

  vec3 color;
  if (texID == -1)
    color = Color;
  else if (texID == 0)
    color = texture(tex0, texcoord).rgb;
  else if (texID == 1)
    color = texture(tex1, texcoord).rgb;
  else if (texID == 2) {
    // http://nuclear.mutantstargoat.com/articles/sdr_fract/
    vec2 z, c;

    c.x = (texcoord.x - 0.5) * scale - 0.777;
    c.y = (texcoord.y - 0.5) * scale;

    int i;
    z = c;
    for(i=0; i<iter; i++) {
      float x = (z.x * z.x - z.y * z.y) + c.x;
      float y = (z.y * z.x + z.x * z.y) + c.y;

      if((x * x + y * y) > 4.0) break;
      z.x = x;
      z.y = y;
    }
    float ramp = (float(i)/100);
    color = texture(fractal, vec2(ramp,0.5)).rgb;
  }
  else{
    outColor = vec4(1,0,0,1);
    return; //This was an error, stop lighting!
  }
  vec3 normal = normalize(vertNormal);
  vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);
  vec3 ambC = color*ambient;
  vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)! (Do you know why?)
  vec3 reflectDir = reflect(viewDir,normal);
  float spec = max(dot(reflectDir,lightDir),0.0);
  if (dot(-lightDir,normal) <= 0.0) spec = 0; //No highlight if we are not facing the light
  float specFactor = 0.7;
  if (texID > -1) specFactor = 0.1;
  vec3 specC = specFactor*vec3(1.0,1.0,1.0)*pow(spec,4);
  vec3 oColor = ambC+diffuseC+specC;
  float lightIntensity = length(oColor);

  if (shaderMode == 0) {
    outColor = vec4(oColor, 1.0);
  } else if (shaderMode == 1) {
    // http://www.sunandblackcat.com/tipFullView.php?l=eng&topicid=27&topic=Hatching-And-Gooch-Shading-GLSL
    vec4 resultingColor = vec4(1.0, 1.0, 1.0, 1.0);

    vec4 black = vec4(0, 0, 0, 1);
    float maxRadius = 4;

    if (lightIntensity < 1.75) {
      if (shouldColorCircle(1, maxRadius / 2)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 1.5) {
      if (shouldColorCircle(2, maxRadius / 2)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 1.25) {
      if (shouldColorCircle(4, maxRadius)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 0.85) {
      // hatch from top left corner to bottom right corner
      if (shouldColorCircle(7, maxRadius)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 0.75) {
      // hatch from right top corner to left boottom
      if (shouldColorCircle(10, maxRadius)) {
        resultingColor = black;
      }
    }
    
    if (lightIntensity < 0.5) {
      // hatch from left top to right bottom
      if (shouldColorCircle(12, maxRadius)) {
        resultingColor = black;
      }
    }
    
    if (lightIntensity < 0.35) {
      // hatch from right top corner to left bottom
      if (shouldColorCircle(16, maxRadius)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 0.15) {
      // hatch from right top corner to left bottom
      if (shouldColorCircle(24, maxRadius)) {
        resultingColor = black;
      }
    }

    if (lightIntensity < 0.05) {
      // hatch from right top corner to left bottom
      if (shouldColorCircle(38, maxRadius)) {
        resultingColor = black;
      }
    }

    outColor = resultingColor;
  } else if (shaderMode == 2) {
    // http://weber.itn.liu.se/~stegu/webglshadertutorial/shadertutorial.html
    float frequency = 200;
    mat2 rot = mat2(0.707, -0.707, .707, .707);
    vec2 nearest = 2.0*fract(frequency * ((rot * gl_FragCoord.xy) / 1200)) - 1.0;
    float dist = length(nearest);
    float radius = (1 / lightIntensity) * 0.3;
    vec3 white = vec3(1.0, 1.0, 1.0);
    vec3 fragcolor = mix(oColor, white, aastep(radius, dist));
    outColor = vec4(fragcolor, 1.0);
  } else if (shaderMode == 3) {
    // http://weber.itn.liu.se/~stegu/webglshadertutorial/shadertutorial.html
    float frequency = 200;
    mat2 rot = mat2(0.707, -0.707, .707, .707);
    vec2 st = (rot * gl_FragCoord.xy) / 1200;
    vec3 black = vec3(0, 0, 0);

    // Perform a rough RGB-to-CMYK conversion
    vec4 cmyk;
    cmyk.xyz = 1.0 - oColor;
    cmyk.w = min(cmyk.x, min(cmyk.y, cmyk.z)); // Create K
    cmyk.xyz -= cmyk.w; // Subtract K equivalent from CMY

    // Distance to nearest point in a grid of
    // (frequency x frequency) points over the unit square
    vec2 Kst = frequency*mat2(0.707, -0.707, 0.707, 0.707)*st;
    vec2 Kuv = 2.0*fract(Kst)-1.0;
    float k = aastep(0.0, sqrt(cmyk.w)-length(Kuv));
    vec2 Cst = frequency*mat2(0.966, -0.259, 0.259, 0.966)*st;
    vec2 Cuv = 2.0*fract(Cst)-1.0;
    float c = aastep(0.0, sqrt(cmyk.x)-length(Cuv));
    vec2 Mst = frequency*mat2(0.966, 0.259, -0.259, 0.966)*st;
    vec2 Muv = 2.0*fract(Mst)-1.0;
    float m = aastep(0.0, sqrt(cmyk.y)-length(Muv));
    vec2 Yst = frequency*st; // 0 deg
    vec2 Yuv = 2.0*fract(Yst)-1.0;
    float y = aastep(0.0, sqrt(cmyk.z)-length(Yuv));

    vec3 rgbscreen = 1.0 - 0.9*vec3(c,m,y);
    rgbscreen = mix(rgbscreen, black, 0.85*k);
    
    outColor = vec4(rgbscreen, 1.0);
  }
}