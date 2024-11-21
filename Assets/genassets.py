import os
import sys
from PIL import Image
import gzip
import subprocess
import tempfile

aid = 0


def int_to_bytes(i):  # Convert an integer to bytes big endian, 4 bytes
    return [(i >> 24) & 0xFF, (i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF]

# Convert a PNG file to bytes
def png_to_bytes(path):
    global aid
    global texture_asset_total_size
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
    
    texture_asset_total_size += img.width * img.height * 4

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
    header.extend(int_to_bytes(type))  # Asset Type

    header.extend(data)

    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

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

def frag_to_bytes(path):
    global aid

    data = list(glsl_to_spv(path))

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: FRAG-SPV data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(5))  # Asset Type (5 = fragment)

    header.extend(data)

    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

def vert_to_bytes(path):
    global aid

    data = list(glsl_to_spv(path))

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: VERT-SPV data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(6))  # Asset Type (6 = vertex)

    header.extend(data)

    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

def tesc_to_bytes(path):
    global aid

    data = list(glsl_to_spv(path))

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: TESC-SPV data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(7))  # Asset Type (7 = tessellation control)

    header.extend(data)

    header[19] = 1
    header[20] = 2
    header[21] = 3
    header[22] = 4

    aid += 1

    return header

def tese_to_bytes(path):
    global aid

    data = list(glsl_to_spv(path))

    data += int_to_bytes(len(data))  # array size (excluding header)
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(0)  # unused
    data += int_to_bytes(aid)  # Padding
    for i in range(0, len(data)):
        if data[i] < 0 or data[i] > 255:
            print("Error: TESE-SPV data out of range")
            sys.exit(1)

    decompressed_len = len(data)

    data = gzip.compress(bytes(data))

    header = bytearray()
    header.extend(int_to_bytes(len(data)))  # Compressed length
    header.extend(int_to_bytes(decompressed_len))  # Decompressed length
    header.extend(int_to_bytes(aid))  # Asset ID
    header.extend(int_to_bytes(8))  # Asset Type (8 = tessellation evaluation)

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
texture_asset_names = []
texture_asset_total_size = 0


def recursive_search(path):
    global count
    global assets_c
    global assets_h
    global texture_asset_names
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
                texture_asset_names.append(name)
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
            elif file.endswith(".frag"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = frag_to_bytes(path + file)
                name = "gzfrag_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".vert"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = vert_to_bytes(path + file)
                name = "gzvert_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".tesc"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = tesc_to_bytes(path + file)
                name = "gztesc_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            elif file.endswith(".tese"):
                count += 1
                print("Converting " + path_from_assets + file)
                data = tese_to_bytes(path + file)
                name = "gztese_" + foldername + "_" + file.split(".")[0]
                assets_c += bytes_to_c_array(data, name)
                assets_h += c_header_array(name, len(data))
            else:
                print("Unrecognized file type: " + file)


recursive_search(path)

print("Converted " + str(count) + " files")

texture_asset_array = "\n\nconst unsigned char *texture_assets[TEXTURE_ASSET_COUNT] = {\n    "
texture_asset_array += ", \n    ".join(texture_asset_names)
texture_asset_array += "\n};\n\n"

texture_asset_array_header = "\n\nextern const unsigned char *texture_assets[];\n\n#define TEXTURE_ASSET_COUNT " + str(len(texture_asset_names)) + "\n\n#define TEXTURE_ASSET_TOTAL_SIZE " + str(texture_asset_total_size) + "\n\n"

assets_c_header = '#include "Assets.h"'
assets_h_header = "#ifndef ASSETS_H\n#define ASSETS_H"
assets_c_footer = "// Automatically generated by genassets.py\n"
assets_h_footer = (
    "\n\n#define ASSET_COUNT "
    + str(aid)
    + "\n\n#endif\n// Automatically generated by genassets.py\n"
)

final_assets_c = assets_c_header + assets_c + texture_asset_array + assets_c_footer
final_assets_h = assets_h_header + assets_h + texture_asset_array_header + assets_h_footer

if os.path.isfile(path + "Assets.c") and os.path.isfile(path + "Assets.h"):
    # Compare the generated files with the existing files
    with open(path + "Assets.c", "r") as f:
        existing_assets_c = f.read()

    with open(path + "Assets.h", "r") as f:
        existing_assets_h = f.read()
        
    if existing_assets_c == final_assets_c and existing_assets_h == final_assets_h:
        print("No changes detected")
        sys.exit(0)

with open(path + "Assets.c", "w") as f:
    f.write(final_assets_c)

with open(path + "Assets.h", "w") as f:
    f.write(final_assets_h)
