#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd{ 0.0, 1.0 };
GLchar* coord_vertexsource, * obj1_vertexsource, * camera_vertexsource;
GLchar* fragmentsource; //--- �ҽ��ڵ� ���� ����
GLuint coord_vertexshader, obj1_vertexshader, camera_vertexshader;
GLuint fragmentshader; //--- ���̴� ��ü

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

GLuint coord_s_program;
GLuint obj1_s_program;
GLuint camera_s_program;
GLfloat mx;
GLfloat my;

static bool projection_type = true;
static bool model_type;
static bool depth_draw;
static int y_rotate;

enum {
	front,
	left,
	back,
	right,
	up,
	down,
};

enum class Shape {
	hexi,
	quad,
};

static Shape shape = Shape::quad;

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
	glm::vec4 center = glm::vec4(1.0f);
	unsigned int modelLocation;
	const char* modelTransform;
	GLuint s_program;


public:
	glm::mat4 T = glm::mat4(1.0f); // �̵� ��ȯ
	glm::mat4 R = glm::mat4(1.0f); // ȸ�� ��ȯ
	glm::mat4 S = glm::mat4(1.0f); // ���� ��ȯ
	glm::mat4 nTR = glm::mat4(1.0f); // ���� ��ȯ
	glm::mat4 SRT = glm::mat4(1.0f); // ��ȯ ���
	glm::mat4 NT = glm::mat4(1.0f); // ��ȯ ���
	glm::mat4 NR = glm::mat4(1.0f); // ��ȯ ���
	glm::mat4 nnt = glm::mat4(1.0f); // ��ȯ ���

	Object() : _trans_info{}, _rotate_info{}, _scale_info{} {

	}
	~Object() {

	}

	Vec3d& get_translate() { return _trans_info; };
	Vec3d& get_rotate() { return _rotate_info; };
	GLfloat& get_sacle() { return _scale_info; };

	GLvoid push_back(Vertice vertices) {
		this->vertices.push_back({ vertices._pos, vertices._color });
	}

	GLvoid translate(GLfloat x, GLfloat y, GLfloat z) {
		T = glm::translate(T, glm::vec3(x, y, z));
		_trans_info._posX += x;
		_trans_info._posY += y;
		_trans_info._posZ += z;
	}

	GLvoid rotate(GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
		R = glm::rotate(R, glm::radians(degree), glm::vec3(x, y, z));

		if (x == 1.0f)
			_rotate_info._posX += degree;
		if (y == 1.0f)
			_rotate_info._posY += degree;
		if (z == 1.0f)
			_rotate_info._posZ += degree;
	}

	GLvoid scale(GLfloat scale) {
		S = glm::scale(S, glm::vec3(scale));
		_scale_info += scale;
	}

	GLvoid print_info() {

	}
};

class Line : public Object
{
public:
	glm::mat4 nR = glm::mat4(1.0f); // ��ȯ ���
	glm::mat4 nnR = glm::mat4(1.0f); // ��ȯ ���

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
		SRT = T * R * S;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawArrays(GL_LINES, 0, 6);
	}

	GLvoid orb_draw(GLuint s_program) {
		glBindVertexArray(vao);
		glUseProgram(s_program);
		unsigned int modelLocation = glGetUniformLocation(s_program, modelTransform);
		SRT = nnR * R * nR * T  * S;
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glPointSize(10);
		glDrawArrays(GL_LINE_STRIP, 0, 360);
	}
};

enum class P_type {
	center,
	sub,
	slope_sub
};

class Obj : public Object
{
	objRead objReader;
	GLint obj;
	GLuint ebo;
	vector<Color> colors;
	vector<GLfloat> color;
	GLint orb_num;
	P_type type;

public:
	Line* orbit;
	Obj(P_type type, GLint orb_num_value) : type{ type }, orb_num{ orb_num_value } {};
	~Obj() {
		delete[] orbit;
	}

	GLvoid all_translate(GLfloat x, GLfloat y, GLfloat z) {
		orbit->translate(x, y, z);
		this->translate(x, y, z);
	}

	GLvoid all_rotate(GLfloat degree, GLfloat x, GLfloat y, GLfloat z) {
		orbit->nR = glm::rotate(orbit->nR, glm::radians(degree), glm::vec3(x, y, z));
	}


	GLvoid init(const char* modelTransform, const char* objfile) {
		obj = objReader.loadObj_normalize_center(objfile);
		this->modelTransform = modelTransform;
		orbit = new Line[orb_num];
		for (int i = 0; i < orb_num; ++i) {
			orbit[i].init(modelTransform);

			if (type == P_type::center)
				orbit[i].scale(0.8);
			else
				orbit[i].scale(0.2);
		}

		if (orb_num == 3) {
			orbit[1].rotate(45, 0, 0, 1);
			orbit[2].rotate(-45, 0, 0, 1);
		}

		for (int i = 0; i < 3; ++i) color.push_back(urd(dre));

		for (int i = 0; i < 360; ++i) {
			GLfloat x = 1 * sin(i / 360.0 * 2 * 3.1415926535);
			GLfloat z = 1 * cos(i / 360.0 * 2 * 3.1415926535);
			for (int i = 0; i < orb_num; ++i) {
				orbit[i].push_back({ x, 0, z, 0, 0, 0 });
			}
		}

		setcolor();
	}

	GLvoid setcolor() {
		if (type == P_type::center) {
			for (int i{}; i < objReader.nr_outvertex.size(); ++i) {
				colors.push_back({ urd(dre), urd(dre), urd(dre) });
			}
		}
		else
			for (int i{}; i < objReader.nr_outvertex.size(); ++i) {
				colors.push_back({ color[0], color[1], color[2] });
			}
	}

	GLvoid set_vbo() {
		for (int i = 0; i < orb_num; ++i) {
			orbit[i].set_vbo();
		}
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�

		glGenBuffers(1, &ebo);
		glGenBuffers(1, &vbo_position);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &objReader.nr_outvertex[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, objReader.vertexIndices.size() * sizeof(glm::vec3), &objReader.vertexIndices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_color);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.nr_outvertex.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program, "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}

	GLvoid draw(GLuint s_program) {
		for (int i = 0; i < orb_num; ++i) {
			orbit[i].orb_draw(s_program);
		}
		glUseProgram(s_program);
		glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program, modelTransform);
		if (type == P_type::slope_sub) {
			SRT = R * orbit->nR * T * nnt * NR * NT * S;
		}
		else
			SRT = orbit->nR * T * R  * S;
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glDrawElements(GL_TRIANGLES, obj, GL_UNSIGNED_INT, 0);
	}
};

//class Camera {
//public:	
//	glm::vec3 cameraPos;
//	glm::vec3 cameraTarget;
//	glm::vec3 cameraDirection;
//	glm::vec3 up;
//	glm::vec3 cameraRight;
//	glm::vec3 cameraUp;
//
//public:
//	Camera() {
//		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
//		cameraDirection = glm::normalize(cameraPos - cameraTarget);
//		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
//		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
//		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
//	}
//
//	GLvoid set_cameraPos(glm::vec3 value) {
//		cameraPos = glm::vec3(value.x, value.y, value.z);
//	}
//	GLvoid set_cameraDir(glm::vec3 value) {
//		cameraDirection = glm::vec3(value.x, value.y, value.z);
//	}
//	GLvoid set_cameraUp(glm::vec3 value) {
//		cameraUp = glm::vec3(value.x, value.y, value.z);
//	}
//
//	GLvoid draw(GLuint s_program) {
//		glUseProgram(s_program);
//
//		set_cameraPos(glm::vec3(0.0, 0.0, 0.5));
//		set_cameraDir(glm::vec3(0.0, 0.0, 0.0));
//		set_cameraUp(glm::vec3(0.0, 1.0, 0.0));
//		glm::mat4 view = glm::mat4(1.0f);
//
//		view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
//		unsigned int viewLocation = glGetUniformLocation(s_program, "viewTransform");
//		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
//	}
//};

Line line;
Obj center_sphere{ P_type::center, 3 };
Obj* sub_sphere[3];
Obj* two_sub_sphere[3];
Obj* quad;
//Camera cam;

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
	camera_vertexsource = filetobuf("camera_vertex.glsl");
	//--- ���ؽ� ���̴� ��ü �����
	coord_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	obj1_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	camera_vertexshader = glCreateShader(GL_VERTEX_SHADER);
	//--- ���̴� �ڵ带 ���̴� ��ü�� �ֱ�
	glShaderSource(coord_vertexshader, 1, (const GLchar**)&coord_vertexsource, 0);
	glShaderSource(obj1_vertexshader, 1, (const GLchar**)&obj1_vertexsource, 0);
	glShaderSource(camera_vertexshader, 1, (const GLchar**)&camera_vertexsource, 0);
	//--- ���ؽ� ���̴� �������ϱ�
	glCompileShader(coord_vertexshader);
	glCompileShader(obj1_vertexshader);
	glCompileShader(camera_vertexshader);
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

	glGetShaderiv(camera_vertexshader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(camera_vertexshader, 512, NULL, errorLog);
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
	camera_s_program = glCreateProgram();
	glAttachShader(coord_s_program, coord_vertexshader);
	glAttachShader(coord_s_program, fragmentshader);
	glLinkProgram(coord_s_program);
	glAttachShader(obj1_s_program, obj1_vertexshader);
	glAttachShader(obj1_s_program, fragmentshader);
	glLinkProgram(obj1_s_program);
	glAttachShader(camera_s_program, camera_vertexshader);
	glLinkProgram(camera_s_program);

	glDeleteShader(coord_vertexshader);
	glDeleteShader(obj1_vertexshader);
	glDeleteShader(camera_vertexshader);
	glDeleteShader(fragmentshader);
}

void InitBuffer()
{
	//--- line vbo
	line.set_vbo();

	//--- obj
	center_sphere.set_vbo();
	for (int i = 0; i < 3; ++i) {
		sub_sphere[i]->set_vbo();
		two_sub_sphere[i]->set_vbo();
	}

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

glm::vec3 cameraPos = glm::vec3(0.0f, 0.2f, -2.0f);		 //--- ī�޶� ��ġ
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- ī�޶� �ٶ󺸴� ����
glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
GLfloat cnt = 0;

GLvoid drawScene()
{
	if (!depth_draw)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	//--- ����� ���� ����
	glClearColor(1.0, 1.0, 1.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//--- camera
	//cam.draw(camera_s_program);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);		 //--- ī�޶� ���� ����
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	unsigned int viewLocation_obj = glGetUniformLocation(obj1_s_program, "viewTransform"); //--- ���� ��ȯ ����
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program, "viewTransform"); //--- ���� ��ȯ ����
	view = glm::rotate(view, glm::radians(cnt), glm::vec3(0, 1, 0));

	if (!projection_type)
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);
	else
		projection = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);

	unsigned int projLoc_obj = glGetUniformLocation(obj1_s_program, "projection");
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program, "projection");

	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);

	//line.draw(coord_s_program);

	glUniformMatrix4fv(viewLocation_obj, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_obj, 1, GL_FALSE, &projection[0][0]);

	center_sphere.draw(obj1_s_program);

	for (int i = 0; i < 3; ++i) {
		sub_sphere[i]->draw(obj1_s_program);
		two_sub_sphere[i]->draw(obj1_s_program);
	}

	model_type ? glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) : glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
	case 'p':
		projection_type = projection_type ? false : true;
		break;
	case 'h':
		depth_draw = depth_draw ? false : true;
		break;
	case 'm':
		model_type = model_type ? false : true;
		break;
	case 'y':
		y_rotate += 1;
		if (y_rotate == 3) y_rotate = 0;
		break;
	case 'a':
		cameraPos.x -= 0.03;
		cameraDirection.x -= 0.03;
		break;
	case 'd':
		cameraPos.x += 0.03;
		cameraDirection.x += 0.03;
		break;
	case 'w':
		cameraPos.y -= 0.03;
		cameraDirection.y -= 0.03;
		break;
	case 's':
		cameraPos.y += 0.03;
		cameraDirection.y += 0.03;
		break;
	case 'x':
		cameraPos.z -= 0.03;
		cameraDirection.z -= 0.03;
		break;
	case 'z':
		cameraPos.z += 0.03;
		cameraDirection.z += 0.03;
		break;
	case 'q':
		exit(1);
		break;
	}
	glutPostRedisplay();
}

void KeyUp(int z, int x, int y)
{

}


void special(int key, int x, int y)
{

}

void TimerFunction(int value)
{
	sub_sphere[0]->all_rotate(0.5f, 0, 1, 0);
	two_sub_sphere[0]->NR = glm::rotate(two_sub_sphere[0]->NR, glm::radians(2.0f), glm::vec3(0, 1, 0));
	two_sub_sphere[0]->rotate(0.5f, 0, 1, 0);

	two_sub_sphere[1]->NR = glm::rotate(two_sub_sphere[1]->NR, glm::radians(2.0f), glm::vec3(0, 1, 0));
	two_sub_sphere[2]->NR = glm::rotate(two_sub_sphere[2]->NR, glm::radians(3.0f), glm::vec3(0, 1, 0));
	//two_sub_sphere[1]->nnt = glm::rotate(two_sub_sphere[1]->nnt, glm::radians(0.2f), glm::vec3(0, 1, 0));
	sub_sphere[1]->all_rotate(0.2f, 0, 1, 0);
	two_sub_sphere[1]->all_rotate(0.2f, 0, 1, 0);
	sub_sphere[2]->all_rotate(0.7f, 0, 1, 0);
	two_sub_sphere[2]->all_rotate(0.7f, 0, 1, 0);

	if (y_rotate == 1) cnt += 1.0;
	else if (y_rotate == 2) cnt -= 1.0;

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
	line.scale(3.0);

	center_sphere.init("obj1_modelTransform", "sphere.obj");
	center_sphere.scale(0.25);

	for (int i = 0; i < 3; ++i) {
		if (i == 0)
			sub_sphere[i] = new Obj{ P_type::sub, 1 };
		else
			sub_sphere[i] = new Obj{ P_type::slope_sub, 1 };
		two_sub_sphere[i] = new Obj{ P_type::slope_sub, 1};
		sub_sphere[i]->init("obj1_modelTransform", "sphere.obj");
		sub_sphere[i]->scale(0.1);
		two_sub_sphere[i]->init("obj1_modelTransform", "sphere.obj");
		two_sub_sphere[i]->scale(0.04f);
	}

	sub_sphere[0]->all_translate(0.8, 0, 0);

	sub_sphere[1]->rotate(45, 0, 0, 1);
	sub_sphere[1]->orbit->rotate(45, 0, 0, 1);
	sub_sphere[1]->all_translate(0.8, 0, 0);

	two_sub_sphere[1]->rotate(45, 0, 0, 1);
	two_sub_sphere[1]->translate(0.8, 0, 0);

	two_sub_sphere[2]->rotate(-45, 0, 0, 1);
	two_sub_sphere[2]->translate(-0.8, 0, 0);

	sub_sphere[2]->rotate(-45, 0, 0, 1);
	sub_sphere[2]->orbit->rotate(-45, 0, 0, 1);
	sub_sphere[2]->all_translate(-0.8, 0, 0);

	two_sub_sphere[0]->NT = glm::translate(two_sub_sphere[0]->NT, glm::vec3(0.2, 0, 0));
	two_sub_sphere[0]->T = glm::translate(two_sub_sphere[0]->T, glm::vec3(0.8, 0, 0));
	two_sub_sphere[1]->NT = glm::translate(two_sub_sphere[1]->NT, glm::vec3(0.2, 0, 0));
	two_sub_sphere[2]->NT = glm::translate(two_sub_sphere[2]->NT, glm::vec3(0.2, 0, 0));

	cout << "===============================" << endl;
	cout << "p/P: ���� ����/���� ����" << endl;
	cout << "m/M: �ָ��� ��/���̾� ��" << endl;
	cout << "w/a/s/d: ���� �������� ��/��/��/�Ϸ� �̵�" << endl;
	cout << "z/x: ���� �������� ��/�ڷ� �̵�" << endl;
	cout << "y/Y: ��ü ��ü���� y������ ��/�� �������� ȸ��" << endl;
	cout << "===============================" << endl;
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