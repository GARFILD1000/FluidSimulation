#include <SFML/Graphics.hpp>
#include "Scene.cpp"

bool run;
bool isdraw;
//NanoTimer *timers;
bool needinit;
bool needclear;
bool stopped;
int curcycl;
int pointx;
int pointy;

void initAllParameters(Scene *scene)
{
    scene->init("Demos//bigmap");
    scene->dh = 5;
    scene->vectorscale = 15;
    needclear = true;

    run = true;
    isdraw = false;
    //sf::RenderWindow *window;
    needinit = true;
    needclear = false;
    stopped = false;
    curcycl = 0;
    pointx=-1;
    pointy=-1;
}
/*
void ReceiveWorkerThread_DoWork(object sender, DoWorkEventArgs e) {
    for (int i = 0;; i++) {
        if (!run) {
            Thread.Sleep(15);
            continue;
        }

        if (needinit && param.reinit) {
            scene.init("Demos\\" + file);
            scene.dh = 10;
            scene.vectorscale = 33;
            needclear = true;
        }

        if (needinit) {
            needinit = false;
        }
        if (instrument.changeinstrument) {
            instrument.changeinstrument = false;
            scene.SetInstrument(instrument);
        }
        if (instrument.step) {
            instrument.step = false;
            scene.RunPhysicStep(true);
            ThreadParam p = new ThreadParam();
            p.world = scene.GetScene();
            p.timers = TimeUtil.Instance().GetTimers();
            ReceiveWorkerThread.ReportProgress(0, p);
        }

        if (instrument.save) {
            instrument.save = false;
            instrument.none = true;
            instrument.changeinstrument = false;
        }
        if (instrument.mouseDown) {
            if (instrument.drop) {
                scene.AddFluidToPoint(instrument.mousex, instrument.mousey, 10);
            }
        }

        if (!stopped)
            scene.RunPhysicStep(false);

        if (ReceiveWorkerThread.CancellationPending == true) {
            break;
        }
        if (i % 30 == 0 && !stopped) {
            //isdraw = true;
//          World world2 = scene.GetScene();
//          timers
            ThreadParam p = new ThreadParam();
            p.world = scene.GetScene();
            p.timers = TimeUtil.Instance().GetTimers();
            ReceiveWorkerThread.ReportProgress(0, p);

            if (i % 10 == 0)
                Thread.Sleep(18);
        }

    }
}
void ReceiveWorkerThread_ProgressChanged()
{
    if (!run)
        return;
    ThreadParam p = (ThreadParam)e.UserState;
    world=p.world;
    timers = p.timers;
    Draw(g);
    //g.Clear(Color.White);
    //Invalidate();
    //panel1.Invalidate();
    //      scene.RenderScene(g, world, this.ClientRectangle.Width, this.ClientRectangle.Height);
    isdraw = false;
    //e.Graphics.Clear(Color.White);
    //    scene.Render(e.Graphics, this.ClientRectangle.Width, this.ClientRectangle.Height);
}

void Draw(sf::RenderWindow *window)
{
    if (window != nullptr && world != nullptr)
    {
        world.pointx=pointx;
        world.pointy=pointy;
        window->clear(sf::Color(0,0,200));
        //scene.RenderSceneSmooth(gr, world, this.ClientRectangle.Width, this.ClientRectangle.Height,1);
        scene.RenderSceneParticles(window, world, this.ClientRectangle.Width, this.ClientRectangle.Height);
        //scene.RenderScene(gr, world, this.ClientRectangle.Width, this.ClientRectangle.Height);
        //scene.RenderSceneFlag(gr, world, this.ClientRectangle.Width, this.ClientRectangle.Height);

        //scene.dh = 10;

        //scene.RenderSmoothParticles(gr, world, this.ClientRectangle.Width, this.ClientRectangle.Height);
        //if (curcycl > 10)
        //{
        //  curcycl = 0;
        //scene.RenderTimers(gr, world, timers, this.ClientRectangle.Width, this.ClientRectangle.Height);
        //}
        //curcycl++;
    }
}
*/


int main()
{
    // create the window




    Scene *scene = new Scene();
    World *world = new World();
    int iterations = 0;
    initAllParameters(scene);
    int windowW = 1, windowH = 1;
    scene->fluid->GetDomainSize(windowW, windowH);
    windowW *= static_cast<int>(scene->dh);
    windowH *= static_cast<int>(scene->dh);
    sf::RenderWindow *window = new sf::RenderWindow(sf::VideoMode(windowW, windowH), "Fluid Dynamics 2D");
    bool mouseLeftPressed = false;
    while (window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
            }
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.key.code == sf::Mouse::Left) {
                    mouseLeftPressed = false;
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.key.code == sf::Mouse::Left) {
                    mouseLeftPressed = true;
                }
                if (event.key.code == sf::Mouse::Right) {
                    scene->fluid->GY = -scene->fluid->GY;
                }
            }
            if (mouseLeftPressed) {
                sf::Vector2f mousePosition = window->mapPixelToCoords(sf::Mouse::getPosition(*((sf::Window *) window)));
                int i;
                int j;
                scene->PointToCell(mousePosition.x, mousePosition.y, i, j);
                std::cout << i << ": " << j << std::endl;
                if (!scene->fluid->IsObstacle(i, j)) {
                    scene->AddFluidToPoint(window, i, j, 10);
                }
            }

        }


        world->pointx = pointx;
        world->pointy = pointy;

        iterations++;
        scene->RunPhysicStep(true);

        if (iterations % 25 == 0) {
            window->clear(sf::Color(255, 255, 255));
            scene->GetScene(world);
            scene->RenderGrid(window, world, window->getSize().x, window->getSize().y);
            //scene->RenderScene(window, world, window->getSize().x, window->getSize().y);
            window->display();
        }
        if (iterations % 25 == 0) {
            //scene->AddFluidToPoint(window,30,60,10);
            iterations = 0;
        }
        //system("PAUSE");

    }
    return 0;
}