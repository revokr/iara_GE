#pragma once

#include "iara\Scene\Component.h"

//namespace iara {
//
//	class MeshRenderer {
//	public:
//		static void BeginScene(EditorCamera& camera);
//		static void EndScene();
//		
//		/// This will load the mesh, and store the data inside the VAO, VBO and so on, preparing data for flush at the end of the scene
//		static void drawMesh(const glm::mat4& transform, const MeshComponent& mesh /*OR STRING PATH*/, const uint32_t material_index);
//	private:
//		static void Flush();
//
//		static void LoadMesh(const std::string& path);
//	};
//}