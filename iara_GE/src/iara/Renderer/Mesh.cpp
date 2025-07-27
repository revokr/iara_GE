#include "ir_pch.h"
#include "Mesh.h"

namespace iara {
	void Mesh::createBuffers() {
		vao = VertexArray::Create();
		vb = VertexBuffer::Create();

		vb->setLayout({
			{ ShaderDataType::Float3, "a_pos" },
			{ ShaderDataType::Float3, "a_normal" },
			{ ShaderDataType::Float2, "a_tex_coords" },
			{ ShaderDataType::Float3, "a_tangent" },
			{ ShaderDataType::Float3, "a_bitangent" }
			//{ ShaderDataType::Int,    "a_entity_id" }
			});

		ib = IndexBuffer::Create();
	}


	bool Mesh::loadModel(const std::string& path, int entityID) {
		bool ret = false;
		std::filesystem::path file_path = path;
		
		aiPostProcessSteps flags = file_path.extension() == ".fbx" ? aiProcess_None : aiProcess_FlipUVs;
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
			 aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			IARA_CORE_ERROR("Error Assimp: {0}", importer.GetErrorString());
			return false;
		}
		else {
			ret = initFromScene(scene, path, entityID);
		}
		//delete scene;
		//aiReleaseImport(scene);
		return ret;
	}

	void Mesh::reloadVertices(const glm::mat4& transform) {
		for (auto& vert : mesh_vertex_array) {
			vert.position = glm::vec3(glm::vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f) * transform);
		}
	}

	bool Mesh::initFromScene(const aiScene* scene, const std::string& filename, int entityID)
	{
		meshes.resize(scene->mNumMeshes);
		//materials.resize(scene->mNumMaterials);

		countVerticesIndices(scene, m_num_vertices, m_num_indices);

		/// reserve space
		mesh_vertex_array.reserve(m_num_vertices);
		indices.reserve(m_num_indices);

		initAllSubMeshes(scene, entityID);

		if (!initMaterials(scene, filename)) {
			return false;
		}

		vb->SetData(mesh_vertex_array.data(), sizeof(MeshVertex) * (uint32_t)mesh_vertex_array.size());
		ib->setData(indices.data(), (uint32_t)indices.size());

		IARA_CORE_TRACE("Max index: {0}", *std::max_element(indices.begin(), indices.end()));

		vao->setVertexBuffer(vb);
		vao->SetIndexBuffer(ib);

		mesh_vertex_array.clear();
		indices.clear();
	}

	void Mesh::countVerticesIndices(const aiScene* scene, uint32_t& vert, uint32_t& ind) {
		for (size_t i = 0; i < meshes.size(); i++) {
			meshes[i].materialInd = scene->mMeshes[i]->mMaterialIndex;
			meshes[i].numInd = scene->mMeshes[i]->mNumFaces * 3;
			meshes[i].baseVertex = m_num_vertices;
			meshes[i].baseIndex = m_num_indices;

			m_num_vertices += scene->mMeshes[i]->mNumVertices;
			m_num_indices += meshes[i].numInd;
		}
	}

	void Mesh::initAllSubMeshes(const aiScene* scene, int entityID) {
		for (size_t i = 0; i < meshes.size(); i++) {
			const aiMesh* mesh = scene->mMeshes[i];
			initSingleSubMeshes(mesh, entityID);
		}
	}

	void Mesh::initSingleSubMeshes(const aiMesh* mesh, int entityID) {
		const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

		for (size_t i = 0; i < mesh->mNumVertices; i++) {
			const aiVector3D& pos = mesh->mVertices[i];
			const aiVector3D& normal = mesh->mNormals[i];
			const aiVector3D& tc = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : zero3D;
			const aiVector3D& tangent = mesh->mTangents[i];
			const aiVector3D& bitangent = mesh->mBitangents[i];

			mesh_vertex_array.push_back({ {pos.x, pos.y, pos.z} , {normal.x, normal.y, normal.z}, {tc.x, tc.y}, 
										  {tangent.x, tangent.y, tangent.z}, {bitangent.x, bitangent.y, bitangent.z}/*, entityID*/ });
		}

		for (size_t i = 0; i < mesh->mNumFaces; i++) {
			const aiFace& face = mesh->mFaces[i];

			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);	
		}
	}

	bool Mesh::initMaterials(const aiScene* scene, const std::string& filename) {
		std::string directory = filename.substr(0, filename.find_last_of('\\'));

		bool ret = true;

		for (size_t i = 0; i < scene->mNumMaterials; i++) {
			const aiMaterial* material = scene->mMaterials[i];
			
			Material mat;
			mat.diffuse_map = m_white_tex;
			mat.specular_map = m_white_tex;
			mat.normal_map = m_white_tex;

			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString path;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string p(path.data);

					if (p.substr(0, 2) == ".\\") {
						p = p.substr(2, p.size() - 2);
					}

					std::string fullpath = directory + "\\" + p;

					mat.diffuse_map = Texture2D::Create(fullpath);

					//IARA_CORE_TRACE("Loaded Diffuse texture {0}", fullpath);
				}
			}


			if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
				aiString path2;
				if (material->GetTexture(aiTextureType_SPECULAR, 0, &path2, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string p(path2.data);

					if (p.substr(0, 2) == ".\\") {
						p = p.substr(2, p.size() - 2);
					}

					std::string fullpath = directory + "\\" + p;

					mat.specular_map = Texture2D::Create(fullpath);

					//IARA_CORE_TRACE("Loaded Specular texture {0}", fullpath);
				}
			}

			if (material->GetTextureCount(aiTextureType_HEIGHT) > 0) {
				aiString path2;
				if (material->GetTexture(aiTextureType_HEIGHT, 0, &path2, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
					std::string p(path2.data);

					if (p.substr(0, 2) == ".\\") {
						p = p.substr(2, p.size() - 2);
					}

					std::string fullpath = directory + "\\" + p;

					mat.normal_map = Texture2D::Create(fullpath);

					//IARA_CORE_TRACE("Loaded Normal texture {0}", fullpath);
				}
			}

			materials.push_back(mat);
		}

		return ret;
	}


}
