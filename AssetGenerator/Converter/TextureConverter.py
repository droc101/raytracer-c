import util
from PIL import Image

def ConvertPNG(path):
    img = Image.open(path)
    img = img.convert("RGBA")
    img_dta = img.getdata()
    data = []

    data += util.IntToBytes(img.width * img.height)  # total pixels
    data += util.IntToBytes(img.width)  # width
    data += util.IntToBytes(img.height)  # height
    data += util.IntToBytes(0) # NOTICE: This is NO LONGER the asset id!

    for pixel in img_dta:
        data.append(pixel[0])
        data.append(pixel[1])
        data.append(pixel[2])
        data.append(pixel[3])
    util.WriteAsset(path, "gtex", "texture", util.EncloseData(data, 0))