import bpy
import mathutils
import bmesh

def subdivide(context):
	scene = context.scene
	obj = context.active_object

	
	if isinstance(obj.data, bpy.types.Mesh):
		mesh = obj.data

		vertex_count = len(mesh.vertices)
		edge_count = len(mesh.edges)
		polygon_count = len(mesh.polygons)

		vertices_info = []
		edges_info = []
		polygons_info = []

		bm = bmesh.new()

		for i in range(0, vertex_count):
			vertices_info.append({'adjacent_polygons': [], 'adjacent_edges': [], 'new_vertex_point': None})
		for i in range(0, edge_count):
			edges_info.append({'adjacent_polygons': [], 'new_edge_point': None, 'twice_mid_point': None})
		for i in range(0, polygon_count):
			polygons_info.append({'new_polygon_point': None})

		for i in range(0, polygon_count):
			polygon = mesh.polygons[i]
			new_polygon_point = mathutils.Vector()
			for loop_index in range(polygon.loop_start, polygon.loop_start + polygon.loop_total):
				vertex_index = mesh.loops[loop_index].vertex_index
				edge_index = mesh.loops[loop_index].edge_index
				vertices_info[vertex_index]['adjacent_polygons'].append(i)
				edges_info[edge_index]['adjacent_polygons'].append(i)
				new_polygon_point += mesh.vertices[vertex_index].co
			new_polygon_point /= polygon.loop_total
			polygons_info[i]['new_polygon_point'] = bm.verts.new(new_polygon_point)
			

		for i in range(0, edge_count):
			edge = mesh.edges[i]
			v0 = edge.vertices[0]
			v1 = edge.vertices[1]
			vertices_info[v0]['adjacent_edges'].append(i)
			vertices_info[v1]['adjacent_edges'].append(i)
			new_edge_point = mesh.vertices[v0].co + mesh.vertices[v1].co
			count = 2
			edges_info[i]['twice_mid_point'] = mathutils.Vector(new_edge_point)
			for polygon_index in edges_info[i]['adjacent_polygons']:
				count += 1
				new_edge_point += polygons_info[polygon_index]['new_polygon_point'].co
			new_edge_point /= count
			edges_info[i]['new_edge_point'] = bm.verts.new(new_edge_point)

		bm.verts.ensure_lookup_table()

		for i in range(0, vertex_count):
			count = len(vertices_info[i]['adjacent_polygons'])
			new_vertex_point = mathutils.Vector()
			for j in range(0, count):
				polygon_index = vertices_info[i]['adjacent_polygons'][j]
				edge_index = vertices_info[i]['adjacent_edges'][j]
				new_vertex_point += polygons_info[polygon_index]['new_polygon_point'].co + edges_info[edge_index]['twice_mid_point']
			new_vertex_point /= count
			new_vertex_point += mesh.vertices[i].co * (count - 3)
			new_vertex_point /= count
			vertices_info[i]['new_vertex_point'] = bm.verts.new(new_vertex_point)
			

		for i in range(0, polygon_count):
			polygon = mesh.polygons[i]
			new_polygon_point = mathutils.Vector()
			for loop_index in range(polygon.loop_start, polygon.loop_start + polygon.loop_total):
				vertex_index = mesh.loops[loop_index].vertex_index
				edge_index = mesh.loops[loop_index].edge_index
				#next_edge_index = mesh.loops[(loop_index + 1) if (loop_index < polygon.loop_start + polygon.loop_total - 1) else polygon.loop_start].edge_index
				previous_edge_index = mesh.loops[(loop_index - 1) if (loop_index > polygon.loop_start) else (polygon.loop_start + polygon.loop_total - 1)].edge_index

				v0 = vertices_info[vertex_index]['new_vertex_point']
				v1 = edges_info[edge_index]['new_edge_point']
				v2 = polygons_info[i]['new_polygon_point']
				v3 = edges_info[previous_edge_index]['new_edge_point']
				bm.faces.new((v0, v1, v2, v3))
			

		bm.to_mesh(mesh)
		bm.free()