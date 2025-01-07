import util
from PIL import Image

texture_asset_total_size = 0
texture_asset_count = 0
texture_asset_names = []

def ConvertPNG(path):
    global texture_asset_total_size
    global texture_asset_count
    global texture_asset_names

    img = Image.open(path)
    img = img.convert("RGBA")
    img_dta = img.getdata()
    data = []

    data += util.IntToBytes(img.width * img.height)  # total pixels
    data += util.IntToBytes(img.width)  # width
    data += util.IntToBytes(img.height)  # height
    data += util.IntToBytes(util.aid)

    for pixel in img_dta:
        data.append(pixel[0])
        data.append(pixel[1])
        data.append(pixel[2])
        data.append(pixel[3])

    asset_name = util.GetAssetName(path)

    texture_asset_total_size += img.width * img.height * 4
    texture_asset_names.append(asset_name) # name without extension
    texture_asset_count += 1

    util.WriteAsset(path, "gtex", "texture", util.EncloseData(data, 0))