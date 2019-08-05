## Single header image resampling
Single header C++ code that resamples and resizes your pretty pictures. There's lots of frameworks out there that do something similar, but few that do **only** this *one*. *simple*. *thing*.

This is actually a simplified and minified version of the image resampling logic in the [Imagine Framework](http://bertolami.com/index.php?engine=portfolio&content=computer-graphics&detail=imagine-framework). It could use some sprucing up, but it gets the job done.

### Supported image data formats:
This resampler supports the most common raw image format around: **three channel with 8 bits per channel color** (e.g. RGB24). If you need support for other formats you could check out [imagine](http://bertolami.com/index.php?engine=portfolio&content=computer-graphics&detail=imagine-framework) which supports a broad variety of configurable formats, or you could modify this header to suit your needs. 

### Supported resampling methods:
Choose from any of the following sampling methods. For more information about how each of these works, check out [this blog post](http://bertolami.com/index.php?engine=blog&content=posts&detail=inside-imagine-kernels).
* Nearest
* Average
* Bilinear
* Bicubic
* Mitchell-Netravali
* Cardinal
* B-Spline
* Lanczos (1 through 5 steps)
* Catmull-Rom
* Gaussian

## Instructions
To get started include **base_resample.h** in your project and follow the super squeaky example below:

```C++
  /* Resampling a 512x512 source image into a 128x128 destination image. */
  unsigned char* src_image_data = /* up to you to allocate and fill this out. */
  unsigned char* dst_image_data = /* up to you to allocate, resampler will fill out. */

  /* Resize our image using bilinear resampling. */
  ResampleImage24(src_image_data, 512, 512, dst_image_data, 128, 128, KernelTypeBilinear);
```
For a more comprehensive example, check out examples/Tutorial1.cpp, which demonstrates loading a bitmap image, resampling it, and saving it back to disk.

## Details

This software is released under the terms of the BSD 2-Clause “Simplified” License.

For more information visit [http://www.bertolami.com](http://bertolami.com).