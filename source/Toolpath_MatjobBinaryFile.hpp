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

#ifndef __TOOLPATH_MATJOBBINARYFILE
#define __TOOLPATH_MATJOBBINARYFILE

#include <string>
#include <cstdint>
#include <cstring>
#include <vector>
#include <stack>

#include "lib3mf_types.hpp"

#include "Toolpath_MatjobConst.hpp"

#include "Common/Platform/NMR_ExportStream.h"

namespace Toolpath {
	
	class CMatJobBinaryFile {
	private:
		uint32_t m_nFileID;
		std::string m_sFileName;

		uint64_t m_nFileSize;

		std::vector<uint8_t> m_Buffer;
		std::stack<uint64_t> m_GroupStartPositionStack;

	public:

		CMatJobBinaryFile (uint32_t nFileID, const std::string sFileName)
			: m_nFileID (nFileID), m_nFileSize (0)
		{
			m_sFileName = sFileName;

			writeHeader();
		}

		virtual ~CMatJobBinaryFile()
		{
		}

		void writeRaw (uint8_t * pBuffer, uint32_t nLength)
		{
			if (nLength > 0) {
				if (pBuffer == nullptr)
					throw std::runtime_error("CMatJobBinaryFile::writeRaw: Buffer is null");

				for (uint32_t nIndex = 0; nIndex < nLength; nIndex++) {
					m_Buffer.push_back(pBuffer[nIndex]);
				}

				m_nFileSize += nLength;
			}

		}

		std::string getFileName()
		{
			return m_sFileName;
		}

		uint32_t getFileID()
		{
			return m_nFileID;
		}

		uint64_t getCurrentFileSize()
		{
			return m_nFileSize;
		}

		void beginGroup(uint32_t nID)
		{
			m_GroupStartPositionStack.push(m_nFileSize);

			uint32_t nDefaultGroupSize = 0xffffffff;
			writeRaw((uint8_t*)&nID, 4);
			writeRaw((uint8_t*)&nDefaultGroupSize, 4);
		}

		void endGroup()
		{
			if (m_GroupStartPositionStack.empty())
				throw std::runtime_error("CMatJobBinaryFile::endGroup: No group to end");

			uint64_t nGroupStartIndex = m_GroupStartPositionStack.top();
			m_GroupStartPositionStack.pop();

			if (nGroupStartIndex + 8 > m_nFileSize)
				throw std::runtime_error("CMatJobBinaryFile::endGroup: Invalid group start index");

			uint64_t nGroupSize64 = (m_nFileSize - (nGroupStartIndex + 8));
			if (nGroupSize64 > 0xffffffff)
				throw std::runtime_error("CMatJobBinaryFile::endGroup: Group size too large");

			uint32_t nGroupSize = (uint32_t) nGroupSize64;
			memcpy(&m_Buffer.at (nGroupStartIndex + 4), &nGroupSize, 4);
		}

		void writeUint8(uint32_t nID, uint8_t nValue)
		{
			writeRaw((uint8_t*)&nID, 4);
			uint32_t nLength = 1;
			writeRaw((uint8_t*)&nLength, 4);
			writeRaw((uint8_t*)&nValue, 1);
		}

		void writeUint32(uint32_t nID, uint32_t nValue)
		{
			writeRaw((uint8_t*)&nID, 4);
			uint32_t nLength = 4;
			writeRaw((uint8_t*)&nLength, 4);
			writeRaw((uint8_t*)&nValue, 4);
		}

		void writeUint64(uint32_t nID, uint64_t nValue)
		{
			writeRaw((uint8_t*)&nID, 4);
			uint32_t nLength = 8;
			writeRaw((uint8_t*)&nLength, 4);
			writeRaw((uint8_t*)&nValue, 8);
		}

		void writeInt32(uint32_t nID, uint32_t nValue)
		{
			writeRaw((uint8_t*)&nID, 4);
			uint32_t nLength = 4;
			writeRaw((uint8_t*)&nLength, 4);
			writeRaw((uint8_t*)&nValue, 4);
		}

		void writePointArray(uint32_t nID, const std::vector<Lib3MF::sPosition2D> & points)
		{
			if (points.size() == 0)
				throw std::runtime_error("CMatJobBinaryFile::writePointArray: Point array is empty");

			uint32_t nNumberOfPoints = (uint32_t)points.size();
			if (nNumberOfPoints > MATJOB_MAXPOINTCOUNTPERPOLYLINE)
				throw std::runtime_error("CMatJobBinaryFile::writePointArray: Too many points in array (" + std::to_string(nNumberOfPoints) + ")");

			writeRaw((uint8_t*)&nID, 4);
			uint32_t nLength = (uint32_t)(8 * nNumberOfPoints + 4);
			writeRaw((uint8_t*)&nLength, 4);
			writeRaw((uint8_t*)&nNumberOfPoints, 4);
			writeRaw((uint8_t*)&points.at(0), (uint32_t) (8 * points.size ()));
		}

		void writeHatchArray(uint32_t nID, const std::vector<Lib3MF::sHatch2D>& hatches)
		{
			if (hatches.size() == 0)
				throw std::runtime_error("CMatJobBinaryFile::writeHatchArray: Hatch array is empty");

			uint32_t nHatchCount = (uint32_t)hatches.size();
			if (nHatchCount > MATJOB_MAXHATCHCOUNTPERBLOCK)
				throw std::runtime_error("CMatJobBinaryFile::writeHatchArray: Too many hatches in array (" + std::to_string (nHatchCount) + ")");

			std::vector<float> coordinates;
			coordinates.reserve(nHatchCount * 4);
			for (auto & hatch : hatches) {
				coordinates.push_back((float)hatch.m_Point1Coordinates[0]);
				coordinates.push_back((float)hatch.m_Point1Coordinates[1]);
				coordinates.push_back((float)hatch.m_Point2Coordinates[0]);
				coordinates.push_back((float)hatch.m_Point2Coordinates[1]);
			}

			writeRaw((uint8_t*)&nID, 4);
			uint32_t nByteLength = (uint32_t) (16 * nHatchCount);
			writeRaw((uint8_t*)&nByteLength, 4);

			writeRaw((uint8_t*)&coordinates.at(0), nByteLength);

		}

		void writeString(uint32_t nID, const std::string& sString) {

			if (sString.empty ())
				throw std::runtime_error("CMatJobBinaryFile::writeString: String is empty");

			if (sString.length() > 0xffffffff)	
				throw std::runtime_error("CMatJobBinaryFile::writeString: String length too large");

			uint32_t nStringLength = (uint32_t)sString.length();

			writeRaw((uint8_t*)&nID, 4);
			writeRaw((uint8_t*)&nStringLength, 4);
			writeRaw((uint8_t*)sString.c_str (), nStringLength);
		}

		void writeArray(uint32_t nID, const std::vector<uint8_t> & buffer) {

			if (buffer.empty())
				throw std::runtime_error("CMatJobBinaryFile::writeArray: Buffer is empty");

			if (buffer.size() > 0xffffffff)
				throw std::runtime_error("CMatJobBinaryFile::writeArray: Buffer size too large");

			uint32_t nArrayLength = (uint32_t)buffer.size();

			writeRaw((uint8_t*)&nID, 4);
			writeRaw((uint8_t*)&nArrayLength, 4);
			writeRaw((uint8_t*)buffer.data(), nArrayLength);
		}

		void writeFloat (uint32_t nID, float fValue)
		{
			uint32_t nFloatLength = 4;
			writeRaw((uint8_t*)&nID, 4);
			writeRaw((uint8_t*)&nFloatLength, 4);
			writeRaw((uint8_t*)&fValue, 4);
		}

		void beginLayer(const double dZHeight)
		{
			beginGroup(MATJOB_GROUP_BEGINLAYER);
			writeFloat (MATJOB_GROUP_ZHEIGHT, (float) dZHeight);
		}

		void finishLayer()
		{
			endGroup();
		}


		void writeHeader()
		{
			beginGroup(MATJOB_GROUP_HEADER);
			writeString(MATJOB_HEADER_SIGNATURE, "AMCPBinaryFile");

			std::vector<uint8_t>unknown11Array;
			unknown11Array.push_back(1);
			for (uint32_t j = 1; j < 8; j++)
				unknown11Array.push_back(0);

			writeArray(MATJOB_HEADER_UNKNOWN_11, unknown11Array);

			writeString(MATJOB_HEADER_FOLDER, "C:\\Temp");

			endGroup();

			beginGroup(MATJOB_GROUP_FILEDATA);
		}

		void storeToStream(NMR::PExportStream pStream)
		{
			if (pStream.get() == nullptr)
				throw std::runtime_error("MatJob Export Stream is null");

			if (m_Buffer.empty())
				throw std::runtime_error("MatJob Strean Buffer is empty");

			pStream->writeBuffer(m_Buffer.data(), m_Buffer.size());
		}


	};

	typedef std::shared_ptr<CMatJobBinaryFile> PMatJobBinaryFile;
	
	
};

#endif // __TOOLPATH_MATJOBBINARYFILE
