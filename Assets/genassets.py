import os
import sys
import subprocess
from PIL import Image
import gzip
import tempfile
import struct
import shutil

if (len(sys.argv) != 3):
	print("Usage: python genassets.py <input_path> <output_path>")
	sys.exit(1)

TSIZETABLE_NAME_LENGTH = 32

input_path = sys.argv[1]
output_path = sys.argv[2]

aid = 0
count = 0

texture_asset_count = 0
texture_asset_total_size = 0
texture_asset_names = []

#region Helpers

def int_to_bytes(i):  # Convert an integer to bytes big endian, 4 bytes
	return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]

def int_to_bytes_le(i):  # Convert an integer to bytes little endian, 4 bytes
	return [i & 0xFF, (i >> 8) & 0xFF, (i >> 16) & 0xFF, (i >> 24) & 0xFF]

def write(out_path, subfolder, name, data):
    with open(out_path + subfolder + name, "wb") as f:
        f.write(data)

#endregion

#region Conversion Functions

def png_to_bytes(path):
	global aid
	global texture_asset_total_size
	img = Image.open(path)
	img = img.convert("RGBA")
	img_dta = img.getdata()
	data = []

	data += int_to_bytes(img.width * img.height)  # array size (excluding header)
	data += int_to_bytes(img.width)  # width
	data += int_to_bytes(img.height)  # height
	data += int_to_bytes(aid)  # Padding

	for pixel in img_dta:
		data.append(pixel[0])
		data.append(pixel[1])
		data.append(pixel[2])
		data.append(pixel[3])

	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: Pixel data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(0))  # Asset Type (0 = Texture)

	header.extend(data)

	# gzip timestamp is 18 bytes in
	# we overwrite this in order to get deterministic output
	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4
	
	texture_asset_total_size += img.width * img.height * 4

	aid += 1

	return header

# Convert an MP3 file to bytes
def mp3_to_bytes(path):
	global aid

	file = open(path, "rb")
	data = list(file.read())

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding

	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: MP3 data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(1))  # Asset Type (1 = mp3)

	header.extend(data)

	aid += 1
	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	return header

# Convert an WAV file to bytes
def wav_to_bytes(path):
	global aid

	file = open(path, "rb")
	data = list(file.read())

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding
	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: WAV data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(2))  # Asset Type (2 = wav)

	header.extend(data)
	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

# Convert a file to bytes (raw)
def file_to_bytes(path, type):
	global aid

	file = open(path, "rb")
	data = list(file.read())

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding
	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: BIN data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(type))  # Asset Type

	header.extend(data)

	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

def glsl_to_spv(glsl_path):
	tmpf = tempfile.mktemp(suffix=".spv")
	# check if windows or linux
	glslc = "glslc"
	if os.name == 'nt':
		glslc = "C:\\VulkanSDK\\1.3.283.0\\Bin\\glslc.exe"
	args = [glslc, glsl_path, "-o", tmpf]

	try:
		subprocess.check_output(args)
	except subprocess.CalledProcessError as e:
		print(e.output)
		sys.exit(1)

	with open(tmpf, "rb") as f:
		data = f.read()

	os.remove(tmpf)

	return data

def frag_to_bytes(path):
	global aid

	data = list(glsl_to_spv(path))

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding
	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: FRAG-SPV data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(5))  # Asset Type (5 = fragment)

	header.extend(data)

	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

def vert_to_bytes(path):
	global aid

	data = list(glsl_to_spv(path))

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding
	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: FRAG-SPV data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(6))  # Asset Type (6 = vertex)

	header.extend(data)

	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

def parse_obj_file(file_path):
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
	bin_data.extend(int_to_bytes_le(idx_val))
	bin_data += struct.pack('4s', b'DAT\0')
	bin_data += vtx_bin_data
 
	return bin_data

def obj_to_bytes(path):
	global aid

	data = list(parse_obj_file(path))

	data += int_to_bytes(len(data))  # array size (excluding header)
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(0)  # unused
	data += int_to_bytes(aid)  # Padding
	for i in range(0, len(data)):
		if data[i] < 0 or data[i] > 255:
			print("Error: BIN data out of range")
			sys.exit(1)

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(int_to_bytes(len(data)))  # Compressed length
	header.extend(int_to_bytes(decompressed_len))  # Decompressed length
	header.extend(int_to_bytes(aid))  # Asset ID
	header.extend(int_to_bytes(7))  # Asset Type (7 = obj)

	header.extend(data)

	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

#endregion


def setup_dirs(out_path):
	# Delete the old assets folder
	if os.path.exists(out_path):
		shutil.rmtree(out_path)
	
    # Create the new assets folder
	os.makedirs(out_path)
	
    # Create the subfolders
	os.makedirs(out_path + "texture/")
	os.makedirs(out_path + "audio/")
	os.makedirs(out_path + "level/")
	os.makedirs(out_path + "glshader/")
	os.makedirs(out_path + "vkshader/")
	os.makedirs(out_path + "model/")

def build_tsizetable():
	global texture_asset_names
	global texture_asset_count
	global texture_asset_total_size
	global count

	tsizetable = bytearray()
	tsizetable.extend(struct.pack('I', texture_asset_count))
	tsizetable.extend(struct.pack('I', texture_asset_total_size))
	tsizetable.extend(struct.pack('I', count))

	for name in texture_asset_names:
		if len(name) > TSIZETABLE_NAME_LENGTH - 1:
			print("Error: Texture asset name is too long: " + name)
			sys.exit(1)
		name = name.ljust(TSIZETABLE_NAME_LENGTH, '\0') # pad with null bytes
		tsizetable.extend(name.encode('utf-8'))
	
	write(output_path, "", "tsizetable.gtsb", tsizetable)


def recursive_search(in_path, out_path):
	global count
	global texture_asset_names
	global texture_asset_count

	foldername = in_path.split("/")[-2]
	files = os.listdir(in_path)
	
	# Sort the files so that the order is deterministic
	files.sort()
	
	for file in files:
		if os.path.isdir(in_path + file):
			recursive_search(in_path + file + "/", out_path)
		else:
			path_from_assets = in_path.split("Assets/")[1]
			if file.endswith(".c") or file.endswith(".h") or file.endswith(".py"):
				pass
			elif file.endswith(".png"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = png_to_bytes(in_path + file)
				name = foldername + "_" + file.split(".")[0] + ".gtex"
				texture_asset_names.append(foldername + "_" + file.split(".")[0]) # name without extension
				texture_asset_count += 1
				write(out_path, "texture/", name, data)
			elif file.endswith(".mp3"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = mp3_to_bytes(in_path + file)
				write(out_path, "audio/", foldername + "_" + file.split(".")[0] + ".gmus", data)
			elif file.endswith(".wav"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = wav_to_bytes(in_path + file)
				write(out_path, "audio/", foldername + "_" + file.split(".")[0] + ".gsnd", data)
			elif file.endswith(".bin"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = file_to_bytes(in_path + file, 4)
				write(out_path, "level/", foldername + "_" + file.split(".")[0] + ".gmap", data)
			elif file.endswith(".frag"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = frag_to_bytes(in_path + file)
				write(out_path, "vkshader/", foldername + "_" + file.split(".")[0] + ".gfrg", data)
			elif file.endswith(".vert"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = vert_to_bytes(in_path + file)
				write(out_path, "vkshader/", foldername + "_" + file.split(".")[0] + ".gvrt", data)
			elif file.endswith(".obj"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = obj_to_bytes(in_path + file)
				write(out_path, "model/", foldername + "_" + file.split(".")[0] + ".gmdl", data)
			elif file.endswith(".glsl"):
				count += 1
				print("Converting " + path_from_assets + file)
				data = file_to_bytes(in_path + file, 4)
				write(out_path, "glshader/", foldername + "_" + file.split(".")[0] + ".gshd", data)
			else:
				print("Unrecognized file type: " + file)

setup_dirs(output_path)
recursive_search(input_path, output_path)
build_tsizetable()
