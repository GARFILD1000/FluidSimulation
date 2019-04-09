//
// Created by Dmitriy on 20.11.2018.
//
#ifndef SCENE_CPP
#define SCENE_CPP

#include "Fluid2D.cpp"
#include <string>
#include <SFML/Graphics.hpp>
#include <sstream>
#include <cmath>

class World{
public:
    enum CellType **fluidcell;
    int xnum;
    int ynum;
    int pointx=-1;
    int pointy=-1;
};

enum DrawMethod
{
    DRAW_WATER,
    DRAW_WATERPARTICLES,
    DRAW_VELOCITY,
    DRAW_PRESSURE,
    DRAW_VELOCITYNUMBER,
    DRAW_VELOCITYVECTOR,
    DRAW_PRESSURENUMBER,
    DRAW_WATERFAST
};

class Scene {
public:

    bool created = false;
    double dh;
    double vectorscale;
//  public bool   byonestep;
    // public bool   readytostep;

    std::string filename;
    DrawMethod drawmethod;
    Fluid2D *fluid = new Fluid2D();
//  WaterPipe       pipe=new WaterPipe();
//  ForceRegion force = new ForceRegion();
//  ForceRegion forceact = new ForceRegion();
    int startx;
    int starty;
    int count = 0;
    bool showneed = false;
    bool showsurf = false;
    bool pause = false;

    Scene() {

//  pipe=NULL;
        dh = 4.0;
//  dh=8.0;
//  vectorscale=0.5;
        startx = 0;
        starty = 0;
        vectorscale = 20.0;
        // byonestep=false;
        // readytostep=true;
        drawmethod = DRAW_WATER;
        //drawmethod=DRAW_WATERPARTICLES;
//  drawmethod=DRAW_VELOCITYVECTOR;
//  drawmethod=DRAW_VELOCITYNUMBER;
        filename = "";
        // riverbegin=NULL;
        created = false;
    }

    void Create() {
        return;
    }

    void init(std::string filename_) {
        filename = filename_;
        if (created){
            fluid->ClearObstacles();
            fluid->ClearFluid();
        }
        if (!fluid->Create(filename))
            return;
        created = true;
    }
/*
    void SetInstrument(Instrument i) {
        if (i.showneed) {
            showneed = true;
            showsurf = false;
        } else if (i.showsurf) {
            showsurf = true;
            showneed = false;
        } else if (i.pause) {
            pause = !pause;
        }

    }
*/
    void SetDrawMethod(DrawMethod method) {
        drawmethod = method;
    }

//return fluid cell by coordinates of screen pixel
    void PointToCell(int x, int y, int &i, int &j) {
        int xnum = 0;
        int ynum = 0;
        fluid->GetDomainSize(xnum, ynum);
        x -= startx;
        y -= starty;
        i = (int) (x / dh);
        j = (int) (ynum - y / dh);
    }

// make fluid drop
    void AddFluidToPoint(sf::RenderWindow *window, int x, int y, int size) {
        int i = x;
        int j = y;


        sf::CircleShape circle;
        circle.setRadius(5);
        circle.setPosition(5, 5);
        circle.setFillColor(sf::Color(0, 255, 0));
        window->draw(circle);

        fluid->SETBCOND();
        int color = 100;

        for (int n = i; n < i + size; n++)
            for (int m = j; m < j + size; m++) {
                if (fluid->GetCellType(n, m) == CELL_EMPTY) {
                    /*sf::CircleShape circle;
                    circle.setRadius(5);
                    circle.setPosition(5, 5);
                    circle.setFillColor(sf::Color(0, 255, 0));
                    window->draw(circle);
                    */
                    fluid->MakeParticlesInCell(n, m, color);
                }
            }
    }

// process physic step of fluid


    void RunPhysicStep(bool onestep) {
        ///if (pause) {
        //    if (onestep == false)
        //        return;
        //}

//  static bool added=false;
        int xnum = 0, ynum = 0;
        fluid->GetDomainSize(xnum, ynum);
        //TimeUtil tu = TimeUtil.Instance();
        //tu.StartTimer(0, "all");

        //tu.StartTimer(1, "SETBCOND");
        fluid->SETBCOND();

        //tu.StopTimer(1);
        //tu.StartTimer(2, "RunPhysic");
        fluid->RunPhysic();
        //t->StopTimer(2);

        //tu.StopTimer(0);

    }


    void GetScene(World *world) {
        fluid->GetDomainSize(world->xnum, world->ynum);
        world->fluidcell = new CellType *[world->xnum];
        for (int i = 0; i < world->xnum; i++) {
            world->fluidcell[i] = new CellType[world->ynum];
        }
        for (int y = 0; y < world->ynum; y++)
            for (int x = 0; x < world->xnum; x++)
                world->fluidcell[x][y] = fluid->GetCellType(x, y);
        return;
    }

    void DrawCellInfo(sf::RenderWindow *window, World *world) {
        int y = 400 + 50 - 20;
        int x = 350;

        int xnum = 0;
        int ynum = 0;
        fluid->GetDomainSize(xnum, ynum);

        int i = 0;
        int j = 0;
        PointToCell(world->pointx, world->pointy, i, j);

        int cy = world->ynum - j - 1;
        int cellx1 = startx + (int) (dh * i);
        int cellx2 = (int) dh + 1;
        int celly1 = starty + (int) (dh * cy);
        int celly2 = (int) dh + 1;
        sf::RectangleShape rect;
        rect.setPosition(cellx1, celly1);
        rect.setSize(sf::Vector2f(cellx2 - cellx1, celly2 - celly1));
        window->draw(rect);


        sf::Font font;
        font.loadFromFile("CyrilicOld.otf");//передаем нашему шрифту файл шрифта
        sf::Text text("", font,
                      20);//создаем объект текст. закидываем в объект текст строку, шрифт, размер шрифта(в пикселях);//сам объект текст (не строка)
        text.setFillColor(sf::Color::Red);
        text.setStyle(sf::Text::Bold |
                      sf::Text::Underlined);//жирный и подчеркнутый текст. по умолчанию он "худой":)) и не подчеркнутый

        int n = 0;
        std::ostringstream stringStream;
        std::string subtype = "";
        if (fluid->IsEmpty(i, j))
            subtype = "Empty";
        stringStream << "i j=" << "\n" << "[ " << i << " ] [ " << j << " ]" << "\nFLAG=" << fluid->GetFlag(i, j)
                     << "\nTYPE=" << fluid->GetCellType(i, j) << "\nSUBTYPE=" << subtype;

        text.setPosition(x, y + n * 20);
        text.setString(stringStream.str());
        window->draw(text);
        text.setString(stringStream.str());
        text.setPosition(x + 150, y + n * 20);
        window->draw(text);

//  n++;
//  gdi.DrawString("U=", font, textBrush, x, y + n * 20);
//  gdi.DrawString(fluid.GetCellVelocityU(i, j).ToString(), font, textBrush, x + 150, y + n * 20);
//  n++;
//  gdi.DrawString("V=", font, textBrush, x, y + n * 20);
//  gdi.DrawString(fluid.GetCellVelocityV(i, j).ToString(), font, textBrush, x + 150, y + n * 20);
//  n++;
//  gdi.DrawString("P=", font, textBrush, x, y + n * 20);
//  gdi.DrawString(fluid.GetCellPressure(i, j).ToString(), font, textBrush, x + 150, y + n * 20);
//  n++;
//  gdi.DrawString("GX GY=", font, textBrush, x, y + n * 20);
//  gdi.DrawString(fluid.GetGX(i, j).ToString() + " " + fluid.GetGY(i, j).ToString(), font, textBrush, x + 150, y + n * 20);
//  n++;
//  gdi.DrawString("F G=", font, textBrush, x, y + n * 20);
//  gdi.DrawString(fluid.F[i][j].ToString() + " " + fluid.G[i][j].ToString(), font, textBrush, x + 150, y + n * 20);
    }

    int Isqrt(int num) {
        if (0 == num) { return 0; }  // Avoid zero divide
        int n = (num / 2) + 1;       // Initial estimate, never low
        int n1 = (n + (num / n)) / 2;
        while (n1 < n) {
            n = n1;
            n1 = (n + (num / n)) / 2;
        } // end while
        return n;
    }

    void RenderScene(sf::RenderWindow *window, World *world, int w, int h) {
        int cy;
        double u = 0, v = 0;
        double p = 0;
        RenderGrid(window, world, w, h);

        for (int y = 0; y < world->ynum; y++)
            for (int x = 0; x < world->xnum; x++) {
                cy = world->ynum - y - 1;
                int cellx1 = startx + (int) (dh * x);
                int cellx2 = (int) dh + 1;
                int celly1 = starty + (int) (dh * cy);
                int celly2 = (int) dh + 1;

                int f = fluid->GetFlag(x, y);
                //if ((f & GG.C_F) != 0 && (f & GG.C_E) == 0)
                //if ((f & GG.C_F) != 0 && (f < 0x0100))
                /*        if (fluid.IsEmpty(x, y))
                        {
                          gdi.FillRectangle(Brushes.Red, cellx1, celly1, cellx2, celly2);
                        }
                      if (fluid.IsFluid(x, y))
                        {
                          gdi.FillRectangle(Brushes.Red, cellx1, celly1, cellx2, celly2);
                        }
                 *   */
                /*
                if (fluid.IsInnerFluid(x, y))
                {
                  gdi.FillRectangle(Brushes.Yellow, cellx1, celly1, cellx2, celly2);
                  continue;
                }
                else if (fluid.IsSurface(x, y))
                {
                  gdi.FillRectangle(Brushes.SpringGreen, cellx1, celly1, cellx2, celly2);
                  continue;
                }
                */
                if (world->fluidcell[x][y] == CELL_FLUID) {
                    u = std::fabs(fluid->GetCellVelocityU(x, y));
                    v = std::fabs(fluid->GetCellVelocityV(x, y));
                    /*
                    if (v > u)
                      u = v;
                     */
                    u *= 1000;
                    v *= 1000;
                    int len = Isqrt((int) (u * u + v * v));
                    if (len > 250)
                        len = 250;
                    if (len < 0)
                        len = 0;
                    if (std::isnan(len))
                        len = 250;
                    int red = 0;
                    red = (int) (len / 1.3 + 5);
                    int blue = (int) (std::fabs(fluid->GetCellPressure(x, y)) * 90);

                    if (blue > 255)
                        blue = 255;
                    blue = 255 - blue;
                    //b = new SolidBrush(Color.FromArgb(red,0, blue));
                    //b = new SolidBrush(Color.FromArgb(red, 0, 220));
                    //b = new SolidBrush(Color.FromArgb(0, 0, blue));
                    sf::RectangleShape rect;
                    rect.setPosition(cellx1, celly1);
                    rect.setOutlineColor(sf::Color(100,0,0));
                    rect.setSize(sf::Vector2f(cellx2 - cellx1, celly2 - celly1));
                    window->draw(rect);
                }
                //else if (world.fluidcell[x, y] == Fluid2D.CellType.CELL_EMPTY)
                //{
                //    gdi.FillRectangle(Brushes.White, cellx1, celly1, cellx2, celly2);
                // }

            }

        if (world->pointx != -1) {
            DrawCellInfo(window, world);
        }


    }

    void RenderGrid(sf::RenderWindow *window, World *world, int w, int h) {
        for (int y = 0; y < world->ynum; y++)
            for (int x = 0; x < world->xnum; x++) {
                int xnum = 0;
                int ynum = 0;
                fluid->GetDomainSize(xnum, ynum);
                int cy;
                cy = world->ynum - y - 1;
                int cellx1 = startx + (int) (dh * x);
                int cellx2 = (int) dh + 1;
                int celly1 = starty + (int) (dh * cy);
                int celly2 = (int) dh + 1;

                sf::RectangleShape rect;
                rect.setPosition(cellx1, celly1);
                rect.setSize(sf::Vector2f(cellx2, celly2));

                //rect.setOutlineThickness(1);
                //int f = fluid->GetFlag(x, y);
                if (world->fluidcell[x][y] == CELL_OBSTACLE) {
                    //rect.setFillColor(sf::Color(0, 0, 0));
                    //window->draw(rect);
                    //rect.setOutlineColor(sf::Color(100, 100,100));
                }
                else if (world->fluidcell[x][y] == CELL_EMPTY) {
                    //rect.setOutlineColor(sf::Color(100,100,100));
                    //rect.setFillColor(sf::Color(100, 100, 100));
                }
                else if (world->fluidcell[x][y] == CELL_FLUID){
                    //rect.setOutlineColor(sf::Color(0, 0, 150));
                    rect.setFillColor(sf::Color(0, 100, 255));
                    window->draw(rect);

                }

            }

    }

    void RenderSceneParticles(sf::RenderWindow *window, World *world, int w, int h) {
        int count = fluid->particlecount;

        int xnum = 0;
        int ynum = 0;
        fluid->GetDomainSize(xnum, ynum);
        int cy;
        double u = 0, v = 0;
        double p = 0;

        RenderGrid(window, world, w, h);
        //RenderSceneFlag(window, world, w, h);
        double cellsizex = 0, cellsizey = 0;
        fluid->GetCellSize(cellsizex, cellsizey);
        double gridlenx = cellsizex * xnum;
        double gridleny = cellsizey * ynum;
        double screengridlenx = dh * xnum;
        double screengridleny = dh * ynum;
        double posscalex = screengridlenx / gridlenx;
        double posscaley = screengridleny / gridleny;
        /*
        for (int i = 0; i < fluid->particlearraysize; i++) {
            // if (fluid.particles[i] == null)
            //   continue;

            int px = (int) (fluid->particles[i].x * posscalex);
            int py = (int) (fluid->particles[i].y * posscaley);
            int cellx1 = startx + (int) (px);
            int celly1 = starty - (int) (0) + (int) (ynum * dh - py);
            sf::CircleShape circle;
            circle.setRadius(5);
            circle.setPosition(cellx1, celly1);
            circle.setFillColor(sf::Color(0, 0, 255));
            window->draw(circle);
        }
         */

    }

    void RenderSceneFlag(sf::RenderWindow *window, World *world, int w, int h) {
        int cy;
        double u = 0, v = 0;
        double p = 0;

        sf::RectangleShape rect;
        for (int y = 0; y < world->ynum; y++)
            for (int x = 0; x < world->xnum; x++) {
                cy = world->ynum - y - 1;
                int cellx1 = startx + (int) (dh * x);
                int cellx2 = (int) dh + 1;
                int celly1 = starty + (int) (dh * cy);
                int celly2 = (int) dh + 1;
                rect.setPosition(cellx1, celly1);
                rect.setSize(sf::Vector2f(cellx2, celly2));
                rect.setOutlineThickness(2);
                rect.setOutlineColor(sf::Color::Black);
                int f = fluid->GetFlag(x, y);
                if (world->fluidcell[x][y] == CELL_OBSTACLE)
                    rect.setFillColor(sf::Color(0, 0, 0));
                if (world->fluidcell[x][y] == CELL_FLUID) {
                    rect.setFillColor(sf::Color(0, 0, 100));
                } else if (world->fluidcell[x][y] == CELL_EMPTY) {
                    rect.setFillColor(sf::Color(100, 100, 100));
                }
                window->draw(rect);
            }


        for (int y = 0; y < world->ynum; y++)
            for (int x = 0; x < world->xnum; x++) {
                cy = world->ynum - y - 1;
                int cellx1 = startx + (int) (dh * x);
                int cellx2 = (int) dh + 1;
                int celly1 = starty + (int) (dh * cy);
                int celly2 = (int) dh + 1;
                rect.setSize(sf::Vector2f(cellx2 - cellx1, celly2 - celly1));
                rect.setPosition(cellx1, celly1);
                rect.setOutlineColor(sf::Color::Red);
                window->draw(rect);
                if (fluid->CALCFLAG != nullptr) {
                    sf::CircleShape circle;
                    int size = 8;
                    circle.setRadius(size);
                    circle.setPosition(cellx1, celly1);
                    circle.setFillColor(sf::Color(0, 0, 255));

                    struct CalcFlag cf = fluid->CALCFLAG[x][y];
                    if (showneed && cf.needu) {
                        circle.setFillColor(sf::Color(0, 0, 0));
                        circle.setPosition(cellx1 + cellx2 - size / 2, celly1 + celly2 / 2);
                        window->draw(circle);
                    }
                    if (showneed && cf.needv) {
                        circle.setFillColor(sf::Color(0, 0, 0));
                        circle.setPosition(cellx1 + cellx2 / 2, celly1 + celly2 - size / 2);
                        window->draw(circle);
                    }
                    if (showsurf && cf.surfu) {
                        circle.setFillColor(sf::Color(255, 0, 0));
                        circle.setPosition(cellx1 + cellx2 - size / 2, celly1 + celly2 / 2);
                        window->draw(circle);
                    }
                    if (showsurf && cf.surfv) {
                        circle.setFillColor(sf::Color(255, 0, 0));
                        circle.setPosition(cellx1 + cellx2 / 2, celly1 + celly2 - size / 2);
                        window->draw(circle);
                    }

                }
            }


        if (world->pointx != -1) {
            DrawCellInfo(window, world);
        }


    }


};


#endif