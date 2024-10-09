#include "iara.h"

// -----Entry Point-----------------------
#include "iara/Core/EntryPoint.h"
// ---------------------------------------

#include "imgui/imgui.h"
#include <memory>
#include "platform/openGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

#include "EditorLayer.h"

class ZIara : public iara::Application {
public:
	ZIara() {
		pushLayer(new iara::EditorLayer());
	}

	~ZIara() {}

};

iara::Application* CreateApplication() {
	return new ZIara();
}