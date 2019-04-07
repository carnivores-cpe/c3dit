
#ifndef MODEL_H
#define MODEL_H


class VERTEX
{
public:

    float mX, mY, mZ;
    short mBone;
	short mShow;
};

class NORM
{
public:
    
	float n[3];

    inline NORM& operator+= (const NORM& norm)
    {
        for (int i=0; i<3; i++)
        n[i] += norm.n[i];
        return *this;
    }
    inline NORM& operator/= (float scalar)
    {
        for (int i=0; i<3; i++)
        n[i] /= scalar;
        return *this;
    }
};

class TRIANGLE
{
public:

    //--Triangle
    int v1,v2,v3;
    int tx1,tx2,tx3,ty1,ty2,ty3;
    int flags,u1;
    int parent;
    // Triangle data? (from 3D Designer)
    int u2,u3,u4,u5;
};

class BONE
{
public:

    char    name[32];
    float   x;
    float   y;
    float   z;
    short   parent;
    short   unknown;
};

class TVtl
{
public:

    //--Animation
    char    Name[32];
    int     KPS;
    int     FrameCount;
    short*  Data;

    TVtl(){ Data = 0; }
    ~TVtl(){ if (Data) delete [] Data; }
};

class SOUND
{
public:

    char Name[32];
    int len;
    BYTE *data;

    SOUND() { data = 0; }
    ~SOUND() { if ( data ) delete [] data; }
};

class OBJ
{
public:

	int  Radius;
    int  YLo, YHi;
    int  linelenght, lintensity;
    int  circlerad, cintensity;
    int  flags;
    int  GrRad;
    int  DefLight;
    int  LastAniTime;
    float BoundR;
    unsigned char res[16];
};


class MODEL_HEADER
{
public:

	MODEL_HEADER(){}
	~MODEL_HEADER(){}

    //--Character header
    OBJ     oInfo;
    char    name[24];
    char    msc[8];
    long    num_anims;
    long    num_sounds;
    long    num_verts;
    long    num_tris;
    long    num_bones;
    long    bytes_tex;

	VERTEX		mVertices[MAX_VERTICES];
	TRIANGLE	mFaces[MAX_TRIANGLES];
	BONE		mBones[32];

    int     AnimFX[64];
};


#endif
