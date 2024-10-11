#include "iara.h"

// -----Entry Point-----------------------
#include "iara/Core/EntryPoint.h"
// ---------------------------------------

#include "iara\Core\Application.h"

#include "imgui/imgui.h"
#include "platform/openGL/OpenGLShader.h"
#include <glm/gtc/matrix_transform.hpp>

#include "EditorLayer.h"

class ZIara : public iara::Application {
public:
	
	ZIara(AppCommandLineArgs args) : Application{ args } {
		pushLayer(new iara::EditorLayer());
	}

	~ZIara() {}

};

iara::Application* CreateApplication(AppCommandLineArgs args) {
	return new ZIara(args);
}