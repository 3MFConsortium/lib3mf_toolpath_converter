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


#include "Toolpath_Exporter_CLIPlus.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <limits>
#include <cfloat>

namespace Toolpath {

	CToolpathExporter_CLIPlus::CToolpathExporter_CLIPlus()
		: m_dUnits(1.0)
		, m_nLayerCount(0)
		, m_dMinX(DBL_MAX)
		, m_dMinY(DBL_MAX)
		, m_dMinZ(DBL_MAX)
		, m_dMaxX(-DBL_MAX)
		, m_dMaxY(-DBL_MAX)
		, m_dMaxZ(-DBL_MAX)
		, m_nNextPartID(1)
		, m_nNextProfileID(1)
		, m_bIncludeLaserParams(true)
	{
	}

	void CToolpathExporter_CLIPlus::initialize(const std::string& sOutputFileName)
	{
		m_sOutputFileName = sOutputFileName;
		std::cout << "Writing CLI+ file " << sOutputFileName << "\n";
	}

	void CToolpathExporter_CLIPlus::beginExport(Lib3MF::PToolpath pToolpath, Lib3MF::PModel pModel)
	{
		m_pToolpath = pToolpath;
		m_dUnits = pToolpath->GetUnits();
		m_nLayerCount = pToolpath->GetLayerCount();

		// Calculate bounding box from layers
		for (uint32_t i = 0; i < m_nLayerCount; i++) {
			double zMin = pToolpath->GetLayerZMin(i) * m_dUnits;
			double zMax = pToolpath->GetLayerZMax(i) * m_dUnits;
			if (zMin < m_dMinZ) m_dMinZ = zMin;
			if (zMax > m_dMaxZ) m_dMaxZ = zMax;
		}

		// Pre-register parts from build items
		auto pBuildItems = pModel->GetBuildItems();
		while (pBuildItems->MoveNext()) {
			auto pBuildItem = pBuildItems->GetCurrent();
			bool bHasUUID = false;
			std::string sUUID = pBuildItem->GetUUID(bHasUUID);
			if (bHasUUID) {
				getOrCreatePartID(sUUID);

				// Update bounding box from part outbox
				auto pObject = pBuildItem->GetObjectResource();
				auto outbox = pObject->GetOutbox();
				if (outbox.m_MinCoordinate[0] < m_dMinX) m_dMinX = outbox.m_MinCoordinate[0];
				if (outbox.m_MinCoordinate[1] < m_dMinY) m_dMinY = outbox.m_MinCoordinate[1];
				if (outbox.m_MaxCoordinate[0] > m_dMaxX) m_dMaxX = outbox.m_MaxCoordinate[0];
				if (outbox.m_MaxCoordinate[1] > m_dMaxY) m_dMaxY = outbox.m_MaxCoordinate[1];
			}
		}

		// Pre-register profiles
		uint32_t nProfileCount = pToolpath->GetProfileCount();
		for (uint32_t i = 0; i < nProfileCount; i++) {
			auto pProfile = pToolpath->GetProfile(i);
			getOrCreateProfileID(pProfile->GetUUID());
		}
	}

	void CToolpathExporter_CLIPlus::processLayer(uint32_t nLayerIndex, Lib3MF::PToolpathLayerReader pLayerReader)
	{
		double dZValue = m_pToolpath->GetLayerZMax(nLayerIndex) * m_dUnits;

		// Write layer start command
		m_GeometryBuffer << "$$LAYER/" << std::fixed << std::setprecision(6) << dZValue << "\n";

		uint32_t nSegmentCount = pLayerReader->GetSegmentCount();

		for (uint32_t nSegmentIndex = 0; nSegmentIndex < nSegmentCount; nSegmentIndex++) {
			Lib3MF::eToolpathSegmentType segmentType;
			uint32_t nPointCount = 0;
			pLayerReader->GetSegmentInfo(nSegmentIndex, segmentType, nPointCount);

			// Get part and profile IDs
			std::string sProfileUUID = pLayerReader->GetSegmentDefaultProfileUUID(nSegmentIndex);
			std::string sBuildItemUUID = pLayerReader->GetSegmentBuildItemUUID(nSegmentIndex);
			uint32_t nPartID = getOrCreatePartID(sBuildItemUUID);
			uint32_t nProfileID = getOrCreateProfileID(sProfileUUID);

			// Get laser parameters if we want to include them (CLI+ extension)
			double dLaserPower = 0.0;
			double dLaserSpeed = 0.0;
			if (m_bIncludeLaserParams && !sProfileUUID.empty()) {
				auto pProfile = m_pToolpath->GetProfileByUUID(sProfileUUID);
				if (pProfile) {
					dLaserPower = pProfile->GetParameterDoubleValueDef("", "laserpower", 0.0);
					dLaserSpeed = pProfile->GetParameterDoubleValueDef("", "laserspeed", 0.0);
				}
			}

			switch (segmentType) {
			case Lib3MF::eToolpathSegmentType::Loop:
			case Lib3MF::eToolpathSegmentType::Polyline:
			{
				std::vector<Lib3MF::sPosition2D> points;
				pLayerReader->GetSegmentPointDataInModelUnits(nSegmentIndex, points);

				if (points.size() < 2)
					continue;

				// Determine direction
				int nDir = (segmentType == Lib3MF::eToolpathSegmentType::Loop) 
					? static_cast<int>(eCLIPolylineDirection::CounterClockwise)
					: static_cast<int>(eCLIPolylineDirection::Open);

				// For loops, ensure the polyline is closed
				if (segmentType == Lib3MF::eToolpathSegmentType::Loop) {
					auto firstPoint = points.front();
					auto lastPoint = points.back();
					if ((firstPoint.m_Coordinates[0] != lastPoint.m_Coordinates[0]) ||
						(firstPoint.m_Coordinates[1] != lastPoint.m_Coordinates[1])) {
						points.push_back(firstPoint);
					}
				}

				// Write polyline command
				// $$POLYLINE/id,dir,n,x1,y1,x2,y2,...
				m_GeometryBuffer << "$$POLYLINE/" << nPartID << "," << nDir << "," << points.size();
				for (const auto& pt : points) {
					m_GeometryBuffer << "," << std::fixed << std::setprecision(6) 
						<< pt.m_Coordinates[0] << "," << pt.m_Coordinates[1];
				}
				m_GeometryBuffer << "\n";

				// CLI+ extension: Add laser parameters as comment
				if (m_bIncludeLaserParams && (dLaserPower > 0 || dLaserSpeed > 0)) {
					m_GeometryBuffer << "// PROFILE=" << nProfileID 
						<< " POWER=" << dLaserPower 
						<< " SPEED=" << dLaserSpeed << " //\n";
				}
				break;
			}

			case Lib3MF::eToolpathSegmentType::Hatch:
			{
				std::vector<Lib3MF::sHatch2D> hatches;
				pLayerReader->GetSegmentHatchDataInModelUnits(nSegmentIndex, hatches);

				if (hatches.empty())
					continue;

				// Write hatches command
				// $$HATCHES/id,n,x1s,y1s,x1e,y1e,x2s,y2s,x2e,y2e,...
				m_GeometryBuffer << "$$HATCHES/" << nPartID << "," << hatches.size();
				for (const auto& hatch : hatches) {
					m_GeometryBuffer << "," << std::fixed << std::setprecision(6)
						<< hatch.m_Point1Coordinates[0] << "," << hatch.m_Point1Coordinates[1] << ","
						<< hatch.m_Point2Coordinates[0] << "," << hatch.m_Point2Coordinates[1];
				}
				m_GeometryBuffer << "\n";

				// CLI+ extension: Add laser parameters as comment
				if (m_bIncludeLaserParams && (dLaserPower > 0 || dLaserSpeed > 0)) {
					m_GeometryBuffer << "// PROFILE=" << nProfileID 
						<< " POWER=" << dLaserPower 
						<< " SPEED=" << dLaserSpeed << " //\n";
				}
				break;
			}

			default:
				// Ignore other segment types
				break;
			}
		}
	}

	void CToolpathExporter_CLIPlus::finalize()
	{
		// Open the output file
		m_OutputStream.open(m_sOutputFileName, std::ios::out | std::ios::trunc);
		if (!m_OutputStream.is_open()) {
			throw std::runtime_error("Failed to open output file: " + m_sOutputFileName);
		}

		// Write header
		writeHeader();

		// Write geometry section
		writeGeometryStart();
		m_OutputStream << m_GeometryBuffer.str();
		writeGeometryEnd();

		m_OutputStream.close();
		std::cout << "CLI+ export complete.\n";
	}

	void CToolpathExporter_CLIPlus::writeHeader()
	{
		m_OutputStream << "$$HEADERSTART\n";
		m_OutputStream << "$$ASCII\n";
		m_OutputStream << "$$UNITS/" << std::fixed << std::setprecision(6) << 1.0 << "\n"; // Units in mm
		m_OutputStream << "$$VERSION/200\n"; // CLI version 2.00

		// Get current date
		std::time_t now = std::time(nullptr);
		std::tm* ltm = std::localtime(&now);
		int dateValue = (ltm->tm_mday * 10000) + ((ltm->tm_mon + 1) * 100) + (ltm->tm_year % 100);
		m_OutputStream << "$$DATE/" << dateValue << "\n";

		// Write dimension (bounding box)
		if (m_dMinX < m_dMaxX && m_dMinY < m_dMaxY && m_dMinZ < m_dMaxZ) {
			m_OutputStream << "$$DIMENSION/" 
				<< std::fixed << std::setprecision(6)
				<< m_dMinX << "," << m_dMinY << "," << m_dMinZ << ","
				<< m_dMaxX << "," << m_dMaxY << "," << m_dMaxZ << "\n";
		}

		m_OutputStream << "$$LAYERS/" << m_nLayerCount << "\n";

		// Write labels for parts
		for (const auto& partEntry : m_PartIDMap) {
			m_OutputStream << "$$LABEL/" << partEntry.second << ",part_" << partEntry.second << "\n";
		}

		// CLI+ extension: Write profile information as user data
		if (m_bIncludeLaserParams && m_pToolpath) {
			m_OutputStream << "// CLI+ EXTENSION: PROFILE DEFINITIONS //\n";
			uint32_t nProfileCount = m_pToolpath->GetProfileCount();
			for (uint32_t i = 0; i < nProfileCount; i++) {
				auto pProfile = m_pToolpath->GetProfile(i);
				std::string sUUID = pProfile->GetUUID();
				std::string sName = pProfile->GetName();
				double dPower = pProfile->GetParameterDoubleValueDef("", "laserpower", 0.0);
				double dSpeed = pProfile->GetParameterDoubleValueDef("", "laserspeed", 0.0);

				uint32_t nProfileID = m_ProfileIDMap[sUUID];
				m_OutputStream << "// PROFILE_DEF=" << nProfileID 
					<< " NAME=\"" << sName << "\""
					<< " POWER=" << dPower 
					<< " SPEED=" << dSpeed << " //\n";
			}
		}

		m_OutputStream << "$$HEADEREND\n";
	}

	void CToolpathExporter_CLIPlus::writeGeometryStart()
	{
		m_OutputStream << "$$GEOMETRYSTART\n";
	}

	void CToolpathExporter_CLIPlus::writeGeometryEnd()
	{
		m_OutputStream << "$$GEOMETRYEND\n";
	}

	uint32_t CToolpathExporter_CLIPlus::getOrCreatePartID(const std::string& sBuildItemUUID)
	{
		if (sBuildItemUUID.empty()) {
			return 0; // Default part ID
		}

		auto it = m_PartIDMap.find(sBuildItemUUID);
		if (it != m_PartIDMap.end()) {
			return it->second;
		}

		uint32_t nID = m_nNextPartID++;
		m_PartIDMap[sBuildItemUUID] = nID;
		return nID;
	}

	uint32_t CToolpathExporter_CLIPlus::getOrCreateProfileID(const std::string& sProfileUUID)
	{
		if (sProfileUUID.empty()) {
			return 0; // Default profile ID
		}

		auto it = m_ProfileIDMap.find(sProfileUUID);
		if (it != m_ProfileIDMap.end()) {
			return it->second;
		}

		uint32_t nID = m_nNextProfileID++;
		m_ProfileIDMap[sProfileUUID] = nID;
		return nID;
	}

	void CToolpathExporter_CLIPlus::setIncludeLaserParams(bool bInclude)
	{
		m_bIncludeLaserParams = bInclude;
	}

} // namespace Toolpath

