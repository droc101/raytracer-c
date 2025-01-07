import struct
import util

def ParseOBJ(file_path):
	verts = []
	uvs = []
	norms = []
	
	with open(file_path, 'r') as f:
		for line in f:
			if line.startswith('v '):
				verts.append(list(map(float, line.split()[1:])))
			elif line.startswith('vt '):
				uvs.append(list(map(float, line.split()[1:])))
			elif line.startswith('vn '):
				norms.append(list(map(float, line.split()[1:])))
	
	# Create a bytearray to store the vertex data as a list of doubles
	vertex_data = bytearray()
	for v in verts:
		vertex_data += struct.pack('3d', *v)
  
	# Create a bytearray to store the uv data as a list of doubles
	uv_data = bytearray()
	for uv in uvs:
		uv_data += struct.pack('2d', *uv)
  
	# Create a bytearray to store the normal data as a list of doubles
	normal_data = bytearray()
	for n in norms:
		normal_data += struct.pack('3d', *n)

	vert_data = []
	idx_val = 0
	# Loop through the file a second time to get the face data
	with open(file_path, 'r') as f:
		for line in f:
			if line.startswith('f '):
				# Get the vertex and uv indices for each face
				face = line.split()[1:]
	
				for v in face:
					# Add the vertex and uv data into the face data array as X Y Z U V (double)
					v_idx, vt_idx, vn_idx = v.split('/')
					vertex = verts[int(v_idx) - 1]
					uv = uvs[int(vt_idx) - 1]
					norm = norms[int(vn_idx) - 1]
					vert_data.extend(vertex)
					vert_data.extend(uv)
					vert_data.extend(norm)

					idx_val += 1
	
	# Pack the vertex and index data into a bytearray with the format: X Y Z U V (double)
	vtx_bin_data = bytearray()
	for v in vert_data:
		vtx_bin_data += struct.pack('f', v)
	
  
	# Pack the vertex and index data into a bytearray with the format:
	# signature (4 bytes, "MESH")
 	# vertex count
	# separator (4 bytes, "DATA")
	# vertex data
	# index data
 
	bin_data = bytearray()
	bin_data += struct.pack('4s', b'MSH\0')
	bin_data.extend(util.IntToBytesLE(idx_val))
	bin_data += struct.pack('4s', b'DAT\0')
	bin_data += vtx_bin_data
 
	return bin_data

def ConvertOBJ(path):
	data = list(ParseOBJ(path))

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)

	util.WriteAsset(path, "gmdl", "model", util.EncloseData(data, 7))