#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

GLchar* coord_vertexsource, * fragmentsource; //--- 소스코드 저장 변수
GLuint coord_vertexshader, fragmentshader; //--- 세이더 객체

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

GLuint VAO[3];
GLuint VBO_position[3];
GLuint VBO_normal[3];
GLuint VBO_color[3];
objRead objReader;
GLint obj = objReader.loadObj_normalize_center("cube.obj");
GLuint coord_s_program;
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
	vector<Vertice> vertices; // 꼭짓점 모델 좌표 정보
	Vec3d _trans_info; // 현재 이동 변환 정보
	Vec3d _rotate_info; // 현재 회전 변환 정보
	GLfloat _scale_info; // 현재 신축 변환 정보
	glm::mat4 T = glm::mat4(1.0f); // 이동 변환
	glm::mat4 R = glm::mat4(1.0f); // 회전 변환
	glm::mat4 S = glm::mat4(1.0f); // 신축 변환
	glm::mat4 SRT = glm::mat4(1.0f); // 변환 행렬
	glm::vec4 center = glm::vec4(1.0f);
	unsigned int modelLocation;
	const char* modelTransform;
	GLuint s_program;


public:
	Object() : _trans_info{}, _rotate_info{}, _scale_info{} {

	}
	~Object() {

	}

	GLvoid init(const char* modelTransform) {
		this->modelTransform = modelTransform;
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
	GLuint vbo;

public:
	GLvoid set_vbo() {
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * 6 * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
	}

	GLvoid draw(GLuint s_program) {
		glUseProgram(s_program);
		unsigned int modelLocation = glGetUniformLocation(s_program, modelTransform);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawArrays(GL_LINES, 0, 6);
	}
};

Line line;

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
	//--- 버텍스 세이더 객체 만들기
	coord_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(coord_vertexshader, 1, (const GLchar**)&coord_vertexsource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(coord_vertexshader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(coord_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(coord_vertexshader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void make_fragmentShader()
{
	fragmentsource = filetobuf("fragment.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentshader, 1, (const GLchar**)&fragmentsource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentshader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentshader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
		return;
	}
}

void InitShader()
{
	make_vertexShader(); //--- 버텍스 세이더 만들기
	make_fragmentShader(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	coord_s_program = glCreateProgram();
	glAttachShader(coord_s_program, coord_vertexshader);
	glAttachShader(coord_s_program, fragmentshader);
	glLinkProgram(coord_s_program);

	glDeleteShader(coord_vertexshader);
	glDeleteShader(fragmentshader);
}

void InitBuffer()
{
	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO_position);
	glGenBuffers(3, VBO_normal);
	glGenBuffers(3, VBO_color);

	glBindVertexArray(VAO[0]); //--- VAO를 바인드하기

	//--- line vbo
	line.set_vbo();

	//--- obj
	glBindVertexArray(VAO[1]); //--- VAO를 바인드하기
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position[1]);
	glBufferData(GL_ARRAY_BUFFER, objReader.outvertex.size() * sizeof(glm::vec3), &objReader.outvertex[0], GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(coord_s_program, "vPos");
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_normal[1]);
	glBufferData(GL_ARRAY_BUFFER, objReader.outnormal.size() * sizeof(glm::vec3), &objReader.outnormal[0], GL_STATIC_DRAW);
	GLint nAttribute = glGetAttribLocation(coord_s_program, "aNormal");
	glVertexAttribPointer(nAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(nAttribute);


	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

glm::mat4 oS = glm::mat4(1.0f); // 신축 변환

GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(0.1, 0.1, 0.1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(VAO[0]);

	line.draw(coord_s_program);

	glBindVertexArray(VAO[1]);
	

	glDrawArrays(GL_TRIANGLES, 0, obj);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
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
	line.scale(0.3);

}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	glutCreateWindow("cg_1-16");
	//--- GLEW 초기화하기
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