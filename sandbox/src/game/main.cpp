#include <fenv.h>

#include <cmath>

#include "window/window.h"
#include "rendering/sprite.h"
#include "rendering/line.h"
#include "rendering/model.h"
#include "rendering/string.h"
#include "rendering/terrain.h"
#include "rendering/text.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#include "utils/lib.h"
#include "utils/timer.h"
#include "utils/lexical_cast.h"
#include "simulation/boids.h"
#include <sstream>
#include <IL/ilut.h>
#include <vector>

class Accumulator
{
    const float mBase;
    const float mStep;
    float mAccumulator;
    bool mRunning;

public:
    Accumulator(float base,
                float step = 1.f):
        mBase(base),
        mStep(step),
        mAccumulator(0.f),
        mRunning(false)
    {}

    void reset()
    {
        mRunning = true;
        mAccumulator = mBase;
    }

    void update()
    {
        if (mRunning) {
            mAccumulator += mStep;
            assert(mAccumulator >= 0.0f);
        }
    }
    void update(float dt)
    {
        if (mRunning) {
            mAccumulator += dt;
            assert(mAccumulator >= 0.0f);
        }
    }

    void stop() { mRunning = false; }
    float getValue() const { return mAccumulator; }
    bool running() const { return mRunning; }
};

struct Scene
{
    std::shared_ptr<sb::Shader> colorShader;
    std::shared_ptr<sb::Shader> textureShader;
    std::shared_ptr<sb::Shader> textureLightShader;
    std::shared_ptr<sb::Shader> shadowShader;

    std::shared_ptr<sb::Shader> fogShader;
    
    sb::Sprite crosshair;
    sb::Model skybox;
    sb::Model dragon;
    sb::Terrain terrain;
    sb::Model tree;
    sb::Model sun;
    sb::Model goat;
    
    std::vector<Vec3> treeCoordinates;
    
    sb::Light pointLight = sb::Light::point(Vec3(10.0, 10.0, 0.0), 1000.0f);
    sb::Light parallelLight = sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 1.0f);

    Scene():
        colorShader(gResourceMgr.getShader("proj_basic.vert", "color.frag")),
        textureShader(gResourceMgr.getShader("proj_texture.vert", "texture.frag")),
        textureLightShader(gResourceMgr.getShader("proj_texture_normal.vert", "texture_normal.frag")),
        shadowShader(gResourceMgr.getShader("proj_shadow.vert", "shadow.frag")),
        fogShader(gResourceMgr.getShader("fog_shader.vert","fog.frag")),
        crosshair("dot.png", textureShader),
        skybox("skybox.obj", textureShader,
               gResourceMgr.getTexture("miramar_no_sun.jpg")),
        dragon("salamon.obj", fogShader),
        terrain("hmap_perlin.jpg", "ground.jpg", fogShader),
        tree("Tree.obj", fogShader, gResourceMgr.getTexture("Tree.jpg")),
        sun("sphere.obj", colorShader),
        goat("koza.obj", textureLightShader, gResourceMgr.getTexture("goat.png")),
        treeCoordinates(),
        pointLight(sb::Light::point(Vec3(10.0, 10.0, 0.0), 1.0f)),
        parallelLight(sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 1.0f))
    {
        dragon.setPosition(5.f, 1.f, 1.f);
        dragon.setScale(10.f);
        crosshair.setPosition(0.f, 0.f, 0.f);
        crosshair.setScale(0.01f, 0.01f * 1.33f, 1.f);

        skybox.setScale(1000.f);

        terrain.setScale(10.f, 50.f, 10.f);
        terrain.setPosition(-640.f, -25.0f, -640.f);
        terrain.setTexture("tex2", gResourceMgr.getTexture("blue_marble.jpg"));
        gLog.debug("terrain @ 0, 0: %f", terrain.getHeightAt(0.0f, 0.0f));

        sun.setScale(100.0f);
        sun.setColor(sb::Color(1.0f, 1.0f, 0.8f));
        goat.setScale(10.0f);
        
        srand((unsigned)time(0));
        const size_t NUM_TREES = 100;
        for (size_t i = 0; i < NUM_TREES; i++ ) {
            Vec3 pos((rand() % 300) - 150, 0, (rand() % 300) - 150);
            pos.y = terrain.getHeightAt(pos.x, pos.z);

            treeCoordinates.push_back(pos);
        }      

        tree.setPosition(1.f, 1.f, 1.f);
        tree.setScale(15.0f);
        gLog.info("all data loaded!\n");
    }
};

class Game
{
public:
    static constexpr float SPEED = 0.5f;
    static constexpr float PHYSICS_UPDATE_STEP = 0.03f;
    static constexpr float fpsUpdateStep = 1.f;   // update FPS-string every fpsUpdateStep seconds

    sb::Window wnd;
    Scene scene;

    Vec3 speed;

    Accumulator deltaTime;

    float fpsCounter;      // how many frames have been rendered in fpsUpdateStep time?
    float fpsCurrValue;    // current FPS value
    Accumulator fpsDeltaTime;
    std::string fpsString;

    Sim::Boids boids;

    Accumulator throwVelocity;
    Accumulator windVelocity;

    Game():
        wnd(1280, 1024),
        scene(),
        speed(0.0f, 0.0f, 0.0f),
        deltaTime(0.0f, 0.0f),
        fpsCounter(0.0f),
        fpsCurrValue(0.0f),
        fpsDeltaTime(0.0f, 0.0f),
        boids(90, scene.textureLightShader),
        throwVelocity(10.f, 0.5f),
        windVelocity(0.5f, 0.5f)
    {
        wnd.setTitle("Sandbox");
        wnd.lockCursor();
        wnd.hideCursor();
        wnd.getCamera().lookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

        deltaTime.reset();
        fpsDeltaTime.reset();
    }

    void drawStrings()
    {
        uint32_t nextLine = 0u;
        wnd.drawString(fpsString, { 0.0f, 0.0f },
                       (fpsCurrValue > 30.f
                           ? sb::Color::Green
                           : (fpsCurrValue > 20.f ? sb::Color::Yellow
                                                  : sb::Color::Red)),
                       nextLine++);
        wnd.drawString(sb::utils::format("pos = {0}\n"
                           "front = {1} len = {2}\n"
                           "right = {3}\n"
                           "upReal = {4}\n"
                           "phi = {5} ({6})\n"
                           "theta = {7} ({8})",
                           wnd.getCamera().getEye(),
                           wnd.getCamera().getFront(),
                           wnd.getCamera().getFront().length(),
                           wnd.getCamera().getRight(),
                           wnd.getCamera().getUpReal(),
                           Degrees(wnd.getCamera().getHorizontalAngle()),
                           Radians(wnd.getCamera().getHorizontalAngle()),
                           Degrees(wnd.getCamera().getVerticalAngle()),
                           Radians(wnd.getCamera().getVerticalAngle())),
                       { 0.f, 0.f }, sb::Color::White, nextLine);
        nextLine += 6;
    }

    void draw()
    {
        wnd.clear(sb::Color(0.f, 0.f, 0.5f));

        wnd.setAmbientLightColor(sb::Color(0.2f, 0.2f, 0.2f));
        wnd.addLight(scene.pointLight);
        wnd.addLight(scene.parallelLight);

        // environment
        wnd.draw(scene.skybox);
        wnd.draw(scene.terrain);

        drawBoids();

        wnd.draw(scene.dragon);
        wnd.draw(scene.goat);

        // crosshair
        wnd.draw(scene.crosshair);

        for (Vec3 position : scene.treeCoordinates){ 
            scene.tree.setPosition(position);
            wnd.draw(scene.tree);
        }

        wnd.draw(scene.sun);
        
        drawStrings();

        wnd.display();
    }


    void drawBoids()
    { 
        for(sb::Fish &fish : boids.shoalOfFish) {
            fish.setVelocity(boids.calculateVelocity(fish, wnd.getCamera().getEye()));
        }

        for(sb::Fish &fish : boids.shoalOfFish) {
            fish.setPosition(boids.calculatePosition(fish));
            wnd.draw(fish);
        }
    }


    bool isRunning() {
        return wnd.isOpened();
    }

    void updateFixedStep(float timeStep)
    {
        throwVelocity.update();
        windVelocity.update();

        static Radians angle(0.0f);
        angle = Radians(angle.value() + 0.1f * timeStep);

        Vec3 lightPos(20.0f * std::sin(angle.value()),
                      -50.0f * std::sin(angle.value()) + 20.0f,
                      20.0f * std::cos(angle.value()));
        scene.parallelLight.pos = lightPos;

        Vec3 sunPos = lightPos.normalized() * 900.0f;
        sunPos.y = -sunPos.y;
        sunPos += wnd.getCamera().getEye();
        scene.sun.setPosition(sunPos);

        Vec3 pos(std::sin(angle.value() * 5.0f), 0.0f, std::cos(angle.value() * 5.0f));
        pos *= 10.0f;
        pos.y = scene.terrain.getHeightAt(pos.x, pos.z);
        scene.goat.setPosition(pos);
    }

    void update(float delta)
    {
        deltaTime.update(delta);
        fpsDeltaTime.update(delta);
        ++fpsCounter;

        // FPS update
        if (fpsDeltaTime.getValue() >= fpsUpdateStep)
        {
            fpsCurrValue = fpsCounter / fpsUpdateStep;
            fpsString = "FPS = " + sb::utils::toString(fpsCurrValue);
            fpsDeltaTime.update(-fpsUpdateStep);
            fpsCounter = 0.f;
        }

        // physics update
        uint32_t guard = 3u;
        while ((deltaTime.getValue() >= PHYSICS_UPDATE_STEP) && guard--)
        {
            deltaTime.update(-PHYSICS_UPDATE_STEP);
            updateFixedStep(PHYSICS_UPDATE_STEP);
        }

        // drawing
        wnd.getCamera().moveRelative(speed);

        // move skybox, so player won't go out of it
        scene.skybox.setPosition(wnd.getCamera().getEye());
    }

    void handleMousePressed(const sb::Event& e)
    {
        switch (e.data.mouse.button)
        {
            case sb::Mouse::ButtonLeft:
                if (!throwVelocity.running()) {
                    throwVelocity.reset();
                }
                break;
            case sb::Mouse::ButtonRight:
                if (!windVelocity.running()) {
                    windVelocity.reset();
                }
                break;
            default:
                break;
        }
    }

    void handleMouseReleased(const sb::Event& e)
    {
        switch(e.data.mouse.button)
        {
        case sb::Mouse::ButtonLeft:
            break;
        case sb::Mouse::ButtonRight:
            break;
        default:
            break;
        }
    }

    void handleKeyPressed(const sb::Event& e)
    {
        switch (e.data.key)
        {
        case sb::Key::Num1:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(50.f, 0.f, 0.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::Num2:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(-50.f, 0.f, 0.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::Num3:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(0.0001f, 50.f, 0.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::Num4:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(0.0001f, -50.f, 0.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::Num5:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(0.f, 0.f, 50.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::Num6:
            speed = Vec3(0.0f, 0.0f, 0.0f);
            wnd.getCamera().lookAt(Vec3(0.f, 0.f, -50.f),
                                   Vec3(0.f, 0.f, 0.f));
            break;
        case sb::Key::A: speed.x = -SPEED; break;
        case sb::Key::D: speed.x = SPEED; break;
        case sb::Key::S: speed.z = -SPEED; break;
        case sb::Key::W: speed.z = SPEED; break;
        case sb::Key::Q: speed.y = SPEED; break;
        case sb::Key::Z: speed.y = -SPEED; break;
        case sb::Key::Esc:
            wnd.close();
            break;
        default:
            break;
        }
    }

    void handleKeyReleased(const sb::Event& e)
    {
        switch (e.data.key)
        {
        case sb::Key::A:
        case sb::Key::D:
            speed.x = 0.f;
            break;
        case sb::Key::S:
        case sb::Key::W:
            speed.z = 0.f;
            break;
        case sb::Key::Q:
        case sb::Key::Z:
            speed.y = 0.f;
            break;
        case sb::Key::PrintScreen:
            {
#ifdef PLATFORM_WIN32
                SYSTEMTIME systime;
                ::GetSystemTime(&systime);
                std::string filename =
                        sb::utils::format("{0}.{1}.{2}.{3}.png",
                                          systime.wHour,
                                          systime.wMinute,
                                          systime.wSecond,
                                          systime.wMilliseconds);
#else // PLATFORM_LINUX
                timeval current;
                gettimeofday(&current, NULL);
                std::string filename =
                        sb::utils::format("{0}.{1}.png", current.tv_sec,
                                          (current.tv_usec / 1000));
#endif // PLATFORM_WIN32

                wnd.saveScreenshot(filename);
                break;
            }
        default:
            break;
        }
    }

    void handleMouseMoved(const sb::Event& e)
    {
        if (wnd.hasFocus())
        {
            Vec2i halfSize = wnd.getSize() / 2;
            int pixelsDtX = (int)e.data.mouse.x - halfSize.x;
            int pixelsDtY = (int)e.data.mouse.y - halfSize.y;

            static const float ROTATION_SPEED = 1.0f;
            Radians dtX = Radians(ROTATION_SPEED * (float)pixelsDtX / halfSize.x);
            Radians dtY = Radians(ROTATION_SPEED * (float)pixelsDtY / halfSize.y);

            wnd.getCamera().mouseLook(dtX, dtY);
        }
    }

    void handleInput()
    {
        // event handling
        sb::Event e;
        while (wnd.getEvent(e))
        {
            switch (e.type)
            {
            case sb::Event::MousePressed:
                handleMousePressed(e);
                break;
            case sb::Event::MouseReleased:
                handleMouseReleased(e);
                break;
            case sb::Event::KeyPressed:
                handleKeyPressed(e);
                break;
            case sb::Event::KeyReleased:
                handleKeyReleased(e);
                break;
            case sb::Event::MouseMoved:
                handleMouseMoved(e);
                break;
            case sb::Event::WindowFocus:
                if (e.data.focus)
                {
                    wnd.hideCursor();
                    wnd.lockCursor();
                }
                else
                {
                    wnd.hideCursor(false);
                    wnd.lockCursor(false);
                    speed = Vec3(0.0f, 0.0f, 0.0f);
                }
                break;
            case sb::Event::WindowResized:
                scene.crosshair.setScale(0.01f, 0.01f * ((float)e.data.wndResize.width / (float)e.data.wndResize.height), 0.01f);
                break;
            case sb::Event::WindowClosed:
                wnd.close();
                break;
            default:
                break;
            }
        }
    }
};

int main()
{
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

    Game game;
    gLog.info("entering main loop\n");

    sb::Timer clock;

    // main loop
    while (game.isRunning())
    {
        float delta = clock.getSecondsElapsed();
        clock.reset();

        game.handleInput();
        game.update(delta);
        game.draw();
    }

    gLog.info("window closed\n");
    return 0;
}
