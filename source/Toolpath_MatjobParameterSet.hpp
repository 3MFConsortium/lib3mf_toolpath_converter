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

#ifndef __TOOLPATH_MATJOBPARAMETERSET
#define __TOOLPATH_MATJOBPARAMETERSET


#include "Toolpath_MatjobProperty.hpp"
#include "Common/Platform/NMR_XmlWriter_Native.h"

namespace Toolpath {

	class CMatJobParameterSet {
	private:
		std::string m_sUUID;
		uint32_t m_nID;
		uint32_t m_nScanFieldID;
		std::string m_sName;
		double m_dLaserSpeed;
		double m_dJumpSpeed;
		uint32_t m_nLaserSetID;
		double m_dLaserDiameter;
		double m_dLaserPower;

		std::map<std::string, PMatJobProperty> m_Properties;

	public:

		CMatJobParameterSet(std::string sUUID, uint32_t nID, uint32_t nScanFieldID, const std::string& sName, double dLaserSpeed, uint32_t nLaserSetID, double dLaserDiameter, double dLaserPower, double dJumpSpeed)
			: m_sUUID(sUUID), m_nID(nID), m_nScanFieldID(nScanFieldID), m_sName(sName), m_dLaserSpeed(dLaserSpeed), m_nLaserSetID(nLaserSetID), m_dLaserDiameter(dLaserDiameter), m_dLaserPower(dLaserPower), m_dJumpSpeed(dJumpSpeed)
		{
		}

		virtual ~CMatJobParameterSet()
		{
		}

		uint32_t getID()
		{
			return m_nID;
		}

		uint32_t getScanFieldID()
		{
			return m_nScanFieldID;
		}

		std::string getName()
		{
			return m_sName;
		}

		double getLaserSpeed()
		{
			return m_dLaserSpeed;
		}

		double getJumpSpeed()
		{
			return m_dJumpSpeed;
		}

		uint32_t getLaserSetID()
		{
			return m_nLaserSetID;
		}

		double getLaserDiameter()
		{
			return m_dLaserDiameter;
		}

		double getLaserPower()
		{
			return m_dLaserPower;
		}

		void addProperty(const std::string& sName, const std::string& sValue, eMatJobPropertyType propertyType)
		{
			if (sName.empty())
				throw std::runtime_error("MatJob ParameterSet Property name cannot be empty");

			auto iIter = m_Properties.find(sName);
			if (iIter != m_Properties.end())
				throw std::runtime_error("MatJob ParameterSet Property with name '" + sName + "' already exists");

			auto pProperty = std::make_shared<CMatJobProperty>(sName, sValue, propertyType);
			m_Properties.insert(std::make_pair(sName, pProperty));
		}

		void writePropertiesToXML(NMR::PXmlWriter_Native xmlWriter)
		{
			for (auto propertyIter : m_Properties)
				propertyIter.second->writeToXML(xmlWriter);

		}
	};

	typedef std::shared_ptr<CMatJobParameterSet> PMatJobParameterSet;
}

#endif // __TOOLPATH_MATJOBPARAMETERSET
