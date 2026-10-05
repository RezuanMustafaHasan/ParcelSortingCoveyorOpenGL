#version 330 core
struct PointLight { vec3 position; vec3 color; float constant; float linear; float quadratic; };
in vec3 fragmentPosition;
in vec3 normal;
uniform vec3 materialColor;
uniform float shininess;
uniform float emissiveStrength;
uniform vec3 viewPosition;
uniform PointLight pointLights[4];
uniform bool pointLightEnabled;
uniform bool floorMaterial;
uniform vec3 directionalDirection;
uniform vec3 directionalColor;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
out vec4 fragmentColor;

float shadowAmount(vec3 n) {
    vec4 lightPosition = lightSpaceMatrix * vec4(fragmentPosition, 1.0);
    vec3 p = lightPosition.xyz / lightPosition.w * 0.5 + 0.5;
    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 0.0;
    float bias = max(0.003 * (1.0 - dot(n, normalize(-directionalDirection))), 0.0012);
    vec2 texel = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;
    for(int x=-1;x<=1;++x) for(int y=-1;y<=1;++y)
        shadow += p.z - bias > texture(shadowMap, p.xy + vec2(x,y)*texel).r ? 1.0 : 0.0;
    return shadow / 9.0;
}
vec3 phong(vec3 direction, vec3 color, vec3 base) {
    vec3 n=normalize(normal), l=normalize(direction);
    float diffuse=max(dot(n,l),0.0);
    vec3 halfway=normalize(l+normalize(viewPosition-fragmentPosition));
    float specular=pow(max(dot(n,halfway),0.0),shininess)*smoothstep(10.0,80.0,shininess)*0.22;
    return (base*diffuse+specular)*color;
}
void main() {
    vec3 n=normalize(normal);
    vec3 base=materialColor;
    if(floorMaterial) {
        float grain=fract(sin(dot(floor(fragmentPosition.xz*95.0),vec2(12.9898,78.233)))*43758.5453);
        base*=0.95+grain*0.10;
    }
    float sky=n.y*0.5+0.5;
    vec3 ambient=mix(vec3(0.20,0.22,0.21),vec3(0.48,0.50,0.47),sky);
    float shadow=shadowAmount(n);
    vec3 result=base*ambient;
    result+=phong(-directionalDirection,directionalColor,base)*(1.0-shadow*0.78);
    if(pointLightEnabled) for(int i=0;i<4;++i) {
        vec3 d=pointLights[i].position-fragmentPosition;
        float dist=length(d);
        float attenuation=1.0/(pointLights[i].constant+pointLights[i].linear*dist+pointLights[i].quadratic*dist*dist);
        result+=phong(d,pointLights[i].color,base)*attenuation*0.36;
    }
    result+=base*emissiveStrength;
    // Gentle highlight rolloff retains the paint colours under bright fixtures.
    result=result/(vec3(1.0)+result*0.22);
    fragmentColor=vec4(result,1.0);
}
