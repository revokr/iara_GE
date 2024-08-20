#include "iara.h"

class ExampleLayer : public iara::Layer {
public:
	ExampleLayer() : Layer{ "Example" } {}

	void onUpdate() override {
		//IARA_INFO("{0}", m_Name);

		if (iara::Input::IsKeyPressed(IARA_KEY_TAB)) {
			IARA_TRACE("Tab key is pressed");
		}
	}

	void onEvent(iara::Event& event) override {
		IARA_TRACE(event.ToString());
		if (event.getEventType() == iara::EventType::KeyPressed) {
			iara::KeyPressedEvent& e = (iara::KeyPressedEvent&)event;
			IARA_TRACE("{0}", (char)e.getKeyCode());
		}
	}
};

class Sandbox : public iara::Application {
public:
	Sandbox() {
		pushLayer(new ExampleLayer());
		pushOverlay(new iara::ImGuiLayer());
	}
	~Sandbox() {}

};

iara::Application* CreateApplication() {
	return new Sandbox();
}