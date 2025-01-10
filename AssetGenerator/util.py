import gzip

aid = 0

input_path = ""
output_path = ""

# Convert an integer to bytes big endian, 4 bytes
def IntToBytes(i):
	return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]

# Convert an integer to bytes little endian, 4 bytes
def IntToBytesLE(i):
	return [i & 0xFF, (i >> 8) & 0xFF, (i >> 16) & 0xFF, (i >> 24) & 0xFF]

# Write to a file in the output folder
def Write(subfolder, name, data):
	global output_path
	with open(output_path + subfolder + name, "wb") as f:
		f.write(data)

# Create a C-style string with a maximum length
def CString(s, maxlen):
	if (len(s) > maxlen - 1):
		raise ValueError("String is too long")
	return s.ljust(maxlen, '\0')

# Pack data into a gzipped asset
def EncloseData(data, type):
	global aid

	decompressed_len = len(data)

	data = gzip.compress(bytes(data))

	header = bytearray()
	header.extend(IntToBytes(len(data)))  # Compressed length
	header.extend(IntToBytes(decompressed_len))  # Decompressed length
	header.extend(IntToBytes(aid))  # Asset ID
	header.extend(IntToBytes(type))  # Asset Type

	header.extend(data)
	header[19] = 1
	header[20] = 2
	header[21] = 3
	header[22] = 4

	aid += 1

	return header

# Get the loadable asset name
def GetAssetName(in_path):
	in_folder_name = in_path.split("/")[-2]
	in_file_name = in_path.split("/")[-1].split(".")[0]
	return in_folder_name + "_" + in_file_name

# Write an asset to the output folder
def WriteAsset(in_path, extension, subfolder, data):
	in_folder_name = in_path.split("/")[-2]
	in_file_name = in_path.split("/")[-1].split(".")[0]
	
	name = in_folder_name + "_" + in_file_name + "." + extension
	
	Write(subfolder + "/", name, data)