/**********************************************************************/
/** main.cpp by Alex Koukoulas (C) 2017 All Rights Reserved          **/
/** File Description: Main entry point, containing the bulk of the   **/
/** Ray tracing code                                                 **/
/**********************************************************************/

#if defined(DEBUG) || defined(_DEBUG)
//#include <vld.h>
#endif

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <iomanip>

#include "win32gui.h"
#include "scene.h"
#include "typedefs.h"
#include "math.h"
#include "image.h"

static const f32 T_MIN = 0.01f;
static const f32 T_MAX = 100.0f;

using namespace std;

// HitInfo is essentially the info storage Struct 
// for each Ray being cast
struct HitInfo
{
    bool hit;
    vec3<f32> position;
    vec3<f32> normal;
    uint8 surfaceMatIndex;
    f32 t;

    HitInfo(const bool hit, 
            const vec3<f32>& position, 
            const vec3<f32>& normal,
            const uint8& surfaceMatIndex,
            const f32 t)
        : hit(hit)
        , position(position)
        , normal(normal)
        , surfaceMatIndex(surfaceMatIndex)
        , t(t)
    {
    }
};

HitInfo rayPlaneIntersectionTest(const Ray& ray, const Plane& plane)
{
    const auto denom = dot(plane.normal, ray.direction);

    if (denom > 1e-6f || denom < -1e-6f)
    {
        const auto t = -(plane.d + (dot(plane.normal, ray.origin)))/denom;
        if (t > 0.0f)
        {
            const auto hitPos = ray.origin + ray.direction * t;
            return HitInfo(true, hitPos, plane.normal, plane.matIndex, t);                        
        }        
    }

    return HitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

HitInfo raySphereIntersectionTest(const Ray& ray, const Sphere& sphere)
{
    const auto toRay = ray.origin - sphere.center;

    const auto a = dot(ray.direction, ray.direction);
    const auto b = 2.0f * dot(toRay, ray.direction);
    const auto c = dot(toRay, toRay) - sphere.radius * sphere.radius;
    const auto det = b * b - 4 * a * c;

    if (det > 0.0f)
    {
        const auto minT = (-b - sqrtf(det)) / 2 * a;
        const auto maxT = (-b + sqrtf(det)) / 2 * a;
        const auto selT = minT > 0.0f ? minT : (maxT > 0.0f ? maxT : 0.0f);

        if (selT > 0.0f)
        {
            const auto hitPos = ray.origin + ray.direction * selT;
            auto normal = normalize(hitPos - sphere.center);

            if (length(ray.origin - sphere.center) < sphere.radius)
            {
                normal = -normal;
            }

            return HitInfo(true, hitPos, normal, sphere.matIndex, selT);
        }                
    }
    
    return HitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
}

HitInfo intersectScene(const Ray& ray)
{
    HitInfo closestHitInfo(false, vec3<f32>(), vec3<f32>(), 0, T_MAX);
    
    const auto sphereCount = Scene::get().getSphereCount();
    for (auto i = 0U; i < sphereCount; ++i)
    {
        auto hitInfo = raySphereIntersectionTest(ray, Scene::get().getSphere(i));

        if (hitInfo.hit && hitInfo.t < closestHitInfo.t)
        {            
            closestHitInfo = hitInfo;
        }
    }

    const auto planeCount = Scene::get().getPlaneCount();
    for (auto i = 0U; i < planeCount; ++i)
    {
        auto hitInfo = rayPlaneIntersectionTest(ray, Scene::get().getPlane(i));

        if (hitInfo.hit && hitInfo.t < closestHitInfo.t)
        {            
            closestHitInfo = hitInfo;
        }
    }

    return closestHitInfo;
}

vec3<f32> shade(const Ray& ray, const Light& light, const HitInfo& hitInfo)
{    
    vec3<f32> colorAccum;

    const auto epsilon = 1e-5f;
    const auto displacedHitPos = hitInfo.position + hitInfo.normal * epsilon;

    const auto hitToLight = normalize(light.position - displacedHitPos);    
    const auto viewDir = normalize(displacedHitPos - ray.origin);
    const auto reflDir = normalize(viewDir - hitInfo.normal * dot(viewDir, hitInfo.normal) * 2.0f);
    
    const auto diffuseTerm = max(0.0f, dot(hitInfo.normal, hitToLight));
    const auto& material = Scene::get().getMaterial(hitInfo.surfaceMatIndex);
    const auto specularTerm = powf(max(0.0f, dot(reflDir, hitToLight)), material.glossiness);

    colorAccum += (material.diffuse * light.color) * diffuseTerm;    
    if (light.getLightType() == Light::POINT_LIGHT)
    {                
        // This downcast might cause issues if it executes concurrently with
        // reconstructing the scene from an existing file, due to the way
        // stubs are returned during construction.
        colorAccum /= 4 * PI * static_cast<const PointLight&>(light).radius;
    }

    colorAccum += (material.specular * light.color) * specularTerm;    
    

    const auto lightHitInfo = intersectScene(Ray(hitToLight, displacedHitPos));        
    const auto displacedLightHitPos = lightHitInfo.position + lightHitInfo.normal * epsilon;

    // Shadow test
    if (lightHitInfo.hit)
    {                                                                
        auto visibility = 1.0f;

        const auto prevHitToLight = light.position - displacedHitPos;
        const auto revHitToLight = light.position - displacedLightHitPos;
        const auto originalHitToLightMag = length(prevHitToLight);
        const auto reverseIntersectionHitToLightMag = length(revHitToLight);
            
        // In order to cancel visibility, i.e. the object is in shadow, we need to make sure that there
        // exists an object inbetween the original hit object and the light's position
        const auto objectInBetweenHitInfoAndLight = (originalHitToLightMag - reverseIntersectionHitToLightMag) > 1e-6f;            
        const auto objectNotBehindLight = dot(normalize(prevHitToLight), normalize(revHitToLight)) >= 1.0f - 1e-6f;

        // Enshadow only if the above conditions are satisfied
        visibility = objectInBetweenHitInfoAndLight && objectNotBehindLight ? 0.0f : 1.0f;
        colorAccum *= visibility;        
    }            

    return colorAccum;
}

vec3<f32> traceForEachLight(const Ray& ray, const HitInfo& hitInfo)
{
    if (!hitInfo.hit) return vec3<f32>();

    vec3<f32> fragment = Scene::get().getMaterial(hitInfo.surfaceMatIndex).ambient;

    const auto lightCount = Scene::get().getLightCount();
    for (auto i = 0U; i < lightCount; ++i)
    {
        fragment += shade(ray, Scene::get().getLight(i), hitInfo);
    }    

    return fragment;
}

f32 fresnel(const Ray& ray, const vec3<f32>& normal, const f32 ior)
{
    return powf(1.0f - dot(-ray.direction, normal), Scene::get().getFresnelPower());
}

vec3<f32> trace(const Ray& ray)
{    
    auto initialRay = ray;
    auto initialHitInfo = intersectScene(ray);    

    auto currentRay = initialRay;
    auto currentHitInfo = initialHitInfo;
    auto currentFragColor = traceForEachLight(currentRay, currentHitInfo);
    auto reflectionWeight = 1.0f;

    // Compute Reflection
    const auto reflectionCount = Scene::get().getReflectionCount();
    for (auto i = 0U; i < reflectionCount; ++i)
    {
        if (!currentHitInfo.hit) break;

        reflectionWeight *= Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).reflectivity > 0.0f ? 0.5f : 0.0f;
                
        const auto reflectionDir = normalize(ray.direction - currentHitInfo.normal * dot(ray.direction, currentHitInfo.normal) * 2.0f);
        const auto epsilon = 1e-3f;
        auto fresnelKr = 1.0f;
        
        if (Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity > 1.0f)
        {
            fresnelKr = fresnel(currentRay, currentHitInfo.normal, Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity);
        }        

        currentRay = Ray(reflectionDir, currentHitInfo.position + epsilon * reflectionDir);
        currentHitInfo = intersectScene(currentRay);        
        currentFragColor += (reflectionWeight * fresnelKr) * traceForEachLight(currentRay, currentHitInfo);
    }

    // Compute Refraction
    const auto refractionCount = Scene::get().getRefractionCount();
    auto refractionWeight = 1.0f;
    currentRay = initialRay;    
    currentHitInfo = initialHitInfo;
    for (auto i = 0U; i < refractionCount; ++i)
    {
        if (!currentHitInfo.hit) break;

        refractionWeight *= Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity > 1.0f ? 0.5f : 0.0f;
        
        
        auto cosi = dot(currentRay.direction, currentHitInfo.normal);                        

        auto etaAir = 1.0f;
        auto etaT = Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity;
        auto n = currentHitInfo.normal;

        if (cosi < 0.0f)
        {
            cosi = -cosi;
        }
        else
        {
            swap(etaAir, etaT);
            n = -currentHitInfo.normal;
        }

        const auto eta = etaAir/etaT;
        const auto k = 1 - eta * eta * (1 - cosi * cosi);
        const auto refractionDir = k < 0.0f ? vec3<f32>() : eta * currentRay.direction + (eta * cosi - sqrtf(k)) * n;
        const auto epsilon = 1e-3f;
        
        auto fresnelKt = 1.0f;

        if (Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).reflectivity > 0.0f)
        {
            fresnelKt = 1.0f - fresnel(currentRay, currentHitInfo.normal, Scene::get().getMaterial(currentHitInfo.surfaceMatIndex).refractivity);
        }

        currentRay = Ray(refractionDir, currentHitInfo.position + epsilon * refractionDir);
        currentHitInfo = intersectScene(currentRay);                
        currentFragColor += (refractionWeight * fresnelKt) * traceForEachLight(currentRay, currentHitInfo);
    }

    return currentFragColor;
}

void render(const sint32 currentRenderWidth,
            const sint32 currentRenderHeight, 
            const sint32 endGoalWidth, 
            const sint32 endGoalHeight, 
            const bool& renderStopFlag,
            HWND windowHandle)
{
    // Initilize ray tracing result
    Image resultImage(currentRenderWidth, currentRenderHeight);    

    // Compute ray direction parameters    
    const auto invWidth = 1.0f / currentRenderWidth;
    const auto invHeight = 1.0f / currentRenderHeight;
    const auto fov = PI / 3.0f; 
    const auto aspect = static_cast<f32>(currentRenderWidth) / currentRenderHeight;
    const auto angle = tan(fov * 0.5f);

    // Could probably parameterize worker count
    const auto threadCount = 2;    
    const auto renderStart = chrono::steady_clock::now();
    
    // Debug-specific thread, announcing Ray tracing completion percentages
    atomic_long rowsRendered = 0;
    thread announcer([&rowsRendered, &renderStopFlag, currentRenderHeight]()
    {
#if defined(DEBUG) || defined(_DEBUG)
        auto currentPercent = 0;
        while (rowsRendered != currentRenderHeight)
        {
            if (renderStopFlag) return;

            const auto completedPerc = static_cast<uint32>(100 * (static_cast<f32>(rowsRendered) / currentRenderHeight));
            if (currentPercent != completedPerc)
            {
                currentPercent = completedPerc;
                OutputDebugString(string("Ray Tracing " + to_string(currentPercent) + "% complete\n").c_str());
            }
        }
#endif
    });

    // Main Ray-tracing workers
    thread workers[threadCount];
    for (auto i = 0; i < threadCount; ++i)
    {
        // Divide work amongst threads
        const auto workPerThread = currentRenderHeight / threadCount;
        auto currentThreadWork = workPerThread;

        // The last thread also gets any outstanding work if present
        if (i == threadCount - 1 && currentRenderHeight % workPerThread != 0)
        {
            currentThreadWork += currentRenderHeight % workPerThread;
        }
    
        workers[i] = thread([&resultImage, &rowsRendered, &renderStopFlag, i, workPerThread, currentThreadWork, currentRenderWidth, invWidth, invHeight, angle, aspect]()
        {
            for (auto y = i * workPerThread; y < i * workPerThread + currentThreadWork && !renderStopFlag; ++y)
            {
                for (auto x = 0; x < currentRenderWidth; ++x)
                {            
                    // Transform to normalized coordinates
                    const auto xx = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspect;
                    const auto yy = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
                    
                    // Compute ray direction
                    vec3<f32> rayDirection(xx, yy, -1.0f);
                    rayDirection = normalize(rayDirection);

                    // Perform Ray tracing
                    Ray ray(rayDirection, vec3<f32>());
                    resultImage[y][x] = trace(ray);                    
                }
                rowsRendered++;
            }
        });
    }

    for (auto i = 0; i < threadCount; ++i)
    {
        workers[i].join();
    }    
    announcer.join();

    const auto diff = chrono::steady_clock::now() - renderStart;
    cout << "Ray Trace finished - " << chrono::duration<double, milli>(diff).count() << " ms elapsed | " << threadCount << " with worker(s)" << endl;    

    if (renderStopFlag) return;    

    // Scale result
    const auto invRoundedScaleFactor = resultImage.scale((endGoalWidth + endGoalHeight) / static_cast<f32>(currentRenderWidth + currentRenderHeight));

    // Create bitmap array
    auto* arr = (COLORREF*)calloc(endGoalWidth * endGoalHeight, sizeof(COLORREF));

    // Fill bitmap array
    for (auto y = 0; y < endGoalHeight; ++y)
    {
        if (renderStopFlag) return;

        for (auto x = 0; x < endGoalWidth; ++x)
        {
            if (renderStopFlag) return;

            const auto colorVec = resultImage[minu(y, resultImage.getHeight() - 1)][minu(x, resultImage.getWidth() - 1)];
            arr[y * endGoalWidth + x] = RGB(minf(1.0f, colorVec.z) * 255, minf(1.0f, colorVec.y) * 255, minf(1.0f, colorVec.x) * 255);
        }
    }
    
    // Creating and display bitmap
    auto map = CreateBitmap(endGoalWidth, endGoalHeight, 1, 8 * 4, (void*)arr);
    auto src = CreateCompatibleDC(GetDC(windowHandle));
    SelectObject(src, map); 
    BitBlt(GetDC(windowHandle), 0, 0, endGoalWidth, endGoalHeight, src, 0, 0, SRCCOPY);
    DeleteDC(src); // Deleting temp HDC
    free(arr);

    // Write result to file
    std::stringstream outputFileNameStream;
    outputFileNameStream << "output_images/last_rendering" << std::fixed << std::setprecision(2) << 1.0f/invRoundedScaleFactor << "x.bmp";
    resultImage.writeToBMP(outputFileNameStream.str());

    cout << "Finished writing output to file.. " << endl;
}


// Window Parameters. These are global in order to be modified from the win32gui functions.
// Unfortunately passing custom win32 messages was such a pain, I had to revert 
// back to using externs :(
auto currentWindowWidth = 842U;
auto currentWindowHeight = 683U;

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmd, int ncmd)
{            
    // Initialize Scene    
    auto renderStopFlag = false;

    // Render Size Parameters. The apparent clutter with respect to these, is 
    // due to the many operations that could trigger re-rendering, such as 
    // window resizing, object editing, resolution upgrading etc..
    auto prevWindowWidth = currentWindowWidth;
    auto prevWindowHeight = currentWindowHeight;

    // This is the final end-goal resolution, at which point,
    // the program will stop trying to increase image quality
    auto endGoalWidth = prevWindowWidth * 4;
    auto endGoalHeight = prevWindowHeight * 4;
    
    // This is the initial resolution that will be rendered
    auto startingRenderWidth = prevWindowWidth / 8;
    auto startingRenderHeight = prevWindowHeight / 8;

    // Initialize Window
    auto windowHandle = win32::CreateMainWindow(instance, prevWindowWidth, prevWindowHeight, "MinTracer");    

    // Render Target Parameters
    auto currentRenderWidth = startingRenderWidth;
    auto currentRenderHeight = startingRenderHeight;    
    auto rendering = false;
    auto saveScene = false;
    auto openScene = false;

    MSG msg = {};    
    while (msg.message != WM_QUIT )
    {
        switch(msg.message)
        {        
            // InvalidateRect has been called, hence sending a WM_PAINT message, 
            // due to resizing, resolution advancements, or other GUI related actions.            
            case WM_PAINT:
            {                        
                if (!rendering && currentRenderWidth <= endGoalWidth)
                { 
                    // The rendering flag is used because we are starting rendering, so if WM_PAINT is sent again for some reason, 
                    // we don't restart the process
                    rendering = true;
                    renderStopFlag = false;

                    // Thread responsible for spawning workers and performing Ray Tracing
                    thread masterRayTraceThread([&rendering, &currentRenderWidth, &currentRenderHeight, prevWindowHeight, prevWindowWidth, endGoalWidth, endGoalHeight, &renderStopFlag, windowHandle]()
                    {                        
                        render(currentRenderWidth, currentRenderHeight, prevWindowWidth, prevWindowHeight, renderStopFlag, windowHandle);
                        SetWindowText(windowHandle, ("MinTracer -- Current resolution: " + to_string(currentRenderWidth) + " x " + to_string(currentRenderHeight)).c_str());
                        
                        // Ray Tracing completed for current resolution, 
                        // double the render resolution for the next rendering
                        if (currentRenderWidth <= endGoalWidth)
                        {
                            currentRenderWidth *= 2;
                            currentRenderHeight *= 2;                        
                        }
                        rendering = false;

                    });

                    masterRayTraceThread.detach();
                }            
            } break;

            // A menu item has been selected (be it a main or submenu), 
            // hence the response is different based on the GUI item's functionality
            case WM_COMMAND:
            {
                switch (LOWORD(msg.wParam))
                {                            
                    case win32::GUID_OPEN_SCENE:
                    {
                        openScene = true;
                    } break;

                    case win32::GUID_SAVE_SCENE:
                    {                        
                        saveScene = true;
                    } break;

                    case win32::GUID_QUIT_SCENE: 
                    {
                        renderStopFlag = true;
                        PostQuitMessage(0);
                    } break; 

                    case win32::GUID_REFL_REFR_COUNT_RENDER:
                    {
                        win32::CreateReflectionAndRefractionCountDialog(windowHandle, instance);
                    } break;

                    case win32::GUID_RESTART_RENDER:
                    {
                        renderStopFlag = true;
                        currentRenderWidth = startingRenderWidth; 
                        currentRenderHeight = startingRenderHeight; 
                    } break;
                }

                // Might also need to create the dynamic gui menus for each 
                // object in the scene, based on the object count
                const auto planeCount = Scene::get().getPlaneCount();
                for (auto i = 0U; i < planeCount; ++i)
                {
                    if (LOWORD(msg.wParam) == win32::PLANE_GUID_OFFSET + i)
                    {
                        win32::CreatePlanesEditDialog(windowHandle, instance, i);
                    }
                }

                const auto sphereCount = Scene::get().getSphereCount();
                for (auto i = 0U; i < sphereCount; ++i)
                {
                    if (LOWORD(msg.wParam) == win32::SPHERE_GUID_OFFSET + i)
                    {
                        win32::CreateSpheresEditDialog(windowHandle, instance, i);
                    } 
                }

                const auto lightCount = Scene::get().getLightCount();
                for (auto i = 0U; i < lightCount; ++i)
                {
                    if (LOWORD(msg.wParam) == win32::LIGHT_GUID_OFFSET + i)
                    {
                        win32::CreateLightsEditDialog(windowHandle, instance, i);
                    }
                }

            } break;

            // A trackbar from children dialogs was updated
            // and hence rendering needs to be restarted
            case WM_HSCROLL:
            {                
                renderStopFlag = true;
                currentRenderWidth = startingRenderWidth;
                currentRenderHeight = startingRenderHeight;
            } break;
        }


        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Handle window resizing (currentWindowWidth/Height not matching previously known dimensions)
        if (prevWindowHeight != currentWindowHeight || prevWindowWidth != currentWindowWidth)
        {
            prevWindowWidth = currentWindowWidth;
            prevWindowHeight = currentWindowHeight;

            endGoalWidth = prevWindowWidth * 4;
            endGoalHeight = prevWindowHeight * 4;
            startingRenderWidth = prevWindowWidth / 8;
            startingRenderHeight = prevWindowHeight / 8;

            renderStopFlag = true;
            currentRenderWidth = startingRenderWidth;
            currentRenderHeight = startingRenderHeight;
        }

        // Force repaint when upgrading resolution
        if (currentRenderWidth <= endGoalWidth && !rendering)
        {
            InvalidateRect(windowHandle, NULL, TRUE);
        }

        // Async scene saving
        if (saveScene)
        {
            win32::CreateIODialog(windowHandle, instance, win32::IO_DIALOG_TYPE::SAVE_AS, [](const win32::IO_DIALOG_RESULT_TYPE resultType)
            {
                switch (resultType)
                {
                    case win32::SUCCESS:
                    {
                        MessageBox(NULL, "Scene successfully saved", "Save Scene", MB_OK);
                    } break;

                    case win32::FAILURE:
                    {
                        MessageBox(NULL, "Error: Could not save scene", "Error", MB_OK | MB_ICONERROR);
                    } break;
                }
                
            });
            saveScene = false;
        }

        // Async scene loading
        if (openScene)
        {
            win32::CreateIODialog(windowHandle, instance, win32::IO_DIALOG_TYPE::OPEN, [&currentRenderWidth, &currentRenderHeight, &renderStopFlag, &startingRenderWidth, &startingRenderHeight](const win32::IO_DIALOG_RESULT_TYPE resultType)
            {
                switch (resultType)
                {
                    case win32::SUCCESS:
                    {
                        renderStopFlag = true;
                        currentRenderWidth = startingRenderWidth;
                        currentRenderHeight = startingRenderHeight;
                    } break;

                    case win32::FAILURE:
                    {
                        MessageBox(NULL, "Error: Could not load scene", "Error", MB_OK | MB_ICONERROR);
                    } break;
                }
            });
            openScene = false;
        }
    }        

    renderStopFlag = true;

    // Wait for threads to catch up to the rendering stop flag and shut down
    Sleep(1000);

    return 0;
}