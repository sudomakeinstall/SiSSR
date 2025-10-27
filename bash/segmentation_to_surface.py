#!/usr/bin/env python

# System
import argparse
import pathlib as pl

# Third Party
import itk
import vtk


def itk_pipeline(input_path, labels, spacing, opening_radius, closing_radius):
    print("        Read...")
    image = itk.imread(input_path, itk.ctype("signed short"))

    print("        Binarize...")
    if isinstance(labels, int):
        image = itk.binary_threshold_image_filter(
            image,
            lower_threshold=labels,
            upper_threshold=labels,
            inside_value=1,
            outside_value=0,
        )
    elif isinstance(labels, (list, tuple)):
        masks = []
        for label in labels:
            mask = itk.binary_threshold_image_filter(
                image,
                lower_threshold=label,
                upper_threshold=label,
                inside_value=1,
                outside_value=0,
            )
            masks.append(mask)
        image = masks[0]
        for mask in masks[1:]:
            image = itk.or_image_filter(image, mask)
    else:
        raise ValueError(f"labels must be int or list of ints, got {type(labels)}")

    print("        Resample...")
    i_spacing = itk.spacing(image)
    i_size = itk.size(image)

    o_spacing = [spacing] * 3
    o_size = [int(round(i_size[i] * i_spacing[i] / spacing)) for i in range(3)]

    image = itk.resample_image_filter(
        image,
        size=o_size,
        output_spacing=o_spacing,
        output_origin=image.GetOrigin(),
        output_direction=image.GetDirection(),
        interpolator=itk.NearestNeighborInterpolateImageFunction.New(image),
    )

    StructuringElementType = itk.FlatStructuringElement[3]

    print("        Morphological opening...")
    opening_structuring_element = StructuringElementType.Ball(radius=opening_radius)

    image = itk.binary_morphological_opening_image_filter(
        image,
        kernel=opening_structuring_element,
        foreground_value=1,
        background_value=0,
    )

    print("        Morphological closing...")
    closing_structuring_element = StructuringElementType.Ball(radius=closing_radius)

    image = itk.binary_morphological_closing_image_filter(
        image,
        kernel=closing_structuring_element,
        foreground_value=1,
    )

    print("        Fill holes...")
    image = itk.binary_fillhole_image_filter(image, foreground_value=1)

    print("        Retain largest connected component...")
    image = itk.connected_component_image_filter(image)
    image = itk.label_shape_keep_n_objects_image_filter(
        image,
        background_value=0,
        number_of_objects=1,
        attribute="NumberOfPixels",
    )

    print("        Pad...")
    image = itk.constant_pad_image_filter(
        image,
        pad_lower_bound=[1, 1, 1],
        pad_upper_bound=[1, 1, 1],
        constant=0,
    )

    return image


def vtk_pipeline(itk_image, threshold=0.5):
    print("        Convert to vtk image...")
    vtk_image = itk.vtk_image_from_image(itk_image)

    print("        Run SurfaceNets...")
    sn = vtk.vtkSurfaceNets3D()
    sn.SetInputData(vtk_image)
    sn.SetValue(0, threshold)
    sn.SetSmoothing(True)
    sn.SetAutomaticSmoothingConstraints(True)
    sn.SetOutputMeshTypeToTriangles()
    sn.Update()

    print("        Clean...")
    cleaner = vtk.vtkCleanPolyData()
    cleaner.SetInputConnection(sn.GetOutputPort())
    cleaner.Update()

    print("        Compute normals...")
    normals_filter = vtk.vtkPolyDataNormals()
    normals_filter.SetInputConnection(cleaner.GetOutputPort())
    normals_filter.ComputePointNormalsOn()
    normals_filter.ComputeCellNormalsOff()
    normals_filter.SplittingOff()
    normals_filter.ConsistencyOn()
    normals_filter.AutoOrientNormalsOn()
    normals_filter.Update()

    mesh = normals_filter.GetOutput()

    return mesh


def run_pipeline(
    input_path: pl.Path,
    labels: int | list[int],
    spacing: float,
    opening_radius: int,
    closing_radius: int,
    force_overwrite: bool,
    candidate_dir: pl.Path,
) -> None:
    vtk_mesh_path = candidate_dir / f"{pl.Path(input_path.stem).stem}.obj"

    needs_vtk_processing = True
    if vtk_mesh_path.is_file() and not force_overwrite:
        input_mtime = input_path.stat().st_mtime
        output_mtime = vtk_mesh_path.stat().st_mtime
        if output_mtime >= input_mtime:
            needs_vtk_processing = False
            print("    ✅ VTK mesh already exists and is up to date! Loading...")
        else:
            print("    🔄 Input file is newer than output, regenerating VTK mesh...")

    if needs_vtk_processing:
        if force_overwrite and vtk_mesh_path.is_file():
            print("    🔄 Force overwrite enabled, regenerating VTK mesh...")
        print("    🔧 Running ITK pipeline...")
        image = itk_pipeline(
            input_path, labels, spacing, opening_radius, closing_radius
        )

        print("    🎯 Running VTK pipeline...")
        vtk_mesh = vtk_pipeline(image)

        print("    💾 Saving VTK mesh...")
        vtk_writer = vtk.vtkOBJWriter()
        vtk_writer.SetFileName(vtk_mesh_path)
        vtk_writer.SetInputData(vtk_mesh)
        vtk_writer.Write()


def main():
    parser = argparse.ArgumentParser(
        description="Convert segmentation files to surface meshes using ITK and VTK"
    )
    parser.add_argument(
        "--input-dir",
        type=pl.Path,
        required=True,
        help="Input directory containing segmentation files (.nii.gz)",
    )
    parser.add_argument(
        "--output-dir",
        type=pl.Path,
        required=True,
        help="Output directory for surface meshes (.obj)",
    )
    parser.add_argument(
        "--spacing",
        type=float,
        default=0.75,
        help="Resampling spacing (default: 0.75)",
    )
    parser.add_argument(
        "--labels",
        type=int,
        nargs="+",
        default=[1, 2, 3],
        help="Label values to extract (default: 1 2 3)",
    )
    parser.add_argument(
        "--opening-radius",
        type=int,
        default=3,
        help="Morphological opening radius (default: 3)",
    )
    parser.add_argument(
        "--closing-radius",
        type=int,
        default=3,
        help="Morphological closing radius (default: 3)",
    )
    parser.add_argument(
        "--force-overwrite",
        action="store_true",
        help="Force overwrite of existing output files",
    )

    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    segmentations = sorted(args.input_dir.glob("*.nii.gz"))

    if not segmentations:
        print(f"No .nii.gz files found in {args.input_dir}")
        return

    print(f"Found {len(segmentations)} segmentation files")

    for seg in segmentations:
        print(seg)
        run_pipeline(
            seg,
            labels=args.labels,
            spacing=args.spacing,
            opening_radius=args.opening_radius,
            closing_radius=args.closing_radius,
            force_overwrite=args.force_overwrite,
            candidate_dir=args.output_dir,
        )


if __name__ == "__main__":
    main()
