/*!*****************************************************************************
\file BlinnPhong.frag.glsl
\author Vadim Surov (vsurov\@digipen.edu)
\co-author Lim Jian Rong, jianrong.lim (jianrong.lim@digipen.edu)
\par Course: CSD2151 Section A
\par Assignment: 4.2
\date 03/02/2024
\brief This file implements a fragment shader that applies Blinn-Phong shading
       to a pixel to determine its final color based on ambient, diffuse, specular
       reflectivity and light intensity. Shader can also apply Phong shading.
*******************************************************************************/
R"(
#version 330 core

struct Material 
{
    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor
};

struct Light 
{
    vec3 position;      // Position of the light source in the world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space

uniform Light light[1];
uniform Material material;
uniform mat4 V;         // View transform matrix
uniform bool use_blinn_phong_shading;
layout(location=0) out vec4 FragColor;


vec3 BlinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{
    if (light.position == position) {
        return vec3(0.f);
    }

    //calculate ambient contribution
    vec3 ambient_contribution = light.La * material.Ka;

    //calculate light pos in view space
    vec3 light_pos_view_space = vec3(view * vec4(light.position, 1.0f));
    vec3 light_direction = normalize(light_pos_view_space - position);
    light_direction = normalize(light_direction);

    //use lambert cosine law and clamp
    float cos_theta = dot(normalize(normal), normalize(light_direction));
    if (cos_theta < 0) {
        cos_theta = 0.f;
    }
    //calculate diffuse contribution
    vec3 diffuse_contribution = material.Kd * light.Ld * cos_theta;

    //calculate vector from fragment to viewer
    vec3 view_direction = normalize(-position);

    //calculate halfway vector
    vec3 half_vec = (light_direction + view_direction);
    half_vec = normalize(half_vec);

    float r_dot_v = 0.f;
    if (cos_theta > 0) {
        r_dot_v = dot(normalize(normal), half_vec);
    }
    if (r_dot_v < 0) {
        r_dot_v = 0.f;
    }

    float r_dot_v_powered = pow(r_dot_v, material.shininess);
    //calculate specular contribution
    vec3 specular_contribution = material.Ks * light.Ls * r_dot_v_powered;

    //sum of all vec3 will give final color
    vec3 final = ambient_contribution + diffuse_contribution + specular_contribution;

    return final;
}

vec3 Phong(vec3 position, vec3 normal)
{
    if (light[0].position == position) {
        return vec3(0.f);
    }

    //calculate ambient contribution
    vec3 ambient_contribution = light[0].La * material.Ka;

    //calculate light pos in view space
    vec3 light_pos_view_space = vec3(V * vec4(light[0].position, 1.0f));
    vec3 light_direction = normalize(light_pos_view_space - position);
    light_direction = normalize(light_direction);
    vec3 light_reflected_direction = reflect(-light_direction, normal);

    //use lambert cosine law and clamp
    float cos_theta = dot(normalize(normal), normalize(light_direction));
    if (cos_theta < 0) {
        cos_theta = 0.f;
    }
    //calculate diffuse contribution
    vec3 diffuse_contribution = material.Kd * light[0].Ld * cos_theta;

    //calculate vector from fragment to viewer
    vec3 view_direction = normalize(-position);

    //calculate halfway vector
    vec3 half_vec = (light_direction + view_direction);
    half_vec = normalize(half_vec);

    float n_dot_h = 0.f;
    if (cos_theta > 0) {
        n_dot_h = dot(normalize(view_direction), light_reflected_direction);
    }
    if (n_dot_h < 0) {
        n_dot_h = 0.f;
    }

    float n_dot_h_powered = pow(n_dot_h, material.shininess);
    //calculate specular contribution
    vec3 specular_contribution = material.Ks * light[0].Ls * n_dot_h_powered;

    //sum of all vec3 will give final color
    vec3 final = ambient_contribution + diffuse_contribution + specular_contribution;

    return final;
}

void main() 
{
    if(use_blinn_phong_shading){
        FragColor = vec4(BlinnPhong(Position, normalize(Normal), light[0], material, V), 1.0f);

    }

    //else, use phong shading
    else{
        FragColor = vec4(Phong(Position, normalize(Normal)), 1.0f);
    }
}
)"