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
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
uniform_real_distribution<float> cd(0, 1.0);
GLuint s_program;
bool draw_on[10]{ false };
GLfloat color[10][3];

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
		-1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, -1.0, 0.0,

		-0.5, -0.5, -0.5, 
		 0.5, -0.5, -0.5, 
		 -0.5, -0.5, 0.5, 
		 0.5, -0.5, 0.5, 

		 -0.5, 0.5, -0.5, 
		 0.5, 0.5, -0.5, 
		 -0.5, 0.5, 0.5, 
		 0.5, 0.5, 0.5,

		 0.0, -0.3, -0.3, 
		 0.4, -0.3, 0.3, 
		 -0.4, -0.3, 0.3,
		 0.0, 0.3, 0.0,

	};

	unsigned int index[] = {
		0, 1, 
		2, 3, 

		4, 5, 6, // �Ʒ���
		5, 6, 7,

		8, 9, 10, // ����
		9, 10, 11,

		5, 7, 9, // ������
		7, 9, 11, 

		4, 6, 8, // ���ʸ�
		6, 8, 10, 

		6, 7, 10, // �޸�
		7, 10, 11, 

		4, 5, 8, // �ո�
		5, 8, 9,

		//================

		12, 13, 14, 
		15, 14, 13, 
		15, 12, 14, 
		15, 13, 12,
	};

	glGenVertexArrays(1, &VAO); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(VAO); //--- VAO�� ���ε��ϱ�
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
}

GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);
	//--- ����� VAO �ҷ�����
	//--- �ﰢ�� �׸���
	glBindVertexArray(VAO);

	glm::mat4 Rz = glm::mat4(1.0f); //--- ȸ�� ��� ����
	glm::mat4 Tx = glm::mat4(1.0f); //--- �̵� ��� ����
	glm::mat4 TR = glm::mat4(1.0f); //--- �ռ� ��ȯ ���

	Tx = glm::translate(Tx, glm::vec3(0.0, 0.0, 0.0));
	Rz = glm::rotate(Rz, glm::radians(10.0f), glm::vec3(1.0, 1.0, 0.0));

	TR = Rz * Tx;

	glm::mat4 unit = glm::mat4(1.0f);
	unsigned int modelLocation = glGetUniformLocation(s_program, "modelTransform");

	int vColorLocation = glGetUniformLocation(s_program, "shape_color");

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(unit)); // line
	glUniform3f(vColorLocation, 1.0, 0.0, 0.0);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
	glUniform3f(vColorLocation, 0.0, 1.0, 0.0);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 2));

	glPointSize(5);

	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR)); // triangle


	for (int i = 0; i < 6; ++i)
	{
		if (draw_on[i]) {
			glUniform3f(vColorLocation, color[i][0], color[i][1], color[i][2]);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (4 + 6 * i)));
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		if (draw_on[i + 6]) {
			glUniform3f(vColorLocation, color[i][0], color[i][1], color[i][2]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (40 + 3 * i)));
		}
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

void Keyboard(unsigned char key, int x, int y)
{
		switch (key)
		{
		case '1':
			if (!draw_on[0])
				draw_on[0] = true;
			else draw_on[0] = false;
			break;
		case '2':
			if (!draw_on[1])
				draw_on[1] = true;
			else draw_on[1] = false;
			break;
		case '3':
			if (!draw_on[2])
				draw_on[2] = true;
			else draw_on[2] = false;
			break;
		case '4':
			if (!draw_on[3])
				draw_on[3] = true;
			else draw_on[3] = false;
			break;
		case '5':
			if (!draw_on[4])
				draw_on[4] = true;
			else draw_on[4] = false;
			break;
		case '6':
			if (!draw_on[5])
				draw_on[5] = true;
			else draw_on[5] = false;
			break;

		case '7':
			if (!draw_on[6])
				draw_on[6] = true;
			else draw_on[6] = false;
			break;
		case '8':
			if (!draw_on[7])
				draw_on[7] = true;
			else draw_on[7] = false;
			break;
		case '9':
			if (!draw_on[8])
				draw_on[8] = true;
			else draw_on[8] = false;
			break;
		case '0':
			if (!draw_on[9])
				draw_on[9] = true;
			else draw_on[9] = false;
			break;
		case 'a':
			if (!draw_on[0])
			{
				draw_on[0] = true;
				draw_on[1] = true;
			}
			else 
			{
				draw_on[0] = false;
				draw_on[1] = false;
			}
			break;
		case 'b':
			if (!draw_on[2])
			{
				draw_on[2] = true;
				draw_on[3] = true;
			}
			else
			{
				draw_on[2] = false;
				draw_on[3] = false;
			}
			break;
		case 'c':
			if (!draw_on[4])
			{
				draw_on[4] = true;
				draw_on[5] = true;
			}
			else
			{
				draw_on[4] = false;
				draw_on[5] = false;
			}
			break;
		case 'e':
			if (!draw_on[7])
			{
				draw_on[6] = true;
				draw_on[7] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[7] = false;
			}
			break;
		case 'f':
			if (!draw_on[8])
			{
				draw_on[6] = true;
				draw_on[8] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[8] = false;
			}
			break;
		case 'g':
			if (!draw_on[9])
			{
				draw_on[6] = true;
				draw_on[9] = true;
			}
			else
			{
				draw_on[6] = false;
				draw_on[9] = false;
			}
			break;
		}
}

void TimerFunction(int value)
{

	InitBuffer();
	glutPostRedisplay();
	glutTimerFunc(100, TimerFunction, 1);
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

}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 3; ++j)
		{
			color[i][j] = cd(dre);
		}
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
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
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

