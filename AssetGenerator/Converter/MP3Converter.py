import util

def ConvertMP3(path):
	global aid

	file = open(path, "rb")
	data = list(file.read())

	data += util.IntToBytes(len(data))  # array size (excluding header)
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(0)  # unused
	data += util.IntToBytes(util.aid)  # Padding

	util.WriteAsset(path, "gmus", "audio", util.EncloseData(data, 1))