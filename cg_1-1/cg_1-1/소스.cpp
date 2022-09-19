#include <iostream>
#include <gl/glew.h> //--- �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void TimerFunction(int value);
float r = 1.0f;
float g = 1.0f;
float b = 1.0f;
bool OnTimer = false;

void main(int argc, char** argv)
{
	srand(static_cast<unsigned int>(time(nullptr)));
	//--- ������ ����ϰ� �ݹ��Լ� ���� { //--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(100, 100); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
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
	glutKeyboardFunc(Keyboard);
	glutMainLoop(); // �̺�Ʈ ó�� ���� }
}


GLvoid drawScene()
{
	glClearColor(r, g, b, 1.0f);
	//--- �ݹ� �Լ�: �׸��� �ݹ� �Լ� { glClearColor( 0.0f, 0.0f, 1.0f, 1.0f ); // �������� ��blue�� �� ����
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h)
{//--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ� {
glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'R':
	case 'r':
		r = 1.0f, g = 0.0f, b = 0.0f;
		break;
	case 'G':
	case 'g':
		r = 0.0f, g = 1.0f, b = 0.0f;
		break;
	case 'B':
	case 'b':
		r = 0.0f, g = 0.0f, b = 1.0f;
		break;
	case 'A':
	case 'a':
		r = (float)rand() / (float)RAND_MAX;
		g = (float)rand() / (float)RAND_MAX;
		b = (float)rand() / (float)RAND_MAX;
		break;
	case 'W':
	case 'w':
		r = 1.0f, g = 1.0f, b = 1.0f;
		break;
	case 'K':
	case 'k':
		r = 0.0f, g = 0.0f, b = 0.0f;
		break;
	case 'T':
	case 't':
		OnTimer = true;
		glutTimerFunc(100, TimerFunction, 1);
		break;
	case 'S':
	case 's':
		OnTimer = false;
		break;
	case 'Q':
	case 'q':
		exit(1);
		break;
	}

	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

void TimerFunction(int value)
{
	if (OnTimer)
	{
		r = (float)rand() / (float)RAND_MAX;
		g = (float)rand() / (float)RAND_MAX;
		b = (float)rand() / (float)RAND_MAX;

		glutPostRedisplay(); // ȭ�� �� ���
		glutTimerFunc(500, TimerFunction, 1); // Ÿ�̸��Լ� �� ����
	}
}