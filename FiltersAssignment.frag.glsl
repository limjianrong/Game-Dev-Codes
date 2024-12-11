/*!*****************************************************************************
\file FiltersAssignment.frag.glsl
\author Vadim Surov (vsurov\@digipen.edu)
\co-author Lim Jian Rong, jianrong.lim (jianrong.lim@digipen.edu)
\par Course: CSD2151 Section A
\par Assignment: 9 (Filters App)
\date 03/16/2024 (MM/DD/YYYY)
\brief This file implements a fragment shader that uses multi-pass rendering to
       implement edge and blur filtering.
*******************************************************************************/
R"(
#version 420

uniform int Pass; // Pass number

struct Light 
{
    vec3 position;      // Position of the light source in the world space   
    vec3 La;             // Light intensity (for PBR), same as LA???
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

// Material properties
struct Material 
{
    // For PBR
    float rough;            // Roughness
    float metal;            // Metallic (1.0f) or dielectric (0.0f)
    vec3 color;             // Diffuse color for dielectrics, f0 for metallic

    vec3 Ka;            // Ambient reflectivity
    vec3 Kd;            // Diffuse reflectivity
    vec3 Ks;            // Specular reflectivity
    float shininess;    // Specular shininess factor

    float effect;           // Additional effect (discard, cartoon, etc)
};

layout(location=0) out vec4 FragColor;

// Pass 0
layout(binding=0) uniform sampler2D Texture;

in vec3 Vec;
in vec3 TexCoord;
// Pass 1

uniform mat4 V;         // View transform matrix

uniform Material material;
uniform Light light[1];
uniform int render_mode;

in vec3 Position;
in vec3 Normal;

uniform float EdgeThreshold; //edge threshold
float Weight[5] = { 0.158435f, 0.148836f, 0.123389f, 0.0902733f, 0.0582848f }; //gaussian blur

/*
    \brief
    Helper function used for edge filter

    \return
    returns a float
*/
float luminance(vec3 color) 
{
    return dot(vec3(0.2126f, 0.7152f, 0.0722f), color);
}
/*
    \brief
    shading using blinn phong

    \param position
    position of vertex

    \param n
    Normal

    \param light
    light source

    \param material
    material of object

    \param view
    view xform matrix

    \return
    color
*/
vec3 blinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
{

    if (light.position == position) {
        return vec3(0.f);
    }

    //calculate ambient contribution
    vec3 ambient_contribution = light.La * material.Ka;

    //calculate light pos in view space
    vec3 light_pos_view_space = vec3(view * vec4(light.position, 1.0));
    vec3 light_direction = normalize(light_pos_view_space - position);

    //use lambert cosine law and clamp
    float cos_theta = max(dot(normal, light_direction),0.f);

    //calculate diffuse contribution
    vec3 diffuse_contribution = material.Kd * light.Ld * cos_theta;

    //calculate vector from fragment to viewer
    vec3 view_direction = normalize(-position);

    //calculate halfway vector
    vec3 half_vec = normalize(light_direction + view_direction);
    half_vec = normalize(half_vec);

    float r_dot_v = 0.f;
    if (cos_theta > 0) {
        r_dot_v = dot(normalize(normal), half_vec);
    }
    if (r_dot_v < 0.f) {
        r_dot_v = 0.f;
    }

    float r_dot_v_powered = pow(r_dot_v,material.shininess);
    //calculate specular contribution
    vec3 specular_contribution = material.Ks * light.Ls * r_dot_v_powered;

    //sum of all vec3 will give final color
    vec3 final = ambient_contribution + diffuse_contribution + specular_contribution;
    return final;
}

/*
    \brief
    blinn phong
*/
void pass0() 
{
    FragColor = vec4(blinnPhong(Position, normalize(Normal), light[0], material, V), 1.0f);
}

/*
    \brief
    blur pass1, mode 1 is no filter and mode 3 is only edge filter, thus no blur for mode 1 and 3
    mode 2 is blur only and mode 4 is blur + edge filter combined
*/
void pass1() 
{
    if(render_mode==1 || render_mode==3){
           vec3 final_color;
           final_color = texture(Texture, TexCoord.xy).rgb;
           FragColor = vec4(final_color,1.f);

    }
    else {
        ivec2 pix = ivec2(gl_FragCoord.xy);
        vec4 sum = texelFetch(Texture, pix, 0);

        sum *= Weight[0];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-4)) * Weight[4];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-3)) * Weight[3];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-2)) * Weight[2];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0,-1)) * Weight[1];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 1)) * Weight[1];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 2)) * Weight[2];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 3)) * Weight[3];
        sum += texelFetchOffset(Texture, pix, 0, ivec2(0, 4)) * Weight[4];

        FragColor = sum;
    }
}

/*
    \brief
    blur pass2, mode 1 is no filter and mode 3 is only edge filter, thus no blur for mode 1 and 3
    mode 2 is blur only and mode 4 is blur + edge filter combined
*/
void pass2() 
{    
    if(render_mode==1 || render_mode==3){
           vec3 final_color;
           final_color = texture(Texture, TexCoord.xy).rgb;
           FragColor = vec4(final_color,1.f);
    }
    else{
        ivec2 pix = ivec2(gl_FragCoord.xy);
    vec4 sum = texelFetch(Texture, pix, 0);
        
    sum *= Weight[0];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(-4,0)) * Weight[4];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(-3,0)) * Weight[3];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(-2,0)) * Weight[2];
    sum += texelFetchOffset(Texture, pix, 0, ivec2(-1,0)) * Weight[1];
    sum += texelFetchOffset(Texture, pix, 0, ivec2( 1,0)) * Weight[1];
    sum += texelFetchOffset(Texture, pix, 0, ivec2( 2,0)) * Weight[2];
    sum += texelFetchOffset(Texture, pix, 0, ivec2( 3,0)) * Weight[3];
    sum += texelFetchOffset(Texture, pix, 0, ivec2( 4,0)) * Weight[4];

    FragColor = sum;
    }
}

/*
    \brief
    edge filter on the last pass, only apply edge filter to mode 3 and 4
*/
void pass3() 
{

    if(render_mode==1 || render_mode==2){
          vec3 final_color;
          final_color = texture(Texture, TexCoord.xy).rgb;
           FragColor = vec4(final_color,1.f);
    }
    else{
        ivec2 pix = ivec2(gl_FragCoord.xy);

        float s00 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1, 1)).rgb);
        float s10 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1, 0)).rgb);
        float s20 = luminance(texelFetchOffset(Texture, pix, 0, ivec2(-1,-1)).rgb);
        float s01 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 0, 1)).rgb);
        float s21 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 0,-1)).rgb);
        float s02 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1, 1)).rgb);
        float s12 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1, 0)).rgb);
        float s22 = luminance(texelFetchOffset(Texture, pix, 0, ivec2( 1,-1)).rgb);

        float sx = s00 + 2*s10 + s20 - (s02 + 2*s12 + s22);
        float sy = s00 + 2*s01 + s02 - (s20 + 2*s21 + s22);

        float g = sx*sx + sy*sy;
        FragColor = (g > EdgeThreshold) ? vec4(1.0f) : vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}


void main()
{
        if      (Pass==0) pass0();
        else if (Pass==1) pass1();
        else if (Pass==2) pass2();
        else if (Pass==3) pass3();
}

)"