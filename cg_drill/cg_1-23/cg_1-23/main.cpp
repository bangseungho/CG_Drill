#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"
#include "camera.h"

using namespace glm;
using namespace std;

GLuint win_width = 1920;
GLuint win_height = 1000;
static int width_num;
static int height_num;
GLfloat mx;
GLfloat my;

GLfloat aspect_ratio{};

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd_color{ 0.2, 1.0 };
uniform_real_distribution<float> urd_speed{ 0.2, 1.0 };

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

Camera* camera;
Camera fps_camera(Person_View::FPS, vec3(0.0f, 0.0f, 0.0f));
Camera quarter_camera(Person_View::QUARTER, -0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, -90, 0);
Camera top_camera(Person_View::QUARTER, 0.0f, 11.0f, 0.0f, 0.0f, 1.0f, 0.0f, 90, -100);

bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = win_width / 2.0;
float lastY = win_height / 2.0;

glm::vec3 light_source(1.0, 1.0, 1.0);

enum class Keyboard_Event {
	KEYUP,

	FOWARD,
	BACKWARD,
	LEFTSIDE,
	RIGHTSIDE,

	UP,
	DOWN,
	LEFT,
	RIGHT
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

GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy)
{
	int w = 800;
	int h = 600;
	*ox = (float)(x - (float)w / 2.0) * (float)(1.0 / (float)(w / 2.0));
	*oy = -(float)(y - (float)h / 2.0) * (float)(1.0 / (float)(h / 2.0));
}

enum class ShaderType {
	vertexshader,
	fragmentshader,
};

class Shader {
	const GLchar* _source;
	ShaderType _type;
	GLuint _shader;
	GLint result;
	GLchar errorLog[512];

public:
	Shader(const GLchar* source, ShaderType type) : _shader{}, result{}, errorLog{} {
		_source = filetobuf(source);
		_type = type;
	}

	GLuint& getShader() {
		return _shader;
	}

	void make_shader() {
		switch (_type) {
		case ShaderType::vertexshader:
			_shader = glCreateShader(GL_VERTEX_SHADER);
			break;
		case ShaderType::fragmentshader:
			_shader = glCreateShader(GL_FRAGMENT_SHADER);
			break;
		}
		glShaderSource(_shader, 1, (const GLchar**)&_source, 0);
		glCompileShader(_shader);
		error_check();
	}

	void error_check() {
		glGetShaderiv(_shader, GL_COMPILE_STATUS, &result);
		if (!result)
		{
			glGetShaderInfoLog(_shader, 512, NULL, errorLog);
			cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
			return;
		}
	}

	void delete_shader() {
		glDeleteShader(_shader);
	}
};

Shader coord_v_shader{ "coord_vertex.glsl", ShaderType::vertexshader };
Shader obj1_v_shader{ "obj1_vertex.glsl", ShaderType::vertexshader };
Shader temp_f_shader{ "fragment.glsl", ShaderType::fragmentshader };
Shader light_vs{ "light_vertex.glsl", ShaderType::vertexshader };
Shader light_fs{ "light_fragment.glsl", ShaderType::fragmentshader };

class ShaderProgram {
	GLuint _s_program;

public:
	ShaderProgram() : _s_program{} {

	}

	GLuint& getSprogram() {
		return _s_program;
	}

	void make_s_program(GLint vertex, GLint fragment) {
		_s_program = glCreateProgram();

		glAttachShader(_s_program, vertex);
		glAttachShader(_s_program, fragment);
		glLinkProgram(_s_program);
	}
};

ShaderProgram obj1_s_program;
ShaderProgram coord_s_program;
ShaderProgram light_s_program;

void InitShader()
{
	coord_v_shader.make_shader();
	obj1_v_shader.make_shader();
	temp_f_shader.make_shader();
	light_vs.make_shader();
	light_fs.make_shader();

	obj1_s_program.make_s_program(obj1_v_shader.getShader(), temp_f_shader.getShader());
	coord_s_program.make_s_program(coord_v_shader.getShader(), temp_f_shader.getShader());
	light_s_program.make_s_program(light_vs.getShader(), light_fs.getShader());

	coord_v_shader.delete_shader();
	obj1_v_shader.delete_shader();
	temp_f_shader.delete_shader();
	light_vs.delete_shader();
	light_fs.delete_shader();
}

struct Color
{
	GLfloat	_r;
	GLfloat	_g;
	GLfloat	_b;
};

enum class Type {
	wall,
	load,
	past_load,
	start_point,
	end_point,
	crush_wall,
};

class Object
{
public:
	vec3 world_position;
	vec3 local_position;

	vec3 world_scale;
	vec3 local_scale;

	vec3 world_rotation;
	vec3 local_rotation;

	vec4 world_pivot;
	vec4 local_pivot;

	mat4 model;

	vector<Color> colors;

	glm::mat4 final_transform;

	glm::vec4 cur_loc;

	objRead objReader;
	GLint obj;
	unsigned int modelLocation;
	const char* modelTransform;

	//======================VAO, VBO========================//
	GLuint VAO;
	GLuint VBO_position;
	GLuint VBO_normal;
	GLuint VBO_color;
	GLuint lightCubeVAO;

public:

	Object(vec4 pivot, const char* objfile) {
		world_pivot = local_pivot = vec4(0);
		world_position = local_position = vec3(0);
		world_rotation = local_rotation = vec3(0);
		world_scale = local_scale = vec3(1);
		world_pivot = pivot;
		final_transform = mat4(1);

		obj = objReader.loadObj_normalize_center(objfile);

		this->modelTransform = "obj1_modelTransform";
		GLfloat red_color = urd_color(dre);
		GLfloat green_color = urd_color(dre);
		GLfloat blue_color = urd_color(dre);
		for (int i{}; i < obj; ++i) {
			colors.push_back({ red_color, green_color, blue_color });
		}

	}
	~Object() {

	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO_position);
		glGenBuffers(1, &VBO_normal);
		glGenBuffers(1, &VBO_color);

		glUseProgram(obj1_s_program.getSprogram());
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
		glBufferData(GL_ARRAY_BUFFER, objReader.outvertex.size() * sizeof(glm::vec3), &objReader.outvertex[0], GL_STATIC_DRAW);
		GLint pAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "vPos");
		glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(pAttribute);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_color);
		glBufferData(GL_ARRAY_BUFFER, objReader.outnormal.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
		GLint cAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_normal);
		glBufferData(GL_ARRAY_BUFFER, objReader.outnormal.size() * sizeof(glm::vec3), &objReader.outnormal[0], GL_STATIC_DRAW);
		GLint nAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "aNormal");
		glVertexAttribPointer(nAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(nAttribute);

		glGenVertexArrays(1, &lightCubeVAO);
		glBindVertexArray(lightCubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	
	void rotate(vec3 degree) {
		world_rotation.x += degree.x;
		world_rotation.y += degree.y;
		world_rotation.z += degree.z;
	}

	void translate(glm::vec3 trans) {
		world_position += trans;
	}

	void set_translate(glm::vec3 trans) {
		world_position = trans;
	}

	void scale(glm::vec3 scale) {
		world_scale = scale;
	}

	void lrotate(vec3 degree) {
		local_rotation.x += degree.x;
		local_rotation.y += degree.y;
		local_rotation.z += degree.z;
	}

	void ltranslate(glm::vec3 trans) {
		local_position += trans;
	}

	void lscale(glm::vec3 scale) {
		local_scale = scale;
	}

	void setting() {
		mat4 local_model = mat4(1.0);
		mat4 world_model = mat4(1.0);

		local_model = glm::translate(local_model, vec3(local_position));
		local_model = glm::rotate(local_model, radians(local_rotation.x), glm::vec3(1.0, 0.0, 0.0));
		local_model = glm::rotate(local_model, radians(local_rotation.y), glm::vec3(0.0, 1.0, 0.0));
		local_model = glm::rotate(local_model, radians(local_rotation.z), glm::vec3(0.0, 0.0, 1.0));
		local_model = glm::translate(local_model, vec3(local_pivot));
		local_model = glm::scale(local_model, vec3(local_scale));

		world_model = glm::translate(world_model, vec3(world_position));
		world_model = glm::rotate(world_model, radians(world_rotation.x), glm::vec3(1.0, 0.0, 0.0));
		world_model = glm::rotate(world_model, radians(world_rotation.y), glm::vec3(0.0, 1.0, 0.0));
		world_model = glm::rotate(world_model, radians(world_rotation.z), glm::vec3(0.0, 0.0, 1.0));
		world_model = glm::translate(world_model, vec3(world_pivot));
		world_model = glm::scale(world_model, vec3(world_scale));

		cur_loc = vec4(1.0);
		cur_loc = world_model * cur_loc;
		final_transform = world_model * local_model;
	}

	glm::vec4 get_cur_loc() const {
		return cur_loc;
	}

	glm::vec3 get_cur_positon() const {
		return world_position;
	}

	void print_cur_loc() const {
		cout << "y: " << local_scale.y << endl;
	}

	void reset_scale() {
	}

	GLvoid orb_draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		//glUseProgram(s_program.getSprogram());
		//glBindVertexArray(VAO);

		//unsigned int obj1_modelLocation = glGetUniformLocation(s_program.getSprogram(), modelTransform);
		//glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(final_transform));
		//glPointSize(10);
		//unsigned int viewLocation_obj1 = glGetUniformLocation(s_program.getSprogram(), "viewTransform");
		//unsigned int projLoc_obj1 = glGetUniformLocation(s_program.getSprogram(), "projection");
		//glUniformMatrix4fv(viewLocation_obj1, 1, GL_FALSE, &cam.GetViewMatrix()[0][0]);
		//glUniformMatrix4fv(projLoc_obj1, 1, GL_FALSE, &projection[0][0]);


		//glDrawArrays(GL_TRIANGLES, 0, obj);
	}

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		glUseProgram(s_program.getSprogram());
		glBindVertexArray(VAO);

		unsigned int obj1_modelLocation = glGetUniformLocation(s_program.getSprogram(), modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(final_transform));
		glPointSize(10);
		unsigned int viewLocation_obj1 = glGetUniformLocation(s_program.getSprogram(), "viewTransform");
		unsigned int projLoc_obj1 = glGetUniformLocation(s_program.getSprogram(), "projection");
		glUniformMatrix4fv(viewLocation_obj1, 1, GL_FALSE, &cam.GetViewMatrix()[0][0]);
		glUniformMatrix4fv(projLoc_obj1, 1, GL_FALSE, &projection[0][0]);


		glDrawArrays(GL_TRIANGLES, 0, obj);
	}
};

Object cube1(Object{ glm::vec4(0.0f), "Sphere.obj"});
Object cube2(Object{ glm::vec4(0.0f), "Sphere.obj"});
Object cube3(Object{ glm::vec4(0.0f), "Sphere.obj"});

Object light_box(Object{ glm::vec4(0.0f), "Cube.obj" });
Object* draw_obj;

void InitBuffer()
{
	cube1.set_vbo();
	cube2.set_vbo();
	cube3.set_vbo();
	light_box.set_vbo();

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

static bool isProjection = true;
static bool isHexi = true;
static bool isOn = true;
static bool isCubeRotation = false;
static bool isLightRotation = false;
static int isFarOrigin = 0;
static int isLightColor = 0;

GLvoid display()
{
	float currentFrame = static_cast<float>(glutGet(GLUT_ELAPSED_TIME));
	deltaTime = (currentFrame - lastFrame) / 1000;
	lastFrame = currentFrame;
	glViewport(0, 0, win_width, win_height);
	//====================set viewport======================//
	//glViewport(0, 0, win_width, win_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//=====================set cam_1, projection=============//
	glm::mat4 projection = glm::mat4(1.0f);
	unsigned int projLoc_coord = glGetUniformLocation(coord_s_program.getSprogram(), "projection");

	glm::mat4 view = camera->GetViewMatrix();

	//===================set cooordinate====================//
	if (isProjection)
		projection = glm::perspective(glm::radians(60.0f), aspect_ratio, 0.1f, 100.0f);
	else
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 100.0f);
	glUseProgram(coord_s_program.getSprogram());
	unsigned int viewLocation_coord = glGetUniformLocation(coord_s_program.getSprogram(), "viewTransform"); //--- 뷰잉 변환 설정
	glUniformMatrix4fv(viewLocation_coord, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projLoc_coord, 1, GL_FALSE, &projection[0][0]);

	//======================right===========================//
	glUseProgram(light_s_program.getSprogram());
	unsigned int lightSoruceLocation = glGetUniformLocation(light_s_program.getSprogram(), "light_source");
	glUniform3f(lightSoruceLocation, light_source.x, light_source.y, light_source.z);

	glUseProgram(obj1_s_program.getSprogram());
	unsigned int lightColorLocation = glGetUniformLocation(obj1_s_program.getSprogram(), "lightColor");
	glUniform3f(lightColorLocation, light_source.x, light_source.y, light_source.z);
	unsigned int lightPosLocation = glGetUniformLocation(obj1_s_program.getSprogram(), "lightPos");
	glUniform3f(lightPosLocation, light_box.cur_loc.x, light_box.cur_loc.y, light_box.cur_loc.z);
	unsigned int viewPosLocation = glGetUniformLocation(obj1_s_program.getSprogram(), "viewPos");
	glUniform3f(viewPosLocation, camera->Position.x, camera->Position.y, camera->Position.z);


	//======================set object======================//
	cube1.setting();
	cube1.draw(obj1_s_program, *camera, projection);	
	cube2.setting();
	cube2.draw(obj1_s_program, *camera, projection);	
	cube3.setting();
	cube3.draw(obj1_s_program, *camera, projection);

	light_box.setting();
	light_box.draw(light_s_program, *camera, projection);

	//======================set mode========================//
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	if (h == 0)
		h = 1;
	aspect_ratio = GLfloat(w) / h;
	glViewport(0, 0, w, h);
}

enum Dir {
	left,
	right,
	top,
	bottom,
};


static Keyboard_Event key_down;

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'p':
	case 'P':
		isProjection = true;
		break;
	case 'n':
	case 'N':
		break;
	case 'm':
	case 'M':
		isOn = isOn ? false : true;
		light_source = isOn ? glm::vec3(1.0f) : glm::vec3(0.3f);
		if (isOn) {
			glUseProgram(obj1_s_program.getSprogram());
			unsigned int isOnLocation = glGetUniformLocation(obj1_s_program.getSprogram(), "isOn");
			glUniform1f(isOnLocation, 1);
		}
		else {
			glUseProgram(obj1_s_program.getSprogram());
			unsigned int isOnLocation = glGetUniformLocation(obj1_s_program.getSprogram(), "isOn");
			glUniform1f(isOnLocation, 0);
		}
		break;
	case '+':
		break;
	case '-':
		break;
	case 'z':
	case 'Z':
		isFarOrigin = (isFarOrigin + 1) % 3;
		break;
	case 'y':
	case 'Y':
		isCubeRotation = isCubeRotation ? false : true;
		break;
	case 'r':
	case 'R':
		isLightRotation = isLightRotation ? false : true;
		break;
	case 'c':
	case 'C':
		isLightColor = (isLightColor + 1) % 4;
		if (isLightColor == 0)
			light_source = vec3(1.0f);
		else if (isLightColor == 1)
			light_source = vec3(urd_color(dre), urd_color(dre), urd_color(dre));
		else if (isLightColor == 2)
			light_source = vec3(urd_color(dre), urd_color(dre), urd_color(dre));
		else 
			light_source = vec3(urd_color(dre), urd_color(dre), urd_color(dre));
		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	if (isCubeRotation) draw_obj->rotate(vec3(0, 0.3, 0));

	if (isLightRotation) {
		light_box.rotate(vec3(0, 1.0f ,0));
	}

	
	if (isFarOrigin == 1) {
		vec4 from_origin = normalize(vec4(0.0f, 0.0f, 0.0f, 1.0f) - light_box.cur_loc);
		light_box.world_pivot.x += -from_origin.x / 50;
		light_box.world_pivot.z += -from_origin.z / 50;
	}
	else if (isFarOrigin == 2) {
		vec4 from_origin = normalize(vec4(0.0f, 0.0f, 0.0f, 1.0f) - light_box.cur_loc);
		light_box.world_pivot.x += from_origin.x / 50;
		light_box.world_pivot.z += from_origin.z / 50;
	}

	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
}
//void PassiveMotion(int x, int y) {
//	float xpos = static_cast<float>(x);
//	float ypos = static_cast<float>(y);
//
//	if (firstMouse)
//	{
//		lastX = xpos;
//		lastY = ypos;
//		firstMouse = false;
//	}
//
//	float xoffset = xpos - lastX;
//	float yoffset = lastY - ypos;
//
//	lastX = xpos;
//	lastY = ypos;
//
//	camera->ProcessMouseMovement(xoffset, yoffset);
//
//	if (x < 100 || x > win_width - 100) {
//		lastX = win_width / 2;
//		lastY = win_height / 2;
//		glutWarpPointer(win_width / 2, win_height / 2);
//	}
//	else if (y < 100 || y > win_height - 100) {
//		lastX = win_width / 2;
//		lastY = win_height / 2;
//		glutWarpPointer(win_width / 2, win_height / 2);
//	}
//}

void Init()
{
	camera = &quarter_camera;

	cube1.scale(vec3(1.0));
	cube2.scale(vec3(0.4));
	cube3.scale(vec3(0.2));

	cube2.world_position.x -= 3;

	cube3.world_position.x -= 4;

	light_box.scale(vec3(0.01));
	light_box.world_pivot.z += 3;
}


void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(400, 200);
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("MOUNTAIN MAGE");
	//--- GLEW 초기화하기
	glClearColor(0.0, 0.0, 0.0, 1.0f);
	glewExperimental = GL_TRUE;
	glewInit();
	Init();
	InitShader();
	InitBuffer();
	glutDisplayFunc(display);
	glutSetCursor(GLUT_CURSOR_NONE);
	//glutSpecialUpFunc(KeyUp);
	//glutMotionFunc(MouseMotion);
	//glutPassiveMotionFunc(PassiveMotion);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(10, TimerFunction, 1);
	//glutSpecialUpFunc(KeyUp);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}