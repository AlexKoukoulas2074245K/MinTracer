/**********************************************************************/
/** scene.cpp by Alex Koukoulas (C) 2017 All Rights Reserved         **/
/** File Description:                                                **/
/**********************************************************************/

#pragma once

// Local Headers
#include "scene.h"
#include "strutils.h"

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
	: _underConstruction(false)
{
	constructDefaultScene();	
}

Sphere& Scene::getSphere(const size_t index) 
{
	if (_underConstruction)
	{
		return _stubSphere;
	}
	return _spheres[index]; 
}

Light& Scene::getLight(const size_t index) 
{
	if (_underConstruction)
	{
		return _stubLight;
	}
	return *_lights[index]; 
}

Material& Scene::getMaterial(const size_t index) 
{
	if (_underConstruction)
	{
		return _stubMaterial;
	}
	return _materials[index]; 
}

Plane& Scene::getPlane(const size_t index) 
{
	if (_underConstruction)
	{
		return _stubPlane;
	}
	return _planes[index]; 
}

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
	auto openThread = std::thread([filePath, callbackOnCompletion, this]()
	{
		std::ifstream inputFile(filePath, std::ios::in);

		if (inputFile.good())
		{
			std::stringstream fileContents;
			fileContents << inputFile.rdbuf();
			this->constructFromString(fileContents.str());
		}

		callbackOnCompletion(win32::IO_DIALOG_RESULT_TYPE::SUCCESS);
	});

	openThread.detach();
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
	
	result << "#End";

	return result.str();
}

void Scene::constructFromString(const std::string& sceneDescription)
{
	_underConstruction = true;
	_lights.clear();
	_materials.clear();
	_spheres.clear();
	_planes.clear();

	_reflectionCount = 0U;
	_refractionCount = 0U;
	_fresnelPower = 0.0f;

	const auto sceneDescLines = strutils::split(sceneDescription, '\n');

	_reflectionCount = std::stoi(sceneDescLines[1]);
	_refractionCount = std::stoi(sceneDescLines[3]);
	_fresnelPower = std::stof(sceneDescLines[5]);
	
	enum ParsingState
	{
		MATERIAL, LIGHT, SPHERE, PLANE, END
	};

	auto parsingState = MATERIAL;
	auto lineCursor = 7; // Start of material entries

	while (parsingState != END)
	{
		const auto& currentLine = sceneDescLines[lineCursor++];
		const auto currentLineComps = strutils::split(currentLine, ' ');

		switch (parsingState)
		{
			case MATERIAL:
			{
				// Could optimize start of next states with fewer letters
				// such as "#L", but kept the full redundant check against
				// "#Lights" for clarity
				if (!strutils::startsWith(currentLine, "#Lights"))
				{
					_materials.push_back(Material(currentLineComps));
				}
				else
				{
					parsingState = LIGHT;					
				}
			} break;

			case LIGHT:
			{
				if (!strutils::startsWith(currentLine, "#Spheres"))
				{
					_lights.push_back(currentLineComps.size() > 2 ? 
						std::make_unique<PointLight>(currentLineComps) :
						std::make_unique<Light>(currentLineComps));
				}
				else
				{
					parsingState = SPHERE;										
				}
			} break;

			case SPHERE:
			{
				if (!strutils::startsWith(currentLine, "#Planes"))
				{
					_spheres.push_back(Sphere(currentLineComps));
				}
				else
				{
					parsingState = PLANE;					
				}
			} break;

			case PLANE:
			{
				if (strutils::startsWith(currentLine, "#End"))
				{
					parsingState = END;
				}
				else
				{
					_planes.push_back(Plane(currentLineComps));
				}
			} break;
		}
	}
	_underConstruction = false;
}

void Scene::constructDefaultScene()
{
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