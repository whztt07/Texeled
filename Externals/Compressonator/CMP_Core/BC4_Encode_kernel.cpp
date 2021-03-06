//=====================================================================
// Copyright (c) 2018    Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=====================================================================
#include "BC4_Encode_kernel.h"

//============================================== BC4 INTERFACES =======================================================

void CompressBlockBC4_Internal(CMP_Vec4uc srcBlockTemp[16],
                               CMP_GLOBAL CGU_UINT32 compressedBlock[2],
                               CMP_GLOBAL CMP_BC15Options *BC15options) {
  if (BC15options->m_fquality) {
    // Reserved!
  }
  CGU_UINT8 blkindex = 0;
  CGU_UINT8 srcindex = 0;
  CGU_UINT8 alphaBlock[16];
  for (CGU_INT32 j = 0; j < 4; j++) {
    for (CGU_INT32 i = 0; i < 4; i++) {
      alphaBlock[blkindex++] =
          (CGU_UINT8)srcBlockTemp[srcindex].x;  // Red channel
      srcindex++;
    }
  }
  CompressAlphaBlock(alphaBlock, (CMP_GLOBAL CGU_UINT32 *)compressedBlock);
}

void DecompressBC4_Internal(CMP_GLOBAL CGU_UINT8 rgbaBlock[64],
                            CGU_UINT32 compressedBlock[2],
                            CMP_BC15Options *BC15options) {
  if (BC15options) {}
  CGU_UINT8 alphaBlock[BLOCK_SIZE_4X4];
  DecompressAlphaBlock(alphaBlock, compressedBlock);

  CGU_UINT8 blkindex = 0;
  CGU_UINT8 srcindex = 0;
  for (CGU_INT32 j = 0; j < 4; j++) {
    for (CGU_INT32 i = 0; i < 4; i++) {
      rgbaBlock[blkindex++] = (CGU_UINT8)alphaBlock[srcindex];  // R
      rgbaBlock[blkindex++] = (CGU_UINT8)alphaBlock[srcindex];  // G
      rgbaBlock[blkindex++] = (CGU_UINT8)alphaBlock[srcindex];  // B
      rgbaBlock[blkindex++] = (CGU_UINT8)alphaBlock[srcindex];  // A
      srcindex++;
    }
  }
}

void CompressBlockBC4_SingleChannel(CGU_UINT8 srcBlockTemp[16],
                               CMP_GLOBAL CGU_UINT32 compressedBlock[2],
                               CMP_GLOBAL CMP_BC15Options *BC15options) {
  if (BC15options) {}
  CompressAlphaBlock(srcBlockTemp, (CMP_GLOBAL CGU_UINT32 *)compressedBlock);
}

void DecompressBlockBC4_SingleChannel(CGU_UINT8 srcBlockTemp[16],
                            CGU_UINT32 compressedBlock[2],
                            CMP_BC15Options *BC15options) {
  if (BC15options) {}
  DecompressAlphaBlock(srcBlockTemp, compressedBlock);
}

//============================================== USER INTERFACES ========================================================
#ifndef ASPM_GPU

int CMP_CDECL CreateOptionsBC4(void **options)
{
    CMP_BC15Options *BC15optionsDefault = new CMP_BC15Options;
    if (BC15optionsDefault) {
        SetDefaultBC15Options(BC15optionsDefault);
        (*options) = BC15optionsDefault;
    }
    else {
        (*options) = NULL;
        return CGU_CORE_ERR_NEWMEM;
    }
    return CGU_CORE_OK;
}

int CMP_CDECL DestroyOptionsBC4(void *options)
{
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BCOptions = reinterpret_cast <CMP_BC15Options *>(options);
    delete BCOptions;
    return CGU_CORE_OK;
}

int CMP_CDECL SetQualityBC4(void *options,
    CGU_FLOAT fquality)
{
    if (!options) return CGU_CORE_ERR_INVALIDPTR;
    CMP_BC15Options *BC15optionsDefault = reinterpret_cast <CMP_BC15Options *>(options);
    if (fquality < 0.0f) fquality = 0.0f;
    else
        if (fquality > 1.0f) fquality = 1.0f;
    BC15optionsDefault->m_fquality = fquality;
    return CGU_CORE_OK;
}

int CMP_CDECL CompressBlockBC4(unsigned char *srcBlock,
                               unsigned int srcStrideInBytes,
                               CMP_GLOBAL unsigned char cmpBlock[8],
                               void *options = NULL) {

    unsigned char inBlock[16];
    //----------------------------------
    // Fill the inBlock with source data
    //----------------------------------
    CGU_INT srcpos = 0;
    CGU_INT dstptr = 0;
    for (CGU_UINT8 row = 0; row < 4; row++)
    {
        srcpos = row * srcStrideInBytes;
        for (CGU_UINT8 col = 0; col < 4; col++)
        {
            inBlock[dstptr++] = CGU_UINT8(srcBlock[srcpos++]);
        }
    }

    CMP_BC15Options *BC15options = (CMP_BC15Options *)options;
    if (BC15options == NULL) {
      CMP_BC15Options BC15optionsDefault;
      BC15options = &BC15optionsDefault;
      SetDefaultBC15Options(BC15options);
    }

    CompressBlockBC4_SingleChannel(inBlock,(CMP_GLOBAL CGU_UINT32 *)cmpBlock, BC15options);
    return CGU_CORE_OK;
}

int CMP_CDECL DecompressBlockBC4(unsigned char cmpBlock[8],
                            CMP_GLOBAL unsigned char srcBlock[16],
                            void *options = NULL) {
    CMP_BC15Options *BC15options = (CMP_BC15Options *)options;
    if (BC15options == NULL)
    {
        CMP_BC15Options BC15optionsDefault;
        BC15options = &BC15optionsDefault;
        SetDefaultBC15Options(BC15options);
    }
    DecompressBlockBC4_SingleChannel(srcBlock, (CGU_UINT32 *)cmpBlock,BC15options);
    return CGU_CORE_OK;
}
#endif

//============================================== OpenCL USER INTERFACE ====================================================
#ifdef ASPM_GPU
CMP_STATIC CMP_KERNEL void CMP_GPUEncoder(
    CMP_GLOBAL const CMP_Vec4uc *ImageSource,
    CMP_GLOBAL CGU_UINT8 *ImageDestination, CMP_GLOBAL Source_Info *SourceInfo,
    CMP_GLOBAL CMP_BC15Options *BC15options) {
  CGU_UINT32 xID;
  CGU_UINT32 yID;

#ifdef ASPM_GPU
  xID = get_global_id(0);
  yID = get_global_id(1);
#else
  xID = 0;
  yID = 0;
#endif

  if (xID >= (SourceInfo->m_src_width / BlockX)) return;
  if (yID >= (SourceInfo->m_src_height / BlockX)) return;
  int srcWidth = SourceInfo->m_src_width;

  CGU_UINT32 destI =
      (xID * BC4CompBlockSize) + (yID * (srcWidth / BlockX) * BC4CompBlockSize);
  int srcindex = 4 * (yID * srcWidth + xID);
  int blkindex = 0;
  CMP_Vec4uc srcData[16];
  srcWidth = srcWidth - 4;

  for (CGU_INT32 j = 0; j < 4; j++) {
    for (CGU_INT32 i = 0; i < 4; i++) {
      srcData[blkindex++] = ImageSource[srcindex++];
    }
    srcindex += srcWidth;
  }

  CompressBlockBC4_Internal(srcData, (CMP_GLOBAL CGU_UINT32 *)&ImageDestination[destI], BC15options);
}
#endif
