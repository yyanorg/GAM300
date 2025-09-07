#pragma once

#include "pch.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include "Asset Manager/Asset.hpp"

std::string get_file_contents(const char* filename);

class Shader : public IAsset {
public:
	GLuint ID;

    //Shader() {};
	//Shader(const char* vertexFile, const char* fragmentFile);

	bool LoadAsset(const std::string& path) override;

	void Activate();
	void Delete();

	/*!
     * @brief Sets a boolean uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value The boolean value to set.
     */
	void setBool(const std::string& name, GLboolean value) const;

    /*!
     * @brief Sets an integer uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value The integer value to set.
     */
	void setInt(const std::string& name, GLint value) const;

    void setIntArray(const std::string& name, const GLint* values, GLint count) const;

    /*!
     * @brief Sets a floating-point uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value The float value to set.
     */
	void setFloat(const std::string& name, GLfloat value) const;

    /*!
     * @brief Sets a 2D vector uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value A glm::vec2 containing the values to set.
     */
    void setVec2(const std::string& name, const glm::vec2& value) const;

    /*!
    * @brief Sets a 2D vector uniform in the shader program using individual components.
    * @param name The name of the uniform variable in the shader.
    * @param x The x-component of the vector.
    * @param y The y-component of the vector.
    */
    void setVec2(const std::string& name, float x, float y) const;
    // ------------------------------------------------------------------------
    /*!
     * @brief Sets a 3D vector uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value A glm::vec3 containing the values to set.
     */
    void setVec3(const std::string& name, const glm::vec3& value) const;

    /*!
     * @brief Sets a 3D vector uniform in the shader program using individual components.
     * @param name The name of the uniform variable in the shader.
     * @param x The x-component of the vector.
     * @param y The y-component of the vector.
     * @param z The z-component of the vector.
     */
    void setVec3(const std::string& name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    /*!
     * @brief Sets a 4D vector uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param value A glm::vec4 containing the values to set.
     */
    void setVec4(const std::string& name, const glm::vec4& value) const;

    /*!
     * @brief Sets a 4D vector uniform in the shader program using individual components.
     * @param name The name of the uniform variable in the shader.
     * @param x The x-component of the vector.
     * @param y The y-component of the vector.
     * @param z The z-component of the vector.
     * @param w The w-component of the vector.
     */
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    // ------------------------------------------------------------------------
    /*!
     * @brief Sets a 2x2 matrix uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param mat A glm::mat2 representing the matrix to set.
     */
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    // ------------------------------------------------------------------------
    /*!
     * @brief Sets a 3x3 matrix uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param mat A glm::mat3 representing the matrix to set.
     */
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    // ------------------------------------------------------------------------
    /*!
     * @brief Sets a 4x4 matrix uniform in the shader program.
     * @param name The name of the uniform variable in the shader.
     * @param mat A glm::mat4 representing the matrix to set.
     */
    void setMat4(const std::string& name, const glm::mat4& mat) const;
};
