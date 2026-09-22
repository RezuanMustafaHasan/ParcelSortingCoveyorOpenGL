#version 330 core

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

in vec3 fragmentPosition;
in vec3 normal;

uniform vec3 materialColor;
uniform float shininess;
uniform float emissiveStrength;
uniform vec3 viewPosition;
uniform PointLight pointLight;
uniform bool pointLightEnabled;
uniform vec3 directionalDirection;
uniform vec3 directionalColor;

out vec4 fragmentColor;

vec3 phong(vec3 lightDirection, vec3 lightColor, float intensity) {
    vec3 n = normalize(normal);
    vec3 l = normalize(lightDirection);
    float diffuseAmount = max(dot(n, l), 0.0);
    vec3 viewDirection = normalize(viewPosition - fragmentPosition);
    vec3 reflected = reflect(-l, n);
    float specularAmount = pow(max(dot(viewDirection, reflected), 0.0), shininess);
    vec3 diffuse = diffuseAmount * materialColor * lightColor;
    vec3 specular = specularAmount * lightColor * 0.42;
    return (diffuse + specular) * intensity;
}

void main() {
    vec3 result = materialColor * vec3(0.16, 0.18, 0.21);

    vec3 directionToPoint = pointLight.position - fragmentPosition;
    float distanceToPoint = length(directionToPoint);
    float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distanceToPoint
                              + pointLight.quadratic * distanceToPoint * distanceToPoint);
    if (pointLightEnabled)
        result += phong(directionToPoint, pointLight.color, attenuation * 1.65);

    result += phong(-directionalDirection, directionalColor, 0.52);
    result += materialColor * emissiveStrength;
    fragmentColor = vec4(result, 1.0);
}
