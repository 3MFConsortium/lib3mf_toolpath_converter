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


#include "Toolpath_Exporter_Matjob.hpp"
#include "Toolpath_MatjobConst.hpp"


namespace Toolpath {

	CToolpathExporter_Matjob::CToolpathExporter_Matjob()
		: m_dUnits(1.0)
		, m_nLayerCount(0)
		, m_nLayersPerBatch(50)
		, m_dGlobalLaserDiameter(0.1)
	{
	}

	void CToolpathExporter_Matjob::initialize(const std::string& sOutputFileName)
	{
		m_sOutputFileName = sOutputFileName;

		std::wstring sOutputFileNameW = NMR::fnUTF8toUTF16(sOutputFileName);
		m_pExportStream = std::make_shared<NMR::CExportStream_Native>(sOutputFileNameW.c_str());
		m_pMatJobWriter = std::make_unique<CMatJobWriter>(m_pExportStream);
	}

	void CToolpathExporter_Matjob::beginExport(Lib3MF::PToolpath pToolpath, Lib3MF::PModel pModel)
	{
		m_pToolpath = pToolpath;
		m_dUnits = pToolpath->GetUnits();
		m_nLayerCount = pToolpath->GetLayerCount();

		// Build feed factors JSON
		std::stringstream feedFactorStream;
		feedFactorStream << "{";
		for (uint32_t nFeedFactorIndex = 0; nFeedFactorIndex < m_nLayerCount; nFeedFactorIndex++) {
			if (nFeedFactorIndex > 0)
				feedFactorStream << ", ";

			double dZMin = pToolpath->GetLayerZMin(nFeedFactorIndex) * m_dUnits;
			feedFactorStream << "\"" << dZMin << "\": 1.5";
		}
		feedFactorStream << "}";

		// Add default properties
		m_pMatJobWriter->addProperty("feed_factors", feedFactorStream.str(), eMatJobPropertyType::mjpJson);
		m_pMatJobWriter->addProperty("recoater_speed_1", "200", eMatJobPropertyType::mjpInteger);
		m_pMatJobWriter->addProperty("recoater_speed_2", "300", eMatJobPropertyType::mjpInteger);
		m_pMatJobWriter->addProperty("gas_velocity", "100", eMatJobPropertyType::mjpFloat);
		m_pMatJobWriter->addProperty("gas_pressure", "2", eMatJobPropertyType::mjpInteger);
		m_pMatJobWriter->addProperty("gas_humidity", "-11", eMatJobPropertyType::mjpInteger);
		m_pMatJobWriter->addProperty("gas_oxygen", "200", eMatJobPropertyType::mjpInteger);
		m_pMatJobWriter->addProperty("gas_selection_port", "A", eMatJobPropertyType::mjpString);
		m_pMatJobWriter->addProperty("gas_selection_name", "Nitrogen", eMatJobPropertyType::mjpString);
		m_pMatJobWriter->addProperty("buildplate_heater_enable", "false", eMatJobPropertyType::mjpBool);
		m_pMatJobWriter->addProperty("buildplate_temp", "100", eMatJobPropertyType::mjpInteger);

		// Add default scan fields
		m_pMatJobWriter->addScanField("Scan Field 1", 0, 0, 0.0, 0.0, 450.0, 300.0);
		m_pMatJobWriter->addScanField("Scan Field 2", 1, 1, 0.0, 0.0, 450.0, 300.0);
		m_pMatJobWriter->addScanField("Scan Field 3", 2, 2, 0.0, 0.0, 450.0, 300.0);
		m_pMatJobWriter->addScanField("Scan Field 4", 3, 3, 0.0, 0.0, 450.0, 300.0);

		// Add parts from build items
		auto pBuildItems = pModel->GetBuildItems();
		while (pBuildItems->MoveNext()) {
			auto pBuildItem = pBuildItems->GetCurrent();
			auto pObject = pBuildItem->GetObjectResource();
			std::string sName = pObject->GetName();

			if (sName.empty())
				sName = "default name";

			bool bHasUUID = false;
			std::string sUUID = pBuildItem->GetUUID(bHasUUID);
			if (!bHasUUID) {
				throw std::runtime_error("Build item has no UUID");
			}

			m_pMatJobWriter->addPart(sName, sUUID);

		}

		// Add parameter sets from profiles
		uint32_t nProfileCount = pToolpath->GetProfileCount();
		for (uint32_t nProfileIndex = 0; nProfileIndex < nProfileCount; nProfileIndex++) {
			auto pProfile = pToolpath->GetProfile(nProfileIndex);
			std::string sUUID = pProfile->GetUUID();
			std::string sProfileName = pProfile->GetName();

			int64_t nLaserIndex = pProfile->GetParameterIntegerValueDef("", "laserindex", 0);
			double dLaserSpeed = pProfile->GetParameterDoubleValue("", "laserspeed");
			double dLaserPower = pProfile->GetParameterDoubleValue("", "laserpower");
			double dJumpSpeed = pProfile->GetParameterDoubleValueDef("", "jumpspeed", dLaserSpeed);

			auto pParameterSet = m_pMatJobWriter->addParameterSet(sUUID, sProfileName, (uint32_t)nLaserIndex, 
				dLaserSpeed, 0, m_dGlobalLaserDiameter, dLaserPower, dJumpSpeed);

			auto nParameterCount = pProfile->GetParameterCount();
			for (uint32_t nParameterIndex = 0; nParameterIndex < nParameterCount; nParameterIndex++) {
				std::string sParameterName = pProfile->GetParameterName(nParameterIndex);
				std::string sParameterNamespace = pProfile->GetParameterNameSpace(nParameterIndex);				

				if (sParameterNamespace == MATJOB_3MFNAMESPACEDOUBLE) {
					double dParameterValue = pProfile->GetParameterDoubleValue(sParameterNamespace, sParameterName);
					pParameterSet->addProperty(sParameterName, std::to_string(dParameterValue), eMatJobPropertyType::mjpDouble);
				}
				
				if (sParameterNamespace == MATJOB_3MFNAMESPACEINTEGER) {
					int64_t nParameterValue = pProfile->GetParameterIntegerValue(sParameterNamespace, sParameterName);
					pParameterSet->addProperty(sParameterName, std::to_string(nParameterValue), eMatJobPropertyType::mjpInteger);
				}

			}
		}
	}

	void CToolpathExporter_Matjob::processLayer(uint32_t nLayerIndex, Lib3MF::PToolpathLayerReader pLayerReader)
	{
		double dZValue = m_pToolpath->GetLayerZMin(nLayerIndex) * m_dUnits;

		// Start a new binary file batch if needed
		if (nLayerIndex % m_nLayersPerBatch == 0) {
			uint32_t nLayerEndIndexOfBatch = nLayerIndex + m_nLayersPerBatch - 1;
			if (nLayerEndIndexOfBatch >= m_nLayerCount)
				nLayerEndIndexOfBatch = m_nLayerCount - 1;

			double dFromZValueInMM = m_pToolpath->GetLayerZMin(nLayerIndex) * m_dUnits;
			int64_t nFromZValueInMicron = (int64_t)round(dFromZValueInMM * 1000.0);

			double dToZValueInMM = m_pToolpath->GetLayerZMax(nLayerEndIndexOfBatch) * m_dUnits;
			int64_t nToValueInMicron = (int64_t)round(dToZValueInMM * 1000.0);

			m_pCurrentFile = m_pMatJobWriter->beginBinaryFile(
				"layer_from_" + std::to_string(nFromZValueInMicron) + "_to_" + std::to_string(nToValueInMicron) + ".bin");
		}

		m_pCurrentFile->beginLayer(dZValue);
		auto pMatJobLayer = m_pMatJobWriter->beginNewLayer(dZValue);

		uint32_t nSegmentCount = pLayerReader->GetSegmentCount();

		for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
			Lib3MF::eToolpathSegmentType segmentType;
			uint32_t nPointCount = 0;
			pLayerReader->GetSegmentInfo(nSegmentIndex, segmentType, nPointCount);

			// Map Profile and Part references
			std::string sProfileUUID = pLayerReader->GetSegmentDefaultProfileUUID(nSegmentIndex);
			std::string sBuildItemUUID = pLayerReader->GetSegmentBuildItemUUID(nSegmentIndex);
			auto pMatJobPart = m_pMatJobWriter->findPartByBuildItemUUID(sBuildItemUUID);
			auto pMatJobParameterSet = m_pMatJobWriter->findParameterSetByUUID(sProfileUUID);

			pMatJobPart->addCoordinatesZ(dZValue);

			double dMarkSpeed = pMatJobParameterSet->getLaserSpeed();
			double dJumpSpeed = pMatJobParameterSet->getJumpSpeed();

			switch (segmentType) {
			case Lib3MF::eToolpathSegmentType::Loop:
			case Lib3MF::eToolpathSegmentType::Polyline:
			{
				std::vector<Lib3MF::sPosition2D> points;
				pLayerReader->GetSegmentPointDataInModelUnits(nSegmentIndex, points);

				if (points.size() != nPointCount)
					throw std::runtime_error("Point count mismatch reading polyline segment");
				if (nPointCount < 2)
					throw std::runtime_error("Invalid point count in polyline segment");

				if (segmentType == Lib3MF::eToolpathSegmentType::Loop) {
					// Convert loop to polyline by closing it if not already closed
					auto firstPoint = points.at(0);
					auto lastPoint = points.at(points.size() - 1);
					if ((firstPoint.m_Coordinates[0] != lastPoint.m_Coordinates[0]) || 
						(firstPoint.m_Coordinates[1] != lastPoint.m_Coordinates[1])) {
						points.push_back(firstPoint);
					}
				}

				pMatJobLayer->addPolylineDataBlock(pMatJobPart, m_pCurrentFile.get(), pMatJobPart->getPartID(),
					pMatJobParameterSet->getID(), points, dMarkSpeed, dJumpSpeed);
				break;
			}

			case Lib3MF::eToolpathSegmentType::Hatch:
			{
				std::vector<Lib3MF::sHatch2D> hatches;
				pLayerReader->GetSegmentHatchDataInModelUnits(nSegmentIndex, hatches);

				if (hatches.size() * 2 != nPointCount)
					throw std::runtime_error("Point count mismatch reading hatch segment");
				if (nPointCount < 2)
					throw std::runtime_error("Invalid point count in hatch segment");

				pMatJobLayer->addHatchDataBlock(pMatJobPart, m_pCurrentFile.get(), pMatJobPart->getPartID(),
					pMatJobParameterSet->getID(), hatches, dMarkSpeed, dJumpSpeed);
				break;
			}

			default:
				// Ignore other segment types
				break;
			}
		}

		m_pCurrentFile->finishLayer();
	}

	void CToolpathExporter_Matjob::finalize()
	{
		m_pMatJobWriter->writeJobMetaData();
		m_pMatJobWriter->writeContent();
		m_pMatJobWriter->finalize();
	}

	void CToolpathExporter_Matjob::setLayersPerBatch(uint32_t nLayersPerBatch)
	{
		m_nLayersPerBatch = nLayersPerBatch;
	}

	void CToolpathExporter_Matjob::setGlobalLaserDiameter(double dDiameter)
	{
		m_dGlobalLaserDiameter = dDiameter;
	}

} // namespace Toolpath

