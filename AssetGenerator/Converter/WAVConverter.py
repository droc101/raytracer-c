import util

def ConvertWAV(path):
	file = open(path, "rb")
	data = list(file.read())

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)

	util.WriteAsset(path, "gsnd", "audio", util.EncloseData(data, 2))