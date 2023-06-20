#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>

#define SPACE_BAR 32

float PLAYER_SPEED_X = 0;

float PLAYER_SPEED_Y = 0;

bool aKeyIsDown = false;
bool dKeyIsDown = false;
bool spaceBarIsDown = false;
bool fKeyIsDown = false;

int punteggio = 0;
bool game_over = false;


float PLAYER_POSITION_X = 0.0f;
float PLAYER_POSITION_Y = -0.9f;

const float PLAYER_WIDTH = 0.1f;
const float PLAYER_HEIGHT = 0.2f;
const float larghezzaPiattaforma = 0.5f;
const float altezzaPiattaforma = 0.05f;
const int numeroPiattaforme = 10;


GLuint quadratoVao, platformVao;
GLuint quadratoVbo, platformVbo;

#include <vector>

struct Platform {
    float x;
    float y;
    float width;
    float height;
    bool surpassed = false;
};

std::vector<Platform> platforms;

void jump() {
    PLAYER_SPEED_Y += 0.04f;
}

bool isOverlapping(Platform platform1, Platform platform2) {
    if (platform1.x < platform2.x + platform2.width &&
        platform1.x + platform1.width > platform2.x &&
        platform1.y < platform2.y + platform2.height &&
        platform1.y + platform1.height > platform2.y) {
        return true;
    }
    return false;
}

void initializePlatforms() {
    // Initialize platforms with random positions
    srand(time(0));
    for (int i = 0; i < numeroPiattaforme; i++) {
        Platform platform;
        bool overlapping;
        do {
            overlapping = false;
            platform.x = (rand() / (float)RAND_MAX) * 1.8f - 0.9f;
            platform.y = (rand() / (float)RAND_MAX) * 1.8f - 0.9f;
            platform.width = larghezzaPiattaforma;
            platform.height = altezzaPiattaforma;
            for (Platform existingPlatform : platforms) {
                if (isOverlapping(platform, existingPlatform)) {
                    overlapping = true;
                    break;
                }
            }
        } while (overlapping);
        platforms.push_back(platform);
    }
}


void displayPlatforms() {
    // Collega il VAO per le piattaforme
    glBindVertexArray(quadratoVao);

    // Itera sulle piattaforme
    for (const Platform& platform : platforms) {
        // Calcola i vertici della piattaforma
        GLfloat platformVertices[] = {
            platform.x, platform.y,
            platform.x + platform.width, platform.y,
            platform.x + platform.width, platform.y + platform.height,
            platform.x, platform.y + platform.height
        };

        // Collega il VBO per i vertici della piattaforma
        glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertices), platformVertices, GL_STATIC_DRAW);

        // Abilita l'array degli attributi dei vertici
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Disegna la piattaforma
        glDrawArrays(GL_QUADS, 0, 4);
    }

    // Scollega il VAO e il VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


float getPlayerPlatformHeight(float playerX, float playerY) {
    for (Platform platform : platforms) {
        if (playerY > platform.y) platform.surpassed = true;
        else platform.surpassed = false;
        if ((playerX >= platform.x &&
            playerX <= platform.x + platform.width &&
            playerY >= platform.y &&
            playerY <= platform.y + platform.height) && platform.surpassed) {
            return platform.y + platform.height;
        }
    }
    return -0.9f;
}
void PlayerGravityHandler() {
    if (PLAYER_SPEED_Y > 0) {
        PLAYER_POSITION_Y += PLAYER_SPEED_Y;
        if (PLAYER_SPEED_Y > 0) {
            PLAYER_SPEED_Y -= 0.001f;
        }
    }
    if ((PLAYER_SPEED_Y < 0 && PLAYER_POSITION_Y > -0.9f) || getPlayerPlatformHeight(PLAYER_POSITION_X, PLAYER_POSITION_Y) == -0.9f) {
        PLAYER_POSITION_Y += PLAYER_SPEED_Y;
        if (PLAYER_SPEED_Y > -0.06f) {
            PLAYER_SPEED_Y -= 0.001f;
        }
    }
    if (PLAYER_POSITION_Y < getPlayerPlatformHeight(PLAYER_POSITION_X, PLAYER_POSITION_Y)) {
        PLAYER_SPEED_Y = 0;
    }
}

void PlayerInertiaHandler() {
    if (PLAYER_SPEED_X > 0) {
        PLAYER_POSITION_X += PLAYER_SPEED_X;
        if (PLAYER_SPEED_X > 0) {
            PLAYER_SPEED_X -= 0.003f;
        }
    }
    if (PLAYER_SPEED_X < 0) {
        PLAYER_POSITION_X += PLAYER_SPEED_X;
        if (PLAYER_SPEED_X < 0) {
            PLAYER_SPEED_X += 0.003f;
        }
    }
    if ((PLAYER_SPEED_X < 0.005f && PLAYER_SPEED_X > -0.005f) || (PLAYER_POSITION_X > 0.09f || PLAYER_POSITION_X < -0.09f)) {
        PLAYER_SPEED_X = 0;
    }
}

void update(int value) {
    if (!game_over) {


    }

    if (punteggio >= 1000) {
        game_over = true;
    }

    // Aggiorna la posizione del giocatore
        //Spostamento orizzontale
        PlayerInertiaHandler();

        //Spostamento verticale e gravità
        PlayerGravityHandler();

    // Ridisegna il quadrato
    glBindVertexArray(quadratoVao);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    GLfloat quadratoVertices[] = {
        PLAYER_POSITION_X, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y + PLAYER_HEIGHT,
        PLAYER_POSITION_X, PLAYER_POSITION_Y + PLAYER_HEIGHT
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);

    // Richiede il ridisegno della scena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void initializeVaoVbo() {
    // Inizializza il VAO e il VBO per il quadrato
    glGenVertexArrays(1, &quadratoVao);
    glBindVertexArray(quadratoVao);

    glGenBuffers(1, &quadratoVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    GLfloat quadratoVertices[] = {
        PLAYER_POSITION_X, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y + PLAYER_HEIGHT,
        PLAYER_POSITION_X, PLAYER_POSITION_Y + PLAYER_HEIGHT
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Inizializza il VAO e il VBO per le piattaforme
    glGenVertexArrays(1, &platformVao);
    glBindVertexArray(platformVao);

    glGenBuffers(1, &platformVbo);
    glBindBuffer(GL_ARRAY_BUFFER, platformVbo);

    // Calcola la dimensione totale del buffer dei dati
    GLsizeiptr platformBufferSize = sizeof(GLfloat) * 4 * platforms.size();

    // Alloca memoria per il buffer dei dati e copia i vertici delle piattaforme
    GLfloat* platformBufferData = new GLfloat[platformBufferSize];
    int platformBufferOffset = 0;
    for (const Platform& platform : platforms) {
        platformBufferData[platformBufferOffset++] = platform.x;
        platformBufferData[platformBufferOffset++] = platform.y;
        platformBufferData[platformBufferOffset++] = platform.x + platform.width;
        platformBufferData[platformBufferOffset++] = platform.y;
        platformBufferData[platformBufferOffset++] = platform.x + platform.width;
        platformBufferData[platformBufferOffset++] = platform.y + platform.height;
        platformBufferData[platformBufferOffset++] = platform.x;
        platformBufferData[platformBufferOffset++] = platform.y + platform.height;
    }

    // Copia i dati nel buffer dei dati del VBO
    glBufferData(GL_ARRAY_BUFFER, platformBufferSize, platformBufferData, GL_STATIC_DRAW);

    // Libera la memoria allocata per il buffer dei dati
    delete[] platformBufferData;

    // Abilita l'array degli attributi dei vertici
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Scollega il VAO e il VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void updatePlayerInteractions() {
    if (aKeyIsDown) {
        if (PLAYER_POSITION_X > -0.9f && PLAYER_SPEED_X > -0.02f) {
            PLAYER_SPEED_X -= 0.06f;
        }
    }
    if (dKeyIsDown) {
        if (PLAYER_POSITION_X < 0.8f && PLAYER_SPEED_X < 0.02f) {
            PLAYER_SPEED_X += 0.06f;
        }
    }
    if (spaceBarIsDown) {
        if (PLAYER_SPEED_Y == 0) {
            jump();
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glBindVertexArray(quadratoVao);
    glColor3f(1.0f, 1.0f, 0.0f);
    glDrawArrays(GL_QUADS, 0, 4);
    displayPlatforms();

    //std::cout << "Punteggio: " << punteggio << std::endl;
    updatePlayerInteractions();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        aKeyIsDown = true;
        break;
    case 'd':
    case 'D':
        dKeyIsDown = true;
        break;
    case SPACE_BAR:
        spaceBarIsDown = true;
        break;
    case 'f':
    case 'F':
        fKeyIsDown = true;
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        aKeyIsDown = false;
        break;
    case 'd':
    case 'D':
        dKeyIsDown = false;
        break;
    case SPACE_BAR:
        spaceBarIsDown = false;
        break;
    case 'f':
    case 'F':
        fKeyIsDown = false;
        break;
    }
}


void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 480);
    glutCreateWindow("Jumper");

    glewInit();
    initializeVaoVbo();
    initializePlatforms();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, update, 0);


    glutMainLoop();
    return 0;
}
