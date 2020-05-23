#include "stdafx.h"

#include "C3Dit.h"

#include <iostream>
#include <fstream>


void LoadCARData(char* fname)
{
	std::ifstream fs(fname);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, "There was an error when opening the file.", "FILE ERROR", MB_ICONERROR);
		return;
	}

	//g_Scene.Reset();

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
		uint32_t prev;
		uint32_t next;
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
	g_Scene.Model.Name = header.name;

	//////////////////////////////////////////////////////////////////////////////
	// Triangles
	for (auto f = header.face_count; f > 0; f--)
	{
		fs.read((char*)&face, sizeof(face));

		auto nface = g_Scene.Model.Faces.insert(g_Scene.Model.Faces.end(), CFace());
		nface->v1 = face.v[0]; nface->v2 = face.v[1]; nface->v3 = face.v[2];
		for (int i = 0; i < 3; i++) {
			nface->tcoord[i].x = face.tx[i];
			nface->tcoord[i].y = face.ty[i];
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

	g_Scene.ObjInfo.YLo = (signed)0x7FFFFFFF;
	g_Scene.ObjInfo.YHi = (signed)0x80000000;

	std::cout << "g_Scene.ObjInfo.YLo = " << g_Scene.ObjInfo.YLo << std::endl;
	std::cout << "g_Scene.ObjInfo.YHi = " << g_Scene.ObjInfo.YHi << std::endl;

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

		g_Scene.Model.Vertices.push_back(vertex);

		if (vert.y < g_Scene.ObjInfo.YLo)
			g_Scene.ObjInfo.YLo = vert.y;
		if (vert.y > g_Scene.ObjInfo.YHi)
			g_Scene.ObjInfo.YHi = vert.y;
	}

	// TODO: Normal calculation

	// -- Set the camera to look at the middle of the model
	g_Camera.TargetX = 0.f;
	g_Camera.TargetY = (g_Scene.ObjInfo.YHi - g_Scene.ObjInfo.YLo) / 2.f;
	g_Camera.TargetZ = 0.f;

	//////////////////////////////////////////////////////////////////////////////
	// Texture Block
	g_Scene.Texture.Clear();
	g_Scene.Texture.Width = 256;
	g_Scene.Texture.Height = 256;

	if (header.texture_bytes != 256 * 256 * sizeof(uint16_t))
	{
		// Some sort of message/warning
		// maybe just make a 256x512 texture space, or 512x512 depending...
		std::string msg = "The texture is not 256x256.\r\n";
		msg += "Cancel: Cancel loading the file.";
		msg += "Try: Resume loading and attempt to resize the texture.\r\n";
		msg += "Continue: Resume loading without modifying the texture.\r\n";
		int mbr = MessageBox(g_hMain, msg.c_str(), "Non-Square Texture", MB_ICONWARNING | MB_CANCELTRYCONTINUE);
	}
	
	if (g_Scene.Texture.Data)
		delete[] g_Scene.Texture.Data;

	g_Scene.Texture.Data = new RGBA[header.texture_bytes / sizeof(uint16_t)];
	RGBA16* tdata = new RGBA16[header.texture_bytes / sizeof(uint16_t)];

	fs.read((char*)tdata, header.texture_bytes);

	// -- Convert the texture from 16-bit to 32-bit
	for (auto i = 0u; i < header.texture_bytes / sizeof(uint16_t); i++) {
		g_Scene.Texture.Data[i] = tdata[i];
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

	//////////////////////////////////////////////////////////////////////////////
	// Animations
	for (auto a = header.anim_count; a > 0; a--)
	{
		fs.read((char*)&anim, sizeof(anim));

		CVertexAnimation vanim;
		vanim.Name = anim.name;
		vanim.KPS = anim.kps;
		vanim.FrameCount = anim.frame_count;
		vanim.Frames.reserve(vanim.FrameCount);
		vanim.Frames.resize(vanim.FrameCount);

		for (auto f = vanim.FrameCount; f > 0; f--)
		{
			auto frame = vanim.Frames.insert(vanim.Frames.end(), CFrame<vec3>());
			
			for (auto v = g_Scene.Model.Vertices.size(); v > 0; v--)
			{
				tvec3<int16_t> vec;
				fs.read((char*)&vec, sizeof(tvec3<int16_t>));

				// TODO: some scaling/manipulation of data
				frame->Positions.push_back(vec3(vec.x, vec.y, vec.z));
			}
		}

		// Deprecated:
		/*
		vanim.Data = new int16_t[g_Scene.Model.Vertices.size() * vanim.FrameCount * 3];
		fs.read((char*)vanim.Data, g_Scene.Model.Vertices.size() * vanim.FrameCount * 3);
		*/

		g_Scene.Animations.push_back(vanim);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Animation Trackbar
	SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)true, (LPARAM)0);

	if (g_Scene.Animations.size() != 0)
	{
		//g_CurrentAnimation = 0;
		SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, g_Scene.Animations.at(0).FrameCount - 1));
	}
	else
	{
		SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, 1));
	}

	//////////////////////////////////////////////////////////////////////////////
	// Sound effects
	for (auto s = 0u; s < g_Scene.SoundEffects.size(); s++)
	{
		auto sound = g_Scene.SoundEffects.insert(g_Scene.SoundEffects.end(), CSound());

		sound->Name.reserve(32);
		sound->Name.resize(32);

		fs.read(sound->Name.data(), 32);
		fs.read((char*)&sound->Length, 4);

		sound->Data = new int16_t[sound->Length / sizeof(int16_t)];

		fs.read((char*)sound->Data, sound->Length);
	}

	//////////////////////////////////////////////////////////////////////////////
	// Animation-Sound Table
	int32_t AnimFX[64];
	fs.read((char*)AnimFX, 64 * sizeof(int32_t));

	for (auto a = 0u; a < g_Scene.Animations.size(); a++)
	{
		try { g_Scene.Animations.at(a).SoundFX = AnimFX[a]; }
		catch (std::out_of_range& oor) { std::cerr << "C++ Exception!\n" << oor.what() << "\nUnable to access animation[" << a << "]" << std::endl; };
	}

	UpdateWindow(g_hMain);
}



void LoadC2OData(char* fname)
{
	std::ifstream fs(fname, std::ios::binary);
	if (!fs.is_open())
	{
		MessageBox(NULL, "There was an error when opening the file.", "File error", MB_ICONERROR);
		return;
	}

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
		uint32_t prev;
		uint32_t next;
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

	CModel& model = g_Scene.Model;

	//////////////////////////////////////////////////////////////////////////////
	// Header
	model.Name, "<Object>";
	fs.read((char*)&g_Scene.ObjInfo, sizeof(CObjectInfo)); // 48 + 16
	fs.read((char*)header.vert_count, 4);
	fs.read((char*)header.face_count, 4);
	fs.read((char*)header.bone_count, 4);
	fs.read((char*)header.texture_bytes, 4);

	//////////////////////////////////////////////////////////////////////////////
	// Faces
	for (auto f = 0u; f < header.face_count; f++)
	{
		auto nface = model.Faces.insert(model.Faces.end(), CFace());
		
		fs.read((char*)&face, sizeof(face));

		nface->v1 = face.v[0]; nface->v2 = face.v[1]; nface->v3 = face.v[2];
		for (int i = 0; i < 3; i++) {
			nface->tcoord[i].x = face.tx[i];
			nface->tcoord[i].y = face.ty[i];
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
	g_Camera.TargetY = (g_Scene.ObjInfo.YHi - g_Scene.ObjInfo.YLo) / 2.f;
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

		auto abone = model.Bones.insert(model.Bones.end(), CBone());

		abone->Name = bone.name;
		abone->x = bone.x;
		abone->y = bone.y;
		abone->z = bone.z;
		abone->Parent = bone.parent;
		abone->Hide = bone.hide;
	}

	//////////////////////////////////////////////////////////////////////////////
	// Texture Block
	g_Scene.Texture.Clear();
	g_Scene.Texture.Width = 256;
	g_Scene.Texture.Height = 256;

	if (header.texture_bytes != 256 * 256 * sizeof(uint16_t))
	{
		// Some sort of message/warning
		// maybe just make a 256x512 texture space, or 512x512 depending...
		std::string msg = "The texture is not 256x256.\r\n";
		msg += "Cancel: Cancel loading the file.";
		msg += "Try: Resume loading and attempt to resize the texture.\r\n";
		msg += "Continue: Resume loading without modifying the texture.\r\n";
		int mbr = MessageBox(g_hMain, msg.c_str(), "Non-Square Texture", MB_ICONWARNING | MB_CANCELTRYCONTINUE);
	}

	if (g_Scene.Texture.Data)
		delete[] g_Scene.Texture.Data;

	g_Scene.Texture.Data = new RGBA[header.texture_bytes / sizeof(uint16_t)];
	RGBA16* tdata = new RGBA16[header.texture_bytes / sizeof(uint16_t)];

	fs.read((char*)tdata, header.texture_bytes);

	// -- Convert the texture from 16-bit to 32-bit
	for (auto i = 0u; i < header.texture_bytes / sizeof(uint16_t); i++) {
		g_Scene.Texture.Data[i] = tdata[i];
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
	if (g_Scene.ObjInfo.Flags & ofANIMATED)
	{
		fs.read((char*)&anim, sizeof(anim));

		CVertexAnimation vanim;
		vanim.Name = anim.name;
		vanim.KPS = anim.kps;
		vanim.FrameCount = anim.frame_count;
		vanim.Frames.reserve(vanim.FrameCount);
		vanim.Frames.resize(vanim.FrameCount);

		for (auto f = vanim.FrameCount; f > 0; f--)
		{
			auto frame = vanim.Frames.insert(vanim.Frames.end(), CFrame<vec3>());

			for (auto v = g_Scene.Model.Vertices.size(); v > 0; v--)
			{
				tvec3<int16_t> vec;
				fs.read((char*)&vec, sizeof(tvec3<int16_t>));

				// TODO: some scaling/manipulation of data
				frame->Positions.push_back(vec3(vec.x, vec.y, vec.z));
			}
		}

		// Deprecated:
		/*
		vanim.Data = new int16_t[g_Scene.Model.Vertices.size() * vanim.FrameCount * 3];
		fs.read((char*)vanim.Data, g_Scene.Model.Vertices.size() * vanim.FrameCount * 3);
		*/

		g_Scene.Animations.push_back(vanim);

		//////////////////////////////////////////////////////////////////////////////
		// Animation Trackbar
		SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)true, (LPARAM)0);

		if (g_Scene.Animations.size() != 0)
		{
			//g_CurrentAnimation = 0;
			SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, g_Scene.Animations.at(0).FrameCount - 1));
		}
		else
		{
			SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)true, (LPARAM)MAKELONG(0, 1));
		}
	}

	fs.close();
}


void SaveCARData(char* fname)
{
	std::string filepath = fname;
	std::ofstream fs(filepath, std::ios::binary);

	if (!fs.is_open())
	{
		MessageBox(g_hMain, "Failed to open the file for reading", "File Error", MB_ICONERROR);
		return;
	}

	fs.write((char*)fname, 1);

	fs.close();

	FILE* fp = fopen(fname, "wb");

	// -- C3Dit watermark
	sprintf(Model.mTag, "C3DIT");

	//--Header
	fwrite(Model.mName, 1, 24, fp);
	fwrite(Model.mTag, 1, 8, fp);
	fwrite(&Model.mAnimCount, 1, 4, fp);
	fwrite(&Model.mSoundCount, 1, 4, fp);
	fwrite(&Model.mVertCount, 1, 4, fp);
	fwrite(&Model.mFaceCount, 1, 4, fp);
	fwrite(&Model.mTextureSize, 1, 4, fp);

	//Triangles
	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fwrite(&g_Triangles[t], 64, 1, fp);
	}

	//Verticies
	for (int v = 0; v < Model.mVertCount; v++)
	{
		fwrite(&g_Verticies[v], 16, 1, fp);
	}

	//Texture
	for (int i = 0; i < 256 * 256; i++)
	{
		WORD w = *((WORD*)Model.mTexture + i);
		int B = ((w >> 0) & 31);
		int G = ((w >> 5) & 31);
		int R = ((w >> 10) & 31);
		int A = 0;
		*((WORD*)Model.mTexture + i) = (B)+(G << 5) + (R << 10) + (A << 15);
	}
	fwrite(Model.mTexture, Model.mTextureSize, 1, fp);

	//Animations
	for (int a = 0; a < Model.mAnimCount; a++)
	{
		fwrite(&g_Animations[a].Name, 32, 1, fp);
		fwrite(&g_Animations[a].KPS, 1, 4, fp);
		fwrite(&g_Animations[a].FrameCount, 1, 4, fp);
		fwrite(g_Animations[a].Data, Model.mVertCount * g_Animations[a].FrameCount * 3, sizeof(short), fp);
	}

	//Sound effects
	for (int s = 0; s < Model.mSoundCount; s++)
	{
		fwrite(&g_Sounds[s].name, 32, 1, fp);
		fwrite(&g_Sounds[s].len, 1, 4, fp);
		fwrite(g_Sounds[s].data, g_Sounds[s].len, 1, fp);
	}

	//Animation-Sound Table
	fwrite(&Model.mAnimSFXTable, 64, 4, fp);

	//Close the file
	fclose(fp);
}


void SaveOBJData(char* fname)
{
	// -- Saves the mesh as a triangluated Wavefront OBJ
	FILE* fp = fopen(fname, "w");

	fprintf(fp, "# Created by C3Dit\n");
	fprintf(fp, "# C3Dit (c) Rexhunter99 2009\n");
	fprintf(fp, "# C3Dit - Wavefront OBJ Exporter Version: 1.0\n\n");

	fprintf(fp, "# Vertices: %u\n", (UINT)Model.mVertCount);
	fprintf(fp, "# Triangles: %u\n", (UINT)Model.mFaceCount);

	fprintf(fp, "\no %s\n\n", Model.mName);

	for (int v = 0; v < Model.mVertCount; v++)
	{
		fprintf(fp, "v %f %f %f\n", g_Verticies[v].x, g_Verticies[v].z, g_Verticies[v].y);
	}

	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fprintf(fp, "vt %f %f 0.0\n", (float)g_Triangles[t].tx1 / 256.0f, (float)g_Triangles[t].ty1 / 256.0f);
		fprintf(fp, "vt %f %f 0.0\n", (float)g_Triangles[t].tx2 / 256.0f, (float)g_Triangles[t].ty2 / 256.0f);
		fprintf(fp, "vt %f %f 0.0\n", (float)g_Triangles[t].tx3 / 256.0f, (float)g_Triangles[t].ty3 / 256.0f);
	}

	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fprintf(fp, "f %u/%u %u/%u %u/%u\n",
			g_Triangles[t].v1 + 1, (t * 3) + 1,
			g_Triangles[t].v2 + 1, (t * 3) + 2,
			g_Triangles[t].v3 + 1, (t * 3) + 3);
	}

	fclose(fp);

	return;
}

void LoadOBJData(char* fname)
{
	// -- Supports Triangulated (preferred) or quadulated geometry

	UINT num_texcoords = 0;
	vector<VERTEX> l_TexCoord;

	// -- Clear the scene
	newScene();

	FILE* fp = fopen(fname, "r");

	while (!feof(fp))
	{
		// # == comment
		// \n == newline
		// \r\n == newline
		// \0 == newline
		// v == vertex
		// vn == vertex normal
		// vt == vertex texture coord
		// f == face

		char str[256];
		fgets(str, 256, fp);
		// -- End Of Line reached

		char* tok = strtok(str, " ");
		//tok = strtok( NULL, " " );

		if (tok[0] == '#') continue;
		if (tok[0] == '\n') continue;
		// skip unused tokens
		if (tok[0] == 'o') continue;
		if (tok[0] == 's') continue;

		if (strcmp(tok, "mtllib") == 0) continue;
		if (strcmp(tok, "usemtl") == 0) continue;

		// Group
		if (tok[0] == 'g')
		{
			tok = strtok(NULL, " ");

			strncpy(Model.mName, tok, 24);
		}

		// Vertex
		if (tok[0] == 'v' && tok[1] == '\0')
		{
			if (Model.mVertCount >= MAX_VERTICES) continue;

			g_Verticies[Model.mVertCount].x = atof(tok = strtok(NULL, " ")) * 4.0f;
			g_Verticies[Model.mVertCount].y = atof(tok = strtok(NULL, " ")) * 4.0f;
			g_Verticies[Model.mVertCount].z = atof(tok = strtok(NULL, " ")) * 4.0f;
			char name[256];
			sprintf(name, "Vertices #%u", (UINT)Model.mVertCount);
			g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], name, LPARAM(&g_Verticies[Model.mVertCount])));
			Model.mVertCount++;
		}

		// Texture Coord
		if (tok[0] == 'v' && tok[1] == 't')
		{
			VERTEX Coords;
			Coords.x = atof(tok = strtok(NULL, " "));
			Coords.y = atof(tok = strtok(NULL, " "));
			l_TexCoord.push_back(Coords);

			num_texcoords++;
		}

		// Normal
		if (tok[0] == 'v' && tok[1] == 'n')
		{
			continue;
		}

		// Face
		if (tok[0] == 'f' && tok[1] == '\0')
		{
			if (Model.mFaceCount >= MAX_TRIANGLES) continue;
			char* slash;

			tok = strtok(NULL, " ");
			if (tok)
			{
				g_Triangles[Model.mFaceCount].v1 = atoi(&tok[0]) - 1;

				if (slash = strchr(tok, '/'))
				{
					g_Triangles[Model.mFaceCount].tx1 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty1 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

			tok = strtok(NULL, " ");
			if (tok)
			{
				g_Triangles[Model.mFaceCount].v2 = atoi(&tok[0]) - 1;

				if (slash = strchr(tok, '/'))
				{
					g_Triangles[Model.mFaceCount].tx2 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty2 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

			tok = strtok(NULL, " ");
			if (tok)
			{
				g_Triangles[Model.mFaceCount].v3 = atoi(&tok[0]) - 1;

				if (slash = strchr(tok, '/'))
				{
					g_Triangles[Model.mFaceCount].tx3 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty3 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

			// -- Add-To-TreeView
			char name[256];
			sprintf(name, "Triangles #%u", (UINT)Model.mFaceCount);
			g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM(&g_Triangles[Model.mFaceCount])));
			Model.mFaceCount++;

			// -- Quad
			tok = strtok(NULL, " \r\n");
			if (tok != NULL)
			{
				TRIANGLE* prev = &g_Triangles[Model.mFaceCount - 1];

				g_Triangles[Model.mFaceCount].v1 = prev->v3;
				g_Triangles[Model.mFaceCount].v2 = atoi(&tok[0]) - 1;
				g_Triangles[Model.mFaceCount].v3 = prev->v1;

				if (slash = strchr(tok, '/'))
				{
					g_Triangles[Model.mFaceCount].tx1 = prev->tx3;
					g_Triangles[Model.mFaceCount].ty1 = prev->ty3;
					g_Triangles[Model.mFaceCount].tx2 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty2 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
					g_Triangles[Model.mFaceCount].tx3 = prev->tx1;
					g_Triangles[Model.mFaceCount].ty3 = prev->ty1;
				}

				// -- Add-To-TreeView
				char name[256];
				sprintf(name, "Triangles #%u", (UINT)Model.mFaceCount);
				g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM(&g_Triangles[Model.mFaceCount])));
				Model.mFaceCount++;
			}
		}
	}

	// Normals
	//Normals
	bool vlist[MAX_VERTICES];
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
	}

	fclose(fp);

	return;
}


void Load3DNData(char* fname)
{
	newScene();

	FILE* fp = fopen(fname, "rb");

	if (!fp)
	{
		MessageBox(g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION);
		return;
	}

	//--Header
	fread(&Model.mVertCount, 1, 4, fp);
	fread(&Model.mFaceCount, 1, 4, fp);
	fread(&Model.mBoneCount, 1, 4, fp);
	fread(Model.mName, 1, 32, fp);
	fseek(fp, 4, SEEK_CUR);
	//fread(&Model.mTextureSize,1,4,fp);
	Model.mTextureSize = 256 * 512;

	if (Model.mVertCount == 0)
	{
		MessageBox(g_hMain, "There are no vertices in this file!", "File Error", MB_ICONEXCLAMATION);
		return;
	}
	if (Model.mFaceCount == 0)
	{
		MessageBox(g_hMain, "There are no triangles in this file!", "File Error", MB_ICONEXCLAMATION);
		return;
	}


	//Vertices
	for (int v = 0; v < Model.mVertCount; v++)
	{
		fread(&g_Verticies[v], 16, 1, fp);

		char str[256];
		sprintf(str, "Vertex #%d", v + 1);
		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM(&g_Verticies[v])));
	}

	//Triangles
	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fread(&g_Triangles[t].v1, 4, 1, fp);
		fread(&g_Triangles[t].v2, 4, 1, fp);
		fread(&g_Triangles[t].v3, 4, 1, fp);

		fread(&g_Triangles[t].tx1, 2, 1, fp);
		fread(&g_Triangles[t].ty1, 2, 1, fp);
		fread(&g_Triangles[t].tx2, 2, 1, fp);
		fread(&g_Triangles[t].ty2, 2, 1, fp);
		fread(&g_Triangles[t].tx3, 2, 1, fp);
		fread(&g_Triangles[t].ty3, 2, 1, fp);

		fread(&g_Triangles[t].u1, 4, 1, fp);
		fread(&g_Triangles[t].flags, 4, 1, fp);
		fread(&g_Triangles[t].parent, 4, 1, fp);
		fread(&g_Triangles[t].u2, 4, 1, fp);
		fread(&g_Triangles[t].u3, 4, 1, fp);
		fread(&g_Triangles[t].u4, 4, 1, fp);
		fread(&g_Triangles[t].u5, 4, 1, fp);

		g_Triangles[t].ty1 = -g_Triangles[t].ty1;
		g_Triangles[t].ty2 = -g_Triangles[t].ty2;
		g_Triangles[t].ty3 = -g_Triangles[t].ty3;

		char str[256];
		sprintf(str, "Triangles #%d", t + 1);
		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM(&g_Triangles[t])));
	}

	//Normals
	bool vlist[MAX_VERTICES];
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
	}

	//Bones
	for (int b = 0; b < Model.mBoneCount; b++)
	{
		fread(&g_Bones[b], sizeof(BONE), 1, fp);

		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], g_Bones[b].name, LPARAM(&g_Bones[b])));
	}

	//Texture
	delete[] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char[Model.mTextureSize];
	memset(Model.mTexture, 255, Model.mTextureSize);
	//fread( Model.mTexture, Model.mTextureSize,1, fp);

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

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

	// -- Close the file
	fclose(fp);
}


void Load3DFData(char* fname)
{
	newScene();

	FILE* fp = fopen(fname, "rb");

	if (!fp)
	{
		MessageBox(g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION);
		return;
	}

	//--Header
	fread(&Model.mVertCount, 1, 4, fp);
	fread(&Model.mFaceCount, 1, 4, fp);
	fread(&Model.mBoneCount, 1, 4, fp);
	fread(&Model.mTextureSize, 1, 4, fp);

	if (Model.mVertCount == 0)
	{
		MessageBox(g_hMain, "There are no vertices in this file!", "File Error", MB_ICONEXCLAMATION);
		return;
	}
	if (Model.mFaceCount == 0)
	{
		MessageBox(g_hMain, "There are no triangles in this file!", "File Error", MB_ICONEXCLAMATION);
		return;
	}


	//Triangles
	for (int t = 0; t < Model.mFaceCount; t++)
	{
		fread(&g_Triangles[t], 64, 1, fp);

		char str[256];
		sprintf(str, "Triangles #%d", t + 1);
		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM(&g_Triangles[t])));
	}

	//Vertices
	for (int v = 0; v < Model.mVertCount; v++)
	{
		fread(&g_Verticies[v], 16, 1, fp);

		char str[256];
		sprintf(str, "Vertex #%d", v + 1);
		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM(&g_Verticies[v])));
	}

	//Normals
	bool vlist[MAX_VERTICES];
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
	}

	//Bones
	for (int b = 0; b < Model.mBoneCount; b++)
	{
		fread(&g_Bones[b], sizeof(BONE), 1, fp);

		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], g_Bones[b].name, LPARAM(&g_Bones[b])));
	}

	//Texture
	delete[] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char[Model.mTextureSize];
	fread(Model.mTexture, Model.mTextureSize, 1, fp);

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

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

	// -- Close the file
	fclose(fp);
}

void Save3DFData(char* fname)
{
	//Pre-processing:
	if (Model.mBoneCount == 0)
	{
		bool bone[32];
		ZeroMemory(&bone, sizeof(bool) * 32);
		for (int v = 0; v < Model.mVertCount; v++)
		{
			if (bone[g_Verticies[v].bone] == false)
			{
				bone[g_Verticies[v].bone] = true;
				sprintf(g_Bones[Model.mBoneCount].name, "Bone-%u", (UINT)Model.mBoneCount);
				g_Bones[Model.mBoneCount].x = 0;
				g_Bones[Model.mBoneCount].z = 0;
				g_Bones[Model.mBoneCount].y = Model.mBoneCount * 2.0f;
				g_Bones[Model.mBoneCount].parent = -1;
				g_Bones[Model.mBoneCount].hide = 0;
				Model.mBoneCount++;
			}
		}
	}
	//End Pre-processing

	FILE* fp = fopen(fname, "wb");

	//--Header
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
		fwrite(&g_Verticies[v], 16, 1, fp);
	}

	//Bones
	for (int b = 0; b < Model.mBoneCount; b++)
	{
		fwrite(g_Bones[b].name, 1, 32, fp);
		fwrite(&g_Bones[b].x, 1, 4, fp);
		fwrite(&g_Bones[b].y, 1, 4, fp);
		fwrite(&g_Bones[b].z, 1, 4, fp);
		fwrite(&g_Bones[b].parent, 1, 2, fp);
		fwrite(&g_Bones[b].hide, 1, 2, fp);
	}

	//Texture
	fwrite(Model.mTexture, Model.mTextureSize, 1, fp);

	//Close the file
	fclose(fp);
}

void SaveC2OData(char* fname)
{
	FILE* fp = fopen(fname, "wb");

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
}

void LoadCMFData(char* fname)
{

	FILE* fp = fopen(fname, "r");

	if (!fp)
	{
		MessageBox(g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION);
		return;
	}

	char line[512];

	while (!feof(fp))
	{
		fgets(line, 512, fp);

		if (line[0] == 0) break;
		if (line[0] == 4) break;

		char* tok = strtok(line, " \n");

		// -- File Types
		if (!strcmp(tok, "Type"))
		{
			newScene();
		}
		else if (!strcmp(tok, "Version"))
		{
			// -- Version check
			// Version == 1.0
		}
		else if (!strcmp(tok, "Name"))
		{
			// -- Version check
			// Version == 1.0
		}
	}

	//Normals
	bool vlist[MAX_VERTICES];
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
	}

	fclose(fp);

	//Texture
	Model.mTextureSize = 256 * 256 * 2;
	delete[] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char[Model.mTextureSize];
	for (int i = 0; i < Model.mTextureSize / 2; i++)
	{
		*((WORD*)Model.mTexture + i) = 0xFFFF;
	}

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

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
}

void SaveCMFData(char* fname)
{
	//Pre-processing:
	if (Model.mBoneCount == 0)
	{
		bool bone[32];
		ZeroMemory(&bone, sizeof(bool) * 32);
		for (int v = 0; v < Model.mVertCount; v++)
		{
			if (bone[g_Verticies[v].bone] == false)
			{
				bone[g_Verticies[v].bone] = true;
				sprintf(g_Bones[Model.mBoneCount].name, "Bone-%u", (UINT)Model.mBoneCount);
				g_Bones[Model.mBoneCount].x = 0;
				g_Bones[Model.mBoneCount].z = 0;
				g_Bones[Model.mBoneCount].y = Model.mBoneCount * 4.0f;
				g_Bones[Model.mBoneCount].parent = -1;
				g_Bones[Model.mBoneCount].hide = 0;
				Model.mBoneCount++;
			}
		}
	}
	//End Pre-processing

	FILE* fp = fopen(fname, "w");

	if (fp == NULL)
	{
		MessageBox(g_hMain, "Failed to open the desired file for writing!\r\nDo you have administrator access?", "C3Dit", MB_ICONWARNING);
		return;
	}

	//--Header ( ID, VersionMajor, VersionMinor, Name, VertexCount, FaceCount, BoneCount )
	fprintf(fp, "H,%u,%u,%s,%u,%u,%u\n", 1, 0, Model.mName, Model.mVertCount, Model.mFaceCount, Model.mBoneCount);

	// --Verts
	for (int v = 0; v < Model.mVertCount; v++)
	{
		fprintf(fp, "V,%f,%f,%f,%u\n",
			g_Verticies[v].x,
			g_Verticies[v].y,
			g_Verticies[v].z,
			g_Verticies[v].bone);
	}

	// --Faces
	for (int f = 0; f < Model.mFaceCount; f++)
	{
		fprintf(fp, "F,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
			g_Triangles[f].v1,
			g_Triangles[f].v2,
			g_Triangles[f].v3,
			g_Triangles[f].tx1,
			g_Triangles[f].tx2,
			g_Triangles[f].tx3,
			g_Triangles[f].ty1,
			g_Triangles[f].ty2,
			g_Triangles[f].ty3,
			g_Triangles[f].flags);
	}

	//-- Bones
	for (int b = 0; b < Model.mBoneCount; b++)
	{
		fprintf(fp, "B,%s,%f,%f,%f\n",
			g_Bones[b].name,
			g_Bones[b].x,
			g_Bones[b].y,
			g_Bones[b].z);
	}

	//Close the file
	fclose(fp);
}
