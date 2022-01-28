#include "stdafx.h"
#include "EditorMain.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "Targa.h"
#include <stb_image.h>


void CProject::Clear()
{
	ObjInfo.Clear();
	Model.Clear();
	Texture.Clear();
	Sprite.Clear();
	Animations.clear();
	SoundEffects.clear();

	this->sound_current = -1;
	this->animation_current = -1;
	this->animation_current_frame = 0;

	g_Camera.x = -24;
	g_Camera.y = -24;
	g_Camera.z = 24;
	g_Camera.TargetX = 0;
	g_Camera.TargetY = 0;
	g_Camera.TargetZ = 0;
	g_Camera.Dist = 24;
	g_Camera.Yaw = 0;
	g_Camera.Pitch = 0;
}

void CTexture::Clear()
{
	Data.reset(nullptr);
	Width = 0u;
	Height = 0u;
}

void CModel::Clear()
{
	Name = "";
	Vertices.clear();
	Faces.clear();
	Bones.clear();
}

void CObjectInfo::Clear()
{
	Radius = (0);
	YLo = (0);
	YHi = (0);
	Line.Length = 0;
	Line.Intensity = 0;
	Circle.Radius = 0;
	Circle.Intensity = 0;
	Flags = (0x00);
	GrRad = (0);
	DefLight = (0xFFFFFF);
	LastAniTime = (0);
	BoundR = (0.0f);
	for (int i = 0; i < 4; i++)
		Reserved[i] = 0;
}

RGBA& RGBA::operator=(const RGB& rhs)
{
	this->r = rhs.r;
	this->g = rhs.g;
	this->b = rhs.b;
	this->a = 255;
	return *this;
}

RGBA& RGBA::operator=(const RGBA16& rhs)
{
	this->r = rhs.r * 4;
	this->g = rhs.g * 4;
	this->b = rhs.b * 4;
	this->a = rhs.a * 255;
	return *this;
}

RGBA16& RGBA16::operator=(const RGB& c)
{
	// TODO: insert return statement here
	r = c.r / 8;
	g = c.g / 8;
	b = c.b / 8;
	a = 1;//( ( r + g + b ) != 0 );
	return *this;
}

RGBA16& RGBA16::operator=(const RGBA& c)
{
	// TODO: insert return statement here
	r = c.r / 8;
	g = c.g / 8;
	b = c.b / 8;
	a = c.a != 0;
	return *this;
}

// Utility macro to more easily check a 4-byte string ID against a DWORD/UINT32
#define BLOCKID_UINT(a) static_cast<uint32_t>(a[0] + a[1] + a[2] + a[3])
bool CSound::Load(const std::string& filepath)
{
	// TODO: Support Vorbis OGG in the future

	//std::string filepath = OpenFileDlg(g_hMain, "WAVE sound file (.wav)\0*.wav\0", "wav");
	if (filepath.empty())
		return false;

	std::ifstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		MessageBox(g_hMain, TEXT("Unable to open the file for reading."), TEXT("File Error"), MB_ICONWARNING);
		return false;
	}

	uint32_t block_ID = 0u;
	uint32_t block_size = 0u;

	fs.read((char*)&block_ID, 4);
	fs.read((char*)&block_size, 4);

	if (block_ID != BLOCKID_UINT("RIFF"))
	{
		MessageBox(g_hMain, TEXT("Expected a RIFF block but didn't read one."), TEXT("File Error"), MB_ICONWARNING);
		return false;
	}

	WAVEFORMATEX wf;
	uint32_t wavedata_length = 0u;
	std::unique_ptr<uint8_t[]> pcm_data(nullptr);
	std::memset(&wf, 0, sizeof(WAVEFORMATEX));

	while (!fs.eof() && !fs.fail())
	{
		fs.read((char*)&block_ID, 4);
		fs.read((char*)&block_size, 4);

		if (block_ID == BLOCKID_UINT("WAVE"))
		{
			fs.read((char*)&block_ID, 4);

			if (block_ID != BLOCKID_UINT("fmt "))
			{
				std::cerr << "Expected \'fmt \' block!" << std::endl;
				return false;
			}

			fs.read((char*)&block_size, 4);
			fs.read((char*)&wf, (block_size <= sizeof(WAVEFORMATEX)) ? block_size : sizeof(WAVEFORMATEX));

			if (block_size > sizeof(WAVEFORMATEX))
			{
				std::cerr << "Warning! the \'fmt \' block size is larger than the structure WAVEFORMATEX!" << std::endl;
				fs.seekg(block_size - sizeof(WAVEFORMATEX), std::ios::cur);
			}

			fs.read((char*)&block_ID, 4); // "fmt "

			if (block_ID != BLOCKID_UINT("data"))
			{
				std::cerr << "Expected \'data\' block!" << std::endl;
				return false;
			}

			fs.read((char*)&wavedata_length, 4);
			pcm_data.reset(new uint8_t[wavedata_length]);
			fs.read((char*)&pcm_data, wavedata_length);
		}
		else // Make a note of unhandled blocks in the error log
		{
			std::cerr << "Unhandled RIFF block: \'";
			for (int i = 0; i < 4; i++) { std::cerr.put(reinterpret_cast<char*>(&block_ID)[i]); }
			std::cerr << "\'" << std::endl;

			fs.seekg(block_size, std::ios::cur);
		}
	}
	fs.close();

	//--Header check
	if (wf.wFormatTag != WAVE_FORMAT_PCM) {
		MessageBox(g_hMain, TEXT("The sound file is not an uncompressed PCM waveform sound!"), TEXT("Warning"), MB_ICONWARNING);
		return false;
	}
	if (wf.nChannels != 1) {
		MessageBox(g_hMain, TEXT("The sound file is not a Mono sound!"), TEXT("Warning"), MB_ICONWARNING);
		return false;
	}
	if (wf.nSamplesPerSec != 22050) {
		MessageBox(g_hMain, TEXT("The sound file does not have a frequency of 22.05 kHz!"), TEXT("Warning"), MB_ICONWARNING);
		return false;
	}
	if (wf.nBlockAlign != 2) {
		MessageBox(g_hMain, TEXT("The sound file does not have a block size of 2 bytes!"), TEXT("Warning"), MB_ICONWARNING);
		return false;
	}
	if (wf.wBitsPerSample != 16) {
		MessageBox(g_hMain, TEXT("The sound file is not a 16-bit sound!"), TEXT("Warning"), MB_ICONWARNING);
		return false;
	}

	SampleCount = (wavedata_length / wf.nBlockAlign) / wf.nChannels;
	Length = wavedata_length;

	// Reset the waveform data pointer
	Data.reset(new int16_t[this->Length / 2]);

	// Convert the waveform (if applicable)
	//...

	//std::copy(pcm_data.get()[0], pcm_data.get()[wavedata_length / 2], Data.get());
	std::memcpy(Data.get(), pcm_data.get(), wavedata_length);

	return true;
}


bool CTexture::Load(const std::string& filepath)
{
	//std::string filepath = OpenFileDlg(g_hMain, "All Supported Files\0*.bmp;*.tga\0TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp\0", "tga");
	std::string fileext = "";
	bool bIgnoreTextureSize = false;

	if (filepath.empty()) {
		return false;
	} else {
		std::stringstream ss;
		ss << std::nouppercase << filepath.substr(filepath.find_last_of('.'), std::string::npos);
		fileext = ss.str();
	}

	int iTexWidth = 0;
	int iTexHeight = 0;
	int iTexChannels = 0;

	auto pmImageData = stbi_load(filepath.c_str(), &iTexWidth, &iTexHeight, &iTexChannels, 4);

	if (!bIgnoreTextureSize) {
		std::wstringstream ssWarnings;

		if (iTexWidth > 256) {
			ssWarnings << TEXT("\n");
		}

		if (iTexHeight > 256) {
			ssWarnings << TEXT("\n");
		}

		MessageBox(g_hMain, ssWarnings.str().c_str(), TEXT("Texture warning..."), MB_ICONWARNING);
	}

#ifdef _DEBUG
	std::cout << "Texture::Load(" << filepath << ") W=" << iTexWidth << " H=" << iTexHeight << " BPP=" << iTexChannels << std::endl;
#endif

	//--Error catch
	if (iTexChannels != 4 && iTexChannels != 3) {
		MessageBox(g_hMain, TEXT("The image file is not a 16-bit, 24-bit or 32-bit image!"), TEXT("File Error"), MB_ICONWARNING);
		return false;
	}

	g_Project.Texture.Width = iTexWidth;
	g_Project.Texture.Height = iTexHeight;

	Data = std::make_unique<RGBA[]>(iTexWidth * iTexHeight);
	std::fill(Data.get(), Data.get() + iTexWidth * iTexHeight, RGBA());
	//std::memset(Data.get(), 0, iTexWidth * iTexHeight * sizeof(RGBA));

	if (Data) {
		try {
			std::unique_ptr<uint8_t[]> tex_data(new uint8_t[tex_width * tex_height * (tex_bits >> 3)]);
			// Convert the image to 32-bit RGBA
			for (auto i = 0u; i < tex_width * tex_height; i++) {
				if (tex_bits == 16) { Data[i] = *(reinterpret_cast<RGBA16*>(tex_data.get()) + i); }
				else if (tex_bits == 24) { Data[i] = *(reinterpret_cast<RGB*>(tex_data.get()) + i); }
				else if (tex_bits == 32) { Data[i] = *(reinterpret_cast<RGBA*>(tex_data.get()) + i); }
			}
		}
		catch (std::bad_alloc& ba) { std::cerr << "CTexture::Load() : Failed to allocate array!\n" << ba.what() << std::endl; return false; };
	}

	/*
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);

	if (bpp == 16) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, w, h, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if (bpp == 24) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Model.mTexture);
	if (bpp == 32) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, Model.mTexture);
	*/

	return true;
}


bool CSound::Save(const std::string& filepath)
{
	// Move the following to places where `importSound()` are called
	//filepath = SaveFileDlg(g_hMain, "WAVE sound file (.wav)\0*.wav\0", "wav");

	if (filepath.empty())
	{
		return false;
	}

	std::ofstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, "Failed to open the WAV file for writing!", "File Error", MB_ICONWARNING);
		return false;
	}

	uint32_t block_size = 0;

	fs.write("RIFF", 4);
	fs.write((char*)&block_size, 4);

	fs.write("WAVE", 4); // Chunk
	fs.write("fmt ", 4);

	block_size = sizeof(WAVEFORMATEX);
	fs.write((char*)&block_size, 4);

	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1u;
	wf.nSamplesPerSec = 22050u;
	wf.nAvgBytesPerSec = wf.nChannels * sizeof(int16_t) * wf.nSamplesPerSec;
	wf.nBlockAlign = wf.nChannels * sizeof(int16_t);
	wf.wBitsPerSample = 16u;
	wf.cbSize = 0u;

	fs.write((char*)&wf, sizeof(WAVEFORMATEX));

	fs.write("data", 4);
	fs.write((char*)&this->Length, 4);
	fs.write((char*)&this->Data, this->Length);

	std::string date = "XXXX-XX-XX"; // TODO: Use <chrono> to get the date
	std::string software = "Carnivores Character 3D Editor";

	// NOTE: LIST chunk isn't necessary but it's a nice touch!
	block_size = 4 + 4 + (date.size() + 1) + 4 + (software.size() + 1);
	fs.write("LIST", 4);
	fs.write((char*)&block_size, 4);
	fs.write("INFO", 4); // Chunk
	fs.write("ICRD", 4);
	fs.write(date.c_str(), (std::streamsize) date.size() + 1);
	fs.write("ISFT", 4);
	fs.write(software.c_str(), (std::streamsize) software.size() + 1);

	// Go back and correct the RIFF block size.
	block_size = static_cast<uint32_t>(fs.tellp()) - 8; // subtract 8 to ignore the RIFF ID and size
	fs.seekp(4, std::ios::beg);
	fs.write((char*)&block_size, 4);
	fs.seekp(0, std::ios::end);

	return true;
}


bool CTexture::Save(const std::string& filepath)
{
	std::string fileext = "";

	if (filepath.empty())
	{
		return false;
	}
	else
	{
		std::stringstream ss;
		ss << std::uppercase << filepath.substr(filepath.find_last_of('.'), std::string::npos);
		fileext = ss.str();
	}

	std::ofstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		return false;
	}

	if (!fileext.compare(".TGA"))
	{
		TARGAINFOHEADER tih;

		std::memset(&tih, 0, sizeof(TARGAINFOHEADER));
		tih.tgaColorMapType = false;
		tih.tgaImageType = TGA_IMAGETYPE_RGB;
		tih.tgaWidth = g_Project.Texture.Width;
		tih.tgaHeight = g_Project.Texture.Height;
		tih.tgaBits = 32;

		fs.write((char*)&tih, sizeof(TARGAINFOHEADER));
	}
	else if (!fileext.compare(".CRT") || !fileext.compare(".CRTLM"))
	{
		uint8_t bits = 32;
		fs.write((char*)&g_Project.Texture.Width, 2);
		fs.write((char*)&g_Project.Texture.Height, 2);
		fs.write((char*)&bits, 1);
	}
	else if (!fileext.compare(".BMP"))
	{
		BITMAPFILEHEADER hdr;
		BITMAPINFOHEADER bmi;

		std::memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
		bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmi.biWidth = g_Project.Texture.Width;
		bmi.biHeight = g_Project.Texture.Height;
		bmi.biPlanes = 1;
		bmi.biBitCount = 32;
		bmi.biCompression = BI_RGB;
		bmi.biSizeImage = g_Project.Texture.Width * g_Project.Texture.Height * sizeof(RGBA);

		std::memset(&hdr, 0, sizeof(BITMAPFILEHEADER));
		hdr.bfType = ('B' << 0) + ('M' << 8); //0x4d42
		hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + bmi.biSize + bmi.biSizeImage);
		hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + bmi.biSize;

		fs.write((char*)&hdr, sizeof(BITMAPFILEHEADER));
		fs.write((char*)&bmi, sizeof(BITMAPINFOHEADER));
	}
	else if (!fileext.compare(".PNG"))
	{
		// TODO: Future version
		return false;
	}
	else { MessageBox(g_hMain, "Somehow you loaded a file that is the incorrect Type! Shame on you!\r\n;-P", "Invalid File Extension", MB_ICONWARNING); return false; }

	fs.write((char*)g_Project.Texture.Data.get(), (std::streamsize)g_Project.Texture.Width * (std::streamsize)g_Project.Texture.Height * sizeof(RGBA));

	return true;
}
