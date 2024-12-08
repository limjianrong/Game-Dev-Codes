/* !
\file		flatAndSmoothShaded.cpp
\author		pghali@digipen.edu
\co-author	jianrong.lim@digipen.edu
\par		Course: CSD 2101
\par		Assignment 01
\date		30/06/2023

\brief
	This file implements functions that are used to create pixel buffer objects,
	texture objects, rectangular models, setting up of shader program. Some functions
	also do the work of emulating the graphics pipe, filling up PBO with data,
	rendering a rectangular model, setting a color and clearing the color buffer with
	said color, rendering texture objects such as models of a cube, teapot and more.
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
	glScissor(0, 0, GLHelper::width, GLHelper::height);
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

/*!	render_triangle
\brief
	Function to render a flat shaded triangle.

\param  p0
	reference to the first point of the triangle

\param p1
	reference to the second point of the triangle

\param p2
	reference to the third point of the triangle

\param clr
	color to be used to draw the flat shaded triangle

\return
	returns true if the PBO was successfully updated with values
	needed to render a flat shaded triangle, false if triangle
	is front facing
*/
bool GLPbo::render_triangle(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 clr) {

	//check for back facing
	float signed_area = 0.5f * ((p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y));
	if (signed_area <= 0.f) {
		++cull_count;
		return false;
	}

	//convert color in glm::vec3 to type GLPbo::Color
	GLPbo::Color draw_clr = { static_cast<GLubyte>(clr.x * 255.f),static_cast<GLubyte>(clr.y * 255.f), static_cast<GLubyte>(clr.z * 255.f), static_cast<GLubyte>(255.0f) };

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
	if (p1p2.a > 0) {
		p1p2.tl = GL_TRUE;
	}
	else if (p1p2.a != 0 && p1p2.b < 0) {
		p1p2.tl = GL_TRUE;
	}

	//compute edge 1
	GLPbo::EdgeEquation p2p0{};
	p2p0.a = p2.y - p0.y;
	p2p0.b = p0.x - p2.x;
	p2p0.c = (p0.y - p2.y) * p2.x - (p0.x - p2.x) * p2.y;
	if (p2p0.a > 0) {
		p2p0.tl = GL_TRUE;
	}
	else if (p2p0.a != 0 && p2p0.b < 0) {
		p2p0.tl = GL_TRUE;
	}

	//compute AABB of triangle
	float xmin = std::floor(std::min({ p0.x, p1.x, p2.x }));

	float ymin = std::floor(std::min({ p0.y, p1.y, p2.y }));

	float xmax = std::ceil(std::max({ p0.x, p1.x, p2.x }));

	float ymax = std::ceil(std::max({ p0.y, p1.y, p2.y }));
	float Eval0, Eval1, Eval2;
	PointInEdgeTopLeft(p0p1, xmin + 0.5f, ymin + 0.5f, Eval2);
	PointInEdgeTopLeft(p1p2, xmin + 0.5f, ymin + 0.5f, Eval0);
	PointInEdgeTopLeft(p2p0, xmin + 0.5f, ymin + 0.5f, Eval1);

	//iterate throught AABB bounding box, if point is in triangle, update PBO with a color value
	for (int y = static_cast<int>(ymin); y < static_cast<int>(ymax); ++y) {
		float hEval0 = Eval0, hEval1 = Eval1, hEval2 = Eval2;
		for (int x = static_cast<int>(xmin); x < static_cast<int>(xmax); ++x) {
			if (PointInEdgeTopLeft(p1p2, x + 0.5f, y + 0.5f, hEval0) && PointInEdgeTopLeft(p0p1, x + 0.5f, y + 0.5f, hEval2)
				&& PointInEdgeTopLeft(p2p0, x + 0.5f, y + 0.5f, hEval1)) {
				GLPbo::set_pixel(x, y, draw_clr);
			}
			hEval0 += p1p2.a;
			hEval1 += p2p0.a;
			hEval2 += p0p1.a;
		}
		Eval0 += p1p2.b;
		Eval1 += p2p0.b;
		Eval2 += p0p1.b;
	}
	return true;
}


/*!	render_triangle
\brief
	Function to render a smooth shaded triangle.

\param  p0
	reference to the first point of the triangle

\param p1
	reference to the second point of the triangle

\param p2
	reference to the third point of the triangle

\param c0
	color value of the first point of the triangle

\param c1
	color value of the second point of the triangle

\param c2
	color value of the third point of the triangle

\return
	returns true if the PBO was successfully updated with values
	needed to render a smooth shaded triangle, false if triangle
	is front facing
*/
bool GLPbo::render_triangle(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 c0,
	glm::vec3 c1, glm::vec3 c2) {

	//values are [0,1], multiply by 255 to get color values
	c0 *= 255.f;
	c1 *= 255.f;
	c2 *= 255.f;

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

	//barycentric coordinates
	float a = (1.f / (2.f * signed_area)) * Eval0;
	float b = (1.f / (2.f * signed_area)) * Eval1;
	float c = 1.f - a - b;

	//compute color
	glm::vec3 color_vec3{ static_cast<float>(a) * c0 + static_cast<float>(b) * c1 + static_cast<float>(c) * c2 };

	//compute increments in color value
	glm::vec3 color_x_increment = { (float)(p1p2.a) * c0 + (float)(p2p0.a) * c1
								+ (float)(p0p1.a) * c2 };
	color_x_increment *= (1.f / (2.f * signed_area));

	glm::vec3 color_y_increment = { static_cast<float>(p1p2.b) * c0 + static_cast<float>(p2p0.b) * c1
								+ static_cast<float>(p0p1.b) * c2 };
	color_y_increment *= (1.f / (2.f * signed_area));

	//iterate through AABB bounding box, increment color value regardless if point is in triangle
	for (int y = static_cast<int>(ymin); y < static_cast<int>(ymax); ++y) {
		float hEval0 = Eval0, hEval1 = Eval1, hEval2 = Eval2;
		glm::vec3 tmp_clr = color_vec3;
		for (int x = static_cast<int>(xmin); x < static_cast<int>(xmax); ++x) {
			if (PointInEdgeTopLeft(p1p2, x + 0.5f, y + 0.5f, hEval0) && PointInEdgeTopLeft(p0p1, x + 0.5f, y + 0.5f, hEval2)
				&& PointInEdgeTopLeft(p2p0, x + 0.5f, y + 0.5f, hEval1)) {
				GLPbo::Color color = { static_cast<GLubyte>(tmp_clr.x), static_cast<GLubyte>(tmp_clr.y),
				static_cast<GLubyte>(tmp_clr.z), (255) };
				GLPbo::set_pixel(x, y, color);
			}
			hEval0 += p1p2.a;
			hEval1 += p2p0.a;
			hEval2 += p0p1.a;

			//increment color value
			tmp_clr += color_x_increment;
		}
		Eval0 += p1p2.b;
		Eval1 += p2p0.b;
		Eval2 += p0p1.b;

		//increment color value
		color_vec3 += color_y_increment;
	}
	return true;
}