#include "pch.h"

#include <filesystem>
#include "Graphics/ShaderClass.h"

std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

std::string Shader::CompileToResource(const std::string& path) {
	std::string vertexFile = path + ".vert";
	std::string fragmentFile = path + ".frag";

	// Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = get_file_contents(vertexFile.c_str());
	std::string fragmentCode = get_file_contents(fragmentFile.c_str());

	// Convert the shader source strings into character arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Create Vertex Shader Object and get its reference
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return std::string{};
	}

	// Create Fragment Shader Object and get its reference
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		return std::string{};
	}

	// Create Shader Program Object and get its reference
	ID = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(ID);
	// check for linking errors
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return std::string{};
	}

	// Delete the now useless Vertex and Fragment Shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Retrieve the binary code of the compiled shader.
	glGetProgramiv(ID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

	binaryData.resize(binaryLength);
	glGetProgramBinary(ID, binaryLength, nullptr, &binaryFormat, binaryData.data());

	// Save the binary code to a file.
	std::filesystem::path p(path);
	std::string shaderPath = (p.parent_path() / p.stem()).generic_string() + ".shader";

	std::ofstream shaderFile(shaderPath, std::ios::binary);
	if (shaderFile.is_open()) {
		// Write the binary format to the file.
		shaderFile.write(reinterpret_cast<const char*>(&binaryFormat), sizeof(binaryFormat));
		// Write the binary length to the file.
		shaderFile.write(reinterpret_cast<const char*>(&binaryLength), sizeof(binaryLength));
		// Write the binary code to the file.
		shaderFile.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
		shaderFile.close();
		return shaderPath;
	}

	return std::string{};
}

bool Shader::LoadResource(const std::string& assetPath)
{
	std::filesystem::path assetPathFS(assetPath);
	std::string resourcePath = (assetPathFS.parent_path() / assetPathFS.stem()).generic_string() + ".shader";

	std::ifstream shaderFile(resourcePath, std::ios::binary);
	if (shaderFile.is_open()) {
		// Read the binary format from the file.
		shaderFile.read(reinterpret_cast<char*>(&binaryFormat), sizeof(binaryFormat));
		// Read the binary length from the file.
		shaderFile.read(reinterpret_cast<char*>(&binaryLength), sizeof(binaryLength));
		binaryData.resize(binaryLength);
		// Read the binary code from the file.
		shaderFile.read(reinterpret_cast<char*>(binaryData.data()), binaryLength);

		// Create a new shader program.
		ID = glCreateProgram();
		glProgramBinary(ID, binaryFormat, binaryData.data(), binaryLength);

		// Check if the program was successfully loaded.
		GLint status = 0;
		glGetProgramiv(ID, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			std::cerr << "[SHADER]: Failed to load shader program from binary." << std::endl;
			return false;
		}

		return true;
	}
	else {
		std::cerr << "[SHADER]: Shader file not found: " << resourcePath << std::endl;
		return false;
	}
}

std::shared_ptr<AssetMeta> Shader::ExtendMetaFile(const std::string& assetPath, std::shared_ptr<AssetMeta> currentMetaData)
{
	return std::shared_ptr<AssetMeta>();
}

void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}

void Shader::setBool(const std::string& name, GLboolean value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform1i(location, (int)value);
	}
}

void Shader::setInt(const std::string& name, int value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform1i(location, value);
	}
}

void Shader::setIntArray(const std::string& name, const GLint* values, GLint count)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform1iv(location, count, values);
	}
}

void Shader::setFloat(const std::string& name, GLfloat value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform1f(location, value);
	}
}

void Shader::setVec2(const std::string& name, const glm::vec2& value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform2fv(location, 1, &value[0]);
	}
}

void Shader::setVec2(const std::string& name, float x, float y)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform2f(location, x, y);
	}
}

void Shader::setVec3(const std::string& name, const glm::vec3& value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform3fv(location, 1, &value[0]);
	}
}

void Shader::setVec3(const std::string& name, float x, float y, float z)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform3f(location, x, y, z);
	}
}

void Shader::setVec4(const std::string& name, const glm::vec4& value)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform4fv(location, 1, &value[0]);
	}
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniform4f(location, x, y, z, w);
	}
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
	GLint location = getUniformLocation(name);
	if (location != -1) {
		glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}

GLint Shader::getUniformLocation(const std::string& name)
{
	auto it = m_uniformCache.find(name);
	if (it != m_uniformCache.end())
	{
		return it->second;
	}

	GLint location = glGetUniformLocation(ID, name.c_str());
	m_uniformCache[name] = location;

	// Debug output for missing uniforms (can be removed later)
	if (location == -1)
	{
		std::cout << "Warning: Uniform '" << name << "' not found in shader ID: " << ID << std::endl;
	}

	return location;
}
