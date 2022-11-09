#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "objRead.h"

using namespace std;

GLuint win_width = 1000;
GLuint win_height = 1000;
GLfloat mx;
GLfloat my;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float> urd{ 0.0, 1.0 };

GLvoid Reshape(int w, int h);
GLvoid convertDeviceXYOpenGlXY(int x, int y, float* ox, float* oy);

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

void InitShader()
{
	coord_v_shader.make_shader();
	obj1_v_shader.make_shader();
	temp_f_shader.make_shader();

	obj1_s_program.make_s_program(obj1_v_shader.getShader(), temp_f_shader.getShader());
	coord_s_program.make_s_program(coord_v_shader.getShader(), temp_f_shader.getShader());

	coord_v_shader.delete_shader();
	obj1_v_shader.delete_shader();
	temp_f_shader.delete_shader();
}

class Camera {
	glm::vec4 pivot;
	glm::vec4 target;

	glm::vec3 Pos;
	glm::vec3 Dir;
	glm::vec3 Up;
	glm::mat4 view;

	glm::vec4 final_transform;
	glm::vec4 final_dir_transform;

	GLfloat rotate_degree;
	glm::vec3 translate;

	GLfloat dir_rotate_degree;
	glm::vec3 dir_translate;

public:
	Camera() {

	}

	void translate(glm::vec3 trans) {
		translate += trans;
	}

	void rotate(GLfloat degree) {
		rotate_degree += degree;
	}

	void dir_translate(glm::vec3 trans) {
		dir_translate += trans;
	}

	void dir_rotate(GLfloat degree) {
		dir_rotate_degree += degree;
	}

	void setting(glm::vec4 pos_pivot, glm::vec4 target_pivot, glm::vec3 up_pivot) {
		pivot = pos_pivot;
		target = target_pivot;
		Up = up_pivot;

		final_transform = glm::vec4(pivot);
		final_dir_transform = glm::vec4(target);

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotate_degree), glm::vec3(0.0, 1.0, 0.0));
		trans = glm::translate(trans, glm::vec3(translate));
		final_transform = trans * final_transform;

		glm::mat4 dir_trans = glm::mat4(1.0f);
		dir_trans = glm::translate(dir_trans, glm::vec3(dir_translate));
		dir_trans = glm::rotate(dir_trans, (GLfloat)glm::radians(dir_rotate_degree), glm::vec3(0.0, 1.0, 0.0));
		final_dir_transform = trans * dir_trans * final_dir_transform;

		Pos = glm::vec3(final_transform);
		Dir = glm::vec3(final_dir_transform);
		view = glm::mat4(1.0f);
		view = glm::lookAt(Pos, Dir, Up);
	}

	glm::mat4 getView() {
		return view;
	}
};

class Object
{
	glm::vec4 pivot;
	GLuint vao;
	GLuint ebo;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_color;
	vector<Color> colors;

	objRead objReader; 
	GLint obj;

	unsigned int modelLocation;
	const char* modelTransform;
	
public:
	Object(const char* modelTransform, const char* objfile) {
		obj = objReader.loadObj_normalize_center(objfile);
		this->modelTransform = modelTransform;

		for (int i{}; i < objReader.nr_outvertex.size(); ++i) {
			colors.push_back({ urd(dre), urd(dre), urd(dre) });
		}
	}
	~Object() {

	}

	GLvoid set_vbo() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao); //--- VAO를 바인드하기

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
		GLint cAttribute = glGetAttribLocation(obj1_s_program.getSprogram(), "vColor");
		glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
		glEnableVertexAttribArray(cAttribute);
	}

	GLvoid draw(GLuint s_program) {
		glUseProgram(s_program);
		glBindVertexArray(vao); //--- VAO를 바인드하기
		unsigned int obj1_modelLocation = glGetUniformLocation(s_program, modelTransform);
		glUniformMatrix4fv(obj1_modelLocation, 1, GL_FALSE, glm::value_ptr(SRT));
		glPointSize(10);
		glDrawElements(GL_TRIANGLES, obj, GL_UNSIGNED_INT, 0);
	}
};
