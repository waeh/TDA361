#include "Model.h"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tiny_obj_loader.h>
//#include <experimental/tinyobj_loader_opt.h>
#include <algorithm>
#include <sstream>
#include <iomanip> 
#include <GL/glew.h>
#include <stb_image.h>

namespace labhelper
{
	///////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////
	Model::~Model()
	{
		// Clear all GPU memory used by the Model
		for (auto & material : m_materials) {
			if (material.m_texture_id.diffuse != -1) glDeleteTextures(1, &material.m_texture_id.diffuse);
			if (material.m_texture_id.specular != -1) glDeleteTextures(1, &material.m_texture_id.specular);
			if (material.m_texture_id.ambient != -1) glDeleteTextures(1, &material.m_texture_id.ambient);
		}
		glDeleteBuffers(1, &m_positions_bo);
		glDeleteBuffers(1, &m_normals_bo);
		glDeleteBuffers(1, &m_texture_coordinates_bo);
	}

	Model * loadModelFromOBJ(std::string path)
	{
		///////////////////////////////////////////////////////////////////////
		// Separate filename into directory, base filename and extension
		// NOTE: This can be made a LOT simpler as soon as compilers properly 
		//		 support std::filesystem (C++17)
		///////////////////////////////////////////////////////////////////////
		size_t separator = path.find_last_of("\\/");
		std::string filename, extension, directory; 
		if (separator != std::string::npos) {
			filename = path.substr(separator + 1, path.size() - separator - 1); 
			directory = path.substr(0, separator + 1); 
		}
		else {
			filename = path; 
			directory = "./";
		}
		separator = filename.find_last_of(".");
		if (separator == std::string::npos) {
			std::cout << "Fatal: loadModelFromOBJ(): Expecting filename ending in '.obj'\n";
			exit(1);
		}
		extension = filename.substr(separator, filename.size() - separator);
		filename = filename.substr(0, separator); 
		///////////////////////////////////////////////////////////////////////
		// Parse the OBJ file using tinyobj
		///////////////////////////////////////////////////////////////////////
		std::cout << "Loading " << path << "..." << std::flush; 
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		// Expect '.mtl' file in the same directory and triangulate meshes 
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, 
			(directory + filename + extension).c_str(), directory.c_str(), 
			true);
		if (!err.empty()) { // `err` may contain warning message.
			std::cerr << err << std::endl;
		}
		if (!ret) { exit(1); }
		Model * model = new Model;
		model->m_name = filename;

		///////////////////////////////////////////////////////////////////////
		// Transform all materials into our datastructure
		///////////////////////////////////////////////////////////////////////
		auto loadTexture = [](const std::string & filename, uint32_t & gl_id) {
			int width, height, components; 
			uint8_t * data = stbi_load(filename.c_str(), &width, &height, &components, 4);
			if (data == nullptr) {
				std::cout << "ERROR: loadModelFromOBJ(): Failed to load texture: " << filename << "\n";
			}

			// most images have data from upper left corner, 
			// but glTexImage2D assumes data start from lower left corner
			int lorow = 0;
			int hirow = height - 1;
			while (lorow < hirow) {
				uint32_t * lorowdata = (uint32_t *)&data[4 * width * lorow];
				uint32_t * hirowdata = (uint32_t *)&data[4 * width * hirow];
				for (int col = 0; col < width; col++) {
					uint32_t tmp = lorowdata[col];
					lorowdata[col] = hirowdata[col];
					hirowdata[col] = tmp;
				}
				lorow++;
				hirow--;
			}

			glGenTextures(1, &gl_id);
			glBindTexture(GL_TEXTURE_2D, gl_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
			stbi_image_free(data);
		};
		for (const auto & m : materials) {
			Material material; 
			material.m_name = m.name;
			material.m_texture_id.diffuse = 0;
			material.m_diffuse_reflectance = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
			if (m.diffuse_texname != "") { 
				loadTexture(directory + m.diffuse_texname, material.m_texture_id.diffuse);
			}
			material.m_texture_id.specular = 0;
			material.m_specular_reflectance = glm::vec3(m.specular[0], m.specular[1], m.specular[2]);
			if (m.specular_texname != "") {
				loadTexture(directory + m.specular_texname, material.m_texture_id.specular);
			}
			material.m_texture_id.ambient = 0;
			material.m_ambient_reflectance = glm::vec3(m.ambient[0], m.ambient[1], m.ambient[2]);
			if (m.ambient_texname != "") {
				loadTexture(directory + m.ambient_texname, material.m_texture_id.ambient);
			}
			material.m_shininess = m.shininess;
			material.m_emission = glm::vec3(m.emission[0], m.emission[1], m.emission[2]);

			model->m_materials.push_back(material);
		}

		///////////////////////////////////////////////////////////////////////
		// A vertex in the OBJ file may have different indices for position, 
		// normal and texture coordinate. We will not even attempt to use 
		// indexed lookups, but will store a simple vertex stream per mesh. 
		///////////////////////////////////////////////////////////////////////
		int number_of_vertices = 0; 
		for (const auto & shape : shapes) {
			number_of_vertices += shape.mesh.indices.size(); 
		}
		model->m_positions.resize(number_of_vertices);
		model->m_normals.resize(number_of_vertices);
		model->m_texture_coordinates.resize(number_of_vertices);

		///////////////////////////////////////////////////////////////////////
		// For each vertex _position_ auto generate a normal that will be used
		// if no normal is supplied. 
		///////////////////////////////////////////////////////////////////////
		std::vector<glm::vec4> auto_normals(attrib.vertices.size() / 3); 
		for (const auto & shape : shapes) {
			for (int face = 0; face < int(shape.mesh.indices.size()) / 3; face++) {
				
				glm::vec3 v0 = glm::vec3(
					attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 0],
					attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 1],
					attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 2]
					);
				glm::vec3 v1 = glm::vec3(
					attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 0],
					attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 1],
					attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 2]
					);
				glm::vec3 v2 = glm::vec3(
					attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 0],
					attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 1],
					attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 2]
					);

				glm::vec3 e0 = glm::normalize(v1 - v0);
				glm::vec3 e1 = glm::normalize(v2 - v0);
				glm::vec3 face_normal = cross(e0, e1);
				
				auto_normals[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(face_normal, 1.0f);
				auto_normals[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(face_normal, 1.0f);
				auto_normals[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(face_normal, 1.0f);
			}
		}
		for (auto & normal : auto_normals) {
			normal = (1.0f / normal.w) * normal; 
		}

		///////////////////////////////////////////////////////////////////////
		// Now we will turn all shapes into Meshes. A shape that has several 
		// materials will be split into several meshes with unique names
		///////////////////////////////////////////////////////////////////////
		int vertices_so_far = 0; 
		for (const auto & shape : shapes)
		{
			///////////////////////////////////////////////////////////////////
			// The shapes in an OBJ file may several different materials. 
			// If so, we will split the shape into one Mesh per Material
			///////////////////////////////////////////////////////////////////
			int number_of_materials_in_shape = 1; 
			int next_material_index = shape.mesh.material_ids[0];
			int next_material_starting_face = 0;
			std::vector<bool> finished_materials(materials.size(), false);
			while (next_material_index != -1)
			{
				int current_material_index = next_material_index; 
				int current_material_starting_face = next_material_starting_face; 
				next_material_index = -1;
				next_material_starting_face = -1; 
				// Process a new Mesh with a unique material
				Mesh mesh;
				mesh.m_name = shape.name + "_" + materials[current_material_index].name; 
				mesh.m_material = &model->m_materials[current_material_index];
				mesh.m_start_index = vertices_so_far;

				int number_of_faces = shape.mesh.indices.size() / 3;
				for (int i = current_material_starting_face; i < number_of_faces; i++)
				{
					if (shape.mesh.material_ids[i] != current_material_index) {
						if (next_material_index >= 0) continue; 
						else if (finished_materials[shape.mesh.material_ids[i]]) continue; 
						else { // Found a new material that we have not processed.
							next_material_index = shape.mesh.material_ids[i]; 
							next_material_starting_face = i;
						}
					}
					else {
						///////////////////////////////////////////////////////
						// Now we generate the vertices
						///////////////////////////////////////////////////////
						for (int j = 0; j < 3; j++) {
							int v = shape.mesh.indices[i * 3 + j].vertex_index;
							model->m_positions[vertices_so_far + j] = glm::vec3(
								attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 0],
								attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 1],
								attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 2]);
							if (shape.mesh.indices[i * 3 + j].normal_index == -1) {
								// No normal, use the autogenerated
								model->m_normals[vertices_so_far + j] = glm::vec3(auto_normals[shape.mesh.indices[i * 3 + j].vertex_index]);
							}
							else {
								model->m_normals[vertices_so_far + j] = glm::vec3(
									attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 0],
									attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 1],
									attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 2]);
							}
							if (shape.mesh.indices[i * 3 + j].texcoord_index == -1) {
								// No UV coordinates. Use null. 
								model->m_texture_coordinates[vertices_so_far + j] = glm::vec2(0.0f);
							}
							else {
								model->m_texture_coordinates[vertices_so_far + j] = glm::vec2(
									attrib.texcoords[shape.mesh.indices[i * 3 + j].texcoord_index * 2 + 0],
									attrib.texcoords[shape.mesh.indices[i * 3 + j].texcoord_index * 2 + 1]);
							}
						}
						vertices_so_far += 3;
					}
				}
				///////////////////////////////////////////////////////////////
				// Finalize and push this mesh to the list
				///////////////////////////////////////////////////////////////
				mesh.m_number_of_vertices =	vertices_so_far - mesh.m_start_index;
				model->m_meshes.push_back(mesh);
				finished_materials[current_material_index] = true; 
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Upload to GPU
		///////////////////////////////////////////////////////////////////////
		glGenVertexArrays(1, &model->m_vaob);
		glBindVertexArray(model->m_vaob);
		glGenBuffers(1, &model->m_positions_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_positions_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_positions.size() * sizeof(glm::vec3),
			&model->m_positions[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &model->m_normals_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_normals_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_normals.size() * sizeof(glm::vec3),
			&model->m_normals[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);
		glGenBuffers(1, &model->m_texture_coordinates_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_texture_coordinates_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_texture_coordinates.size() * sizeof(glm::vec2),
			&model->m_texture_coordinates[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(2);

		std::cout << "done.\n";
		return model; 
	}

	///////////////////////////////////////////////////////////////////////
	// Free model 
	///////////////////////////////////////////////////////////////////////
	void freeModel(Model * model) {
		if(model != nullptr) delete model; 
	}

	///////////////////////////////////////////////////////////////////////
	// Loop through all Meshes in the Model and render them
	///////////////////////////////////////////////////////////////////////
	void render(const Model * model)
	{
		glBindVertexArray(model->m_vaob);
		for (auto & mesh : model->m_meshes)
		{
			bool has_diffuse_texture = mesh.m_material->m_texture_id.diffuse != 0;
			bool has_specular_texture = mesh.m_material->m_texture_id.specular != 0;
			bool has_ambient_texture = mesh.m_material->m_texture_id.ambient != 0;
			if (has_diffuse_texture) glBindTextures(0, 1, &mesh.m_material->m_texture_id.diffuse);
			if (has_specular_texture) glBindTextures(1, 1, &mesh.m_material->m_texture_id.specular);
			if (has_ambient_texture) glBindTextures(2, 1, &mesh.m_material->m_texture_id.ambient);

			GLint current_program = 0;
			glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
			glUniform1i(glGetUniformLocation(current_program, "has_diffuse_texture"), has_diffuse_texture);
			glUniform3fv(glGetUniformLocation(current_program, "material_diffuse_color"), 1, &mesh.m_material->m_diffuse_reflectance.x);
			glUniform1i(glGetUniformLocation(current_program, "has_specular_texture"), has_specular_texture);
			glUniform3fv(glGetUniformLocation(current_program, "material_specular_color"), 1, &mesh.m_material->m_specular_reflectance.x);
			glUniform1i(glGetUniformLocation(current_program, "has_ambient_texture"), has_ambient_texture);
			glUniform3fv(glGetUniformLocation(current_program, "material_ambient_color"), 1, &mesh.m_material->m_ambient_reflectance.x);
			glUniform3fv(glGetUniformLocation(current_program, "material_emissive_color"), 1, &mesh.m_material->m_emission.x);
			glUniform1f(glGetUniformLocation(current_program, "material_shininess"), mesh.m_material->m_shininess);
			glDrawArrays(GL_TRIANGLES, mesh.m_start_index, (GLsizei)mesh.m_number_of_vertices);
		}
	}
}
