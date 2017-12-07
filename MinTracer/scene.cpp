/**********************************************************************/
/** scene.cpp by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "scene.h"

// Remote Headers

Scene& Scene::get()
{
	static Scene scene;
	return scene;
}

Scene::~Scene()
{
}

Scene::Scene()
{
	constructScene();
}

Sphere& Scene::getSphere(const size_t index) { return _spheres[index]; }
Light& Scene::getLight(const size_t index) { return _lights[index]; }
Material& Scene::getMaterial(const size_t index) { return _materials[index]; }
Plane& Scene::getPlane(const size_t index) { return _planes[index]; }

size_t Scene::getSphereCount() const { return _spheres.size(); }
size_t Scene::getLightCount() const { return _lights.size(); }
size_t Scene::getMaterialCount() const { return _materials.size(); }
size_t Scene::getPlaneCount() const { return _planes.size(); }
uint32 Scene::getReflectionCount() const { return _reflectionCount; }
uint32 Scene::getRefractionCount() const { return _refractionCount; }
f32 Scene::getFresnelPower() const { return _fresnelPower; }

void Scene::setReflectionCount(const uint32 reflectionCount) { _reflectionCount = reflectionCount; }
void Scene::setRefractionCount(const uint32 refractionCount) { _refractionCount = refractionCount; }
void Scene::setFresnelPower(const f32 fresnelPower) { _fresnelPower = fresnelPower; }

void Scene::constructScene()
{
	_lights.emplace_back(vec3<f32>(2.0f, 0.0f, -3.0f), vec3<f32>(0.5f, 0.5f, 0.5f));

	_materials.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.3f, 0.1f, 0.1f), vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f), 128.0f, 0.5f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.2f, 0.2f, 0.2f), 1.0f, 0.5f, 1.05f);
	_materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f, 0.1f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.1f, 0.1f, 0.4f), vec3<f32>(0.3f, 0.3f, 0.9f), vec3<f32>(0.3f, 0.3f, 0.9f), 24.0f, 0.5f, 0.0f);

	_spheres.emplace_back(2.0f, vec3<f32>(-3.0f, 1.0f, -9.0f), 1);
	_spheres.emplace_back(0.3f, vec3<f32>(0.0f, 0.0f, -1.3f), 2);
	_spheres.emplace_back(0.5f, vec3<f32>(0.0f, 0.0f, -5.0f), 4);

	_planes.emplace_back(vec3<f32>(0.0f, 0.0f, 1.0f), 10.0f, 3);
	_planes.emplace_back(vec3<f32>(0.0f, 0.0f, -1.0f), 2.0f, 3);
	_planes.emplace_back(vec3<f32>(0.0f, 1.0f, 0.0f), 2.0f, 3);
	_planes.emplace_back(vec3<f32>(0.0f, -1.0f, 0.0f), 4.0f, 3);
	_planes.emplace_back(vec3<f32>(-1.0f, 0.0f, 0.0f), 4.0f, 3);
	_planes.emplace_back(vec3<f32>(1.0f, 0.0f, 0.0f), 4.0f, 3);	

	_reflectionCount = 1;
	_refractionCount = 2;
	_fresnelPower = 1.5f;
}