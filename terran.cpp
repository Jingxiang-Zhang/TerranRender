#include "terran.h"
#include <algorithm>

char shaderBasePath[1024] = "openGL";

// declear variable
int mousePos[2];
terran::mouse_button_state mouseButtonState = { 0, 0, 0 };
terran::control_state controlState;
// control the mouse move speed
float mouse_move_factor_x, mouse_move_factor_y;

// specify whether bg image exist or not
bool bgImageExist = false;

// view state
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

// windows initial parameter
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "Ray Tracer";

// frame save path
const char* frame_save_path = nullptr;
// frame save count
unsigned int frame_save_count = 0;
// windows FPS
unsigned int FPS = 60;

// OpenGL relate variables
// vertex array object
// vertexArray_mode3 is the background color display mode
GLuint vertexArray_mode1, vertexArray_mode2, vertexArray_mode3;
// vertex information
GLuint vertexBuffer, colorBuffer, bgImageBuffer;
// smoothe mode vertex buffer
GLuint vertexBuffer_right, vertexBuffer_bottom, vertexBuffer_left, vertexBuffer_top;
// transformation matrix
// shader program
OpenGLMatrix matrix;
// shader program
Shader* pipelineProgram;
// display mode
terran::displayMode mode = terran::displayMode::triangles;
// number of indices that need to be plot
unsigned int nVertices;
unsigned int nTriIndices;
unsigned int nLineIndices;
// indices buffer for ploting lines
GLuint triModeIndicesBuffer;
// indices buffer for ploting triangles
GLuint lineModeIndicesBuffer;

// declear function
void saveScreenshot(const char* filename);

// Glut function
// perform animation inside idleFunc
void idleFunc();
// callback for resizing the window
void reshapeFunc(int w, int h);
// callback for idle mouse movement
void mouseMotionDragFunc(int x, int y);
// callback for mouse drags
void mouseMotionFunc(int x, int y);
// callback for mouse button changes
void mouseButtonFunc(int button, int state, int x, int y);
// callback for pressing the keys on the keyboard
void keyboardFunc(unsigned char key, int x, int y);
// control output FPS
void timerFunc(int t);
// tells glut to use a particular display function to redraw 
void displayFunc();
// switch mode to smoothed triangle mode or other mode
void switch_mode(terran::displayMode mode);

// OpenGL function
// generate gl buffer
void generate_gl_buffer(GLuint& vertexBuffer, unsigned int size, float* vertices_mesh);
// generate gl element buffer
void generate_gl_elem_buffer(GLuint& vertexBuffer, unsigned int size,
    unsigned int* vertices_mesh);
// set vertices layout pattern, size = 3 for vertices, size = 4 for colors
void gl_layout(GLuint& vertexBuffer, const char* variable_name, unsigned int step = 3);

// generate grid mesh data
// generate grid mesh for vertices location and it corresponding color
void generate_grid_mesh(IN LoadJPG* heightmapImage, IN float boarder,
    OUT float* grid_mesh, OUT float* color_mesh);
// generate background image if it's given
void generate_bgImage(IN LoadJPG* bgImage, IN unsigned int width,
    IN unsigned int height, OUT float* color_mesh);
void generate_smoothed_other_4_vertices(IN unsigned int width,
    IN unsigned int height, IN float* grid_mesh,
    OUT float* grid_mesh_right, OUT float* grid_mesh_left,
    OUT float* grid_mesh_top, OUT float* grid_mesh_bottom);
void generate_line_indices(unsigned int width, unsigned int height,
    unsigned int* lineIndices);
void generate_triangle_indices(unsigned int width, unsigned int height,
    unsigned int* TriIndices);



void saveScreenshot(const char* filename) {
    unsigned char* screenshotData = new unsigned char[windowWidth * windowHeight * 3];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);
    LoadJPG screenshotImg(windowWidth, windowHeight, 3, screenshotData);

    if (screenshotImg.save(filename) == LoadJPG::OK)
        std::cout << "File " << filename << " saved successfully." << std::endl;
    else std::cout << "Failed to save file " << filename << '.' << std::endl;

    delete[] screenshotData;
}

void displayFunc()
{
    float time = glutGet(GLUT_ELAPSED_TIME);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.LoadIdentity();
    matrix.LookAt(0, -4, 3, 0, 0, 0, 0, 1, 0);
    matrix.Scale(landScale[0], landScale[1], landScale[2]);
    matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
    matrix.Rotate(landRotate[0] / 5, 1, 0, 0);
    matrix.Rotate(landRotate[1] / 5, 0, 1, 0);
    matrix.Rotate(landRotate[2] / 5, 0, 0, 1);
    // matrix.LookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

    float m[16];
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.GetMatrix(m);
    float p[16];
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    // matrix.Perspective(70.0f, 1920.0f / 1080.0f, 0.1f, 100.0f);
    matrix.GetMatrix(p);
    // bind shader
    pipelineProgram->Bind();
    // set variable
    pipelineProgram->SetModelViewMatrix(m);
    pipelineProgram->SetProjectionMatrix(p);
    // set display mode
    switch (mode) {
    case::terran::displayMode::points:
        glBindVertexArray(vertexArray_mode1);
        glDrawArrays(GL_POINTS, 0, nVertices);
        break;
    case::terran::displayMode::lines:
        glBindVertexArray(vertexArray_mode1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lineModeIndicesBuffer);
        glDrawElements(GL_LINES, nLineIndices, GL_UNSIGNED_INT, nullptr);
        break;
    case::terran::displayMode::triangles:
        glBindVertexArray(vertexArray_mode1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triModeIndicesBuffer);
        glDrawElements(GL_TRIANGLES, nTriIndices, GL_UNSIGNED_INT, nullptr);
        break;
    case::terran::displayMode::smoothTriangles:
        glBindVertexArray(vertexArray_mode2);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triModeIndicesBuffer);
        glDrawElements(GL_TRIANGLES, nTriIndices, GL_UNSIGNED_INT, nullptr);
        break;
    case::terran::displayMode::imageDisplay:
        glBindVertexArray(vertexArray_mode3);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triModeIndicesBuffer);
        glDrawElements(GL_TRIANGLES, nTriIndices, GL_UNSIGNED_INT, nullptr);
    }
    // __debugbreak();
    // errorDetection();
    glutSwapBuffers();
}

void idleFunc()
{
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
    glViewport(0, 0, w, h);
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.LoadIdentity();
    matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 1000.0f);
}

void mouseMotionDragFunc(int x, int y)
{
    // mouse has moved and one of the mouse buttons is pressed (dragging)
    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
    switch (controlState) {
    case terran::TRANSLATE:
        // translate the landscape
        if (mouseButtonState.leftMouseButton) {
            // control x,y translation via the left mouse button
            landTranslate[0] += mousePosDelta[0] * 0.01f;
            landTranslate[1] -= mousePosDelta[1] * 0.01f;
        }
        if (mouseButtonState.middleMouseButton) {
            // control z translation via the middle mouse button
            landTranslate[2] -= mousePosDelta[1] * 0.01f;
        }
        break;
    case terran::ROTATE:
        // rotate the landscape
        if (mouseButtonState.leftMouseButton) {
            // control x,y rotation via the left mouse button
            landRotate[0] += mousePosDelta[1];
            landRotate[1] += mousePosDelta[0];
        }
        if (mouseButtonState.middleMouseButton) {
            // control z rotation via the middle mouse button
            landRotate[2] += mousePosDelta[1];
        }
        break;
    case terran::SCALE:
        // scale the landscape
        if (mouseButtonState.leftMouseButton) {
            // control x,y scaling via the left mouse button
            landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
            landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
        }
        if (mouseButtonState.middleMouseButton) {
            // control z scaling via the middle mouse button
            landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
        }
        break;
    }
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
    // keep track of the mouse button state, in leftMouseButton, middleMouseButton,
    // rightMouseButton variables
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        mouseButtonState.leftMouseButton = (state == GLUT_DOWN);
        break;
    case GLUT_MIDDLE_BUTTON:
        mouseButtonState.middleMouseButton = (state == GLUT_DOWN);
        break;
    case GLUT_RIGHT_BUTTON:
        mouseButtonState.rightMouseButton = (state == GLUT_DOWN);
        break;
    }

    // keep track of whether CTRL and SHIFT keys are pressed
    switch (glutGetModifiers())
    {
    case GLUT_ACTIVE_CTRL:
        controlState = terran::TRANSLATE;
        break;
    case GLUT_ACTIVE_SHIFT:
        controlState = terran::SCALE;
        break;
        // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
        controlState = terran::ROTATE;
        break;
    }
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27: // ESC key
        exit(0); // exit the program
        break;
    case '1':
        if (mode != terran::displayMode::points) {
            switch_mode(terran::displayMode::points);
            std::cout << "mode switch to points display." << std::endl;
            mode = terran::displayMode::points;
        }
        break;
    case '2':
        if (mode != terran::displayMode::lines) {
            switch_mode(terran::displayMode::lines);
            std::cout << "mode switch to lines display." << std::endl;
            mode = terran::displayMode::lines;
        }
        break;
    case '3':
        if (mode != terran::displayMode::triangles) {
            switch_mode(terran::displayMode::triangles);
            std::cout << "mode switch to triangles display." << std::endl;
            mode = terran::displayMode::triangles;
        }
        break;
    case '4':
        if (mode != terran::displayMode::smoothTriangles) {
            switch_mode(terran::displayMode::smoothTriangles);
            std::cout << "mode switch to smoothTriangles display." << std::endl;
            mode = terran::displayMode::smoothTriangles;
        }
        break;
    case '5':
        if (bgImageExist) {
            switch_mode(terran::displayMode::imageDisplay);
            mode = terran::displayMode::imageDisplay;
        }
        else {
            std::cout << "background image not exist, can't change background status." << std::endl;
        }
        break;
    case 'x':
        // take a screenshot
        saveScreenshot("screenshot.jpg");
        break;
    }
}

void timerFunc(int t)
{
    glutPostRedisplay();
    // save frame
    if (frame_save_path != nullptr) {
        char path[128];
        strcpy(path, frame_save_path);
        strcat(path, "/");
        char number[4] = { '0' ,'0', '0', 0 };
        int loc = 2;
        unsigned int frame_save_count_ = frame_save_count;
        while (loc >= 0) {
            number[loc] = frame_save_count_ % 10 + '0';
            frame_save_count_ = frame_save_count_ / 10;
            loc--;
        }
        strcat(path, number);
        strcat(path, ".jpg");
        frame_save_count++;
        saveScreenshot(path);
    }
    glutTimerFunc(1000.0f / (float)FPS, timerFunc, 0);
}

void switch_mode(terran::displayMode mode)
{
    GLuint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");
    if (mode == terran::displayMode::smoothTriangles) {
        glUniform1i(loc, 1);
    }
    else {
        glUniform1i(loc, 0);
    }
}

void generate_gl_buffer(GLuint& glBuffer, unsigned int size, float* vertices_mesh)
{
    glGenBuffers(1, &glBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, glBuffer);
    glBufferData(GL_ARRAY_BUFFER, size, vertices_mesh, GL_STATIC_DRAW);
}

void gl_layout(GLuint& vertexBuffer, const char* variable_name, unsigned int step) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), variable_name);
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, step, GL_FLOAT, GL_FALSE, 0, (const void*)0);
}

void generate_gl_elem_buffer(GLuint& indicesBuffer, unsigned int size,
    unsigned int* indices_mesh) {
    glGenBuffers(1, &indicesBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices_mesh
        , GL_STATIC_DRAW);
}

void generate_grid_mesh(IN LoadJPG* heightmapImage, IN float boarder,
    OUT float* grid_mesh, OUT float* color_mesh) {
    unsigned int width = heightmapImage->getWidth();
    unsigned int height = heightmapImage->getHeight();
    unsigned int bits = heightmapImage->getBytesPerPixel();
    unsigned char* pixels = heightmapImage->getPixels();

    // generate grid mesh vertices
    float max_color = INT_MIN, min_color = INT_MAX;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 3;
            grid_mesh[pos++] = i * boarder / (float)height - boarder / 2;
            grid_mesh[pos++] = j * boarder / (float)width - boarder / 2;
            grid_mesh[pos] = (float)pixels[i * width + j] / 256.0f / 2.0f;
            max_color = std::max(max_color, (float)pixels[i * width + j]);
            min_color = std::min(min_color, (float)pixels[i * width + j]);
        }
    }

    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 4;
            // mapping the pixel height to the color
            /*
            float gray = ((float)pixels[i * width + j] - min_color) /
                (max_color - min_color) * 255.0f;
            gray = gray / 255.0f / 1.25f + 0.2f;
            */
            float gray = (float)pixels[i * width + j] / 255.0f;
            color_mesh[pos++] = gray;
            color_mesh[pos++] = gray;
            color_mesh[pos++] = gray;
            color_mesh[pos++] = 1.0f;
        }
    }

}

void generate_bgImage(IN LoadJPG* bgImage, IN unsigned int width,
    IN unsigned int height, OUT float* color_mesh) {
    unsigned int width_bg = bgImage->getWidth();
    unsigned int height_bg = bgImage->getHeight();
    unsigned int bits = bgImage->getBytesPerPixel();
    unsigned char* pixels = bgImage->getPixels();
    float ratio_width = (float)width_bg / (float)width;
    float ratio_height = (float)height_bg / (float)height;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 4;
            unsigned int i_bg = (unsigned int)(ratio_height * i);
            unsigned int j_bg = (unsigned int)(ratio_width * j);
            color_mesh[pos++] = (float)pixels[(i_bg * width_bg + j_bg) * 3] / 255.0f;
            color_mesh[pos++] = (float)pixels[(i_bg * width_bg + j_bg) * 3 + 1] / 255.0f;
            color_mesh[pos++] = (float)pixels[(i_bg * width_bg + j_bg) * 3 + 2] / 255.0f;
            color_mesh[pos++] = 1.0f;
        }
    }
}

void generate_smoothed_other_4_vertices(IN unsigned int width,
    IN unsigned int height, IN float* grid_mesh,
    OUT float* grid_mesh_right, OUT float* grid_mesh_left,
    OUT float* grid_mesh_top, OUT float* grid_mesh_bottom)
{
    // generate grid_mesh_right
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 3;
            if (j != width - 1) {
                grid_mesh_right[pos] = grid_mesh[pos + 3]; pos++;
                grid_mesh_right[pos] = grid_mesh[pos + 3]; pos++;
                grid_mesh_right[pos] = grid_mesh[pos + 3];
            }
            else {
                grid_mesh_right[pos] = grid_mesh[pos]; pos++;
                grid_mesh_right[pos] = grid_mesh[pos]; pos++;
                grid_mesh_right[pos] = grid_mesh[pos];
            }
        }
    }
    // generate grid_mesh_bottom
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 3;
            if (i != height - 1) {
                grid_mesh_bottom[pos] = grid_mesh[pos + width * 3]; pos++;
                grid_mesh_bottom[pos] = grid_mesh[pos + width * 3]; pos++;
                grid_mesh_bottom[pos] = grid_mesh[pos + width * 3];
            }
            else {
                grid_mesh_bottom[pos] = grid_mesh[pos]; pos++;
                grid_mesh_bottom[pos] = grid_mesh[pos]; pos++;
                grid_mesh_bottom[pos] = grid_mesh[pos];
            }
        }
    }
    // generate grid_mesh_left
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 3;
            if (j != 0) {
                grid_mesh_left[pos] = grid_mesh[pos - 3]; pos++;
                grid_mesh_left[pos] = grid_mesh[pos - 3]; pos++;
                grid_mesh_left[pos] = grid_mesh[pos - 3];
            }
            else {
                grid_mesh_left[pos] = grid_mesh[pos]; pos++;
                grid_mesh_left[pos] = grid_mesh[pos]; pos++;
                grid_mesh_left[pos] = grid_mesh[pos];
            }
        }
    }
    // generate grid_mesh_top
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = (i * width + j) * 3;
            if (i != 0) {
                grid_mesh_top[pos] = grid_mesh[pos - width * 3]; pos++;
                grid_mesh_top[pos] = grid_mesh[pos - width * 3]; pos++;
                grid_mesh_top[pos] = grid_mesh[pos - width * 3];
            }
            else {
                grid_mesh_top[pos] = grid_mesh[pos]; pos++;
                grid_mesh_top[pos] = grid_mesh[pos]; pos++;
                grid_mesh_top[pos] = grid_mesh[pos];
            }
        }
    }

}

void generate_line_indices(unsigned int width, unsigned int height, unsigned int* lineIndices)
{
    unsigned int currentIndex = 0;
    // line of all rows
    for (unsigned int i = 0; i < height - 1; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned int pos = i * width + j;
            unsigned int linkto = pos + width;
            lineIndices[currentIndex++] = pos;
            lineIndices[currentIndex++] = linkto;
        }
    }
    // line of all columns
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width - 1; j++) {
            unsigned int pos = i * width + j;
            unsigned int linkto = pos + 1;
            lineIndices[currentIndex++] = pos;
            lineIndices[currentIndex++] = linkto;
        }
    }
}

void generate_triangle_indices(unsigned int width, unsigned int height,
    unsigned int* TriIndices)
{
    unsigned int currentIndex = 0;
    for (unsigned int i = 0; i < height - 1; i++) {
        for (unsigned int j = 0; j < width - 1; j++) {
            unsigned int pos_left = i * width + j;
            unsigned int pos_right = pos_left + 1;
            unsigned int pos_bottom_left = pos_left + width;
            unsigned int pos_bottom_right = pos_bottom_left + 1;
            // construct the first triangle
            TriIndices[currentIndex++] = pos_left;
            TriIndices[currentIndex++] = pos_right;
            TriIndices[currentIndex++] = pos_bottom_left;
            // construct the second triangle
            TriIndices[currentIndex++] = pos_right;
            TriIndices[currentIndex++] = pos_bottom_right;
            TriIndices[currentIndex++] = pos_bottom_left;
        }
    }
}

void terran::initScene(LoadJPG* heightmapImage, LoadJPG* bgImage)
{
    // read color
    unsigned int width = heightmapImage->getWidth();
    unsigned int height = heightmapImage->getHeight();
    unsigned int bits = heightmapImage->getBytesPerPixel();
    unsigned char* pixels = heightmapImage->getPixels();
    nVertices = width * height;

    float* vertices_mesh = new float[width * height * 3];
    float* color = new float[width * height * 4];
    // generate color mesh and vertices mesh
    generate_grid_mesh(heightmapImage, 4.0f, vertices_mesh, color);
    float* bgColor = nullptr;
    if (bgImage != nullptr) {
        bgImageExist = true;
        bgColor = new float[width * height * 4];
        generate_bgImage(bgImage, width, height, bgColor);
    }

    // smoothed mode variable
    float* vertices_mesh_right = new float[width * height * 3];
    float* vertices_mesh_bottom = new float[width * height * 3];
    float* vertices_mesh_left = new float[width * height * 3];
    float* vertices_mesh_top = new float[width * height * 3];
    generate_smoothed_other_4_vertices(width, height, vertices_mesh,
        vertices_mesh_right, vertices_mesh_left, vertices_mesh_top, vertices_mesh_bottom);

    // generate indices for draw lines
    nLineIndices = (width * (height - 1) + (width - 1) * height) * 2;
    unsigned int* lineIndices = new unsigned int[nLineIndices];
    generate_line_indices(width, height, lineIndices);

    // generate indices for draw triangles
    nTriIndices = (width - 1) * (height - 1) * 6;
    unsigned int* TriIndices = new unsigned int[nTriIndices];
    generate_triangle_indices(width, height, TriIndices);

    {
        // generate vertex buffer
        unsigned int vertices_size = sizeof(float) * width * height * 3;
        unsigned int colors_size = sizeof(float) * width * height * 4;
        // generate vertex buffer
        generate_gl_buffer(vertexBuffer, vertices_size, vertices_mesh);
        // generate color buffer
        generate_gl_buffer(colorBuffer, colors_size, color);
        if (bgImage != nullptr) {
            generate_gl_buffer(bgImageBuffer, colors_size, bgColor);
        }
        // generate smoothed vertex buffer
        generate_gl_buffer(vertexBuffer_right, vertices_size, vertices_mesh_right);
        generate_gl_buffer(vertexBuffer_bottom, vertices_size, vertices_mesh_bottom);
        generate_gl_buffer(vertexBuffer_left, vertices_size, vertices_mesh_left);
        generate_gl_buffer(vertexBuffer_top, vertices_size, vertices_mesh_top);
        // generate line indices buffer
        generate_gl_elem_buffer(lineModeIndicesBuffer, sizeof(float) * nLineIndices, lineIndices);
        // generate triangle indices buffer
        generate_gl_elem_buffer(triModeIndicesBuffer, sizeof(float) * nTriIndices, TriIndices);
    }
    { // create shader program for mode 1
        
        pipelineProgram = new Shader(shaderBasePath);
        // generate VAO of mode 1
        glGenVertexArrays(1, &vertexArray_mode1);
        glBindVertexArray(vertexArray_mode1);
        // bind shader vertices variable layout of VAO
        gl_layout(vertexBuffer, "position", 3);
        // bind shader color variable layout of VAO
        gl_layout(colorBuffer, "color", 4);
    }
    { // create shader program for mode 2
        glGenVertexArrays(1, &vertexArray_mode2);
        glBindVertexArray(vertexArray_mode2);
        // bind shader vertices variable layout of VAO
        gl_layout(vertexBuffer, "position", 3);
        gl_layout(vertexBuffer_right, "position_right", 3);
        gl_layout(vertexBuffer_bottom, "position_bottom", 3);
        gl_layout(vertexBuffer_left, "position_left", 3);
        gl_layout(vertexBuffer_top, "position_top", 3);
        // bind shader color variable layout of VAO
        gl_layout(colorBuffer, "color", 4);
    }
    { // create shader program for mode 3
        glGenVertexArrays(1, &vertexArray_mode3);
        glBindVertexArray(vertexArray_mode3);
        // bind shader vertices variable layout of VAO
        gl_layout(vertexBuffer, "position", 3);
        if (bgImage != nullptr) {
            gl_layout(bgImageBuffer, "color", 4);
        }
        else {
            gl_layout(colorBuffer, "color", 4);
        }
    }
    // enable depth test
    glEnable(GL_DEPTH_TEST);
    // initial shader mode to mode 1
    GLuint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");
    pipelineProgram->Bind();
    glUniform1i(loc, 0);
    std::cout << "GL error: " << glGetError() << std::endl;
    // release memory
    delete[] vertices_mesh;
    delete[] color;
    delete[] lineIndices;
    delete[] vertices_mesh_right;
    delete[] vertices_mesh_bottom;
    delete[] vertices_mesh_left;
    delete[] vertices_mesh_top;
    if (bgImage != nullptr) {
        delete[] bgColor;
    }
}

void terran::render(LoadJPG* heightmapImage, LoadJPG* bgImage,
    const char* frame_save_path_, unsigned int FPS_)
{
    frame_save_path = frame_save_path_;
    FPS = FPS_;
    std::cout << "Initializing GLUT..." << std::endl;
    int argc = 1;
    glutInit(&argc, nullptr);
    std::cout << "Initializing OpenGL..." << std::endl;
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(windowTitle);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // tells glut to use a particular display function to redraw 
    glutDisplayFunc(displayFunc);
    // perform animation inside idleFunc
    // glutIdleFunc(idleFunc);
    // callback for mouse drags
    glutMotionFunc(mouseMotionDragFunc);
    // callback for idle mouse movement
    glutPassiveMotionFunc(mouseMotionFunc);
    // callback for mouse button changes
    glutMouseFunc(mouseButtonFunc);
    // callback for resizing the window
    glutReshapeFunc(reshapeFunc);
    // callback for pressing the keys on the keyboard
    glutKeyboardFunc(keyboardFunc);
    // control output FPS
    glutTimerFunc(1000.0f / (float)FPS_, timerFunc, 0);

    // init glew
    GLint result = glewInit();
    if (result != GLEW_OK) {
        std::cout << "error: " << glewGetErrorString(result) << std::endl;
        exit(EXIT_FAILURE);
    }
    // do initialization
    initScene(heightmapImage, bgImage);
    // sink forever into the glut loop
    glutMainLoop();
}

