import util

def ConvertGLShader(path):
	file = open(path, "rb")
	data = list(file.read())
	data += [0] # null terminator, do NOT remove

	util.WriteAsset(path, "gshd", "glshader", util.EncloseData(data, 4))