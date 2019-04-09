#ifndef FLUID2D_CPP
#define FLUID2D_CPP

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <iostream>

class GG{
        /* Macros for the integer array FLAG      */
        //                         OWSN
public:
    static const int C_B = 0x0000;//  0                      0000      /* interior obstacle cells                */public const int B_N = 0x0001;//  1                      0001  /* obstacle cells adjacent to fluid cells */
    static const int B_S = 0x0002;//  2                      0010  /* in the respective direction            */
    static const int B_W = 0x0004;//  4                      0100
    static const int B_NW = 0x0005;// 5                      0101
    static const int B_SW = 0x0006;// 6                      0110
    static const int B_O = 0x0008;//  8                      1000
    static const int B_NO = 0x0009;// 9                      1001
    static const int B_SO = 0x000a;// 10                     1010
    static const int C_F = 0x0010;//  16                   1 0000     /* fluid cell */
    static const int C_O = 0x0100;//  256   0 0000 0001 0000 0000
    static const int C_W = 0x0200;//  512   0 0000 0010 0000 0000             /* in the respective direction */
    static const int C_WO = 0x0300;// 768   0 0000 0011 0000 0000
    static const int C_S = 0x0400;//  1024  0 0000 0100 0000 0000              /* adjacent to empty cells     */
    static const int C_SO = 0x0500;// 1280  0 0000 0101 0000 0000
    static const int C_SW = 0x0600;// 1536  0 0000 0110 0000 0000
    static const int C_SWO = 0x0700;//1792  0 0000 0111 0000 0000
    static const int C_N = 0x0800;//  2048  0 0000 1000 0000 0000            /* free surface cells          */
    static const int C_NO = 0x0900;// 2304  0 0000 1001 0000 0000
    static const int C_NW = 0x0a00;// 2560  0 0000 1010 0000 0000
    static const int C_NWO = 0x0b00;//2816  0 0000 1011 0000 0000
    static const int C_NS = 0x0c00;// 3072  0 0000 1100 0000 0000
    static const int C_NSO = 0x0d00;//3328  0 0000 1101 0000 0000
    static const int C_NSW = 0x0e00;//3584  0 0000 1110 0000 0000
    static const int C_NSWO = 0x0f00;//3840 0 0000 1111 0000 0000
    static const int C_E = 0x1000;//  4096  0 0001 0000 0000 0000     /* empty cell                 */
};

enum CellType
{
    CELL_FLUID = 1,
    CELL_OBSTACLE = 2,
    CELL_EMPTY = 3,
    CELL_NULL = 4
};

struct Particle
{
    double x;
    double y;
    double u;
    double v;
    long color;
};

struct CalcFlag
{
    bool needu=false;
    bool needv = false;
    bool surfu = false;
    bool surfv = false;
};

class Fluid2D{
public:
    GG gg;

        // fluid particles for changing fluid geometry
        // not used if fluid occupies all cells(driven.geo for example) and there are no empty cells
    Particle *particles;
    int particlecount;
    int particlearraysize;
    //number particles in cell line
    int ppc = 2;
    int N;

    //.par file
    std::string InputFile;
    std::string *parlines;
    std::string *geolines;

    double xlength;
    double ylength;
        //time step size
    double delt = 0.001;
    double delx;
    double dely;
        // number of cells in x y directions
    int imax;
    int jmax;

        // initial fluid velocities
    double UI;
    double VI;
        // type of boundary - no-slip(velocity of water near obstacle=0 )
        // slip(velocity of water near obstacle=velocity of nearest cell )
        // inflow - constant velocity of fluid located in inflow region
    int wE;
    int wN;
    int wS;
    int wW;
    double tau;
    double gamma;
    double omg;
        // defines the type of fluid - high or low viscousity
    double Re;
    int itermax;
        // gravity or any other force for all fluid particles
    double GX;
    double GY;
        // right hand side of poisson equation
    double** RHS;
    double** F;
    double** G;
        // type of fluid cell
    int** FLAG;
    int** FLAGSURF;
        // velocity of fluid U[i][j] , i=0..imax+1
    double** U;
    double** V;
        // pressure of fluid
    double** P;

    CalcFlag** CALCFLAG;

    Fluid2D()
        {
            ppc = 2;
            delt = 0.001;
            //particles=NULL;
            particlecount = 0;
            CALCFLAG = nullptr;
        }

    void GetDomainSize(int &x,int &y)
        {
            // +2 - border boundary cells
            x=imax+2;
            y=jmax+2;
        }
    double GetStepSize()
        {
            return delt;
        }
    void GetCellSize(double &cellsizex, double &cellsizey)
        {
            cellsizex = delx;
            cellsizey = dely;
        }
    double GetU(int x, int y)
        {
            return (U[x][y] + U[x - 1][y]) / 2;
        }
    double GetV(int x, int y)
        {
            return (V[x][y] + V[x][y - 1]) / 2;
        }
    double GetCellVelocityU(int x, int y)
        {
            return U[x][y];
        }
    double GetCellVelocityV(int x, int y)
        {
            return V[x][y];
        }
    double GetCellPressure(int x, int y)
        {
            return P[x][y];
        }
    int GetFlag(int x, int y)
        {
            return FLAG[x][y];
        }
    enum CellType GetCellType(int x, int y){
            if (x >= imax + 2 || y >= jmax + 2) return CELL_NULL;
            if ((FLAG[x][y] & gg.C_E)!=0)
                return CELL_EMPTY;//empty
            else if ((FLAG[x][y] & gg.C_F)!=0)
                return CELL_FLUID;//fluid
            return CELL_OBSTACLE;//obstacle
        }
    void SetFlagEmpty(int x, int y)
        {
        if(x < imax+2 && y < jmax+2){
            FLAG[x][y] =  gg.C_E;
        }
        }
    void SetFlagFluid(int x, int y)
        {
        if(x < imax+2 && y < jmax+2){
            FLAG[x][y] = gg.C_F;
        }
        }
    void SetFlagObstacle(int x, int y)
        {
        if(x < imax+2 && y < jmax+2){
            FLAG[x][y] = gg.C_B;
        }
        }
    bool IsObstacle(int x, int y)
        {
        if(x <= imax+2 && y <= jmax+2){
            return FLAG[x][y] == gg.C_B;// < GG.C_F;
        }
        else return false;
        }
    bool IsFluid(int x, int y)
        {
        if(x <= imax+2 && y <= jmax+2){
            return FLAG[x][y] ==  gg.C_F;
        }
        else return false;
        }
    bool IsSurface(int x, int y)
        {
        if(x <= imax+2 && y <= jmax+2){
            return FLAGSURF[x][y] != 0;
        }
        else return false;
        }
    bool IsInnerFluid(int x, int y)
        {
        if(x <= imax+2 && y <= jmax+2){
            return IsFluid(x, y) && !IsSurface(x, y);
        }
        else return false;
        }
    bool IsEmpty(int x, int y)
        {
        if(x <= imax+2 && y <= jmax+2){
            return !IsFluid(x, y) && !IsObstacle(x, y);
        }
        else return false;
        }


    double convertToDouble(std::string s)
    {
        double x = atof(s.c_str());
        return x;
    }

        //    READ_PARAMETER reads the input parameters from "Inputfile".
    int READ_PARAMETER()
        {
            std::string tempString = InputFile;
            tempString.append(".geo");
            char *fileName = new char[tempString.size() + 1];
            strcpy(fileName, tempString.c_str());
            tempString.clear();
            std::fstream file;
            file.open(fileName);
            std::string str;
            int stringNumber = 0;
            while(!file.eof()) {
                std::getline(file,str);
                stringNumber++;
                std::cout << "Readed" << str << std::endl;
            }
            geolines = new std::string[stringNumber];
            file.clear();
            file.seekg(0);
            for (int i  = 0; i < stringNumber; i++) {
                getline(file,geolines[i]);
            }
            imax = geolines[0].length() - 2;
            jmax = stringNumber - 2;
            file.close();

            tempString = InputFile;
            tempString.append(".par");
            strcpy(fileName, tempString.c_str());
            file.open(fileName);
            stringNumber = 0;
            while(getline(file,str)) {
                stringNumber++;
            }
            parlines = new std::string[stringNumber];
            file.clear();
            file.seekg(0);

            for (int i  = 0; i < stringNumber; i++) {
                getline(file,parlines[i]);
            }
            file.close();
            for (int i = 0; i < stringNumber; i++)
            {
                int pos = parlines[i].find('\t');
                if(pos >= 0)
                    parlines[i] = parlines[i].substr(0, pos);

                pos = parlines[i].find(' ');
                if (pos >= 0)
                    parlines[i] = parlines[i].substr(0, pos);
                //pos = parlines[i].find('.');
                //while(pos >= 0) {
                    //parlines[i] = parlines[i].replace(pos, 1, ".");
                    //pos = parlines[i].find('.');
                //}
            }

            xlength=convertToDouble(parlines[1]);
            ylength=convertToDouble(parlines[2]);
            delt=convertToDouble(parlines[3]);
            tau=convertToDouble(parlines[4]);;
            N=(int)convertToDouble(parlines[5]);;
            itermax=(int)convertToDouble(parlines[6]);
            omg=convertToDouble(parlines[8]);
            gamma=convertToDouble(parlines[9]);
            Re=convertToDouble(parlines[10]);
            GX=convertToDouble(parlines[13]);
            GY=convertToDouble(parlines[14]);
            UI=convertToDouble(parlines[15]);
            VI=convertToDouble(parlines[16]);
            wW=(int)convertToDouble(parlines[17]);
            wE=(int)convertToDouble(parlines[18]);
            wN=(int)convertToDouble(parlines[19]);
            wS=(int)convertToDouble(parlines[20]);
            delete[](parlines);
            delete[](fileName);
            return 0;
        }

    void SETBCOND()
        {
            int i;
            int j;
            for (j = 0; j <= jmax + 1; j++)
            {
                U[0][j] = 0.0;
                V[0][j] = -V[1][j];

                U[imax+1][j] = 0.0;
                V[imax + 1][j] = -V[imax][j];
            }
            //  northern and southern boundary.
            for (i = 0; i <= imax + 1; i++)
            {
                U[i][0] = -U[i][1];
                V[i][0] = 0.0;

                U[i][jmax + 1] = -U[i][jmax];
                V[i][jmax ] = 0.0;
            }

            for (i = 1; i <= imax; i++)
                for (j = 1; j <= jmax; j++)
                {
                    if (IsObstacle(i, j))
                    {
                        if (IsFluid(i - 1, j))
                        {
                            U[i - 1][j] = 0.0;
                            V[i][j] = -V[i - 1][j];
                            V[i][j - 1] = -V[i - 1][j - 1];
                        }
                        else if (IsFluid(i + 1, j))
                        {
                            U[i][j] = 0.0;
                            V[i][j] = -V[i + 1][j];
                            V[i][j - 1] = -V[i + 1][j - 1];
                        }
                        else if (IsFluid(i, j - 1))
                        {
                            V[i][j - 1] = 0.0;
                            U[i][j] = -U[i][j - 1];
                            U[i - 1][j] = -U[i - 1][j - 1];
                        }
                        else if (IsFluid(i, j + 1))
                        {
                            V[i][j] = 0.0;
                            U[i][j] = -U[i][j + 1];
                            U[i - 1][j] = -U[i - 1][j + 1];
                        }
                    }
                }

        }


        // -------------------- FREE BOUNDARY AND PARTICLES ---------------------------------



    void MakeParticlesInCell(int i, int j, long color=1)
        {
            if(i > imax || j > jmax) return;
            if (particles == nullptr){
                particles = new Particle[1000];
                particlearraysize = 1000;
                particlecount = 0;
            }
            if (particlearraysize <= particlecount + ppc * 2)
            {
                int newsize = particlearraysize + 100;

                Particle *tmp = new Particle[newsize];
                for(int k = 0; k < particlearraysize; k++) {
                    tmp[k] = particles[k];
                }
                delete[](particles);
                particles=tmp;
                particlearraysize = newsize;
            }
            double x, y;
            for (int ip = 1; ip <= ppc; ip++)
            {
                x = i * delx + ((static_cast<double>(ip) - 0.5) / static_cast<double>(ppc)) * delx;
                for (int jp = 1; jp <= ppc; jp++)
                {
                    y = j * dely + ((static_cast<double>(jp) - 0.5) / (double)ppc) * dely;
                    particles[particlecount].x = x;
                    particles[particlecount].y = y;
                    particles[particlecount].color = color;
                    particlecount++;
                }
            }
            SetFlagFluid(i,j);
//    FLAG[i][j] = GG.C_F;
        }

        //    MARK_CELLS marks the cells of the fluid domain.
        //    SET_UVP_SURFACE sets the boundary values at a free surface.
        //    ADVANCE_PARTICLES updates the position of particles.

    void MARK()
        {
            int i, j;

            for (i = 1; i <= imax; i++)
                for (j = 1; j <= jmax; j++)
                {
                    if (!IsObstacle(i, j))
                        SetFlagEmpty(i, j);
                }

            //  Mark cells containing particles as fluid cells (loop over particles) .
            for (int k = 0; k < particlecount; k++)
            {
                i = static_cast<int>(particles[k].x / delx);
                j = static_cast<int>(particles[k].y / dely);
                if (!IsObstacle(i, j))
                    SetFlagFluid(i, j);
            }
        }

    //    ADVANCE_PARTICLES updates the position of particles.
    void ADVANCE_PARTICLES() {
        int i, j;
        double u, v;
        double x, y;
        double endx = (imax + 1) * delx;
        double endy = (jmax + 1) * dely;

        #pragma omp parallel for
        for (int k = 0; k < particlecount; k++) {
            x = particles[k].x;
            y = particles[k].y;
            i = static_cast<int>(x / delx);
            j = static_cast<int>(y / dely);

            u = U[i][j];
            v = V[i][j];

            x += delt * u;
            y += delt * v;
            //  Determine new cell for the particle.
            if (x < endx && y < endy && x >= 0 && y >= 0) {
                particles[k].x = x;
                particles[k].y = y;
                particles[k].u = u;
                particles[k].v = v;
            }

        }

        MARK();
        for (i = 0; i <= imax + 1; i++)
            for (j = 0; j <= jmax + 1; j++)
                FLAGSURF[i][j] = 0;
        #pragma omp parallel for
        for (j = 1; j <= jmax+1; j++){
            for (i = 1; i <= imax+1; i++){
                if (IsEmpty(i, j)){
                        U[i][j] = 0.0;
                        V[i][j] = 0.0;

                    //if ((FLAG[i][j] & GG.C_E) != 0 && (FLAG[i + 1][j] & GG.C_E) != 0)
                    U[i][j] = 0;
                    V[i][j] = 0;
                    P[i][j] = 0;
                    F[i][j] = 0;
                    G[i][j] = 0;
                    RHS[i][j] = 0;
                }
                else if (IsFluid(i, j)) {
                    if (IsEmpty(i - 1, j))
                        FLAGSURF[i][j] = FLAGSURF[i][j] | gg.C_W;
                    if (IsEmpty(i + 1, j))
                        FLAGSURF[i][j] = FLAGSURF[i][j] | gg.C_O;
                    if (IsEmpty(i, j - 1))
                        FLAGSURF[i][j] = FLAGSURF[i][j] | gg.C_S;
                    if (IsEmpty(i, j + 1))
                        FLAGSURF[i][j] = FLAGSURF[i][j] | gg.C_N;
                }
                if (IsSurface(i, j)) {
                    P[i][j] = 0.0;
                }
            }
        }


        //  Mark surface cells.



        //good
        for (j = 1; j <= jmax; j++)
            for (i = 1; i <= imax; i++) {
                //  treat only surface cells.
                if (FLAGSURF[i][j] == gg.C_S) {
                    U[i][j - 1] = U[i][j];
                    V[i][j - 1] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_N) {
                    V[i][j] = V[i][j - 1];
                    U[i][j + 1] = U[i][j];
                } else if (FLAGSURF[i][j] == gg.C_O) {
                    U[i][j] = U[i - 1][j];
                    V[i + 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_NO) {
                    V[i][j] = V[i][j - 1];
                    U[i][j] = U[i - 1][j];
                } else if (FLAGSURF[i][j] == gg.C_W) {
                    U[i - 1][j] = U[i][j];
                    V[i - 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_NW) {
                    V[i][j] = V[i][j - 1];
                    U[i - 1][j] = U[i][j];
                    V[i - 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_SO) {
                    U[i][j] = U[i - 1][j];
                    V[i][j - 1] = V[i][j];
                    V[i + 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_SW) {
                    U[i][j - 1] = U[i][j];
                    V[i][j - 1] = V[i][j];
                    U[i - 1][j] = U[i][j];
                    V[i - 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_NSW) {
                    V[i][j] = V[i + 1][j];
                    U[i][j + 1] = U[i][j];
                    U[i][j - 1] = U[i][j];
                    U[i - 1][j] = U[i][j];
                } else if (FLAGSURF[i][j] == gg.C_SWO) {
                    U[i][j] = U[i][j + 1];
                    V[i - 1][j] = V[i][j];
                    V[i + 1][j] = V[i][j];
                    V[i][j - 1] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_NSO) {
                    U[i][j] = U[i - 1][j];
                    V[i][j] = V[i - 1][j];
                } else if (FLAGSURF[i][j] == gg.C_NWO) {
                    U[i][j] = U[i][j - 1];
                    V[i][j] = V[i][j - 1];
                } else if (FLAGSURF[i][j] == gg.C_WO) {
                    U[i][j] = U[i][j - 1];
                    V[i - 1][j] = V[i][j];
                    V[i + 1][j] = V[i][j];
                } else if (FLAGSURF[i][j] == gg.C_NS) {
                    V[i][j] = V[i - 1][j];
                    U[i][j + 1] = U[i][j];
                    U[i][j - 1] = U[i][j];
                }
            }
/*
        for (j = 1; j <= jmax; j++)
            for (i = 1; i <= imax; i++) {
                //  treat only surface cells.
                if (FLAGSURF[i][j] == gg.C_S) {
                    V[i][j - 1] = CalcBound(i, j, i, j - 1, false);
                    U[i - 1][j - 1] = CalcBound(i, j, i - 1, j - 1, true);
                } else if (FLAGSURF[i][j] == gg.C_NO) {
                    U[i][j] = CalcBound(i, j, i, j, true);
                    V[i][j] = CalcBound(i, j, i, j, false);
                    U[i - 1][j + 1] = CalcBound(i, j, i - 1, j + 1, true);
                    U[i][j + 1] = CalcBound(i, j, i, j + 1, true);
                    V[i + 1][j] = CalcBound(i, j, i + 1, j, false);
                    V[i + 1][j - 1] = CalcBound(i, j, i + 1, j - 1, false);
                } else if (FLAGSURF[i][j] == gg.C_NW) {
                    U[i - 1][j] = CalcBound(i, j, i - 1, j, true);
                    V[i][j] = CalcBound(i, j, i, j, false);
                    U[i - 1][j + 1] = CalcBound(i, j, i - 1, j + 1, true);
                    V[i - 1][j] = CalcBound(i, j, i - 1, j, false);
                    V[i - 1][j - 1] = CalcBound(i, j, i - 1, j - 1, false);
                } else if (FLAGSURF[i][j] == gg.C_N) {
                    V[i][j] = CalcBound(i, j, i, j, false);
                    U[i - 1][j + 1] = CalcBound(i, j, i - 1, j + 1, true);
                } else if (FLAGSURF[i][j] == gg.C_O) {
                    U[i][j] = CalcBound(i, j, i, j, true);
                    V[i + 1][j - 1] = CalcBound(i, j, i + 1, j - 1, false);
                } else if (FLAGSURF[i][j] == gg.C_W) {
                    U[i - 1][j] = CalcBound(i, j, i - 1, j, true);
                    V[i - 1][j - 1] = CalcBound(i, j, i - 1, j - 1, false);
                } else if (FLAGSURF[i][j] == gg.C_SW) {
                    U[i - 1][j] = CalcBound(i, j, i - 1, j, true);
                    V[i][j - 1] = CalcBound(i, j, i, j - 1, false);
                    U[i - 1][j - 1] = CalcBound(i, j, i - 1, j - 1, true);
                    V[i - 1][j - 1] = CalcBound(i, j, i - 1, j - 1, false);
                }
                else if (FLAGSURF[i][j] == gg.C_SO){
                    U[i][j] = CalcBound(i, j, i, j, true);
                    V[i][j - 1] = CalcBound(i, j, i, j - 1, false);
                    U[i - 1][j - 1] = CalcBound(i, j, i, j, true);
                    U[i][j - 1] = CalcBound(i, j, i, j - 1, true);
                    V[i + 1][j - 1] = CalcBound(i, j, i + 1, j - 1, false);
                }
            }*/
        }

        double CalcBound(int curi, int curj, int calci, int calcj, bool calcU)
        {
            int i = calci;
            int j = calcj;
            //int neari=calci;
            //int nearj=calcj;
            double r;

            if ((i == curi && j == curj) || IsEmpty(i, j))
            {
                while (true)
                {
                    if (calcU)
                    {
                        if ((i - 1 >= curi - 1) && !IsEmpty(i - 1, j))
                        {
                            i--;
                            break;
                        }
                        if ((j - 1 >= curj - 1) && !IsEmpty(i, j - 1))
                        {
                            j--;
                            break;
                        }
                    }
                    else
                    {
                        if ((j - 1 >= curj - 1) && !IsEmpty(i, j - 1))
                        {
                            j--;
                            break;
                        }
                        if ((i - 1 >= curi - 1) && !IsEmpty(i - 1, j))
                        {
                            i--;
                            break;
                        }
                    }
                    if ((j + 1 <= curj + 1) && !IsEmpty(i, j + 1))
                    {
                        j++;
                        break;
                    }
                    if ((i + 1 <= curi + 1) && !IsEmpty(i + 1, j))
                    {
                        i++;
                        break;
                    }
                    if ((j - 1 >= curj - 1 && i - 1 >= curi - 1) && !IsEmpty(i - 1, j - 1))
                    {
                        i--;
                        j--;
                        break;
                    }
                    if ((j + 1 <= curj + 1 && i - 1 >= curi - 1) && !IsEmpty(i - 1, j + 1))
                    {
                        i--;
                        j++;
                        break;
                    }
                    if ((j + 1 <= curj + 1 && i + 1 <= curi + 1) && !IsEmpty(i + 1, j + 1))
                    {
                        i++;
                        j++;
                        break;
                    }
                    if ((j - 1 >= curj - 1 && i + 1 <= curi + 1) && !IsEmpty(i + 1, j - 1))
                    {
                        i++;
                        j--;
                        break;
                    }

                    break;
                }
            }
            if(calcU)
                r = U[i][j];
            else
                r = V[i][j];


            return r;

        }


        // -------------------- COMPUTE ---------------------------------
    double MixingDerivatives(double**D1, double** D2, int i, int j, int iadd, int jadd)
        {
            return (D1[i][j] * D2[i][j] - D1[i + iadd][j + jadd] * D2[i + iadd][j + jadd]);
        }

        //    COMP_FG computes the tentative velocity field (F,G).
    void COMP_FG()
        {
            int i, j;
            double DU2DX, DUVDY, DUVDX, DV2DY, LAPLU, LAPLV;
            //  F und G at external boundary.
            for (i = 0; i <= imax + 1; i++)
                for (j = 0; j <= jmax + 1; j++)
                {
                    F[i][j] = U[i][j];
                    G[i][j] = V[i][j];
                }
            #pragma omp parallel for
            for (i = 1; i <= imax - 1; i++)
                for (j = 1; j <= jmax; j++)
                {
                    if (IsFluid(i, j) && IsFluid(i+1, j))
                    {
                        //DU2DX = MixingDerivatives(U, U, i, j, -1, 0) / (4.0 * delx);
                        //DUVDY = MixingDerivatives(U, V, i, j, 0, -1) / (4.0 * delx);
                        DU2DX = 0;
                        DUVDY = 0;
                        LAPLU = (U[i + 1][j] + U[i - 1][j] - 2 * U[i][j]) / delx / delx +
                                (U[i][j + 1] + U[i][j - 1] - 2 * U[i][j]) / dely / dely;

                        F[i][j] = U[i][j] + delt * (LAPLU / Re - DU2DX - DUVDY + GX );
                    }
                }
            #pragma omp parallel for
            for (i = 1; i <= imax; i++)
                for (j = 1; j <= jmax - 1; j++)
                {
                    if (IsFluid(i, j) && IsFluid(i, j+1))
                    {
                        //DUVDX = MixingDerivatives(U, V, i, j, -1, 0) / (4.0 * delx);
                        //DV2DY = MixingDerivatives(V, V, i, j, 0, -1) / (4.0 * delx);
                        DUVDX = 0;
                        DV2DY = 0;
                        LAPLV = (V[i + 1][j] - 2 * V[i][j] + V[i - 1][j]) / delx / delx +
                                (V[i][j + 1] - 2 * V[i][j] + V[i][j - 1]) / dely / dely;

                        G[i][j] = V[i][j] + delt * (LAPLV / Re - DUVDX - DV2DY + GY );
                    }
                }

        }

        //    POISSON - solve Poisson equation for the pressure.
    int POISSON()
        {
            int i, j;

            //int flag = 0;
            double X = delx * delx;
            double Y = dely * dely;
            //double app = (1.0 / delt) * X * Y;
                //  copy values at external boundary
                for (i = 1; i <= imax; i++)
                {
                    P[i][0] = P[i][1];
                    P[i][jmax + 1] = P[i][jmax];
                }
                for (j = 1; j <= jmax; j++)
                {
                    P[0][j] = P[1][j];
                    P[imax + 1][j] = P[imax][j];
                }

                #pragma omp parallel for
                for (i = 1; i <= imax; i++)
                    for (j = 1; j <= jmax; j++)
                    {
                        if (IsObstacle(i, j))
                        {
                            if (IsFluid(i, j - 1))
                            {
                                P[i][j] = P[i][j - 1];
                            }
                            else if (IsFluid(i - 1, j))
                            {
                                P[i][j] = P[i - 1][j];
                            }
                            else if (IsFluid(i + 1, j))
                            {
                                P[i][j] = P[i + 1][j];
                            }
                            else if (IsFluid(i, j + 1))
                            {
                                P[i][j] = P[i][j + 1];
                            }

                            if (IsFluid(i - 1, j) && IsFluid(i, j+1))
                            {
                                P[i][j] = (P[i][j + 1]+P[i-1][j])/2;
                            }
                            if (IsFluid(i + 1, j) && IsFluid(i, j + 1))
                            {
                                P[i][j] = (P[i][j + 1] + P[i + 1][j]) / 2;
                            }
                            if (IsFluid(i - 1, j) && IsFluid(i, j - 1))
                            {
                                P[i][j] = (P[i][j - 1] + P[i - 1][j]) / 2;
                            }
                            if (IsFluid(i + 1, j) && IsFluid(i, j - 1))
                            {
                                P[i][j] = (P[i][j - 1] + P[i + 1][j]) / 2;
                            }
                            /*
                            if (IsObstacle(i - 1, j) && IsObstacle(i, j - 1) && IsFluid(i - 1, j - 1))
                            {
                              P[i][j] = P[i-1][j - 1];

                            }
                            */
                        }
                    }

                // gauss zeidel iteration - work with only 1 iteration !
                //double coeffij = 2 * (X + Y);
                #pragma omp parallel for
                for (i = 1; i <= imax; i++)
                    for (j = 1; j <= jmax; j++)
                        if (IsInnerFluid(i, j) )
                        {
                            P[i][j] = (
                                         -(P[i + 1][j] / (X) +
                                         P[i - 1][j] / (X) +
                                         P[i][j + 1] / (Y) +
                                         P[i][j - 1] / (Y)) +
                                         (1.0 / delt) *
                                         (
                                             (F[i][j] - F[i - 1][j]) / delx +
                                             (G[i][j] - G[i][j - 1]) / dely
                                         )
                                      )
                                      /(-2/X - 2/Y);
                        }
            return 0;
        }

        //    ADAP_UV computes the new velocity values.
    void ADAP_UV()
        {
            int i, j;

            for (i = 1; i <= imax - 1; i++)
                for (j = 1; j <= jmax; j++)
                {
                    if (IsFluid(i, j) && IsFluid(i + 1, j))
                        U[i][j] = F[i][j] -(P[i + 1][j] - P[i][j]) * delt / delx;
                }
            for (i = 1; i <= imax; i++)
                for (j = 1; j <= jmax - 1; j++)
                    if (IsFluid(i, j) && IsFluid(i, j + 1))
                        V[i][j] = G[i][j] -(P[i][j + 1] - P[i][j]) * delt / dely;
        }


    bool Create(std::string filename_)
        {
            InputFile = filename_;
            //  strcpy(Inputfile,filename_.c_str());
            if (READ_PARAMETER() != 0)
                return false;
            delx = xlength / imax;
            dely = ylength / jmax;
            int ia = 0;
            U = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                U[ia] = new double[jmax + 2];
            V = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                V[ia] = new double[jmax + 2];
            F = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                F[ia] = new double[jmax + 2];
            G = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                G[ia] = new double[jmax + 2];
            P = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                P[ia] = new double[jmax + 2];
            RHS = new double*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                RHS[ia] = new double[jmax + 2];
            FLAG = new int*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                FLAG[ia] = new int[jmax + 2];
            FLAGSURF = new int*[imax + 2];
            for (ia = 0; ia < imax + 2; ia++)
                FLAGSURF[ia] = new int[jmax + 2];
            InitFlag();
            //init uvp
            int i, j;
            for (i = 0; i <= imax + 1; i++)
                for (j = 0; j <= jmax + 1; j++)
                {
                    U[i][j] = UI;
                    V[i][j] = VI;
                    P[i][j] = 0;
                }
            //  INIT_FLAGS
            for (i = 0; i <= imax + 1; i++)
            {
                SetFlagObstacle(i,0);
                SetFlagObstacle(i, jmax + 1);
            }
            for (j = 1; j <= jmax; j++)
            {
                SetFlagObstacle(0,j);
                SetFlagObstacle(imax + 1,j);
            }
            particlearraysize = imax * jmax * 4;
            if (particlearraysize < 1000)
                particlearraysize = 1000;
            {
                particles = new Particle[particlearraysize];
            }
            particlecount = 0;
            std::string line;
            for (j = 1; j <= jmax; j++)
            {
                line = geolines[jmax - j + 1];
                for (i = 1; i <= imax; i++)
                {
                    if (line[i] == ('_'))
                        SetFlagEmpty(i,j);
//          FLAG[i][j] = GG.C_F | GG.C_E;
                    else if (line[i] == 'o')
                        SetFlagObstacle(i,j);
//          FLAG[i][j] = GG.C_B;
                    else if (line[i] == '=')
                        MakeParticlesInCell(i, j);
                    else
                        SetFlagEmpty(i, j);
                }
            }
    }

            bool CreateEmpty(int x, int y)
                {
                    InputFile = "defaultSettings";
                    if (READ_PARAMETER() != 0)
                        return false;
                    imax = x;
                    jmax = y;
                    delete(geolines);
                    delx = xlength / imax;
                    dely = ylength / jmax;
                    int ia = 0;
                    U = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        U[ia] = new double[jmax + 2];
                    V = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        V[ia] = new double[jmax + 2];
                    F = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        F[ia] = new double[jmax + 2];
                    G = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        G[ia] = new double[jmax + 2];
                    P = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        P[ia] = new double[jmax + 2];
                    RHS = new double*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        RHS[ia] = new double[jmax + 2];
                    FLAG = new int*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        FLAG[ia] = new int[jmax + 2];
                    FLAGSURF = new int*[imax + 2];
                    for (ia = 0; ia < imax + 2; ia++)
                        FLAGSURF[ia] = new int[jmax + 2];
                    InitFlag();
                    //init uvp
                    int i, j;
                    for (i = 0; i <= imax + 1; i++)
                        for (j = 0; j <= jmax + 1; j++)
                        {
                            U[i][j] = UI;
                            V[i][j] = VI;
                            P[i][j] = 0;
                        }
                    //  INIT_FLAGS
                    for (i = 0; i <= imax + 1; i++)
                    {
                        SetFlagObstacle(i,0);
                        SetFlagObstacle(i, jmax + 1);
                    }
                    for (j = 1; j <= jmax; j++)
                    {
                        SetFlagObstacle(0,j);
                        SetFlagObstacle(imax + 1,j);
                    }
                    particlearraysize = imax * jmax * 4;
                    if (particlearraysize < 1000)
                        particlearraysize = 1000;
                    {
                        particles = new Particle[particlearraysize];
                    }
                    particlecount = 0;
                    for (j = 1; j <= jmax; j++)
                    {
                        for (i = 1; i <= imax; i++)
                        {
                            SetFlagEmpty(i,j);
        //          FLAG[i][j] = GG.C_F | GG.C_E;
                        }
                    }


            return true;
        }
    void ClearObstacles()
        {
            for (int i  = 1; i <= imax; i++) {
                for (int j  = 1; j <= jmax; j++) {
                    SetFlagEmpty(i,j);
                }
            }
        }
    void ClearFluid()
        {
            particlearraysize = 0;
            particlecount = 0;
            delete[](particles);
            particles = nullptr;
        }
    void InitFlag()
        {
            CALCFLAG = new CalcFlag*[imax + 2];
            for (int i  = 0; i < imax + 2; i++) {
                CALCFLAG[i] = new CalcFlag[jmax + 2];
            }
        }

    void RunPhysic()
        {
            delt = 0.001;

            InitFlag();
            // timer4.SetBeginPeriod();
            if (N > 0)
                ADVANCE_PARTICLES();

            //  Compute tentative velocity field (F,G)
            COMP_FG();

            //  Compute right hand side for pressure equation
            //  Solve the pressure equation
            POISSON();

            //  Compute the new velocity field
            ADAP_UV();
        }


    };

#endif

