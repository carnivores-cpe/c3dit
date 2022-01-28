#include "stdafx.h"

#ifdef _UNICODE
#define STBI_WINDOWS_UTF8
// NOTE: Use `stbi_convert_wchar_to_utf8()` to convert a wstring to UTF8 for `stbi_load*(...)`
#endif //_UNICODE

#define STBI_ONLY_PNG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STBI_ONLY_GIF
#define STBI_MAX_DIMENSIONS 4096 // The image dimensions don't need to be any bigger than this

// This source file is an implementation file for STB
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
