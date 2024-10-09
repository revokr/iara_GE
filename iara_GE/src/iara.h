#pragma once

#include "iara/Core/Log.h"
#include "iara/Core/Layer.h"
#include "iara/Core/Input.h"
#include "iara/events/Event.h"
#include "iara/Core/KeyCodes.h"
#include "iara/Core/Timestep.h"
#include "iara/Core/Application.h"
#include "iara/ImGui/ImGuiLayer.h"
#include "iara/Core/MouseButtonCodes.h"

// ---- Renderer -----
#include "iara/Renderer/Camera.h"
#include "iara/Renderer/Shader.h"
#include "iara/Renderer/Texture.h"
#include "iara/Renderer/Renderer.h"
#include "iara/Renderer/Renderer2D.h"
#include "iara/Renderer/VertexArray.h"
#include "iara/Renderer/Framebuffer.h"
#include "iara/Renderer/RenderCommand.h"
#include "platform/openGL/OpenGLBuffer.h"
#include "iara/Renderer/CameraController.h"
#include "platform/openGL/OpenGLVertexArray.h"

// -----Scene------
#include "iara/Scene/Scene.h"
#include "iara/Scene/Entity.h"
#include "iara/Scene/ScriptableEntity.h"
#include "iara/Scene/Component.h"

