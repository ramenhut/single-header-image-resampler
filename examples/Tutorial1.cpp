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

#include <memory>
#include <string>
#include <vector>
#include "bitmap.h"
#include "../base_resample.h"

#define MB (1024 * 1024)

using namespace base;
using ::std::make_unique;
using ::std::string;
using ::std::vector;

void PrintUsage(const char* programName) {
  printf("Usage: %s [options]\n", programName);
  printf("  --src [source filename]  \tSource image to load.\n");
  printf("  --dst [dest filename]  \tDestination image to save.\n");
  printf("  --width [integer]  \t\tSets the width of the output image.\n");
  printf("  --height [integer]  \t\tSets the height of the output image.\n");
  printf("  --kernel [integer]  \t\tSpecifies the kernel type.\n");
  printf("      1: nearest\n");
  printf("      2: average\n");
  printf("      3: bilinear\n");
  printf("      4: bicubic\n");
  printf("      5: Mitchell-Netravali\n");
  printf("      6: Cardinal\n");
  printf("      7: B-Spline\n");
  printf("      8: Lanczos\n");
  printf("      9: Lanczos-2\n");
  printf("      10: Lanczos-3\n");
  printf("      11: Lanczos-4\n");
  printf("      12: Lanczos-5\n");
  printf("      13: Catrom\n");
  printf("      14: Gaussian\n");
}

int main(int argc, char** argv) {
  string errors;
  string src_filename;
  string dst_filename;
  uint32 src_width = 0;
  uint32 src_height = 0;
  vector<uint8> src_image;
  uint32 output_width = 0;
  uint32 output_height = 0;
  KernelType kernel = KernelTypeBilinear;

  if (argc <= 1) {
    PrintUsage(argv[0]);
    return 0;
  }

  for (int i = 1; i < argc; i++) {
    char* optBegin = argv[i];
    for (int j = 0; j < 2; j++) (optBegin[0] == '-') ? optBegin++ : optBegin;

    switch (optBegin[0]) {
      case 's':
        src_filename = argv[++i];
        break;
      case 'd':
        dst_filename = argv[++i];
        break;
      case 'w':
        output_width = atoi(argv[++i]);
        break;
      case 'h':
        output_height = atoi(argv[++i]);
        break;
      case 'k':
        kernel = (KernelType)atoi(argv[++i]);
        break;
    }
  }

  if (src_filename.empty() || dst_filename.empty()) {
    printf("You must specify a valid source and destination filename.\n");
    return 0;
  }

  if (!output_width || !output_height || output_width > MB ||
      output_height > MB) {
    printf("Output dimensions must be in the range of (0, 1,048,576].\n");
    return 0;
  }

  /* Load our bitmap file into memory. */
  if (!::base::LoadBitmapImage(src_filename, &src_image, &src_width,
                               &src_height, &errors)) {
    printf("Error! %s\n", errors.c_str());
    return 0;
  }

  printf("Loaded %s with dimensions %i, %i.\n", src_filename.c_str(), src_width,
         src_height);

  /* Attempt our resample operation. */
  vector<uint8> dst_image(3 * output_width * output_height);

  printf(
      "Resampling image using kernel %i and destination dimensions %i, %i.\n",
      kernel, output_width, output_height);

  if (!ResampleImage24(&src_image.at(0), src_width, src_height,
                       &dst_image.at(0), output_width, output_height, kernel)) {
    printf("Error resampling image!\n");
    return 0;
  }

  /* Save our resampled image out to a bitmap file. */
  if (!::base::SaveBitmapImage(dst_filename, &dst_image, output_width,
                               output_height, &errors)) {
    printf("Error! %s\n", errors.c_str());
    return 0;
  }

  printf("Saved %s to disk.\n", dst_filename.c_str());

  return 0;
}