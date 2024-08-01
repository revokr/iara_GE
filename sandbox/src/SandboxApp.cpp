#include "iara.h"

class Sandbox : public iara::Application {
public:
	Sandbox() {}
	~Sandbox() {}

};

iara::Application* CreateApplication() {
	return new Sandbox();
}