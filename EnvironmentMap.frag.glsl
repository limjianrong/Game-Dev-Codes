/*!*****************************************************************************
\file EnvironmentMap.frag.glsl
\author Vadim Surov (vsurov\@digipen.edu)
\co-author Lim Jian Rong, jianrong.lim (jianrong.lim@digipen.edu)
\par Course: CSD2151 Section A
\par Assignment: 5.2
\date 02/12/2024 (MM/DD/YYYY)
\brief This file implements a fragment shader that uses multi-pass rendering,
       environment mapping, to create a checkerboard texture and map it onto objects,
       as well as applying reflection, refraction and gamma correction.
*******************************************************************************/
R"(
#version 420

// Material properties for reflection and refraction
struct Material 
{
    vec4 color;             
    float reflectionFactor; // The light reflection factor
    float eta;              // The ratio of indices of refraction
};

uniform int Pass; // Pass number

layout(location=0) out vec4 FragColor;

uniform Material material;


in vec3 Vec;

vec4 checkerboardTexture(vec2 uv, float size)
{
    //column
    float col_float = (floor(uv.x * size));
    int column = int(col_float);

    //row
    float row_float = (floor(uv.y * size));
    int row =int (row_float);

    //return black color
    if (((((column% 2) == 0) && (row%2 )==0)) || ((((column% 2) == 1) && ((row% 2) == 1)))) {
        return vec4 (0.f,0.f,0.f,1.f);
    }

    //return white color
    return vec4(1.0f);
}

/*
    Read specs for Assignment 5.1
*/
vec2 vec2uv(vec3 v)
{
    //normalize first
    v = normalize(v);


    //calculate absolute values of x,y,z components
    vec3 abs_vec = v;
    if (abs_vec.x < 0.f) {
        abs_vec.x *= -1.f;
    }

    if (abs_vec.y < 0.f) {
        abs_vec.y *= -1.f;
    }

    if (abs_vec.z < 0.f) {
        abs_vec.z *= -1.f;
    }

    //uv coordinates
    float u = 0, v_coord = 0;


    //check top or bottom face
    if (abs_vec.y > abs_vec.x && abs_vec.y >= abs_vec.z) {
        return vec2(0.f);
    }

    // check which side of cube is being intersected
    //check left or side
    if (abs_vec.x >= abs_vec.y && abs_vec.x >= abs_vec.z) {
        u = (-v.z / abs_vec.x);
        u *= 0.5f;
        u += 0.5f;
        bool intersect_left = v.x < 0.f ? true : false;
        //if left side
        if (intersect_left) {
            u *= -1.f;
        }

        v_coord = (-v.y / abs_vec.x);
        v_coord *= 0.5f;
        v_coord += 0.5f;
        u = 1.0f - u;

        if (abs_vec.x == abs_vec.y && abs_vec.y == abs_vec.z) {
            u = 0.f;
        }

        if (abs_vec.x == abs_vec.z) {
            u = 0.f;
        }

    }

    //else, front or back
    else {
        u =  (v.x / abs_vec.z);
        u *= 0.5f;
        u += 0.5f;

        v_coord =  (-v.y / abs_vec.z);
        v_coord *= 0.5f;
        v_coord += 0.5f;

        bool intersect_outwards = v.z > 0.f ? true : false;
        //if outwards
        if (intersect_outwards) {
            u = 1.f - u;
        }

    }

    //clamp
    if (u > 1.f) {
        u -= 1.f;
    }
    if (u < 0.f) {
        u = 0.f;
    }

    v_coord = 1.f - v_coord;
    if (v_coord > 1.f) {
        v_coord -= 1.f;
    }
    if (v_coord < 0.f) {
        v_coord = 0.f;
    }
    return vec2(u, v_coord);
}

// Pass 0
void pass0() 
{
    // Access the cube map texture
    vec2 uv_coord = vec2uv(normalize(Vec)); // Ensure Vec is normalized

    // Generate checkerboard texture color
    vec3 black_or_white = vec3(checkerboardTexture(uv_coord, 10.0)); // Adjust the size as needed

    black_or_white = pow(black_or_white, vec3(1.f/2.2f));
    
    FragColor = vec4(black_or_white,1.f);
}

// Pass 1

in vec3 ReflectDir;
in vec3 RefractDir;

uniform float reflection_factor;

void pass1() 
{
    //normalize vectors
    vec3 normalized_reflect_dir = normalize(ReflectDir);
    vec3 normalized_refract_dir = normalize(RefractDir);

    //calculate uv coordinates of reflect and refract vector
    vec2 uv_coord_reflect = vec2uv(normalized_reflect_dir);
    vec2 uv_coord_refract = vec2uv(normalized_refract_dir);

    //calculate color
    vec3 reflected_color = vec3(checkerboardTexture(uv_coord_reflect,10.0));

    vec3 refracted_color = vec3(checkerboardTexture(uv_coord_refract,10.0));
    
    vec3 final_color = mix(refracted_color.rgb, reflected_color.rgb,reflection_factor);
    //allow for gamma correction
   final_color = pow(final_color, vec3(1.0f/2.2f));

   FragColor = vec4(final_color, 1.0); // Set fragment color

}


void main()
{
    if      (Pass==0) pass0();
    else if (Pass==1) pass1();
}
)"