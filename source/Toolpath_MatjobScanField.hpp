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

#ifndef __TOOLPATH_MATJOBSCANFIELD
#define __TOOLPATH_MATJOBSCANFIELD


#include <string>
#include <memory>
#include <cstdint>

namespace Toolpath {

	class CMatJobScanField {
	private:
		std::string m_sReference;
		uint32_t m_nLaserID;
		uint32_t m_nScanFieldID;
		double dXMin;
		double dYMin;
		double dXMax;
		double dYMax;

	public:

		CMatJobScanField(const std::string& sReference, uint32_t nLaserID, uint32_t nScanFieldID, double dXMin, double dYMin, double dXMax, double dYMax)
			: m_sReference(sReference), m_nLaserID(nLaserID), m_nScanFieldID(nScanFieldID), dXMin(dXMin), dYMin(dYMin), dXMax(dXMax), dYMax(dYMax)
		{
		}

		virtual ~CMatJobScanField()
		{
		}

		std::string getReference()
		{
			return m_sReference;
		}

		uint32_t getLaserID()
		{
			return m_nLaserID;
		}

		uint32_t getScanFieldID()
		{
			return m_nScanFieldID;
		}

		double getXMin()
		{
			return dXMin;
		}

		double getYMin()
		{
			return dYMin;
		}

		double getXMax()
		{
			return dXMax;
		}

		double getYMax()
		{
			return dYMax;
		}

	};

	typedef std::shared_ptr<CMatJobScanField> PMatJobScanField;

}

#endif // __TOOLPATH_MATJOBSCANFIELD
