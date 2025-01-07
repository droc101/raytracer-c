import os
import subprocess
import tempfile
import sys
import util

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

def ConvertVulkanFrag(path):
	global aid

	data = list(glsl_to_spv(path))

	data += util.IntToBytes(len(data))  # array size (excluding header)
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(util.aid)  # Padding

	util.WriteAsset(path, "gfrg", "vkshader", util.EncloseData(data, 5))

def ConvertVulkanVert(path):
	global aid

	data = list(glsl_to_spv(path))

	data += util.IntToBytes(len(data))  # array size (excluding header)
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(util.aid)  # Padding
	
	util.WriteAsset(path, "gvrt", "vkshader", util.EncloseData(data, 6))