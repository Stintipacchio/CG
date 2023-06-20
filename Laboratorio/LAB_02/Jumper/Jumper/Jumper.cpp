#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>

#define SPACE_BAR 32

float PLAYER_SPEED_X = 0;

float PLAYER_SPEED_Y = 0;

bool aKeyIsDown = false;
bool dKeyIsDown = false;
bool spaceBarIsDown = false;

int punteggio = 0;
bool game_over = false;


float posizioneXGiocatore = 0.0f;
float posizioneYGiocatore = -0.9f;

const float quadratoWidth = 0.1f;
const float quadratoHeight = 0.1f;
const float larghezzaPiattaforma = 0.5f;
const float altezzaPiattaforma = 0.05f;
const int numeroPiattaforme = 10;
const int velocitaSalto = 12;
const int gravita = 1;


GLuint quadratoVao;
GLuint quadratoVbo;

#include <vector>

struct Platform {
    float x;
    float y;
    float width;
    float height;
};

std::vector<Platform> platforms;

void initializePlatforms() {
    // Initialize platforms with random positions and sizes
    for (int i = 0; i < numeroPiattaforme; i++) {
        Platform platform;
        platform.x = (rand() % 200 - 100) / 100.0f;
        platform.y = (rand() % 200 - 100) / 100.0f;
        platform.width = larghezzaPiattaforma;
        platform.height = altezzaPiattaforma;
        platforms.push_back(platform);
    }
}

void displayPlatforms() {
    // Draw platforms
    for (const Platform& platform : platforms) {
        glBegin(GL_QUADS);
        glVertex2f(platform.x, platform.y);
        glVertex2f(platform.x + platform.width, platform.y);
        glVertex2f(platform.x + platform.width, platform.y + platform.height);
        glVertex2f(platform.x, platform.y + platform.height);
        glEnd();
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
    if (PLAYER_SPEED_X > 0) {
        posizioneXGiocatore += PLAYER_SPEED_X;
        if (PLAYER_SPEED_X > 0) {
            PLAYER_SPEED_X -= 0.003f;
        }
    }
    if (PLAYER_SPEED_X < 0) {
        posizioneXGiocatore += PLAYER_SPEED_X;
        if (PLAYER_SPEED_X < 0) {
            PLAYER_SPEED_X += 0.003f;
        }
    }
    if ((PLAYER_SPEED_X < 0.005f && PLAYER_SPEED_X > -0.005f) || (posizioneXGiocatore > 0.09f || posizioneXGiocatore < -0.09f)) {
        PLAYER_SPEED_X = 0;
    }

    //Spostamento verticale e gravità
    if (PLAYER_SPEED_Y > 0) {
        posizioneYGiocatore += PLAYER_SPEED_Y;
        if (PLAYER_SPEED_Y > 0) {
            PLAYER_SPEED_Y -= 0.001f;
        }
    }
    if (PLAYER_SPEED_Y < 0 && posizioneYGiocatore > -0.9f) {
        posizioneYGiocatore += PLAYER_SPEED_Y;
        if (PLAYER_SPEED_Y > -0.06f) {
            PLAYER_SPEED_Y -= 0.001f;
        }
    }
    if (posizioneYGiocatore <= -0.9f) {
        PLAYER_SPEED_Y = 0;
    }



    // Ridisegna il quadrato
    glBindVertexArray(quadratoVao);
    glBindBuffer(GL_ARRAY_BUFFER, quadratoVbo);

    GLfloat quadratoVertices[] = {
        posizioneXGiocatore, posizioneYGiocatore,
        posizioneXGiocatore + quadratoWidth, posizioneYGiocatore,
        posizioneXGiocatore + quadratoWidth, posizioneYGiocatore + quadratoHeight,
        posizioneXGiocatore, posizioneYGiocatore + quadratoHeight
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
        posizioneXGiocatore, posizioneYGiocatore,
        posizioneXGiocatore + quadratoWidth, posizioneYGiocatore,
        posizioneXGiocatore + quadratoWidth, posizioneYGiocatore + quadratoHeight,
        posizioneXGiocatore, posizioneYGiocatore + quadratoHeight
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(quadratoVertices), quadratoVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void updatePlayerSpeed() {
    if (aKeyIsDown) {
        if (posizioneXGiocatore > -0.9f && PLAYER_SPEED_X > -0.02f) {
            PLAYER_SPEED_X -= 0.06f;
        }
    }
    if (dKeyIsDown) {
        if (posizioneXGiocatore < 0.8f && PLAYER_SPEED_X < 0.02f) {
            PLAYER_SPEED_X += 0.06f;
        }
    }
    if (spaceBarIsDown) {
        if (PLAYER_SPEED_Y == 0) {
            PLAYER_SPEED_Y += 0.04f;
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
    updatePlayerSpeed();
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
    glutInitWindowSize(400, 400);
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
