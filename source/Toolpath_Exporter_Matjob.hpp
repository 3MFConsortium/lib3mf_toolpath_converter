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

#ifndef __TOOLPATH_EXPORTER_MATJOB
#define __TOOLPATH_EXPORTER_MATJOB

#include "Toolpath_Exporter.hpp"

#include <sstream>
#include <cmath>
#include <stdexcept>

#include "Toolpath_MatjobWriter.hpp"
#include "Toolpath_MatjobBinaryFile.hpp"
#include "Common/NMR_StringUtils.h"
#include "Common/Platform/NMR_ExportStream_Native.h"

namespace Toolpath {

	/**
	 * Toolpath exporter for MatJob format.
	 */
	class CToolpathExporter_Matjob : public IToolpathExporter {
	private:
		std::string m_sOutputFileName;
		std::unique_ptr<CMatJobWriter> m_pMatJobWriter;
		NMR::PExportStream m_pExportStream;

		// Cached toolpath info
		Lib3MF::PToolpath m_pToolpath;
		double m_dUnits;
		uint32_t m_nLayerCount;
		uint32_t m_nLayersPerBatch;
		PMatJobBinaryFile m_pCurrentFile;

		double m_dGlobalLaserDiameter;

	public:
		CToolpathExporter_Matjob();
		virtual ~CToolpathExporter_Matjob() = default;

		void initialize(const std::string& sOutputFileName) override;
		void beginExport(Lib3MF::PToolpath pToolpath, Lib3MF::PModel pModel) override;
		void processLayer(uint32_t nLayerIndex, Lib3MF::PToolpathLayerReader pLayerReader) override;
		void finalize() override;

		// MatJob-specific configuration
		void setLayersPerBatch(uint32_t nLayersPerBatch);
		void setGlobalLaserDiameter(double dDiameter);
	};

	typedef std::shared_ptr<CToolpathExporter_Matjob> PToolpathExporter_Matjob;

} // namespace Toolpath

#endif // __TOOLPATH_EXPORTER_MATJOB
