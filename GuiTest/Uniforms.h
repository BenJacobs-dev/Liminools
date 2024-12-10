#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>

#ifndef MAX_OBJECTS
#define MAX_OBJECTS 32
#endif

#ifndef MAX_IMAGES
#define MAX_IMAGES 16
#endif


#ifndef MAX_SUBSTRUCTURE_DEPTH
#define MAX_SUBSTRUCTURE_DEPTH 10
#endif

#ifndef WORLD_OBJECT_H
#define WORLD_OBJECT_H

struct WorldObject {
    alignas(16) glm::vec3 center;
    alignas(16) glm::vec3 size;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 textureIndex;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 is_negated;
    alignas(4) glm::float32 shadow_blur;
    alignas(4) glm::float32 shadow_intensity;
    alignas(4) glm::float32 reflectivity;
    alignas(4) glm::float32 transparency;
    alignas(4) glm::float32 diffuse_intensity;
    alignas(4) glm::float32 refractive_index;
};

struct WorldObjectCombineModifier {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 index2;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectDomainModifier {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectIndex {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index;
    alignas(4) glm::int32 type;
};

struct WorldObject_v0_1_3 {
    alignas(16) glm::vec3 center;
    alignas(16) glm::vec3 size;
    alignas(16) glm::vec3 color;
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 textureIndex;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 is_negated;
    alignas(4) glm::float32 shadow_blur;
    alignas(4) glm::float32 shadow_intensity;
    alignas(4) glm::float32 reflectivity;
    alignas(4) glm::float32 transparency;
    alignas(4) glm::float32 diffuse_intensity;
    alignas(4) glm::float32 refractive_index;
};

struct WorldObjectCombineModifier_v0_1_3 {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 index2;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectDomainModifier_v0_1_3 {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectIndex_v0_1_3 {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3;
    alignas(16) glm::vec4 data4;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::int32 index;
    alignas(4) glm::int32 type;
};

struct WorldObject_v0_1_2 {
    alignas(16) glm::vec3 center;
    alignas(16) glm::vec3 size;
    alignas(16) glm::vec3 color;
    alignas(4) int type;
    alignas(4) int is_negated;
    alignas(4) int has_shadow;
    alignas(4) float reflectivity;
    alignas(4) float transparency;
    alignas(4) float diffuse_intensity;
};

struct WorldObjectCombineModifier_v0_1_2 {
    alignas(16) glm::vec4 data1;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 index2;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectDomainModifier_v0_1_2 {
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(4) glm::int32 index1;
    alignas(4) glm::int32 type;
    alignas(4) glm::int32 index1Type;
};

struct WorldObjectIndex_v0_1_2 {
    alignas(16) glm::vec4 padding;
    alignas(4) glm::int32 index;
    alignas(4) glm::int32 type;
};

#endif

#ifndef CAMERA_DATA_H
#define CAMERA_DATA_H

struct CameraData {
    alignas(16) glm::vec3 camera_pos;
    alignas(16) glm::vec3 camera_rot;
    alignas(16) glm::vec3 light_pos;
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3; //data.w: fog density
    alignas(16) glm::vec4 data4; //data4.x: FOV, data4.y: Player Speed, data4.z: Normal Offset, data4.w: timeMultiplier
    alignas(8) glm::vec2 resolution;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::float32 time;
    alignas(4) glm::int32 num_steps;
    alignas(4) glm::float32 min_step;
    alignas(4) glm::float32 max_dist;
    alignas(4) glm::int32 ray_depth;
};

struct CameraData_v0_1_3 {
    alignas(16) glm::vec3 camera_pos;
    alignas(16) glm::vec3 camera_rot;
    alignas(16) glm::vec3 light_pos;
    alignas(16) glm::vec4 data1;
    alignas(16) glm::vec4 data2;
    alignas(16) glm::vec4 data3; //data.w: fog density
    alignas(16) glm::vec4 data4; //data4.x: FOV, data4.y: Player Speed, data4.z: Normal Offset, data4.w: timeMultiplier
    alignas(8) glm::vec2 resolution;
    alignas(4) glm::int32 int1;
    alignas(4) glm::int32 int2;
    alignas(4) glm::int32 int3;
    alignas(4) glm::int32 int4;
    alignas(4) glm::int32 int5;
    alignas(4) glm::int32 int6;
    alignas(4) glm::int32 int7;
    alignas(4) glm::int32 int8;
    alignas(4) glm::float32 time;
    alignas(4) glm::int32 num_steps;
    alignas(4) glm::float32 min_step;
    alignas(4) glm::float32 max_dist;
    alignas(4) glm::int32 ray_depth;
};

struct CameraData_v0_1_2 {
    alignas(8) glm::vec2 u_resolution;
    alignas(4) glm::float32 u_time;
    alignas(16) glm::vec3 u_camera_pos;
    alignas(16) glm::vec3 u_camera_rot;
    alignas(16) glm::vec3 u_light_pos;
    alignas(4) glm::int32 u_num_steps;
    alignas(4) glm::float32 u_min_step;
    alignas(4) glm::float32 u_max_dist;
};

#endif


#ifndef WORLD_DATA_H
#define WORLD_DATA_H

struct WorldObjectsData {
    alignas(4) glm::int32 num_objects;
    alignas(16) WorldObject objects[MAX_OBJECTS];
    alignas(4) glm::int32 num_combine_modifiers;
    alignas(16) WorldObjectCombineModifier combineModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_domain_modifiers;
    alignas(16) WorldObjectDomainModifier domainModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_indices;
    alignas(16) WorldObjectIndex indices[MAX_OBJECTS];
};

struct WorldObjectsData_v0_1_3 {
    alignas(4) glm::int32 num_objects;
    alignas(16) WorldObject objects[MAX_OBJECTS];
    alignas(4) glm::int32 num_combine_modifiers;
    alignas(16) WorldObjectCombineModifier combineModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_domain_modifiers;
    alignas(16) WorldObjectDomainModifier domainModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_indices;
    alignas(16) WorldObjectIndex indices[MAX_OBJECTS];
};

struct WorldObjectsData_v0_1_2 {
    alignas(4) glm::int32 num_objects;
    alignas(16) WorldObject_v0_1_2 objects[MAX_OBJECTS];
    alignas(4) glm::int32 num_combine_modifiers;
    alignas(16) WorldObjectCombineModifier_v0_1_2 combineModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_domain_modifiers;
    alignas(16) WorldObjectDomainModifier_v0_1_2 domainModifiers[MAX_OBJECTS];
    alignas(4) glm::int32 num_indices;
    alignas(16) WorldObjectIndex_v0_1_2 indices[MAX_OBJECTS];
};

#endif // !WORLD_DATA_H

#ifndef ANIMATION_DATA_H
#define ANIMATION_DATA_H

struct AnimationData {
    size_t offset;
	int typeVal;
	int typeAnimation;
	int curI;
	int stepI;
	int minI;
	int maxI;
	float timePerStep;
	float timeSinceLastStep;
	float stepF;
	float curF;
	float minF;
	float maxF;
	bool increasing;
};

struct AnimationData_v0_1_3 {
    int typeVal;
    int typeAnimation;
    int curI;
    int stepI;
    int minI;
    int maxI;
    float timePerStep;
    float timeSinceLastStep;
    float stepF;
    float curF;
    float minF;
    float maxF;
    bool increasing;
};

struct AnimationKey {
    enum class DataSource { Camera, World } source;
    size_t offset;  // Byte offset within the corresponding struct

    // Define equality operator for use in std::unordered_map
    bool operator==(const AnimationKey& other) const {
        return source == other.source && offset == other.offset;
    }
};

// Define a hash function for the AnimationKey structure
namespace std {
    template <>
    struct hash<AnimationKey> {
        size_t operator()(const AnimationKey& key) const {
            return hash<int>()(static_cast<int>(key.source)) ^ hash<size_t>()(key.offset);
        }
    };
}


#endif

struct SaveData {
    CameraData camData;
    WorldObjectsData worldData;
};

struct SaveData_v0_1_3 {
    CameraData_v0_1_3 camData;
    WorldObjectsData_v0_1_3 worldData;
};