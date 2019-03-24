#ifndef SHADER_H
#define SHADER_H


// OpenGL Functionality
#include "glad/glad.h"

// GL Math Library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Adapted from Joey De Vries at https://learnopengl.com
// Source: https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h

class Shader
{
public:
	unsigned int ID;

	Shader(const char* vertexFile, const char* fragmentFile)
	{
		std::string v_code;
		std::string f_code;
		std::ifstream v_file;
		std::ifstream f_file;

		v_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		f_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		// Read Files
		try
		{
			v_file.open(vertexFile);
			f_file.open(fragmentFile);
			std::stringstream v_stream, f_stream;
	
			v_stream << v_file.rdbuf();
			f_stream << f_file.rdbuf();

			v_file.close();
			f_file.close();

			v_code = v_stream.str();
			f_code = f_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "Vertex or Fragment Shader Read Failure" << std::endl;
			std::cout << vertexFile << std::endl;
			std::cout << fragmentFile << std::endl;
		}
		// Convert to c strings
		const char* cstr_v_code = v_code.c_str();
		const char * cstr_f_code = f_code.c_str();

	
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		// Compile Shaders

		// Vertex Shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &cstr_v_code, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &cstr_f_code, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		// Link Shaders
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);

		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		// Delete once finished
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void use()
	{
		glUseProgram(ID);
	}
	
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setMat4(const char* name, glm::mat4 matrix)
	{
		unsigned int location = glGetUniformLocation(ID, name);
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void setMat3(const char* name, glm::mat4 matrix)
	{
		unsigned int location = glGetUniformLocation(ID, name);
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void setVec2(const char* name, glm::vec2 vec)
	{
		unsigned int location = glGetUniformLocation(ID, name);
		glUniform2fv(location, 1, glm::value_ptr(vec));
	}

	void setVec3(const char* name, glm::vec3 vec)
	{		
		unsigned int location = glGetUniformLocation(ID, name);
		glUniform3fv(location, 1, glm::value_ptr(vec));
	}

	unsigned int getID()
	{
		return ID;
	}


	void addGeometryShader(const char* file)
	{
		std::cout << "Geo Shader Linking Started" << std::endl;
		std::string geoCode;
		std::ifstream gFile;
		gFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			gFile.open(file);
			std::stringstream v_stream;
			
			v_stream << gFile.rdbuf();
			
			gFile.close();
			
			geoCode = v_stream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "Geometry Shader Failed To Read" << std::endl;
			std::cout << file << std::endl;
		}

		const char* gShaderCode = geoCode.c_str();
		unsigned int geo;
		int success;
		char infoLog[512];

		geo = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geo, 1, &gShaderCode, NULL);
		glCompileShader(geo);
		checkCompileErrors(geo, "GEOMETRY");

		glAttachShader(ID, geo);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		glDeleteShader(geo);
		std::cout << "Geometry Shader Successfully Added" << std::endl;
	}

	/*
	void addTessellationShader(const char* control, const char* evaluation)
	{
		std::cout << "Tessellation Shader Linking" << std::endl;
		std::string controlCode;
		std::string evalCode;

		std::ifstream cShaderFile;
		std::ifstream eShaderFile;

		cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		eShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			cShaderFile.open(control);
			std::stringstream cShaderStream;
			cShaderStream << cShaderFile.rdbuf();
			cShaderFile.close();
			controlCode = cShaderStream.str();

			eShaderFile.open(evaluation);
			std::stringstream eShaderStream;
			eShaderStream << eShaderFile.rdbuf();
			eShaderFile.close();
			evalCode = eShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "Cannot Read Tess Shaders" << std::endl;
			std::cout << e.what() << std::endl;
			std::cout << control << std::endl;
			std::cout << evaluation << std::endl;
		}

		const char* cShaderCode = controlCode.c_str();
		const char* eShaderCode = evalCode.c_str();
		int success;
		char infoLog[512];


		unsigned int tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tcs, 1, &cShaderCode, NULL);
		glCompileShader(tcs);
		checkCompileErrors(tcs, "TESS_CONTROL_SHADER");
		glAttachShader(ID, tcs);

		unsigned int tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tes, 1, &eShaderCode, NULL);
		glCompileShader(tes);
		checkCompileErrors(tes, "TESS_EVALUATION_SHADER");
		glAttachShader(ID, tes);

		glLinkProgram(ID);
		checkCompileErrors(ID, "TESS_LINKING");

		glDeleteShader(tcs);
		glDeleteShader(tes);
		std::cout << "Tessellation Shaders Successfully Added" << std::endl;
	}
	*/


private:
	void checkCompileErrors(unsigned int shader, std::string type)
	{
		int success;
		char infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
#endif