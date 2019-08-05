/*
//
// Copyright (c) 1998-2019 Joe Bertolami. All Right Reserved.
//
//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//   AND ANY EXPRESS OR IMPLIED WARRANTIES, CLUDG, BUT NOT LIMITED TO, THE
//   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//   ARE DISCLAIMED.  NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//   LIABLE FOR ANY DIRECT, DIRECT, CIDENTAL, SPECIAL, EXEMPLARY, OR
//   CONSEQUENTIAL DAMAGES (CLUDG, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSESS TERRUPTION)
//   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER  CONTRACT, STRICT
//   LIABILITY, OR TORT (CLUDG NEGLIGENCE OR OTHERWISE) ARISG  ANY WAY  OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional Information:
//
//   For more information, visit http://www.bertolami.com.
//
*/

#ifndef __BITMAP_H__
#define __BITMAP_H__

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

using ::std::ifstream;
using ::std::ofstream;
using ::std::string;
using ::std::vector;

#ifndef __BASE_TYPES_H__
#define __BASE_TYPES_H__

#if defined(WIN32) || defined(_WIN64)
#define BASE_PLATFORM_WINDOWS
#include "windows.h"
#elif defined(__APPLE__)
#define BASE_PLATFORM_MACOS
#include "TargetConditionals.h"
#include "ctype.h"
#include "sys/types.h"
#include "unistd.h"
#else
#error "Unsupported target platform detected."
#endif

namespace base {

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef int64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef float float32;
typedef double float64;

}  // namespace base

#endif  // __BASE_TYPES_H__

namespace base {

#ifndef BI_RGB
#define BI_RGB (0)
#endif

#pragma pack(push)
#pragma pack(2)

typedef struct PTCX_BITMAP_FILE_HEADER {
  uint16 type;
  uint32 size;
  uint16 reserved[2];
  uint32 off_bits;

} PTCX_BITMAP_FILE_HEADER;

typedef struct PTCX_BITMAP_INFO_HEADER {
  uint32 size;
  int32 width;
  int32 height;
  uint16 planes;
  uint16 bit_count;
  uint32 compression;
  uint32 size_image;
  int32 x_pels_per_meter;
  int32 y_pels_per_meter;
  uint32 clr_used;
  uint32 clr_important;

} PTCX_BITMAP_INFO_HEADER;

#pragma pack(pop)

inline uint32 greater_multiple(uint32 value, uint32 multiple) {
  uint32 mod = value % multiple;

  if (0 != mod) {
    value += multiple - mod;
  }

  return value;
}

/* Loads a 24 bit RGB bitmap file into a vector. */
bool LoadBitmapImage(const string &filename, vector<uint8> *output,
                     uint32 *width, uint32 *height, string *error = nullptr) {
  if (filename.empty() || !output) {
    if (error) {
      *error = "Invalid inputs to LoadBitmapImage.";
    }
    return false;
  }

  uint32 bytes_read = 0;
  PTCX_BITMAP_INFO_HEADER bih;
  PTCX_BITMAP_FILE_HEADER bmf_header;

  ifstream input_file(filename, ::std::ios::in | ::std::ios::binary);

  if (!input_file.read((char *)&bmf_header, sizeof(PTCX_BITMAP_FILE_HEADER))) {
    if (error) {
      *error = "Failed to read bitmap file header";
    }
    return false;
  }

  if (!input_file.read((char *)&bih, sizeof(PTCX_BITMAP_INFO_HEADER))) {
    if (error) {
      *error = "Failed to read bitmap info header.\n";
    }
    return false;
  }

  if (bih.bit_count != 24) {
    if (error) {
      *error = "Unsupported bitmap data format.\n";
    }
    return false;
  }

  uint32 image_row_pitch = bih.width * 3;
  uint32 image_size = image_row_pitch * bih.height;

  output->resize(image_size);
  *width = bih.width;
  *height = bih.height;

  /* The BMP format requires each scanline to be 32 bit aligned, so we insert
     padding if necessary. */
  uint32 scanline_padding =
      greater_multiple(bih.width * 3, 4) - (bih.width * 3);

  uint32 row_stride = bih.width * 3;

  for (uint32 i = 0; i < bih.height; i++) {
    uint32 y_offset = i * row_stride;
    uint8 *dest_row = &output->at(y_offset);

    if (!input_file.read((char *)dest_row, row_stride)) {
      if (error) {
        *error = "Abrupt error reading file.\n";
      }
      return false;
    }

    uint32 dummy = 0; /* Padding will always be < 4 bytes. */
    if (!input_file.read((char *)&dummy, scanline_padding)) {
      if (error) {
        *error = "Abrupt error reading file.\n";
      }
      return false;
    }

    /* Swap the R and B channels (as BMP stores its data in BGR). */
    for (uint32 j = 0; j < bih.width; j++) {
      uint32 x_offset = y_offset + j * 3;
      uint8 temp_channel = output->at(x_offset + 0);
      output->at(x_offset + 0) = output->at(x_offset + 2);
      output->at(x_offset + 2) = temp_channel;
    }
  }

  return true;
}

bool SaveBitmapImage(const string &filename, vector<uint8> *input, uint32 width,
                     uint32 height, string *error = nullptr) {
  if (filename.empty() || input->empty()) {
    if (error) {
      *error = "Invalid inputs to SaveBitmapImage.";
    }
    return false;
  }

  uint32 bytes_written = 0;
  uint32 total_image_bytes = (3 * width) * height;
  uint32 header_size =
      sizeof(PTCX_BITMAP_FILE_HEADER) + sizeof(PTCX_BITMAP_INFO_HEADER);

  PTCX_BITMAP_FILE_HEADER bmf_header = {0x4D42,  // BM
                                        header_size + total_image_bytes, 0, 0,
                                        header_size};

  PTCX_BITMAP_INFO_HEADER bih = {sizeof(PTCX_BITMAP_INFO_HEADER),
                                 width,
                                 height,
                                 1,
                                 24,
                                 BI_RGB,
                                 total_image_bytes,
                                 0,
                                 0,
                                 0,
                                 0};

  ofstream output_file(filename, ::std::ios::out | ::std::ios::binary);

  if (!output_file.write((char *)&bmf_header,
                         sizeof(PTCX_BITMAP_FILE_HEADER))) {
    if (error) {
      *error = "Failed to write bitmap file header";
    }
    return false;
  }

  if (!output_file.write((char *)&bih, sizeof(PTCX_BITMAP_INFO_HEADER))) {
    if (error) {
      *error = "Failed to write bitmap info header.\n";
    }
    return false;
  }

  uint32 image_row_pitch = bih.width * 3;
  uint32 image_size = image_row_pitch * bih.height;

  /* The BMP format requires each scanline to be 32 bit aligned, so we insert
     padding if necessary. */
  uint32 scanline_padding =
      greater_multiple(bih.width * 3, 4) - (bih.width * 3);

  ::std::vector<uint8> one_texel_row;
  one_texel_row.resize(image_row_pitch);
  uint8 *row_ptr = &one_texel_row.at(0);
  uint32 row_stride = bih.width * 3;

  for (uint32 i = 0; i < bih.height; i++) {
    uint32 y_offset = i * row_stride;
    uint8 *src_row = &input->at(y_offset);

    /* Swap the R and B channels (as BMP stores its data in BGR). */
    for (uint32 j = 0; j < bih.width; j++) {
      uint32 x_offset = y_offset + j * 3;
      uint8 temp_channel = input->at(x_offset + 0);
      input->at(x_offset + 0) = input->at(x_offset + 2);
      input->at(x_offset + 2) = temp_channel;
    }

    if (!output_file.write((char *)src_row, row_stride)) {
      if (error) {
        *error = "Abrupt error writing file.\n";
      }
      return false;
    }

    uint32 dummy = 0; /* Padding will always be < 4 bytes. */
    if (!output_file.write((char *)&dummy, scanline_padding)) {
      if (error) {
        *error = "Abrupt error writing file.\n";
      }
      return false;
    }
  }

  return true;
}

}  // namespace base

#endif  // __BITMAP_H__