#include "components.h"
#include "extraray.h"


XYZcontrol init_XYZ_controls() {
    XYZcontrol xyz;
    xyz.hidden_box = GenMeshCube(20000, 20000, 20000);

    xyz.x.ray.hit = false;
    xyz.y.ray.hit = false;
    xyz.z.ray.hit = false;

    xyz.x.axis = (Vector3){1, 0, 0};
    xyz.y.axis = (Vector3){0, 1, 0};
    xyz.z.axis = (Vector3){0, 0, 1};

    xyz.x.rotation_axis = (Vector3){0, 1, 0};
    xyz.y.rotation_axis = (Vector3){0, 0, 1};
    xyz.z.rotation_axis = (Vector3){1, 0, 0};

    xyz.x.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.y.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.z.hit_box.mesh = GenMeshPlane(.6, 7.5, 1, 1);

    xyz.x.rotation_box.material = LoadMaterialDefault();
    xyz.x.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF0000FF);

    xyz.y.rotation_box.material = LoadMaterialDefault();
    xyz.y.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00FF00FF);

    xyz.z.rotation_box.material = LoadMaterialDefault();
    xyz.z.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x0000FFFF);

    xyz.x.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.y.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.z.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);

    return xyz;
}