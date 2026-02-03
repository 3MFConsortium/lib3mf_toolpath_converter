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

#ifndef __TOOLPATH_MATJOBPROPERTY
#define __TOOLPATH_MATJOBPROPERTY

#include "Common/Platform/NMR_XmlWriter_Native.h"

namespace Toolpath {

	enum class eMatJobPropertyType : uint32_t {
		mjpUnknown = 0,
		mjpJson,
		mjpString,
		mjpInteger,
		mjpFloat,
		mjpDouble,
		mjpBool
	};

	class CMatJobProperty {
	private:
		std::string m_sName;
		std::string m_sValue;
		eMatJobPropertyType m_Type;

	public:

		CMatJobProperty(const std::string& sName, const std::string& sValue, eMatJobPropertyType propertyType)
			: m_sName(sName), m_sValue(sValue), m_Type(propertyType)
		{

		}

		virtual ~CMatJobProperty()
		{
		}

		std::string getName()
		{
			return m_sName;
		}

		std::string getValue()
		{
			return m_sValue;
		}

		eMatJobPropertyType getType()
		{
			return m_Type;
		}

		std::string getTypeString()
		{
			switch (m_Type) {
			case eMatJobPropertyType::mjpJson: return "Json";
			case eMatJobPropertyType::mjpString: return "String";
			case eMatJobPropertyType::mjpInteger: return "Integer";
			case eMatJobPropertyType::mjpFloat: return "Float";
			case eMatJobPropertyType::mjpDouble: return "Double";
			case eMatJobPropertyType::mjpBool: return "Boolean";

			default:
				throw std::runtime_error("Unknown MatJob Property Type");
			}
		}

		void writeToXML(NMR::PXmlWriter_Native xmlWriter)
		{
			if (m_sName.empty())
				throw std::runtime_error("MatJob Property Name is empty");

			std::string sTypeString = getTypeString();

			xmlWriter->WriteStartElement(nullptr, "Property", "");
			xmlWriter->WriteAttributeString(nullptr, "Name", nullptr, m_sName.c_str());
			xmlWriter->WriteAttributeString(nullptr, "Type", nullptr, sTypeString.c_str());

			xmlWriter->WriteStartElement(nullptr, "Value", "");
			if (!m_sValue.empty())
				xmlWriter->WriteText(m_sValue.c_str(), (uint32_t)m_sValue.length());
			xmlWriter->WriteEndElement();

			xmlWriter->WriteEndElement();
		}


	};

	typedef std::shared_ptr<CMatJobProperty> PMatJobProperty;

}

#endif // __TOOLPATH_MATJOBPROPERTY
