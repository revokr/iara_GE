#pragma once

#include "platform\openGL\OpenGLBuffer.h"
#include "platform\openGL\OpenGLVertexArray.h"
#include "platform\openGL\OpenGLShader.h"
#include "iara\Renderer\Texture.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>


namespace iara {

	struct Material {
		glm::vec4 diffuse = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
		float shininess = 10.0f;
		Ref<Texture2D> diffuse_map = nullptr;
		Ref<Texture2D> specular_map = nullptr;
		Ref<Texture2D> normal_map = nullptr;
	};

	struct MaterialPaths {
		std::string diff_path;
		std::string spec_path;
		std::string norm_path;
	};

	struct MeshVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex_coord;
		glm::vec3 tangent;
		glm::vec3 bitangent;

		//int entityID;

		bool operator==(const MeshVertex& other) const {
			return position == other.position &&
				normal == other.normal &&
				tex_coord == other.tex_coord &&
				tangent == other.tangent &&
				bitangent == other.bitangent;
		}
	};

	struct BasicMeshEntry {
		BasicMeshEntry() {
			numInd = 0;
			baseVertex = 0;
			baseIndex = 0;
			materialInd = 0xFFFFFFFF;
		}

		uint32_t numInd;
		uint32_t baseVertex;
		uint32_t baseIndex;
		uint32_t materialInd;
	};
	

	class Mesh {
	public:
		Mesh() {
			
			
		}
		
		void createBuffers();
		void createMaterials();
		bool loadModel(const std::string& path, int entityID);
		void reloadVertices(const glm::mat4& transform);

	private:
		bool initFromScene(const aiScene* scene, const std::string& filename, int entityID);

		void countVerticesIndices(const aiScene* scene, uint32_t& vert, uint32_t& ind);

		void initAllSubMeshes(const aiScene* scene, int entityID);

		void initSingleSubMeshes(const aiMesh* scene, int entityID);

		bool initMaterials(const aiScene* scene, const std::string& filename);

		Ref<Texture2D> m_white_tex;
		
	public:
		Ref<VertexArray> vao;
		Ref<VertexBuffer> vb;
		Ref<IndexBuffer> ib;

		std::vector<BasicMeshEntry> meshes;
		std::vector<MaterialPaths> material_paths;
		std::vector<Material> materials;

		std::vector<MeshVertex> mesh_vertex_array;
		std::vector<uint32_t>  indices;

		uint32_t m_num_vertices = 0;
		uint32_t m_num_indices = 0;
	};
}