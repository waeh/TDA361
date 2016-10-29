#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace labhelper
{	
	struct Material
	{
		std::string m_name; 
		glm::vec3 m_diffuse_reflectance; 
		glm::vec3 m_specular_reflectance;
		glm::vec3 m_ambient_reflectance;
		struct { uint32_t diffuse, specular, ambient; } m_texture_id;
		float m_shininess; 
		glm::vec3 m_emission;
	};

	struct Mesh
	{
		std::string m_name;
		Material * m_material; 
		// Where this Mesh's vertices start
		uint32_t m_start_index; 
		uint32_t m_number_of_vertices;
	};

	class Model
	{
	public: 
		~Model(); 
		// The name of the whole model
		std::string m_name; 
		// The materials 
		std::vector<Material> m_materials; 
		// A model will contain one or more "Meshes"
		std::vector<Mesh> m_meshes; 
		// Buffers on CPU
		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec2> m_texture_coordinates; 
		// Buffers on GPU
		uint32_t m_positions_bo;
		uint32_t m_normals_bo;
		uint32_t m_texture_coordinates_bo;
		// Vertex Array Object
		uint32_t m_vaob;
	};

	Model * loadModelFromOBJ(std::string filename);
	void freeModel(Model * model);
	void render(const Model * model); 
}