objs = FreeCADGui.Selection.getSelection()

for o in objs:
    print(f"{o.Label}:")
    x, y, z = int(o.Shape.BoundBox.XLength), int(
        o.Shape.BoundBox.YLength), int(o.Shape.BoundBox.ZLength)
    measures = []
    for i in (x, y, z):
        if i != 14:
            measures.append(i)
    print(measures)
