#define STB_IMAGE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"
#include "camera.h"
#include "stb_image.h"

using namespace glm;
using namespace std;

GLuint win_width = 1280;
GLuint win_height = 960;
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
Camera quarter_camera(Person_View::QUARTER, -5.0f, 5.0f, 5.0f, 0.0f, 1.0f, 0.0f, -90, -90);

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
Shader bg_v_shader{ "background_vertex.glsl", ShaderType::vertexshader };

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
ShaderProgram bg_s_program;

void InitShader()
{
	coord_v_shader.make_shader();
	obj1_v_shader.make_shader();
	temp_f_shader.make_shader();
	light_vs.make_shader();
	light_fs.make_shader();
	bg_v_shader.make_shader();

	obj1_s_program.make_s_program(obj1_v_shader.getShader(), temp_f_shader.getShader());
	coord_s_program.make_s_program(coord_v_shader.getShader(), temp_f_shader.getShader());
	light_s_program.make_s_program(light_vs.getShader(), light_fs.getShader());
	bg_s_program.make_s_program(bg_v_shader.getShader(), temp_f_shader.getShader());

	coord_v_shader.delete_shader();
	obj1_v_shader.delete_shader();
	temp_f_shader.delete_shader();
	light_vs.delete_shader();
	light_fs.delete_shader();
	bg_v_shader.delete_shader();
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
	const char* image_file;

	//======================VAO, VBO========================//
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint VBO_position;
	GLuint VBO_normal;
	GLuint VBO_color;
	GLuint lightCubeVAO;
	unsigned int texture;

	float ratio;

public:
	Object(vec4 pivot, const char* image_file) {
		world_pivot = local_pivot = vec4(0);
		world_position = local_position = vec3(0);
		world_rotation = local_rotation = vec3(0);
		world_scale = local_scale = vec3(1);
		world_pivot = pivot;
		final_transform = mat4(1);

		this->image_file = image_file;
	}
	~Object() {

	}

	GLvoid set_vbo() {


		float vertexData[] = {
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
		};

		unsigned int indices[] = {
			1, 0, 3,
			2, 1, 3,
		};


		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);


		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 이미지 뒤집기 안하면 거꾸로 나옴
		stbi_set_flip_vertically_on_load(true);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(image_file, &width, &height, &nrChannels, 0);

		//float ratio;
		//if (width > height) {
		//	ratio = static_cast<float>(height) / width;
		//	world_scale.y *= ratio;
		//}
		//else {
		//	ratio = static_cast<float>(width) / height;
		//	world_scale.x *= ratio;
		//}

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
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

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {
		glEnable(GL_BLEND);
		unsigned int ortho_projection2 = glGetUniformLocation(s_program.getSprogram(), "projectionTransform");
		projection = ortho(double(-aspect_ratio), double(aspect_ratio), -1.0, 1.0, -1.0, 100.0);
		glUniformMatrix4fv(ortho_projection2, 1, GL_FALSE, &projection[0][0]);

		glUseProgram(s_program.getSprogram());
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(final_transform));
		glUniform1i(glGetUniformLocation(s_program.getSprogram(), "texture1"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glDisable(GL_BLEND);
	}
};

class Object2
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
	const char* image_file;

	//======================VAO, VBO========================//
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint VBO_position;
	GLuint VBO_normal;
	GLuint VBO_color;
	GLuint lightCubeVAO;
	unsigned int texture;

	float ratio;

public:
	Object2(vec4 pivot, const char* image_file) {
		world_pivot = local_pivot = vec4(0);
		world_position = local_position = vec3(0);
		world_rotation = local_rotation = vec3(0);
		world_scale = local_scale = vec3(1);
		world_pivot = pivot;
		final_transform = mat4(1);

		this->image_file = image_file;
	}
	~Object2() {

	}

	GLvoid set_vbo() {
		float vertexData[] = {
		 0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   0.5f, 1.5f, // top right
		 0.25f,  -0.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.25f,  -0.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		};

		unsigned int indices[] = {
			0, 1, 2
		};


		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);


		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// 이미지 뒤집기 안하면 거꾸로 나옴
		stbi_set_flip_vertically_on_load(true);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(image_file, &width, &height, &nrChannels, 0);

		float ratio;
		if (width > height) {
			ratio = static_cast<float>(height) / width;
			world_scale.y *= ratio;
		}
		else {
			ratio = static_cast<float>(width) / height;
			world_scale.x *= ratio;
		}

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
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

	GLvoid draw(ShaderProgram s_program, Camera cam, glm::mat4& projection) {

		unsigned int ortho_projection2 = glGetUniformLocation(s_program.getSprogram(), "projectionTransform");
		projection = ortho(double(-aspect_ratio), double(aspect_ratio), -1.0, 1.0, -1.0, 1.0);
		glUniformMatrix4fv(ortho_projection2, 1, GL_FALSE, &projection[0][0]);

		glUseProgram(s_program.getSprogram());
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(final_transform));
		glUniform1i(glGetUniformLocation(s_program.getSprogram(), "texture1"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
	}
};

Object2 quad(Object2{ glm::vec4(0.0f), "afont.png" });
Object2 quad2(Object2{ glm::vec4(0.0f), "bfont.png" });
Object2 quad3(Object2{ glm::vec4(0.0f), "cfont.png" });
Object2 quad4(Object2{ glm::vec4(0.0f), "dfont.png" });
Object quad5(Object{ glm::vec4(0.0f), "efont.png" });

Object cube(Object{ glm::vec4(0.0f), "afont.png"});
Object cube2(Object{ glm::vec4(0.0f), "bfont.png"});
Object cube3(Object{ glm::vec4(0.0f), "cfont.png"});
Object cube4(Object{ glm::vec4(0.0f), "dfont.png"});
Object cube5(Object{ glm::vec4(0.0f), "efont.png"});
Object cube6(Object{ glm::vec4(0.0f), "ffont.png"});

Object background(Object{ glm::vec4(0.0f), "background.png" });


//Object quad(Object{ glm::vec4(0.0f), "Quad.obj"});
//Object light_box(Object{ glm::vec4(0.0f), "Cube.obj" });
//Object* draw_obj;
vector<Object*> draw_obj;
vector<Object2*> draw_obj2;

void InitBuffer()
{
	for (auto& a : draw_obj) {
		a->set_vbo();
	}
	for (auto& a : draw_obj2) {
		a->set_vbo();
	}

	background.set_vbo();

	//quad.set_vbo();
	//cube.set_vbo();

	glEnable(GL_DEPTH_TEST);
	glBindVertexArray(0);
}

static bool isProjection = true;
static bool isHexi = true;
static bool isOn = true;
static bool isCubeRotation = false;
static bool isLightRotation = false;
static int isFarOrigin = 0;
static int isCube = true;

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


	//======================set object======================//
	//draw_obj->setting();
	//draw_obj->draw(obj1_s_program, *camera, projection);

	if (isCube)
	for (auto& a : draw_obj) {
		a->setting();
		a->draw(obj1_s_program, *camera, projection);
	}
	else 
	for (auto& a : draw_obj2) {
		a->setting();
		a->draw(obj1_s_program, *camera, projection);
	}
	//quad.setting();
	//quad.draw(obj1_s_program, *camera, projection);
	//cube.setting();
	//cube.draw(obj1_s_program, *camera, projection);
	background.setting();
	background.draw(obj1_s_program, *camera, projection);

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
		isCube = true;
		break;
	case 'c':
	case 'C':
		isCube = false;
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
	case 'x':
	case 'X':
		isLightRotation = isLightRotation ? false : true;
		break;
	case 's':
	case 'S':
		isCubeRotation = false;
		isLightRotation = false;
		if (isCube) {
			for (auto& a : draw_obj) {
				a->world_rotation.x = 30;
				a->world_rotation.y = 30;
				a->world_rotation.z = 0;
			}
		}
		else {
			quad.world_rotation.x = 30;
			quad.world_rotation.y = 30;
			quad.world_rotation.z = 0;
			quad2.world_rotation.x = 30;
			quad2.world_rotation.y = 30;
			quad2.world_rotation.z = 0;

			quad3.world_rotation = vec3(0);
			quad3.local_rotation = vec3(0);
		}

		break;
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	if (isCubeRotation) {
		if (isCube) {
			for (auto& a : draw_obj) {
				a->rotate(vec3(0, 1, 0));
			}
		}
		else {
			for (auto& a : draw_obj2) {
				a->rotate(vec3(0, 1, 0));
			}
		}
	}	
	
	if (isLightRotation) {
		if (isCube) {
			for (auto& a : draw_obj) {
				a->rotate(vec3(1, 0, 0));
			}
		}
		else {
			for (auto& a : draw_obj2) {
				a->rotate(vec3(1, 0, 0));
			}
		}
	}


	glutPostRedisplay();
	glutTimerFunc(10, TimerFunction, 1);
}


void Init()
{
	camera = &quarter_camera;
	draw_obj.push_back(&cube);
	draw_obj.push_back(&cube2);
	draw_obj.push_back(&cube3);
	draw_obj.push_back(&cube4);
	draw_obj.push_back(&cube5);
	draw_obj.push_back(&cube6);

	draw_obj2.push_back(&quad);
	draw_obj2.push_back(&quad2);
	draw_obj2.push_back(&quad3);
	draw_obj2.push_back(&quad4);

	cube.local_position.z += 0.5;

	cube2.local_position.z -= 0.5;
	cube3.lrotate(vec3(0, -90, 0));
	cube3.local_position.x += 0.5;
	cube4.lrotate(vec3(0, -90, 0));
	cube4.local_position.x -= 0.5;

	cube5.lrotate(vec3(-90, 0, 0));
	cube5.local_position.y -= 0.5;
	cube6.lrotate(vec3(-90, 0, 0));
	cube6.local_position.y += 0.5;

	for (auto& a : draw_obj) {
		a->rotate(vec3(30, 30, 0));
		a->scale(vec3(0.5));
	}

	//quad.lrotate(vec3(0, 90, 0));
	quad.world_rotation.y = 90;
	quad.lrotate(vec3(30, 0, 0));
	quad.local_position.z -= 0.25;

	quad2.world_rotation.y = 90;
	quad2.lrotate(vec3(-30, 0, 0));
	quad2.local_position.z += 0.25;

	quad3.lrotate(vec3(30, 0, 0));
	quad3.local_position.z -= 0.25;

	quad4.lrotate(vec3(-30, 0, 0));
	quad4.local_position.z += 0.25;

	//quad.lrotate(vec3(30, 0, 0));
	//quad.world_pivot.z += 0.25;
	//quad.world_position.x += 0.25;

	//quad2.local_rotation.y = 90;
	//quad2.world_rotation.z = -30;
	//quad2.local_pivot.x -= 0.25;

	//quad2.local_position.x -= 0.25;
	//quad2.lrotate(vec3(0, 90, 0));

	//quad3.local_position.z += 0.25;
	//quad4.local_position.z -= 0.25;

	//quad.lrotate(vec3(0, 90, 0));

	for (auto& a : draw_obj2) {
		a->rotate(vec3(30, 30, 0));
	}
	//draw_obj = &cube;
	background.world_position.z -= 99;
	background.scale(vec3(3));
	//cube.scale(vec3(1.0));
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
	glClearColor(0.1, 0.1, 0.1, 1.0f);
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