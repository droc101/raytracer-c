import os
import sys
import struct
import shutil
import json
import util

# Converters
import Converter.TextureConverter
import Converter.FileConverter
import Converter.MP3Converter
import Converter.OBJConverter
import Converter.VulkanShaderConverter
import Converter.WAVConverter

if (len(sys.argv) != 3):
	print("Usage: python genassets.py <input_path> <output_path>")
	sys.exit(1)

util.input_path = sys.argv[1]
util.output_path = sys.argv[2]

TSIZETABLE_NAME_LENGTH = 32
LDATATABLE_STR_LENGTH = 32

# Recreate the assets folder and subfolders
def SetupDirs(out_path):
	# Delete the old assets folder
	if os.path.exists(out_path):
		shutil.rmtree(out_path)
	
	os.makedirs(out_path)
	os.makedirs(out_path + "texture/")
	os.makedirs(out_path + "audio/")
	os.makedirs(out_path + "level/")
	os.makedirs(out_path + "glshader/")
	os.makedirs(out_path + "vkshader/")
	os.makedirs(out_path + "model/")

# Build the texture size table based on the converted assets
def BuildTextureSizeTable():
	tsizetable = bytearray()
	tsizetable.extend(struct.pack('I', Converter.TextureConverter.texture_asset_count))
	tsizetable.extend(struct.pack('I', util.aid))

	for name in Converter.TextureConverter.texture_asset_names:
		try:
			name = util.CString(name, TSIZETABLE_NAME_LENGTH)
		except ValueError:
			print("Error: Texture asset name is too long: " + name)
			sys.exit(1)
		tsizetable.extend(name.encode('utf-8'))
	
	util.Write("", "tsizetable.gtsb", tsizetable)

# Recursively search for files in the input folder and convert them
def RecursiveSearch(in_path, out_path):
	files = os.listdir(in_path)
	
	# Sort the files so that the order is deterministic
	files.sort()
	
	for file in files:
		if os.path.isdir(in_path + file):
			RecursiveSearch(in_path + file + "/", out_path)
		else:
			path_from_assets = in_path.split("Assets/")[1]
			if file.endswith("ldatatable.json"):
				pass # We'll handle this later
			elif file.endswith(".png"):
				print("Converting " + path_from_assets + file)
				Converter.TextureConverter.ConvertPNG(in_path + file)
			elif file.endswith(".mp3"):
				print("Converting " + path_from_assets + file)
				Converter.MP3Converter.ConvertMP3(in_path + file)
			elif file.endswith(".wav"):
				print("Converting " + path_from_assets + file)
				Converter.WAVConverter.ConvertWAV(in_path + file)
			elif file.endswith(".bin"):
				print("Converting " + path_from_assets + file)
				Converter.FileConverter.ConvertFile(in_path + file, 4, "gmap", "level")
			elif file.endswith(".frag"):
				print("Converting " + path_from_assets + file)
				Converter.VulkanShaderConverter.ConvertVulkanFrag(in_path + file)
			elif file.endswith(".vert"):
				print("Converting " + path_from_assets + file)
				Converter.VulkanShaderConverter.ConvertVulkanVert(in_path + file)
			elif file.endswith(".obj"):
				print("Converting " + path_from_assets + file)
				Converter.OBJConverter.ConvertOBJ(in_path + file)
			elif file.endswith(".glsl"):
				print("Converting " + path_from_assets + file)
				Converter.FileConverter.ConvertFile(in_path + file, 4, "gshd", "glshader")
			else:
				print("Unrecognized file type: " + file)

SetupDirs(util.output_path)
RecursiveSearch(util.input_path, util.output_path)
BuildTextureSizeTable()
