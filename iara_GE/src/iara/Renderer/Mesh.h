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
	};

	struct MeshVertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 tex_coord;

		//int entityID;
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
			vao = VertexArray::Create();
			vb = VertexBuffer::Create();

			vb->setLayout({
				{ ShaderDataType::Float3, "a_pos" },
				{ ShaderDataType::Float3, "a_normal" },
				{ ShaderDataType::Float2, "a_tex_coords" }
				//{ ShaderDataType::Int,    "a_entity_id" }
			});

			ib = IndexBuffer::Create();
			m_white_tex = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			m_white_tex->setData(&whiteTextureData, sizeof(uint32_t));
		}

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
		std::vector<Material> materials;

		std::vector<MeshVertex> mesh_vertex_array;
		std::vector<uint32_t>  indices;
	};
}