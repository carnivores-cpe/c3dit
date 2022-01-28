#include "stdafx.h"

#include "EditorMain.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <string>

#include <glm/glm.hpp>


#define tstring basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>
#define tstringstream basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>


const int iProjectVersion = 1;


/* Create bones based on approximation data from vertices! */
void CreateMissingBoneData()
{
	if (MessageBox(g_hMain, TEXT("The file is missing bone data but the vertices reference bone indices,\r\ndo you want to create approximated bone data?"), TEXT("Missing bone data"), MB_ICONINFORMATION | MB_YESNO) == IDNO) {
		return;
	}

	// -- Create missing bone data from
	for (auto vertex : g_Project.Model.Vertices)
	{
		if (vertex.Bone == -1) continue;

		if ((unsigned)vertex.Bone >= g_Project.Model.Bones.size()) {
			CBone bone(nullptr, 0.f, 0.f, 0.f, -1, false);
			g_Project.Model.Bones.push_back(bone);
		}
	}

	// Second Pass
	for (auto b = 0u; b < g_Project.Model.Bones.size(); b++)
	{
		std::stringstream ss("Bone ");
		ss << b;
		CBone& bone = g_Project.Model.Bones.at(b);

		bone.Name = ss.str();
		bone.Hide = false;

		std::vector<float> accum_x;
		std::vector<float> accum_y;
		std::vector<float> accum_z;

		for (auto vertex : g_Project.Model.Vertices)
		{
			if (vertex.Bone != b) continue;

			accum_x.push_back(vertex.x);
			accum_y.push_back(vertex.y);
			accum_z.push_back(vertex.z);
		}

		if (!accum_x.empty())
		{
			bone.Position.x = accum_x.at(0);
			bone.y = accum_y.at(0);
			bone.z = accum_z.at(0);
			for (auto v = accum_x.begin() + 1; v != accum_x.end(); v++) bone.x += *v;
			for (auto v = accum_y.begin() + 1; v != accum_y.end(); v++) bone.y += *v;
			for (auto v = accum_z.begin() + 1; v != accum_z.end(); v++) bone.z += *v;
			bone.x /= static_cast<float>(accum_x.size());
			bone.y /= static_cast<float>(accum_y.size());
			bone.z /= static_cast<float>(accum_z.size());
		}
	}
}


bool LoadCARFile(const std::string& filepath)
{
	bool vertices_have_bones = false;
	std::ifstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, TEXT("There was an error when opening the file."), TEXT("File Not Found"), MB_ICONERROR);
		return false;
	}

	struct _header {
		char name[32];
		uint32_t anim_count;
		uint32_t sound_count;
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t texture_bytes;
	} header;

	struct _face {
		uint32_t v[3];
		int32_t tx[3];
		int32_t ty[3];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} face;

	struct _vert {
		float x, y, z;
		int16_t bone;
		int16_t hide;
	} vert;

	struct _anim {
		char name[32];
		uint32_t kps;
		uint32_t frame_count;
	} anim;

	//////////////////////////////////////////////////////////////////////////////
	// Header
	fs.read((char*)&header, sizeof(header));
	g_Project.Model.Name = header.name;

	//////////////////////////////////////////////////////////////////////////////
	// Triangles
	for (auto f = header.face_count; f > 0; f--)
	{
		fs.read((char*)&face, sizeof(face));

		auto nface = g_Project.Model.Faces.insert(g_Project.Model.Faces.end(), CFace());
		nface->v1 = face.v[0]; nface->v2 = face.v[1]; nface->v3 = face.v[2];
		for (int i = 0; i < 3; i++) {
			nface->tcoord[i].x = static_cast<float>(face.tx[i]);
			nface->tcoord[i].y = static_cast<float>(face.ty[i]);
		}
		nface->Flags = face.flags;
		nface->DMask = face.dmask;
		nface->Prev = face.prev;
		nface->Next = face.next;
		nface->Group = face.group;
		for (int i = 0; i < 3; i++)
			nface->Reserved[i] = face.reserved[i];

		// TODO: Convert
	}

	g_Project.ObjInfo.YLo = (signed)0x7FFFFFFF;
	g_Project.ObjInfo.YHi = (signed)0x80000000;

	std::cout << "g_Project.ObjInfo.YLo = " << g_Project.ObjInfo.YLo << std::endl;
	std::cout << "g_Project.ObjInfo.YHi = " << g_Project.ObjInfo.YHi << std::endl;

	//////////////////////////////////////////////////////////////////////////////
	// Vertices
	for (auto v = header.vert_count; v > 0; v--)
	{
		fs.read((char*)&vert, sizeof(vert));

		CVertex vertex;
		vertex.x = vert.x;
		vertex.y = vert.y;
		vertex.z = vert.z;
		vertex.Bone = vert.bone;
		vertex.Hidden = vert.hide;

		g_Project.Model.Vertices.push_back(vertex);

		if (vert.y < g_Project.ObjInfo.YLo)
			g_Project.ObjInfo.YLo = static_cast<int32_t>(vert.y);

		if (vert.y > g_Project.ObjInfo.YHi)
			g_Project.ObjInfo.YHi = static_cast<int32_t>(vert.y);

		if (!vertices_have_bones && vertex.Bone != -1)
			vertices_have_bones = true;
	}

	// TODO: Normal calculation

	// -- Set the camera to look at the middle of the model
	g_Camera.TargetX = 0.f;
	g_Camera.TargetY = (g_Project.ObjInfo.YHi - g_Project.ObjInfo.YLo) / 2.f;
	g_Camera.TargetZ = 0.f;

	//////////////////////////////////////////////////////////////////////////////
	// Texture Block
	g_Project.Texture.Clear();
	g_Project.Texture.Width = 256;
	g_Project.Texture.Height = 256;

	int mbr = IDCONTINUE;

	if (header.texture_bytes != 256 * 256 * sizeof(uint16_t))
	{
		// Some sort of message/warning
		// maybe just make a 256x512 texture space, or 512x512 depending...
		std::wstring msg = L"The texture is not 256x256.\r\n" \
			"Cancel: Cancel loading the file." \
			"Try: Resume loading and attempt to resize the texture.\r\n" \
			"Continue: Resume loading without modifying the texture.\r\n";
		mbr = MessageBoxW(g_hMain, msg.c_str(), TEXT("Non-Square Texture"), MB_ICONWARNING | MB_CANCELTRYCONTINUE);

		if (mbr == IDCANCEL) {
			g_Project.Clear();
			return false;
		}
	}

	if (mbr == IDTRYAGAIN)
	{
		// Resize the texture to valid dimensions

		std::unique_ptr<RGBA16[]> tdata(new RGBA16[header.texture_bytes / sizeof(uint16_t)]);
		fs.read((char*)tdata.get(), header.texture_bytes);

		// TODO: Do some resizing here
	}
	else if (mbr == IDCONTINUE)
	{
		// Load the texture as-is
		g_Project.Texture.Data.reset(new RGBA[header.texture_bytes / 2]);
		std::unique_ptr<RGBA16[]> tdata(new RGBA16[header.texture_bytes / sizeof(uint16_t)]);

		fs.read((char*)tdata.get(), header.texture_bytes);

		// -- Convert the texture from 16-bit to 32-bit
		for (auto i = 0u; i < header.texture_bytes / sizeof(uint16_t); i++) {
			g_Project.Texture.Data[i] = tdata[i];
		}
	}

	// TODO: Call the Renderer DLL to create a texture and return a handle
	/*glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	if (Model.mTextureColor == 16)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if (Model.mTextureColor == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, Model.mTexture);
	if (Model.mTextureColor == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);
		*/

		//////////////////////////////////////////////////////////////////////////////
		// Animations
	for (auto a = header.anim_count; a > 0; a--)
	{
		fs.read((char*)&anim, sizeof(anim));

		CVertexAnimation vaVertAnim;
		vaVertAnim.Name = anim.name;
		vaVertAnim.KPS = anim.kps;
		vaVertAnim.FrameCount = anim.frame_count;

		for (auto f = vaVertAnim.FrameCount; f > 0; f--)
		{
			auto frame = vaVertAnim.Frames.insert(vaVertAnim.Frames.end(), CFrame());

			for (auto v = g_Project.Model.Vertices.size(); v > 0; v--)
			{
				glm::tvec3<int16_t> vPosition;
				
				fs.read((char*)&vPosition, sizeof(vPosition));

				frame->Positions.push_back(vPosition);
			}
		}

		// Deprecated:
		/*
		vanim.Data = new int16_t[g_Project.Model.Vertices.size() * vanim.FrameCount * 3];
		fs.read((char*)vanim.Data, g_Project.Model.Vertices.size() * vanim.FrameCount * 3);
		*/

		g_Project.Animations.push_back(vaVertAnim);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Animation Trackbar
	SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)true, (LPARAM)0);

	if (g_Project.Animations.size() != 0)
	{
		//g_CurrentAnimation = 0;
		SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, g_Project.Animations.at(0).FrameCount - 1));
	}
	else
	{
		SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, 1));
	}

	//////////////////////////////////////////////////////////////////////////////
	// Sound effects
	for (auto s = 0u; s < g_Project.SoundEffects.size(); s++)
	{
		auto sound = g_Project.SoundEffects.insert(g_Project.SoundEffects.end(), CSound());

		sound->Name.reserve(32);
		sound->Name.resize(32);

		fs.read(sound->Name.data(), 32);
		fs.read((char*)&sound->Length, 4);

		sound->Data.reset(new int16_t[sound->Length / sizeof(int16_t)]);

		fs.read((char*)sound->Data.get(), sound->Length);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Animation-Sound Table
	int32_t AnimFX[64];
	fs.read((char*)AnimFX, 64 * sizeof(int32_t));

	for (auto a = 0u; a < g_Project.Animations.size(); a++)
	{
		try { g_Project.Animations.at(a).SoundFX = AnimFX[a]; }
		catch (std::out_of_range& oor) { std::cerr << "C++ Exception!\n" << oor.what() << "\nUnable to access animation[" << a << "]" << std::endl; };
	}

	//////////////////////////////////////////////////////////////////////////////
	// If bones are missing create approximate data
	//TODO: have a setting for this
	if (vertices_have_bones && g_Project.Model.Bones.empty()) {
		CreateMissingBoneData();
	}

	return true;
}


/* !DEPRECATED!
*/
bool LoadC1OFile(const std::string& filepath)
{
	// TODO: Implement
	return false;
}

/* !DEPRECATED!
*/
bool LoadC2OFile(const std::string& filepath)
{
	struct _header {
		uint32_t bone_count;
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t texture_bytes;
	} header;

	struct _face {
		uint32_t v[3];
		int32_t tx[3];
		int32_t ty[3];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} face;

	struct _vert {
		float x, y, z;
		int16_t bone;
		int16_t hide;
	} vert;

	struct _bone {
		char name[32];
		float x, y, z;
		int16_t parent;
		int16_t hide;
	} bone;

	struct _anim {
		char name[32];
		uint32_t kps;
		uint32_t frame_count;
	} anim;

	std::ifstream fs(filepath, std::ios::binary);
	if (!fs.is_open())
	{
		MessageBox(g_hMain, TEXT("There was an error when opening the file."), TEXT("File error"), MB_ICONERROR);
		return false;
	}

	CModel& model = g_Project.Model;

	//////////////////////////////////////////////////////////////////////////////
	// Header
	model.Name, "<Object>";
	fs.read((char*)&g_Project.ObjInfo, sizeof(CObjectInfo)); // 48 + 16
	fs.read((char*)&header.vert_count, 4);
	fs.read((char*)&header.face_count, 4);
	fs.read((char*)&header.bone_count, 4);
	fs.read((char*)&header.texture_bytes, 4);

	//////////////////////////////////////////////////////////////////////////////
	// Faces
	for (auto f = 0u; f < header.face_count; f++)
	{
		auto nface = model.Faces.insert(model.Faces.end(), CFace());

		fs.read((char*)&face, sizeof(face));

		nface->v1 = face.v[0]; nface->v2 = face.v[1]; nface->v3 = face.v[2];
		for (int i = 0; i < 3; i++) {
			nface->tcoord[i].x = static_cast<float>(face.tx[i]);
			nface->tcoord[i].y = static_cast<float>(face.ty[i]);
		}
		nface->Flags = face.flags;
		nface->DMask = face.dmask;
		nface->Prev = face.prev;
		nface->Next = face.next;
		nface->Group = face.group;
		for (int i = 0; i < 3; i++)
			nface->Reserved[i] = face.reserved[i];

		// TODO: Convert
	}

	//////////////////////////////////////////////////////////////////////////////
	// Vertices
	for (auto v = header.vert_count; v > 0; v--)
	{
		fs.read((char*)&vert, sizeof(vert));

		auto vertex = model.Vertices.insert(model.Vertices.end(), CVertex());
		vertex->x = vert.x;
		vertex->y = vert.y;
		vertex->z = vert.z;
		vertex->Bone = vert.bone;
		vertex->Hidden = vert.hide;
	}

	// TODO: Normal calculation

	// -- Set the camera to look at the middle of the model
	g_Camera.TargetX = 0.f;
	g_Camera.TargetY = (g_Project.ObjInfo.YHi - g_Project.ObjInfo.YLo) / 2.f;
	g_Camera.TargetZ = 0.f;

	// Normals
	/*bool vlist[MAX_VERTICES];
	ZeroMemory(vlist, sizeof(bool));
	for (int f = 0; f < Model.mFaceCount; f++)
	{
		int v1 = g_Triangles[f].v1;
		int v2 = g_Triangles[f].v2;
		int v3 = g_Triangles[f].v3;
		g_Normals[f] = ComputeNormals(g_Verticies[v1], g_Verticies[v2], g_Verticies[v3]);
		if (!vlist[v1])
		{
			g_VNormals[v1] = g_Normals[f];
			vlist[v1] = true;
		}
		else
		{
			g_VNormals[v1] += g_Normals[f];
			g_VNormals[v1] /= 2;
		}
		if (!vlist[v2])
		{
			g_VNormals[v2] = g_Normals[f];
			vlist[v2] = true;
		}
		else
		{
			g_VNormals[v2] += g_Normals[f];
			g_VNormals[v2] /= 2;
		}
		if (!vlist[v3])
		{
			g_VNormals[v3] = g_Normals[f];
			vlist[v3] = true;
		}
		else
		{
			g_VNormals[v3] += g_Normals[f];
			g_VNormals[v3] /= 2;
		}
	}*/

	//////////////////////////////////////////////////////////////////////////////
	// Bones
	for (auto b = 0u; b < header.bone_count; b++)
	{
		fs.read((char*)&bone, sizeof(bone));

		CBone abone(bone.name, bone.x, bone.y, bone.z, bone.parent, bone.hide);

		model.Bones.push_back(abone);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Texture Block
	g_Project.Texture.Clear();
	g_Project.Texture.Width = 256;
	g_Project.Texture.Height = 256;

	if (header.texture_bytes != 256 * 256 * sizeof(uint16_t))
	{
		// Some sort of message/warning
		// maybe just make a 256x512 texture space, or 512x512 depending...
		
		std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>> msg = TEXT("The texture is not 256x256.\r\n" \
			"Cancel: Cancel loading the file." \
			"Try: Resume loading and attempt to resize the texture.\r\n" \
			"Continue: Resume loading without modifying the texture.\r\n");
		switch (MessageBox(g_hMain, msg.c_str(), TEXT("Non-Square Texture"), MB_ICONWARNING | MB_CANCELTRYCONTINUE)) {
		case IDCANCEL: g_Project.Model.Clear();  return false;
		case IDTRYAGAIN: break;
		case IDCONTINUE: break;
		}
	}

	g_Project.Texture.Data.reset(new RGBA[header.texture_bytes / 2]);
	RGBA16* tdata = new RGBA16[header.texture_bytes / sizeof(uint16_t)];

	fs.read((char*)tdata, header.texture_bytes);

	// -- Convert the texture from 16-bit to 32-bit
	for (auto i = 0u; i < header.texture_bytes / sizeof(uint16_t); i++) {
		g_Project.Texture.Data[i] = tdata[i];
	}

	delete[] tdata; tdata = nullptr;

	// TODO: Call the Renderer DLL to create a texture and return a handle
	/*glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	if (Model.mTextureColor == 16)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if (Model.mTextureColor == 24)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, Model.mTexture);
	if (Model.mTextureColor == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);
		*/

		//Sprite
	RGBA16 sprite[128 * 128];
	fs.read((char*)&sprite, 128 * 128 * sizeof(uint16_t));

	//////////////////////////////////////////////////////////////////////////////
	// Animations
	if (g_Project.ObjInfo.Flags & ofANIMATED)
	{
		fs.read((char*)&anim, sizeof(anim));

		CVertexAnimation vanim;
		vanim.Name = anim.name;
		vanim.KPS = anim.kps;
		vanim.FrameCount = anim.frame_count;
		//vanim.Frames.reserve(vanim.FrameCount);
		//vanim.Frames.resize(vanim.FrameCount);

		for (auto f = vanim.FrameCount; f > 0; f--) {
			auto frame = vanim.Frames.insert(vanim.Frames.end(), CFrame());

			for (auto v = g_Project.Model.Vertices.size(); v > 0; v--) {
				glm::tvec3<int16_t> pos_compressed;
				fs.read((char*)&pos_compressed, sizeof(pos_compressed));

				// TODO: some scaling/manipulation of data

				frame->Positions.push_back(glm::vec3(pos_compressed.x, pos_compressed.y, pos_compressed.z));
			}
		}

		// Deprecated:
		/*
		vanim.Data = new int16_t[g_Project.Model.Vertices.size() * vanim.FrameCount * 3];
		fs.read((char*)vanim.Data, g_Project.Model.Vertices.size() * vanim.FrameCount * 3);
		*/

		g_Project.Animations.push_back(vanim);

		//////////////////////////////////////////////////////////////////////////////
		// Animation Trackbar
		SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)true, (LPARAM)0);

		if (g_Project.Animations.size() != 0)
		{
			//g_CurrentAnimation = 0;
			SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, g_Project.Animations.at(0).FrameCount - 1));
		}
		else
		{
			SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, 1));
		}
	}

	return true;
}


bool SaveCARFile(const std::string& filepath)
{
	struct SHeader {
		char name[32];
		uint32_t anim_count;
		uint32_t sound_count;
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t texture_bytes;
	} sHeader;

	struct SVertex {
		float x, y, z;
		int16_t bone;
		int16_t hidden;
	} sVertex;

	struct SFace {
		uint32_t v[3];
		int32_t tcoord[6];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} sFace;

	struct SAnim {
		char name[32];
		uint32_t kps;
		uint32_t frame_count;
	} sAnim;

	struct SSound {
		char name[32];
		uint32_t bytes;
	} sSound;

	std::ofstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, TEXT("Failed to open the file for reading"), TEXT("File Write Error"), MB_ICONERROR);
		return false;
	}

	std::fill(&sHeader.name[0], &sHeader.name[32], 0);
	for (auto i = 0u; i < std::min(static_cast<size_t>(32), g_Project.Model.Name.size()); i++) {
		if (i < g_Project.Model.Name.size()) sHeader.name[i] = g_Project.Model.Name.at(i);
	}
	sHeader.vert_count = g_Project.Model.Vertices.size();
	sHeader.face_count = g_Project.Model.Faces.size();
	sHeader.anim_count = g_Project.Animations.size();
	sHeader.sound_count = g_Project.SoundEffects.size();
	sHeader.texture_bytes = g_Project.Texture.Width * g_Project.Texture.Height * sizeof(uint16_t);

	// Header
	fs.write((char*)&sHeader, sizeof(SHeader));

	// Faces
	for (auto f : g_Project.Model.Faces) {
		/* Convert the native face format to the required format for this file */
		sFace.v[0] = f.v1;
		sFace.v[1] = f.v2;
		sFace.v[2] = f.v3;

		for (int i = 0; i < 3; i++) {
			sFace.tcoord[(i * 3) + 0] = static_cast<int32_t>(f.tcoord[i].x * 256);
			sFace.tcoord[(i * 3) + 1] = static_cast<int32_t>(f.tcoord[i].y * 256);
		}

		sFace.flags = f.Flags;
		sFace.dmask = f.DMask;
		sFace.prev = f.Prev;
		sFace.next = f.Next;
		sFace.group = f.Group;

		for (int i = 0; i < 3; i++) {
			sFace.reserved[i] = f.Reserved[i];
		}

		fs.write((char*)&sFace, sizeof(SFace));
	}

	// vertices
	for (auto v : g_Project.Model.Vertices)
	{
		sVertex.x = v.x;
		sVertex.y = v.y;
		sVertex.z = v.z;
		sVertex.bone = v.Bone;
		sVertex.hidden = v.Hidden;

		fs.write((char*)&sVertex, sizeof(SVertex));
	}

	// texture
	if (sHeader.texture_bytes)
	{
		std::unique_ptr<RGBA16[]> tdata(new RGBA16[g_Project.Texture.Width * g_Project.Texture.Height]);

		for (auto i = 0u; i < g_Project.Texture.Width * g_Project.Texture.Height; i++)
		{
			tdata[i] = g_Project.Texture.Data[i];
		}

		fs.write((char*)tdata.get(), sHeader.texture_bytes);
	}

	// Animations
	for (auto a : g_Project.Animations)
	{
		std::fill(&sAnim.name[0], &sAnim.name[32], 0);

		for (auto i = 0u; i < ((a.Name.size() < 32) ? a.Name.size() : 32); i++) {
			if (i < a.Name.size()) sAnim.name[i] = a.Name.at(i);
		}
		sAnim.kps = a.KPS;
		sAnim.frame_count = a.Frames.size();

		fs.write(sAnim.name, 32);
		fs.write((char*)&sAnim.kps, 4);
		fs.write((char*)&sAnim.frame_count, 4);

		for (auto frame : a.Frames)
		{
			for (auto v = 0u; v < frame.Positions.size(); v++) {
				// TODO: some sort of conversion!
				auto npos = frame.Positions.at(v);
				fs.write((char*)&npos, sizeof(npos));
			}
		}
	}

	// Sounds
	for (auto s : g_Project.SoundEffects)
	{
		std::fill(&sSound.name[0], &sSound.name[32], 0);

		for (auto i = 0u; i < ((s.Name.size() < 32) ? s.Name.size() : 32); i++) {
			if (i < s.Name.size()) sSound.name[i] = s.Name.at(i);
		}

		sSound.bytes = s.Length;
		fs.write((char*)&sSound, sizeof(SSound));

		fs.write((char*)s.Data.get(), s.Length);
	}

	// Animation-To-Sound Table
	int32_t sound_table[64];
	for (auto i = 0u; i < 64; i++) {
		if (i < g_Project.Animations.size()) {
			auto a = g_Project.Animations.at(i);
			sound_table[i] = a.SoundFX;
		} else {
			sound_table[i] = -1;
		}
	}
	fs.write((char*)sound_table, sizeof(sound_table));

	return true;
}


bool SaveOBJFile(const std::string& filepath)
{
	// -- Saves the mesh as a triangluated Wavefront OBJ
	std::ofstream fs(filepath);

	if (!fs.is_open()) {
		MessageBox(g_hMain, TEXT("Unable to open the file for writing!"), TEXT("File Not Found"), MB_ICONWARNING);
		return false;
	}

	fs << "# Created by C3Dit\n";
	fs << "# C3Dit - Wavefront OBJ Exporter Version: 1.1\n\n";
	fs << "# Vertices: " << g_Project.Model.Vertices.size() << "\n";
	fs << "# Triangles: " << g_Project.Model.Faces.size() << "\n";
	fs << "\n";

	fs << "o " << g_Project.Model.Name << "\n";
	fs << std::endl;

	for (auto vertex : g_Project.Model.Vertices) {
		fs << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
	}
	fs << std::endl;

	for (auto face : g_Project.Model.Faces)
	{
		for (int i = 0; i < 3; i++)
			fs << "vt " << face.tcoord[i].x << " " << face.tcoord[i].y << " 0.0\n";
	}
	fs << std::endl;

	for (auto f = 0u; f < g_Project.Model.Faces.size(); f++)
	{
		auto face = g_Project.Model.Faces.at(f);
		fs << "f ";
		fs << face.v1 + 1 << "/" << (f * 3) + 1 << " ";
		fs << face.v2 + 1 << "/" << (f * 3) + 2 << " ";
		fs << face.v3 + 1 << "/" << (f * 3) + 3 << "\n";
	}

	return true;
}


/* Loads a Wavefront OBJ format model
Supports:
- Tri Faces
- Quad Faces
- Texture Co-ordinates */
bool LoadOBJFile(const std::string& filepath)
{
	Settings* g = GlobalContainer::GlobalVar();
	// -- Supports Triangulated (preferred) or quadulated geometry

	const float scale = 4.0f; // TODO: Make this an importer option
	std::vector<glm::vec3> l_TexCoord;
	bool bIgnoreVerts = false;
	bool bIgnoreFaces = false;

	std::ifstream fs(filepath);

	if (!fs.is_open()) {
		MessageBox(g_hMain, TEXT("The file could not be opened for reading!"), TEXT("File Not Found"), MB_ICONWARNING);
		return false;
	}

	while (!fs.eof() && !fs.fail()) {
		// # == comment
		// \n == newline
		// \r\n == newline
		// \0 == newline
		// v == vertex
		// vn == vertex normal
		// vt == vertex texture coord
		// f == face

		std::string line = "";
		std::getline(fs, line);

		std::vector<std::string> tokens;
		std::tokenize(tokens, line, " \t");

		if (tokens.empty()) continue;

		if (tokens[0].find_first_of("#\n") != std::string::npos) continue;

		// skip unused tokens
		if (tokens[0].at(0) == 'o') continue;
		if (tokens[0].at(0) == 's') continue;
		if (tokens[0].compare("mtllib") == 0) continue;
		if (tokens[0].compare("usemtl") == 0) continue;

		// Group
		if (tokens[0].at(0) == 'g')
		{
			g_Project.Model.Name = tokens[1];
		}
		// Vertex
		else if (tokens[0].compare("v") == 0 && tokens.size() >= 4)
		{
			if (bIgnoreVerts) continue;

			if (g_Project.Model.Vertices.size() == MAX_VERTICES) { // Only do this once
				std::tstringstream ss;
				ss << TEXT("The mesh data has more than ") << MAX_VERTICES << TEXT(" vertices!\r\n");
				ss << TEXT("The games support a maximum of ") << MAX_VERTICES << TEXT(" vertices, attempting to\r\n");
				ss << TEXT("load more in the game could result in crashes and memory leaks!\r\n");

				if (g->bEnforceGameLimits) {
					ss << TEXT("Do you want to continue loading the file but start ignoring all extra vertices?\r\n");
					int m = MessageBox(g_hMain, ss.str().c_str(), TEXT("OBJ Import Error"), MB_YESNO | MB_ICONERROR);
					if (m == IDYES) { bIgnoreVerts = true; continue; }
					else if (m == IDNO) { g_Project.Clear(); break; }
				}
				else MessageBox(g_hMain, ss.str().c_str(), TEXT("OBJ Import Warning"), MB_ICONWARNING);
			}

			auto vertex = g_Project.Model.Vertices.insert(g_Project.Model.Vertices.end(), CVertex());

			vertex->x = static_cast<float>(atof(tokens[1].c_str())) * scale;
			vertex->y = static_cast<float>(atof(tokens[2].c_str())) * scale;
			vertex->z = static_cast<float>(atof(tokens[3].c_str())) * scale;
			vertex->Bone = -1;
			vertex->Hidden = false;
		}
		// Texture Coord
		else if (tokens[0].compare("vt") == 0 && tokens.size() >= 3)
		{
			glm::vec3 tcoord(
				static_cast<float>(atof(tokens[1].c_str())),
				static_cast<float>(atof(tokens[2].c_str())),
				0.0f);
			//l_TexCoord.push_back(tcoord);
		}
		// Normal
		else if (tokens[0].compare("vn") == 0 && tokens.size() >= 4)
		{
			//TODO implement this
			continue;

			glm::vec3 normal();
		}

		// Face
		if (tokens[0].compare("f") == 0 && tokens.size() >= 4)
		{
			if (bIgnoreFaces) continue;

			if (g_Project.Model.Faces.size() == MAX_TRIANGLES) { // Only do this once
				std::tstringstream ss;
				ss << "The mesh data has more than " << MAX_TRIANGLES << " faces!\r\n";
				ss << "The games support a maximum of " << MAX_TRIANGLES << " faces, attempting to\r\n";
				ss << "load more in the game could result in crashes and memory leaks!\r\n";

				if (g->bEnforceGameLimits) {
					ss << "Do you want to continue loading the file but start ignoring all extra faces?\r\n";
					int m = MessageBox(g_hMain, ss.str().c_str(), TEXT("OBJ Import Error"), MB_YESNO | MB_ICONERROR);
					if (m == IDYES) { bIgnoreVerts = true; continue; }
					else if (m == IDNO) { g_Project.Clear(); break; }
				}
				else MessageBox(g_hMain, ss.str().c_str(), TEXT("OBJ Import Warning"), MB_ICONWARNING);
			}

			auto face = g_Project.Model.Faces.insert(g_Project.Model.Faces.end(), CFace());

			std::string tok = tokens[1].substr(tokens[1].find_first_of('/'));
			if (!tok.empty())
			{
				face->v1 = atoi(tok.c_str()) - 1;

				tok = tokens[1].substr(tokens[1].find_first_of('/') + 1, std::string::npos);
				if (!tok.empty())
				{
					int t = (int)atoi(tok.c_str()) - 1;
					face->tcoord[0].x = l_TexCoord[t].x;
					face->tcoord[0].y = l_TexCoord[t].y;
				}
			}

			tok = tokens[2].substr(tokens[2].find_first_of('/'));
			if (!tok.empty())
			{
				face->v2 = atoi(tok.c_str()) - 1;

				tok = tokens[2].substr(tokens[2].find_first_of('/') + 1, std::string::npos);
				if (!tok.empty())
				{
					int t = atoi(tok.c_str()) - 1;
					face->tcoord[1].x = l_TexCoord[t].x;
					face->tcoord[1].y = l_TexCoord[t].y;
				}
			}

			tok = tokens[3].substr(tokens[3].find_first_of('/'));
			if (!tok.empty())
			{
				face->v3 = atoi(tok.c_str()) - 1;

				tok = tokens[3].substr(tokens[3].find_first_of('/') + 1, std::string::npos);
				if (!tok.empty())
				{
					int t = atoi(tok.c_str()) - 1;
					face->tcoord[2].x = l_TexCoord[t].x;
					face->tcoord[2].y = l_TexCoord[t].y;
				}
			}

			// -- Quad
			if (tokens.size() >= 5)
			{
				tok = tokens[4].substr(tokens[4].find_first_of('/'));
				if (!tok.empty())
				{
					auto pface = face;
					face = g_Project.Model.Faces.insert(g_Project.Model.Faces.end(), CFace());

					face->v1 = pface->v3;
					face->v2 = atoi(&tok[0]) - 1;
					face->v3 = pface->v1;

					tok = tokens[4].substr(tokens[4].find_first_of('/') + 1, std::string::npos);
					if (!tok.empty())
					{
						int i = atoi(tok.c_str()) - 1;
						face->tcoord[0].x = pface->tcoord[2].x;
						face->tcoord[0].y = pface->tcoord[2].y;
						face->tcoord[1].x = l_TexCoord[i].x;
						face->tcoord[1].y = l_TexCoord[i].y;
						face->tcoord[2].x = pface->tcoord[0].x;
						face->tcoord[2].y = pface->tcoord[0].y;
					}
				}
			}
		}
	}

	// -- Normals
	/*bool vlist[MAX_VERTICES];
	ZeroMemory(vlist, sizeof(bool));
	for (int f = 0; f < Model.mFaceCount; f++)
	{
		int v1 = g_Triangles[f].v1;
		int v2 = g_Triangles[f].v2;
		int v3 = g_Triangles[f].v3;
		g_Normals[f] = ComputeNormals(g_Verticies[v1], g_Verticies[v2], g_Verticies[v3]);
		if (!vlist[v1])
		{
			g_VNormals[v1] = g_Normals[f];
			vlist[v1] = true;
		}
		else
		{
			g_VNormals[v1] += g_Normals[f];
			g_VNormals[v1] /= 2;
		}
		if (!vlist[v2])
		{
			g_VNormals[v2] = g_Normals[f];
			vlist[v2] = true;
		}
		else
		{
			g_VNormals[v2] += g_Normals[f];
			g_VNormals[v2] /= 2;
		}
		if (!vlist[v3])
		{
			g_VNormals[v3] = g_Normals[f];
			vlist[v3] = true;
		}
		else
		{
			g_VNormals[v3] += g_Normals[f];
			g_VNormals[v3] /= 2;
		}
	}*/

	return true;
}


bool Load3DNFile(const std::string& filepath)
{
	bool vertices_have_bones = false;

	std::ifstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, TEXT("The selected file does not exist or is corrupt!"), TEXT("File Error"), MB_ICONEXCLAMATION);
		return false;
	}

	struct SHeader {
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t bone_count;
		char name[32];
		uint32_t has_sprite;
		//char sprite_name[32];
	} sHeader;

	struct SVertex {
		float x, y, z;
		int16_t bone;
		int16_t hidden;
	} sVertex;

	struct SFace {
		uint32_t v[3];
		int16_t tcoord[6];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} sFace;

	struct SVert {
		float x, y, z;
		int16_t bone;
		int16_t hide;
	} sVert;

	struct SBone {
		char name[32];
		float x, y, z;
		int16_t parent;
		int16_t hide;
	} sBone;

	// -- Header
	fs.read((char*)&sHeader, sizeof(SHeader));

	if (sHeader.has_sprite) {
		fs.seekg(32, std::ios::cur);
	}

	if (sHeader.vert_count == 0) {
		MessageBox(g_hMain, TEXT("There are no vertices in this file!"), TEXT("File Error"), MB_ICONEXCLAMATION);
		return false;
	}
	if (sHeader.face_count == 0) {
		MessageBox(g_hMain, TEXT("There are no triangles in this file!"), TEXT("File Error"), MB_ICONEXCLAMATION);
		return false;
	}

	// -- Vertices
	for (auto v = 0u; v < sHeader.vert_count; v++)
	{
		CVertex nvertex;
		fs.read((char*)&nvertex, sizeof(CVertex));
		g_Project.Model.Vertices.push_back(nvertex);

		if (nvertex.Bone != -1 && !vertices_have_bones)
			vertices_have_bones = true;
	}

	// -- Triangles
	for (auto t = 0u; t < sHeader.face_count; t++)
	{
		fs.read((char*)&sFace, sizeof(SFace));

		auto nface = g_Project.Model.Faces.insert(g_Project.Model.Faces.end(), CFace());

		nface->v1 = sFace.v[0];
		nface->v2 = sFace.v[1];
		nface->v3 = sFace.v[2];

		for (int i = 0; i < 3; i++) {
			nface->tcoord[i].x = static_cast<float>(sFace.tcoord[(i * 3) + 0]) / 256.f;
			nface->tcoord[i].y = static_cast<float>(sFace.tcoord[(i * 3) + 1]) / 256.f;
		}

		nface->Flags = sFace.flags;
		nface->DMask = sFace.dmask;
		nface->Prev = sFace.prev;
		nface->Next = sFace.next;
		nface->Group = sFace.group;

		for (int i = 0; i < 3; i++) {
			nface->Reserved[i] = sFace.reserved[i];
		}
	}

	// -- Normals
	std::array<bool, MAX_VERTICES> vlist;
	std::fill(vlist.begin(), vlist.end(), false);

	for (auto face : g_Project.Model.Faces) {
		int v1 = face.v1;
		int v2 = face.v2;
		int v3 = face.v3;
		g_Normals[f] = ComputeNormals(g_Verticies[v1], g_Verticies[v2], g_Verticies[v3]);
		if (!vlist[v1])
		{
			g_VNormals[v1] = g_Normals[f];
			vlist[v1] = true;
		}
		else
		{
			g_VNormals[v1] += g_Normals[f];
			g_VNormals[v1] /= 2;
		}
		if (!vlist[v2])
		{
			g_VNormals[v2] = g_Normals[f];
			vlist[v2] = true;
		}
		else
		{
			g_VNormals[v2] += g_Normals[f];
			g_VNormals[v2] /= 2;
		}
		if (!vlist[v3])
		{
			g_VNormals[v3] = g_Normals[f];
			vlist[v3] = true;
		}
		else
		{
			g_VNormals[v3] += g_Normals[f];
			g_VNormals[v3] /= 2;
		}
	}

	// -- Bones
	for (auto b = 0u; b < header.bone_count; b++)
	{
		CBone nbone;
		fs.read((char*)&nbone, sizeof(nbone));
		g_Project.Model.Bones.push_back(nbone);
	}

	// -- Texture
	g_Project.Texture.Width = 16;
	g_Project.Texture.Height = 16;
	g_Project.Texture.Data.reset(new RGBA[16 * 16]);
	std::memset(g_Project.Texture.Data.get(), 255, 16 * 16 * sizeof(RGBA));

	// OpenGL Texture goes here
	//.... TODO ....//

	//////////////////////////////////////////////////////////////////////////////
	// If bones are missing create approximate data
	//TODO: have a setting for this
	if (vertices_have_bones && g_Project.Model.Bones.empty()) {
		CreateMissingBoneData();
	}

	// -- Close the file
	return true;
}


bool Load3DFFile(const std::string& filepath)
{
	struct SHeader {
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t bone_count;
		uint32_t texture_bytes;
	} sHeader;

	struct SFace {
		uint32_t v[3];
		int32_t tx[3];
		int32_t ty[3];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} sFace;

	struct SVertex {
		float x, y, z;
		int16_t bone;
		int16_t hide;
	} sVertex;

	struct SBone {
		char name[32];
		float x, y, z;
		int16_t parent;
		int16_t hide;
	} sBone;

	std::ifstream fs(filepath, std::ios::binary);

	if (!fs.is_open() || fs.fail())
	{
		MessageBox(g_hMain, TEXT("Unable to open the file for reading!"), TEXT("File Open Error"), MB_ICONWARNING);
		return false;
	}

	// -- Header
	fs.read((char*)&sHeader, sizeof(SHeader));

	if (sHeader.vert_count == 0) {
		MessageBox(g_hMain, TEXT("There are no vertices in this file!"), TEXT("File Error"), MB_ICONEXCLAMATION);
		return false;
	}
	if (sHeader.face_count == 0) {
		MessageBox(g_hMain, TEXT("There are no triangles in this file!"), TEXT("File Error"), MB_ICONEXCLAMATION);
		return false;
	}

	// -- Triangles
	for (auto t = 0u; t < sHeader.face_count; t++) {
		fs.read((char*)&sFace, sizeof(SFace));

		auto fi = g_Project.Model.Faces.insert(g_Project.Model.Faces.end(), CFace());
		fi->v1 = sFace.v[0];
		fi->v2 = sFace.v[1];
		fi->v3 = sFace.v[2];
		for (int i = 0; i < 3; i++) {
			fi->tcoord[i].x = static_cast<float>(sFace.tx[i]) / 256.f;
			fi->tcoord[i].y = static_cast<float>(sFace.tx[i]) / 256.f;
		}
		fi->Flags = sFace.flags;
		fi->DMask = sFace.dmask;
		fi->Prev = sFace.prev;
		fi->Next = sFace.next;
		fi->Group = sFace.group;
		for (int i = 0; i < 3; i++)
			fi->Reserved[i] = sFace.reserved[i];
	}

	// -- Vertices
	for (auto v = 0u; v < sHeader.vert_count; v++) {
		CVertex vi;
		fs.read((char*)&vi, sizeof(vi));
		g_Project.Model.Vertices.push_back(vi);
	}

	// -- Bones
	for (auto b = 0u; b < sHeader.bone_count; b++)
	{
		fs.read((char*)&sBone, sizeof(SBone));
		CBone bi { sBone.name, sBone.x, sBone.y, sBone.z, sBone.parent, sBone.hide };
		g_Project.Model.Bones.push_back(bi);

	}

	//Texture
	g_Project.Texture.Data.reset(new RGBA[header.texture_bytes / 2]);
	fs.read((char*)g_Project.Texture.Data.get(), header.texture_bytes);

	/*
	glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

	if (Model.mTextureColor == 16)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if (Model.mTextureColor == 32)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);
	*/

	// -- Close the file
	fs.close();

	// -- Perform any post loading operations
	// -- Normals
	/*bool vlist[MAX_VERTICES];
	ZeroMemory(vlist, sizeof(bool));
	for (int f = 0; f < Model.mFaceCount; f++)
	{
		int v1 = g_Triangles[f].v1;
		int v2 = g_Triangles[f].v2;
		int v3 = g_Triangles[f].v3;
		g_Normals[f] = ComputeNormals(g_Verticies[v1], g_Verticies[v2], g_Verticies[v3]);
		if (!vlist[v1])
		{
			g_VNormals[v1] = g_Normals[f];
			vlist[v1] = true;
		}
		else
		{
			g_VNormals[v1] += g_Normals[f];
			g_VNormals[v1] /= 2;
		}
		if (!vlist[v2])
		{
			g_VNormals[v2] = g_Normals[f];
			vlist[v2] = true;
		}
		else
		{
			g_VNormals[v2] += g_Normals[f];
			g_VNormals[v2] /= 2;
		}
		if (!vlist[v3])
		{
			g_VNormals[v3] = g_Normals[f];
			vlist[v3] = true;
		}
		else
		{
			g_VNormals[v3] += g_Normals[f];
			g_VNormals[v3] /= 2;
		}
	}*/

	return true;
}


bool Save3DFFile(const std::string& filepath)
{
	struct SHeader {
		uint32_t vert_count;
		uint32_t face_count;
		uint32_t bone_count;
		uint32_t texture_bytes;
	} sHeader;

	struct SFace {
		uint32_t v[3];
		int32_t tx[3];
		int32_t ty[3];
		uint16_t flags;
		uint16_t dmask;
		int32_t prev;
		int32_t next;
		uint32_t group;
		uint32_t reserved[3];
	} sFace;

	struct SVertex {
		float x, y, z;
		int16_t bone;
		int16_t hide;
	} sVertex;

	struct SBone {
		char name[32];
		float x, y, z;
		int16_t parent;
		int16_t hide;
	} sBone;

	sHeader.vert_count = g_Project.Model.Vertices.size();
	sHeader.face_count = g_Project.Model.Faces.size();
	sHeader.bone_count = g_Project.Model.Bones.size();
	sHeader.texture_bytes = g_Project.Texture.Width * g_Project.Texture.Height * sizeof(uint16_t);

	std::ofstream fs(filepath, std::ios::binary);

	if (!fs.is_open()) {
		MessageBox(g_hMain, TEXT("Failed to open the file for writing!"), TEXT("File open error"), MB_ICONWARNING);
		return false;
	}

	// -- Header
	fs.write((char*)&sHeader, sizeof(SHeader));

	// faces
	for (auto f : g_Project.Model.Faces)
	{
		sFace.v[0] = f.v1;
		sFace.v[1] = f.v2;
		sFace.v[2] = f.v3;

		for (int i = 0; i < 3; i++) {
			sFace.tx[i] = static_cast<int32_t>(f.tcoord[i].x * 256);
			sFace.ty[i] = static_cast<int32_t>(f.tcoord[i].y * 256);
		}

		sFace.flags = f.Flags;
		sFace.dmask = f.DMask;
		sFace.prev = f.Prev;
		sFace.next = f.Next;
		sFace.group = f.Group;

		for (int i = 0; i < 3; i++) {
			sFace.reserved[i] = f.Reserved[i];
		}

		fs.write((char*)&sFace, sizeof(SFace));
	}

	// vertices
	for (auto v : g_Project.Model.Vertices)
	{
		sVertex.x = v.x;
		sVertex.y = v.y;
		sVertex.z = v.z;
		sVertex.bone = v.Bone;
		sVertex.hide = v.Hidden;

		fs.write((char*)&sVertex, sizeof(SVertex));
	}

	//Bones
	for (auto b : g_Project.Model.Bones) {
		std::fill(&sBone.name[0], &sBone.name[32], 0);
		std::copy(b.Name.begin(), b.Name.end(), sBone.name);
		sBone.x = b.x;
		sBone.y = b.y;
		sBone.z = b.z;
		sBone.parent = b.Parent;
		sBone.hide = b.Hide;
		fs.write((char*)&sBone, sizeof(SBone));
	}

	// Texture
	if (sHeader.texture_bytes)
	{
		std::unique_ptr<RGBA16[]> tdata(new RGBA16[g_Project.Texture.Width * g_Project.Texture.Height]);

		for (auto i = 0u; i < g_Project.Texture.Width * g_Project.Texture.Height; i++)
		{
			tdata[i] = g_Project.Texture.Data[i];
		}

		fs.write((char*)tdata.get(), sHeader.texture_bytes);
	}

	return true;
}

/*
* !DEPRECATED!
* Save  to a C2Object format for machf's editors
*/
bool SaveC2OFile(const std::string& filepath)
{
	/*FILE* fp = fopen(fname, "wb");

	if (g_Animations[0].FrameCount) Model.mObjInfo.flags |= ofANIMATED;

	//--Header
	fwrite(&Model.mObjInfo, 1, 48, fp);
	fwrite(&Model.mObjInfo.res, 1, 16, fp);
	fwrite(&Model.mVertCount, 1, 4, fp);
	fwrite(&Model.mFaceCount, 1, 4, fp);
	fwrite(&Model.mBoneCount, 1, 4, fp);
	fwrite(&Model.mTextureSize, 1, 4, fp);

	//Triangles
	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fwrite(&g_Triangles[t], 64, 1, fp);
	}

	//Verticies
	for (int v = 0; v < Model.mVertCount; v++)
	{
		fwrite(&g_Verticies[v], sizeof(VERTEX), 1, fp);
	}

	//Bones
	for (int b = 0; b < Model.mBoneCount; b++)
	{
		fwrite(&g_Bones[b], sizeof(BONE), 1, fp);
	}

	//Texture
	fwrite(Model.mTexture, Model.mTextureSize, 1, fp);

	//Sprite
	if (!(Model.mObjInfo.flags & ofNOBMP))
		fwrite(&g_Sprite, 128 * 128, 2, fp);

	//Animation
	if (g_Animations[0].FrameCount)
	{
		fwrite(&g_Animations[0].Name, 32, 1, fp);
		fwrite(&g_Animations[0].KPS, 4, 1, fp);
		fwrite(&g_Animations[0].FrameCount, 4, 1, fp);
		fwrite(g_Animations[0].Data, g_Animations[0].FrameCount * Model.mVertCount * 6, 1, fp);
	}

	//Close the file
	fclose(fp);

	return true;*/
	return false;
}


/* Load a project from disk
TODO: Replace with JSON (https://github.com/nlohmann/json)
WARNING: Incomplete! */
bool LoadProjectFile(const std::string& filepath)
{
	std::string line = "";
	std::ifstream fs(filepath);

	if (!fs.is_open())
	{
		return false;
	}

	while (!std::getline(fs, line).eof())
	{
		std::stringstream ss;
		ss << std::uppercase << line.substr(line.find_first_of(' '));
		std::string token = ss.str();
		ss.str("");
		ss.clear();

		if (!token.compare("VERSION")) {
			// Get the version from line
			ss.str(line.substr(line.find_first_of(' ') + 1, std::string::npos));

		}
		else
		{
		}
	}

	//fs << "Version " << iProjectVersion << std::endl;

	return true;
}


/* Save the current project to disk
TODO: Replace with JSON (https://github.com/nlohmann/json)
TODO: Complete */
bool SaveProjectFile(const std::string& filepath)
{
	std::string model_fp = "";
	std::string texture_fp = "";
	std::string sprite_fp = "";
	std::vector<std::string> animations_fp;
	std::vector<std::string> soundfx_fp;

	std::ofstream fs(filepath);

	if (!fs.is_open()) {
		MessageBox(g_hMain, TEXT("Failed to open the file for writing."), TEXT("File open error"), MB_ICONWARNING);
		return false;
	}

	// TODO: export the parts to the "Projects/" directory

	fs << "Type Character\n";
	fs << "Version " << iProjectVersion << std::endl;

	fs << "ObjectInfo {\n";
	// TODO: populate
	fs << "} //ObjectInfo\n";

	fs << "Model {\n";
	fs << "\'" << model_fp << "\'\n";

	fs << "Texture \'" << texture_fp << "\'\n";

	fs << "Sprite \'" << sprite_fp << "\'\n";

	fs << "Animations {\n";
	for (auto a = 0u; a < g_Project.Animations.size(); a++)
		fs << "\t\'" << animations_fp.at(a) << "\'\n";
	fs << "}\n";

	fs << "Sounds {\n";
	for (auto s = 0u; s < g_Project.SoundEffects.size(); s++)
		fs << "\t\'" << soundfx_fp.at(s) << "\'\n";
	fs << "}\n";

	fs << "AnimSoundTable {\n";
	for (auto a = 0u; a < g_Project.Animations.size(); a++)
		fs << "\t" << g_Project.Animations.at(a).SoundFX << "\n";
	fs << "}\n";

	fs << std::endl; // Flush

	return true;
}


bool CVertexAnimation::Save(const std::string& filepath)
{
	if (filepath.empty())
		return false;

	std::ofstream f(filepath, std::ios::binary);
	if (!f.is_open()) {
		return false;
	}

	FrameCount = Frames.size();

	uint32_t vert_count = g_Project.Model.Vertices.size();
	f.write((char*)&vert_count, 4);
	f.write((char*)&KPS, 4);
	f.write((char*)&FrameCount, 4);
	for (auto frame : Frames) {
		for (auto pos : frame.Positions) {
			//tvec3<int16_t> npos((int16_t)pos.x, (int16_t)pos.y, (int16_t)pos.z);
			//f.write((char*)&npos, sizeof(npos));
		}
	}

	return true;
}


bool CVertexAnimation::Load(const std::string& filepath)
{
	if (filepath.empty())
		return false;

	std::ifstream fs(filepath, std::ios::binary);
	if (!fs.is_open()) {
		return false;
	}

	uint32_t vert_count = g_Project.Model.Vertices.size();
	fs.read((char*)&vert_count, 4);

	if (vert_count != g_Project.Model.Vertices.size())
	{
		MessageBox(g_hMain, TEXT("The VTL file has an incorrect number of vertices!"), TEXT("Invalid file data"), MB_ICONWARNING);
		return false;
	}

	fs.read((char*)&KPS, 4);
	fs.read((char*)&FrameCount, 4);
	for (auto frame : Frames) {
		for (auto pos : frame.Positions) {
			/*tvec3<int16_t> npos;
			fs.read((char*)&npos, sizeof(npos));
			pos.x = static_cast<float>(npos.x);
			pos.y = static_cast<float>(npos.y);
			pos.z = static_cast<float>(npos.z);*/
		}
	}

	return true;
}
