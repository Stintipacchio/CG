#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>
#include <ctime>
#include <cmath>
#include <vector>
#include <chrono>
#include <thread>

#define SPACE_BAR 32

bool pausa = false;

float PLAYER_SPEED_X = 0;

float PLAYER_SPEED_Y = 0;

float PLAYER_ACCELERATION = 0.03f;

bool aKeyIsDown = false;
bool dKeyIsDown = false;
bool spaceBarIsDown = false;
int bulletDirection = 1;

float timeSinceLastEnemy = 0.0f;
float enemySpawnTime = 5.0f; // Tempo iniziale per creare un nuovo nemico
float timeSinceLastFrame = 0.0f;
float deltaTime;

int punteggio = 0;
bool game_over = false;


float PLAYER_POSITION_X = 0.0f;
float PLAYER_POSITION_Y = -0.9f;
const float PLAYER_WIDTH = 0.05f;
const float PLAYER_HEIGHT = 0.2f;
const float PLAYER_JUMP_FORCE = 0.04f;


const float larghezzaPiattaforma = 0.5f;
const float altezzaPiattaforma = 0.05f;
const int numeroPiattaforme = 6;


GLuint playerVao, platformVao, bulletVao, enemyVao;
GLuint playerVbo, platformVbo, bulletVbo, enemyVbo;


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
    float width = 0.02f;
    float height = 0.04f;
};

std::vector<Bullet> bullets;

struct Enemy {
    float x;
    float y;
    float width;
    float height;
    float speedX = 0;
    float speedY = 0;
    float acceleration = 0;
    float jumpForce;
    bool alive = true;
};

std::vector<Enemy> enemies;



float platforCollisionDetector(float objectX, float objectY, float object_width) {
    float angolo_dx_player = objectX + object_width;

    for (Platform platform : platforms) {
        if (objectY + platform.height > platform.y) platform.surpassed = true;
        else platform.surpassed = false;
        if (((objectX >= platform.x && objectX <= platform.x + platform.width) ||
            (angolo_dx_player >= platform.x && angolo_dx_player <= platform.x + platform.width)) &&
            objectY >= platform.y && objectY <= platform.y + platform.height && platform.surpassed) {
            return platform.y + platform.height;
        }
    }
    return -0.9f;
}

void ObjectGravityHandler(float& objectX, float& objectY, float& object_speedY, float object_width) {
    if (object_speedY > 0) {
        objectY += object_speedY;
        if (object_speedY > 0) {
            object_speedY -= 0.001f;
        }
    }
    if ((object_speedY < 0 && objectY > -0.9f) || platforCollisionDetector(objectX, objectY, object_width) == -0.9f) {
        objectY += object_speedY;
        if (object_speedY > -0.06f) {
            object_speedY -= 0.001f;
        }
    }
    float tmp = platforCollisionDetector(objectX, objectY, object_width);
    if (objectY < tmp) {
        object_speedY = 0;
        objectY = tmp;
    }
}

void ObjectInertiaHandler(float& objectX, float& object_speedX) {
    if (object_speedX > 0) {
        objectX += object_speedX;
        if (object_speedX > 0) {
            object_speedX -= 0.003f;
        }
    }
    if (object_speedX < 0) {
        objectX += object_speedX;
        if (object_speedX < 0) {
            object_speedX += 0.003f;
        }
    }
    if ((object_speedX < 0.005f && object_speedX > -0.005f) || (objectX > 0.09f || objectX < -0.09f)) {
        object_speedX = 0;
    }
}

void jump(float& object_speedY, float jumpForce) {
    if (object_speedY == 0) {
        object_speedY += jumpForce;
    }
}

void ObjectLeftMover(float& objectX, float& objectY, float& object_speedX, float acceleration) {
    if (objectX > -0.98f && object_speedX > -0.02f) {
        object_speedX -= acceleration;
    }
}

void ObjectRightMover(float& objectX, float& objectY, float& object_speedX, float acceleration) {
    if (objectX < 0.94f && object_speedX < 0.02f) {
        object_speedX += acceleration;
    }
}

int randomSign(){
    int randomNumber = rand();
    if (randomNumber % 2 == 0)
        return 1;
    else
        return -1;
}


void createEnemies() {
    srand(time(NULL));
    // Creazione di un nemico
    Enemy enemy;
    enemy.x = randomSign();// Posizione X del nemico
    enemy.y = -0.9f;// Posizione Y del nemico
    enemy.width = PLAYER_WIDTH;// Larghezza del nemico
    enemy.height = PLAYER_HEIGHT;// Altezza del nemico
    enemy.acceleration = PLAYER_ACCELERATION / 3;
    enemy.speedX = 0.005f;
    enemy.jumpForce = PLAYER_JUMP_FORCE * 0.8;
    // Altri attributi inizializzati
    enemies.push_back(enemy);
}

void increasedSpawnrate() {
    enemySpawnTime *= 0.98f; // Riduci il tempo per creare un nuovo nemico del 10%
}

void enemiesSpawner() {
    float currentTime = GetTickCount() / 1000.0f;
    deltaTime = currentTime - timeSinceLastFrame;
    timeSinceLastFrame = currentTime;
    timeSinceLastEnemy += deltaTime;
    if (timeSinceLastEnemy >= enemySpawnTime) {
        createEnemies();
        increasedSpawnrate();
        timeSinceLastEnemy = 0.0f;
    }
    // Aggiorna il resto del gioco
}


void moveEnemies() {
    for (Enemy& enemy : enemies) {
        if (enemy.alive) {
 
            if (PLAYER_POSITION_X > enemy.x) {
                ObjectRightMover(enemy.x, enemy.y, enemy.speedX, enemy.acceleration);
            }
            if (PLAYER_POSITION_X < enemy.x) {
                ObjectLeftMover(enemy.x, enemy.y, enemy.speedX, enemy.acceleration);
            }
            if (PLAYER_POSITION_Y > enemy.y) {
                jump(enemy.speedY, enemy.jumpForce);
            }
            // Aggiorna la posizione dei nemici in base alla logica di movimento
            // Ad esempio, puoi incrementare o decrementare le coordinate X e Y
            //Spostamento orizzontale
            ObjectInertiaHandler(enemy.x, enemy.speedX);

            //Spostamento verticale e gravità
            ObjectGravityHandler(enemy.x, enemy.y, enemy.speedY, enemy.width);
        }

    }
}

void drawEnemies() {
    // Colore dei nemici
    glColor3f(0.0f, 1.0f, 0.0f); // Verde


    for (const auto& enemy : enemies) {
        if (enemy.alive) {
            // Calcola i vertici del nemico
            GLfloat enemyVertices[] = {
                enemy.x, enemy.y,
                enemy.x + enemy.width, enemy.y,
                enemy.x + enemy.width, enemy.y + enemy.height,
                enemy.x, enemy.y + enemy.height
            };

            // Genera e bind del Vertex Array Object (VAO) per il nemico
            GLuint enemyVao;
            glGenVertexArrays(1, &enemyVao);
            glBindVertexArray(enemyVao);

            // Genera e bind del Vertex Buffer Object (VBO) per i vertici del nemico
            GLuint enemyVbo;
            glGenBuffers(1, &enemyVbo);
            glBindBuffer(GL_ARRAY_BUFFER, enemyVbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(enemyVertices), enemyVertices, GL_STATIC_DRAW);

            // Imposta l'attributo di posizione del nemico
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            // Disegna il nemico
            glDrawArrays(GL_QUADS, 0, 4);

            // Pulizia delle risorse
            glDisableVertexAttribArray(0);
            glDeleteBuffers(1, &enemyVbo);
            glDeleteVertexArrays(1, &enemyVao);
        }
    }
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
    // Inizializza le piattaforme in posizioni casuali
    srand(time(NULL));

    // Crea una piattafroma base
    Platform basePlatform;
    basePlatform.x = -1.0f;  // Posizione x iniziale
    basePlatform.y = -0.95f;  // Posizione y
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


void shootBullet() {
    // Crea un nuovo proiettile
    Bullet newBullet;

    // Imposta la posizione e la velocità del proiettile iniziale
    if (bulletDirection == 1) {
        newBullet.x = PLAYER_POSITION_X + PLAYER_WIDTH * 0.8;
    }
    else newBullet.x = PLAYER_POSITION_X - (PLAYER_WIDTH * (2/3));
    newBullet.y = PLAYER_POSITION_Y + PLAYER_HEIGHT/2;
    newBullet.speed = 0.02f;
    newBullet.isActive = true;

    // Imposta la direzione dello sparo in base all'ultimo input dell'utente
    newBullet.direction = bulletDirection; // 1 se è stato premuto "d", -1 se è stato premuto "a"

    // Aggiungi il nuovo proiettile al vettore dei proiettili
    bullets.push_back(newBullet);
}


void bulletMovementHandler() {
    // Muove i proiettili verso destra o sinistra
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end(); /* empty */) {
        if (bulletIt->isActive) {
            bulletIt->x += (bulletIt->speed * bulletIt->direction); // Muovi il proiettile in base alla direzione dello sparo
            if (bulletIt->x > 1.0f || bulletIt->x < -1.0f) {
                bulletIt = bullets.erase(bulletIt); // Rimuovi il proiettile dagli array
            } else {
                ++bulletIt;
            }
        } else {
            ++bulletIt;
        }
    }
}


void checkBulletCollision() {
    for (auto bulletIt = bullets.begin(); bulletIt != bullets.end(); /* empty */) {
        bool bulletDeleted = false;

        for (auto enemyIt = enemies.begin(); enemyIt != enemies.end(); /* empty */) {
            if (bulletIt->x < enemyIt->x + enemyIt->width &&
                bulletIt->x + bulletIt->width > enemyIt->x &&
                bulletIt->y < enemyIt->y + enemyIt->height &&
                bulletIt->y + bulletIt->height > enemyIt->y) {

                bulletDeleted = true;
                enemyIt = enemies.erase(enemyIt);
                punteggio += 100;
            }
            else {
                ++enemyIt;
            }
        }

        if (bulletDeleted) {
            bulletIt = bullets.erase(bulletIt);
        }
        else {
            ++bulletIt;
        }
    }
}

void checkPlayerEnemyCollison() {
    for (Enemy& enemy : enemies) {
        if (PLAYER_POSITION_X < enemy.x + enemy.width &&
            PLAYER_POSITION_X + PLAYER_WIDTH > enemy.x &&
            PLAYER_POSITION_Y < enemy.y + enemy.height &&
            PLAYER_POSITION_Y + PLAYER_HEIGHT > enemy.y) {

            game_over = true;
        }
    }
}


void update(int value) {
    if (!game_over && !pausa) {
        // Aggiorna la posizione del giocatore
            //Spostamento orizzontale
        ObjectInertiaHandler(PLAYER_POSITION_X, PLAYER_SPEED_X);

        //Spostamento verticale e gravità
        ObjectGravityHandler(PLAYER_POSITION_X, PLAYER_POSITION_Y, PLAYER_SPEED_Y, PLAYER_WIDTH);

        // Aggiorna la posizione del proiettile
        bulletMovementHandler();

        checkBulletCollision();

        checkPlayerEnemyCollison();

        // Aggiorna la posizione dei nemici
        moveEnemies();

        enemiesSpawner();
    }
        // Richiede il ridisegno della scena
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
}

void drawPlayer() {
    // Colore del giocatore
    glColor3f(1.0f, 0.0f, 0.0f);


    // Ridisegna il giocatore
    glBindVertexArray(playerVao);
    glBindBuffer(GL_ARRAY_BUFFER, playerVbo);

    GLfloat PLAYER_VERTICES[] = {
        PLAYER_POSITION_X, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y,
        PLAYER_POSITION_X + PLAYER_WIDTH, PLAYER_POSITION_Y + PLAYER_HEIGHT,
        PLAYER_POSITION_X, PLAYER_POSITION_Y + PLAYER_HEIGHT
    };

    // Genera e bind del Vertex Array Object (VAO) per il giocatore
    GLuint playerVao;
    glGenVertexArrays(1, &playerVao);
    glBindVertexArray(playerVao);

    // Genera e bind del Vertex Buffer Object (VBO) per i vertici del giocatore
    GLuint playerVbo;
    glGenBuffers(1, &playerVbo);
    glBindBuffer(GL_ARRAY_BUFFER, playerVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PLAYER_VERTICES), PLAYER_VERTICES, GL_STATIC_DRAW);

    // Imposta l'attributo di posizione del giocatore
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Disegna il giocatore
    glDrawArrays(GL_QUADS, 0, 4);

    // Pulizia delle risorse
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &playerVbo);
    glDeleteVertexArrays(1, &playerVao);
  
}


void drawBullets() {
    // Colore del proiettile
    glColor3f(0.0f, 0.0f, 1.0f);


    for (const auto& bullet : bullets) {
        if (bullet.isActive) {
            // Calcola i vertici del proiettile
            GLfloat bulletVertices[] = {
                bullet.x, bullet.y,
                bullet.x + bullet.width, bullet.y,
                bullet.x + bullet.width, bullet.y + bullet.height,
                bullet.x, bullet.y + bullet.height
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



void drawPlatforms() {
    // Colore delle piattaforme
    glColor3f(1.0f, 1.0f, 0.0f);
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


        // Genera e bind del Vertex Array Object (VAO) per la piattaforma
        GLuint platformVao;
        glGenVertexArrays(1, &platformVao);
        glBindVertexArray(platformVao);

        // Genera e bind del Vertex Buffer Object (VBO) per i vertici della piattaforma
        GLuint platformVbo;
        glGenBuffers(1, &platformVbo);
        glBindBuffer(GL_ARRAY_BUFFER, platformVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(platformVertices), platformVertices, GL_STATIC_DRAW);

        // Imposta l'attributo di posizione della piattaforma
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        // Disegna la piattaforma
        glDrawArrays(GL_QUADS, 0, 4);

        // Pulizia delle risorse
        glDisableVertexAttribArray(0);
        glDeleteBuffers(1, &platformVbo);
        glDeleteVertexArrays(1, &platformVao);
    }
}



void updatePlayerInteractions() {
    if (!game_over) {
        if (aKeyIsDown) {
            ObjectLeftMover(PLAYER_POSITION_X, PLAYER_POSITION_Y, PLAYER_SPEED_X, PLAYER_ACCELERATION);
        }
        if (dKeyIsDown) {
            ObjectRightMover(PLAYER_POSITION_X, PLAYER_POSITION_Y, PLAYER_SPEED_X, PLAYER_ACCELERATION);
        }
        if (spaceBarIsDown) {
            jump(PLAYER_SPEED_Y, PLAYER_JUMP_FORCE);
        }
    }
}


void MostraPunteggio(int x, int y, float r, float g, float b, void* font, int punteggio){
    char buffer[20];
    sprintf_s(buffer, "Punteggio: %d", punteggio);
    glColor3f(r, g, b);
    glWindowPos2f(x, y);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(font, buffer[i]);
    }
}

void GameOver() {
    char buffer[10];
    sprintf_s(buffer, "Game Over");
    glColor3f(1, 0, 0);
    glWindowPos2f(640, 360);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
    }
}

void Pausa() {
    char buffer[10];
    sprintf_s(buffer, "PAUSA");
    glColor3f(1, 1, 1);
    glWindowPos2f(640, 360);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
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
    drawPlatforms();
 
    drawPlayer();

    // Disegna i proiettili sparati dal giocatore
    drawBullets();

    drawEnemies();

    MostraPunteggio(10, 700, 1, 1, 1, GLUT_BITMAP_9_BY_15, punteggio);

    if (game_over) {
        GameOver();
    }

    if (pausa) {
        Pausa();
    }

    updatePlayerInteractions();
    glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':
    case 'A':
        bulletDirection = -1;
        aKeyIsDown = true;
        break;
    case 'd':
    case 'D':
        bulletDirection = 1;
        dKeyIsDown = true;
        break;
    case SPACE_BAR:
        spaceBarIsDown = true;
        break;
    case 'f':
    case 'F':
        shootBullet();
        break;
    case 'p':
    case 'P':
        if (!pausa) {
            pausa = true;
        }
        else if (pausa) {
            pausa = false;
        }
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
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Jumper");

    glewInit();
    initializePlatforms();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
