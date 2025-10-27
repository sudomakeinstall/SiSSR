#!/usr/bin/env python

# System
import argparse
import pathlib as pl
import sys
import logging

# Third Party
import vtk
from vtk.util import numpy_support
import numpy as np
import open3d as o3d
from scipy.spatial.distance import pdist, squareform


def o3d_mesh_remove_valence_3_vertices(mesh, max_iterations=1000):
    """Remove valence-3 vertices one at a time and fill holes with triangles."""

    removed_count = 0
    for iteration in range(max_iterations):
        mesh.compute_adjacency_list()
        adjacency_list = mesh.adjacency_list

        valence_3_vertex = None
        for vertex_idx, neighbors in enumerate(adjacency_list):
            if len(neighbors) == 3:
                valence_3_vertex = vertex_idx
                break

        if valence_3_vertex is None:
            print(f"Removed {removed_count} valence-3 vertices")
            return mesh

        neighbors = list(adjacency_list[valence_3_vertex])
        vertices = np.asarray(mesh.vertices)
        triangles = np.asarray(mesh.triangles)

        triangles_with_vertex = []
        for i, tri in enumerate(triangles):
            if valence_3_vertex in tri:
                triangles_with_vertex.append(tri)

        if triangles_with_vertex:
            ref_tri = triangles_with_vertex[0]
            vertex_pos = np.where(ref_tri == valence_3_vertex)[0][0]

            if vertex_pos == 0:
                edge_neighbors = [ref_tri[1], ref_tri[2]]
            elif vertex_pos == 1:
                edge_neighbors = [ref_tri[2], ref_tri[0]]
            else:
                edge_neighbors = [ref_tri[0], ref_tri[1]]
        else:
            edge_neighbors = neighbors[:2]

        mesh.remove_vertices_by_index([valence_3_vertex])
        removed_count += 1

        adjusted_neighbors = []
        for n in neighbors:
            if n > valence_3_vertex:
                adjusted_neighbors.append(n - 1)
            else:
                adjusted_neighbors.append(n)

        adj_edge_neighbors = []
        for n in edge_neighbors:
            if n > valence_3_vertex:
                adj_edge_neighbors.append(n - 1)
            else:
                adj_edge_neighbors.append(n)

        third_neighbor = None
        for n in adjusted_neighbors:
            if n not in adj_edge_neighbors:
                third_neighbor = n
                break

        if third_neighbor is not None:
            new_triangle = np.array([adj_edge_neighbors + [third_neighbor]])
        else:
            new_triangle = np.array([adjusted_neighbors])

        triangles = np.asarray(mesh.triangles)
        updated_triangles = np.vstack([triangles, new_triangle])
        mesh.triangles = o3d.utility.Vector3iVector(updated_triangles)

        mesh.remove_duplicated_triangles()
        mesh.remove_degenerate_triangles()
        mesh.remove_unreferenced_vertices()

        mesh.compute_vertex_normals()
        mesh.compute_triangle_normals()

    raise RuntimeError(
        f"Failed to remove all valence-3 vertices after {max_iterations} iterations"
    )


def o3d_mesh_find_vertices_by_valence(mesh, target_valence):
    """Find vertices with specified valence in a triangle mesh."""
    mesh.compute_adjacency_list()
    adjacency_list = mesh.adjacency_list

    target_vertices = []
    for vertex_idx, neighbors in enumerate(adjacency_list):
        if len(neighbors) == target_valence:
            target_vertices.append(vertex_idx)

    return np.array(target_vertices)


def o3d_mesh_print_topology_info(m):
    is_watertight = m.is_watertight()
    is_edge_manifold = m.is_edge_manifold(allow_boundary_edges=False)
    is_vertex_manifold = m.is_vertex_manifold()
    non_manifold_edges = m.get_non_manifold_edges()
    non_manifold_vertices = m.get_non_manifold_vertices()

    logging.warning(f"Watertight: {is_watertight}")
    logging.warning(f"Edge Manifold: {is_edge_manifold}")
    logging.warning(f"Vertex Manifold: {is_vertex_manifold}")
    logging.warning(f"Non-manifold edges: {len(non_manifold_edges)}")
    logging.warning(f"Non-manifold vertices: {len(non_manifold_vertices)}")

    boundary_edges = m.get_non_manifold_edges(allow_boundary_edges=True)
    boundary_only_edges = len(boundary_edges) - len(non_manifold_edges)
    logging.warning(f"Boundary edges: {boundary_only_edges}")

    vertices = len(m.vertices)
    triangles = len(m.triangles)

    edges_from_triangles = triangles * 3 // 2
    euler_char = vertices - edges_from_triangles + triangles
    logging.warning(f"Euler characteristic: {euler_char} (should be 2 for watertight)")

    triangle_clusters, cluster_n_triangles, cluster_area = (
        m.cluster_connected_triangles()
    )
    n_clusters = len(set(triangle_clusters))
    logging.warning(f"Connected components: {n_clusters}")
    if n_clusters > 1:
        cluster_sizes = [
            len([x for x in triangle_clusters if x == i]) for i in range(n_clusters)
        ]
        logging.warning(f"Cluster sizes: {cluster_sizes}")

    m.compute_adjacency_list()
    valences = [len(neighbors) for neighbors in m.adjacency_list]
    valence_counts = {}
    for v in valences:
        valence_counts[v] = valence_counts.get(v, 0) + 1
    logging.warning(f"Valence distribution: {dict(sorted(valence_counts.items()))}")

    low_valence = [i for i, v in enumerate(valences) if v <= 2]
    if low_valence:
        logging.warning(f"Low valence vertices (≤2): {len(low_valence)}")

    high_valence = [i for i, v in enumerate(valences) if v > 10]
    if high_valence:
        logging.warning(f"High valence vertices (>10): {len(high_valence)}")


def o3d_mesh_check_geometric_consistency(m):
    """Check for geometric consistency issues that may prevent watertight detection"""
    logging.warning("=== Geometric Consistency Analysis ===")

    vertices = np.asarray(m.vertices)
    triangles = np.asarray(m.triangles)

    distances = squareform(pdist(vertices))
    np.fill_diagonal(distances, np.inf)
    duplicate_pairs = np.where(distances < 1e-10)
    n_duplicates = len(duplicate_pairs[0]) // 2
    if n_duplicates > 0:
        logging.warning(f"Duplicate vertices at same position: {n_duplicates}")

    m.compute_triangle_normals()
    normals = np.asarray(m.triangle_normals)

    zero_normals = np.where(np.linalg.norm(normals, axis=1) < 1e-10)[0]
    if len(zero_normals) > 0:
        logging.warning(
            f"Triangles with zero normals (degenerate): {len(zero_normals)}"
        )

    triangle_areas = []
    for tri in triangles:
        v0, v1, v2 = vertices[tri]
        edge1 = v1 - v0
        edge2 = v2 - v0
        area = 0.5 * np.linalg.norm(np.cross(edge1, edge2))
        triangle_areas.append(area)

    triangle_areas = np.array(triangle_areas)
    tiny_triangles = np.where(triangle_areas < 1e-10)[0]
    if len(tiny_triangles) > 0:
        logging.warning(f"Tiny triangles (area < 1e-10): {len(tiny_triangles)}")

    m.compute_adjacency_list()
    inconsistent_orientations = 0
    total_adjacent_pairs = 0

    for i, triangle in enumerate(triangles):
        for j in range(3):
            v1 = triangle[j]
            v2 = triangle[(j + 1) % 3]

            for k, other_triangle in enumerate(triangles[i + 1 :], i + 1):
                if v1 in other_triangle and v2 in other_triangle:
                    dot_product = np.dot(normals[i], normals[k])
                    if dot_product < 0:
                        inconsistent_orientations += 1
                    total_adjacent_pairs += 1

    if total_adjacent_pairs > 0:
        logging.warning(
            f"Inconsistent triangle orientations: {inconsistent_orientations}/{total_adjacent_pairs}"
        )

    if len(triangles) < 10000:
        self_intersections = 0
        for i in range(len(triangles)):
            for j in range(i + 1, len(triangles)):
                tri1_verts = vertices[triangles[i]]
                tri2_verts = vertices[triangles[j]]

                if len(set(triangles[i]) & set(triangles[j])) > 0:
                    continue

                tri1_min, tri1_max = tri1_verts.min(axis=0), tri1_verts.max(axis=0)
                tri2_min, tri2_max = tri2_verts.min(axis=0), tri2_verts.max(axis=0)

                if np.all(tri1_max >= tri2_min) and np.all(tri2_max >= tri1_min):
                    self_intersections += 1

        if self_intersections > 0:
            logging.warning(
                f"Potential self-intersections (bbox overlap): {self_intersections}"
            )
    else:
        logging.warning("Skipping self-intersection check (mesh too large)")


def o3d_to_vtk_mesh(o3d_mesh):
    """Convert Open3D mesh to VTK mesh"""
    points = np.asarray(o3d_mesh.vertices)
    triangles = np.asarray(o3d_mesh.triangles)
    normals = np.asarray(o3d_mesh.vertex_normals)

    vtk_points = vtk.vtkPoints()
    for point in points:
        vtk_points.InsertNextPoint(point)

    vtk_normals = vtk.vtkFloatArray()
    vtk_normals.SetNumberOfComponents(3)
    vtk_normals.SetName("Normals")
    for normal in normals:
        vtk_normals.InsertNextTuple(normal)

    vtk_cells = vtk.vtkCellArray()
    for triangle in triangles:
        vtk_cells.InsertNextCell(3, triangle)

    vtk_mesh = vtk.vtkPolyData()
    vtk_mesh.SetPoints(vtk_points)
    vtk_mesh.SetPolys(vtk_cells)
    vtk_mesh.GetPointData().SetNormals(vtk_normals)

    return vtk_mesh


def process_mesh(
    input_path: pl.Path,
    output_path: pl.Path,
    depth: int,
    target_triangles: int | None,
    remove_valence_3: bool,
) -> None:
    print(f"Loading mesh from {input_path}...")
    vtk_reader = vtk.vtkOBJReader()
    vtk_reader.SetFileName(str(input_path))
    vtk_reader.Update()
    vtk_mesh = vtk_reader.GetOutput()

    print("Extracting points and normals...")
    points = numpy_support.vtk_to_numpy(vtk_mesh.GetPoints().GetData())
    normals_data = vtk_mesh.GetPointData().GetNormals()

    if normals_data is None:
        print("No normals found in input mesh, computing normals...")
        normals_filter = vtk.vtkPolyDataNormals()
        normals_filter.SetInputData(vtk_mesh)
        normals_filter.ComputePointNormalsOn()
        normals_filter.ComputeCellNormalsOff()
        normals_filter.Update()
        vtk_mesh = normals_filter.GetOutput()
        normals_data = vtk_mesh.GetPointData().GetNormals()

    normals = numpy_support.vtk_to_numpy(normals_data)

    print("Constructing Open3D point cloud...")
    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(points)
    pcd.normals = o3d.utility.Vector3dVector(normals)

    print(f"Running Poisson reconstruction (depth={depth})...")
    poisson_mesh, _ = o3d.geometry.TriangleMesh.create_from_point_cloud_poisson(
        pcd, depth=depth, width=0, scale=1.1, linear_fit=False
    )

    if target_triangles is not None:
        print(f"Decimating to {target_triangles} triangles...")
        poisson_mesh = poisson_mesh.simplify_quadric_decimation(target_triangles)
        poisson_mesh.compute_vertex_normals()

    if remove_valence_3:
        print("Removing valence-3 vertices...")
        poisson_mesh = o3d_mesh_remove_valence_3_vertices(poisson_mesh)

    actual_triangles = len(poisson_mesh.triangles)
    print(f"Final mesh has {actual_triangles} triangles")

    if not poisson_mesh.is_watertight():
        print("\nWARNING: mesh is not watertight.", file=sys.stderr)
        o3d_mesh_print_topology_info(poisson_mesh)
        o3d_mesh_check_geometric_consistency(poisson_mesh)
        print()

    print(f"Saving watertight mesh to {output_path}...")
    vtk_o3d_mesh = o3d_to_vtk_mesh(poisson_mesh)
    vtk_writer = vtk.vtkOBJWriter()
    vtk_writer.SetFileName(str(output_path))
    vtk_writer.SetInputData(vtk_o3d_mesh)
    vtk_writer.Write()

    loop_output_path = (
        output_path.parent / f"{output_path.stem}_loop{output_path.suffix}"
    )
    print(f"Performing loop subdivision...")
    o3d_mesh_loop = poisson_mesh.subdivide_loop(number_of_iterations=1)
    o3d_mesh_loop.compute_vertex_normals()

    loop_triangles = len(o3d_mesh_loop.triangles)
    print(f"Loop subdivision mesh has {loop_triangles} triangles")
    print(f"Saving loop subdivision mesh to {loop_output_path}...")
    vtk_loop_mesh = o3d_to_vtk_mesh(o3d_mesh_loop)
    vtk_writer = vtk.vtkOBJWriter()
    vtk_writer.SetFileName(str(loop_output_path))
    vtk_writer.SetInputData(vtk_loop_mesh)
    vtk_writer.Write()

    print("Done!")


def main():
    parser = argparse.ArgumentParser(
        description="Convert surface mesh to watertight mesh using Poisson reconstruction"
    )
    parser.add_argument(
        "--input",
        type=pl.Path,
        required=True,
        help="Input .obj file path",
    )
    parser.add_argument(
        "--output",
        type=pl.Path,
        required=True,
        help="Output .obj file path",
    )
    parser.add_argument(
        "--depth",
        type=int,
        default=8,
        help="Poisson reconstruction depth (default: 8)",
    )
    parser.add_argument(
        "--target-triangles",
        type=int,
        default=125,
        help="Target triangle count for decimation (optional)",
    )
    parser.add_argument(
        "--skip-valence-removal",
        action="store_true",
        help="Skip valence-3 vertex removal",
    )

    args = parser.parse_args()

    if not args.input.exists():
        print(f"Error: Input file {args.input} does not exist", file=sys.stderr)
        sys.exit(1)

    args.output.parent.mkdir(parents=True, exist_ok=True)

    process_mesh(
        input_path=args.input,
        output_path=args.output,
        depth=args.depth,
        target_triangles=args.target_triangles,
        remove_valence_3=not args.skip_valence_removal,
    )


if __name__ == "__main__":
    main()
