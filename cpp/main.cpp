#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <conio.h>
#include "openGA.hpp"

#define RADIO 50
#define NR 720
#define NC 720

using std::string;
using std::cout;
using std::endl;

std::ofstream output_file;
std::ofstream circles_file;

int** image;
int NCIRCLES = 20;

struct MySolution {
    int* x;
    int* y;

    MySolution() {
        x = new int[NCIRCLES];
        y = new int[NCIRCLES];
    }

    string to_string() const {
        string radios;

        for(int i = 0; i < NCIRCLES; i++)
            radios.append("" + std::to_string(x[i]) + "," + std::to_string(y[i]) + ",");

        return radios;
    }

};

struct MyMiddleCost {
    double fitness;
};

typedef EA::Genetic<MySolution, MyMiddleCost> GA_Type;
typedef EA::GenerationType<MySolution, MyMiddleCost> Generation_Type;

int** createMatrix(int, int);
int** copyMatrix(int**, int,int);
int deleteMatrix(int**, int);
void showMatrix(int**, int, int);
int** getMatrixFromFile(char*, int, int);
void exportBestCircles(int*, int);

void drawCirclesInCenter(int**, int, int, int, int, int);
bool validateCircle(int**, int, int, int, int, int);
int getAreaMeanCircle(int**, int, int, int, int, int);

void init_genes(MySolution&, const std::function<double(void)>&);
bool eval_solution(const MySolution&, MyMiddleCost&);
MySolution mutate(const MySolution&, const std::function<double(void)>&, double);
MySolution crossover(const MySolution&, const MySolution&, const std::function<double(void)>&);
double calculate_SO_total_fitness(const GA_Type::thisChromosomeType&);
void SO_report_generation(int, const EA::GenerationType<MySolution, MyMiddleCost>&, const MySolution&);

int main() {
    circles_file.open("circles.txt");
    output_file.open("results.txt");
    output_file << "step" << "\t" << "cost_avg" << "\t" << "cost_best" << "\t" << "solution_best" << "\n";

    image = getMatrixFromFile((char*)"piel.csv", NR, NC);

    EA::Chronometer timer;
    timer.tic();

    GA_Type ga_obj;
    ga_obj.problem_mode = EA::GA_MODE::SOGA;
    ga_obj.multi_threading = false;
    ga_obj.verbose = false;
    ga_obj.population = 100;
    ga_obj.generation_max = 25;
    ga_obj.calculate_SO_total_fitness = calculate_SO_total_fitness;
    ga_obj.init_genes = init_genes;
    ga_obj.eval_solution = eval_solution;
    ga_obj.mutate = mutate;
    ga_obj.crossover = crossover;
    ga_obj.SO_report_generation = SO_report_generation;
    ga_obj.crossover_fraction = 1;
    ga_obj.mutation_rate = 1;
    // Mean
    /* ga_obj.best_stall_max = 5; */
    // Circles
    ga_obj.best_stall_max = 10;
    ga_obj.elite_count = 3;
    ga_obj.solve();

    cout << "The problem is optimized in " << timer.toc() << " seconds." << endl;

    circles_file.close();
    output_file.close();

    deleteMatrix(image, NR);

    return 0;
}

void init_genes(MySolution& p, const std::function<double(void)>& rnd01) {
    for(int i = 0; i < NCIRCLES; i++){
        p.x[i] = NC * rnd01();
        p.y[i] = NR * rnd01();
    }
}

// Fit Mac 1
/* bool eval_solution(const MySolution& p, MyMiddleCost& cc) {
    cc.fitness = 0.0;

    int** matrixCopy = copyMatrix(image, NR, NC);

    for(int i = 0; i < NCIRCLES; i++)
        cc.fitness += getAreaMeanCircle(matrixCopy, NR, NC, RADIO, p.x[i], p.y[i]);

    deleteMatrix(matrixCopy, NR);

    return true;
} */

// Fit Chikis 1 && Mac 2
bool eval_solution(const MySolution& p, MyMiddleCost& cc) {
    cc.fitness = 0.0;

    int **matrixCopy = copyMatrix(image, NR, NC), *pointList = new int[(NCIRCLES * 2)];

    for(int i = 0; i < NCIRCLES; i++) {
        int** matrixCopyTmp = copyMatrix(matrixCopy, NR, NC);

        drawCirclesInCenter(matrixCopyTmp, NR, NC, RADIO, p.x[i], p.y[i]);

        if(validateCircle(matrixCopyTmp, NR, NC, RADIO, p.x[i], p.y[i])) {
        // if(getAreaMeanCircle(matrixCopyTmp, NR, NC, RADIO, p.x[i], p.y[i]) == 1) {
        // if(validateCircle(matrixCopyTmp, NR, NC, RADIO, p.x[i], p.y[i]) && getAreaMeanCircle(matrixCopyTmp, NR, NC, RADIO, p.x[i], p.y[i]) == 1) {
            drawCirclesInCenter(matrixCopy, NR, NC, RADIO, p.x[i], p.y[i]);
            pointList[(i * 2)] = p.x[i];
            pointList[(i * 2) + 1] = p.y[i];
            cc.fitness++;
        }

        deleteMatrix(matrixCopyTmp, NR);
    }

    exportBestCircles(pointList, NCIRCLES);

    delete[] pointList;
    deleteMatrix(matrixCopy, NR);

    cc.fitness = (1 / cc.fitness);
    // cc.fitness = pow(abs(NCIRCLES - cc.fitness), 2);

    return true;
}

MySolution crossover(const MySolution& X1, const MySolution& X2, const std::function<double(void)>& rnd01) {
    MySolution X_new;
    double r;

    for(int i = 0; i < NCIRCLES; i++) {
        r = rnd01();
        X_new.x[i] = r * X1.x[i] + (1.0 - r) * X2.x[i];
        r = rnd01();
        X_new.y[i] = r * X1.y[i] + (1.0 - r) * X2.y[i];
    }

    return X_new;
}

MySolution mutate(const MySolution& X_base, const std::function<double(void)>& rnd01, double shrink_scale) {
    MySolution X_new;
    const double mu = 0.5 * shrink_scale; // mutation radius (adjustable)
    bool in_range;

    for(int i = 0; i < NCIRCLES; i++)
        do {
            in_range = true;
            X_new = X_base;
            X_new.x[i] += mu * (rnd01() - rnd01());

            in_range = in_range && (X_new.x[i] >= 0.0 && X_new.x[i] < NC);

            X_new.y[i] += mu * (rnd01() - rnd01());
            in_range = in_range && (X_new.y[i] >= 0.0 && X_new.y[i] < NR);
        } while (!in_range);


    return X_new;
}

double calculate_SO_total_fitness(const GA_Type::thisChromosomeType& X) {
    double final_cost = 0;

    final_cost += X.middle_costs.fitness;

    return final_cost;
}

void SO_report_generation(int generation_number, const EA::GenerationType<MySolution, MyMiddleCost>& last_generation, const MySolution& best_genes) {
    cout << "Generation [" << generation_number << "]" << endl << "Exe_time = " << last_generation.exe_time << endl << "Fitness: " << last_generation.best_total_cost << endl;

    output_file << generation_number << "\t" << last_generation.average_cost << "\t"
    << last_generation.best_total_cost << "\t" << best_genes.to_string() << "\n";
}

int** createMatrix(int nRows, int nCols) {
    int** matrix;
    matrix = new int*[nRows];

    for (int i = 0; i < nRows; i++)
        matrix[i] = new int[nCols];

    for (int i = 0; i < nRows; i++)
        for (int j = 0; j < nCols; j++)
            matrix[i][j] = 0;

    return matrix;
}

int** copyMatrix(int** matrix, int nRows, int nCols) {
    int** matrixCopy = new int*[nRows];

    for(int i = 0; i < nCols; i++)
        matrixCopy[i] = new int[nCols];

    for(int i =0; i < nRows; i++)
        for(int j = 0; j < nCols; j++ )
            matrixCopy[i][j] = matrix[i][j];

    return matrixCopy;
}

int deleteMatrix(int** matrix, int nRows) {
    for(int r = 0; r < nRows; r++)
        delete[] matrix[r];
    delete[] matrix;
    return 0;
}

void showMatrix(int** matrix, int nRows, int nCols) {
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++)
            printf("%d ", matrix[i][j]);
        cout << endl;
    }
}

int** getMatrixFromFile(char* filename, int nRows, int nCols) {
    FILE* localFile = fopen(filename, "r");

    if(localFile != NULL) {
        int** data = createMatrix(nRows, nCols);

        fseek(localFile, 0L, SEEK_SET);

        for(int i = 0; i < nRows; i++){
            char buffer[20024];
            fscanf(localFile, "%s\n", buffer);
            char* token = strtok(buffer, ",");

            for(int j = 0; j < nCols; j++) {
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

void exportBestCircles(int* array, int nCircles) {
    circles_file << "[";

    for (int i = 0; i < (nCircles * 2); i++)
        if(i != ((nCircles * 2) - 1))
            circles_file << array[i] << ", ";
        else circles_file << array[i];

    circles_file << "]\n";
}

void drawCirclesInCenter(int** matrix, int nRows, int nCols, int radio, int x, int y) {
    if(x > NR || y > NC || x < 0 || y < 0) return;

    // int maxX = ((x - radio) >= 0) ? x - radio : 0, maxY = ((y - radio) >= 0) ? y - radio : 0, minX = ((x + radio) < nCols)? x + radio + 1 : nCols, minY = ((y + radio) < nRows)? y + radio + 1 : nRows;

    for(int i = 0; i < nRows; i++)
        for(int j = 0; j < nCols; j++)
            if((pow((i - x), 2) + pow((j - y), 2)) <= pow(radio, 2))
                matrix[i][j]++;
}

bool validateCircle(int** matrix, int nRows, int nCols, int radio, int x, int y) {
    if(x > NR || y > NC || x < 0 || y < 0) return false;

    // int maxX = ((x - radio) >= 0) ? x - radio : 0, maxY = ((y - radio) >= 0) ? y - radio : 0, minX = ((x + radio) < nCols)? x + radio + 1 : nCols, minY = ((y + radio) < nRows)? y + radio + 1 : nRows;

    for(int i = 0; i < nRows; i++)
        for(int j = 0; j < nCols; j++)
            if((pow((i - x), 2) + pow((j - y), 2)) <= pow(radio, 2))
                if(matrix[i][j] != 1)
                    return false;

    return true;
}

int getAreaMeanCircle(int** matrix, int nRows, int nCols, int radio, int x, int y) {
    if(x > NR || y > NC || x < 0 || y < 0) return 1000;

    // int maxX = ((x - radio) >= 0) ? x - radio : 0, maxY = ((y - radio) >= 0) ? y - radio : 0, minX = ((x + radio) < nCols)? x + radio + 1 : nCols, minY = ((y + radio) < nRows)? y + radio + 1 : nRows;
    int sum = 0, pixelCount = 0;

    for(int i = 0; i < nRows; i++)
        for(int j = 0; j < nCols; j++)
            if((pow((i - x), 2) + pow((j - y), 2)) <= pow(radio, 2)) {
                matrix[i][j]++;
                sum += matrix[i][j];
                pixelCount++;
            }

    return (sum / pixelCount);
}
