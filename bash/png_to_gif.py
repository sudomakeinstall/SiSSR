#!/usr/bin/env python

# System
import argparse
import pathlib as pl

# Third Party
import numpy as np
from PIL import Image


def create_gif(
    input_dir: pl.Path,
    output_path: pl.Path,
    duration: int,
    loop: int,
    optimize: bool,
    resize: tuple[int, int] | None,
    colors: int | None,
) -> None:
    png_files = sorted(input_dir.glob("*.png"), key=lambda p: int(p.stem))

    if not png_files:
        print(f"No PNG files found in {input_dir}")
        return

    print(f"Found {len(png_files)} PNG files")

    images = []
    for png_file in png_files:
        img = Image.open(png_file)
        if resize:
            img = img.resize(resize, Image.Resampling.LANCZOS)
        images.append(img)

    if colors is None:
        colors = 256

    print(f"Converting to palette mode with {colors} colors...")
    converted_images = []
    for img in images:
        if img.mode == "RGBA":
            alpha = img.getchannel("A")
            rgb_img = img.convert("RGB")
            p_img = rgb_img.quantize(colors=colors-1, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG)

            palette = p_img.getpalette()
            transparency_index = colors - 1
            palette.extend([0, 0, 0])
            p_img.putpalette(palette)

            p_img_array = np.array(p_img)
            alpha_array = np.array(alpha)
            p_img_array[alpha_array <= 128] = transparency_index
            p_img = Image.fromarray(p_img_array, mode="P")
            p_img.putpalette(palette)
            p_img.info["transparency"] = transparency_index

            converted_images.append(p_img)
        else:
            rgb_img = img.convert("RGB")
            p_img = rgb_img.quantize(colors=colors, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.FLOYDSTEINBERG)
            converted_images.append(p_img)

    images = converted_images

    print(f"Creating GIF at {output_path}...")
    images[0].save(
        output_path,
        save_all=True,
        append_images=images[1:],
        duration=duration,
        loop=loop,
        optimize=optimize,
        disposal=2,
    )

    file_size = output_path.stat().st_size / (1024 * 1024)
    print(f"Done! GIF size: {file_size:.2f} MB")


def main():
    parser = argparse.ArgumentParser(
        description="Convert a sequence of PNG files to an animated GIF"
    )
    parser.add_argument(
        "--input-dir",
        type=pl.Path,
        required=True,
        help="Input directory containing PNG files",
    )
    parser.add_argument(
        "--output",
        type=pl.Path,
        required=True,
        help="Output GIF file path",
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=100,
        help="Duration between frames in milliseconds (default: 100)",
    )
    parser.add_argument(
        "--loop",
        type=int,
        default=0,
        help="Number of loops (0 = infinite, default: 0)",
    )
    parser.add_argument(
        "--optimize",
        action="store_true",
        help="Enable GIF optimization",
    )
    parser.add_argument(
        "--resize",
        type=str,
        default=None,
        help="Resize frames to WIDTHxHEIGHT (e.g., 800x600)",
    )
    parser.add_argument(
        "--colors",
        type=int,
        default=None,
        help="Max colors in palette (default: no limit, max: 256). Lower values = smaller file but reduced quality",
    )

    args = parser.parse_args()

    if not args.input_dir.exists():
        print(f"Error: Input directory {args.input_dir} does not exist")
        return

    resize = None
    if args.resize:
        try:
            width, height = map(int, args.resize.split("x"))
            resize = (width, height)
        except ValueError:
            print(
                f"Error: Invalid resize format '{args.resize}'. Use WIDTHxHEIGHT (e.g., 800x600)"
            )
            return

    args.output.parent.mkdir(parents=True, exist_ok=True)

    create_gif(
        input_dir=args.input_dir,
        output_path=args.output,
        duration=args.duration,
        loop=args.loop,
        optimize=args.optimize,
        resize=resize,
        colors=args.colors,
    )


if __name__ == "__main__":
    main()
