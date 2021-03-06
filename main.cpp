/*
    main.cpp - Entry point dell'applicazione.

    Configurazione dell'ambiente OpenGL/OpenAL
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <string>

using namespace std;
// Inclusione di OpenGL/GLUT/ALUT cross platform
#if defined __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
    #include "/usr/local/Cellar/freealut/1.1.0/include/AL/alut.h"
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    #include <AL/al.h>
    #include <AL/alc.h>
    #include <AL/alut.h>
#endif

#include "block.h"
#include "maze.h"
#include "textureBMP.h"

#define ASPECT_RATIO 1.0f
#define PI 3.141592653

void handleResize(int, int);
string getDirection();
void drawText(float x, float y, string text, float bckWidth, float bckHeight, float margin);
void drawScene();
void handleKeyDown(unsigned char key, int x, int y);
void handleKeyUp(unsigned char key, int x, int y);
bool detectCollision();
void handleVictory();
void handleDefeat();
void timerTick(int);
void initLight();
void playSoundAtPosition(ALuint, float, float, float);
void initAudio();

// Mantiene in memoria i tasti premuti 
// (nel caso ci siano più tasti premuti contemporanemante)
bool* keyStates;
int pressedKeys = 0;

GLfloat X = 0; // x osservatore
GLfloat Z = 0; // z osservatore
GLfloat A = 0; // angolo osservatore rispetto l'asse Y
GLfloat Y = 0.5f; // y osservatore

int ELAPSED_TIME = 0;

int VIEWPORT_HEIGHT; // Dimensioni del riquadro in cui viene renderizzata la scena
int VIEWPORT_WIDTH;

bool GAME_ACTIVE = true; // Booleano che specifica se c'è una partita in corso

Maze* m; // Labirinto

ALCdevice* device; // dispositivo di riproduzione audio per ALUT
ALCcontext* context; // contesto audio per ALUT
ALuint explosion_sound_source, explosion_sound_buffer;
ALuint victory_sound_source, victory_sound_buffer;
ALuint pick_up_sound_source, pick_up_sound_buffer;

/*
    Funzione che gestisce il resize della finestra
*/
void handleResize(int ww, int wh) {
    float war = (float)ww / (float)wh; // Window aspect ratio
    int vw, vh; // Viewport width e height
    int x = 0, y = 0;

    cout <<war <<" "<< ASPECT_RATIO<<endl;
    if (war > ASPECT_RATIO) {// La finestra è più larga
        vh = wh;
        vw = vh * ASPECT_RATIO;
        x = (ww - vw)/2;
    } else { // la finestra è più alta
        vw = ww;
        vh = vw * (1/ASPECT_RATIO);
        y = (wh- vh) /2;
    }
    VIEWPORT_HEIGHT = vh;
    VIEWPORT_WIDTH = vw;
    /*
        Imposta un Viewport quadrato che occupa il massimo spazio possibile
        della finestra. Lo spazio inutilizzato viene lasciato nero.
    */
    glViewport(x, y, vw, vh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /*
        glFrustum ( left, right, bottom, top, nearVal, farVal)

        Da problemi sul clipping dei punti.
        Conviene usare gluPrespective 
        https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
    */
    //glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 100);
    gluPerspective(70.0f, ASPECT_RATIO, 0.05f, 100);
}   

/*
    Ottiene l'orientamento in termini di "coordinate geografiche" a partire 
    dall'angolo globale
*/
string getDirection() {
    int a = (int)A % 360;
    if (a < 0) { a += 360;}

    if ( (a >= 0 && a < 22.5) || (a >= 337.5) ) { return "NORD"; }
    if ( a >= 22.5 and a < 67.5) { return "NORD-OVEST"; }
    if ( a >= 67.5 and a < 112.5) { return "OVEST"; }
    if ( a >= 112.5 and a < 157.5) { return "SUD-OVEST"; }
    if ( a >= 157.5 and a < 202.5) { return "SUD"; }
    if ( a >= 202.5 and a < 247.5) { return "SUD-EST"; }
    if ( a >= 247.5 and a < 292.5) { return "EST"; }
    if ( a >= 292.5 and a < 337.5) { return "NORD-EST"; }
    
    return "";
}

/*
    Disegna del testo a video:
        - x,y: cooradinate 2d del testo
        - text: stringa da disegnare
        - bckWidth: larghezza dello sfondo
        - bckHeight: altezza dello sfondo
        - margin: margine (CSS-like)
        drawText(x, y, testo, larghezza, altezza, margine)
*/
void drawText(float x, float y, string text, float bckWidth, float bckHeight, float margin){
    glMatrixMode( GL_PROJECTION ); 
    glPushMatrix(); // Push di una matrice per non alterare lo stato corrente
    glLoadIdentity();
    gluOrtho2D( 0, VIEWPORT_WIDTH, 0, VIEWPORT_HEIGHT );// Proiezione in modalità 2D

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING); // Disattiva le luci in modo da utilizzare i colori flat

    // Disegna il testo
    glColor4f(0.0f,0.0f,0.0f,1.0f);
    glRasterPos2i( x+ margin, y+margin );  // distanzia il testo dal bordo dello sfondo
    glColor3d(1.0, 1.0, 1.0);
    int len = text.length();
    for ( int i = 0; i < len; ++i ) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);
    }
    
    // Disegna lo sfondo
    glColor4f(1.0f,1.0f,1.0f,0.7f);
    glBegin(GL_POLYGON);
        glVertex2f(x, y+0);
        glVertex2f(x+bckWidth , y);
        glVertex2f(x+bckWidth, y+bckHeight);
        glVertex2f(x+0, y+bckHeight);
    glEnd();

    glPopMatrix();
    glMatrixMode( GL_PROJECTION ); // Ripristina lo stack della matrice di proiezione
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glEnable(GL_LIGHTING);
}

/*
    Renderizza la scena principale
*/
void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND ); // Blending per l'HUD semi-trasparente
    glMatrixMode(GL_MODELVIEW);
    if (!GAME_ACTIVE) return; // evita che venga chiamato più volte
    glLoadIdentity();
    
    // Check per vittora o sconfitta.
    int timeLeft = m->getMazeTime() - ELAPSED_TIME + 1; //"+1" per mostrare i comandi per 1 s
    if (m->getActiveTntCount() == 0 ) { handleVictory();}
    if (timeLeft == 0) { handleDefeat(); }

    if (timeLeft > m->getMazeTime()){
        // Mostra i comandi per il primo secondo della partita
        string comandi = "   COMANDI DEL GIOCO: ";
        string comandi1 = "w:    Avanti";
        string comandi2 = "s:    Indietro";
        string comandi3 = "a:    Spostamento a sinistra";
        string comandi4 = "d:    Spostamento a destra";
        string comandi5 = "q:    Rotazione a sinistra";
        string comandi6 = "e:    Rotazione a destra";
        string comandi7 = "esc:  Esci";

        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 + 90, comandi, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 + 60, comandi1, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 + 30, comandi2, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 , comandi3, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 - 30, comandi4, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 - 60, comandi5, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 - 90, comandi6, 280 , 30, 10);
        drawText(VIEWPORT_WIDTH/2 - 140, VIEWPORT_HEIGHT/2 - 120, comandi7, 280 , 30, 10);
    }

    //if (GAME_ACTIVE) {
    if (GAME_ACTIVE && timeLeft <= m->getMazeTime()) {
        // Posizionamento dell'osservatore
        // Rototraslazione inversa
        glRotatef(-A, 0, 1, 0);
        glTranslatef(-X, -Y, -Z); 
        alListener3f(AL_POSITION, X, Y, Z);
        // orientamento dell'ascoltatore
        float radA = A * PI/180.0f;
        ALfloat listenerOri[] = {
            (float)sin(-radA),0,(float)-cos(radA), 
            0,1,0
        };  
        alListenerfv(AL_ORIENTATION, listenerOri);

        // Disegna il labirinto
        m->draw();

        // Disegna le informazioni
        float margin = 10;
        string bombString = "BOMBE RIMASTE: "+to_string(m->getActiveTntCount())+"/"+to_string(m->getTntCount());
        string timeString = "TEMPO RIMASTO: "+to_string(timeLeft);
        string directionString = getDirection();

        float lineHeight = 15+2*margin;
        float bckWidth = max(timeString.length(), bombString.length())*9 + 2*margin;
        
        drawText(0,0, bombString, bckWidth, lineHeight, margin);
        drawText(0,15+margin, timeString,bckWidth, lineHeight, margin);

        float compassWidth = directionString.length()*9+2*margin;
        float compassX = (VIEWPORT_WIDTH - compassWidth) /2;
        drawText(compassX, VIEWPORT_WIDTH - 30, directionString, compassWidth,lineHeight,margin);
        
    }

    glDisable( GL_BLEND );
    glutSwapBuffers();
    cout <<"Posizione camera: ";
    cout <<"X:"<< X <<" Z: " <<Z <<" A: " <<A <<endl;
}

void handleKeyDown(unsigned char key, int , int ) { 
    keyStates[key] = true; 

    // variabili per ripristinare la poszione in caso di collisione
    float oldX = X;
    float oldZ = Z;

    // A è la rotazione sull'asse Y espressa in gradi
    // cresce in negativo se giro verso destra
    if(keyStates[(int)'e']) A -= 5; // Gira a destra
    if(keyStates[(int)'q']) A += 5; // Gira a sinistra
    float radA = A * PI/180.0f;
    //cout <<"A: " <<A <<" COS A " << cos(radA) <<" SIN -A: " <<sin(-radA) <<endl;

    if(keyStates[(int)'a']) { // Laterale a sinistra
        Z += 0.1f * sin(radA);
        X -= 0.1f * cos(radA);
    }
    if(keyStates[(int)'d']) { // Laterale a destra
        Z -= 0.1f * sin(radA);
        X += 0.1f * cos(radA);
    }

    if(keyStates[(int)'w']) { // Avanti
        cout << "AVANTI"<<endl;
        Z -= 0.1f * cos(radA);
        X += 0.1f * sin(-radA);
    }
    //if(key == 's') { // Indietro
    if(keyStates[(int)'s']) { // Indietro
        Z += 0.1f * cos(radA);
        X -= 0.1f * sin(-radA);
    }
    
    if(key == 27) exit(0); // ESC per uscire

    if (detectCollision()){
        X = oldX;
        Z = oldZ;
    }

    // Parte relativa alla disattivazione della tnt
    int x,z;
    x = round(X);
    z = round(Z);
    if (m->deactiveTnt(x,z)){
        cout << "TNT disattivata in "<< x << " " << z <<endl; 
        playSoundAtPosition(pick_up_sound_source,X,Y,Z);
    }
    glutPostRedisplay();
}

void handleKeyUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

bool detectCollision() {
    // m labiritino
    float allowedOffest = 0.4f; //massima distanza dal centro consentita
    int x,z;
    x = round(X);
    z = round(Z);
    //cout << "SONO IN "<<x <<" "<<z<<endl;
    if (m->isWall(x,z)){ return true;} // Sono finito dentro al muro
    if (m->isWall(x+1,z) && X > x+allowedOffest){ // collisione con muro a destra
        cout <<"Collisione a destra"<<endl;
        return true;
    }
    if (m->isWall(x-1,z) && X < x-allowedOffest) { // collisione con muro a sinsitra
        cout <<"Collisione a sinsitra"<<endl;
        return true;
    }
    if (m->isWall(x, z+1) && Z > z+allowedOffest) { // collisione con muro in basso
        cout <<"Collisione in basso"<<endl;
        return true;
    }
    if (m->isWall(x, z-1) && Z < z-allowedOffest) { // collisione con muro in alto
        cout <<"Collisione in alto"<<endl;
        return true;
    }
    return false;
}

void handleVictory() {
    GAME_ACTIVE = false;
    // Disegna le informazioni
    float margin = 20;
    string text = "HAI VINTO!";

    float lineHeight = 15+2*margin;
    float bckWidth = text.length()*9 + 2*margin;
    
    float textX = (VIEWPORT_WIDTH - bckWidth) /2;
    float textY = (VIEWPORT_HEIGHT - lineHeight) /2;

    drawText(textX, textY, text, bckWidth, lineHeight,margin);

    ALint state;
    alGetSourcei(victory_sound_source, AL_SOURCE_STATE, &state);
    // Controlla che il suono della vittoria non sia già in riproduzione
    if (state != AL_PLAYING) {
        m->stopSounds();
        // Riproduce il suono che da gloria al vincitore
        playSoundAtPosition(victory_sound_source,X,Y,Z);
    }
}

void handleDefeat() {
    GAME_ACTIVE = false;
    float margin = 20;
    string text = "HAI PERSO!";

    float lineHeight = 15+2*margin;
    float bckWidth = text.length()*9 + 2*margin;
    
    float textX = (VIEWPORT_WIDTH - bckWidth) /2;
    float textY = (VIEWPORT_HEIGHT - lineHeight) /2;

    drawText(textX,textY, text, bckWidth, lineHeight,margin);


    ALint state;
    alGetSourcei(explosion_sound_source, AL_SOURCE_STATE, &state);
    // Controlla che il suono della sconfitta non sia già in riproduzione
    if (state != AL_PLAYING) {
        m->stopSounds();
        // Riproduce il suono di un esplosione
        playSoundAtPosition(explosion_sound_source,X,Y,Z);
    }
}

// Funzione che scandisce il tempo, viene richiamata ogni 1000ms
void timerTick(int value){
    // il timer funziona solo se c'è una partita in corso
    if (GAME_ACTIVE){ 
        double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        ELAPSED_TIME = (int)t;
        glutTimerFunc(1000, timerTick, 0);
        glutPostRedisplay();
    }
} 

void initLight() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0); //luce solo sul lato principale dei poligoni
    
    // #####################
    // Configurazione luce ambientale
    // #####################
    GLfloat ambientLight[4] = { 0.08f, 0.08f, 0.08f, 1 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    // #####################
    // Configurazione torcia
    // #####################
    GLfloat torchLight[4] = { 1.0f, 1.0f, 1.0f, 1 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, torchLight);
    
    glLightf(GL_LIGHT0,GL_SPOT_CUTOFF, 35.0f);
    glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,4.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.2f);
    

    GLfloat torchPosition[4] = { 0.0f, 0.2f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, torchPosition);
    GLfloat light_direction[] = { 0,0,-1};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION,light_direction);

}

void playSoundAtPosition(ALuint source, float x, float y, float z){
    alSource3f(source, AL_POSITION, x,y,z);
    alSourcePlay(source);
}

void initAudio(){
    // tutte le sorgenti di default emettono a 360 gradi
    device = alcOpenDevice(NULL);
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    alListener3f(AL_VELOCITY, 0, 0, 0); // Velocità osservatore
    // Per far si che i suoni vengano spazializzati correttamente
    // devono essere suoni stereo

    // esplosione fnale
    alGenSources(1, &explosion_sound_source);
    string file = "assets/sounds/explosion.wav";
    explosion_sound_buffer = alutCreateBufferFromFile(file.c_str());
    alSourcei(explosion_sound_source, AL_BUFFER, explosion_sound_buffer);
    alSourcef(explosion_sound_source, AL_PITCH, 1.0f);
    alSourcef(explosion_sound_source, AL_GAIN, 1.0f);
    alSource3f(explosion_sound_source, AL_POSITION, 0, 0, 0);
    alSource3f(explosion_sound_source, AL_VELOCITY, 0, 0, 0);
    alSourcei(explosion_sound_source, AL_LOOPING, AL_FALSE); 
  
    alSourcef(explosion_sound_source, AL_ROLLOFF_FACTOR, 2);//Velocità con cui si attenua
    alSourcef(explosion_sound_source, AL_REFERENCE_DISTANCE, 1); // Distanza entro la quale non c'è attenuazione
    alSourcef(explosion_sound_source, AL_MAX_DISTANCE, 3); // Massima distanza fino alla quale si sente il suono

    // vittoria finale
    alGenSources(1, &victory_sound_source);
    file = "assets/sounds/victory.wav";
    victory_sound_buffer = alutCreateBufferFromFile(file.c_str());
    alSourcei(victory_sound_source, AL_BUFFER, victory_sound_buffer);
    alSourcef(victory_sound_source, AL_PITCH, 1.0f);
    alSourcef(victory_sound_source, AL_GAIN, 1.0f);
    alSource3f(victory_sound_source, AL_POSITION, 0, 0, 0);
    alSource3f(victory_sound_source, AL_VELOCITY, 0, 0, 0);
    alSourcei(victory_sound_source, AL_LOOPING, AL_FALSE); 
    alSourcef(victory_sound_source, AL_ROLLOFF_FACTOR, 2);//Velocità con cui si attenua
    alSourcef(victory_sound_source, AL_REFERENCE_DISTANCE, 1); // Distanza entro la quale non c'è attenuazione
    alSourcef(victory_sound_source, AL_MAX_DISTANCE, 3); // Massima distanza fino alla quale si sente il suono

    // pick-up di una bomba
    alGenSources(1, &pick_up_sound_source);
    file = "assets/sounds/pickup.wav";
    pick_up_sound_buffer = alutCreateBufferFromFile(file.c_str());
    alSourcei(pick_up_sound_source, AL_BUFFER, pick_up_sound_buffer);
    alSourcef(pick_up_sound_source, AL_PITCH, 1.0f);
    alSourcef(pick_up_sound_source, AL_GAIN, 4.0f);
    alSource3f(pick_up_sound_source, AL_POSITION, 0, 0, 0);
    alSource3f(pick_up_sound_source, AL_VELOCITY, 0, 0, 0);
    alSourcei(pick_up_sound_source, AL_LOOPING, AL_FALSE); 
    alSourcef(pick_up_sound_source, AL_ROLLOFF_FACTOR, 1);//Velocità con cui si attenua
    alSourcef(pick_up_sound_source, AL_REFERENCE_DISTANCE, 2); // Distanza entro la quale non c'è attenuazione
    alSourcef(pick_up_sound_source, AL_MAX_DISTANCE, 3); // Massima distanza fino alla quale si sente il suono
}

int main(int argc, char **argv) {
    // Array che specifica quali tasti sono premuti nel caso l'utente prema
    // più tasti contemporaneamente.
    // E' di 256 elementi perché così può essere indicizzato utilizzando
    // una variabile di tipo char.
    keyStates = new bool[256];
    // E' necessario inizializzare l'array perché sennò
    // può capitare che ci siano dei valori a true che sfasano
    // la ricognizione dei tasti.
    for (int i = 0; i < 256; i++){ keyStates[i] = false;}

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(400, 400);
    glutCreateWindow("MazeRunner");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1.0f);
    glEnable(GL_CULL_FACE); // Non renderizza le facce interne
    
    initLight();
    
    // Inizializzazione comparto audio
    alutInit(0, NULL);
    initAudio();
    
    glutReshapeFunc(handleResize);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutDisplayFunc(drawScene);

    //Lettura delle specifiche dello schema dal file ".txt" passato come paramentro
    //cout<<"prima lettura specifiche"<<endl;
    //cout <<"argc " <<argc <<endl;
    if (argc==1){
        m = new Maze("levels/debug1.txt");
    }
    else if (argc==2)
    {
        char * path= argv[1];
        //cout << path << endl;
        m = new Maze(path);
    }
    else{
        cout<<"ERRORE nell'inserimento dei parametri"<<endl;
        return (0); 
    }
    
    X = m->getMazeObeserverX();
    Z = m->getMazeObeserverZ();
    A = m->getMazeObeserverA();
    alListener3f(AL_POSITION, X, Y, Z);


    // ###########
    // Configurazione del timer - https://www.opengl.org/resources/libraries/glut/spec3/node64.html
    // 20 - lower bound per i ms tra una chiamata e l'altra
    // timerTick - funzione da chiamare, riceve un int che specifica il timer che è scattato
    // 0 - id del timer
    glutTimerFunc(20, timerTick, 0);


    glutMainLoop();
    
    // Uscita ambiente audio    
    alDeleteSources(1, &explosion_sound_source);
    alDeleteBuffers(1, &explosion_sound_buffer);
    alDeleteSources(1, &victory_sound_source);
    alDeleteBuffers(1, &victory_sound_buffer);
    alDeleteSources(1, &pick_up_sound_source);
    alDeleteBuffers(1, &pick_up_sound_buffer);

    alcDestroyContext(context);
    alcCloseDevice(device);
    alutExit();
    return(0);
}
