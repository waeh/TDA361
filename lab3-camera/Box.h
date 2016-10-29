#pragma once

#include <GL/glew.h>

// STB_IMAGE for loading images of many filetypes
#include <stb_image.h>

#include <vector>

#include <glm/glm.hpp>
using namespace glm;

using std::vector; 

class Box
{
private: 
	vector<vec3>	m_vertices; 
	vector<vec2>	m_texcoords; 
	vector<int>		m_indices; 
	GLuint positionBuffer; 
	GLuint texcoordBuffer; 
	GLuint indexBuffer; 
	GLuint vertexArrayObject;
	GLuint m_texture; 
	void addParallelogram(const vec3 &corner, const vec3 &U, const vec3 &V)
	{
		int base = m_vertices.size(); 
		m_vertices.push_back(corner); 
		m_vertices.push_back(corner + V); 
		m_vertices.push_back(corner + V + U); 
		m_vertices.push_back(corner + U); 
		m_texcoords.push_back(vec2(0.0f, 0.0f));
		m_texcoords.push_back(vec2(0.0f, 1.0f));
		m_texcoords.push_back(vec2(1.0f, 1.0f));
		m_texcoords.push_back(vec2(1.0f, 0.0f));
		m_indices.push_back(base); 
		m_indices.push_back(base+1); 
		m_indices.push_back(base+2); 
		m_indices.push_back(base); 
		m_indices.push_back(base+2); 
		m_indices.push_back(base+3); 
	}
public: 
	// Constructor
	Box()
	{
		addParallelogram(vec3(-1.0f, -1.0f, -1.0f), vec3(2.0f, 0.0f, 0.0f), vec3(0.0f, 2.0f, 0.0f));
		addParallelogram(vec3(-1.0f, -1.0f, -1.0f), vec3(0.0f, 2.0f, 0.0f), vec3(0.0f, 0.0f, 2.0f));
		addParallelogram(vec3(-1.0f, -1.0f, -1.0f), vec3(2.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 2.0f));
		addParallelogram(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, -2.0f, 0.0f));
		addParallelogram(vec3(1.0f, 1.0f, 1.0f), vec3(-2.0f, 0.0f, 0.0f), vec3(0.0f, -2.0f, 0.0f));
		addParallelogram(vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 0.0f, -2.0f), vec3(-2.0f, 0.0f, 0.0f));
		// Create the buffer objects
		glGenBuffers( 1, &positionBuffer );			
		glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(vec3), &m_vertices[0].x, GL_STATIC_DRAW);
		glGenBuffers( 1, &texcoordBuffer );			
		glBindBuffer( GL_ARRAY_BUFFER, texcoordBuffer );
		glBufferData(GL_ARRAY_BUFFER, m_texcoords.size() * sizeof(vec2), &m_texcoords[0].x, GL_STATIC_DRAW);
		glGenBuffers( 1, &indexBuffer );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );										
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(int), &m_indices[0], GL_STATIC_DRAW );			
		// Create the vertex array object
		glGenVertexArrays(1, &vertexArrayObject);
		glBindVertexArray(vertexArrayObject);
		glBindBuffer( GL_ARRAY_BUFFER, positionBuffer );	
		glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );	
		glBindBuffer( GL_ARRAY_BUFFER, texcoordBuffer );
		glVertexAttribPointer(2, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/ );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBuffer );
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(2);
		// Load texture
		int width, height, comp;
		unsigned char* image = stbi_load("Crate.bmp", &width, &height, &comp, STBI_rgb_alpha);

		glGenTextures(1, &m_texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		free(image);

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	}
	~Box()
	{
		glDeleteBuffers(1, &positionBuffer);
		glDeleteBuffers(1, &texcoordBuffer);
		glDeleteBuffers(1, &indexBuffer);
		glDeleteVertexArrays(1, &vertexArrayObject);
		glDeleteTextures(1, &m_texture); 
	}
	// Render box
	void draw()
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glBindVertexArray(vertexArrayObject);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	}
};
