#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

struct PointLight { vec3 position; vec3 color; float constant; float linear; float quadratic; };

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool useGouraud;
uniform vec3 materialColor;
uniform float shininess;
uniform vec3 viewPosition;
uniform PointLight pointLights[4];
uniform bool pointLightEnabled;
uniform vec3 directionalDirection;
uniform vec3 directionalColor;

out vec3 fragmentPosition;
out vec3 normal;
out vec3 gouraudColor;
out vec3 gouraudDirectionalColor;

// Same lighting formula as the fragment shader, evaluated at a vertex.
vec3 lightingAtVertex(vec3 direction, vec3 color) {
    vec3 n = normalize(normal), l = normalize(direction);
    float diffuse = max(dot(n, l), 0.0);
    vec3 halfway = normalize(l + normalize(viewPosition - fragmentPosition));
    float specular = pow(max(dot(n, halfway), 0.0), shininess)
                   * smoothstep(10.0, 80.0, shininess) * 0.22;
    return (materialColor * diffuse + specular) * color;
}

void main() {
    vec4 worldPosition = model * vec4(aPosition, 1.0);
    fragmentPosition = worldPosition.xyz;
    normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * worldPosition;

    gouraudColor = vec3(0.0);
    gouraudDirectionalColor = vec3(0.0);
    if (useGouraud) {
        // Gouraud: calculate vertex colours; OpenGL interpolates them across triangles.
        float sky = normalize(normal).y * 0.5 + 0.5;
        vec3 ambient = mix(vec3(0.20, 0.22, 0.21), vec3(0.48, 0.50, 0.47), sky);
        gouraudColor = materialColor * ambient;
        // Keep daylight separate so the existing shadow map still works.
        gouraudDirectionalColor = lightingAtVertex(-directionalDirection, directionalColor);
        if (pointLightEnabled) for (int i = 0; i < 4; ++i) {
            vec3 d = pointLights[i].position - fragmentPosition;
            float dist = length(d);
            float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist
                                    + pointLights[i].quadratic * dist * dist);
            gouraudColor += lightingAtVertex(d, pointLights[i].color) * attenuation * 0.36;
        }
    }
}
