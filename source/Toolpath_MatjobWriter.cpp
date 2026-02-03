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

#include <stdexcept>
#include <exception>
#include <sstream>
#include <iomanip>
#include "Toolpath_MatjobWriter.hpp"

namespace Toolpath {

	// Helper function to format double with 4 decimal places
	static std::string formatDouble4(double value) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(4) << value;
		return oss.str();
	}

	CMatJobWriter::CMatJobWriter(NMR::PExportStream pExportStream)
	{
		if (pExportStream.get() == nullptr)
			throw std::runtime_error("Invalid export stream parameter");

		// Meta information
		m_sMetaDataFileName = "JobMetaData.job";
		m_sConverterVersion = "0.1";
		m_sJobMaterial = "1.4404";

		// Dummy Job Information
		m_sJobUUID = "42bb5e58-5f23-4852-bb9c-0d9fa4c76fd5";
		m_sJobName = "testjob.job";


		addProperty("material", m_sJobMaterial, eMatJobPropertyType::mjpString);

		addVectorType("Hatching", VECTORTYPEID_HATCH, true, false);
		addVectorType("Border", VECTORTYPEID_BORDER, false, true);

		m_pZIPWriter = std::make_shared<NMR::CPortableZIPWriter>(pExportStream, true);
	}

	CMatJobWriter::~CMatJobWriter()
	{
		m_pZIPWriter = nullptr;
	}


	void CMatJobWriter::writeContent()
	{
		auto pEntry = m_pZIPWriter->createEntry("Content.xml", 0);

		auto contentWriter = std::make_shared<NMR::CXmlWriter_Native>(pEntry);
		contentWriter->WriteStartDocument();
		contentWriter->WriteStartElement(nullptr, "ContainerContent", nullptr);
		contentWriter->WriteAttributeString(nullptr, "xmlns", nullptr, "http://schemas.materialise.com/AM/MatJob/Content");

		contentWriter->WriteStartElement(nullptr, "Version", nullptr);
		contentWriter->WriteAttributeString(nullptr, "Major", nullptr, "2");
		contentWriter->WriteAttributeString(nullptr, "Minor", nullptr, "1");
		contentWriter->WriteAttributeString(nullptr, "Revision", nullptr, "0");
		contentWriter->WriteEndElement();

		contentWriter->WriteStartElement(nullptr, "EncryptionStrategies", nullptr);

		contentWriter->WriteStartElement(nullptr, "EncryptionStrategy", nullptr);
		contentWriter->WriteAttributeString(nullptr, "Id", nullptr, "none");
		contentWriter->WriteAttributeString(nullptr, "Method", nullptr, "none");
		contentWriter->WriteEndElement();

		contentWriter->WriteEndElement();

		contentWriter->WriteStartElement(nullptr, "ContainerFiles", nullptr);

		contentWriter->WriteStartElement(nullptr, "MetadataFile", nullptr);
		contentWriter->WriteAttributeString(nullptr, "FileName", nullptr, m_sMetaDataFileName.c_str());
		contentWriter->WriteAttributeString(nullptr, "EncryptionStrategyRef", nullptr, "none");
		contentWriter->WriteEndElement();

		for (auto binaryFile : m_BinaryFiles) {
			contentWriter->WriteStartElement(nullptr, "BinaryFile", nullptr);
			contentWriter->WriteAttributeString(nullptr, "FileName", nullptr, binaryFile->getFileName().c_str());
			contentWriter->WriteAttributeString(nullptr, "EncryptionStrategyRef", nullptr, "none");
			contentWriter->WriteAttributeString(nullptr, "IsOutsideContainer", nullptr, "false");

			contentWriter->WriteEndElement();
		}
		contentWriter->WriteEndElement();

		contentWriter->WriteFullEndElement();
		contentWriter->WriteEndDocument();

	}

	PMatJobBinaryFile CMatJobWriter::beginBinaryFile(const std::string& sFileName)
	{
		if (sFileName.empty())
			throw std::runtime_error("Invalid binary file filename: " + sFileName);

		closeCurrentBinaryFile();

		uint32_t nFileID = (uint32_t)m_BinaryFiles.size();

		m_pOpenBinaryFile = std::make_shared<CMatJobBinaryFile>(nFileID, sFileName);

		m_BinaryFiles.push_back(m_pOpenBinaryFile);

		return m_pOpenBinaryFile;

	}

	void CMatJobWriter::closeCurrentBinaryFile()
	{
		if (m_pOpenBinaryFile != nullptr) {
			auto pEntry = m_pZIPWriter->createEntry(m_pOpenBinaryFile->getFileName(), 0);
			m_pOpenBinaryFile->storeToStream(pEntry);

		}

		m_pOpenBinaryFile = nullptr;
	}


	void CMatJobWriter::writeJobMetaData()
	{
		closeCurrentBinaryFile();

		auto pEntry = m_pZIPWriter->createEntry(m_sMetaDataFileName, 0);

		auto metaDataWriter = std::make_shared<NMR::CXmlWriter_Native>(pEntry);
		metaDataWriter->WriteStartDocument();
		metaDataWriter->WriteStartElement(nullptr, "BuildJob", "");
		metaDataWriter->WriteAttributeString(nullptr, "xmlns", nullptr, "http://schemas.materialise.com/AM/MatJob/MetaData");

		metaDataWriter->WriteStartElement(nullptr, "JobID", "");
		if (m_sJobUUID.empty())
			throw std::runtime_error("Job UUID is empty!");

		metaDataWriter->WriteText(m_sJobUUID.c_str(), (uint32_t)m_sJobUUID.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "FileInfo", "");

		metaDataWriter->WriteStartElement(nullptr, "Version", "");
		metaDataWriter->WriteStartElement(nullptr, "Major", "");
		metaDataWriter->WriteText("2", 1);
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Minor", "");
		metaDataWriter->WriteText("1", 1);
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "WrittenBy", "");

		metaDataWriter->WriteStartElement(nullptr, "Application", "");
		metaDataWriter->WriteAttributeString(nullptr, "Name", nullptr, "3MFtoMatJob");
		if (m_sConverterVersion.empty())
			throw std::runtime_error("Converter Version is Empty!");
		metaDataWriter->WriteAttributeString(nullptr, "Version", nullptr, m_sConverterVersion.c_str());
		metaDataWriter->WriteFullEndElement();

		metaDataWriter->WriteEndElement();
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "GeneralInfo", "");

		metaDataWriter->WriteStartElement(nullptr, "JobName", "");
		if (m_sJobName.empty())
			throw std::runtime_error("Job Name is Empty!");
		metaDataWriter->WriteText(m_sJobName.c_str(), (uint32_t)m_sJobName.length());
		metaDataWriter->WriteEndElement();

		double dJobMinX, dJobMinY, dJobMinZ, dJobMaxX, dJobMaxY, dJobMaxZ;
		calculateGlobalBounds(dJobMinX, dJobMinY, dJobMinZ, dJobMaxX, dJobMaxY, dJobMaxZ);

		std::string sXMin = formatDouble4(dJobMinX);
		std::string sYMin = formatDouble4(dJobMinY);
		std::string sZMin = formatDouble4(dJobMinZ);
		std::string sXMax = formatDouble4(dJobMaxX);
		std::string sYMax = formatDouble4(dJobMaxY);
		std::string sZMax = formatDouble4(dJobMaxZ);

		metaDataWriter->WriteStartElement(nullptr, "JobDimensions", "");
		metaDataWriter->WriteStartElement(nullptr, "Xmin", "");
		metaDataWriter->WriteText(sXMin.c_str(), (uint32_t)sXMin.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Ymin", "");
		metaDataWriter->WriteText(sYMin.c_str(), (uint32_t)sYMin.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Zmin", "");
		metaDataWriter->WriteText(sZMin.c_str(), (uint32_t)sZMin.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Xmax", "");
		metaDataWriter->WriteText(sXMax.c_str(), (uint32_t)sXMax.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Ymax", "");
		metaDataWriter->WriteText(sYMax.c_str(), (uint32_t)sYMax.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Zmax", "");
		metaDataWriter->WriteText(sZMax.c_str(), (uint32_t)sZMax.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Material", "");
		if (!m_sJobMaterial.empty())
			metaDataWriter->WriteText(m_sJobMaterial.c_str(), (uint32_t)m_sJobMaterial.length());
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Properties", "");

		for (auto propertyIter : m_Properties) {
			propertyIter.second->writeToXML(metaDataWriter);
		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "MachineType", "");
		metaDataWriter->WriteStartElement(nullptr, "PhysicalProperties", "");
		metaDataWriter->WriteAttributeString(nullptr, "xmlns", nullptr, "http://schemas.materialise.com/AM/Common/Machine/MachineType");

		metaDataWriter->WriteStartElement(nullptr, "ScanFields", "");

		for (auto scanFieldIter : m_ScanFields) {
			std::string sReference = scanFieldIter.second->getReference();
			std::string sScanFieldID = std::to_string(scanFieldIter.second->getScanFieldID());
			std::string sMinX = formatDouble4(scanFieldIter.second->getXMin());
			std::string sMinY = formatDouble4(scanFieldIter.second->getYMin());
			std::string sMaxX = formatDouble4(scanFieldIter.second->getXMax());
			std::string sMaxY = formatDouble4(scanFieldIter.second->getYMax());


			metaDataWriter->WriteStartElement(nullptr, "ScanField", "");
			metaDataWriter->WriteAttributeString(nullptr, "ID", nullptr, sScanFieldID.c_str());
			metaDataWriter->WriteAttributeString(nullptr, "Reference", nullptr, sReference.c_str());

			metaDataWriter->WriteStartElement(nullptr, "Dimension", "");
			metaDataWriter->WriteAttributeString(nullptr, "XMin", nullptr, sMinX.c_str());
			metaDataWriter->WriteAttributeString(nullptr, "YMin", nullptr, sMinY.c_str());
			metaDataWriter->WriteAttributeString(nullptr, "XMax", nullptr, sMaxX.c_str());
			metaDataWriter->WriteAttributeString(nullptr, "YMax", nullptr, sMaxY.c_str());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();

		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Lasers", "");

		for (auto scanFieldIter : m_ScanFields) {
			std::string sReference = scanFieldIter.second->getReference();
			std::string sLaserID = std::to_string(scanFieldIter.second->getLaserID());
			metaDataWriter->WriteStartElement(nullptr, "Laser", "");
			metaDataWriter->WriteAttributeString(nullptr, "ID", nullptr, sLaserID.c_str());
			metaDataWriter->WriteAttributeString(nullptr, "Reference", nullptr, sReference.c_str());
			metaDataWriter->WriteEndElement();
		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Parts", "");

		for (auto partIter : m_Parts) {
			std::string sIDString = std::to_string(partIter.second->getPartID());
			std::string sName = partIter.second->getName();
			std::string sMinX = formatDouble4(partIter.second->getMinX());
			std::string sMinY = formatDouble4(partIter.second->getMinY());
			std::string sMinZ = formatDouble4(partIter.second->getMinZ());
			std::string sMaxX = formatDouble4(partIter.second->getMaxX());
			std::string sMaxY = formatDouble4(partIter.second->getMaxY());
			std::string sMaxZ = formatDouble4(partIter.second->getMaxZ());


			metaDataWriter->WriteStartElement(nullptr, "Part", "");
			metaDataWriter->WriteStartElement(nullptr, "ID", "");
			metaDataWriter->WriteText(sIDString.c_str(), (uint32_t)sIDString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Name", "");
			if (sName.empty())
				throw std::runtime_error("Part name is empty!");
			metaDataWriter->WriteText(sName.c_str(), (uint32_t)sName.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Dimensions", "");
			metaDataWriter->WriteStartElement(nullptr, "Xmin", "");
			metaDataWriter->WriteText(sMinX.c_str(), (uint32_t)sMinX.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Ymin", "");
			metaDataWriter->WriteText(sMinY.c_str(), (uint32_t)sMinY.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Zmin", "");
			metaDataWriter->WriteText(sMinZ.c_str(), (uint32_t)sMinZ.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Xmax", "");
			metaDataWriter->WriteText(sMaxX.c_str(), (uint32_t)sMaxX.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Ymax", "");
			metaDataWriter->WriteText(sMaxY.c_str(), (uint32_t)sMaxY.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Zmax", "");
			metaDataWriter->WriteText(sMaxZ.c_str(), (uint32_t)sMaxZ.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();


			metaDataWriter->WriteEndElement();

		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "VectorTypes", "");
		for (auto vectorTypeIter : m_VectorTypes) {

			std::string sIDString = std::to_string(vectorTypeIter.second->getID());
			std::string sNameString = vectorTypeIter.second->getName();

			metaDataWriter->WriteStartElement(nullptr, "VectorType", "");
			metaDataWriter->WriteAttributeString(nullptr, "Id", nullptr, sIDString.c_str());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Name", "");
			metaDataWriter->WriteText(sNameString.c_str(), (uint32_t)sNameString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Flags", "");
			if (vectorTypeIter.second->isHatching())
				metaDataWriter->WriteAttributeString(nullptr, "Hatching", nullptr, "1");

			if (vectorTypeIter.second->isBorder())
				metaDataWriter->WriteAttributeString(nullptr, "Border", nullptr, "1");

			metaDataWriter->WriteEndElement(); // Close Flags

			metaDataWriter->WriteEndElement(); // Close VectorType
		}
		metaDataWriter->WriteEndElement(); // Close VectorTypes

		metaDataWriter->WriteStartElement(nullptr, "ProcessParameterSets", "");
		for (auto parameterSetIter : m_ParameterSets) {

			std::string sIDString = std::to_string(parameterSetIter.second->getID());
			std::string sScanFieldIDString = std::to_string(parameterSetIter.second->getScanFieldID());
			std::string sName = parameterSetIter.second->getName();

			if (sName.empty())
				throw std::runtime_error("Parameter Set name is empty!");

			std::string sLaserSetIDString = std::to_string(parameterSetIter.second->getLaserSetID());
			std::string sLaserSpeedString = formatDouble4(parameterSetIter.second->getLaserSpeed());
			std::string sLaserDiameterString = formatDouble4(parameterSetIter.second->getLaserDiameter());
			std::string sLaserPowerString = formatDouble4(parameterSetIter.second->getLaserPower());

			metaDataWriter->WriteStartElement(nullptr, "ParameterSet", "");
			metaDataWriter->WriteAttributeString(nullptr, "ScanField", nullptr, sScanFieldIDString.c_str());

			metaDataWriter->WriteStartElement(nullptr, "ID", "");
			metaDataWriter->WriteText(sIDString.c_str(), (uint32_t)sIDString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Name", "");
			metaDataWriter->WriteText(sName.c_str(), (uint32_t)sName.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "LaserSpeed", "");
			metaDataWriter->WriteText(sLaserSpeedString.c_str(), (uint32_t)sLaserSpeedString.length());
			metaDataWriter->WriteEndElement();

			/*metaDataWriter->WriteStartElement(nullptr, "JumpSpeed", "");
			metaDataWriter->WriteText(sJumpSpeedString.c_str(), (uint32_t)sJumpSpeedString.length());
			metaDataWriter->WriteEndElement();*/

			metaDataWriter->WriteStartElement(nullptr, "LaserSet", "");
			metaDataWriter->WriteStartElement(nullptr, "ID", "");
			metaDataWriter->WriteText(sLaserSetIDString.c_str(), (uint32_t)sLaserSetIDString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "LaserDiameter", "");
			metaDataWriter->WriteText(sLaserDiameterString.c_str(), (uint32_t)sLaserDiameterString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "LaserPower", "");
			metaDataWriter->WriteText(sLaserPowerString.c_str(), (uint32_t)sLaserPowerString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Properties", "");
			parameterSetIter.second->writePropertiesToXML(metaDataWriter);
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();

		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "BinaryFiles", "");

		for (auto binaryFileIter : m_BinaryFiles) {
			std::string sIDString = std::to_string(binaryFileIter->getFileID());
			std::string sFileSizeString = std::to_string(binaryFileIter->getCurrentFileSize());
			std::string sFileName = binaryFileIter->getFileName();

			if (sFileName.empty())
				throw std::runtime_error("Binary File name is empty!");

			metaDataWriter->WriteStartElement(nullptr, "BinaryFile", "");

			metaDataWriter->WriteStartElement(nullptr, "ID", "");
			metaDataWriter->WriteText(sIDString.c_str(), (uint32_t)sIDString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "Name", "");
			metaDataWriter->WriteText(sFileName.c_str(), (uint32_t)sFileName.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteStartElement(nullptr, "CRC", "");

			metaDataWriter->WriteStartElement(nullptr, "FileSize", "");
			metaDataWriter->WriteText(sFileSizeString.c_str(), (uint32_t)sFileSizeString.length());
			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();

			metaDataWriter->WriteEndElement();
		}

		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteStartElement(nullptr, "Layers", "");
		for (auto iLayerIter : m_Layers) {
			iLayerIter->writeToXML(metaDataWriter);
		}
		metaDataWriter->WriteEndElement();

		metaDataWriter->WriteEndElement();
		metaDataWriter->WriteEndDocument();
	}

	void CMatJobWriter::addProperty(const std::string& sName, const std::string& sValue, eMatJobPropertyType propertyType)
	{
		if (sName.empty())
			throw std::runtime_error("invalid matjob property name");

		auto iIter = m_Properties.find(sName);
		if (iIter != m_Properties.end())
			throw std::runtime_error("duplicate matjob property name: " + sName);

		auto pProperty = std::make_shared<CMatJobProperty>(sName, sValue, propertyType);
		m_Properties.insert(std::make_pair(sName, pProperty));
	}


	void CMatJobWriter::addScanField(const std::string& sReference, uint32_t nLaserID, uint32_t nScanFieldID, double dXMin, double dYMin, double dXMax, double dYMax)
	{
		if (sReference.empty())
			throw std::runtime_error("invalid matjob scanfield reference");
		auto iIter = m_ScanFields.find(nScanFieldID);
		if (iIter != m_ScanFields.end())
			throw std::runtime_error("duplicate matjob scanfield id: " + std::to_string(nScanFieldID));

		auto pScanField = std::make_shared<CMatJobScanField>(sReference, nLaserID, nScanFieldID, dXMin, dYMin, dXMax, dYMax);
		m_ScanFields.insert(std::make_pair(nScanFieldID, pScanField));
	}


	void CMatJobWriter::addPart(const std::string& sName, const std::string& sBuildItemUUID)
	{
		if (sName.empty())
			throw std::runtime_error("invalid matjob part name");

		uint32_t nPartID = (uint32_t)m_Parts.size();

		auto iIter = m_Parts.find(nPartID);
		if (iIter != m_Parts.end())
			throw std::runtime_error("duplicate matjob part id: " + std::to_string(nPartID));

		auto iUUIDIter = m_PartsByBuildItemUUID.find(sBuildItemUUID);
		if (iUUIDIter != m_PartsByBuildItemUUID.end())
			throw std::runtime_error("duplicate matjob part builditem uuid: " + sBuildItemUUID);

		auto pPart = std::make_shared<CMatJobPart>(sName, nPartID, sBuildItemUUID);
		m_Parts.insert(std::make_pair(nPartID, pPart));
		m_PartsByBuildItemUUID.insert(std::make_pair(sBuildItemUUID, pPart));

	}

	CMatJobPart* CMatJobWriter::findPartByBuildItemUUID(const std::string& sBuildItemUUID)
	{
		auto iUUIDIter = m_PartsByBuildItemUUID.find(sBuildItemUUID);
		if (iUUIDIter != m_PartsByBuildItemUUID.end())
			return iUUIDIter->second.get();

		throw std::runtime_error("matjob part builditem uuid not found: " + sBuildItemUUID);
	}


	void CMatJobWriter::addVectorType(const std::string& sName, uint32_t nID, bool bIsHatching, bool bIsBorder)
	{
		if (sName.empty())
			throw std::runtime_error("invalid matjob vectortype name");

		auto iIter = m_VectorTypes.find(nID);
		if (iIter != m_VectorTypes.end())
			throw std::runtime_error("duplicate matjob vectortype id: " + std::to_string(nID));

		auto pVectorType = std::make_shared<CMatJobVectorType>(sName, nID, bIsHatching, bIsBorder);
		m_VectorTypes.insert(std::make_pair(nID, pVectorType));
	}

	PMatJobParameterSet CMatJobWriter::addParameterSet(const std::string& sUUID, const std::string& sName, uint32_t nScanFieldID, double dLaserSpeed, uint32_t nLaserSetID, double dLaserDiameter, double dLaserPower, double dJumpSpeed)
	{
		if (sName.empty())
			throw std::runtime_error("invalid matjob parameterset name");

		uint32_t nID = (uint32_t)m_ParameterSets.size();

		auto iIter = m_ParameterSets.find(nID);
		if (iIter != m_ParameterSets.end())
			throw std::runtime_error("duplicate matjob parameterset id: " + std::to_string(nID));

		auto iUUIDIter = m_ParameterSetsByUUID.find(sUUID);
		if (iUUIDIter != m_ParameterSetsByUUID.end())
			throw std::runtime_error("duplicate matjob parameterset uuid: " + sUUID);

		auto pParameterSet = std::make_shared<CMatJobParameterSet>(sUUID, nID, nScanFieldID, sName, dLaserSpeed, nLaserSetID, dLaserDiameter, dLaserPower, dJumpSpeed);
		m_ParameterSets.insert(std::make_pair(nID, pParameterSet));
		m_ParameterSetsByUUID.insert(std::make_pair(sUUID, pParameterSet));

		return pParameterSet;

	}

	CMatJobParameterSet* CMatJobWriter::findParameterSetByUUID(const std::string& sUUID)
	{
		auto iUUIDIter = m_ParameterSetsByUUID.find(sUUID);
		if (iUUIDIter != m_ParameterSetsByUUID.end())
			return iUUIDIter->second.get();

		throw std::runtime_error("matjob parameterset uuid not found: " + sUUID);
	}


	PMatJobLayer CMatJobWriter::beginNewLayer(double dZValue)
	{
		if (m_Layers.size() > 0) {
			auto lastLayer = m_Layers.back();
			if (dZValue <= lastLayer->getZValue())
				throw std::runtime_error("New layer Z value must be greater than previous layer Z value");
		}

		m_pOpenLayer = std::make_shared<CMatJobLayer>(dZValue);
		m_Layers.push_back(m_pOpenLayer);

		return m_pOpenLayer;
	}

	void CMatJobWriter::calculateGlobalBounds(double& dMinX, double& dMinY, double& dMinZ, double& dMaxX, double& dMaxY, double& dMaxZ)
	{
		if (m_Parts.size () == 0)
			throw std::runtime_error("No parts defined in matjob");

		bool bFirst = true;
		
		dMinX = 0.0;
		dMinY = 0.0;
		dMinZ = 0.0;
		dMaxX = 0.0;
		dMaxY = 0.0;
		dMaxZ = 0.0;

		for (auto iPartIter : m_Parts) 
		{

			if (!iPartIter.second->hasPartBounds ())
				throw std::runtime_error("Part has no bounds defined in matjob. Part may be empty!" + iPartIter.second->getName());

			double dPartMinX = iPartIter.second->getMinX();
			double dPartMinY = iPartIter.second->getMinY();
			double dPartMinZ = iPartIter.second->getMinZ();
			double dPartMaxX = iPartIter.second->getMaxX();
			double dPartMaxY = iPartIter.second->getMaxY();
			double dPartMaxZ = iPartIter.second->getMaxZ();

			if (bFirst) {
				dMinX = dPartMinX;
				dMinY = dPartMinY;
				dMinZ = dPartMinZ;
				dMaxX = dPartMaxX;
				dMaxY = dPartMaxY;
				dMaxZ = dPartMaxZ;
				bFirst = false;
			}
			else {
				if (dPartMinX < dMinX)
					dMinX = dPartMinX;
				if (dPartMinY < dMinY)
					dMinY = dPartMinY;
				if (dPartMinZ < dMinZ)
					dMinZ = dPartMinZ;
				if (dPartMaxX > dMaxX)
					dMaxX = dPartMaxX;
				if (dPartMaxY > dMaxY)
					dMaxY = dPartMaxY;
				if (dPartMaxZ > dMaxZ)
					dMaxZ = dPartMaxZ;

			}


		}


	}


}