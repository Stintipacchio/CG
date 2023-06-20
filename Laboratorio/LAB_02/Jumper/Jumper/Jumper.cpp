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

float PLAYER_ACCELERATION = 0.03f;

bool aKeyIsDown = false;
bool dKeyIsDown = false;
bool spaceBarIsDown = false;
int tmp_direction = 1;

int punteggio = 0;
bool game_over = false;


float PLAYER_POSITION_X = 0.0f;
float PLAYER_POSITION_Y = -0.9f;

const float PLAYER_WIDTH = 0.1f;
const float PLAYER_HEIGHT = 0.2f;
const float larghezzaPiattaforma = 0.5f;
const float altezzaPiattaforma = 0.05f;
const int numeroPiattaforme = 6;


GLuint playerVao, platformVao, bulletVao;
GLuint playerVbo, platformVbo, bulletVbo;

#include <vector>

struct Platform {
    float x;
    float y;
    float width;
    float height;
    bool surpassed = false;
};

std::vector<Platform> platforms;

struct Bullet {
    float x;
    float y;
    float speed;
    bool isActive;
    int direction;
};

std::vector<Bullet> bullets;

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

    // Create a base platform
    Platform basePlatform;
    basePlatform.x = -1.0f;  // Posizione x iniziale
    basePlatform.y = -0.9f;  // Posizione y
    basePlatform.width = 2.0f;  // Larghezza dello schermo
    basePlatform.height = altezzaPiattaforma;  // Altezza della piattaforma

    platforms.push_back(basePlatform);

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
    glBindVertexArray(playerVao);

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
        glBindBuffer(GL_ARRAY_BUFFER, playerVbo);
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
        if (playerY + platform.height > platform.y) platform.surpassed = true;
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

void shootBullet() {
    // Crea un nuovo proiettile
    Bullet newBullet;

    // Imposta la posizione e la velocità del proiettile iniziale
    if (tmp_direction == 1) {
        newBullet.x = PLAYER_POSITION_X + PLAYER_WIDTH * 0.8;
    }
    else newBullet.x = PLAYER_POSITION_X - (PLAYER_WIDTH * (2/3));
    newBullet.y = PLAYER_POSITION_Y + PLAYER_HEIGHT/2;
    newBullet.speed = 0.02f;
    newBullet.isActive = true;

    // Imposta la direzione dello sparo in base all'ultimo input dell'utente
    newBullet.direction = tmp_direction; // 1 se è stato premuto "d", -1 se è stato premuto "a"

    // Aggiungi il nuovo proiettile al vettore dei proiettili
    bullets.push_back(newBullet);
}


void bulletMovementHandler() {
    // Muove i proiettili verso destra o sinistra
    for (Bullet& bullet : bullets) {
        if (bullet.isActive) {
            bullet.x += (bullet.speed * bullet.direction); // Muovi il proiettile in base alla direzione dello sparo
            if (bullet.x > 1.0f || bullet.x < -1.0f) {
                bullet.isActive = false; // Disattiva il proiettile se esce dallo schermo
            }
        }
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

    // Aggiorna la posizione del proiettile
    bulletMovementHandler();

    // Ridisegna il quadrato
    glBindVertexArray(playerVao);
    glBindBuffer(GL_ARRAY_BUFFER, playerVbo);

    GLfloat PLAYER_VERTICES[] = {
        PLAYER_POSITION_X, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y + PLAYER_HEIGHT,
        PLAYER_POSITION_X, PLAYER_POSITION_Y + PLAYER_HEIGHT
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(PLAYER_VERTICES), PLAYER_VERTICES, GL_STATIC_DRAW);

    // Richiede il ridisegno della scena
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void drawBullets() {
    // Colore del proiettile
    glColor3f(0.0f, 0.0f, 1.0f);

    // Dimensioni del proiettile
    const float bulletWidth = 0.02f;
    const float bulletHeight = 0.04f;

    for (const auto& bullet : bullets) {
        if (bullet.isActive) {
            // Calcola i vertici del proiettile
            GLfloat bulletVertices[] = {
                bullet.x, bullet.y,
                bullet.x + bulletWidth, bullet.y,
                bullet.x + bulletWidth, bullet.y + bulletHeight,
                bullet.x, bullet.y + bulletHeight
            };

            // Genera e bind del Vertex Array Object (VAO) per il proiettile
            GLuint bulletVao;
            glGenVertexArrays(1, &bulletVao);
            glBindVertexArray(bulletVao);

            // Genera e bind del Vertex Buffer Object (VBO) per i vertici del proiettile
            GLuint bulletVbo;
            glGenBuffers(1, &bulletVbo);
            glBindBuffer(GL_ARRAY_BUFFER, bulletVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(bulletVertices), bulletVertices, GL_STATIC_DRAW);

            // Imposta l'attributo di posizione del proiettile
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            // Disegna il proiettile
            glDrawArrays(GL_QUADS, 0, 4);

            // Pulizia delle risorse
            glDisableVertexAttribArray(0);
            glDeleteBuffers(1, &bulletVbo);
            glDeleteVertexArrays(1, &bulletVao);
        }
    }
}



void initializeVaoVbo() {
    // Inizializza il VAO e il VBO per il quadrato
    glGenVertexArrays(1, &playerVao);
    glBindVertexArray(playerVao);

    glGenBuffers(1, &playerVbo);
    glBindBuffer(GL_ARRAY_BUFFER, playerVbo);

    GLfloat PLAYER_VERTICES[] = {
        PLAYER_POSITION_X, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y + PLAYER_HEIGHT,
        PLAYER_POSITION_X, PLAYER_POSITION_Y + PLAYER_HEIGHT
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(PLAYER_VERTICES), PLAYER_VERTICES, GL_STATIC_DRAW);

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
            PLAYER_SPEED_X -= PLAYER_ACCELERATION;
        }
    }
    if (dKeyIsDown) {
        if (PLAYER_POSITION_X < 0.8f && PLAYER_SPEED_X < 0.02f) {
            PLAYER_SPEED_X += PLAYER_ACCELERATION;
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

    // Disegna il giocatore
    glBindVertexArray(playerVao);
    glColor3f(1.0f, 0.0f, 0.0f); // Imposta il colore a rosso
    glDrawArrays(GL_QUADS, 0, 4);

    // Disegna le piattaforme
    glBindVertexArray(platformVao);
    glColor3f(1.0f, 1.0f, 0.0f); // Imposta il colore a giallo

    // Itera sulle piattaforme e disegna ciascuna di esse
    for (const Platform& platform : platforms) {
        GLfloat platformVertices[] = {
            platform.x, platform.y,
            platform.x + platform.width, platform.y,
            platform.x + platform.width, platform.y + platform.height,
            platform.x, platform.y + platform.height
        };

        glBindBuffer(GL_ARRAY_BUFFER, platformVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertices), platformVertices, GL_STATIC_DRAW);

        glDrawArrays(GL_QUADS, 0, 4);
    }

    // Disegna i proiettili sparati dal giocatore
    drawBullets();

    updatePlayerInteractions();
    glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        tmp_direction = -1;
        aKeyIsDown = true;
        break;
    case 'd':
    case 'D':
        tmp_direction = 1;
        dKeyIsDown = true;
        break;
    case SPACE_BAR:
        spaceBarIsDown = true;
        break;
    case 'f':
    case 'F':
        shootBullet();
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
