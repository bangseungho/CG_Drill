#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <gl/glew.h> // �ʿ��� ������� include
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <random>
using namespace std;

GLchar* vertexsource, * fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint vertexshader, fragmentshader; //--- ���̴� ��ü
GLuint VAO, VBO[3], EBO;
GLvoid Reshape(int w, int h);
const double pi = 3.14159265358979;
GLfloat mx;
GLfloat my;
void convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);
random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> d(-1.0, 1.0);
GLuint s_program;
bool left_button_vertex = false;
bool left_button_inside = false;
int num;
GLfloat movex;
GLfloat movey;

struct vector2
{
	float x, y;
};

typedef vector<vector2> polygon;

polygon p;

bool isInside(vector2 B, const polygon& p) {    

	int crosses = 0;   
	for(int i = 0 ; i < p.size() ; i++)
	{        
		int j = (i+1)%p.size();     
		if((p[i].y > B.y) != (p[j].y > B.y) ){          
			double atX = (p[j].x- p[i].x)*(B.y-p[i].y)/(p[j].y-p[i].y)+p[i].x;   
			if(B.x < atX)                
				crosses++;        
		}    
	}    
	return crosses % 2 > 0;
}


const GLfloat colors[] = {
	1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 1.0, 1.0,
};

GLfloat points[]= {
	-0.5,  0.5, 0.0,
	 0.5,  0.5, 0.0,
	 0.5, -0.5, 0.0,
	-0.5, -0.5, 0.0,

	-1.0,  0.0, 0.0,
	 1.0,  0.0, 0.0,
	 0.0, -1.0, 0.0,
	 0.0,  1.0, 0.0
};

unsigned int index[] = {
	0, 1, 2, 
	1, 2, 3,

	4, 5, 
	6, 7,
};

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

void InitBuffer()
{
	glGenVertexArrays(1, &VAO); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(VAO); //--- VAO�� ���ε��ϱ�

	glGenBuffers(2, VBO); //--- 1���� VBO�� �����ϰ� �Ҵ��ϱ�

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

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

GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(1.0, 1.0, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(s_program);
	//--- ����� VAO �ҷ�����
	glBindVertexArray(VAO);
	//--- �ﰢ�� �׸���

	int movelocation = glGetUniformLocation(s_program, "in_move");
	glUseProgram(s_program);
	glUniform2f(movelocation, movex, movey);

	glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
	glDrawElements(GL_LINES, 9, GL_UNSIGNED_INT, (void*)(sizeof(int) * 6));

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLfloat lgap{ 0.0f };
GLfloat bgap{ 0.0f };
GLfloat rgap{ 0.0f };
GLfloat tgap{ 0.0f };
GLfloat lgap2{ 0.0f };
GLfloat bgap2{ 0.0f };
GLfloat rgap2{ 0.0f };
GLfloat tgap2{ 0.0f };
void Mouse(int button, int state, int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);
	left_button_vertex = false;
	left_button_inside = false;

	if (button == GLUT_LEFT_BUTTON)
	{
		vector2 v;

		for (int i = 0; i < 4; ++i)
		{
			v.x = points[3 * i];
			v.y = points[3 * i + 1];
			p.push_back(v);
		}
		
		v.x = mx;
		v.y = my;

		for (int i{ 0 }; i < 4; ++i)
		{
			if (mx > points[3 * i] - 0.02 && mx < points[3 * i] + 0.02 &&
				my > points[3 * i + 1] - 0.02 && my < points[3 * i + 1] + 0.02)
			{
				left_button_vertex = true;

				num = i;
				break;
			}
		}

		if (isInside(v, p))
		{
			left_button_inside = true;

			lgap = mx - points[0];
			rgap = points[3] - mx;
			bgap = my - points[4];
			tgap = points[1] - my;

			lgap2 = points[9] - mx;
			rgap2 = mx - points[6];
			bgap2 = my - points[7];
			tgap2 = my - points[10];
		}

	}
}

void Motion(int x, int y)
{
	convertDeviceXYOpenGlXY(x, y, &mx, &my);

	if (left_button_vertex == true)
	{
		points[3 * num] = mx;
		points[3 * num + 1] = my;
	}

	else if (left_button_inside == true)
	{
		points[0] = mx - abs(lgap);
		points[1] = my + abs(tgap);

		points[3] = mx + abs(rgap);
		points[4] = my + abs(bgap);

		points[6] = mx + abs(rgap2);
		points[7] = my - abs(bgap2);

		points[9] = mx - abs(lgap2);
		points[10] = my -abs(tgap2);
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
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
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
	glutMotionFunc(Motion);
	glutTimerFunc(100, TimerFunction, 1);
	glutMainLoop();
}

