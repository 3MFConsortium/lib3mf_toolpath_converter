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

#ifndef __TOOLPATH_MATJOBPART
#define __TOOLPATH_MATJOBPART

#include <string>
#include <cstdint>
#include <memory>

namespace Toolpath
{


	class CMatJobPart {
	private:
		std::string m_sName;
		uint32_t m_nPartID;

		double m_dPartMinX;
		double m_dPartMinY;
		double m_dPartMinZ;
		double m_dPartMaxX;
		double m_dPartMaxY;
		double m_dPartMaxZ;

		std::string m_sBuildItemUUID;


	public:

		CMatJobPart(const std::string& sName, uint32_t nPartID, const std::string& sBuildItemUUID, double dPartMinX, double dPartMinY, double dPartMinZ, double dPartMaxX, double dPartMaxY, double dPartMaxZ)
			: m_sName(sName), m_nPartID(nPartID), m_dPartMinX(dPartMinX), m_dPartMinY(dPartMinY), m_dPartMinZ(dPartMinZ), m_dPartMaxX(dPartMaxX), m_dPartMaxY(dPartMaxY), m_dPartMaxZ(dPartMaxZ), m_sBuildItemUUID(sBuildItemUUID)
		{

		}

		virtual ~CMatJobPart()
		{
		}

		std::string getName()
		{
			return m_sName;
		}

		uint32_t getPartID()
		{
			return m_nPartID;
		}

		double getMinX()
		{
			return m_dPartMinX;
		}

		double getMinY()
		{
			return m_dPartMinY;
		}

		double getMinZ()
		{
			return m_dPartMinZ;
		}

		double getMaxX()
		{
			return m_dPartMaxX;
		}

		double getMaxY()
		{
			return m_dPartMaxY;
		}

		double getMaxZ()
		{
			return m_dPartMaxZ;
		}


	};

	typedef std::shared_ptr<CMatJobPart> PMatJobPart;

}


#endif // __TOOLPATH_MATJOBPART
