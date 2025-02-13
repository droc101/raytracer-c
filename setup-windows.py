# This script downloads the latest versions of SDL2, SDL2_mixer, glew and cglm from GitHub releases, as well as the FindSDL2 and FindSDL2_mixer CMake modules.

import requests
import regex as re
import zipfile
import os
import shutil

sdl_version_regex = re.compile("^2.\\d*.\\d*$") # 2.x.x
any_version_regex = re.compile("^.*$") # Any version

sdl_zip_regex = re.compile("^SDL2-devel-2.\\d*.\\d*-mingw.zip$") # SDL2-devel-2.x.x-mingw.zip
sdl_release_url = "https://api.github.com/repos/libsdl-org/SDL/releases"
sdl_zip_folder_regex = re.compile("^SDL2-2.\\d*.\\d*$") # SDL2-2.x.x

sdl_mixer_zip_regex = re.compile("^SDL2_mixer-devel-2.\\d*.\\d*-mingw.zip$") # SDL2_mixer-devel-2.x.x-mingw.zip
sdl_mixer_release_url = "https://api.github.com/repos/libsdl-org/SDL_Mixer/releases"
sdl_mixer_zip_folder_regex = re.compile("^SDL2_mixer-2.\\d*.\\d*$") # SDL2_mixer-2.x.x

glew_zip_regex = re.compile("^glew-2.\\d*.\\d*-win32.zip$") # glew-2.x.x.zip
glew_release_url = "https://api.github.com/repos/nigels-com/glew/releases"
glew_zip_folder_regex = re.compile("^glew-2.\\d*.\\d*$") # glew-2.x.x

cglm_release_url = "https://api.github.com/repos/recp/cglm/releases"
cglm_zip_folder_regex = re.compile("^recp-cglm-.*$") # recp-cglm-* (should end with a short commit hash)

# Download a release zip and extract & rename it according to the given regex
def handle_release_zip(asset_url, output_folder, output_folder_regex):
    print(f"Downloading {asset_url}")
    response = requests.get(asset_url)
    response.raise_for_status()


    with open("temp/" + output_folder + ".zip", "wb") as file:
        file.write(response.content)

    with zipfile.ZipFile("temp/" + output_folder + ".zip", "r") as zip_ref:
        zip_ref.extractall("temp")

    target_folder = None
    for folder in os.listdir("temp"):
        if output_folder_regex.match(folder):
            target_folder = folder
            break
    else:
        os.remove("temp/" + output_folder + ".zip")
        raise Exception("No matching folder found in the zip file for " + output_folder)
    
    os.rename("temp/" + target_folder, "lib/" + output_folder)
    os.remove("temp/" + output_folder + ".zip")

# Download the latest version of a GitHub repo that matches the given regex
def download_release(api_url, version_regex, asset_regex, output_folder, output_folder_regex):
    if os.path.exists("lib/" + output_folder):
        print(output_folder + " already exists, redownloading...")
        shutil.rmtree("lib/" + output_folder)

    response = requests.get(api_url)
    response.raise_for_status()
    releases = response.json()
    for release in releases:
        tag_name = release["name"]
        if version_regex.match(tag_name):
            if len(release["assets"]) == 0:
                # Download the source code zip file
                asset_url = release["zipball_url"]
                handle_release_zip(asset_url, output_folder, output_folder_regex)
                return
            for asset in release["assets"]:
                asset_name = asset["name"]
                if asset_regex.match(asset_name):
                    asset_url = asset["browser_download_url"]
                    handle_release_zip(asset_url, output_folder, output_folder_regex)
                    return
    raise Exception("No matching version found for " + output_folder + " (found releases: " + str([release["name"] for release in releases]) + ")")

# Download a CMake module to the (local) module folder
def download_cmake_module(url, filename):
    print(f"Downloading {url}")
    response = requests.get(url)
    response.raise_for_status()

    with open("module/" + filename, "wb") as file:
        file.write(response.content)

# Ensure the lib, temp and module folders exist
os.makedirs("lib", exist_ok=True)
os.makedirs("temp", exist_ok=True)
os.makedirs("module", exist_ok=True)

# Get libraries
download_release(sdl_release_url, sdl_version_regex, sdl_zip_regex, "SDL2", sdl_zip_folder_regex)
download_release(sdl_mixer_release_url, sdl_version_regex, sdl_mixer_zip_regex, "SDL2_mixer", sdl_mixer_zip_folder_regex)
download_release(glew_release_url, any_version_regex, glew_zip_regex, "glew", glew_zip_folder_regex)
download_release(cglm_release_url, any_version_regex, None, "cglm", cglm_zip_folder_regex)

# Get CMake modules
download_cmake_module("https://raw.githubusercontent.com/tcbrindle/sdl2-cmake-scripts/refs/heads/master/FindSDL2.cmake", "FindSDL2.cmake")
download_cmake_module("https://raw.githubusercontent.com/tcbrindle/sdl2-cmake-scripts/refs/heads/master/FindSDL2_mixer.cmake", "FindSDL2_mixer.cmake")

# Clean up the temp folder
shutil.rmtree("temp")