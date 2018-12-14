#version 330 core

out vec4 FragColor;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse1;
uniform samplerCube depthMap;


uniform float far_plane;
in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in float visibility; 

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec4 temp = texture(texture_diffuse1, TexCoords);
  
  if(temp.w < 0.5)
  discard;
  else
  {
    vec4 outColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(temp.xyz, 1.0), visibility);
    FragColor = vec4(outColor);
   }

}
