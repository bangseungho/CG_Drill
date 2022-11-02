#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd{0.0, 1.0};
GLchar* coord_vertexsource, * obj1_vertexsource;
GLchar* fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint coord_vertexshader, obj1_vertexshader;
GLuint fragmentshader; //--- ���̴� ��ü

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

GLuint coord_s_program;
GLuint obj1_s_program;
GLfloat mx;
GLfloat my;

struct Vec3d
{
	GLfloat _posX;
	GLfloat _posY;
	GLfloat _posZ;
};

struct Color
{
	GLfloat	_r;
	GLfloat	_g;
	GLfloat	_b;
};

struct Vertice
{
	Vec3d _pos;
	Color _color;
};

class Object
{
protected:
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_color;
	vector<Vertice> vertices; // ������ �� ��ǥ ����
	Vec3d _trans_info; // ���� �̵� ��ȯ ����
	Vec3d _rotate_info; // ���� ȸ�� ��ȯ ����
	GLfloat _scale_info; // ���� ���� ��ȯ ����
	glm::mat4 T = glm::mat4(1.0f); // �̵� ��ȯ
	glm::mat4 R = glm::mat4(1.0f); // ȸ�� ��ȯ
	glm::mat4 S = glm::mat4(1.0f); // ���� ��ȯ
	glm::mat4 SRT = glm::mat4(1.0f); // ��ȯ ���
	glm::vec4 center = glm::vec4(1.0f);
	unsigned int modelLocation;
	const char* modelTransform;
	GLuint s_program;


public:
	Object() : _trans_info{}, _rotate_info{}, _scale_info{} {

	}
	~Object() {

	}

	GLvoid push_back(Vertice vertices) {
		this->vertices.push_back({ vertices._pos, vertices._color });
	}

	GLvoid transformation() {
		SRT = S * R * T;
	}

	GLvoid translate(GLfloat x, GLfloat y, GLfloat z) {
		T = glm::translate(T, glm::vec3(x, y, z));
		_trans_info._posX += x;
		_trans_info._posY += y;
		_trans_info._posZ += z;

		transformation();
	}

	GLvoid rotate(GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
		R = glm::rotate(R, glm::radians(degree), glm::vec3(x, y, z));

		if (x == 1.0f)
			_rotate_info._posX += degree;
		if (y == 1.0f)
			_rotate_info._posY += degree;
		if (z == 1.0f)
			_rotate_info._posZ += degree;

		transformation();
	}

	GLvoid scale(GLfloat scale) {
		S = glm::scale(S, glm::vec3(scale));
		_scale_info += scale;

		transformation();
	}

	GLvoid print_info() {
		cout << "Translate : (" << _trans_info._posX << ", " << _trans_info._posY << ", " << _trans_info._posZ << ")" << endl;
		cout << "Rotate : (" << _rotate_info._posX << ", " << _rotate_info._posY << ", " << _rotate_info._posZ << ")" << endl;
		cout << "Scale : " << _scale_info << endl;
	}
};

class Line : public Object
{
public:
	GLvoid init(const char* modelTransform) {
		this->modelTransform = modelTransform;
	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo_position);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 6 * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	GLvoid draw(GLuint s_program) {
		glBindVertexArray(vao);
		glUseProgram(s_program);
		unsigned int modelLocation = glGetUniformLocation(s_program, modelTransform);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawArrays(GL_LINES, 0, 6);
	}
};

class Obj : public Object
{
	objRead objReader;
	GLint obj;
	vector<Color> colors;

public:
	GLvoid init(const char* modelTransform, const char* objfile) {
		obj = objReader.loadObj_normalize_center(objfile);
		this->modelTransform = modelTransform;
		setcolor();
	}

	GLvoid setcolor() {
		for (int i{}; i < objReader.outvertex.size(); ++i) {
			colors.push_back({ urd(dre), urd(dre), urd(dre) });
		}
	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�

		glGenBuffers(1, &vbo_position);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.outvertex.size() * sizeof(glm::vec3), &objReader.outvertex[0], GL_STATIC_DRAW);
		GLint pAttribute = glGetAttribLocation(obj1_s_program, "vPos");
		glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(pAttribute);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program, "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}


	GLvoid draw(GLuint s_program) {
		glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�
		glUseProgram(s_program);
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program, modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawArrays(GL_TRIANGLES, 0, obj);
	}
};

Line line;
Obj hexi;
Obj dragonLee;

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


GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 600;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

void make_vertexShader()
{
	coord_vertexsource = filetobuf("coord_vertex.glsl");
	obj1_vertexsource = filetobuf("obj1_vertex.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	coord_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	obj1_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(coord_vertexshader, 1, (const GLchar**)&coord_vertexsource, 0);
	glShaderSource(obj1_vertexshader, 1, (const GLchar**)&obj1_vertexsource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(coord_vertexshader);
	glCompileShader(obj1_vertexshader);
	//--- �������� ����� ���� ���� ���: ���� üũ
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(coord_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(coord_vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader ������ ����\n" << errorLog << endl;
		return;
	}

	glGetShaderiv(obj1_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(obj1_vertexshader, 512, NULL, errorLog);
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
	coord_s_program = glCreateProgram();
	obj1_s_program = glCreateProgram();
	glAttachShader(coord_s_program, coord_vertexshader);
	glAttachShader(coord_s_program, fragmentshader);
	glLinkProgram(coord_s_program);
	glAttachShader(obj1_s_program, obj1_vertexshader);
	glAttachShader(obj1_s_program, fragmentshader);
	glLinkProgram(obj1_s_program);


	glDeleteShader(coord_vertexshader);
	glDeleteShader(obj1_vertexshader);
	glDeleteShader(fragmentshader);
}

void InitBuffer()
{

	//--- line vbo
	line.set_vbo();

	//--- obj
	hexi.set_vbo();

	//--- dragonLee
	dragonLee.set_vbo();

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

glm::mat4 oS = glm::mat4(1.0f); // ���� ��ȯ

GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	line.draw(coord_s_program);

	hexi.draw(obj1_s_program);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	case 'q':
		exit(1);
		break;
	}
}

void KeyUp(int z, int x, int y)
{

}


void special(int key, int x, int y)
{

}

void TimerFunction(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1, TimerFunction, 1);
}

void Init()
{
	line.init("coord_modelTransform");
	line.push_back({ -1.0, 0.0, 0.0, 1.0, 0.0, 0.0 });
	line.push_back({ 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 });
	line.push_back({ 0.0, 1.0, 0.0, 0.0, 1.0, 0.0 });
	line.push_back({ 0.0, -1.0, 0.0, 0.0, 1.0, 0.0 });
	line.push_back({ 0.0, 0.0, -1.0, 0.0, 0.0, 1.0 });
	line.push_back({ 0.0, 0.0, 1.0, 0.0, 0.0, 1.0 });
	line.rotate(30, 1, 0, 0);
	line.rotate(30, 0, 1, 0);

	hexi.init("obj1_modelTransform", "cube.obj");
	hexi.rotate(30, 1, 0, 0);
	hexi.rotate(30, 0, 1, 0);
	hexi.scale(0.3);

	//line.scale(0.3);

}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("cg_1-16");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	//--- My_init
	Init();
	line.print_info();
	//---
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