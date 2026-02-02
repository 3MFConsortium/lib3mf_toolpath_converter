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

#ifndef __TOOLPATH_MATJOBWRITER
#define __TOOLPATH_MATJOBWRITER

#include "Toolpath_MatjobBinaryFile.hpp"
#include "Toolpath_MatjobVectorType.hpp"
#include "Toolpath_MatjobProperty.hpp"
#include "Toolpath_MatjobPart.hpp"
#include "Toolpath_MatjobLayer.hpp"
#include "Toolpath_MatjobScanField.hpp"
#include "Toolpath_MatjobParameterSet.hpp"

#include "Common/Platform/NMR_PortableZIPWriter.h"

namespace Toolpath {

	class CMatJobWriter {
	private:
		NMR::PPortableZIPWriter m_pZIPWriter;
		std::vector<PMatJobBinaryFile> m_BinaryFiles;

		PMatJobBinaryFile m_pOpenBinaryFile;
		PMatJobLayer m_pOpenLayer;

		// Job Information
		std::string m_sJobUUID;
		std::string m_sJobName;
		std::string m_sBuildItemUUID;

		double m_dJobMinX;
		double m_dJobMinY;
		double m_dJobMinZ;
		double m_dJobMaxX;
		double m_dJobMaxY;
		double m_dJobMaxZ;
		std::string m_sJobMaterial;

		std::map<std::string, PMatJobProperty> m_Properties;
		std::map<uint32_t, PMatJobScanField> m_ScanFields;
		std::map<uint32_t, PMatJobPart> m_Parts;
		std::map<std::string, PMatJobPart> m_PartsByBuildItemUUID;

		std::map<uint32_t, PMatJobVectorType> m_VectorTypes;
		std::map<uint32_t, PMatJobParameterSet> m_ParameterSets;
		std::map<std::string, PMatJobParameterSet> m_ParameterSetsByUUID;

		std::vector<PMatJobLayer> m_Layers;

		// Meta Information
		std::string m_sMetaDataFileName;
		std::string m_sConverterVersion;

	public:

		CMatJobWriter(NMR::PExportStream pExportStream);

		virtual ~CMatJobWriter();

		void writeContent();

		PMatJobBinaryFile beginBinaryFile(const std::string& sFileName);

		void closeCurrentBinaryFile();

		void writeJobMetaData();

		void addProperty(const std::string& sName, const std::string& sValue, eMatJobPropertyType propertyType);

		void addScanField(const std::string& sReference, uint32_t nLaserID, uint32_t nScanFieldID, double dXMin, double dYMin, double dXMax, double dYMax);

		void addPart(const std::string& sName, const std::string& sBuildItemUUID, double dPartMinX, double dPartMinY, double dPartMinZ, double dPartMaxX, double dPartMaxY, double dPartMaxZ);

		CMatJobPart* findPartByBuildItemUUID(const std::string& sBuildItemUUID);

		void addVectorType(const std::string& sName, uint32_t nID, bool bIsHatching, bool bIsBorder);

		PMatJobParameterSet addParameterSet(const std::string& sUUID, const std::string& sName, uint32_t nScanFieldID, double dLaserSpeed, uint32_t nLaserSetID, double dLaserDiameter, double dLaserPower, double dJumpSpeed);

		CMatJobParameterSet* findParameterSetByUUID(const std::string& sUUID);

		PMatJobLayer beginNewLayer(double dZValue);

	};

}


#endif // __TOOLPATH_MATJOBWRITER
