#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>

#define WINDOW_X (1200)
#define WINDOW_Y (1000)
#define WINDOW_NAME "SLOT MACHINE"

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);

void draw_pyramid();
void draw_cell();

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 30.0;
double cell_size = 2.5;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;

int main(int argc, char *argv[]){
	/* OpenGLの初期化 */
	init_GL(argc,argv);

	/* このプログラム特有の初期化 */
	init();

	/* コールバック関数の登録 */
	set_callback_functions();

	/* メインループ */
	glutMainLoop(); 

	return 0;
}


void init_GL(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_X,WINDOW_Y);
	glutCreateWindow(WINDOW_NAME);
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);         // 背景の塗りつぶし色を指定
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutMouseFunc(glut_mouse);
	glutMotionFunc(glut_motion);
	glutPassiveMotionFunc(glut_motion);
}

void glut_keyboard(unsigned char key, int x, int y){
	switch(key){
	case 'q':
	case 'Q':
	case '\033':
		exit(0);
	}
	glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON){
		if(state == GLUT_UP){
			g_isLeftButtonOn = false;
		}else if(state == GLUT_DOWN){
			g_isLeftButtonOn = true;
		}
	}

	if(button == GLUT_RIGHT_BUTTON){
		if(state == GLUT_UP){
			g_isRightButtonOn = false;
		}else if(state == GLUT_DOWN){
			g_isRightButtonOn = true;
		}
	}
}

void glut_motion(int x, int y){
	static int px = -1, py = -1;
	if(g_isLeftButtonOn == true){
		if(px >= 0 && py >= 0){
			g_angle1 += (double)-(x - px)/20;
			g_angle2 += (double)(y - py)/20;
		}
		px = x;
		py = y;
	}else if(g_isRightButtonOn == true){
		if(px >= 0 && py >= 0){
			g_distance += (double)(y - py)/20;
		}
		px = x;
		py = y;
	}else{
		px = -1;
		py = -1;
	}
	glutPostRedisplay();
}

void glut_display(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (cos(g_angle2)>0){
	gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1), 
		g_distance * sin(g_angle2), 
		g_distance * cos(g_angle2) * cos(g_angle1), 
		0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  } else {
	gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                g_distance * sin(g_angle2),
                g_distance * cos(g_angle2) * cos(g_angle1),
                0.0, 0.0, 0.0, 0.0, -1.0, 0.0);}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

  // draw reel
  for (int i = -1; i < 2; i++){
    for (int j = -1; j < 2; j++){
      glPushMatrix();
      glTranslatef(cell_size * i, 0.0, 0.0);
      glTranslatef(0.0, (cell_size * 2 / 3) * j, 0.0);
      glColor3f(1.0, 1.0, 1.0);
      draw_cell();
      glPopMatrix();
    }
  }

	glFlush();
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void draw_cell() {
  double half = cell_size / 2;
  double one_third = cell_size / 3;

  glBegin(GL_QUADS);
  glVertex3d(-half, -one_third, 0.0);
  glVertex3d(half, -one_third, 0.0);
  glVertex3d(half, one_third, 0.0);
  glVertex3d(-half, one_third, 0.0);
  glEnd();
}