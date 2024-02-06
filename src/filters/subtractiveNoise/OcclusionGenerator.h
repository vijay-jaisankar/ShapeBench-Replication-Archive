#pragma once

#include <shapeDescriptor/shapeDescriptor.h>
#include "json.hpp"
#include "benchmarkCore/ComputedConfig.h"
#include "benchmarkCore/Dataset.h"
#include "GLFW/glfw3.h"
#include "utils/gl/Shader.h"
#include "utils/gl/GeometryBuffer.h"

namespace ShapeBench {
    class OccludedSceneGenerator {
        GLFWwindow* window = nullptr;
        bool isDestroyed = false;
        Shader objectIDShader;
        Shader fullscreenQuadShader;
        uint32_t frameBufferID = 0;
        uint32_t renderBufferID = 0;
        uint32_t renderTextureID = 0;
        std::vector<unsigned char> localFramebufferCopy;
        GeometryBuffer screenQuadVAO;
        uint32_t offscreenTextureWidth = 0;
        uint32_t offscreenTextureHeight = 0;

    public:
        explicit OccludedSceneGenerator(const nlohmann::json& config, const nlohmann::json& computedConfig);
        ~OccludedSceneGenerator();
        ShapeDescriptor::cpu::Mesh computeOccludedMesh(ShapeDescriptor::cpu::Mesh mesh, uint64_t seed);
        void init();
        void destroy();
    };
}
