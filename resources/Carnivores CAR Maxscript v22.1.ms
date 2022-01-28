--(c) Rexhunter99 2022
-- Version: 22.1


utility GeomExp "Carnivores MAXScript"
(
	group "Import"
	(
		button carImport "Import CAR" toolTip: "Import a CAR to the scene"
		button x3dfImport "Import 3DF" toolTip: "Import a 3DF to the scene"
	)
	group "Export"
	(
		button x3dfExport "Export 3DF" toolTip: "Export the entire scene as a 3DF"
		button vtlExport "Export VTL" toolTip: "Export the animated scene as a VTL"
	)
	group "About"
	(
	  	label lab1 "Carnivores MAXScript"
		label lab2 "Version: 22.1"
	  	label lab3 "License: MIT"
	  	label lab4 "Author: Rexhunter99, 2022"
	)
	
	fn ReadCAR fname =
	(
		--Define some variables
		cname = fname
		msc = "msc: 0"
		num_tris = 0
		num_verts = 0
		num_anims = 0
		num_sounds = 0
		num_bones = 0
		tri_list = #()
		vert_list = #()
		tex_list = #()
		tc_x = #()
		tc_y = #()
		tex_size = 256 * 512
		
		--Open the file
		f = fopen fname "rb"
		
		--Read the header
		cname = ReadString f
		fseek f 24 #seek_set
		msc = ReadString f
		fseek f 32 #seek_set
		num_anims = ReadLong f #signed
		num_sounds = ReadLong f #signed
		num_verts = ReadLong f #signed
		num_tris = ReadLong f #signed
		tex_size = ReadLong f #signed
		
		for t=1 to num_tris do (
			v1 = ReadLong f				--Vert 1
			v2 = ReadLong f				--Vert 2
			v3 = ReadLong f				--Vert 3
			tx1 = ReadLong f			--U1
			tx2 = ReadLong f			--U2
			tx3 = ReadLong f			--U3
			ty1 = ReadLong f			--V1
			ty2 = ReadLong f			--V2
			ty3 = ReadLong f			--V3
			flags = ReadLong f			--Flags
			fseek f 4 #seek_cur
			par = ReadLong f			--Parent
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			append tri_list ([v1+1,v2+1,v3+1])
			append tc_x ((tx1/256.0) as float)
			append tc_x ((tx2/256.0) as float)
			append tc_x ((tx3/256.0) as float)
			append tc_y ((ty1/256.0) as float)
			append tc_y ((ty2/256.0) as float)
			append tc_y ((ty3/256.0) as float)
		)
		
		for v=1 to num_verts do (
			x = ReadFloat f				--X
			y = ReadFloat f				--Z
			z = ReadFloat f				--Y
			b = ReadShort f				--Bone
			h = ReadShort f				--Hide
			
			if b>num_bones then
			num_bones = b
			
			append vert_list ([-x,z,y])
		)
		
		fclose f
		
		mat = multiMaterial numsubs:1 name:cname
		m = mesh name:cname position:[0, 0, 0] scale:[1, 1, 1] \
		faces:tri_list vertices:vert_list material:mat
		
		setNumTVerts m (num_tris * 3)
		for f = 1 to (num_tris * 3) do 
		(
			setTVert m (f+0) tc_x[f+0] tc_y[f+0] 0
		)
			
		-- Create our texture vertice faces
		buildTVFaces m
		for f = 1 to (num_tris-1) do
		(
			v1 = ((f - 1) * 3) + 1
			v2 = ((f - 1) * 3) + 2
			v3 = ((f - 1) * 3) + 3
			setTVFace m f [v1,v2,v3]
		)
	)
	
	fn Read3DF fname =
	(
		--Define some variables
		cname = fname
		-- TODO: extract the model file name from fname
		num_tris = 0
		num_verts = 0
		num_bones = 0
		tri_list = #()
		vert_list = #()
		bone_list = #()
		tex_list = #()
		tc_x = #()
		tc_y = #()
		tex_size = 256 * 512
		
		--Open the file
		f = fopen fname "rb"
		
		--Read the header
		num_verts = ReadLong f #signed
		num_tris = ReadLong f #signed
		num_bones = ReadLong f #signed
		tex_size = ReadLong f #signed
		
		for t=1 to num_tris do (
			v1 = ReadLong f				--Vert 1
			v2 = ReadLong f				--Vert 2
			v3 = ReadLong f				--Vert 3
			tx1 = ReadLong f			--U1
			tx2 = ReadLong f			--U2
			tx3 = ReadLong f			--U3
			ty1 = ReadLong f			--V1
			ty2 = ReadLong f			--V2
			ty3 = ReadLong f			--V3
			flags = ReadLong f			--Flags
			fseek f 4 #seek_cur
			par = ReadLong f			--Parent
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			fseek f 4 #seek_cur
			append tri_list ([v1+1,v2+1,v3+1])
			append tc_x ((tx1 / 256.0) as float)
			append tc_x ((tx2 / 256.0) as float)
			append tc_x ((tx3 / 256.0) as float)
			append tc_y ((ty1 / 256.0) as float)
			append tc_y ((ty2 / 256.0) as float)
			append tc_y ((ty3 / 256.0) as float)
		)
		
		for v=1 to num_verts do (
			x = ReadFloat f				--X
			y = ReadFloat f				--Z
			z = ReadFloat f				--Y
			b = ReadShort f				--Bone
			h = ReadShort f				--Hide
			
			if b > num_bones then
			num_bones = b
			
			append vert_list ([-x,z,y])
		)
		
		for i=1 to num_bones do
		(
			fpos = ftell f
			fpos = fpos + 32
			
			bone_name = ReadString f
			
			fseek f fpos #seek_set
			
			bone_x = ReadFloat f
			bone_y = ReadFloat f
			bone_z = ReadFloat f
			bone_parent = ReadShort f -1
			bone_hide = ReadShort f 0
			
			-- TODO: Create objects for the bones.
		)
		
		fclose f
		
		mat = multiMaterial numsubs:1 name:cname
		m = mesh name:cname position:[0, 0, 0] scale:[1, 1, 1] faces:tri_list vertices:vert_list material:mat
		
		setNumTVerts m (num_tris * 3)
		for f = 1 to (num_tris * 3) do 
		(
			setTVert m (f+0) tc_x[f+0] tc_y[f+0] 0
		)
			
		-- Create our texture vertice faces
		buildTVFaces m
		for f = 1 to (num_tris-1) do
		(
			v1 = ((f - 1) * 3) + 1
			v2 = ((f - 1) * 3) + 2
			v3 = ((f - 1) * 3) + 3
			setTVFace m f [v1,v2,v3]
		)
	)
	
	fn Save3DF fname =
	(
		if fname == undefined then exit
			
		obj = 0
		obones = #()
		numbones = 0
		
		format "$objects.count = %\n" $objects.count
		
		for o=1 to $objects.count do
		(
			if $objects[o].name[1]=="#" then
			(
				append obones $objects[o]
				numbones += 1
				format "  % %\n" $objects[o].name $objects[o].parent
			)
			else
				obj = $objects[o]
		)
		
		
		if $selection[1] == undefined then
		(
			obj = $objects[1]
		)
		else
		(
			obj = $selection[1]
		)
		
		format "BoneCount = %\n" numbones
		numbones = 0
		
		fp = fopen fname "wb"
		
		WriteLong fp obj.numverts #unsigned
		WriteLong fp obj.numfaces #unsigned
		WriteLong fp numbones #unsigned
		WriteLong fp (256*512) #unsigned
		
		for i=1 to obj.numfaces do
		(
			f = getface obj i
			t = gettvface obj i
			tx = gettvert obj t.x
			ty = gettvert obj t.y
			tz = gettvert obj t.z
			WriteLong fp (f.z-1) #unsigned
			WriteLong fp (f.y-1) #unsigned
			WriteLong fp (f.x-1) #unsigned
			WriteLong fp (tz.x * 256) #unsigned
			WriteLong fp (ty.x * 256) #unsigned
			WriteLong fp (tx.x * 256) #unsigned
			WriteLong fp (tz.y * 256) #unsigned
			WriteLong fp (ty.y * 256) #unsigned
			WriteLong fp (tx.y * 256) #unsigned
			WriteLong fp 0
			WriteLong fp 0
			WriteLong fp -1
			WriteLong fp 0
			WriteLong fp 0
			WriteLong fp 0
			WriteLong fp 0
		)
		
		for i=1 to obj.numverts do
		(
			v = getvert obj i
			WriteFloat fp (-v.x)
			WriteFloat fp v.z
			WriteFloat fp v.y
			--Get the bone if the mesh is Skinned
			WriteLong fp 0
		)
		
		for i=1 to numbones do
		(
			for c=1 to 32 do
			(
				if c >= obones[i].name.length then
					WriteByte fp 0
				else
					WriteByte fp obones[i].name[c]
			)
			WriteFloat fp obones[i].x
			WriteFloat fp obones[i].y
			WriteFloat fp obones[i].z
			WriteShort fp -1
			WriteShort fp 0
		)
		
		texfp = fopen obj.material.diffuseMap.fileName "rb"
		fseek texfp 18 #seek_set
		
		for i=1 to 256*256 do
		(
			pixel = ReadShort texfp #unsigned
			WriteShort fp pixel #unsigned
		)
		
		fclose texfp
		
		fclose fp
	)
	
	fn SaveVTL fname =
	(
		if $selection[1] == undefined then
		(
			obj = $objects[1]
		)
		else
		(
			obj = $selection[1]
		)
		
		if fname == undefined then exit
			
		num_frames = ( 1 + animationRange.end - animationRange.start )
		
		fp = fopen fname "wb"
		
		WriteLong fp obj.numverts #unsigned
		WriteLong fp 20 #unsigned
		WriteLong fp num_frames #unsigned
		
		--Animation
		sliderTime = animationRange.start
		for i=animationRange.start to animationRange.end do
		(
			for j=1 to obj.numverts do
			(
				v = getvert obj j
				WriteShort fp ((-v.x*16) as integer)
				WriteShort fp ((v.z*16) as integer)
				WriteShort fp ((v.y*16) as integer)
			)
			sliderTime+=1f
		)
		sliderTime = animationRange.start
		
		fclose fp
	)
	
	on carImport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getOpenFileName "Open a CAR file" types:"CAR (.car)|*.car|"
		
  		ReadCAR fname
  	)
	
	on x3dfImport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getOpenFileName "Open a 3DF file" types:"3DF (.3DF)|*.3DF|"
		
  		Read3DF fname
  	)
	
	on x3dfExport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getSaveFileName "Save a 3DF file" types:"3D File (.3DF)|*.3DF|"
		
  		Save3DF fname
  	)
	
	on vtlExport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getSaveFileName "Save a VTL file" \
		types:"Vertex Transformation List (.VTL)|*.VTL|"
		
  		SaveVTL fname
  	)
)