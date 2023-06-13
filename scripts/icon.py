import sys
import os
import PIL.Image
import PIL.ImageDraw
import PIL.ImageFont


def draw(text, size):
    img = PIL.Image.new("RGB", (64, 64), (255, 128, 12))
    draw = PIL.ImageDraw.Draw(img)
    font = PIL.ImageFont.truetype("arial.ttf", size)
    x, y, w, h = draw.textbbox((0, 0), text, font)
    # print(w, h)
    draw.text(((64 - w) / 2, (64 - h) / 2), text, (0, 0, 0), font=font)
    return img


def main():
    img = draw("V", 48)
    img.save("src/vrmeditor/icon.ico")
    # img.save("icon.png")


if __name__ == "__main__":
    main()
