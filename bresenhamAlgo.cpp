/* !
\file		glpbo.cpp
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

void GLPbo::set_pixel(GLint& x, GLint& y, GLPbo::Color& clr) {
	//if invalid coordinate, return
	if (x < 0 || x >= GLHelper::width || y < 0 || y >= GLHelper::height) {
		return;
	}
	glScissor(0, 0, GLHelper::width, GLHelper::height);
	ptr_to_pbo[(y * GLHelper::width) + x] = clr;
}

void GLPbo::render_linebresenham(GLint px0, GLint py0, GLint px1, GLint py1, glm::vec3 draw_clr) {

	int dx = px1 - px0;
	int dy = py1 - py0;
	int xstep = (dx < 0) ? -1 : 1;
	int ystep = (dy < 0) ? -1 : 1;
	dx = (dx < 0) ? -dx : dx;
	dy = (dy < 0) ? -dy : dy;

	//convert color in glm::vec3 to GLPbo::Color
	GLPbo::Color clr = { static_cast<GLubyte>(draw_clr.x * 255.f), static_cast<GLubyte>(draw_clr.y * 255.f) , static_cast<GLubyte>(draw_clr.z * 255.f), 255 };
	if (dx == 0) {
		for (int y = py0; y != py1; y += ystep) {
			set_pixel(px0, y, clr);
		}
		set_pixel(px1, py1, clr);
	}
	else if (dy == 0) {
		for (int x = px0; x != px1; x += xstep) {
			set_pixel(x, py0, clr);
		}
		set_pixel(px1, py1, clr);
	}

	//For octants 0347
	else if (abs(dy) <= abs(dx)) {
		int d = 2 * dy - dx;
		int dmin = 2 * dy;
		int dmaj = 2 * dy - 2 * dx;
		set_pixel(px0, py0, clr);
		while (--dx) {
			py0 += (d > 0) ? ystep : 0;
			d += (d > 0) ? dmaj : dmin;
			px0 += xstep;
			set_pixel(px0, py0, clr);
		}
	}

	//for octants 1256
	else {
		int d = 2 * dx - dy;
		int dmin = 2 * dx;
		int dmaj = 2 * dx - 2 * dy;

		set_pixel(px0, py0, clr);
		while (--dy) {
			px0 += (d > 0) ? xstep : 0;
			d += (d > 0) ? dmaj : dmin;
			py0 += ystep;
			set_pixel(px0, py0, clr);
		}
	}
}

