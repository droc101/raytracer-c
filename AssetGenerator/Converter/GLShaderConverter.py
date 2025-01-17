import util

def ConvertGLShader(path):
	file = open(path, "rb")
	data = list(file.read())
	data += [0]

	# data += util.IntToBytes(len(data))
	# data += util.IntToBytes(0)
	# data += util.IntToBytes(0)
	# data += util.IntToBytes(util.aid)

	util.WriteAsset(path, "gshd", "glshader", util.EncloseData(data, 4))