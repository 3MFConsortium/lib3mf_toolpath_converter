/*++

Copyright (C) 2026 3MF Consortium

All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--*/

#ifndef __TOOLPATH_MATJOBLAYER
#define __TOOLPATH_MATJOBLAYER

#include <string>
#include <cstdint>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>

#include "Toolpath_MatjobBinaryFile.hpp"

#include "lib3mf_dynamic.hpp"

namespace Toolpath
{
	// Helper function to format double with 4 decimal places
	inline std::string formatDouble4Layer(double value) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(4) << value;
		return oss.str();
	}


	typedef struct _sMatJobDataBlock {
		uint32_t m_nPartID;
		uint32_t m_nParameterSetID;
		uint32_t m_nVectorTypeID;
		double m_dMarkDistance;
		double m_dJumpDistance;
		uint32_t m_nNumMarkSegments;
		uint32_t m_nNumJumpSegments;
		uint32_t m_nFileID;
		uint64_t m_nDataPosition;
	} sMatJobDataBlock;


	class CMatJobLayer {
	private:
		double m_dZValue;
		double m_dLayerScanTime;
		double m_dTotalMarkDistance;
		double m_dTotalJumpDistance;
		double m_dMinX;
		double m_dMinY;
		double m_dMaxX;
		double m_dMaxY;

		double m_dCurrentX;
		double m_dCurrentY;
		bool m_bIsFirstMoveInLayer;
		bool m_bIsFirstMoveInBlock;

		uint32_t m_nCurrentNumMarkSegments;
		uint32_t m_nCurrentNumJumpSegments;
		double m_dCurrentMarkDistance;
		double m_dCurrentJumpDistance;

		std::vector <sMatJobDataBlock> m_DataBlocks;

		void moveTo(double dX, double dY, double dSpeedInMMperS, bool bDoMark)
		{

			if (m_bIsFirstMoveInLayer) {
				m_dMinX = dX;
				m_dMaxX = dX;
				m_dMinY = dY;
				m_dMaxY = dY;
			}
			else {

				double dDeltaX = dX - m_dCurrentX;
				double dDeltaY = dY - m_dCurrentY;
				double dDistance = sqrt((dDeltaX * dDeltaX) + (dDeltaY * dDeltaY));
				if (dSpeedInMMperS > 0.0) {
					double dTime = dDistance / dSpeedInMMperS;
					m_dLayerScanTime += dTime;
				}

				if (bDoMark) {
					m_dTotalMarkDistance += dDistance;
					if (m_bIsFirstMoveInLayer)
						throw std::runtime_error("first move in block is not a jump move!");

					m_dCurrentMarkDistance += dDistance;
					m_nCurrentNumMarkSegments++;
				}
				else {
					m_dTotalJumpDistance += dDistance;
					if (!m_bIsFirstMoveInLayer) {
						m_dCurrentJumpDistance += dDistance;
						m_dCurrentJumpDistance++;
					}
				}

				if (dX < m_dMinX)
					m_dMinX = dX;
				if (dX > m_dMaxX)
					m_dMaxX = dX;
				if (dY < m_dMinY)
					m_dMinY = dY;
				if (dY > m_dMaxY)
					m_dMaxY = dY;
			}

			m_dCurrentX = dX;
			m_dCurrentY = dY;
			m_bIsFirstMoveInBlock = false;
			m_bIsFirstMoveInLayer = false;
		}


	public:

		CMatJobLayer(double dZValue)
			: m_dZValue(dZValue),
			m_dLayerScanTime(0.0),
			m_dTotalMarkDistance(0.0),
			m_dTotalJumpDistance(0.0),
			m_dMinX(0.0),
			m_dMinY(0.0),
			m_dMaxX(0.0),
			m_dMaxY(0.0),
			m_dCurrentX(0.0),
			m_dCurrentY(0.0),
			m_dCurrentJumpDistance(0.0),
			m_dCurrentMarkDistance(0.0),
			m_nCurrentNumJumpSegments(0),
			m_nCurrentNumMarkSegments(0),
			m_bIsFirstMoveInBlock(true),
			m_bIsFirstMoveInLayer(true)


		{

		}

		virtual ~CMatJobLayer()
		{
		}

		double getZValue()
		{
			return m_dZValue;
		}

		double getLayerScanTime()
		{
			return m_dLayerScanTime;
		}

		double getTotalMarkDistance()
		{
			return m_dTotalMarkDistance;
		}

		double getTotalJumpDistance()
		{
			return m_dTotalJumpDistance;
		}

		double getMinX()
		{
			return m_dMinX;
		}

		double getMinY()
		{
			return m_dMinY;
		}

		double getMaxX()
		{
			return m_dMaxX;
		}

		double getMaxY()
		{
			return m_dMaxY;
		}

		void addDataBlock(const sMatJobDataBlock& dataBlock)
		{
			m_DataBlocks.push_back(dataBlock);
		}

		void addPolylineDataBlock(CMatJobPart* pPart, CMatJobBinaryFile* pBinaryFile, uint32_t nPartID, uint32_t nParameterSetID, const std::vector<Lib3MF::sPosition2D>& points, double dMarkSpeedInMMPerS, double dJumpSpeedInMMPerS)
		{
			
			if (pBinaryFile == nullptr)
				throw std::runtime_error("MatJob Polyline DataBlock has invalid binary file");
			if (pPart == nullptr)
				throw std::runtime_error("MatJob Polyline DataBlock has invalid part");
			if (points.size() == 0)
				throw std::runtime_error("MatJob Polyline DataBlock has no points");

			sMatJobDataBlock dataBlock;
			memset(&dataBlock, 0, sizeof(sMatJobDataBlock));
			dataBlock.m_nPartID = nPartID;
			dataBlock.m_nFileID = pBinaryFile->getFileID();
			dataBlock.m_nParameterSetID = nParameterSetID;
			dataBlock.m_nVectorTypeID = VECTORTYPEID_BORDER;
			dataBlock.m_nDataPosition = pBinaryFile->getCurrentFileSize();

			pBinaryFile->beginGroup(MATJOB_GROUP_DATABLOCK);
			pBinaryFile->writeUint8(MATJOB_GROUP_DATABLOCKTYPE, MATJOB_DATABLOCKTYPE_POLYLINELIST);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2121, 0);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2122, -1);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2123, 0);
			pBinaryFile->writePointArray(MATJOB_GROUP_DATABLOCKPOINTS, points);
			pBinaryFile->endGroup();

			m_bIsFirstMoveInBlock = true;
			m_dCurrentJumpDistance = 0.0;
			m_dCurrentMarkDistance = 0.0;
			m_nCurrentNumJumpSegments = 0;
			m_nCurrentNumMarkSegments = 0;

			auto& startPoint = points.at(0);
			double dStartX = startPoint.m_Coordinates[0];
			double dStartY = startPoint.m_Coordinates[1];
			pPart->addCoordinatesXY(dStartX, dStartY);

			moveTo(dStartX, dStartY, dJumpSpeedInMMPerS, false);
			for (auto i = 1; i < points.size(); i++) {

				auto& movePoint = points.at(i);
				double dMoveX = movePoint.m_Coordinates[0];
				double dMoveY = movePoint.m_Coordinates[1];

				pPart->addCoordinatesXY(dMoveX, dMoveY);
				moveTo(dMoveX, dMoveY, dMarkSpeedInMMPerS, true);
			}

			// Store Stastitics...
			dataBlock.m_nNumJumpSegments = m_nCurrentNumJumpSegments;
			dataBlock.m_nNumMarkSegments = m_nCurrentNumMarkSegments;
			dataBlock.m_dMarkDistance = m_dCurrentMarkDistance;
			dataBlock.m_dJumpDistance = m_dCurrentJumpDistance;

			m_DataBlocks.push_back(dataBlock);

		}

		void addHatchDataBlock(CMatJobPart* pPart, CMatJobBinaryFile* pBinaryFile, uint32_t nPartID, uint32_t nParameterSetID, const std::vector<Lib3MF::sHatch2D>& hatches, double dMarkSpeedInMMPerS, double dJumpSpeedInMMPerS)
		{
			if (pBinaryFile == nullptr)
				throw std::runtime_error("MatJob Polyline DataBlock has invalid binary file");

			sMatJobDataBlock dataBlock;
			memset(&dataBlock, 0, sizeof(sMatJobDataBlock));
			dataBlock.m_nPartID = nPartID;
			dataBlock.m_nFileID = pBinaryFile->getFileID();
			dataBlock.m_nParameterSetID = nParameterSetID;
			dataBlock.m_nVectorTypeID = VECTORTYPEID_HATCH;
			dataBlock.m_nDataPosition = pBinaryFile->getCurrentFileSize();

			pBinaryFile->beginGroup(MATJOB_GROUP_DATABLOCK);
			pBinaryFile->writeUint8(MATJOB_GROUP_DATABLOCKTYPE, MATJOB_DATABLOCKTYPE_HATCHBLOCK);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2121, 0);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2122, -1);
			pBinaryFile->writeInt32(MATJOB_GROUP_DATABLOCKUNKNOWN2123, 0);
			pBinaryFile->writeHatchArray(MATJOB_GROUP_DATABLOCKPOINTS, hatches);
			pBinaryFile->endGroup();

			m_bIsFirstMoveInBlock = true;
			m_dCurrentJumpDistance = 0.0;
			m_dCurrentMarkDistance = 0.0;
			m_nCurrentNumJumpSegments = 0;
			m_nCurrentNumMarkSegments = 0;

			for (auto& hatch : hatches) {


				pPart->addCoordinatesXY(hatch.m_Point1Coordinates[0], hatch.m_Point1Coordinates[1]);
				pPart->addCoordinatesXY(hatch.m_Point2Coordinates[0], hatch.m_Point2Coordinates[1]);

				moveTo(hatch.m_Point1Coordinates[0], hatch.m_Point1Coordinates[1], dJumpSpeedInMMPerS, false);
				moveTo(hatch.m_Point2Coordinates[0], hatch.m_Point2Coordinates[1], dMarkSpeedInMMPerS, true);
			}

			// Store Stastitics...
			dataBlock.m_nNumJumpSegments = m_nCurrentNumJumpSegments;
			dataBlock.m_nNumMarkSegments = m_nCurrentNumMarkSegments;
			dataBlock.m_dMarkDistance = m_dCurrentMarkDistance;
			dataBlock.m_dJumpDistance = m_dCurrentJumpDistance;

			m_DataBlocks.push_back(dataBlock);
		}

		void writeToXML(NMR::PXmlWriter_Native xmlWriter)
		{

			std::string sZValue = formatDouble4Layer(m_dZValue);
			std::string sLayerScanTimeString = formatDouble4Layer(m_dLayerScanTime);

			std::string sTotalMarkDistanceString = formatDouble4Layer(m_dTotalMarkDistance);
			std::string sTotalJumpDistanceString = formatDouble4Layer(m_dTotalJumpDistance);
			std::string sMinXString = formatDouble4Layer(m_dMinX);
			std::string sMinYString = formatDouble4Layer(m_dMinY);
			std::string sMaxXString = formatDouble4Layer(m_dMaxX);
			std::string sMaxYString = formatDouble4Layer(m_dMaxY);

			xmlWriter->WriteStartElement(nullptr, "Layer", "");

			xmlWriter->WriteStartElement(nullptr, "Z", "");
			xmlWriter->WriteText(sZValue.c_str(), (uint32_t)sZValue.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "LayerScanTime", "");
			xmlWriter->WriteText(sLayerScanTimeString.c_str(), (uint32_t)sLayerScanTimeString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "Summary", "");

			xmlWriter->WriteStartElement(nullptr, "TotalMarkDistance", "");
			xmlWriter->WriteText(sTotalMarkDistanceString.c_str(), (uint32_t)sTotalMarkDistanceString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "TotalJumpDistance", "");
			xmlWriter->WriteText(sTotalJumpDistanceString.c_str(), (uint32_t)sTotalJumpDistanceString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "XMin", "");
			xmlWriter->WriteText(sMinXString.c_str(), (uint32_t)sMinXString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "YMin", "");
			xmlWriter->WriteText(sMinYString.c_str(), (uint32_t)sMinYString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "XMax", "");
			xmlWriter->WriteText(sMaxXString.c_str(), (uint32_t)sMaxXString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteStartElement(nullptr, "YMax", "");
			xmlWriter->WriteText(sMaxYString.c_str(), (uint32_t)sMaxYString.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteEndElement();

			writeDataBlocksToXML(xmlWriter);

			xmlWriter->WriteEndElement();
		}

		void writeDataBlocksToXML(NMR::PXmlWriter_Native xmlWriter)
		{
			for (auto& dataBlock : m_DataBlocks) {

				xmlWriter->WriteStartElement(nullptr, "DataBlock", "");

				xmlWriter->WriteStartElement(nullptr, "References", "");
				xmlWriter->WriteAttributeString(nullptr, "Part", nullptr, std::to_string(dataBlock.m_nPartID).c_str());
				xmlWriter->WriteAttributeString(nullptr, "Process", nullptr, std::to_string(dataBlock.m_nParameterSetID).c_str());
				xmlWriter->WriteAttributeString(nullptr, "VectorTypeRef", nullptr, std::to_string(dataBlock.m_nVectorTypeID).c_str());
				xmlWriter->WriteEndElement();

				xmlWriter->WriteStartElement(nullptr, "Summary", "");
				xmlWriter->WriteAttributeString(nullptr, "MarkDistance", nullptr, formatDouble4Layer(dataBlock.m_dMarkDistance).c_str());
				xmlWriter->WriteAttributeString(nullptr, "JumpDistance", nullptr, formatDouble4Layer(dataBlock.m_dJumpDistance).c_str());
				xmlWriter->WriteAttributeString(nullptr, "NumMarkSegments", nullptr, std::to_string(dataBlock.m_nNumMarkSegments).c_str());
				xmlWriter->WriteAttributeString(nullptr, "NumJumpSegments", nullptr, std::to_string(dataBlock.m_nNumJumpSegments).c_str());
				xmlWriter->WriteEndElement();

				xmlWriter->WriteStartElement(nullptr, "Bin", "");
				xmlWriter->WriteAttributeString(nullptr, "FileID", nullptr, std::to_string(dataBlock.m_nFileID).c_str());
				xmlWriter->WriteAttributeString(nullptr, "Pos", nullptr, std::to_string(dataBlock.m_nDataPosition).c_str());
				xmlWriter->WriteEndElement();
				xmlWriter->WriteEndElement();
			}
		}

	};

	typedef std::shared_ptr<CMatJobLayer> PMatJobLayer;

}

#endif // __TOOLPATH_MATJOBLAYER
