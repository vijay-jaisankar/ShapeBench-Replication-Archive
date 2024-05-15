#pragma once

#include <shapeDescriptor/shapeDescriptor.h>
#include "json.hpp"
#include "dataset/Dataset.h"

namespace ShapeBench {
    inline ShapeDescriptor::cpu::Mesh readMeshFile(const std::filesystem::path& meshFilePath, const DatasetEntry &datasetEntry) {
        float computedBoundingSphereRadius = std::max<float>(datasetEntry.computedObjectRadius, 0.0000001);
        ShapeDescriptor::cpu::float3 computedBoundingSphereCentre = datasetEntry.computedObjectCentre;

        ShapeDescriptor::cpu::Mesh mesh = ShapeDescriptor::loadMesh(meshFilePath);

        // Scale mesh down to a unit sphere
        float scaleFactor = 1.0f / float(computedBoundingSphereRadius);
        for (uint32_t i = 0; i < mesh.vertexCount; i++) {
            mesh.vertices[i] = scaleFactor * (mesh.vertices[i] - computedBoundingSphereCentre);
            mesh.normals[i] = normalize(mesh.normals[i]);
        }
        return mesh;
    }

    inline void downloadDatasetMesh(const nlohmann::json &config, std::filesystem::path meshDestinationPath, const DatasetEntry &datasetEntry, bool enableCompression) {
        throw std::runtime_error("Not implemented!");
    }

    inline ShapeDescriptor::cpu::Mesh readDatasetMesh(const nlohmann::json &config, const DatasetEntry &datasetEntry) {
        if(!config.contains("datasetSettings")) {
            throw std::runtime_error("Configuration is missing the key 'datasetSettings'. Aborting.");
        }
        std::filesystem::path datasetBasePath = config.at("datasetSettings").at("objaverseRootDir");
        std::filesystem::path compressedDatasetBasePath = config.at("datasetSettings").at("compressedRootDir");
        const std::filesystem::path &pathInDataset = datasetEntry.meshFile;

        std::filesystem::path originalMeshPath = datasetBasePath / pathInDataset;
        std::filesystem::path compressedMeshPath = compressedDatasetBasePath / pathInDataset;
        compressedMeshPath = compressedMeshPath.replace_extension(".cm");

        bool originalDatabaseMeshExists = std::filesystem::exists(originalMeshPath);
        bool compressedDatabaseMeshExists = std::filesystem::exists(compressedMeshPath);

        bool compressionEnabled = config.at("datasetSettings").at("enableDatasetCompression");
        if(compressionEnabled) {
            if(!compressedDatabaseMeshExists) {
                downloadDatasetMesh(config, compressedMeshPath, datasetEntry, true);
            }
            return readMeshFile(compressedMeshPath, datasetEntry);
        } else {
            if(!originalDatabaseMeshExists) {
                downloadDatasetMesh(config, compressedMeshPath, datasetEntry, false);
            }
            return readMeshFile(originalMeshPath, datasetEntry);
        }
    }
}
