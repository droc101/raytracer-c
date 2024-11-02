import os
import sys
from PIL import Image
import gzip

aid = 0


def int_to_bytes(i):  # Convert an integer to bytes big endian, 4 bytes
    return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]

# Convert a PNG file to bytes
def png_to_bytes(path):
    global aid
    img = Image.open(path)
    img = img.convert("RGBA")
    img_dta = img.getdata()
    data = []

    data += int_to_bytes(img.width * img.height)  # array size (excluding header)
    data += int_to_bytes(img.width)  # width
    data += int_to_bytes(img.height)  # height
    data += int_to_bytes(aid)  # Padding

    for pixel in img_dta:
        data.append(pixel[0])
        data.append(pixel[1])
        data.append(pixel[2])
        data.append(pixel[3])

    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: Pixel data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(0))  # Asset Type (0 = Texture)

    header.extend(data)

    # gzip timestamp is 18 bytes in
    # we overwrite this in order to get deterministic output
    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

# Convert an MP3 file to bytes
def mp3_to_bytes(path):
    global aid

    file = open(path, "rb")
    data = list(file.read())

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding

    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: MP3 data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(1))  # Asset Type (1 = mp3)

    header.extend(data)

    aid += 1
    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    return header

# Convert an WAV file to bytes
def wav_to_bytes(path):
    global aid

    file = open(path, "rb")
    data = list(file.read())

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: WAV data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(2))  # Asset Type (2 = wav)

    header.extend(data)
    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

# Convert a file to bytes (raw)
def file_to_bytes(path, type):
    global aid

    file = open(path, "rb")
    data = list(file.read())

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: BIN data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(type))  # Asset Type (3 = bin)

    header.extend(data)

    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

# Convert the bytes to a C array (for the .c file)
def bytes_to_c_array(data, name):
    output = "const unsigned char " + name + "[] = {\n"
    for i in range(0, len(data)):
        output += "0x{:02x}".format(data[i]) + ", "
        if i % 16 == 15:
            output += "\n"
    output += "};\n"
    return output

# Generate the header for the array (the actual array is in the .c file)
def c_header_array(name, size):
    return "extern const unsigned char " + name + "[];\n"


path = sys.argv[1]

assets_c = ""
assets_h = ""

count = 0


def recursive_search(path):
    global count
    global assets_c
    global assets_h
    assets_c += "\n"
    assets_h += "\n"

    foldername = path.split("/")[-2]
    files = os.listdir(path)
    
    # Sort the files so that the order is deterministic
    files.sort()
    
    for file in files:
        if os.path.isdir(path + file):
            recursive_search(path + file + "/")
        else:
            path_from_assets = path.split("Assets/")[1]
            if file.endswith(".c") or file.endswith(".h") or file.endswith(".py"):
                pass
            elif file.endswith(".png"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = png_to_bytes(path + file)
                name = "gztex_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".mp3"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = mp3_to_bytes(path + file)
                name = "gzmpg_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".wav"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = wav_to_bytes(path + file)
                name = "gzwav_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".bin"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = file_to_bytes(path + file, 3)
                name = "gzbin_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".glsl"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = file_to_bytes(path + file, 4)
                name = "gzshd_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            else:
                print("Unrecognized file type: " + file)


recursive_search(path)

print("Converted " + str(count) + " files")

assets_c_header = '#include "Assets.h"'
assets_h_header = "#ifndef ASSETS_H\n#define ASSETS_H"
assets_c_footer = "// Automatically generated by genassets.py\n"
assets_h_footer = (
    "\n\n#define ASSET_COUNT "
    + str(aid)
    + "\n\n#endif\n// Automatically generated by genassets.py\n"
)

if os.path.isfile(path + "Assets.c") and os.path.isfile(path + "Assets.h"):
    # Compare the generated files with the existing files
    with open(path + "Assets.c", "r") as f:
        existing_assets_c = f.read()

    with open(path + "Assets.h", "r") as f:
        existing_assets_h = f.read()
        
    if existing_assets_c == assets_c_header + assets_c + assets_c_footer and existing_assets_h == assets_h_header + assets_h + assets_h_footer:
        print("No changes detected")
        sys.exit(0)

with open(path + "Assets.c", "w") as f:
    f.write(assets_c_header + assets_c + assets_c_footer)

with open(path + "Assets.h", "w") as f:
    f.write(assets_h_header + assets_h + assets_h_footer)
