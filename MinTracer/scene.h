/********************************************************************/
/** scene.h by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers
#include "math.h"

// Remote Headers
#include <vector>


struct Material
{
	vec3<f32> ambient;
	vec3<f32> diffuse;
	vec3<f32> specular;
	f32 glossiness;
	f32 reflectivity;

	Material(const vec3<f32>& ambient, const vec3<f32>& diffuse, const vec3<f32>& specular, const f32 glossiness, const f32 reflectivity)
		: ambient(ambient)
		, diffuse(diffuse)
		, specular(specular)
		, glossiness(glossiness)
		, reflectivity(reflectivity)
	{
	}
};

struct Sphere
{
	f32 radius;
	vec3<f32> center;
	uint32 matIndex;

	Sphere(const f32 radius, const vec3<f32>& center, const uint32 matIndex)
		: radius(radius)
		, center(center)
		, matIndex(matIndex)
	{
	}
};

struct Plane
{
	vec3<f32> normal;
	f32 d;
	uint32 matIndex;

	Plane(const vec3<f32>& normal, const f32 d, const uint32 matIndex)
		: normal(normal)
		, d(d)
		, matIndex(matIndex)
	{
	}
};

struct Ray
{
	vec3<f32> direction;
	vec3<f32> origin;

	Ray(const vec3<f32>& direction, const vec3<f32>& origin)
		: direction(direction)
		, origin(origin)
	{
	}
};

struct Light
{
	vec3<f32> position;
	vec3<f32> color;

	Light(const vec3<f32>& position, const vec3<f32>& color)
		: position(position)
		, color(color)
	{
	}
};

class Scene final
{
public:
	static Scene& get();
	~Scene();

	Sphere& getSphere(const size_t index);
	Light& getLight(const size_t index);
	Material& getMaterial(const size_t index);
	Plane& getPlane(const size_t index);
	uint32 getReflectionCount() const;
	uint32 getRefractionCount() const;

	size_t getSphereCount() const;
	size_t getLightCount() const;
	size_t getMaterialCount() const;
	size_t getPlaneCount() const;

	void setReflectionCount(const uint32 reflectionCount);
	void setRefractionCount(const uint32 refractionCount);

private:
	Scene();
	void constructScene();

private:
	std::vector<Sphere> _spheres;
	std::vector<Light> _lights;
	std::vector<Material> _materials;
	std::vector<Plane> _planes;

	uint32 _reflectionCount;
	uint32 _refractionCount;
};