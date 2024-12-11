#include <glut.h>
#include <vector>
#include <cstdlib>  // For rand()

//Bagian 1
// Camera & Platform Settings
float fx = 0.0, fy = 10.0, fz = -14.0;
float tx = 0, ty = 0, tz = 0;
float yp = 0;

float ballScaleFactor = 1.0;
float targetBallScaleFactor = 1.0;
float ballScaleInterpolationSpeed = 0.1;

// Global variables for smooth paddle width change
float targetPaddleWidth = 2.0;
float currentPaddleWidth = 2.0;
float interpolationSpeed = 0.1; // Speed of width change

// Linear interpolation function for smooth transitions
float lerp(float start, float end, float t) {
    return start + t * (end - start);
}
// Paddle settings
float paddleX = 0.0;
float paddleWidth = 2.0;
float paddleHeight = 0.5;
float paddleDepth = 0.25;
float paddleZ = -9.5;
float platformSize = 10.0;
bool moveLeft = false;
bool moveRight = false;
float paddleSpeed = 0.4; // Initial paddle speed
float maxPaddleSpeed = 1.0; // Maximum paddle speed

// Rotation angles for the platform
float rotationX = 0.0;
float rotationY = 0.0;

// Ball settings
float ballX = 0.0, ballY = 0.5, ballZ = -8.0; // Starting position
float ballRadius = 0.2;
float ballSpeedX = 0.05, ballSpeedY = 0.1, ballSpeedZ = 0.05;

// Block structure
struct Block {
    float x, y, z;
    bool active;  // Is the block still present?
};

std::vector<Block> blocks;  // Stores blocks

// Power-Up Structure
struct PowerUp {
    float x, y, z;
    bool active;
    bool isIncrease;  // true for increase, false for decrease
};

float powerUpSpeed = 0.05;  // Speed at which power-ups fall

// Vector to store active power-ups
std::vector<PowerUp> powerUps;

bool isGameOver = false;  // Game starts in running state

// Function prototypes
void initLighting();
void cameraLook();
void display();
//Bagian 2
void draw();
void drawBlocks();
void drawPaddle();
void drawBall();
void reshape(int w, int h);
void timer(int value);
void handleSpecialKeys(int key, int x, int y);
void handleSpecialUpKeys(int key, int x, int y);
void handleKeyboardKeys(unsigned char key, int x, int y);
//Bagian 3
void checkCollisions();
void drawPowerUps();
void checkPowerUpCollision();
void spawnPowerUp(float x, float z);  // New function
void restartGame();

//Bagian 3
int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(300, 300);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("3D Platform with Ball");
    glClearColor(0.32f, 0.38f, 0.43f, 0.0);

    glEnable(GL_DEPTH_TEST); // Enable depth testing

    initLighting();

    // Initialize blocks
    float blockSize = 1.0;
    int numRows = 10, numCols = 5;
    float xStart = -platformSize + blockSize / 2;
    float zStart = 1.0;

    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            Block block;
            block.x = xStart + i * blockSize * 2;
            block.y = 0.5;  // Height above platform
            block.z = zStart + j * blockSize * 2;
            block.active = true;
            blocks.push_back(block);
        }
    }

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(handleSpecialKeys);
    glutSpecialUpFunc(handleSpecialUpKeys);
    glutKeyboardFunc(handleKeyboardKeys);
    glutTimerFunc(16, timer, 0);  // Approximately 60 FPS
    glutMainLoop();

    return 0;
}

void cameraLook() {
    gluLookAt(fx, fy, fz, tx, ty, tz, 0, 1, 0);
}

void applyMaterial(GLfloat* ambient, GLfloat* diffuse, GLfloat* specular, GLfloat shininess) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

void draw() {
    float depth = -0.25; // Depth of the platform along negative Y-axis
    float size = 11.0;    // Adjust size to increase platform dimensions

    // Top surface of the platform
    glColor3ub(200, 200, 200);
    glBegin(GL_QUADS);
    glVertex3f(-size, 0, -size);
    glVertex3f(size, 0, -size);
    glVertex3f(size, 0, size);
    glVertex3f(-size, 0, size);
    glEnd();

    // Sides of the platform
    glColor3ub(150, 150, 150);
    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-size, 0, size);
    glVertex3f(size, 0, size);
    glVertex3f(size, depth, size);
    glVertex3f(-size, depth, size);

    // Back face
    glVertex3f(-size, 0, -size);
    glVertex3f(size, 0, -size);
    glVertex3f(size, depth, -size);
    glVertex3f(-size, depth, -size);

    // Left face
    glVertex3f(-size, 0, -size);
    glVertex3f(-size, 0, size);
    glVertex3f(-size, depth, size);
    glVertex3f(-size, depth, -size);

    // Right face
    glVertex3f(size, 0, -size);
    glVertex3f(size, 0, size);
    glVertex3f(size, depth, size);
    glVertex3f(size, depth, -size);
    glEnd();

    // Bottom surface of the platform
    glColor3ub(100, 100, 100);
    glBegin(GL_QUADS);
    glVertex3f(-size, depth, -size);
    glVertex3f(size, depth, -size);
    glVertex3f(size, depth, size);
    glVertex3f(-size, depth, size);
    glEnd();
}

void drawBlocks() {
    float blockSize = 1.0;

    GLfloat ambient[] = { 0.0f, 0.0f, 0.5f, 1.0f };
    GLfloat diffuse[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    GLfloat specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat shininess = 32.0f;

    for (const Block& block : blocks) {
        if (!block.active) continue;

        glPushMatrix();
        applyMaterial(ambient, diffuse, specular, shininess);
        glTranslatef(block.x, block.y, block.z);
        glutSolidCube(blockSize);
        glPopMatrix();
    }
}

void drawPaddle() {
    GLfloat ambient[] = { 0.5f, 0.0f, 0.0f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    GLfloat specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat shininess = 16.0f;

    glPushMatrix();
    applyMaterial(ambient, diffuse, specular, shininess);
    glBegin(GL_QUADS);

    // Top face
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);

    // Bottom face
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);

    // Front face
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);

    // Back face
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);

    // Left face
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX - paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);

    // Right face
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ - paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, 0.0, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ + paddleDepth / 2);
    glVertex3f(paddleX + paddleWidth / 2, paddleHeight, paddleZ - paddleDepth / 2);

    glEnd();
    glPopMatrix();
}

void drawBall() {
    GLfloat ambient[] = { 0.5f, 0.5f, 0.0f, 1.0f };
    GLfloat diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    GLfloat specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat shininess = 64.0f;

    glPushMatrix();
    applyMaterial(ambient, diffuse, specular, shininess);
    glTranslatef(ballX, ballY, ballZ);
    glScalef(ballScaleFactor, ballScaleFactor, ballScaleFactor);
    glutSolidSphere(ballRadius, 20, 20);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    cameraLook();

    glRotatef(rotationX, 1.0, 0.0, 0.0);
    glRotatef(rotationY, 0.0, 1.0, 0.0);

    draw();

    if (isGameOver) {
        glDisable(GL_LIGHTING); // Disable lighting to render text properly
        glColor3f(1.0, 0.8, 0.0);  // Gold color for the text

        void* font = GLUT_BITMAP_TIMES_ROMAN_24; // Larger font for better visibility
        const char* message = "Game Over! Press Space to Restart.";

        // Calculate the position to center the text
        float textWidth = 0.0f;
        for (const char* c = message; *c != '\0'; ++c) {
            textWidth += glutBitmapWidth(font, *c);
        }
        textWidth *= 0.001f; // Scale the text width for the scene
        float xPos = -textWidth / 2.0f;

        glRasterPos3f(xPos, 0.0f, -5.0f); // Center the text
        for (const char* c = message; *c != '\0'; ++c) {
            glutBitmapCharacter(font, *c);
        }
        glEnable(GL_LIGHTING); // Re-enable lighting after rendering text
    }
    else {
        drawBlocks();
        drawPaddle();
        drawBall();
        drawPowerUps();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75, (GLfloat)w / h, 1, 300);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    if (!isGameOver) {
        // Game update logic
        ballX += ballSpeedX;
        ballZ += ballSpeedZ;

        float step = paddleSpeed;

        // Increase speed if a key is held
        if (moveLeft) {
            if (paddleX + paddleWidth / 2 < platformSize) {
                paddleX += step;
            }
        }
        if (moveRight) {
            if (paddleX - paddleWidth / 2 > -platformSize) {
                paddleX -= step;
            }
        }

        // Update ball position (only X and Z)
        ballX += ballSpeedX;
        ballZ += ballSpeedZ;

        // Move power-ups downward along the Z-axis
        for (PowerUp& powerUp : powerUps) {
            if (powerUp.active) {
                powerUp.z -= powerUpSpeed;  // Move power-up toward the paddle

                // Deactivate if it goes beyond the paddle
                if (powerUp.z < -platformSize) {
                    powerUp.active = false;
                }
            }
        }

        checkCollisions();
        checkPowerUpCollision();
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void handleSpecialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        moveLeft = true;  // Set flag for left movement
        break;
    case GLUT_KEY_RIGHT:
        moveRight = true;  // Set flag for right movement
        break;
    }
}

void handleSpecialUpKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        moveLeft = false;  // Reset flag for left movement
        break;
    case GLUT_KEY_RIGHT:
        moveRight = false;  // Reset flag for right movement
        break;
    }
}

void handleKeyboardKeys(unsigned char key, int x, int y) {
    float rotationStep = 5.0; // Rotation step size

    switch (key) {
    case 'a': // Rotate left around Y-axis
        rotationY -= rotationStep;
        break;
    case 'd': // Rotate right around Y-axis
        rotationY += rotationStep;
        break;
    case 'w': // Rotate up around X-axis
        rotationX -= rotationStep;
        break;
    case 's': // Rotate down around X-axis
        rotationX += rotationStep;
        break;
    case ' ': // Restart the game
        if (isGameOver) {
            restartGame();
        }
        break;
    }

    glutPostRedisplay(); // Request redisplay to apply changes
}

void checkCollisions() {
    // Check collision with platform boundaries (left, right, and top)
    float scaledBallRadius = ballRadius * ballScaleFactor;

    if (ballX + scaledBallRadius > platformSize || ballX - scaledBallRadius < -platformSize) {
        ballSpeedX = -ballSpeedX;
    }
    if (ballZ + scaledBallRadius > platformSize) {
        ballSpeedZ = -ballSpeedZ;
    }

    // Check collision with paddle (only in X and Z)
    if (ballZ - scaledBallRadius < paddleZ + paddleDepth / 2 &&
        ballZ + scaledBallRadius > paddleZ - paddleDepth / 2 &&
        ballX > paddleX - paddleWidth / 2 &&
        ballX < paddleX + paddleWidth / 2) {

        // Reflect ball in Z-axis
        ballSpeedZ = -ballSpeedZ;

        // Calculate the offset of the collision point from the paddle center
        float collisionOffset = (ballX - paddleX) / (paddleWidth / 2);

        // Adjust ballSpeedX based on the collision offset
        ballSpeedX += collisionOffset * 0.1f;  // The factor scales the influence of the offset

        // Add a small random variation to ballSpeedX
        float randomFactor = ((float)rand() / RAND_MAX) * 0.05f - 0.025f;  // Range: [-0.025, 0.025]
        ballSpeedX += randomFactor;

        // Normalize ball speed to ensure it doesn't stop moving
        if (fabs(ballSpeedX) < 0.05f) {
            ballSpeedX = ballSpeedX > 0 ? 0.05f : -0.05f;  // Set a minimum speed
        }
    }

    // Check if the ball crosses the bottom boundary
    if (ballZ - scaledBallRadius < -platformSize) {
        isGameOver = true;  // Set game over state
    }

    // Check collision with blocks (only X and Z checks)
    for (Block& block : blocks) {
        if (!block.active) continue;

        if (ballX + scaledBallRadius > block.x - 0.5 && ballX - scaledBallRadius < block.x + 0.5 &&
            ballZ + scaledBallRadius > block.z - 0.5 && ballZ - scaledBallRadius < block.z + 0.5) {

            block.active = false;  // Deactivate block
            ballSpeedZ = -ballSpeedZ;  // Reflect ball in Z-axis

            // Randomly decide to spawn a power-up (50% chance)
            if (rand() % 2 == 0) {
                spawnPowerUp(block.x, block.z);
            }
            break;
        }
    }
}

void spawnPowerUp(float x, float z) {
    PowerUp powerUp;
    powerUp.x = x;
    powerUp.y = 0.5;  // Slightly above platform
    powerUp.z = z;
    powerUp.active = true;
    powerUp.isIncrease = rand() % 2 == 0;  // Randomly decide type (increase or decrease)

    powerUps.push_back(powerUp);
}

void drawPowerUps() {
    GLfloat materialColor[4]; // RGBA material color

    for (const PowerUp& powerUp : powerUps) {
        if (!powerUp.active) continue;

        glPushMatrix();

        // Set the color for the sphere based on power-up type
        if (powerUp.isIncrease) {
            materialColor[0] = 0.0f; // Red
            materialColor[1] = 0.0f; // Green
            materialColor[2] = 1.0f; // Blue
        }
        else {
            materialColor[0] = 1.0f; // Red
            materialColor[1] = 0.0f; // Green
            materialColor[2] = 0.0f; // Blue
        }
        materialColor[3] = 1.0f; // Alpha (opacity)

        // Apply material properties
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor);

        // Draw the sphere
        glTranslatef(powerUp.x, powerUp.y, powerUp.z);
        glutSolidSphere(0.3, 20, 20); // Radius 0.3, 20 slices, 20 stacks
        glPopMatrix();
    }
}

void checkPowerUpCollision() {
    for (PowerUp& powerUp : powerUps) {
        if (!powerUp.active) continue;

        // Check if paddle is within the power-up position (X and Z only)
        if (powerUp.z < paddleZ + paddleDepth / 2 &&
            powerUp.z > paddleZ - paddleDepth / 2 &&
            powerUp.x > paddleX - paddleWidth / 2 &&
            powerUp.x < paddleX + paddleWidth / 2) {

            // Set target width based on power-up type
            if (powerUp.isIncrease) {
                targetPaddleWidth = std::min(currentPaddleWidth + 1.0, 5.0);

                targetBallScaleFactor = std::min(ballScaleFactor * 1.5, 3.0);  // Limit max scale
                // Limit max size
            }
            else {
                targetPaddleWidth = std::max(currentPaddleWidth - 1.0, 1.0);

                targetBallScaleFactor = std::max(ballScaleFactor / 1.5, 0.5);  // Limit min scale
                // Limit min size
            }

            powerUp.active = false;  // Deactivate power-up after collection
        }
    }

    // Smoothly interpolate paddle width
    currentPaddleWidth = lerp(currentPaddleWidth, targetPaddleWidth, interpolationSpeed);
    paddleWidth = currentPaddleWidth;
    ballScaleFactor = lerp(ballScaleFactor, targetBallScaleFactor, ballScaleInterpolationSpeed);
}

void restartGame() {
    // Reset ball position and speed
    ballX = 0.0;
    ballY = 0.5;
    ballZ = -8.0;
    ballScaleFactor = 1.0;
    targetBallScaleFactor = 1.0;
    // Reset ball speed to initial values
    ballSpeedX = 0.05f;
    ballSpeedZ = 0.05f;

    // Reset paddle position and size
    paddleX = 0.0;
    paddleWidth = 2.0;

    // Reactivate all blocks
    for (Block& block : blocks) {
        block.active = true;
    }

    targetPaddleWidth = 2.0;
    currentPaddleWidth = 2.0;

    // Clear all power-ups
    powerUps.clear();

    // Reset game state
    isGameOver = false;
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightPosition[] = { 0.0f, 10.0f, 5.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}