/********************************************************************/
/** scene.h by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                              **/
/********************************************************************/

#pragma once

// Local Headers
#include "math.h"

// Remote Headers
#include <vector>
#include <memory>

struct Material
{
	vec3<f32> ambient;
	vec3<f32> diffuse;
	vec3<f32> specular;
	f32 glossiness;
	f32 reflectivity;
	f32 refractivity;

	Material(const vec3<f32>& ambient, 
		     const vec3<f32>& diffuse,
		     const vec3<f32>& specular, 
		     const f32 glossiness, 
		     const f32 reflectivity,
		     const f32 refractivity)
		: ambient(ambient)
		, diffuse(diffuse)
		, specular(specular)
		, glossiness(glossiness)
		, reflectivity(reflectivity)
		, refractivity(refractivity)
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
	// Hack I know
	enum LightType
	{
		DIR_LIGHT, POINT_LIGHT
	};

	vec3<f32> position;
	vec3<f32> color;

	Light(const vec3<f32>& position, const vec3<f32>& color)
		: position(position)
		, color(color)
	{
	}

	virtual ~Light(){}

	virtual LightType getLightType() const { return DIR_LIGHT; }
};

struct PointLight: public Light
{
	f32 radius;

	PointLight(const vec3<f32>& position, const vec3<f32>& color, const f32 radius)
		: Light(position, color)
		, radius(radius)
	{
	}

	LightType getLightType() const override { return POINT_LIGHT; }
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
	f32 getFresnelPower() const;

	size_t getSphereCount() const;
	size_t getLightCount() const;
	size_t getMaterialCount() const;
	size_t getPlaneCount() const;

	void setReflectionCount(const uint32 reflectionCount);
	void setRefractionCount(const uint32 refractionCount);
	void setFresnelPower(const f32 fresnelPower);

private:
	Scene();
	void constructScene();

private:
	std::vector<std::unique_ptr<Light>> _lights;	
	std::vector<Sphere> _spheres;
	std::vector<Material> _materials;
	std::vector<Plane> _planes;

	uint32 _reflectionCount;
	uint32 _refractionCount;
	f32    _fresnelPower;

};