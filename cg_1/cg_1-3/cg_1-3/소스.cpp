#include <iostream>
#include <gl/glew.h> // 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);

const int WindowWidth = 800;
const int WindowHeight = 800;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
void Motion(int x, int y);
void Mouse(int button, int state, int x, int y);

class Rec {
public:
	struct Color {
		float _r;
		float _g;
		float _b;
	};

	struct Position {
		GLfloat _left;
		GLfloat _bottom;
		GLfloat _right;
		GLfloat _top;
	};

	Color _color;
	Position _pos;

public:
	Rec() : _color{ (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX }, _pos{ -0.5f, -0.5f, 0.5f, 0.5f }{};
	void Move(GLfloat left, GLfloat bottom, GLfloat right, GLfloat top) { _pos._left = left, _pos._bottom = bottom, _pos._right = right, _pos._top = top; }
};

Rec* r[5];
static int index{ 0 };
static int rec_num{ 0 };
static bool left_button{false};

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	for (int i{ 0 }; i < 5; ++i) r[i] = new Rec();

	srand(static_cast<unsigned int>(time(nullptr)));
	//--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(WindowWidth, WindowHeight); // 윈도우의 크기 지정
	glutCreateWindow("cg_1-3"); // 윈도우 생성(윈도우 이름)
	glewExperimental = GL_TRUE; //--- GLEW 초기화하기
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";

	glutDisplayFunc(drawScene); // 출력 콜백함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 콜백함수 지정
	glutKeyboardFunc(Keyboard); // 키보드 입력 콜백함수 지정
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutMainLoop(); // 이벤트 처리 시작
} 

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	//--- 변경된 배경색 설정
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //--- 바탕색을 변경
	glClear(GL_COLOR_BUFFER_BIT); //--- 설정된 색으로 전체를 칠하기

	for (int i{ 0 }; i < rec_num + 1; ++i)
	{
		if (rec_num > 4) rec_num = 4;
		glColor3f(r[i]->_color._r, r[i]->_color._g, r[i]->_color._b);
		glRectf(r[i]->_pos._left, r[i]->_pos._bottom, r[i]->_pos._right, r[i]->_pos._top);
	}


	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'A':
	case 'a':
		index++;
		rec_num++;
		if (index > 4)
		{
			rec_num = 4;
			index = 4;
		}
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLfloat mx;
GLfloat my;

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = WindowWidth;
	int h = WindowHeight;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

GLfloat lgap{ 0.0f };
GLfloat bgap{ 0.0f };
GLfloat rgap{ 0.0f };
GLfloat tgap{ 0.0f };

void Mouse(int button, int state, int x, int y)
{
	// 윈도우 좌표에서 OpenGL좌표로 변환
	
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	left_button = false;

	if (button == GLUT_LEFT_BUTTON)
	{
		for (int i = rec_num; i >= 0; --i)
		{
			if (mx >= r[i]->_pos._left && mx <= r[i]->_pos._right &&
				my >= r[i]->_pos._bottom && my <= r[i]->_pos._top)
			{
				index = i;
				i = 0;

				lgap = mx - r[index]->_pos._left;
				bgap = my - r[index]->_pos._bottom;
				rgap = r[index]->_pos._right - mx;
				tgap = r[index]->_pos._top - my;
				left_button = true;
			}
		}

	}
}

void Motion(int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (left_button == true)
	{
		r[index]->Move(mx - lgap, my - bgap, mx + rgap, my + tgap);
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}
