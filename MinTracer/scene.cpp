/**********************************************************************/
/** scene.cpp by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "scene.h"

// Remote Headers
#include <thread>
#include <fstream>

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
Light& Scene::getLight(const size_t index) { return *_lights[index]; }
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

void Scene::saveScene(const std::string& filePath, win32::io_result_callback callbackOnCompletion)
{
	auto savingThread = std::thread([filePath, callbackOnCompletion, this]() 
	{
		std::ofstream outputFile(filePath, std::ios::out);

		if (outputFile.good())
		{	
			outputFile << this->toString();
			callbackOnCompletion(win32::IO_DIALOG_RESULT_TYPE::SUCCESS);
		}
		else
		{
			callbackOnCompletion(win32::IO_DIALOG_RESULT_TYPE::FAILURE);
		}
	});

	savingThread.detach();
}

void Scene::openScene(const std::string& filePath, win32::io_result_callback callbackOnCompletion)
{
	const auto b = false;
}

std::string Scene::toString() const
{
	std::stringstream result;
	result << "#Reflection Count\n";
	result << _reflectionCount << "\n";
	result << "#Refraction Count\n";
	result << _refractionCount << "\n";
	result << "#Fresnel Power\n";
	result << _fresnelPower << "\n";

	result << "#Materials\n";
	for (const auto& material: _materials)
	{
		result << material.toString() << "\n";
	}

	result << "#Lights\n";	
	for (const auto& light : _lights)
	{
		result << light->toString() << "\n";
	}

	result << "#Spheres\n";	
	for (const auto& sphere: _spheres)
	{
		result << sphere.toString() << "\n";
	}
	
	result << "#Planes\n";
	for (const auto& plane : _planes)
	{
		result << plane.toString() << "\n";
	}

	return result.str();
}

void Scene::constructScene()
{
	//_lights.emplace_back(std::make_unique<Light>(vec3<f32>(2.0f, 0.0f, -3.0f), vec3<f32>(0.5f, 0.5f, 0.5f)));
	_lights.emplace_back(std::make_unique<PointLight>(vec3<f32>(0.0f, -0.5f, -6.0f), vec3<f32>(1.0f, 1.0f, 1.0f), 0.2f));

	_materials.emplace_back(vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.3f, 0.1f, 0.1f), vec3<f32>(0.9f, 0.3f, 0.3f), vec3<f32>(0.9f, 0.3f, 0.3f), 128.0f, 1.0f, 1.10f);
	_materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.3f, 0.3f, 0.3f), vec3<f32>(0.3f, 0.3f, 0.3f), 1.0f, 0.5f, 1.05f);
	_materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f, 0.1f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.2f, 0.2f, 0.2f), vec3<f32>(0.5f, 0.5f, 0.5f), vec3<f32>(0.5f, 0.5f, 0.5f), 1.0f, 0.0f, 0.0f);
	_materials.emplace_back(vec3<f32>(0.1f, 0.1f, 0.4f), vec3<f32>(0.3f, 0.3f, 0.9f), vec3<f32>(0.3f, 0.3f, 0.9f), 24.0f, 0.0f, 0.0f);

	_spheres.emplace_back(2.0f, vec3<f32>(-3.0f, 1.0f, -4.0f), 1);
	_spheres.emplace_back(0.3f, vec3<f32>(0.5f, 0.0f, -1.3f), 2);
	_spheres.emplace_back(0.5f, vec3<f32>(2.0f, 0.0f, -5.0f), 5);

	_planes.emplace_back(vec3<f32>(0.0f, 0.0f, 1.0f), 10.0f, 3);
	_planes.emplace_back(vec3<f32>(0.0f, 0.0f, -1.0f), 2.0f, 4);
	_planes.emplace_back(vec3<f32>(0.0f, 1.0f, 0.0f), 2.0f, 4);
	_planes.emplace_back(vec3<f32>(0.0f, -1.0f, 0.0f), 4.0f, 4);
	_planes.emplace_back(vec3<f32>(-1.0f, 0.0f, 0.0f), 4.0f, 4);
	_planes.emplace_back(vec3<f32>(1.0f, 0.0f, 0.0f), 4.0f, 4);	

	_reflectionCount = 1;
	_refractionCount = 2;
	_fresnelPower = 1.5f;
}