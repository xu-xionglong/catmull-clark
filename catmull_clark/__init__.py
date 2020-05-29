bl_info = {
	"name": "catmull clark subdivision",
	"author": "xionglong.xu",
	"version": (0, 0, 1),
	"blender": (2, 82, 0),
	"category": "Object"
}

if "bpy" in locals():
    import importlib
    if "catmull_clark" in locals():
        importlib.reload(catmull_clark)

import bpy

class Subdivision(bpy.types.Operator):
	"""Subdivide mesh with catmull clark algorithm"""
	bl_idname = "object.catmull_clark_subdivide"
	bl_label = "Catmull Clark Subdivision"
	bl_options = {'REGISTER', 'UNDO'}

	def execute(self, context):
		from . import catmull_clark
		catmull_clark.subdivide(context)
		return {'FINISHED'}		

def register():
	bpy.utils.register_class(Subdivision)

def unregister():
	bpy.utils.unregister_class(Subdivision)
	
if __name__ == "__main__":
    register() 