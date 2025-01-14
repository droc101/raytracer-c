import util

def ConvertFile(path, type, extension, subfolder):
	file = open(path, "rb")
	data = list(file.read())

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)

	util.WriteAsset(path, extension, subfolder, util.EncloseData(data, type))