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
#include "simulation/simulation.h"
#include "simulation/ball.h"
#include <sstream>
#include <IL/ilut.h>

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

    sb::Line xaxis;
    sb::Line yaxis;
    sb::Line zaxis;

    sb::Sprite crosshair;
    sb::Model skybox;
    sb::Terrain terrain;

    sb::Light pointLight = sb::Light::point(Vec3(10.0, 10.0, 0.0), 100.0f);
    sb::Light parallelLight = sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 100.0f);

    Scene():
        colorShader(gResourceMgr.getShader("proj_basic.vert", "color.frag")),
        textureShader(gResourceMgr.getShader("proj_texture.vert", "texture.frag")),
        textureLightShader(gResourceMgr.getShader("proj_texture_normal.vert", "texture_normal.frag")),
        shadowShader(gResourceMgr.getShader("proj_shadow.vert", "shadow.frag")),
        xaxis(Vec3(1000.f, 0.f, 0.f),
              sb::Color::Red, colorShader),
        yaxis(Vec3(0.f, 1000.f, 0.f),
              sb::Color::Blue, colorShader),
        zaxis(Vec3(0.f, 0.f, 1000.f),
              sb::Color::Green, colorShader),
        crosshair("dot.png", textureShader),
        skybox("skybox.obj", textureShader,
               gResourceMgr.getTexture("miramar.jpg")),
        terrain("hmap_flat.jpg", "ground.jpg", shadowShader),
        pointLight(sb::Light::point(Vec3(10.0, 10.0, 0.0), 100.0f)),
        parallelLight(sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 100.0f))
    {
        crosshair.setPosition(0.f, 0.f, 0.f);
        crosshair.setScale(0.01f, 0.01f * 1.33f, 1.f);

        skybox.setScale(1000.f);

        terrain.setScale(10.f, 1.f, 10.f);
        terrain.setPosition(-640.f, 0.f, -640.f);
        terrain.setTexture("tex2", gResourceMgr.getTexture("blue_marble.jpg"));

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

    Sim::Simulation sim;
    Accumulator throwVelocity;
    Accumulator windVelocity;

    bool displayHelp;
    bool displaySimInfo;
    bool displayBallInfo;

    Game():
        wnd(1280, 1024),
        scene(),
        speed(0.0f, 0.0f, 0.0f),
        deltaTime(0.0f, 0.0f),
        fpsCounter(0.0f),
        fpsCurrValue(0.0f),
        fpsDeltaTime(0.0f, 0.0f),
        sim(Sim::Simulation::SimSingleThrow,
            scene.textureLightShader, scene.colorShader),
        throwVelocity(10.f, 0.5f),
        windVelocity(0.5f, 0.5f),
        displayHelp(false),
        displaySimInfo(false),
        displayBallInfo(false)
    {
        wnd.setTitle("Sandbox");
        wnd.lockCursor();
        wnd.hideCursor();
        wnd.getCamera().lookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

        sim.setThrowStart(Vec3d(0., 1., 0.), Vec3d(30., 30., 0.));

        deltaTime.reset();
        fpsDeltaTime.reset();
    }

    void drawStrings()
    {
        static const std::string helpString =
            "controls:\n"
            "wasdqz + mouse - moving\n"
            "123456 - camera presets\n"
            "esc - exit\n"
            "f1 - show/hide controls info\n"
            "f2 - show/hide simulation info\n"
            "f3 - show/hide ball info\n"
            "f4 - show/hide ball launcher lines\n"
            "f8 - exit + display debug info\n"
            "print screen - save screenshot\n"
            "p - pause simulation\n"
            "o - switch vector display mode (forces/accelerations)\n"
            "i - enable/disable auto-pause when a ball hits ground\n"
            "t - reset simulation (single ball)\n"
            "r - reset simulation (multiple balls)\n"
            "mouse left - set throw position/initial velocity*\n"
            "mouse right - set wind force*\n"
            "cf - decrease/increase slow motion factor\n"
            "vg - decrease/increase line width\n"
            "bh - decrease/increase air density\n"
            "nj - decrease/increase ball path length\n"
            "mk - decrease/increase ball spawn delay\n"
            ",l - decrease/increase ball limit by 5\n"
            ".; - decrease/increase ball mass**\n"
            "/' - decrease/increase ball radius**\n"
            "* hold button to adjust value\n"
            "** doesn't affect existing balls";
        static const uint32_t helpStringLines = 27u;

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

        wnd.drawString(sb::utils::format("throw velocity = {0}",
                                         throwVelocity.getValue()),
                       { 0.f, 0.f }, Sim::ColorThrow, nextLine++);
        wnd.drawString(sb::utils::format("wind velocity = {0}",
                                         windVelocity.getValue()),
                       { 0.f, 0.f }, Sim::ColorWind, nextLine++);

        if (displayHelp)
        {
            wnd.drawString(helpString, { 0.f, 0.0f },
                           sb::Color::White, ++nextLine);
            nextLine += helpStringLines;
        }
        if (displaySimInfo) {
            nextLine = sim.printParametersToScreen(wnd, { 0.f, 0.f }, ++nextLine);
        }
        if (displayBallInfo) {
            nextLine = sim.printBallParametersToScreen(
                    wnd, sim.raycast(wnd.getCamera().getEye(),
                                     wnd.getCamera().getFront().normalized()),
                    { 0.f, 0.0f }, nextLine);
        }

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

        // axes - disable edpth test to prevent blinking
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest, false);
        wnd.draw(scene.xaxis);
        wnd.draw(scene.yaxis);
        wnd.draw(scene.zaxis);
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest);

        // balls & forces
        sim.drawAll(wnd.getRenderer());

        // crosshair
        wnd.draw(scene.crosshair);

        drawStrings();

        wnd.display();
    }

    bool isRunning() {
        return wnd.isOpened();
    }

    void updateFixedStep(float timeStep)
    {
        sim.update(timeStep);

        throwVelocity.update();
        windVelocity.update();

        static Radians angle(0.0f);
        angle = Radians(angle.value() + 0.02f);
        scene.parallelLight.pos = Vec3(20.0f * std::sin(angle.value()), -20.0f, 20.0f * std::cos(angle.value()));
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
            {
                Vec3 pos = wnd.getCamera().getEye();
                if (pos.y < sim.getBallRadius()) {
                    pos.y = (float)sim.getBallRadius();
                }

                Vec3 v = wnd.getCamera().getFront().normalized()
                                        * throwVelocity.getValue();
                sim.setThrowStart(Vec3d(pos), Vec3d(v));
                sim.reset();
                throwVelocity.stop();
                break;
            }
            break;
        case sb::Mouse::ButtonRight:
            sim.setWind(Vec3d(wnd.getCamera().getFront().normalized()
                              * windVelocity.getValue()));
            windVelocity.stop();
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
        case sb::Key::N:
            sim.increaseBallPathLength(-1.0);
            break;
        case sb::Key::J:
            sim.increaseBallPathLength(1.0);
            break;
        case sb::Key::M:
            sim.increaseBallThrowDelay(-0.1f);
            break;
        case sb::Key::K:
            sim.increaseBallThrowDelay(0.1f);
            break;
        case sb::Key::B:
            sim.increaseAirDensity(-0.01);
            break;
        case sb::Key::H:
            sim.increaseAirDensity(0.01);
            break;
        case sb::Key::Comma:
            sim.increaseMaxBalls(-5);
            break;
        case sb::Key::L:
            sim.increaseMaxBalls(5);
            break;
        case sb::Key::A: speed.x = -SPEED; break;
        case sb::Key::D: speed.x = SPEED; break;
        case sb::Key::S: speed.z = -SPEED; break;
        case sb::Key::W: speed.z = SPEED; break;
        case sb::Key::Q: speed.y = SPEED; break;
        case sb::Key::Z: speed.y = -SPEED; break;
        case sb::Key::C:
            sim.increaseSloMoFactor(-0.1f);
            break;
        case sb::Key::F:
            sim.increaseSloMoFactor(0.1f);
            break;
        case sb::Key::Slash:
            sim.increaseBallRadius(-0.1f);
            break;
        case sb::Key::Apostrophe:
            sim.increaseBallRadius(0.1f);
            break;
        case sb::Key::Period:
            sim.increaseBallMass(-0.1f);
            break;
        case sb::Key::Colon:
            sim.increaseBallMass(0.1f);
            break;
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
        case sb::Key::P:
            sim.togglePause();
            break;
        case sb::Key::O:
            sim.toggleVectorDisplayType();
            break;
        case sb::Key::I:
            sim.togglePauseOnGroundHit();
            break;
        case sb::Key::R:
            sim = Sim::Simulation(Sim::Simulation::SimContiniousThrow,
                                  scene.textureShader, scene.colorShader);
            break;
        case sb::Key::T:
            sim = Sim::Simulation(Sim::Simulation::SimSingleThrow,
                                  scene.textureShader, scene.colorShader);
            break;
        case sb::Key::V:
            {
                GLfloat lineWidth = 0.f;
                GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
                if (lineWidth > 1.f) {
                    GL_CHECK(glLineWidth(lineWidth / 2.f));
                }
                break;
            }
        case sb::Key::G:
            {
                GLfloat lineWidth = 0.f;
                GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
                if (lineWidth < 9.f) {
                    GL_CHECK(glLineWidth(lineWidth * 2.f));
                }
                break;
            }
        case sb::Key::F1:
            displayHelp = !displayHelp;
            break;
        case sb::Key::F2:
            displaySimInfo = !displaySimInfo;
            break;
        case sb::Key::F3:
            displayBallInfo = !displayBallInfo;
            break;
        case sb::Key::F4:
            sim.toggleShowLauncherLines();
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

