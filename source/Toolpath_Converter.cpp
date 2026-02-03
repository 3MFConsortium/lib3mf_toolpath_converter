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

#include <iostream>
#include <vector>
#include "lib3mf_dynamic.hpp"

#include "Toolpath_Exporter.hpp"
#include "Toolpath_Exporter_Matjob.hpp"
#include "Toolpath_Exporter_CLIPlus.hpp"

using namespace Toolpath;

int main(int argc, char* argv[])
{
    try {
		std::string sInputFileName;
		std::string sOutputFileName;
		std::string sOutputFormat = "matjob"; // Default format

		std::vector<std::string> commandArguments;
		for (int idx = 1; idx < argc; idx++)
			commandArguments.push_back(argv[idx]);

		for (size_t nIndex = 0; nIndex < commandArguments.size(); nIndex++) {
			std::string sArgument = commandArguments[nIndex];

			if (sArgument == "--input") {
				nIndex++;
				if (nIndex >= commandArguments.size())
					throw std::runtime_error("missing --input path");

				sInputFileName = commandArguments[nIndex];
			}

			if (sArgument == "--output") {
				nIndex++;
				if (nIndex >= commandArguments.size())
					throw std::runtime_error("missing --output path");

				sOutputFileName = commandArguments[nIndex];
			}

			if (sArgument == "--format") {
				nIndex++;
				if (nIndex >= commandArguments.size())
					throw std::runtime_error("missing --format value");

				sOutputFormat = commandArguments[nIndex];
			}
		}

		std::cout << "Input filename: " << sInputFileName << "\n";
		std::cout << "Output filename: " << sOutputFileName << "\n";
		std::cout << "Output format: " << sOutputFormat << "\n";

		if (sInputFileName.empty() || sOutputFileName.empty())
			throw std::runtime_error("Usage: converter.exe --input toolpath.3mf --output output_file [--format matjob|cliplus]");

		// Create the appropriate exporter based on format
		PToolpathExporter pExporter;
		if (sOutputFormat == "matjob") {
			pExporter = std::make_shared<CToolpathExporter_Matjob>();
		}
		else if (sOutputFormat == "cliplus" || sOutputFormat == "cli") {
			pExporter = std::make_shared<CToolpathExporter_CLIPlus>();
		}
		else {
			throw std::runtime_error("Unknown output format: " + sOutputFormat + ". Supported formats: matjob, cliplus, cli");
		}

		std::cout << "Reading 3MF file " << sInputFileName << "\n";

		auto pLib3MFWrapper = Lib3MF::CWrapper::loadLibrary("lib3mf_win64.dll");
		auto pModel = pLib3MFWrapper->CreateModel();

		auto pSource = pModel->CreatePersistentSourceFromFile(sInputFileName);
		auto pReader = pModel->QueryReader("3mf");
		pReader->ReadFromPersistentSource(pSource);

		std::cout << "3MF File opened..\n";

		auto pLib3MFToolpaths = pModel->GetToolpaths();
		if (!pLib3MFToolpaths->MoveNext()) {
			throw std::runtime_error("No toolpath data found in 3MF file.");
		}
		auto pLib3MFToolpath = pLib3MFToolpaths->GetCurrentToolpath();

		if (pLib3MFToolpaths->MoveNext()) {
			throw std::runtime_error("Multiple toolpath data sets found in 3MF file. Only one is supported.");
		}

		double dUnits = pLib3MFToolpath->GetUnits();
		uint32_t nLayerCount = pLib3MFToolpath->GetLayerCount();

		std::cout << "Layer Count: " << nLayerCount << ", Units: " << dUnits << "\n";

		std::cout << "Initializing" << std::endl;
		// Use the abstract exporter interface
		pExporter->initialize(sOutputFileName);

		std::cout << "Beginning export" << std::endl;
		pExporter->beginExport(pLib3MFToolpath, pModel);

		// Process all layers
		for (uint32_t nLayerIndex = 0; nLayerIndex < nLayerCount; nLayerIndex++) {
			std::cout << "Writing layer " << nLayerIndex << "..." << std::endl;

			auto pLayerReader = pLib3MFToolpath->ReadLayerData(nLayerIndex);
			pExporter->processLayer(nLayerIndex, pLayerReader);
		}

		std::cout << "finalizing..." << std::endl;
		pExporter->finalize();

		pExporter = nullptr;

		std::cout << "Done.\n";
    }
    catch (std::exception& E) {
        std::cout << "fatal error: " << E.what() << std::endl;
    }
    
}

