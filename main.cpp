/*	main.cpp - Entry point dell'applicazione.	Configurazione dell'ambiente OpenGl */#include <stdio.h>#include <stdlib.h>#include <time.h>#include <math.h>#include <iostream>using namespace std;// Inclusione di OpenGL/GLUT cross platform#if defined __APPLE__    #include <OpenGL/gl.h>    #include <OpenGL/glu.h>    #include <GLUT/glut.h>#else    #include <GL/gl.h>    #include <GL/glu.h>    #include <GL/glut.h>#endif#include "block.h"#include "maze.h"#include "textureBMP.h"#define ASPECT_RATIO 1.0f#define PI 3.141592653void handleResize(int, int);void drawScene();bool detectCollision();void handleKeyDown(unsigned char key, int x, int y);void handleKeyUp(unsigned char key, int x, int y);// Mantiene in memoria i tasti premuti // (nel caso ci siano più tasti premuti contemporanemante)bool* keyStates = new bool[256];int pressedKeys = 0;GLfloat X = 1.0f; // x osservatoreGLfloat Z =	1.0f; // z osservatoreGLfloat A = 0; // angolo osservatore rispetto l'asse YGLfloat Y = 0.5f; // y osservatoreint ELAPSED_TIME = 0;int VIEWPORT_HEIGHT;int VIEWPORT_WIDTH;bool GAME_ACTIVE = true;// LabirintoMaze* m;void handleResize(int ww, int wh){	float war = (float)ww / (float)wh; // Window aspect ratio	int vw, vh; // Viewport width e height	int x = 0, y = 0;	cout <<war <<" "<< ASPECT_RATIO<<endl;	if (war > ASPECT_RATIO) {// La finestra è più larga		vh = wh;		vw = vh * ASPECT_RATIO;		x = (ww - vw)/2;	} else { // la finestra è più alta		vw = ww;		vh = vw * (1/ASPECT_RATIO);		y = (wh- vh) /2;	}	VIEWPORT_HEIGHT = vh;	VIEWPORT_WIDTH = vw;	glViewport(x, y, vw, vh);	glMatrixMode(GL_PROJECTION);	glLoadIdentity();	/*		glFrustum ( left, right, bottom, top, nearVal, farVal)		Da problemi sul clipping dei punti.		Conviene usare gluPrespective 		https://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml	*/	//glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 100);	gluPerspective(70.0f, ASPECT_RATIO, 0.05f, 100);}	string getDirection() {	int a = (int)A % 360;	if (a < 0) { a += 360;}	if ( (a >= 0 && a < 22.5) || (a >= 337.5) ) { return "NORD"; }	if ( a >= 22.5 and a < 67.5) { return "NORD-OVEST"; }	if ( a >= 67.5 and a < 112.5) { return "OVEST"; }	if ( a >= 112.5 and a < 157.5) { return "SUD-OVEST"; }	if ( a >= 157.5 and a < 202.5) { return "SUD"; }	if ( a >= 202.5 and a < 247.5) { return "SUD-EST"; }	if ( a >= 247.5 and a < 292.5) { return "EST"; }	if ( a >= 292.5 and a < 337.5) { return "NORD-EST"; }		return "";}void drawText(float x, float y, string text, float bckWidth, float bckHeight, float margin){	glMatrixMode( GL_PROJECTION );	glPushMatrix();	glLoadIdentity();	gluOrtho2D( 0, VIEWPORT_WIDTH, 0, VIEWPORT_HEIGHT );	glMatrixMode( GL_MODELVIEW );	glPushMatrix();	glLoadIdentity();	glDisable(GL_LIGHTING);	// Disegna il testo	glColor4f(0.0f,0.0f,0.0f,1.0f);	glRasterPos2i( x+ margin, y+margin );  // distanzia il testo dal bordo dello sfondo	glColor3d(1.0, 1.0, 1.0);	int len = text.length();	for ( int i = 0; i < len; ++i ) {	    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);	}		// Disegna lo sfondo	glColor4f(1.0f,1.0f,1.0f,0.7f);		glBegin(GL_POLYGON);	    glVertex2f(x, y+0);	    glVertex2f(x+bckWidth , y);	    glVertex2f(x+bckWidth, y+bckHeight);	    glVertex2f(x+0, y+bckHeight);	glEnd();	glPopMatrix();	glMatrixMode( GL_PROJECTION );	glPopMatrix();	glMatrixMode( GL_MODELVIEW );	glEnable(GL_LIGHTING);}void handleVictory() {	GAME_ACTIVE = false;	// Disegna le informazioni	float margin = 20;	string text = "HAI VINTO!";	float lineHeight = 15+2*margin;	float bckWidth = text.length()*9 + 2*margin;		float textX = (VIEWPORT_WIDTH - bckWidth) /2;	float textY = (VIEWPORT_HEIGHT - lineHeight) /2;	drawText(textX,textY, text, bckWidth, lineHeight,margin);}void handleDefeat() {	GAME_ACTIVE = false;	// TODO: riproduci un suono di un esplosione	// Disegna le informazioni	float margin = 20;	string text = "HAI PERSO!";	float lineHeight = 15+2*margin;	float bckWidth = text.length()*9 + 2*margin;		float textX = (VIEWPORT_WIDTH - bckWidth) /2;	float textY = (VIEWPORT_HEIGHT - lineHeight) /2;	drawText(textX,textY, text, bckWidth, lineHeight,margin);}void drawScene(){	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	glEnable( GL_BLEND );	glMatrixMode(GL_MODELVIEW);	glLoadIdentity();		// TODO: check per vittora o sconfitta.	int timeLeft = m->getMazeTime() - ELAPSED_TIME;	if (m->getActiveTntCount() == 0 ) { handleVictory();}	if (timeLeft == 0) { handleDefeat(); }	if (GAME_ACTIVE) {		// Rototraslazione inversa		glRotatef(-A, 0, 1, 0);		glTranslatef(-X, -Y, -Z); 		// Disegna il labirinto		m->draw();		// Disegna le informazioni		float margin = 10;		string bombString = "BOMBE RIMASTE: "+to_string(m->getActiveTntCount())+"/"+to_string(m->getTntCount());		string timeString = "TEMPO RIMASTO: "+to_string(timeLeft);		string directionString = getDirection();		float lineHeight = 15+2*margin;		float bckWidth = max(timeString.length(), bombString.length())*9 + 2*margin;				drawText(0,0, bombString, bckWidth, lineHeight,margin);		drawText(0,15+margin, timeString,bckWidth, lineHeight,margin);		float compassWidth = directionString.length()*9+2*margin;		float compassX = (VIEWPORT_WIDTH - compassWidth) /2;		drawText(compassX, VIEWPORT_WIDTH - 30, directionString, compassWidth,lineHeight,margin);			}	glDisable( GL_BLEND );	glutSwapBuffers();	cout <<"Posizione camera: ";    cout <<"X:"<< X <<" Z: " <<Z <<" A: " <<A <<endl;}void handleKeyDown(unsigned char key, int , int ) {		keyStates[key] = true; 	// variabili per ripristinare la poszione in caso di collisione	float oldX = X;	float oldZ = Z;	// A è la rotazione sull'asse Y espressa in gradi	// cresce in negativo se giro verso destra	if(keyStates[(int)'e']) A -= 5; // Gira a destra	if(keyStates[(int)'q']) A += 5; // Gira a sinistra	float radA = A * PI/180.0f;	//cout << "COS A " << cos(radA) <<" SIN -A: " <<sin(-radA) <<endl;		if(keyStates[(int)'a']) { // Laterale a sinistra		Z += 0.1f * sin(radA);		X -= 0.1f * cos(radA);	}	if(keyStates[(int)'d']) { // Laterale a destra		Z -= 0.1f * sin(radA);		X += 0.1f * cos(radA);	}	if(keyStates[(int)'w']) { // Avanti		cout << "AVANTI"<<endl;		Z -= 0.1f * cos(radA);		X += 0.1f * sin(-radA);	}	//if(key == 's') { // Indietro	if(keyStates[(int)'s']) { // Indietro		Z += 0.1f * cos(radA);		X -= 0.1f * sin(-radA);	}		if(key == 27) exit(0); // ESC per uscire	if (detectCollision()){		X = oldX;		Z = oldZ;	}	// Parte relativa alla disattivazione della tnt	int x,z;	x = round(X);	z = round(Z);	if (m->deactiveTnt(x,z)){		// TODO: Riproduci il suono della tnt disattivata		cout << "TNT disattivata in "<< x << " " << z <<endl; 	}	glutPostRedisplay();}bool detectCollision() {	// X Z	// m labiritino	float allowedOffest = 0.4f; //massima distanza dal centro consentita	int x,z;	x = round(X);	z = round(Z);	cout << "SONO IN "<<x <<" "<<z<<endl;	if (m->isWall(x,z)){ return true;} // Sono finito dentro al muro	if (m->isWall(x+1,z) && X > x+allowedOffest){ // collisione con muro a destra		cout <<"Collisione a destra"<<endl;		return true;	}	if (m->isWall(x-1,z) && X < x-allowedOffest) { // collisione con muro a sinsitra		cout <<"Collisione a sinsitra"<<endl;		return true;	}	if (m->isWall(x, z+1) && Z > z+allowedOffest) { // collisione con muro in basso		cout <<"Collisione in basso"<<endl;		return true;	}	if (m->isWall(x, z-1) && Z < z-allowedOffest) { // collisione con muro in alto		cout <<"Collisione in alto"<<endl;		return true;	}	return false;}void handleKeyUp(unsigned char key, int x, int y) {	keyStates[key] = false;}// Funzione che scandisce il tempo, viene richiamata ogni 1000msvoid timerTick(int value){	// il timer funziona solo se c'è una partita in corso	if (GAME_ACTIVE){ 	    double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;	    ELAPSED_TIME = (int)t;	    glutTimerFunc(1000, timerTick, 0);	    glutPostRedisplay();	}} int main(int argc, char **argv){	glutInit(&argc, argv);	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);	glutInitWindowPosition(100, 100);	glutInitWindowSize(400, 400);	glutCreateWindow("MazeRunner");		glEnable(GL_DEPTH_TEST);	glClearColor(0, 0, 0, 1.0f);	glEnable(GL_CULL_FACE);		glEnable(GL_LIGHTING);	glEnable(GL_LIGHT0);	//glEnable(GL_LIGHT1);	//glEnable(GL_LIGHT2);	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0); //luce solo sul lato 	principale dei poligoni		// #####################	// Configurazione luce ambientale	// #####################	GLfloat ambientLight[4] = { 0.3f, 0.3f, 0.3f, 1 };	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);	// #####################	// Configurazione torcia	// #####################	GLfloat torchLight[4] = { 1.0f, 1.0f, 1.0f, 1 };	glLightfv(GL_LIGHT0, GL_AMBIENT, torchLight);	//glLightfv(GL_LIGHT1, GL_AMBIENT, torchLight);	//glLightfv(GL_LIGHT2, GL_AMBIENT, torchLight);	// Gli altri parametri per LIGHT0 sono di default a {1,1,1,1}	//GLfloat dLite[4] = { 1.0f, 1.0f, 1.0f, 1 };	//GLfloat sLite[4] = { 1.0f, 1.0f, 1.0f, 1 };	//glLightfv(GL_LIGHT1, GL_DIFFUSE, dLite);	//glLightfv(GL_LIGHT1, GL_SPECULAR, sLite);	//glLightfv(GL_LIGHT2, GL_DIFFUSE, dLite);	//glLightfv(GL_LIGHT2, GL_SPECULAR, sLite);	    glLightf(GL_LIGHT0,GL_SPOT_CUTOFF, 60.0f);    glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,0.0f);        //glLightf(GL_LIGHT1,GL_SPOT_CUTOFF, 20.0f);    //glLightf(GL_LIGHT1,GL_SPOT_EXPONENT,128.0f);    //    //glLightf(GL_LIGHT2,GL_SPOT_CUTOFF, 40.0f);    //glLightf(GL_LIGHT2,GL_SPOT_EXPONENT,0.0f);        GLfloat torchPosition[4] = { 0.0f, 0.0f, 1.0f, 1.0f };	glLightfv(GL_LIGHT0, GL_POSITION, torchPosition);		//GLfloat torchPosition1[4] = { 0.0f, 0.0f, -1.0f, 1.0f };	//glLightfv(GL_LIGHT1, GL_POSITION, torchPosition1);	//GLfloat torchPosition2[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	//glLightfv(GL_LIGHT2, GL_POSITION, torchPosition2);		GLfloat light_direction[] = { 0,0,-1};	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION,light_direction);	//glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION,light_direction);	//glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION,light_direction);		//glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.0f);	//glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 2.0f);		// materiale	//GLfloat ambiente[4] = { 1.0f, 1.0f, 1.0f, 1 };	//GLfloat direttiva[4] = { 1, 1, 1, 1 };	//GLfloat brillante[4] = { 1, 1, 1, 1 };	//	////glMateriali(GL_FRONT, GL_SHININESS, 32);	//	//glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);	//glMaterialfv(GL_FRONT, GL_DIFFUSE, direttiva);	//glMaterialfv(GL_FRONT, GL_SPECULAR, brillante);			glutReshapeFunc(handleResize);	glutKeyboardFunc(handleKeyDown);	glutKeyboardUpFunc(handleKeyUp);	glutDisplayFunc(drawScene);			m = new Maze();	// ###########	// Configurazione del timer - https://www.opengl.org/resources/libraries/glut/spec3/node64.html	// 20 - lower bound per i ms tra una chiamata e l'altra	// timerTick - funzione da chiamare, riceve un int che specifica il timer che è scattato	// 0 - id del timer	glutTimerFunc(20, timerTick, 0);	glutMainLoop();		return(0);}