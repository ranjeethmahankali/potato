"""Generate the png assets required for this project."""
from os import path
from PIL import Image
from argparse import ArgumentParser
import numpy as np
import subprocess


def generateTexture():
    """Generate the texture as a png file and a raw binary file."""
    parser = ArgumentParser()
    parser.add_argument(
        'dest', help='The target directory to copy the generated assets to')
    args = parser.parse_args()
    root = path.dirname(__file__)
    dest = path.join(root, args.dest)
    src = path.join(root, 'pieces.svg')
    pngpath = path.join(dest, 'pieces.png')
    print("Saving the texture to {} ...".format(pngpath))
    subprocess.run(['inkscape', src, '-o', pngpath])
    arr = np.array(Image.open(pngpath))
    assert arr.shape[2] == 4
    arr = np.flip(arr, axis=0)
    pixels = []
    for x in range(arr.shape[0]):
        for y in range(arr.shape[1]):
            p = arr[x, y]
            pixels.append(
                np.uint32(p[0] | (p[1] << 8) | (p[2] << 16)
                          | (p[3] << 24)))
    assert len(pixels) == 768 * 256
    final = np.array(pixels).astype(np.uint32)
    outpath = path.join(dest, 'pieces.dat')
    print("Writing texture to {} ...".format(outpath))
    final.tofile(outpath)


if __name__ == "__main__":
    generateTexture()
