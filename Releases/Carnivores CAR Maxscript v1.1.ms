--Written by: RexHunter99 2020
--Version 1.1


utility CarnivoresExporter "Carnivores MAXScript"
(
	group "Import"
	(
		button ClickCARImport "Import CAR" toolTip: "Imports just the model data"
		button Click3DFImport "Import 3DF" toolTip: "Import the model"
	)
	group "Export"
	(
		button Click3DFExport "Export 3DF" toolTip: "Export the model"
		button ClickVTLExport "Export VTL" toolTip: "Export the animation"
	)
	group "About"
	(
	  	label lab1 "Carnivores MAXScript"
	  	label lab2 "Version: v20.1"
	  	label lab3 "License: MIT 2020"
	  	label lab4 "Author: Rexhunter99"
	)
	
	fn LoadCARFile fname =
	(
		--Define some variables
		cname = "NULL"
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
		tex_size = 256*256*2
		
		--Open the file
		f = fopen fname "rb"
		
		--Read the header
		cname = ReadString f
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
			flags = ReadShort f			--Flags
			dmask = ReadShort f
			prev = ReadLong f           --Previous Face
			next = ReadLong f			--Next Face
			group = ReadLong f
			fseek f 4 #seek_cur			--Reserved
			fseek f 4 #seek_cur			--Reserved
			fseek f 4 #seek_cur			--Reserved
			
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
		
		mat = multiMaterial numsubs:1 name:fname
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
	
	fn Load3DFFile fname =
	(
		--Define some variables
		cname = "NULL"
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
		tex_size = 256*256*2
		
		--Open the file
		f = fopen fname "rb"
		
		--Read the header
		cname = ReadString f
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
			flags = ReadShort f			--Flags
			dmask = ReadShort f
			prev = ReadLong f           --Previous Face
			next = ReadLong f			--Next Face
			group = ReadLong f
			fseek f 4 #seek_cur			--Reserved
			fseek f 4 #seek_cur			--Reserved
			fseek f 4 #seek_cur			--Reserved
			
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
		
		mat = multiMaterial numsubs:1 name:fname
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
	
	fn Save3DFFile fname =
	(
		-- Exports the selected mesh if there is one, if there is no selected mesh it exports
		-- the first mesh in the mesh list.
		-- Any meshes with '#' at the start of their name are treated as 'bones'
		
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
			WriteShort fp -1 -- No bones currently
			WriteShort fp 0 -- Not hidden
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
	
	fn SaveVTLFile fname =
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
				WriteShort fp ((-v.x) as integer)
				WriteShort fp ((v.z) as integer)
				WriteShort fp ((v.y) as integer)
			)
			sliderTime+=1f
		)
		sliderTime = animationRange.start
		
		fclose fp
	)
	
	
	on ClickCARImport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getOpenFileName "Open a CAR file" \
		types:"CAR (.car)|*.car|"
		
  		LoadCARFile fname
  	)
	
	on Click3DFImport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getOpenFileName "Open a 3DF file" \
		types:"3DF (.3df)|*.3df|"
		
  		Load3DFFile fname
  	)
	
	on Click3DFExport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getSaveFileName "Save a 3DF file" \
		types:"3D File (.3DF)|*.3DF|"
		
  		Save3DFFile fname
  	)
	
	on ClickVTLExport pressed do
 	(
  		clearListener()
  		
		--Find a file
		fname =  getSaveFileName "Save a VTL file" \
		types:"Vertex Transformation List (.VTL)|*.VTL|"
		
  		SaveVTLFile fname
  	)
)