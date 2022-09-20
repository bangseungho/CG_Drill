#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
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
	//--- ������ ����ϰ� �ݹ��Լ� ���� { //--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(100, 100); // �������� ��ġ ����
	glutInitWindowSize(800, 800); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ����(������ �̸�)
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew �ʱ�ȭ
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMouseFunc(Mouse);
	glutMainLoop(); // �̺�Ʈ ó�� ���� }
} 

GLvoid drawScene()
{
	glClearColor(r, g, b, 1.0f);
	//--- �ݹ� �Լ�: �׸��� �ݹ� �Լ� { glClearColor( 0.0f, 0.0f, 1.0f, 1.0f ); // �������� ��blue�� �� ����
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�

	glColor3f(_r, _g, _b);
	glRectf(_left, _bottom, _right, _top);
	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)
{//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
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
