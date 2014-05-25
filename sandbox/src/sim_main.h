#include "window/window.h"
#include "rendering/sprite.h"
#include "rendering/line.h"
#include "rendering/model.h"
#include "rendering/string.h"
#include "rendering/terrain.h"
#include "utils/logger.h"
#include "utils/stringUtils.h"
#include "utils/lib.h"
#include "utils/timer.h"
#include "simulation/simulation.h"
#include "simulation/ball.h"
#include <sstream>
#include <IL/ilut.h>

void wait_for_key()
{
    getchar();
}

class Accumulator
{
    float mAccumulator,
          mBase,
          mStep;
    bool mRunning;

public:
    Accumulator(float base, float step = 1.f): mAccumulator(0.f), mBase(base), mStep(step), mRunning(false) {}
    void reset() { mRunning = true; mAccumulator = mBase; }
    void stop() { mRunning = false; }
    void update() { if (mRunning) mAccumulator += mStep; }
    void update(float dt) { if (mRunning) mAccumulator += dt; }
    float getValue() const { return mAccumulator; }
    bool running() const { return mRunning; }
};

int main()
{
    using sb::utils::makeString;

    sb::Window wnd(1200, 900);

    auto colorShader = gResourceMgr.getShader("proj_basic.vert", "color.frag");
    auto textureShader = gResourceMgr.getShader("proj_texture.vert", "texture.frag");

    wnd.setTitle("Sandbox");
    wnd.lockCursor();
    wnd.hideCursor();

    Sim::Simulation sim(Sim::Simulation::SimSingleThrow,
                        textureShader,
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

    sb::Model skybox("skybox.obj", textureShader);
    skybox.setScale(1000.f);

    sb::Terrain terrain("hmap_flat.jpg", "ground.jpg", textureShader);
    terrain.setScale(10.f, 1.f, 10.f);
    terrain.setPosition(-640.f, 0.f, -640.f);

    gLog.info("all data loaded!\n");

    wnd.getCamera().lookAt(Vec3(5.f, 5.f, 20.f), Vec3(5.f, 5.f, 0.f));

    float moveSpeed = 0.f, strafeSpeed = 0.f, ascendSpeed = 0.f;
    const float SPEED = 0.5f;

    sb::Timer clock;
    float lastFrameTime = 0.f,
          physicsUpdateStep = 0.03f,
          fpsCounter = 0.f,        // how many frames have been rendered in fpsUpdateStep time?
          fpsUpdateStep = 1.f,    // update FPS-string every fpsUpdateStep seconds
          fpsCurrValue = 0.f;    // current FPS value

    Accumulator deltaTime(0.f, 0.f),
                fpsDeltaTime(0.f, 0.f),
                throwVelocity(10.f, 0.5f),
                windVelocity(0.5f, 0.5f);
    deltaTime.reset();
    fpsDeltaTime.reset();

    std::string fpsString;

    bool displayHelp = true, displaySimInfo = true, displayBallInfo = true;
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
                        if (!throwVelocity.running())
                            throwVelocity.reset();
                        break;
                    case sb::Mouse::ButtonRight:
                        if (!windVelocity.running())
                            windVelocity.reset();
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
                            if (pos[1] < sim.mBallRadius)
                                pos[1] = (float)sim.mBallRadius;

                            Vec3 v = glm::normalize(wnd.getCamera().getFront()) * throwVelocity.getValue();
                            sim.setThrowStart(Vec3d(pos[0], pos[1], pos[2]), Vec3d(v[0], v[1], v[2]));
                            sim.reset();
                            throwVelocity.stop();
                            break;
                        }
                        break;
                    case sb::Mouse::ButtonRight:
                        sim.setWind(Vec3d(glm::normalize(wnd.getCamera().getFront()) * windVelocity.getValue()));
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
                        wnd.getCamera().lookAt(Vec3(50.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num2:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(-50.f, 0.f, 0.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num3:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, 50.f, 0.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num4:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.0001f, -50.f, 0.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num5:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.f, 0.f, 50.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::Num6:
                        moveSpeed = strafeSpeed = 0.f;
                        wnd.getCamera().lookAt(Vec3(0.f, 0.f, -50.f), Vec3(0.f, 0.f, 0.f));
                        break;
                    case sb::Key::N:
                        sim.mBallPathLength = sb::math::clamp(sim.mBallPathLength - 1.0, 0.0, 1000.0);
                        break;
                    case sb::Key::J:
                        sim.mBallPathLength = sb::math::clamp(sim.mBallPathLength + 1.0, 0.0, 1000.0);
                        break;
                    case sb::Key::M:
                        sim.mBallThrowDelay -= 0.1f;
                        break;
                    case sb::Key::K:
                        sim.mBallThrowDelay += 0.1f;
                        break;
                    case sb::Key::B:
                        sim.mAirDensity -= 0.01;
                        break;
                    case sb::Key::H:
                        sim.mAirDensity += 0.01;
                        break;
                    case sb::Key::Comma:
                        if (sim.mSimType == Sim::Simulation::SimContiniousThrow && sim.mMaxBalls > 1u)
                            sim.mMaxBalls -= 5;
                        break;
                    case sb::Key::L:
                        if (sim.mSimType == Sim::Simulation::SimContiniousThrow)
                            sim.mMaxBalls += 5;
                        break;
                    case sb::Key::A: strafeSpeed = -SPEED; break;
                    case sb::Key::D: strafeSpeed = SPEED; break;
                    case sb::Key::S: moveSpeed = -SPEED; break;
                    case sb::Key::W: moveSpeed = SPEED; break;
                    case sb::Key::Q: ascendSpeed = SPEED; break;
                    case sb::Key::Z: ascendSpeed = -SPEED; break;
                    case sb::Key::C:
                        sim.mSloMoFactor -= 0.1f;
                        break;
                    case sb::Key::F:
                        sim.mSloMoFactor += 0.1f;
                        break;
                    case sb::Key::Slash:
                        sim.mBallRadius -= 0.1f;
                        break;
                    case sb::Key::Apostrophe:
                        sim.mBallRadius += 0.1f;
                        break;
                    case sb::Key::Period:
                        sim.mBallMass -= 0.1f;
                        break;
                    case sb::Key::Colon:
                        sim.mBallMass += 0.1f;
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
                        sim.mVectorDisplayType = (sim.mVectorDisplayType == Sim::Simulation::DisplayForce ? Sim::Simulation::DisplayAcceleration : Sim::Simulation::DisplayForce);
                        break;
                    case sb::Key::I:
                        sim.mPauseOnGroundHit = !sim.mPauseOnGroundHit;
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
                            if (lineWidth > 1.f)
                                GL_CHECK(glLineWidth(lineWidth / 2.f));
                            break;
                        }
                    case sb::Key::G:
                        {
                            GLfloat lineWidth = 0.f;
                            GL_CHECK(glGetFloatv(GL_LINE_WIDTH, &lineWidth));
                            if (lineWidth < 9.f)
                                GL_CHECK(glLineWidth(lineWidth * 2.f));
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
                        sim.mShowLauncherLines = !sim.mShowLauncherLines;
                        break;
                    case sb::Key::PrintScreen:
                        {
#ifdef PLATFORM_WIN32
                            SYSTEMTIME systime;
                            ::GetSystemTime(&systime);
                            std::string filename =
                                    makeString(systime.wHour, ".",
                                               systime.wMinute, ".",
                                               systime.wSecond, ".",
                                               systime.wMilliseconds, ".png");
#else // PLATFORM_LINUX
                            timeval current;
                            gettimeofday(&current, NULL);
                            std::string filename =
                                    makeString(current.tv_sec, ".",
                                               (current.tv_usec / 1000), ".png");
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
                        Degrees dtX = Degrees(((int)e.data.mouse.x - wnd.getSize().x / 2)
                                              / (float)(wnd.getSize().x / 2));
                        Degrees dtY = Degrees(((int)e.data.mouse.y - wnd.getSize().y / 2)
                                              / (float)(wnd.getSize().y / 2));

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

        // environment
        gLog.info("skybox\n");
        wnd.draw(skybox);
        gLog.info("terrain\n");
        wnd.draw(terrain);

        // axes - disable edpth test to prevent blinking
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest, false);
        gLog.info("axes\n");
        wnd.draw(xaxis);
        wnd.draw(yaxis);
        wnd.draw(zaxis);
        wnd.getRenderer().enableFeature(sb::Renderer::FeatureDepthTest);

        // balls & forces
        gLog.info("sim\n");
        sim.drawAll(wnd.getRenderer());

        // crosshair
        gLog.info("crosshair\n");
        wnd.draw(crosshair);

        gLog.info("strings\n");
        // info strings
        uint32_t nextLine = 0u;
        sb::String::print(fpsString, 0.f, 0.f,
                          (fpsCurrValue > 30.f
                              ? sb::Color::Green
                              : (fpsCurrValue > 20.f ? sb::Color::Yellow
                                                     : sb::Color::Red)),
                          nextLine++);
        sb::String::print(
                makeString("pos = ", wnd.getCamera().getEye(),
                           "\nfront = ", wnd.getCamera().getFront(),
                           "\nphi = ",
                           Degrees(wnd.getCamera().getHorizontalAngle()),
                           "\ntheta = ",
                           Degrees(wnd.getCamera().getVerticalAngle())),
                0.f, 0.f, sb::Color::White, nextLine);
        nextLine += 4;

        sb::String::print(makeString("throw velocity = ",
                                     throwVelocity.getValue()),
                           0.f, 0.f, Sim::ColorThrow, nextLine++);
        sb::String::print(makeString("wind velocity = ",
                                     windVelocity.getValue()),
                          0.f, 0.f, Sim::ColorWind, nextLine++);

        if (displayHelp)
        {
            sb::String::print(helpString,
                              0.f, 0.0f, sb::Color::White, ++nextLine);
            nextLine += helpStringLines;
        }
        if (displaySimInfo)
            nextLine = sim.printParametersToScreen(0.f, 0.f, ++nextLine);
        if (displayBallInfo)
            nextLine = sim.printBallParametersToScreen(
                    sim.raycast(wnd.getCamera().getEye(),
                                glm::normalize(wnd.getCamera().getFront())),
                    0.f, 0.0f, nextLine);

        wnd.display();
    }

    gLog.info("window closed\n");
    return 0;
}
