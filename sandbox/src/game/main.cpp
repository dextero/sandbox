#include <fenv.h>

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
#include "simulation/simulation.h"
#include "simulation/ball.h"
#include <sstream>
#include <IL/ilut.h>

class Accumulator
{
    float mAccumulator;
    float mBase;
    float mStep;
    bool mRunning;

public:
    Accumulator(float base,
                float step = 1.f):
        mAccumulator(0.f),
        mBase(base),
        mStep(step),
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
        }
    }
    void update(float dt)
    {
        if (mRunning) {
            mAccumulator += dt;
        }
    }

    void stop() { mRunning = false; }
    float getValue() const { return mAccumulator; }
    bool running() const { return mRunning; }
};

int main()
{
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

    sb::Window wnd(1280, 1024);

    auto colorShader = gResourceMgr.getShader("proj_basic.vert", "color.frag");
    auto textureShader = gResourceMgr.getShader("proj_texture.vert", "texture.frag");
    auto textureLightShader = gResourceMgr.getShader("proj_texture_normal.vert", "texture_normal.frag");
    auto shadowShader = gResourceMgr.getShader("proj_shadow.vert", "shadow.frag");

    wnd.setTitle("Sandbox");
    wnd.lockCursor();
    wnd.hideCursor();

    Sim::Simulation sim(Sim::Simulation::SimSingleThrow,
                        textureLightShader,
                        colorShader);
    sim.setThrowStart(Vec3d(0., 1., 0.), Vec3d(30., 30., 0.));

    sb::Sprite crosshair("dot.png", textureShader);
    crosshair.setPosition(0.f, 0.f, 0.f);
    crosshair.setScale(0.01f, 0.01f * 1.33f, 1.f);

    sb::Line xaxis(Vec3(1000.f, 0.f, 0.f),
                   sb::Color(0.f, 0.f, 1.f),
                   colorShader);
    sb::Line yaxis(Vec3(0.f, 1000.f, 0.f),
                   sb::Color(1.f, 0.f, 0.f),
                   colorShader);
    sb::Line zaxis(Vec3(0.f, 0.f, 1000.f),
                   sb::Color(0.f, 1.f, 0.f),
                   colorShader);

    sb::Model skybox("skybox.obj",
                     textureShader,
                     gResourceMgr.getTexture("miramar.jpg"));
    skybox.setScale(1000.f);

    sb::Terrain terrain("hmap_flat.jpg", "ground.jpg", shadowShader);
    terrain.setScale(10.f, 1.f, 10.f);
    terrain.setPosition(-640.f, 0.f, -640.f);

    gLog.info("all data loaded!\n");

    wnd.getCamera().lookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

    float moveSpeed = 0.f, strafeSpeed = 0.f, ascendSpeed = 0.f;
    const float SPEED = 0.5f;

    sb::Timer clock;
    float lastFrameTime = 0.f;
    float physicsUpdateStep = 0.03f;
    float fpsCounter = 0.f;      // how many frames have been rendered in fpsUpdateStep time?
    float fpsUpdateStep = 1.f;   // update FPS-string every fpsUpdateStep seconds
    float fpsCurrValue = 0.f;    // current FPS value

    Accumulator deltaTime(0.f, 0.f);
    Accumulator fpsDeltaTime(0.f, 0.f);
    Accumulator throwVelocity(10.f, 0.5f);
    Accumulator windVelocity(0.5f, 0.5f);

    deltaTime.reset();
    fpsDeltaTime.reset();

    std::string fpsString;

    bool displayHelp = true;
    bool displaySimInfo = true;
    bool displayBallInfo = true;

    const std::string helpString =
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
    const uint32_t helpStringLines = 27u;

    gLog.info("entering main loop\n");

    // main loop
    while(wnd.isOpened())
    {
        float delta = clock.getSecsElapsed() - lastFrameTime;
        deltaTime.update(delta);
        fpsDeltaTime.update(delta);
        ++fpsCounter;
        lastFrameTime = clock.getSecsElapsed();

        // FPS update
        if (fpsDeltaTime.getValue() >= fpsUpdateStep)
        {
            fpsCurrValue = fpsCounter / fpsUpdateStep;
            fpsString = "FPS = " + sb::utils::toString(fpsCurrValue);
            fpsDeltaTime.update(-fpsUpdateStep);
            fpsCounter = 0.f;
        }

        // event handling
        sb::Event e;
        while (wnd.getEvent(e))
        {
            switch (e.type)
            {
            case sb::Event::MousePressed:
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
                    break;
                }
            case sb::Event::MouseReleased:
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
                    break;
                }
            case sb::Event::KeyPressed:
                {
                    switch (e.data.key)
                    {
                    case sb::Key::Num1:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(50.f, 0.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num2:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(-50.f, 0.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num3:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, 50.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num4:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, -50.f, 0.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num5:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.f, 0.f, 50.f),
                                               Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num6:
                        moveSpeed = strafeSpeed = 0.f;
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
                    case sb::Key::A: strafeSpeed = -SPEED; break;
                    case sb::Key::D: strafeSpeed = SPEED; break;
                    case sb::Key::S: moveSpeed = -SPEED; break;
                    case sb::Key::W: moveSpeed = SPEED; break;
                    case sb::Key::Q: ascendSpeed = SPEED; break;
                    case sb::Key::Z: ascendSpeed = -SPEED; break;
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
                    break;
                }
            case sb::Event::KeyReleased:
                {
                    switch (e.data.key)
                    {
                    case sb::Key::A:
                    case sb::Key::D:
                        strafeSpeed = 0.f;
                        break;
                    case sb::Key::S:
                    case sb::Key::W:
                        moveSpeed = 0.f;
                        break;
                    case sb::Key::Q:
                    case sb::Key::Z:
                        ascendSpeed = 0.f;
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
                                              textureShader,
                                              colorShader);
                        break;
                    case sb::Key::T:
                        sim = Sim::Simulation(Sim::Simulation::SimSingleThrow,
                                              textureShader,
                                              colorShader);
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
                    break;
                }
            case sb::Event::MouseMoved:
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
                    break;
                }
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
                    moveSpeed = strafeSpeed = 0.f;
                }
                break;
            case sb::Event::WindowResized:
                crosshair.setScale(0.01f, 0.01f * ((float)e.data.wndResize.width / (float)e.data.wndResize.height), 0.01f);
                break;
            case sb::Event::WindowClosed:
                wnd.close();
                break;
            default:
                break;
            }
        }

        // physics update
        uint32_t guard = 3u;
        while ((deltaTime.getValue() >= physicsUpdateStep) && guard--)
        {
            sim.update(physicsUpdateStep);
            deltaTime.update(-physicsUpdateStep);

            throwVelocity.update();
            windVelocity.update();
        }

        // drawing
        wnd.getCamera().moveRelative(Vec3(strafeSpeed, ascendSpeed, moveSpeed));

        // move skybox, so player won't go out of it
        skybox.setPosition(wnd.getCamera().getEye());

        wnd.clear(sb::Color(0.f, 0.f, 0.5f));

        static sb::Light pointLight = sb::Light::point(Vec3(10.0, 10.0, 0.0), 100.0f);
        static sb::Light parallelLight = sb::Light::parallel(Vec3(5.0f, -10.0f, 5.0f), 100.0f);
        wnd.setAmbientLightColor(sb::Color(0.2f, 0.2f, 0.2f));
        wnd.addLight(pointLight);
        wnd.addLight(parallelLight);

        // environment
        wnd.draw(skybox);
        wnd.draw(terrain);

        // axes - disable edpth test to prevent blinking
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest, false);
        wnd.draw(xaxis);
        wnd.draw(yaxis);
        wnd.draw(zaxis);
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest);

        // balls & forces
        sim.drawAll(wnd.getRenderer());

        // crosshair
        wnd.draw(crosshair);

        // info strings
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

        wnd.display();
    }

    gLog.info("window closed\n");
    return 0;
}

