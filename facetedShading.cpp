/* !
\file		facetedShading.cpp
\author		pghali@digipen.edu
\co-author	jianrong.lim@digipen.edu
\par		Course: CSD 2101
\par		Assignment 02
\date		18/07/2023

\brief
	This file implements functions that are used to create pixel buffer objects,
	texture objects, rectangular models, setting up of shader program. Some functions
	also do the work of emulating the graphics pipe, filling up PBO with data,
	rendering models in different modes, setting a color and clearing the color buffer with
	said color, rendering texture objects, calculating new coordinates of
	vertices in a model.
*//*__________________________________________________________________________*/

/*!	set_pixel
\brief
	Function that updates a location in PBO with a color value

\param x
	Reference to x coordinate of the PBO to be updated

\param y
	Reference to y coordinate of the PBO to be updated

\param clr
	Reference to color value that the coordinate in PBO is to be updated with

\return None
*/
void GLPbo::set_pixel(GLint& x, GLint& y, GLPbo::Color& clr) {
	//if invalid coordinate, return
	if (x < 0 || x >= GLHelper::width || y < 0 || y >= GLHelper::height) {
		return;
	}
	ptr_to_pbo[(y * GLHelper::width) + x] = clr;
}

/*!	PointInEdgeTopLeft
\brief
	Function to check if a point is on the interior/positive side of an edge

\param  E
	reference to the edge that the point is to be compared with

\param x
	x coordinate of the point

\param y
	y coordinate of the point

\param Eval
	reference to Eval

\return
	returns true if the point is on the interior side of the edge, false otherwise
*/
bool GLPbo::PointInEdgeTopLeft(GLPbo::EdgeEquation& E, float x, float y, float& Eval) {
	Eval = E.a * x + E.b * y + E.c;
	return (Eval > 0.f || (Eval == 0.f && E.tl)) ? true : false;
}

/*!	render_triangle_faceted
\brief
	Function that renders triangles using faceted shading through the use
	of triangle normals, triangle centroid, as well as a light source.

\param  p0
	Reference to window coordinates of first vertex of triangle

\param  p1
	Reference to window coordinates of second vertex of triangle

\param  p2
	Reference to window coordinates of third vertex of triangle

\param  pm0
	Reference to model coordinates of first vertex of triangle

\param  pm1
	Reference to model coordinates of second vertex of triangle

\param  pm2
	Reference to model coordinates of third vertex of triangle

\return
	Returns true if triangle is front facing and rendering of triangle is successful.
	Returns false otherwise
*/
bool GLPbo::render_triangle_faceted(glm::vec3 const& p0, glm::vec3 const& p1, glm::vec3 const& p2,
	glm::vec3 const& pm0, glm::vec3 const& pm1, glm::vec3 const& pm2) {

	//check for back facing
	float signed_area = 0.5f * ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
	if (signed_area <= 0.f) {
		++cull_count;
		return false;
	}

	//compute edge 2
	GLPbo::EdgeEquation p0p1{};
	p0p1.a = p0.y - p1.y;
	p0p1.b = p1.x - p0.x;
	p0p1.c = (p1.y - p0.y) * p0.x - (p1.x - p0.x) * p0.y;
	if (p0p1.a > 0.f) {
		p0p1.tl = GL_TRUE;
	}
	else if (p0p1.a != 0.f && p0p1.b < 0.f) {
		p0p1.tl = GL_TRUE;
	}

	//compute edge 0
	GLPbo::EdgeEquation p1p2{};
	p1p2.a = p1.y - p2.y;
	p1p2.b = p2.x - p1.x;
	p1p2.c = (p2.y - p1.y) * p1.x - (p2.x - p1.x) * p1.y;
	if (p1p2.a > 0.f) {
		p1p2.tl = GL_TRUE;
	}
	else if (p1p2.a != 0.f && p1p2.b < 0.f) {
		p1p2.tl = GL_TRUE;
	}

	//compute edge 1
	GLPbo::EdgeEquation p2p0{};
	p2p0.a = p2.y - p0.y;
	p2p0.b = p0.x - p2.x;
	p2p0.c = (p0.y - p2.y) * p2.x - (p0.x - p2.x) * p2.y;
	if (p2p0.a > 0.f) {
		p2p0.tl = GL_TRUE;
	}
	else if (p2p0.a != 0.f && p2p0.b < 0.f) {
		p2p0.tl = GL_TRUE;
	}

	//compute AABB
	float xmin = std::floor(std::min({ p0.x, p1.x, p2.x }));

	float ymin = std::floor(std::min({ p0.y, p1.y, p2.y }));

	float xmax = std::ceil(std::max({ p0.x, p1.x, p2.x }));

	float ymax = std::ceil(std::max({ p0.y, p1.y, p2.y }));

	float Eval0, Eval1, Eval2;
	//compute Eval
	PointInEdgeTopLeft(p0p1, xmin + 0.5f, ymin + 0.5f, Eval2);
	PointInEdgeTopLeft(p1p2, xmin + 0.5f, ymin + 0.5f, Eval0);
	PointInEdgeTopLeft(p2p0, xmin + 0.5f, ymin + 0.5f, Eval1);

	float twice_area_reciprocal = (1.f / (2.f * signed_area));
	//barycentric coordinates
	float a = twice_area_reciprocal * Eval0;
	float b = twice_area_reciprocal * Eval1;
	float c = 1.f - a - b;

	//compute initial interpolated depth
	float initial_interpolated_depth{ static_cast<float>(a) * p0.z + static_cast<float>(b) * p1.z + static_cast<float>(c) * p2.z };

	//compute increments in depth value
	float interpolated_depth_x_increment = { (float)(p1p2.a) * p0.z + (float)(p2p0.a) * p1.z
								+ (float)(p0p1.a) * p2.z };
	interpolated_depth_x_increment *= twice_area_reciprocal;

	float interpolated_depth_y_increment = { static_cast<float>(p1p2.b) * p0.z + static_cast<float>(p2p0.b) * p1.z
								+ static_cast<float>(p0p1.b) * p2.z };
	interpolated_depth_y_increment *= twice_area_reciprocal;

	//compute normal of the triangle
	glm::vec3 linep0p1 = pm1 - pm0;
	glm::vec3 linep0p2 = pm2 - pm0;
	glm::vec3 triangle_nml = glm::normalize(glm::cross(linep0p1, linep0p2));

	//vertex pos must be at triangle centroid
	glm::vec3 vtx_pos = { (1 / 3.f) * (pm0.x + pm1.x + pm2.x),
						  (1 / 3.f) * (pm0.y + pm1.y + pm2.y),
						  (1 / 3.f) * (pm0.z + pm1.z + pm2.z) };

	//vector from triangle centroid to light source
	glm::vec3 vtx_to_light = glm::normalize(point_light.mdl_pos - vtx_pos); //is point_light pos correctly xformed?

	float cos_theta = glm::dot(triangle_nml, vtx_to_light) > 0 ? glm::dot(triangle_nml, vtx_to_light) : 0; //WHICH VERTEX NML TO USE?? NML0? NML1? NML2

	//compute incoming light
	glm::vec3 incoming_light = cos_theta * point_light.intensity;
	//std::cout << "point lighht " << point_light.intensity.x << point_light.intensity.y<< point_light.intensity.z <<"\n\n\n\n\n\n\n\n";
	glm::vec3 diffuse_color = { 1.f,1.f,1.f };

	//compute outgoing light
	glm::vec3 outgoing_light = { diffuse_color.x * incoming_light.x,
								diffuse_color.y * incoming_light.y,
								diffuse_color.z * incoming_light.z };
	//iterate through AABB bounding box, increment color value regardless if point is in triangle
	for (int y = static_cast<int>(ymin); y < static_cast<int>(ymax); ++y) {
		float hEval0 = Eval0, hEval1 = Eval1, hEval2 = Eval2;
		float tmp_depth = initial_interpolated_depth;

		for (int x = static_cast<int>(xmin); x < static_cast<int>(xmax); ++x) {
			if (PointInEdgeTopLeft(p1p2, x + 0.5f, y + 0.5f, hEval0) && PointInEdgeTopLeft(p0p1, x + 0.5f, y + 0.5f, hEval2)
				&& PointInEdgeTopLeft(p2p0, x + 0.5f, y + 0.5f, hEval1)) {

				//if zd is less than zd value in depth_buffer, set corresponding location in color buffer to new color value
				if (tmp_depth < depth_buffer[y * GLPbo::width + x]) {
					depth_buffer[y * GLPbo::width + x] = tmp_depth;
					//all fragments in triangle will have the same color
					GLPbo::Color color = { static_cast<GLubyte>((outgoing_light.x) * 255),
										   static_cast<GLubyte>((outgoing_light.y) * 255),
										   static_cast<GLubyte>((outgoing_light.z) * 255),
										   (255) };

					GLPbo::set_pixel(x, y, color);
				}
			}

			//increment value
			tmp_depth += interpolated_depth_x_increment;
		}

		//increment value
		initial_interpolated_depth += interpolated_depth_y_increment;
	}
	return true;
}