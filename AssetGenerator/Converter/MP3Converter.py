import util

def ConvertMP3(path):
	file = open(path, "rb")
	data = list(file.read())

	data += util.IntToBytes(len(data))
	data += util.IntToBytes(0)
	data += util.IntToBytes(0)
	data += util.IntToBytes(util.aid)

	util.WriteAsset(path, "gmus", "audio", util.EncloseData(data, 1))