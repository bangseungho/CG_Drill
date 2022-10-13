#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <random>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

using namespace std;

GLchar* vertexsource, * fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader; //--- ���̴� ��ü
GLuint VAO, VBO, VBO_line, EBO;
GLvoid Reshape(int w, int h);
const double pi = 3.14159265358979;
GLfloat mx;
GLfloat my;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
void special(int key, int x, int y);
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
uniform_real_distribution<float> cd(0, 1.0);
GLuint s_program;
bool draw_on[10]{ false };
GLfloat color[10][3];

static bool c_draw = false;
static bool p_draw = true;
static bool w_draw = false;
static bool depth_draw = false;
static int rotate_cw_x = 2;
static int rotate_cw_y = 2;
static int key_down = 0;
static int reset = false;
int cross_shape[3][2];

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading 
	if (!fptr) // Return NULL on failure 
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file 
	length = ftell(fptr); // Find out how many bytes into the file we are 
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator 
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file 
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer 
	fclose(fptr); // Close the file 
	buf[length] = 0; // Null terminator 
	return buf; // Return the buffer 
}

void make_vertexShader()
{
	vertexsource = filetobuf("vertex.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(vertexshader, 1, (const GLchar**)&vertexsource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(vertexshader);
	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment.glsl");
	//--- �����׸�Ʈ ���̴� ��ü �����
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	//--- �����׸�Ʈ ���̴� ������
	glCompileShader(fragmentshader);
	//--- �������� ����� ���� ���� ���: ������ ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader ������ ����\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- ���ؽ� ���̴� �����
	make_fragmentShader(); //--- �����׸�Ʈ ���̴� �����
	//-- shader Program
	s_program = glCreateProgram();
	glAttachShader(s_program, vertexshader);
	glAttachShader(s_program, fragmentshader);
	glLinkProgram(s_program);
	//--- ���̴� �����ϱ�
	glDeleteShader(vertexshader);
	glDeleteShader(fragmentshader);
	//--- Shader Program ����ϱ�
	glUseProgram(s_program);
}

void InitBuffer()
{
	GLfloat vertices[] = {
		-1.0, 0.0, 0.0,		1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,		0.0, 1.0, 0.0,
		0.0, -1.0, 0.0,		0.0, 1.0, 0.0,
		0.0, 0.0, -1.0,     0.0, 0.0, 1.0,
		0.0, 0.0, 1.0,      0.0, 0.0, 1.0,

		-0.2, -0.2, -0.2,	1.0, 0.0, 0.0,
		 0.2, -0.2, -0.2,	0.0, 1.0, 0.0,
		 -0.2, -0.2, 0.2,	0.0, 0.0, 1.0,
		 0.2, -0.2, 0.2,	1.0, 0.0, 1.0,

		 -0.2, 0.2, -0.2,	1.0, 1.0, 0.0,
		 0.2, 0.2, -0.2,	 0.0, 1.0, 1.0,
		 -0.2, 0.2, 0.2,	0.5, 0.0, 1.0,
		 0.2, 0.2, 0.2,		 0.0, 0.5, 1.0,


		// 0.0, 0.4, 0.0,		0.0, 0.5, 1.0,
		//-0.4, -0.4, -0.4,	1.0, 0.0, 0.0,
		// 0.4, -0.4, -0.4,	0.0, 1.0, 0.0,
		// 0.4, -0.4, 0.4,	1.0, 0.0, 1.0,
		// -0.4, -0.4, 0.4,	0.0, 0.0, 1.0,



	};

	unsigned int index[] = {
		0, 1, 
		2, 3,
		4, 5,

		6, 7, 8, // �Ʒ���
		7, 9, 8,

		10, 13, 11, // ����
		10, 12, 13,

		11, 13, 7, // ������
		7, 13, 9, 

		10, 6, 8, // ���ʸ�
		10, 8, 12, 

		8, 9, 12, // �޸�
		9, 13, 12, 

		11, 7, 6, // �ո�
		10, 11, 6,

		//================

		15, 17, 18,
		15, 16, 17,
		14, 15, 18, 
		14, 16, 15, 
		14, 17, 16, 
		14, 18, 17,
	};

	glGenVertexArrays(1, &VAO); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(VAO); //--- VAO�� ���ε��ϱ�
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

glm::mat4 R = glm::mat4(1.0f); //--- ȸ�� ��� ����
glm::mat4 T = glm::mat4(1.0f); //--- �̵� ��� ����
glm::mat4 TR = glm::mat4(1.0f); //--- �ռ� ��ȯ ���

GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!depth_draw)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);
	//--- ����� VAO �ҷ�����
	//--- �ﰢ�� �׸���
	glBindVertexArray(VAO);


	TR = R * T;

	glm::mat4 unit = glm::mat4(1.0f);


	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");

	int vColorLocation = glGetUniformLocation(s_program, "shape_color");


	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 6));
	

	if (w_draw) {
		glPolygonMode(GL_FRONT, GL_LINE);
		glPolygonMode(GL_BACK, GL_LINE);
	}
	else if(!w_draw) {
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
	}

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (button == GLUT_LEFT_BUTTON)
	{
		
	}
}

void KeyUp(int z, int x, int y)
{
	key_down = 0;
}

void Keyboard(unsigned char key, int x, int y)
{
		switch (key)
		{
		case 'c':
			if (!c_draw) {
				p_draw = false;
				c_draw = true;
			}
			break;
		case 'p':
			if (!p_draw) {
				p_draw = true;
				c_draw = false;
			}
			break;
		case 'w':
			if (!w_draw)
				w_draw = true;
			else
				w_draw = false;
			break;
		case 'h':
			if (!depth_draw)
				depth_draw = true;
			else
				depth_draw = false;
			break;
		case 'x':
			rotate_cw_y = 2;
			if (rotate_cw_x == 2 || rotate_cw_x == 1)
				rotate_cw_x = 0;
			else
				rotate_cw_x = 1;
			break;
		case 'y':
			rotate_cw_x = 2;
			if (rotate_cw_y == 2 || rotate_cw_y == 1)
				rotate_cw_y = 0;
			else
				rotate_cw_y = 1;
			break;
		case 's':
			reset = true;
			break;
		case 'q':
			exit(1);
			break;
		}


}

void special(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		key_down = 1;
	}
	if (key == GLUT_KEY_RIGHT)
	{
		key_down = 2;
	}
	if (key == GLUT_KEY_UP)
	{
		key_down = 3;
	}
	if (key == GLUT_KEY_DOWN)
	{
		key_down = 4;
	}
}

void TimerFunction(int value)
{
	static float rotate_value = 0.1;

	if(rotate_cw_x == 0)
		R = glm::rotate(R, glm::radians(rotate_value), glm::vec3(1.0, 0.0, 0.0));
	else if(rotate_cw_x == 1)
		R = glm::rotate(R, glm::radians(-rotate_value), glm::vec3(1.0, 0.0, 0.0));
	else if (rotate_cw_y == 0)
		R = glm::rotate(R, glm::radians(rotate_value), glm::vec3(0.0, 1.0, 0.0));
	else if (rotate_cw_y == 1)
		R = glm::rotate(R, glm::radians(-rotate_value), glm::vec3(0.0, 1.0, 0.0));

	if (key_down != 0)
	{
		switch (key_down) {
		case 1:
			T = glm::translate(T, glm::vec3(-0.005, 0.0, 0.0));
			break;
		case 2:
			T = glm::translate(T, glm::vec3(0.005, 0.0, 0.0));
			break;
		case 3:
			T = glm::translate(T, glm::vec3(0.0, 0.005, 0.0));
			break;
		case 4:
			T = glm::translate(T, glm::vec3(0.0, -0.005, 0.0));
			break;
		}
	}

	InitBuffer();
	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}

void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 600;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void Init()
{
	T = glm::translate(T, glm::vec3(-0.5, 0.0, 0.0));
	R = glm::rotate(R, glm::radians(-45.0f), glm::vec3(1.0, 0.0, 0.0));
	R = glm::rotate(R, glm::radians(-45.0f), glm::vec3(0.0, 1.0, 0.0));
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Example1");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	Init();
	glewInit();
	InitShader();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(special);
	glutSpecialUpFunc(KeyUp);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

