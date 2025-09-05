#include <GL/glut.h>
#include <GL/glu.h>
#include <vector>
#include <cmath>
#include <iostream>

using namespace std; // Use std namespace to avoid std:: prefix

// Define a structure for RGB colors
struct Color {
    float r, g, b;
    Color() : r(0.0f), g(0.0f), b(0.0f) {} // Default constructor
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {}
};

// Define colors for the Rubik's Cube faces
const Color RED(1.0f, 0.0f, 0.0f);      // Front (z = +1)
const Color ORANGE(1.0f, 0.5f, 0.0f);   // Back (z = -1)
const Color WHITE(1.0f, 1.0f, 1.0f);    // Top (y = +1)
const Color YELLOW(1.0f, 1.0f, 0.0f);   // Bottom (y = -1)
const Color GREEN(0.0f, 1.0f, 0.0f);    // Left (x = -1)
const Color BLUE(0.0f, 0.0f, 1.0f);     // Right (x = +1)

struct Cubelet {
    float x, y, z;
    float rotX, rotY, rotZ; // Kept for compatibility, but not used
    Color colors[6]; // 0: front, 1: back, 2: top, 3: bottom, 4: left, 5: right
    Cubelet(float px, float py, float pz) : x(px), y(py), z(pz), rotX(0), rotY(0), rotZ(0) {
        // Initialize colors: outer faces get colors, inner faces get black
        colors[0] = (pz > 0.9f) ? RED : ((pz < -0.9f) ? ORANGE : Color(0, 0, 0));
        colors[1] = (pz < -0.9f) ? ORANGE : ((pz > 0.9f) ? RED : Color(0, 0, 0));
        colors[2] = (py > 0.9f) ? WHITE : ((py < -0.9f) ? YELLOW : Color(0, 0, 0));
        colors[3] = (py < -0.9f) ? YELLOW : ((py > 0.9f) ? WHITE : Color(0, 0, 0));
        colors[4] = (px < -0.9f) ? GREEN : ((px > 0.9f) ? BLUE : Color(0, 0, 0));
        colors[5] = (px > 0.9f) ? BLUE : ((px < -0.9f) ? GREEN : Color(0, 0, 0));
    }
};

vector<Cubelet> cubelets;
float globalRotX = 0.0f, globalRotY = 0.0f;
bool rotatingLayer = false;
float layerAngle = 0.0f;
char selectedAxis = 'z';
float selectedLayer = 0.0f;
int direction = 1; // 1 for clockwise, -1 for counterclockwise
bool spinning = true;

void initCube() {
    float size = 1.0f;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                if (x == 0 && y == 0 && z == 0) continue; // Skip the center
                cubelets.push_back(Cubelet(x * size, y * size, z * size));
            }
        }
    }
}

void drawCubelet(float x, float y, float z, float rotX, float rotY, float rotZ, const Color* colors) {
    float s = 0.45f;
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotX, 1, 0, 0);
    glRotatef(rotY, 0, 1, 0);
    glRotatef(rotZ, 0, 0, 1);

    glBegin(GL_QUADS);
    // Front (z = +s)
    glColor3f(colors[0].r, colors[0].g, colors[0].b);
    glVertex3f(-s, -s, s); glVertex3f(s, -s, s); glVertex3f(s, s, s); glVertex3f(-s, s, s);
    // Back (z = -s)
    glColor3f(colors[1].r, colors[1].g, colors[1].b);
    glVertex3f(-s, -s, -s); glVertex3f(s, -s, -s); glVertex3f(s, s, -s); glVertex3f(-s, s, -s);
    // Top (y = +s)
    glColor3f(colors[2].r, colors[2].g, colors[2].b);
    glVertex3f(-s, s, -s); glVertex3f(s, s, -s); glVertex3f(s, s, s); glVertex3f(-s, s, s);
    // Bottom (y = -s)
    glColor3f(colors[3].r, colors[3].g, colors[3].b);
    glVertex3f(-s, -s, -s); glVertex3f(s, -s, -s); glVertex3f(s, -s, s); glVertex3f(-s, -s, s);
    // Left (x = -s)
    glColor3f(colors[4].r, colors[4].g, colors[4].b);
    glVertex3f(-s, -s, -s); glVertex3f(-s, -s, s); glVertex3f(-s, s, s); glVertex3f(-s, s, -s);
    // Right (x = +s)
    glColor3f(colors[5].r, colors[5].g, colors[5].b);
    glVertex3f(s, -s, -s); glVertex3f(s, -s, s); glVertex3f(s, s, s); glVertex3f(s, s, -s);
    glEnd();
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5, 5, 5, 0, 0, 0, 0, 1, 0);

    glPushMatrix();
    glRotatef(globalRotX, 1, 0, 0);
    glRotatef(globalRotY, 0, 1, 0);

    for (auto& cube : cubelets) {
        if (rotatingLayer && (
            (selectedAxis == 'z' && abs(cube.z - selectedLayer) < 0.1f) ||
            (selectedAxis == 'x' && abs(cube.x - selectedLayer) < 0.1f) ||
            (selectedAxis == 'y' && abs(cube.y - selectedLayer) < 0.1f))) {
            glPushMatrix();
            if (selectedAxis == 'z') glRotatef(direction * layerAngle, 0, 0, 1);
            if (selectedAxis == 'x') glRotatef(direction * layerAngle, 1, 0, 0);
            if (selectedAxis == 'y') glRotatef(direction * layerAngle, 0, 1, 0);
            drawCubelet(cube.x, cube.y, cube.z, cube.rotX, cube.rotY, cube.rotZ, cube.colors);
            glPopMatrix();
        }
        else {
            drawCubelet(cube.x, cube.y, cube.z, cube.rotX, cube.rotY, cube.rotZ, cube.colors);
        }
    }
    glPopMatrix();

    glutSwapBuffers();
}

void rotateLayer() {
    vector<pair<int, int>> indices; // (i, j) for 3x3 face grid
    vector<size_t> cubeletIndices;  // Indices in cubelets vector
    vector<Cubelet> tempCubelets = cubelets; // Copy for reference

    // Step 1: Identify cubelets in the selected layer
    for (size_t idx = 0; idx < cubelets.size(); ++idx) {
        auto& cube = cubelets[idx];
        float tolerance = 0.1f;
        if (selectedAxis == 'z' && abs(cube.z - selectedLayer) < tolerance) {
            int i = static_cast<int>(cube.y + 1.5f); // y: -1,0,1 -> 0,1,2
            int j = static_cast<int>(cube.x + 1.5f); // x: -1,0,1 -> 0,1,2
            if (i >= 0 && i < 3 && j >= 0 && j < 3) {
                indices.emplace_back(i, j);
                cubeletIndices.push_back(idx);
            }
        }
        else if (selectedAxis == 'x' && abs(cube.x - selectedLayer) < tolerance) {
            int i = static_cast<int>(cube.y + 1.5f); // y: -1,0,1 -> 0,1,2
            int j = static_cast<int>(cube.z + 1.5f); // z: -1,0,1 -> 0,1,2
            if (i >= 0 && i < 3 && j >= 0 && j < 3) {
                indices.emplace_back(i, j);
                cubeletIndices.push_back(idx);
            }
        }
        else if (selectedAxis == 'y' && abs(cube.y - selectedLayer) < tolerance) {
            int i = static_cast<int>(cube.z + 1.5f); // z: -1,0,1 -> 0,1,2
            int j = static_cast<int>(cube.x + 1.5f); // x: -1,0,1 -> 0,1,2
            if (i >= 0 && i < 3 && j >= 0 && j < 3) {
                indices.emplace_back(i, j);
                cubeletIndices.push_back(idx);
            }
        }
    }

    // Step 2: Update cubelet positions and colors
    for (size_t k = 0; k < indices.size(); ++k) {
        int i = indices[k].first;
        int j = indices[k].second;
        size_t cubeIdx = cubeletIndices[k];

        // Determine new indices after rotation
        int newI, newJ;
        if (direction == 1) { // Clockwise
            newI = 2 - j;
            newJ = i;
        }
        else { // Counterclockwise
            newI = j;
            newJ = 2 - i;
        }

        // Find source cubelet for position (newI, newJ)
        for (size_t m = 0; m < indices.size(); ++m) {
            if (indices[m].first == newI && indices[m].second == newJ) {
                Cubelet& targetCube = cubelets[cubeIdx];
                const Cubelet& sourceCube = tempCubelets[cubeletIndices[m]];

                // Update position with rounding to prevent drift
                auto roundCoord = [](float coord) {
                    return round(coord * 1000.0f) / 1000.0f;
                    };

                if (selectedAxis == 'z') {
                    targetCube.x = roundCoord(direction == 1 ? -sourceCube.y : sourceCube.y);
                    targetCube.y = roundCoord(direction == 1 ? sourceCube.x : -sourceCube.x);
                    targetCube.z = roundCoord(sourceCube.z);
                }
                else if (selectedAxis == 'x') {
                    targetCube.x = roundCoord(sourceCube.x);
                    targetCube.y = roundCoord(direction == 1 ? -sourceCube.z : sourceCube.z);
                    targetCube.z = roundCoord(direction == 1 ? sourceCube.y : -sourceCube.y);
                }
                else if (selectedAxis == 'y') {
                    targetCube.x = roundCoord(direction == 1 ? sourceCube.z : -sourceCube.z);
                    targetCube.y = roundCoord(sourceCube.y);
                    targetCube.z = roundCoord(direction == 1 ? -sourceCube.x : sourceCube.x);
                }

                // Update colors
                Color oldColors[6];
                for (int n = 0; n < 6; ++n) oldColors[n] = sourceCube.colors[n];

                // Adjust direction for negative layers
                int effectiveDirection = direction;
                if ((selectedAxis == 'z' && selectedLayer < 0) ||
                    (selectedAxis == 'x' && selectedLayer < 0) ||
                    (selectedAxis == 'y' && selectedLayer < 0)) {
                    effectiveDirection = -direction; // Flip for negative layers
                }

                if (selectedAxis == 'z') {
                    if (effectiveDirection == 1) { // Clockwise (e.g., 'f' for front, 'B' for back)
                        // For front (z=1): top -> right, right -> bottom, bottom -> left, left -> top
                        targetCube.colors[0] = oldColors[0]; // Front = old Front (RED for z=1)
                        targetCube.colors[1] = oldColors[1]; // Back = old Back (ORANGE for z=-1)
                        targetCube.colors[2] = oldColors[5]; // Top = old Right
                        targetCube.colors[3] = oldColors[4]; // Bottom = old Left
                        targetCube.colors[4] = oldColors[2]; // Left = old Top
                        targetCube.colors[5] = oldColors[3]; // Right = old Bottom
                    }
                    else { // Counterclockwise (e.g., 'F' for front, 'b' for back)
                        // For front (z=1): top -> left, left -> bottom, bottom -> right, right -> top
                        targetCube.colors[0] = oldColors[0]; // Front = old Front
                        targetCube.colors[1] = oldColors[1]; // Back = old Back
                        targetCube.colors[2] = oldColors[4]; // Top = old Left
                        targetCube.colors[3] = oldColors[5]; // Bottom = old Right
                        targetCube.colors[4] = oldColors[3]; // Left = old Bottom
                        targetCube.colors[5] = oldColors[2]; // Right = old Top
                    }
                }
                else if (selectedAxis == 'x') {
                    if (effectiveDirection == 1) { // Clockwise
                        targetCube.colors[0] = oldColors[3]; // Front = old Bottom
                        targetCube.colors[1] = oldColors[2]; // Back = old Top
                        targetCube.colors[2] = oldColors[0]; // Top = old Front
                        targetCube.colors[3] = oldColors[1]; // Bottom = old Back
                        targetCube.colors[4] = oldColors[4]; // Left = old Left
                        targetCube.colors[5] = oldColors[5]; // Right = old Right
                    }
                    else { // Counterclockwise
                        targetCube.colors[0] = oldColors[2]; // Front = old Top
                        targetCube.colors[1] = oldColors[3]; // Back = old Bottom
                        targetCube.colors[2] = oldColors[1]; // Top = old Back
                        targetCube.colors[3] = oldColors[0]; // Bottom = old Front
                        targetCube.colors[4] = oldColors[4]; // Left = old Left
                        targetCube.colors[5] = oldColors[5]; // Right = old Right
                    }
                }
                else if (selectedAxis == 'y') {
                    if (effectiveDirection == 1) { // Clockwise
                        targetCube.colors[0] = oldColors[4]; // Front = old Left
                        targetCube.colors[1] = oldColors[5]; // Back = old Right
                        targetCube.colors[2] = oldColors[2]; // Top = old Top
                        targetCube.colors[3] = oldColors[3]; // Bottom = old Bottom
                        targetCube.colors[4] = oldColors[1]; // Left = old Back
                        targetCube.colors[5] = oldColors[0]; // Right = old Front
                    }
                    else { // Counterclockwise
                        targetCube.colors[0] = oldColors[5]; // Front = old Right
                        targetCube.colors[1] = oldColors[4]; // Back = old Left
                        targetCube.colors[2] = oldColors[2]; // Top = old Top
                        targetCube.colors[3] = oldColors[3]; // Bottom = old Bottom
                        targetCube.colors[4] = oldColors[0]; // Left = old Front
                        targetCube.colors[5] = oldColors[1]; // Right = old Back
                    }
                }

                break;
            }
        }
    }
}

void update(int value) {
    if (spinning) {
        globalRotX += 1.0f;
        globalRotY += 1.0f;
    }
    if (rotatingLayer) {
        layerAngle += 5.0f;
        if (layerAngle >= 90.0f) {
            layerAngle = 0.0f;
            rotatingLayer = false;
            rotateLayer();
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:    globalRotX -= 5.0f; break;
    case GLUT_KEY_DOWN:  globalRotX += 5.0f; break;
    case GLUT_KEY_LEFT:  globalRotY -= 5.0f; break;
    case GLUT_KEY_RIGHT: globalRotY += 5.0f; break;
    }
    glutPostRedisplay();
}

void keyboards(unsigned char key, int x, int y) {
    if (!rotatingLayer) {
        switch (key) {
            // Clockwise (lowercase)
        case 'u': rotatingLayer = true; selectedAxis = 'y'; selectedLayer = 1.0f;  direction = 1; break; // Top
        case 'd': rotatingLayer = true; selectedAxis = 'y'; selectedLayer = -1.0f; direction = 1; break; // Bottom
        case 'r': rotatingLayer = true; selectedAxis = 'x'; selectedLayer = 1.0f;  direction = 1; break; // Right
        case 'l': rotatingLayer = true; selectedAxis = 'x'; selectedLayer = -1.0f; direction = 1; break; // Left
        case 'f': rotatingLayer = true; selectedAxis = 'z'; selectedLayer = 1.0f;  direction = 1; break; // Front
        case 'b': rotatingLayer = true; selectedAxis = 'z'; selectedLayer = -1.0f; direction = 1; break; // Back
            // Counterclockwise (uppercase)
        case 'U': rotatingLayer = true; selectedAxis = 'y'; selectedLayer = 1.0f;  direction = -1; break; // Top
        case 'D': rotatingLayer = true; selectedAxis = 'y'; selectedLayer = -1.0f; direction = -1; break; // Bottom
        case 'R': rotatingLayer = true; selectedAxis = 'x'; selectedLayer = 1.0f;  direction = -1; break; // Right
        case 'L': rotatingLayer = true; selectedAxis = 'x'; selectedLayer = -1.0f; direction = -1; break; // Left
        case 'F': rotatingLayer = true; selectedAxis = 'z'; selectedLayer = 1.0f;  direction = -1; break; // Front
        case 'B': rotatingLayer = true; selectedAxis = 'z'; selectedLayer = -1.0f; direction = -1; break; // Back
        case 32:  spinning = !spinning; break; // Spacebar
        }
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);
    glViewport(0, 0, 800, 600);
    initCube();
}

int main(int argc, char** argv) {
    cout << "3D Rubik's Cube Controls:\n";
    cout << "  Up/Down Arrows - Tilt cube vertically\n";
    cout << "  Left/Right Arrows - Spin cube horizontally\n";
    cout << "  'u'/'U' - Rotate Top layer (clockwise/counterclockwise)\n";
    cout << "  'd'/'D' - Rotate Bottom layer (clockwise/counterclockwise)\n";
    cout << "  'r'/'R' - Rotate Right layer (clockwise/counterclockwise)\n";
    cout << "  'l'/'L' - Rotate Left layer (clockwise/counterclockwise)\n";
    cout << "  'f'/'F' - Rotate Front layer (clockwise/counterclockwise)\n";
    cout << "  'b'/'B' - Rotate Back layer (clockwise/counterclockwise)\n";
    cout << "  Spacebar - Pause/Unpause auto-spinning\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Rubik's Cube");

    init();
    glutDisplayFunc(display);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyboards);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}