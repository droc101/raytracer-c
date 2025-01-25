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
		glslc = "glslc.exe"
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
	data = list(glsl_to_spv(path))

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)

	util.WriteAsset(path, "gfrg", "vkshader", util.EncloseData(data, 5))

def ConvertVulkanVert(path):
	data = list(glsl_to_spv(path))

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)
	
	util.WriteAsset(path, "gvrt", "vkshader", util.EncloseData(data, 6))