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

#ifndef __TOOLPATH_EXPORTER_CLIPLUS
#define __TOOLPATH_EXPORTER_CLIPLUS

#include "Toolpath_Exporter.hpp"
#include <fstream>
#include <sstream>
#include <map>
#include <stdexcept>

namespace Toolpath {

	/**
	 * Polyline direction for CLI format
	 */
	enum class eCLIPolylineDirection : int {
		Clockwise = 0,
		CounterClockwise = 1,
		Open = 2
	};

	/**
	 * Toolpath exporter for CLI+ (Common Layer Interface) format.
	 * 
	 * CLI+ is an extended version of the CLI format commonly used
	 * for laser-based additive manufacturing systems.
	 * 
	 * Output format: ASCII CLI version 2.0 with extensions for
	 * laser power, speed, and profile information.
	 */
	class CToolpathExporter_CLIPlus : public IToolpathExporter {
	private:
		std::string m_sOutputFileName;
		std::ofstream m_OutputStream;
		std::stringstream m_GeometryBuffer;

		// Cached toolpath info
		Lib3MF::PToolpath m_pToolpath;
		double m_dUnits;
		uint32_t m_nLayerCount;

		// Bounding box
		double m_dMinX, m_dMinY, m_dMinZ;
		double m_dMaxX, m_dMaxY, m_dMaxZ;

		// Part/Profile ID mapping
		std::map<std::string, uint32_t> m_PartIDMap;
		std::map<std::string, uint32_t> m_ProfileIDMap;
		uint32_t m_nNextPartID;
		uint32_t m_nNextProfileID;

		// Configuration
		bool m_bIncludeLaserParams;

		// Internal methods
		void writeHeader();
		void writeGeometryStart();
		void writeGeometryEnd();
		uint32_t getOrCreatePartID(const std::string& sBuildItemUUID);
		uint32_t getOrCreateProfileID(const std::string& sProfileUUID);

	public:
		CToolpathExporter_CLIPlus();
		virtual ~CToolpathExporter_CLIPlus() = default;

		void initialize(const std::string& sOutputFileName) override;
		void beginExport(Lib3MF::PToolpath pToolpath, Lib3MF::PModel pModel) override;
		void processLayer(uint32_t nLayerIndex, Lib3MF::PToolpathLayerReader pLayerReader) override;
		void finalize() override;

		// CLI+-specific configuration
		void setIncludeLaserParams(bool bInclude);
	};

	typedef std::shared_ptr<CToolpathExporter_CLIPlus> PToolpathExporter_CLIPlus;

} // namespace Toolpath

#endif // __TOOLPATH_EXPORTER_CLIPLUS
