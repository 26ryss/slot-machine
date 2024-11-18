#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <random>
#include <ctime>
#include <GLUT/glut.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <alure.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WINDOW_X (1200)
#define WINDOW_Y (800)
#define WINDOW_NAME "SLOT MACHINE"
#define TEXTURE_WIDTH (360)
#define TEXTURE_HEIGHT (150)

std::mt19937 rng(static_cast<unsigned int>(std::time(0))); // 乱数生成器を初期化
std::uniform_real_distribution<double> dist(0.0, 1.0); // 0.0から1.0までの一様分布

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
void draw_machine();
void glut_idle();
void set_texture();
int calc_result(int* reel);
int calc_score(int* comb);
void play_sound(char* filename);

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 30.0;
double cell_width = 2.5;
double cell_height = cell_width * 2 / 3;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
int credit = 50;
int reel[3] = {0, 5, 9};
int machine_mode = 0; // 0: normal, 1: gogo, 2: bonus
int bonus_count = 0;
bool is_bet = false;
bool is_reeling[3] = {false, false, false};
GLuint g_TextureHandles[8] = {0, 0, 0, 0, 0, 0, 0, 0};

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

  // gogo
  glBindTexture(GL_TEXTURE_2D, g_TextureHandles[7]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 200, 150, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

  // left reel
  case '1':
    if(is_reeling[0]){
      play_sound("sound/button.wav");
      if (machine_mode == 0) {
        is_reeling[0] = false;
      } else if (machine_mode == 1){
        is_reeling[0] = false;
        reel[0] = 0;
      } else {
        is_reeling[0] = false;
        reel[0] = 2;
      }
    }
    break;

  // center reel
  case '2':
    if(is_reeling[1] && !is_reeling[0]){
      play_sound("sound/button.wav");
      if (machine_mode == 0) {
        is_reeling[1] = false;
      } else if (machine_mode == 1){
        is_reeling[1] = false;
        reel[1] = 0;
      } else {
        is_reeling[1] = false;
        reel[1] = 1;
      }
    }
    break;

  // right reel
  case '3':
    if(is_reeling[2] && !is_reeling[1]){
      play_sound("sound/button.wav");
      if (machine_mode == 0) {
        is_reeling[2] = false;
        if (dist(rng) < 0.9) {
          machine_mode = 1;
          play_sound("sound/gogo.wav");
        }
        credit += calc_result(reel); 
      } else if (machine_mode == 1){
        play_sound("sound/big.wav");
        is_reeling[2] = false;
        reel[2] = 0;
        // 777 -> bonus mode
        machine_mode = 2;
        bonus_count = 0;
      } else {
        is_reeling[2] = false;
        reel[2] = 4;
        credit += calc_result(reel); 
        bonus_count += calc_result(reel);
        if (bonus_count >= 290) {
          machine_mode = 0;
          bonus_count = 0;
        }
      }
      std::cout << "credit: " << credit << '\n';
    }
    break;
  
  // maxbet button
  case '4':
    if (!is_reeling[0] && !is_reeling[1] && !is_reeling[2] && is_bet == false && credit >= 3){
      play_sound("sound/bet.wav");
      is_bet = true;
      credit -= 3;
    }
    break;

  // pushdown lever
  case '5':
    if (is_bet){
      play_sound("sound/lever.wav");
      is_reeling[0] = true;
      is_reeling[1] = true;
      is_reeling[2] = true;
      is_bet = false;
    }
    break;

  // for debug
  case '6':
    machine_mode = 1;
    break;
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
      glTranslatef(cell_width * i, 0.0, 0.0);
      glTranslatef(0.0, (cell_width * 2 / 3) * j, 0.0);
      draw_cell(reel_num);
      glPopMatrix();
    }
  }

  glPushMatrix();
  draw_machine();
  glPopMatrix();

	glFlush();
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void glut_idle(){
  static int counter = 0;

  if (counter == 2){
    if (is_reeling[0]) reel[0] = (reel[0] - 1 + 21) % 21;
    if (is_reeling[1]) reel[1] = (reel[1] - 1 + 21) % 21;
    if (is_reeling[2]) reel[2] = (reel[2] - 1 + 21) % 21;
    counter = -1;
  }
  counter++;
  glutPostRedisplay();
}

void draw_cell(int reel_num) {
  double half = cell_width / 2;
  double one_third = cell_width / 3;

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

void draw_machine() {
  double width = cell_width * 3;
  double height = cell_height * 3;
  double offset_x = 3.0;
  double offset_y = 2.0;
  GLdouble body_color[3] = {0.941, 0.157, 0.314};
  GLdouble silver[3] = {0.753, 0.753, 0.753};

  // top board
  glColor3d(body_color[0], body_color[1], body_color[2]);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 2, cell_height * 1.5, 0.5);
  glVertex3d(width / 2, cell_height * 1.5, 0.5);
  glVertex3d(width / 2, cell_height * 1.5 + offset_y, 0.5);
  glVertex3d(-width / 2, cell_height * 1.5 + offset_y, 0.5);
  glEnd();

  // bottom board
  glColor3d(body_color[0], body_color[1], body_color[2]);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 2, -cell_height * 1.5 - offset_y, 0.5);
  glVertex3d(width / 2, -cell_height * 1.5 - offset_y, 0.5);
  glVertex3d(width / 2, -cell_height * 1.5, 0.5);
  glVertex3d(-width / 2, -cell_height * 1.5, 0.5);
  glEnd();

  // left board
  glColor3d(body_color[0], body_color[1], body_color[2]);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 2 - offset_x, cell_height * 1.5 + offset_y, 0.5);
  glVertex3d(-width / 2, cell_height * 1.5 + offset_y, 0.5);
  glVertex3d(-width / 2, -cell_height * 1.5 - offset_y, 0.5);
  glVertex3d(-width / 2 - offset_x, -cell_height * 1.5 - offset_y, 0.5);
  glEnd();

  // right board
  glColor3d(body_color[0], body_color[1], body_color[2]);
  glBegin(GL_POLYGON);
  glVertex3d(width / 2 + offset_x, cell_height * 1.5 + offset_y, 0.5);
  glVertex3d(width / 2, cell_height * 1.5 + offset_y, 0.5);
  glVertex3d(width / 2, -cell_height * 1.5 - offset_y, 0.5);
  glVertex3d(width / 2 + offset_x, -cell_height * 1.5 - offset_y, 0.5);
  glEnd();

  // stand
  glColor3d(0.2, 0.2, 0.2);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 2 - offset_x, -cell_height * 1.5 - offset_y, 3.0);
  glVertex3d(width / 2 + offset_x, -cell_height * 1.5 - offset_y, 3.0);
  glVertex3d(width / 2 + offset_x, -cell_height * 1.5 - offset_y, 0.1);
  glVertex3d(-width / 2 - offset_x, -cell_height * 1.5 - offset_y, 0.1);
  glEnd();

  // maxbet button
  glColor3f(0.1, 0.1, 0.1);
  GLUquadric* quad_bet_button = gluNewQuadric();
  GLUquadric* quad_bet_button_top = gluNewQuadric();
  glPushMatrix();
  glTranslatef(-width / 2, -cell_height * 1.5 - offset_y, 1.5);
  glRotatef(-90, 1.0, 0.0, 0.0);
  gluCylinder(quad_bet_button, 0.7, 0.5, 0.1, 32, 32);
  glTranslatef(0.0, 0.0, 0.1);
  glColor3f(body_color[0], body_color[1], body_color[2]);
  gluDisk(quad_bet_button_top, 0.0, 0.5, 100, 1);
  glPopMatrix();
  gluDeleteQuadric(quad_bet_button);
  gluDeleteQuadric(quad_bet_button_top);

  // front
  glColor3d(silver[0], silver[1], silver[2]);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 2 - offset_x, -cell_height * 1.5 - offset_y - 3.0, 3.0);
  glVertex3d(width / 2 + offset_x, -cell_height * 1.5 - offset_y - 3.0, 3.0);
  glVertex3d(width / 2 + offset_x, -cell_height * 1.5 - offset_y, 3.0);
  glVertex3d(-width / 2 - offset_x, -cell_height * 1.5 - offset_y, 3.0);
  glEnd();

  // front button area
  glColor3d(0.0, 0.0, 0.0);
  glBegin(GL_POLYGON);
  glVertex3d(-width / 3, -cell_height * 1.5 - offset_y - 3.0, 3.01);
  glVertex3d(width / 3, -cell_height * 1.5 - offset_y - 3.0, 3.01);
  glVertex3d(width / 3, -cell_height * 1.5 - offset_y, 3.01);
  glVertex3d(-width / 3, -cell_height * 1.5 - offset_y, 3.01);
  glEnd();

  // front button
  for (int i = -1; i < 2; i++){
    glPushMatrix();
    glTranslatef(cell_width / 1.5 * i, -cell_height * 1.5 - offset_y - 1.5, 3.02);
    glColor3d(silver[0], silver[1], silver[2]);
    GLUquadric* quad = gluNewQuadric();
    gluDisk(quad, 0.0, 0.5, 100, 1);
    glPopMatrix();
  }

  // lever
  glPushMatrix();
  glColor3d(0.1, 0.1, 0.1);
  glTranslatef(-width / 2, -cell_height * 1.5 - offset_y - 1.5, 4.0);
  glutSolidSphere(0.5, 20, 20);
  glPopMatrix();

  // lever stick
  glColor3f(0.6, 0.6, 0.6);
  GLUquadric* quad = gluNewQuadric();
  glPushMatrix();
  glTranslatef(-width / 2, -cell_height * 1.5 - offset_y - 1.5, 3.0);
  gluCylinder(quad, 0.1, 0.1, 1.0, 32, 32);
  glPopMatrix();
  gluDeleteQuadric(quad);

  // gogo
  if (machine_mode == 1) {
    glPushMatrix();
    glTranslatef(0.0, -cell_height * 1.5 - offset_y / 2 , 0.6);
    glColor3d(1.0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[7]);
    glEnable(GL_TEXTURE_2D);
    GLUquadric* quad_pekari = gluNewQuadric();
    gluQuadricTexture(quad_pekari, GL_TRUE);
    gluDisk(quad_pekari, 0.0, 0.9, 100, 1);
    gluDeleteQuadric(quad_pekari);
    glDisable(GL_TEXTURE_2D);

    glColor3d(1.0, 0.843, 0.0);
    GLUquadric* quad_pekari2 = gluNewQuadric();
    gluCylinder(quad, 1.0, 0.8, 0.15, 32, 32);
    glPopMatrix();
  } else {
    glPushMatrix();
    glTranslatef(0.0, -cell_height * 1.5 - offset_y / 2 , 0.6);
    glColor3d(0.1, 0.1, 0.1);
    GLUquadric* quad_pekari = gluNewQuadric();
    gluQuadricTexture(quad_pekari, GL_TRUE);
    gluDisk(quad_pekari, 0.0, 0.9, 100, 1);

    glColor3d(1.0, 0.843, 0.0);
    GLUquadric* quad_pekari2 = gluNewQuadric();
    gluCylinder(quad, 1.0, 0.8, 0.15, 32, 32);
    glPopMatrix();
  }

  // payout display
  glColor3d(0.1, 0.1, 0.1);
  glBegin(GL_POLYGON);
  glVertex3d(- width / 2 + 0.5, -cell_height * 1.5 - offset_y + 0.4, 0.51);
  glVertex3d(- width / 2 + 1.7, -cell_height * 1.5 - offset_y + 0.4, 0.51);
  glVertex3d(- width / 2 + 1.7, -cell_height * 1.5 - offset_y + 1.6, 0.51);
  glVertex3d(- width / 2 + 0.5, -cell_height * 1.5 - offset_y + 1.6, 0.51);
  glEnd();

  // credit display
  glPushMatrix();
  glTranslatef(-1.4, 0.0, 0.0);
  glColor3d(0.1, 0.1, 0.1);
  glBegin(GL_POLYGON);
  glVertex3d(- width / 2 + 0.5, -cell_height * 1.5 - offset_y + 0.4, 0.51);
  glVertex3d(- width / 2 + 1.7, -cell_height * 1.5 - offset_y + 0.4, 0.51);
  glVertex3d(- width / 2 + 1.7, -cell_height * 1.5 - offset_y + 1.6, 0.51);
  glVertex3d(- width / 2 + 0.5, -cell_height * 1.5 - offset_y + 1.6, 0.51);
  glEnd(); 
  glPopMatrix();
}

void set_texture() {
  const char *inputFileNames[8] = {
    "img/seven.png",
    "img/bar.png",
    "img/tiger.png",
    "img/grape.png",
    "img/cherry.png",
    "img/bell.png",
    "img/clown.png",
    "img/gogo_r.png",
  };
  for (int i = 0; i < 7; i++){
    cv::Mat input = cv::imread(inputFileNames[i], 1);

    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, (TEXTURE_WIDTH - input.cols) / 2, (TEXTURE_HEIGHT - input.rows) / 2, input.cols, input.rows, GL_BGR, GL_UNSIGNED_BYTE, input.data);
  }

  cv::Mat input = cv::imread(inputFileNames[7], 1);
  glBindTexture(GL_TEXTURE_2D, g_TextureHandles[7]);
  glTexSubImage2D(GL_TEXTURE_2D, 0, (200 - input.cols) / 2, (150 - input.rows) / 2, input.cols, input.rows, GL_BGR, GL_UNSIGNED_BYTE, input.data);
}


/*
Map between the index and reel image
  0: seven
  1: bar
  2: tiger
  3: grape
  4: cherry
  5: bell
  6: clown
*/
int calc_score(int* comb){
  int score = 0;
  if (comb[0] == comb[1] && comb[1] == comb[2]){
    play_sound("sound/grape.wav");
    if (comb[0] == 2) score = 3;
    else if (comb[0] == 3) score = 14;
    else if (comb[0] == 5) score = 14;
    else if (comb[0] == 6) score = 10;
  } else if (comb[0] == 4){
    score = 7;
  }
  return score;
}

int calc_result(int* reel){
  bool is_atari = false;
  int score = 0;
  int cell[9];
  // 0 3 6
  // 1 4 7
  // 2 5 8
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      if (i == 0) cell[3 * i + j] = reel1_dict[(reel[i] + j) % 21];
      else if (i == 1) cell[3 * i + j] = reel2_dict[(reel[i] + j) % 21];
      else cell[3 * i + j] = reel3_dict[(reel[i] + j) % 21];
    }
  }

  for (int i = 0; i < 3; i++){
    // check row
    if (cell[i] == cell[i+3] && cell[i+3] == cell[i+6]){
      int comb[3] = {cell[i], cell[i+3], cell[i+6]};
      score = calc_score(comb);
      break;
    }
    // check diagonal
    if (cell[0] == cell[4] && cell[4] == cell[8]){
      int comb[3] = {cell[0], cell[4], cell[8]};
      score = calc_score(comb);
      break;
    }
    if (cell[2] == cell[4] && cell[4] == cell[6]){
      int comb[3] = {cell[2], cell[4], cell[6]};
      score = calc_score(comb);
      break;
    }
    // check cherry
    if (cell[0] == 4 || cell[1] == 4 || cell[2] == 4){
      int comb[3] = {cell[0], cell[1], cell[2]};
      score = calc_score(comb);
      break;
    }
  }
  return score;
}

void play_sound(char* filename){
  alureInitDevice(NULL, NULL);
  ALuint source;
  alGenSources(1, &source); 
  ALuint buffer = alureCreateBufferFromFile(filename);
  alSourcei(source, AL_BUFFER, buffer);
  alSourcePlay(source);
}