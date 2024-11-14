#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLUT/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WINDOW_X (1200)
#define WINDOW_Y (1000)
#define WINDOW_NAME "SLOT MACHINE"
#define TEXTURE_WIDTH (360)
#define TEXTURE_HEIGHT (150)

const int reel1_dict[21] = {5, 0, 2, 3, 2, 4, 1, 4, 3, 2, 3, 0, 6, 3, 2, 3, 4, 2, 3, 2, 3};
const int reel2_dict[21] = {2, 0, 3, 4, 6, 2, 1, 3, 4, 2, 5, 3, 4, 2, 1, 3, 4, 2, 5, 3, 4};
const int reel3_dict[21] = {3, 0, 1, 5, 2, 3, 6, 5, 2, 3, 6, 5, 2, 3, 6, 5, 2, 3, 6, 5, 2};

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);

void draw_pyramid();
void draw_cell(int reel_num);
void glut_idle();
void set_texture();

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 30.0;
double cell_size = 2.5;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
int reel[3] = {0, 5, 9};
GLuint g_TextureHandles[7] = {0, 0, 0, 0, 0, 0, 0};

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
  glGenTextures(7, g_TextureHandles);

  for(int i = 0; i < 7; i++){
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  set_texture();
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutMouseFunc(glut_mouse);
	glutMotionFunc(glut_motion);
	glutPassiveMotionFunc(glut_motion);
  glutIdleFunc(glut_idle);
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
  int reel_num;
  for (int i = -1; i < 2; i++){
    for (int j = -1; j < 2; j++){
      if (i == -1){
        if (j == 1) reel_num = reel1_dict[reel[0]];
        else if (j == 0) reel_num = reel1_dict[(reel[0] + 1) % 21];
        else if (j == -1) reel_num = reel1_dict[(reel[0] + 2) % 21];
      } else if (i == 0) {
        if (j == 1) reel_num = reel2_dict[reel[1]];
        else if (j == 0) reel_num = reel2_dict[(reel[1] + 1) % 21];
        else if (j == -1) reel_num = reel2_dict[(reel[1] + 2) % 21];
      } else {
        if (j == 1) reel_num = reel3_dict[reel[2]];
        else if (j == 0) reel_num = reel3_dict[(reel[2] + 1) % 21];
        else if (j == -1) reel_num = reel3_dict[(reel[2] + 2) % 21];
      }

      glPushMatrix();
      glTranslatef(cell_size * i, 0.0, 0.0);
      glTranslatef(0.0, (cell_size * 2 / 3) * j, 0.0);
      draw_cell(reel_num);
      glPopMatrix();
    }
  }

	glFlush();
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void glut_idle(){
  static int counter = 0;

  if (counter == 3){
    reel[0] = (reel[0] - 1 + 21) % 21;
    reel[1] = (reel[1] - 1 + 21) % 21;
    reel[2] = (reel[2] - 1 + 21) % 21;
    counter = -1;
  }
  counter++;
  glutPostRedisplay();
}

void draw_cell(int reel_num) {
  double half = cell_size / 2;
  double one_third = cell_size / 3;

  glColor3d(1.0, 1.0, 1.0);

  glBindTexture(GL_TEXTURE_2D, g_TextureHandles[reel_num]);
  glEnable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  glTexCoord2d(0.0, 1.0); glVertex3d(-half, -one_third, 0.0);
  glTexCoord2d(1.0, 1.0); glVertex3d(half, -one_third, 0.0);
  glTexCoord2d(1.0, 0.0); glVertex3d(half, one_third, 0.0);
  glTexCoord2d(0.0, 0.0); glVertex3d(-half, one_third, 0.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

void set_texture() {
  const char *inputFileNames[7] = {
    "img/seven.png",
    "img/bar.png",
    "img/tiger.png",
    "img/grape.png",
    "img/cherry.png",
    "img/bell.png",
    "img/clown.png",
  };
  for (int i = 0; i < 7; i++){
    cv::Mat input = cv::imread(inputFileNames[i], 1);

    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, (TEXTURE_WIDTH - input.cols) / 2, (TEXTURE_HEIGHT - input.rows) / 2, input.cols, input.rows, GL_BGR, GL_UNSIGNED_BYTE, input.data);
  }
}