#pragma once

#include "json.hpp"
#include "results/ExperimentResult.h"
#include <tabulate/table.hpp>

namespace ShapeBench {
    template<typename T>
    struct SinglePropertyStatistics {
        uint32_t count = 0;
        uint32_t currentCountIndentical = 0;
        double currentAverage = 0;
        double currentSum = 0;
        double currentMax = -std::numeric_limits<double>::max();

        double max() {
            return currentMax;
        }
        double average() {
            return currentAverage;
        }
        double sum() {
            return currentSum;
        }
        std::string countIdentical() {
            return std::to_string(currentCountIndentical) + " / " + std::to_string(count);
        }

        void registerValue(double deviation) {
            if(deviation == 0) {
                currentCountIndentical++;
            }
            count++;
            currentSum += deviation;
            currentMax = std::max<double>(currentMax, deviation);
            currentAverage += (double(deviation) - currentAverage) / double(count);
        }

        void registerValue(T value1, T value2) {
            double deviation = double(value1) - double(value2);
            registerValue(deviation);
        }
    };

    inline void checkReplicatedExperimentResults(const ShapeBench::ExperimentResult& replicatedResults,
                                                 const nlohmann::json& previouslyComputedResults) {
        std::cout << "Validating replicated results.." << std::endl;

        uint32_t validatedResultCount = 0;
        SinglePropertyStatistics<double> clutterStats;
        SinglePropertyStatistics<double> occlusionStats;
        SinglePropertyStatistics<uint32_t> descriptorRankStats;
        SinglePropertyStatistics<double> meshIDCorrectStats;
        SinglePropertyStatistics<double> nearestNeighbourPRCStats;
        SinglePropertyStatistics<double> secondNearestNeighbourPRCStats;

        std::vector<SinglePropertyStatistics<double>> filterProperties;
        std::vector<std::string> filterPropertyNames;

        // I forgot to include an ID of some kind into the results.
        // Since the chosen vertex and mesh ID are
        uint32_t excludedResultCount = 0;

        for(uint32_t vertexIndex = 0; vertexIndex < replicatedResults.vertexResults.size(); vertexIndex++) {
            const ExperimentResultsEntry& replicatedResult = replicatedResults.vertexResults.at(vertexIndex);
            const nlohmann::json& originalResult = previouslyComputedResults.at(vertexIndex);

            if(!replicatedResult.included) {
                continue;
            }

            uint32_t propertyIndex = 0;
            auto originalResultFilterOutputIterator = originalResult["filterOutput"].begin();
            for(const auto& [key, value] : replicatedResult.filterOutput.items()) {
                if(!value.is_number()) {
                    continue;
                }
                if(validatedResultCount == 0) {
                    filterProperties.emplace_back();
                    filterPropertyNames.push_back("Filter Specific Property \"" + key + "\"");
                }
                filterProperties.at(propertyIndex).registerValue(value, originalResultFilterOutputIterator.value());
                propertyIndex++;
                originalResultFilterOutputIterator++;
            }

            validatedResultCount++;

            clutterStats.registerValue(originalResult["fractionAddedNoise"], replicatedResult.fractionAddedNoise);
            occlusionStats.registerValue(originalResult["fractionSurfacePartiality"], replicatedResult.fractionSurfacePartiality);
            descriptorRankStats.registerValue(originalResult["filteredDescriptorRank"], replicatedResult.filteredDescriptorRank);
            meshIDCorrectStats.registerValue(originalResult["meshID"] == replicatedResult.sourceVertex.meshID ? 0 : 1);
            nearestNeighbourPRCStats.registerValue(originalResult["PRC"]["distanceToNearestNeighbour"], replicatedResult.prcMetadata.distanceToNearestNeighbour);
            secondNearestNeighbourPRCStats.registerValue(originalResult["PRC"]["distanceToSecondNearestNeighbour"], replicatedResult.prcMetadata.distanceToSecondNearestNeighbour);


            /*
             * entryJson["fractionAddedNoise"] = entry.fractionAddedNoise;
             * entryJson["fractionSurfacePartiality"] = entry.fractionSurfacePartiality;
             * entryJson["filteredDescriptorRank"] = entry.filteredDescriptorRank;
            entryJson["originalVertex"] = toJSON(entry.originalVertexLocation.vertex);
            entryJson["originalNormal"] = toJSON(entry.originalVertexLocation.normal);
            entryJson["filteredVertex"] = toJSON(entry.filteredVertexLocation.vertex);
            entryJson["filteredNormal"] = toJSON(entry.filteredVertexLocation.normal);
            entryJson["meshID"] = entry.sourceVertex.meshID;
            entryJson["vertexIndex"] = entry.sourceVertex.vertexIndex;
            entryJson["filterOutput"] = entry.filterOutput;
            if(isPRCEnabled) {
                entryJson["PRC"]["distanceToNearestNeighbour"] = entry.prcMetadata.distanceToNearestNeighbour;
                entryJson["PRC"]["distanceToSecondNearestNeighbour"] = entry.prcMetadata.distanceToSecondNearestNeighbour;
                entryJson["PRC"]["modelPointMeshID"] = entry.prcMetadata.modelPointMeshID;
                entryJson["PRC"]["scenePointMeshID"] = entry.prcMetadata.scenePointMeshID;
                entryJson["PRC"]["nearestNeighbourVertexModel"] = toJSON(entry.prcMetadata.nearestNeighbourVertexModel);
                entryJson["PRC"]["nearestNeighbourVertexScene"] = toJSON(entry.prcMetadata.nearestNeighbourVertexScene);
            }*/


        }

        std::cout << "    Validation complete." << std::endl;
        std::cout << "    Table: Overview over the extent to which the replicated and original values deviate from each other." << std::endl;
        std::cout << "           Total deviation is the sum of differences between all replicated values." << std::endl;


        tabulate::Table outputTable;
        outputTable.add_row({"Result", "Total Deviation", "Average Deviation", "Maximum Deviation", "Number of Identical Results"});

        outputTable.add_row(tabulate::RowStream{} << "Clutter" << clutterStats.sum() << clutterStats.average() << clutterStats.max() << clutterStats.countIdentical());
        outputTable.add_row(tabulate::RowStream{} << "Occlusion" << occlusionStats.sum() << occlusionStats.average() << occlusionStats.max() << occlusionStats.countIdentical());
        outputTable.add_row(tabulate::RowStream{} << "Descriptor Distance Index" << descriptorRankStats.sum() << descriptorRankStats.average() << descriptorRankStats.max() << descriptorRankStats.countIdentical());
        outputTable.add_row(tabulate::RowStream{} << "Distance to Nearest Neighbour (PRC)" << nearestNeighbourPRCStats.sum() << nearestNeighbourPRCStats.average() << nearestNeighbourPRCStats.max() << nearestNeighbourPRCStats.countIdentical());
        outputTable.add_row(tabulate::RowStream{} << "Distance to Second Nearest Neighbour (PRC)" << secondNearestNeighbourPRCStats.sum() << secondNearestNeighbourPRCStats.average() << secondNearestNeighbourPRCStats.max() << secondNearestNeighbourPRCStats.countIdentical());
        for(uint32_t i = 0; i < filterProperties.size(); i++) {
            outputTable.add_row(tabulate::RowStream{} << filterPropertyNames.at(i) << filterProperties.at(i).sum() << filterProperties.at(i).average() << filterProperties.at(i).max() << filterProperties.at(i).countIdentical());
        }


        std::cout << outputTable << std::endl << std::endl;

        std::cout << "    Validated " << validatedResultCount << " results." << std::endl;
    }
}
