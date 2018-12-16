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

const float ambient = .25;

// Fractal parameters
const int iter = 150;
const float scale = 2;

void main() {
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

    c.x = 1.3333 * (texcoord.x - 0.5) * scale;
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
  outColor = vec4(oColor,1);
}