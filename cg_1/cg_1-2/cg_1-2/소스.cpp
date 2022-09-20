#include <iostream>
#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void Mouse(int button, int state, int x, int y);
GLfloat r = (float)rand() / (float)RAND_MAX;
GLfloat g = (float)rand() / (float)RAND_MAX;
GLfloat b = (float)rand() / (float)RAND_MAX;
GLfloat _r = (float)rand() / (float)RAND_MAX;
GLfloat _g = (float)rand() / (float)RAND_MAX;
GLfloat _b = (float)rand() / (float)RAND_MAX;
GLfloat _left = -0.5f;
GLfloat _bottom = -0.5f;
GLfloat _top = 0.5f;
GLfloat _right = 0.5f;
GLfloat _windowsize = 800;
bool OnTimer = false;

void main(int argc, char** argv)
{
	srand(static_cast<unsigned int>(time(nullptr)));
	//--- 윈도우 출력하고 콜백함수 설정 { //--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(800, 800); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성(윈도우 이름)
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutMouseFunc(Mouse);
	glutMainLoop(); // 이벤트 처리 시작 }
} 

GLvoid drawScene()
{
	glClearColor(r, g, b, 1.0f);
	//--- 콜백 함수: 그리기 콜백 함수 { glClearColor( 0.0f, 0.0f, 1.0f, 1.0f ); // 바탕색을 ‘blue’ 로 지정
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다

	glColor3f(_r, _g, _b);
	glRectf(_left, _bottom, _right, _top);
	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h)
{//--- 콜백 함수: 다시 그리기 콜백 함수 {
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (_left <= -(1 - x/400.0f) && _right >= - (1 - x / 400.0f) && _top >= (1 - y / 400.0f) && _bottom <= (1 - y / 400.0f))
		{
			_r = (float)rand() / (float)RAND_MAX;
			_g = (float)rand() / (float)RAND_MAX;
			_b = (float)rand() / (float)RAND_MAX;
		}
		else
		{
			r = (float)rand() / (float)RAND_MAX;
			g = (float)rand() / (float)RAND_MAX;
			b = (float)rand() / (float)RAND_MAX;
		}
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (_left <= -(1 - x / 400.0f) && _right >= -(1 - x / 400.0f) && _top >= (1 - y / 400.0f) && _bottom <= (1 - y / 400.0f))
		{
			_left += -0.05f;
			_bottom += -0.05f;
			_top += 0.05f;
			_right += 0.05f;
		}
		else
		{
			_left += 0.05f;
			_bottom += 0.05f;
			_top += -0.05f;
			_right += -0.05f;
		}
	}
}
