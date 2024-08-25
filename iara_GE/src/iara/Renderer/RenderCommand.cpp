#include "ir_pch.h"
#include "RenderCommand.h"
#include "platform/openGL/OpenGLRendererAPI.h"

namespace iara {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();	

}