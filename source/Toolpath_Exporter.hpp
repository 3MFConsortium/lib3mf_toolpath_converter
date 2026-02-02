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

#ifndef __TOOLPATH_EXPORTER
#define __TOOLPATH_EXPORTER

#include <string>
#include <vector>
#include <memory>
#include "lib3mf_dynamic.hpp"

namespace Toolpath {

	/**
	 * Abstract interface for toolpath exporters.
	 * Implement this interface to export toolpath data to different file formats.
	 */
	class IToolpathExporter {
	public:
		virtual ~IToolpathExporter() = default;

		/**
		 * Initialize the exporter with the output file path.
		 * @param sOutputFileName Path to the output file
		 */
		virtual void initialize(const std::string& sOutputFileName) = 0;

		/**
		 * Begin exporting from a 3MF toolpath.
		 * This sets up internal state based on the toolpath metadata.
		 * @param pToolpath The lib3mf toolpath to export
		 * @param pModel The lib3mf model containing build items
		 */
		virtual void beginExport(Lib3MF::PToolpath pToolpath, Lib3MF::PModel pModel) = 0;

		/**
		 * Process a single layer from the toolpath.
		 * @param nLayerIndex Index of the layer to process
		 * @param pLayerReader Layer reader for the layer data
		 */
		virtual void processLayer(uint32_t nLayerIndex, Lib3MF::PToolpathLayerReader pLayerReader) = 0;

		/**
		 * Finalize and write the export file.
		 */
		virtual void finalize() = 0;
	};

	typedef std::shared_ptr<IToolpathExporter> PToolpathExporter;

} // namespace Toolpath

#endif // __TOOLPATH_EXPORTER
