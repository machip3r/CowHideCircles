// main.cpp

#include <string>
#include <iostream>
#include <fstream>
#include "openGA.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <conio.h>



#define NUMBER_CIRCLES 25

using std::string;
using std::cout;
using std::endl;


int **imagen;
int nR = 720;
int nC = 720;
int radio = 50;

int hgMatrixDelete(int **M, int nR);
int **copyMatrix(int** mcpy, int r,int c);
bool validateCircle(int **img, int r,int c, int radio,int x,int y);
void drawCircleCenter(int **img, int r,int c, int radio,int x,int y);
int **hgMatrixNew(int R, int C);
int** hgLeerArchivo(char* filename, int* nRows, int* nCols);
void hgMostrarMatriz(int** matrix, int nRows, int nCols);
double getAreaMeanCircle(int **img, int r,int c, int radio,int x,int y);


struct MySolution
{
    int x[NUMBER_CIRCLES];
    int y[NUMBER_CIRCLES];
    int numCircles = NUMBER_CIRCLES;


    string to_string() const
    {
        string radios ;
        for(int i = 0; i < numCircles; i++) {
            radios.append("" + std::to_string(x[i]) + "," + std::to_string(y[i]) + ",");
        }

        return radios;
    }

};



struct MyMiddleCost
{

    double measureCircles;
};




typedef EA::Genetic<MySolution,MyMiddleCost> GA_Type;
typedef EA::GenerationType<MySolution,MyMiddleCost> Generation_Type;

void init_genes(MySolution& p,const std::function<double(void)> &rnd01)
{
    for(int i = 0; i < p.numCircles; i++){
        p.x[i] = 0.0 + nR*rnd01();
        p.y[i] = 0.0 + nC*rnd01();
    }

}


//Calcula la fitness desde aqui
//Esta fitness es para minimizar


int hgMatrixDelete(int **M, int nR)
{
    for (int r=0; r<nR; r++)
        delete[] M[r];
    delete[] M;
    return 0;
}


int **copyMatrix(int** mcpy, int r,int c){
    int ** mCpy = new int*[r];
    for(int i = 0; i < c; i++){
        mCpy[i] = new int [c];
    }

    for(int i =0; i < r; i++){
        for(int j = 0; j < c; j++ ){
            mCpy[i][j] = mcpy[i][j];
        }
    }
    return mCpy;
}


void drawCircleCenter(int **img, int r,int c, int radio,int x,int y){


    int xinf =  ((x-radio)>=0)?x-radio: 0;
    int yinf =  ((y-radio)>=0)?y-radio: 0;

    int xSup = ((x+radio)< r)? x+radio + 1 : r;
    int ySup = ((y+radio)< c)? y+radio + 1 : c;

    for( int i = xinf; i < xSup  ; i++){
        for(int j = yinf; j < ySup  ; j++){
            if(( pow( i - x,  2) + pow(j - y,2)) <= pow(radio,2))
                img[i][j] ++;
        }
    }
}
double getAreaMeanCircle(int **img, int r,int c, int radio,int x,int y){

    double sum = 0;
    double pixelCount = 0;
    int xinf =  ((x-radio)>=0)?x-radio: 0;
    int yinf =  ((y-radio)>=0)?y-radio: 0;

    int xSup = ((x+radio)< c)? x+radio +1: c;
    int ySup = ((y+radio)< r)? y+radio +1: r;

    for( int i = yinf; i < ySup; i++){
        for(int j = xinf; j < xSup; j++){
            if(( pow( j - x,  2) + pow(i - y,2)) <= pow(radio,2))
                sum += img[i][j] ;
                pixelCount++;

        }

    }
    return  sum/ pixelCount;
}
bool validateCircle(int **img, int r,int c, int radio,int x,int y){

    bool validate = true;
    int xinf =  ((x-radio)>=0)?x-radio: 0;
    int yinf =  ((y-radio)>=0)?y-radio: 0;

    int xSup = ((x+radio)< r)? x+radio + 1 : r;
    int ySup = ((y+radio)< c)? y+radio + 1: c;

    for( int i = xinf; i < xSup ; i++){
        for(int j = yinf; j < ySup ; j++){
            if(( pow( (i - x),  2) + pow((j - y),2)) <= pow(radio,2))
                if(img[i][j] != 1)
                    validate = false;

        }
    }
    return  validate;
}
bool eval_solution(const MySolution& p,MyMiddleCost &cc){
    cc.measureCircles = 0.0;

    int **mCpy = copyMatrix(imagen,nR,nC);

    for(int c = 0; c < p.numCircles; c++){

        int ** mCpyTemp = copyMatrix(mCpy,nR,nC);

        drawCircleCenter(mCpyTemp,nR,nC,radio,p.x[c],p.y[c]);
        if(validateCircle(mCpyTemp,nR,nC,radio,p.x[c],p.y[c])) {
            drawCircleCenter(mCpy,nR,nC,radio,p.x[c],p.y[c]);
            cc.measureCircles ++;
        }
        hgMatrixDelete(mCpyTemp,nR);
    }
    hgMatrixDelete(mCpy,nR);

    cc.measureCircles = pow(abs(p.numCircles-cc.measureCircles),2);

    return true; // solution is accepted
}

MySolution mutate(
        const MySolution& X_base,
        const std::function<double(void)> &rnd01,
        double shrink_scale)
{


    MySolution X_new;
    const double mu = 0.8*shrink_scale; // mutation radius (adjustable)
    bool in_range;

    for(int i = 0; i < X_new.numCircles; i++) {
        do {
            in_range = true;
            X_new = X_base;
            X_new.x[i] += mu * (rnd01() - rnd01());
            //Limites de la variable
            in_range = in_range && (X_new.x[i] >= 0.0 && X_new.x[i] < nR);
            //Mutacion
            X_new.y[i] += mu * (rnd01() - rnd01());
            in_range = in_range && (X_new.y[i] >= 0.0 && X_new.y[i] < nC);
        } while (!in_range);
    }
    return X_new;
}

MySolution crossover(
        const MySolution& X1,
        const MySolution& X2,
        const std::function<double(void)> &rnd01)
{
    MySolution X_new;
    double r;

    for(int i = 0; i < X1.numCircles; i++) {
        r=rnd01();
        X_new.x[i] = r * X1.x[i] + (1.0 - r) * X2.x[i];
        r = rnd01();
        X_new.y[i] = r * X1.y[i] + (1.0 - r) * X2.y[i];
    }

    return X_new;
}


//Solution fitness total

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType &X)
{
    // finalize the cost

    double final_cost=0;
    final_cost += X.middle_costs.measureCircles;
    return  final_cost;
}

std::ofstream output_file;

void SO_report_generation(
        int generation_number,
        const EA::GenerationType<MySolution,MyMiddleCost> &last_generation,
        const MySolution& best_genes)
{
    cout
            <<"Generation ["<<generation_number<<"], "
            <<"Exe_time="<<last_generation.exe_time
            <<endl;

    output_file
            <<generation_number<<"\t"
            <<last_generation.average_cost<<"\t"
            <<last_generation.best_total_cost<<"\t"
            <<best_genes.to_string()<<"\n";
}

int** hgLeerArchivo(char* filename, int* nRows, int* nCols) {
    FILE* localFile = fopen(filename, "r");

    if(localFile != NULL) {
        int rows = 0;
        int maxCols = 0;
        while(!feof(localFile)){
            char buffer[20024];
            int cols = 0;
            fscanf(localFile, "%s\n", buffer);
            char* token = strtok(buffer, ",");
            while(token != NULL) {
                token = strtok(NULL, ",");
                cols++;
            }
            if(maxCols < cols) maxCols = cols;
            rows++;
        }
        int **data = hgMatrixNew(rows, maxCols );
        fseek(localFile, 0L, SEEK_SET);
        *nRows = rows;
        *nCols = maxCols;
        for(int i = 0; i < rows; i++){
            char buffer[20024];
            fscanf(localFile, "%s\n", buffer);
            char *token = strtok(buffer, ",");
            for(int j = 0; j < maxCols; j++){
                if(token != NULL) data[i][j] = atoi(token);
                else data[i][j] = 0;
                token = strtok(nullptr, ",");
            }
        }
        fclose(localFile);
        return data;
    }

    return nullptr;
}

int **hgMatrixNew(int R, int C)
{
    int **M;
    M = new int *[R];

    for (int k=0; k<R; k++)
        M[k] = new int [C];

    for (int r=0; r<R; r++)
        for (int c=0; c<C; c++)
            M[r][c] = 0;

    return M;
}

void hgMostrarMatriz(int** matrix, int nRows, int nCols) {
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++)
            printf("%d ", matrix[i][j]);
        cout << endl;
    }
}

int main()
{
    output_file.open("results.txt");
    output_file<<"step"<<"\t"<<"cost_avg"<<"\t"<<"cost_best"<<"\t"<<"solution_best"<<"\n";
    imagen = hgLeerArchivo("imagen.csv",&nR,&nC);
    //hgMostrarMatriz(imagen,nR,nC);
    EA::Chronometer timer;
    timer.tic();
    GA_Type ga_obj;
    ga_obj.problem_mode=EA::GA_MODE::SOGA;
    ga_obj.multi_threading= true;
    ga_obj.verbose=false;
    ga_obj.population=25;
    ga_obj.generation_max=1000;
    ga_obj.calculate_SO_total_fitness=calculate_SO_total_fitness;
    ga_obj.init_genes=init_genes;
    ga_obj.eval_solution=eval_solution;
    ga_obj.mutate=mutate;
    ga_obj.crossover=crossover;
    ga_obj.SO_report_generation=SO_report_generation;
    ga_obj.crossover_fraction=0.7;
    ga_obj.mutation_rate=0.2;
    ga_obj.best_stall_max= 10 ;
    ga_obj.elite_count = 5;
    ga_obj.dynamic_threading = true;
    ga_obj.use_quick_sort = true;
    ga_obj.solve();
    cout<<"The problem is optimized in "<<timer.toc()<<" seconds."<<endl;

    output_file.close();
    return 0;
}