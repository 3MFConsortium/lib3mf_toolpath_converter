/*++

Copyright (C) 2025 3MF Consortium

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Autodesk Inc. nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL AUTODESK INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <iostream>
#include <vector>
#include "lib3mf_dynamic.hpp"

#include <map>
#include <cmath>
#include <sstream>

#include "Common/NMR_StringUtils.h"
#include "Common/Platform/NMR_ExportStream_Native.h"


#include "Toolpath_MatjobWriter.hpp"
#include "Toolpath_MatjobBinaryFile.hpp"

using namespace Toolpath;

int main(int argc, char* argv[])
{
    try {
        auto p3MFWrapper = Lib3MF::CWrapper::loadLibrary("lib3mf_win64.dll");

		std::string sInputFileName;
		std::string sOutputFileName;

		std::vector<std::string> commandArguments;
		for (int idx = 1; idx < argc; idx++)
			commandArguments.push_back(argv[idx]);

		for (size_t nIndex = 0; nIndex < commandArguments.size(); nIndex++) {
			std::string sArgument = commandArguments[nIndex];

			if (sArgument == "--input") {
				nIndex++;
				if (nIndex >= commandArguments.size())
					throw std::runtime_error("missing --input path");

				sInputFileName = commandArguments[nIndex];
			}

			if (sArgument == "--output") {
				nIndex++;
				if (nIndex >= commandArguments.size())
					throw std::runtime_error("missing --output path");

				sOutputFileName = commandArguments[nIndex];
			}

		}

		std::cout << "Input filename: " << sInputFileName << "\n";
		std::cout << "Output filename: " << sOutputFileName << "\n";

		if (sInputFileName.empty() || sOutputFileName.empty())
			throw std::runtime_error("Usage: converter.exe --input toolpath.3mf --output matjob.job");

		std::cout << "Reading 3MF file " << sInputFileName << "\n";

		auto pLib3MFWrapper = Lib3MF::CWrapper::loadLibrary("lib3mf_win64.dll");
		auto pModel = pLib3MFWrapper->CreateModel();

		auto pSource = pModel->CreatePersistentSourceFromFile(sInputFileName);
		auto pReader = pModel->QueryReader("3mf");
		pReader->ReadFromPersistentSource(pSource);

		std::cout << "3MF File opened..\n";


		auto pLib3MFToolpaths = pModel->GetToolpaths();
		if (!pLib3MFToolpaths->MoveNext()) {
			throw std::runtime_error("No toolpath data found in 3MF file.");
		}
		auto pLib3MFToolpath = pLib3MFToolpaths->GetCurrentToolpath();

		if (pLib3MFToolpaths->MoveNext()) {
			throw std::runtime_error("Multiple toolpath data sets found in 3MF file. Only one is supported.");
		}

		double dUnits = pLib3MFToolpath->GetUnits();
		uint32_t nLayerCount = pLib3MFToolpath->GetLayerCount();

		std::cout << "Layer Count: " << nLayerCount << ", Units: " << dUnits << "\n";


		//----------------------- FROM HERE PLEASE REPLACE WITH ABSTRACT INTERFACE ----------------------------

		std::cout << "Writing MatJob file " << sOutputFileName << "\n";

		std::wstring sOutputFileNameW = NMR::fnUTF8toUTF16(sOutputFileName);

		auto pExportStream = std::make_shared<NMR::CExportStream_Native>(sOutputFileNameW.c_str());


		CMatJobWriter matJobWriter(pExportStream);

		std::stringstream feedFactorStream;
		feedFactorStream << "{";
		for (uint32_t nFeedFactorIndex = 0; nFeedFactorIndex < nLayerCount; nFeedFactorIndex++) {
			if (nFeedFactorIndex > 0)
				feedFactorStream << ", ";

			double dZMin = pLib3MFToolpath->GetLayerZMin(nFeedFactorIndex) * dUnits;
			feedFactorStream << "\"" << dZMin << "\": 1.5";

		}
		feedFactorStream << "}";


		matJobWriter.addProperty("feed_factors", feedFactorStream.str(), eMatJobPropertyType::mjpJson);
		matJobWriter.addProperty("recoater_speed_1", "200", eMatJobPropertyType::mjpInteger);
		matJobWriter.addProperty("recoater_speed_2", "300", eMatJobPropertyType::mjpInteger);
		matJobWriter.addProperty("gas_velocity", "100", eMatJobPropertyType::mjpFloat);
		matJobWriter.addProperty("gas_pressure", "2", eMatJobPropertyType::mjpInteger);
		matJobWriter.addProperty("gas_humidity", "-11", eMatJobPropertyType::mjpInteger);
		matJobWriter.addProperty("gas_oxygen", "200", eMatJobPropertyType::mjpInteger);
		matJobWriter.addProperty("gas_selection_port", "A", eMatJobPropertyType::mjpString);
		matJobWriter.addProperty("gas_selection_name", "Nitrogen", eMatJobPropertyType::mjpString);
		matJobWriter.addProperty("buildplate_heater_enable", "false", eMatJobPropertyType::mjpBool);
		matJobWriter.addProperty("buildplate_temp", "100", eMatJobPropertyType::mjpInteger);

		matJobWriter.addScanField("Scan Field 1", 0, 0, 0.0, 0.0, 450.0, 300.0);
		matJobWriter.addScanField("Scan Field 2", 1, 1, 0.0, 0.0, 450.0, 300.0);
		matJobWriter.addScanField("Scan Field 3", 2, 2, 0.0, 0.0, 450.0, 300.0);
		matJobWriter.addScanField("Scan Field 4", 3, 3, 0.0, 0.0, 450.0, 300.0);

		auto pBuildItems = pModel->GetBuildItems();
		while (pBuildItems->MoveNext()) {
			auto pBuildItem = pBuildItems->GetCurrent();
			auto pObject = pBuildItem->GetObjectResource();
			std::string sName = pObject->GetName();
			auto outbox = pObject->GetOutbox();

			outbox.m_MinCoordinate[0] = 10.0;
			outbox.m_MinCoordinate[1] = 20.0;
			outbox.m_MinCoordinate[2] = 30.0;
			outbox.m_MaxCoordinate[0] = 100.0;
			outbox.m_MaxCoordinate[1] = 200.0;
			outbox.m_MaxCoordinate[2] = 300.0;

			if (sName.empty())
				sName = "default name";

			bool bHasUUID = false;
			std::string sUUID = pBuildItem->GetUUID(bHasUUID);
			if (!bHasUUID) {
				throw std::runtime_error("Build item has no UUID");
			}

			matJobWriter.addPart(sName, sUUID, outbox.m_MinCoordinate[0], outbox.m_MinCoordinate[1], outbox.m_MinCoordinate[2], outbox.m_MaxCoordinate[0], outbox.m_MaxCoordinate[1], outbox.m_MaxCoordinate[2]);
		}


		double dGlobalLaserDiameter = 0.1;

		uint32_t nProfileCount = pLib3MFToolpath->GetProfileCount();
		for (uint32_t nProfileIndex = 0; nProfileIndex < nProfileCount; nProfileIndex++) {
			auto pProfile = pLib3MFToolpath->GetProfile(nProfileIndex);
			std::string sUUID = pProfile->GetUUID();
			std::string sProfileName = pProfile->GetName();

			int64_t nLaserIndex = pProfile->GetParameterIntegerValueDef("", "laserindex", 0);
			double dLaserSpeed = pProfile->GetParameterDoubleValue("", "laserspeed");
			double dLaserPower = pProfile->GetParameterDoubleValue("", "laserpower");
			double dJumpSpeed = pProfile->GetParameterDoubleValueDef("", "jumpspeed", dLaserSpeed);

			auto pParameterSet = matJobWriter.addParameterSet(sUUID, sProfileName, (uint32_t)nLaserIndex, dLaserSpeed, 0, dGlobalLaserDiameter, dLaserPower, dJumpSpeed);
		}


		uint32_t nLayersPerBatch = 50;

		PMatJobBinaryFile pCurrentFile = nullptr;

		for (uint32_t nLayerIndex = 0; nLayerIndex < nLayerCount; nLayerIndex++) {

			double dZValue = pLib3MFToolpath->GetLayerZMin(nLayerIndex) * dUnits;

			if (nLayerIndex % nLayersPerBatch == 0) {

				uint32_t nLayerEndIndexOfBatch = nLayerIndex + nLayersPerBatch - 1;
				if (nLayerEndIndexOfBatch >= nLayerCount)
					nLayerEndIndexOfBatch = nLayerCount - 1;

				double dFromZValueInMM = pLib3MFToolpath->GetLayerZMin(nLayerIndex) * dUnits;
				int64_t nFromZValueInMicron = (int64_t)round(dFromZValueInMM * 1000.0);

				double dToZValueInMM = pLib3MFToolpath->GetLayerZMax(nLayerEndIndexOfBatch) * dUnits;
				int64_t nToValueInMicron = (int64_t)round(dToZValueInMM * 1000.0);

				pCurrentFile = matJobWriter.beginBinaryFile("layer_from_" + std::to_string(nFromZValueInMicron) + "_to_" + std::to_string(nToValueInMicron) + ".bin");

			}


			pCurrentFile->beginLayer(dZValue);

			auto pMatJobLayer = matJobWriter.beginNewLayer(dZValue);

			auto p3MFLayer = pLib3MFToolpath->ReadLayerData(nLayerIndex);

			double dUnits = pLib3MFToolpath->GetUnits();

			uint32_t nSegmentCount = p3MFLayer->GetSegmentCount();

			for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
				Lib3MF::eToolpathSegmentType segmentType;
				uint32_t nPointCount = 0;
				p3MFLayer->GetSegmentInfo(nSegmentIndex, segmentType, nPointCount);


				// Map Profile and Part references
				std::string sProfileUUID = p3MFLayer->GetSegmentDefaultProfileUUID(nSegmentIndex);
				std::string sBuildItemUUID = p3MFLayer->GetSegmentBuildItemUUID(nSegmentIndex);
				auto pMatJobPart = matJobWriter.findPartByBuildItemUUID(sBuildItemUUID);
				auto pMatJobParameterSet = matJobWriter.findParameterSetByUUID(sProfileUUID);

				double dMarkSpeed = pMatJobParameterSet->getLaserSpeed();
				double dJumpSpeed = pMatJobParameterSet->getJumpSpeed();

				//				std::cout << "Layer " << nLayerIndex << ", Segment " << nSegmentIndex << ": Type " << (uint32_t)segmentType << ", Points: " << nPointCount << ", Part ID: " << pMatJobPart->getPartID() << ", Profile ID: " << pMatJobParameterSet->getID() << ", Mark Speed: " << dMarkSpeed << "\n";

				switch (segmentType) {
				case Lib3MF::eToolpathSegmentType::Loop:
				case Lib3MF::eToolpathSegmentType::Polyline:
				{
					std::vector<Lib3MF::sPosition2D> points;
					p3MFLayer->GetSegmentPointDataInModelUnits(nSegmentIndex, points);

					if (points.size() != nPointCount)
						throw std::runtime_error("Point count mismatch reading polyline segment");
					if (nPointCount < 2)
						throw std::runtime_error("Invalid point count in polyline segment");

					if (segmentType == Lib3MF::eToolpathSegmentType::Loop) {
						// Convert loop to polyline
						auto firstPoint = points.at(0);
						auto lastPoint = points.at(points.size() - 1);
						if ((firstPoint.m_Coordinates[0] != lastPoint.m_Coordinates[0]) || (firstPoint.m_Coordinates[1] != lastPoint.m_Coordinates[1])) {
							// Close polyline if not already closed
							points.push_back(firstPoint);
						}

					}

					pMatJobLayer->addPolylineDataBlock(pCurrentFile.get(), pMatJobPart->getPartID(), pMatJobParameterSet->getID(), points, dMarkSpeed, dJumpSpeed);

					break;
				}

				case Lib3MF::eToolpathSegmentType::Hatch:
				{
					std::vector<Lib3MF::sHatch2D> hatches;
					p3MFLayer->GetSegmentHatchDataInModelUnits(nSegmentIndex, hatches);

					if (hatches.size() * 2 != nPointCount)
						throw std::runtime_error("Point count mismatch reading hatch segment");
					if (nPointCount < 2)
						throw std::runtime_error("Invalid point count in hatch segment");

					pMatJobLayer->addHatchDataBlock(pCurrentFile.get(), pMatJobPart->getPartID(), pMatJobParameterSet->getID(), hatches, dMarkSpeed, dJumpSpeed);

					break;
				}

				}
			}

			pCurrentFile->finishLayer();


		}


		matJobWriter.writeJobMetaData();

		matJobWriter.writeContent();


		std::cout << "Done.\n";



    }
    catch (std::exception& E) {
        std::cout << "fatal error: " << E.what() << std::endl;
    }
    
}

