#include "ShaderMaker.h"
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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static unsigned int programId;

#define SPACE_BAR 32
#define ENTER '\r'

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
bool startGame = false;
bool pausa = false;
bool showHitBox = false;


float PLAYER_POSITION_X = 0.0f;
float PLAYER_POSITION_Y = -0.9f;
const float PLAYER_WIDTH = 0.15f;
const float PLAYER_HEIGHT = 0.2f;
const float PLAYER_JUMP_FORCE = 0.04f;


const float larghezzaPiattaforma = 0.5f;
const float altezzaPiattaforma = 0.05f;
const int numeroPiattaforme = 6;

unsigned int PlayerTexture;
unsigned int EnemyTexture;
unsigned int BulletTexture;
unsigned int PlatformTexture;
unsigned int BackgroundTexture;


GLuint playerVao, platformVao, bulletVao, enemyVao, backgroundVao;
GLuint playerVbo, platformVbo, bulletVbo, enemyVbo, backgroundVbo;


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
    float speedX = 0.0f;
    float speedY = 0.0f;
    float acceleration = 0.0f;
    float jumpForce;
    int direction;
    bool alive = true;
};

std::vector<Enemy> enemies;


void initShader(void)
{
    GLenum ErrorCheckValue = glGetError();

    char* vertexShader = (char*)"vertexShader.glsl";
    char* fragmentShader = (char*)"fragmentShader.glsl";

    programId = ShaderMaker::createProgram(vertexShader, fragmentShader);

}

void loadPlayerTexture() {

    stbi_set_flip_vertically_on_load(true);

    // Abilita il blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carica l'immagine della texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Textures/Player.png", &width, &height, &nrChannels, 0);

    // Genera un ID per la texture e la lega
    glGenTextures(1, &PlayerTexture);
    glBindTexture(GL_TEXTURE_2D, PlayerTexture);

    // Imposta i parametri di wrapping e filtering della texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carica i dati dell'immagine nella texture
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture Player" << std::endl;
    }
    stbi_image_free(data);

}

void loadEnemyTexture() {

    stbi_set_flip_vertically_on_load(true);

    // Abilita il blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carica l'immagine della texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Textures/Enemy.png", &width, &height, &nrChannels, 0);

    // Genera un ID per la texture e la lega
    glGenTextures(1, &EnemyTexture);
    glBindTexture(GL_TEXTURE_2D, EnemyTexture);

    // Imposta i parametri di wrapping e filtering della texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carica i dati dell'immagine nella texture
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture Enemy" << std::endl;
    }
    stbi_image_free(data);

}

void loadBulletTexture() {

    stbi_set_flip_vertically_on_load(true);

    // Abilita il blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carica l'immagine della texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Textures/bullet.png", &width, &height, &nrChannels, 0);

    // Genera un ID per la texture e la lega
    glGenTextures(1, &BulletTexture);
    glBindTexture(GL_TEXTURE_2D, BulletTexture);

    // Imposta i parametri di wrapping e filtering della texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carica i dati dell'immagine nella texture
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture Bullet" << std::endl;
    }
    stbi_image_free(data);

}

void loadPlatformTexture() {

    stbi_set_flip_vertically_on_load(true);

    // Abilita il blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carica l'immagine della texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Textures/platform.png", &width, &height, &nrChannels, 0);

    // Genera un ID per la texture e la lega
    glGenTextures(1, &PlatformTexture);
    glBindTexture(GL_TEXTURE_2D, PlatformTexture);

    // Imposta i parametri di wrapping e filtering della texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carica i dati dell'immagine nella texture
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture Platform" << std::endl;
    }
    stbi_image_free(data);
}

void loadBackgroundTexture() {

    stbi_set_flip_vertically_on_load(true);

    // Abilita il blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Carica l'immagine della texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("./Textures/background.png", &width, &height, &nrChannels, 0);

    // Genera un ID per la texture e la lega
    glGenTextures(1, &BackgroundTexture);
    glBindTexture(GL_TEXTURE_2D, BackgroundTexture);

    // Imposta i parametri di wrapping e filtering della texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carica i dati dell'immagine nella texture
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture Background" << std::endl;
    }
    stbi_image_free(data);
}

void texturesLoader() {
    loadPlayerTexture();
    loadEnemyTexture();
    loadBulletTexture();
    loadPlatformTexture();
    loadBackgroundTexture();
}

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

int randomSign() {
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
    enemy.width = PLAYER_WIDTH / 2;// Larghezza del nemico
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
                enemy.direction = 1;
            }
            if (PLAYER_POSITION_X < enemy.x) {
                ObjectLeftMover(enemy.x, enemy.y, enemy.speedX, enemy.acceleration);
                enemy.direction = -1;
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
            GLfloat enemyVertices[16];

            if (enemy.direction == 1) {
                enemyVertices[0] = enemy.x;
                enemyVertices[1] = enemy.y;
                enemyVertices[2] = 0.0f;
                enemyVertices[3] = 0.0f;

                enemyVertices[4] = enemy.x + enemy.width;
                enemyVertices[5] = enemy.y;
                enemyVertices[6] = 1.0f;
                enemyVertices[7] = 0.0f;

                enemyVertices[8] = enemy.x + enemy.width;
                enemyVertices[9] = enemy.y + enemy.height;
                enemyVertices[10] = 1.0f;
                enemyVertices[11] = 1.0f;

                enemyVertices[12] = enemy.x;
                enemyVertices[13] = enemy.y + enemy.height;
                enemyVertices[14] = 0.0f;
                enemyVertices[15] = 1.0f;
            }
            else {
                enemyVertices[0] = enemy.x;
                enemyVertices[1] = enemy.y;
                enemyVertices[2] = 1.0f;
                enemyVertices[3] = 0.0f;

                enemyVertices[4] = enemy.x + enemy.width;
                enemyVertices[5] = enemy.y;
                enemyVertices[6] = 0.0f;
                enemyVertices[7] = 0.0f;

                enemyVertices[8] = enemy.x + enemy.width;
                enemyVertices[9] = enemy.y + enemy.height;
                enemyVertices[10] = 0.0f;
                enemyVertices[11] = 1.0f;

                enemyVertices[12] = enemy.x;
                enemyVertices[13] = enemy.y + enemy.height;
                enemyVertices[14] = 1.0f;
                enemyVertices[15] = 1.0f;
            }

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
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            // Imposta l'attributo delle coordinate della texture del nemico
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            // Attiva la texture
            glBindTexture(GL_TEXTURE_2D, EnemyTexture);

            // Disegna il nemico
            glDrawArrays(GL_QUADS, 0, 4);

            // Pulizia delle risorse
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
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
            platform.y = (rand() / (float)RAND_MAX) * 1.0f - 0.7f;
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


void shootBullet() {
    // Crea un nuovo proiettile
    Bullet newBullet;

    // Imposta la posizione e la velocità del proiettile iniziale
    if (bulletDirection == 1) {
        newBullet.x = PLAYER_POSITION_X + PLAYER_WIDTH * 0.8;
    }
    else newBullet.x = PLAYER_POSITION_X - (PLAYER_WIDTH * (2 / 3));
    newBullet.y = PLAYER_POSITION_Y + PLAYER_HEIGHT / 3;
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
            }
            else {
                ++bulletIt;
            }
        }
        else {
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

void drawPlayer() {
    // Colore del giocatore
    glColor3f(1.0f, 0.0f, 0.0f);

    // Ridisegna il giocatore
    glBindVertexArray(playerVao);
    glBindBuffer(GL_ARRAY_BUFFER, playerVbo);

    GLfloat PLAYER_VERTICES[16];

    if (bulletDirection == 1) {
        PLAYER_VERTICES[0] = PLAYER_POSITION_X;
        PLAYER_VERTICES[1] = PLAYER_POSITION_Y;
        PLAYER_VERTICES[2] = 0.0f;
        PLAYER_VERTICES[3] = 0.0f;

        PLAYER_VERTICES[4] = PLAYER_POSITION_X + PLAYER_WIDTH;
        PLAYER_VERTICES[5] = PLAYER_POSITION_Y;
        PLAYER_VERTICES[6] = 1.0f;
        PLAYER_VERTICES[7] = 0.0f;

        PLAYER_VERTICES[8] = PLAYER_POSITION_X + PLAYER_WIDTH;
        PLAYER_VERTICES[9] = PLAYER_POSITION_Y + PLAYER_HEIGHT;
        PLAYER_VERTICES[10] = 1.0f;
        PLAYER_VERTICES[11] = 1.0f;

        PLAYER_VERTICES[12] = PLAYER_POSITION_X;
        PLAYER_VERTICES[13] = PLAYER_POSITION_Y + PLAYER_HEIGHT;
        PLAYER_VERTICES[14] = 0.0f;
        PLAYER_VERTICES[15] = 1.0f;
    }
    else {
        PLAYER_VERTICES[0] = PLAYER_POSITION_X;
        PLAYER_VERTICES[1] = PLAYER_POSITION_Y;
        PLAYER_VERTICES[2] = 1.0f;
        PLAYER_VERTICES[3] = 0.0f;

        PLAYER_VERTICES[4] = PLAYER_POSITION_X + PLAYER_WIDTH;
        PLAYER_VERTICES[5] = PLAYER_POSITION_Y;
        PLAYER_VERTICES[6] = 0.0f;
        PLAYER_VERTICES[7] = 0.0f;

        PLAYER_VERTICES[8] = PLAYER_POSITION_X + PLAYER_WIDTH;
        PLAYER_VERTICES[9] = PLAYER_POSITION_Y + PLAYER_HEIGHT;
        PLAYER_VERTICES[10] = 0.0f;
        PLAYER_VERTICES[11] = 1.0f;

        PLAYER_VERTICES[12] = PLAYER_POSITION_X;
        PLAYER_VERTICES[13] = PLAYER_POSITION_Y + PLAYER_HEIGHT;
        PLAYER_VERTICES[14] = 1.0f;
        PLAYER_VERTICES[15] = 1.0f;
    }


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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Imposta l'attributo delle coordinate della texture del giocatore
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Attiva la texture
    glBindTexture(GL_TEXTURE_2D, PlayerTexture);

    // Disegna il giocatore
    glDrawArrays(GL_QUADS, 0, 4);

    // Pulizia delle risorse
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
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
                // Posizioni         // Coordinate della texture
                bullet.x, bullet.y, 0.0f, 0.0f,
                bullet.x + bullet.width, bullet.y, 1.0f, 0.0f,
                bullet.x + bullet.width, bullet.y + bullet.height, 1.0f, 1.0f,
                bullet.x, bullet.y + bullet.height, 0.0f, 1.0f
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
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            // Imposta l'attributo delle coordinate della texture del proiettile
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            // Attiva la texture
            glBindTexture(GL_TEXTURE_2D, BulletTexture);

            // Disegna il proiettile
            glDrawArrays(GL_QUADS, 0, 4);

            // Pulizia delle risorse
            glDisableVertexAttribArray(0);
            glDisableVertexAttribArray(1);
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
            // Posizioni         // Coordinate della texture
            platform.x, platform.y, 0.0f, 0.0f,
            platform.x + platform.width, platform.y, 1.0f, 0.0f,
            platform.x + platform.width, platform.y + platform.height, 1.0f, 1.0f,
            platform.x, platform.y + platform.height, 0.0f, 1.0f
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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        // Imposta l'attributo delle coordinate della texture della piattaforma
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        // Attiva la texture
        glBindTexture(GL_TEXTURE_2D, PlatformTexture);

        // Disegna la piattaforma
        glDrawArrays(GL_QUADS, 0, 4);

        // Pulizia delle risorse
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDeleteBuffers(1, &platformVbo);
        glDeleteVertexArrays(1, &platformVao);
    }
}

void drawBackground() {
    // Calcola i vertici del rettangolo di sfondo
    GLfloat backgroundVertices[] = {
        // Posizioni         // Coordinate della texture
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };

    // Genera e bind del Vertex Array Object (VAO) per lo sfondo
    GLuint backgroundVao;
    glGenVertexArrays(1, &backgroundVao);
    glBindVertexArray(backgroundVao);

    // Genera e bind del Vertex Buffer Object (VBO) per i vertici dello sfondo
    GLuint backgroundVbo;
    glGenBuffers(1, &backgroundVbo);
    glBindBuffer(GL_ARRAY_BUFFER, backgroundVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(backgroundVertices), backgroundVertices, GL_STATIC_DRAW);

    // Imposta l'attributo di posizione dello sfondo
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Imposta l'attributo delle coordinate della texture dello sfondo
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // Attiva la texture
    glBindTexture(GL_TEXTURE_2D, BackgroundTexture);

    // Disegna lo sfondo
    glDrawArrays(GL_QUADS, 0, 4);

    // Pulizia delle risorse
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, &backgroundVbo);
    glDeleteVertexArrays(1, &backgroundVao);
}



void MostraPunteggio(int x, int y, float r, float g, float b, void* font, int punteggio) {
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

void printGameOver() {
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
void printRetry() {
    char buffer[100];
    sprintf_s(buffer, "Premere R per riprovare");
    glColor3f(0, 1, 0);
    glWindowPos2f(570, 330);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
    }
}

void printPausa() {
    char buffer[10];
    sprintf_s(buffer, "PAUSA");
    glColor3f(1, 1, 1);
    glWindowPos2f(640, 340);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
    }
}

void printStartGame() {
    char buffer[100];
    sprintf_s(buffer, "Premi invio per cominciare");
    glColor3f(1, 1, 1);
    glWindowPos2f(560, 340);
    int len, i;
    len = (int)strlen(buffer);
    for (i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, buffer[i]);
    }
}


void update(int value) {
    if (!game_over && !pausa && startGame) {
        // Aggiorna la posizione del giocatore
            //Spostamento orizzontale
        ObjectInertiaHandler(PLAYER_POSITION_X, PLAYER_SPEED_X);
        updatePlayerInteractions();

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

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (showHitBox) {
        // Itera sulle piattaforme e disegna ciascuna di esse
        drawPlatforms();

        drawPlayer();

        // Disegna i proiettili sparati dal giocatore
        drawBullets();

        drawEnemies();
    }
    else {
        glUseProgram(programId);

        drawBackground();

        // Itera sulle piattaforme e disegna ciascuna di esse
        drawPlatforms();

        drawPlayer();

        // Disegna i proiettili sparati dal giocatore
        drawBullets();

        drawEnemies();
        glUseProgram(0);
    }

    MostraPunteggio(10, 700, 1, 1, 1, GLUT_BITMAP_9_BY_15, punteggio);

    if (game_over) {
        printGameOver();
        printRetry();
    }

    if (pausa && !game_over) {
        printPausa();
    }

    if (!startGame) {
        printStartGame();
    }

    glutSwapBuffers();
}

void resetGame() {
    PLAYER_SPEED_X = 0;
    PLAYER_SPEED_Y = 0;
    PLAYER_POSITION_X = 0.0f;
    PLAYER_POSITION_Y = -0.9f;
    enemies.clear();
    bullets.clear();
    platforms.clear();
    initializePlatforms();
    punteggio = 0;
    game_over = false;
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
    case 'h':
    case 'H':
        if (!showHitBox) {
            showHitBox = true;
        }
        else if (showHitBox) {
            showHitBox = false;
        }
        break;
    case 'r':
    case 'R':
        resetGame();
    case ENTER:
        startGame = true;
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

    texturesLoader();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, update, 0);

    initShader();
    glutMainLoop();
    return 0;
}
