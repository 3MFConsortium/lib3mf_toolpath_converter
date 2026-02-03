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


		bool m_bHasPartBoundsXY;
		bool m_bHasPartBoundsZ;
		double m_dPartMinX;
		double m_dPartMinY;
		double m_dPartMinZ;
		double m_dPartMaxX;
		double m_dPartMaxY;
		double m_dPartMaxZ;

		std::string m_sBuildItemUUID;


	public:

		CMatJobPart(const std::string& sName, uint32_t nPartID, const std::string& sBuildItemUUID)
			: m_sName(sName), m_nPartID(nPartID), m_dPartMinX(0.0), m_dPartMinY(0.0), m_dPartMinZ(0.0), m_dPartMaxX(0.0), m_dPartMaxY(0.0), m_dPartMaxZ(0.0), m_sBuildItemUUID(sBuildItemUUID),
			m_bHasPartBoundsXY(false), m_bHasPartBoundsZ(false)
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

		bool hasPartBoundsXY()
		{
			return m_bHasPartBoundsXY;
		}

		bool hasPartBoundsZ()
		{
			return m_bHasPartBoundsZ;
		}

		bool hasPartBounds()
		{
			return m_bHasPartBoundsXY && m_bHasPartBoundsZ;
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


		void addCoordinatesXY(double dX, double dY) 
		{
			if (!m_bHasPartBoundsXY) {
				m_dPartMinX = dX;
				m_dPartMaxX = dX;
				m_dPartMinY = dY;
				m_dPartMaxY = dY;
				m_bHasPartBoundsXY = true;
			}
			else {
				if (dX < m_dPartMinX)
					m_dPartMinX = dX;
				if (dX > m_dPartMaxX)
					m_dPartMaxX = dX;
				if (dY < m_dPartMinY)
					m_dPartMinY = dY;
				if (dY > m_dPartMaxY)
					m_dPartMaxY = dY;
			}

		}

		void addCoordinatesZ(double dZ)
		{
			if (!m_bHasPartBoundsZ) {
				m_dPartMinZ = dZ;
				m_dPartMaxZ = dZ;
				m_bHasPartBoundsZ = true;
			}
			else {
				if (dZ < m_dPartMinZ)
					m_dPartMinZ = dZ;
				if (dZ > m_dPartMaxZ)
					m_dPartMaxZ = dZ;
			}
		}

	};

	typedef std::shared_ptr<CMatJobPart> PMatJobPart;

}


#endif // __TOOLPATH_MATJOBPART
