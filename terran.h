#ifndef _TERRAN_H_
#define _TERRAN_H_
#include "openGL/shader.h"
#include "openGL/openGLMatrix.h"
#include "jpg/loadJPG.h"
#include <GL/glew.h>
#include <iostream>
#include <cstring>
#include <GL/glut.h>

#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#ifndef OUT
#define OUT
#endif
#ifndef IN
#define IN
#endif

namespace terran {
	// the mourse button state
	struct mouse_button_state {
		int leftMouseButton;	 // 1 if pressed, 0 if not 
		int middleMouseButton;	 // 1 if pressed, 0 if not
		int rightMouseButton;	 // 1 if pressed, 0 if not
	};
	// graph transformation state
	typedef enum { 
		ROTATE, 
		TRANSLATE, 
		SCALE 
	} control_state;
	// display mode
	enum class displayMode {
		points, // display by points
		lines, // display by triangles
		triangles, // display by lines
		smoothTriangles, // display by smooth triangles
		imageDisplay
	};

	// do initialization
	void initScene(LoadJPG* heightmapImage, LoadJPG* bgImage);

	void render(LoadJPG* heightmapImage, LoadJPG* bgImage = nullptr,
		const char* frame_save_path = nullptr, unsigned int FPS = 60);
}

#endif