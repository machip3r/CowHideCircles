# Libraries

from xmlrpc.client import FastUnmarshaller
import numpy
import random
import pygad
# import cv2

# Manipulate files


def getMatrixFromCSV(filename):
    return numpy.loadtxt(open(filename, "rb"), delimiter=",", dtype=float, skiprows=1).astype(int)

# Manipulate circles


def calculateX(radio, y):
    return round(((radio ** 2) - ((y - radio) ** 2)) ** .5)


def drawCirclesInCenter(matrix, radio, x, y):
    minX = int(x - radio if (x - radio) >= 0 else 0)
    minY = int(y - radio if (y - radio) >= 0 else 0)
    maxX = int(x + radio if (x + radio) < len(matrix) else len(matrix))
    maxY = int(y + radio if (y + radio) < len(matrix[0]) else len(matrix[0]))

    for i in range(minX, maxX):
        for j in range(minY, maxY):
            if ((((i - x) ** 2) + ((j - y) ** 2)) <= radio ** 2):
                matrix[i][j] += 1


def drawCirclesWithList(matrix, radio, circlesList):
    for point in range(len(circlesList)):
        if point % 2 == 0:
            x0 = circlesList[point]
            y0 = circlesList[point + 1]

            minX = int(x0 - radio if (x0 - radio) >= 0 else 0)
            minY = int(y0 - radio if (y0 - radio) >= 0 else 0)
            maxX = int(x0 + radio if (x0 + radio) <
                       len(matrix) else len(matrix))
            maxY = int(y0 + radio if (y0 + radio) <
                       len(matrix[0]) else len(matrix[0]))

            for i in range(minX, maxX):
                for j in range(minY, maxY):
                    if ((((i - x0) ** 2) + ((j - y0) ** 2)) <= radio ** 2):
                        matrix[i][j] += 1


def getAreaMeanCircle(matrix, radio, x, y):
    sum = 0
    pixelCount = 0

    minX = int(x - radio if (x - radio) >= 0 else 0)
    minY = int(y - radio if (y - radio) >= 0 else 0)
    maxX = int(x + radio if (x + radio) < len(matrix) else len(matrix))
    maxY = int(y + radio if (y + radio) < len(matrix[0]) else len(matrix[0]))

    for i in range(minX, maxX):
        for j in range(minY, maxY):
            if ((((i - x) ** 2) + ((j - y) ** 2)) <= radio ** 2):
                sum += matrix[i][j]
                pixelCount += 1

    return (sum / pixelCount)


def validateCircle(matrix, radio, x, y):
    isValid = True

    minX = int(x - radio if (x - radio) >= 0 else 0)
    minY = int(y - radio if (y - radio) >= 0 else 0)
    maxX = int(x + radio if (x + radio) < len(matrix) else len(matrix))
    maxY = int(y + radio if (y + radio) < len(matrix[0]) else len(matrix[0]))

    for i in range(minX, maxX):
        for j in range(minY, maxY):
            if ((((i - x) ** 2) + ((j - y) ** 2)) <= radio ** 2):
                if matrix[i][j] != 1:
                    isValid = False

    return isValid

# Manipulate binary


def listBinaryToDecimal(bitList):
    binaryList = []
    binStr = ""
    decimalList = []

    for bit in range(len(bitList)):
        if bit == 0 or bit % 10:
            binStr += str(bitList[bit])
            if bit == (len(bitList) - 1):
                binaryList.append(binStr)
        else:
            binaryList.append(binStr)
            binStr = ""
            binStr += str(bitList[bit])

    for point in binaryList:
        decimalList.append(int(point, 2))

    return decimalList

# Fitness function 1


def fit1(solution, solution_idx):
    fitness = 0

    matrix = getMatrixFromCSV("piel-de-carnero_bin.csv")

    for point in range(len(solution)):
        matrixCopy = matrix
        if point % 2 == 0:
            drawCirclesInCenter(matrixCopy, RADIO,
                                solution[point], solution[point + 1])
            fitness += getAreaMeanCircle(matrixCopy, RADIO,
                                         solution[point], solution[point + 1])

    return (1 / fitness)

# Fitness function 2


def fit2(solution, solution_idx):
    fitness = 0

    matrix = getMatrixFromCSV("piel-de-carnero_bin.csv")

    for point in range(len(solution)):
        matrixCopy = matrix
        if point % 2 == 0:
            drawCirclesInCenter(matrixCopy, RADIO,
                                solution[point], solution[point + 1])
            if (getAreaMeanCircle(matrix, RADIO, solution[point], solution[point + 1]) == 1):
                drawCirclesInCenter(matrix, RADIO,
                                    solution[point], solution[point + 1])
                fitness += 1

    return fitness

# Fitness function 3


def fit3(solution, solution_idx):
    fitness = 0

    matrix = getMatrixFromCSV("piel-de-carnero_bin.csv")

    for point in range(len(solution)):
        matrixCopy = matrix
        if point % 2 == 0:
            drawCirclesInCenter(matrixCopy, RADIO,
                                solution[point], solution[point + 1])
            if (validateCircle(matrixCopy, RADIO, solution[point], solution[point + 1])):
                drawCirclesInCenter(matrix, RADIO,
                                    solution[point], solution[point + 1])
                fitness += 1

    return (1 / ((N_CIRCLES - fitness) ** 2))


def doInGeneration(ga_instance):
    solution, solution_fitness, solution_idx = ga_instance.best_solution()

    print("Generation: ", ga_instance.generations_completed)
    print("Fitness: ", solution_fitness)

    """ matrixCopy = matrix

    if ga_instance.generations_completed == 1 or not (ga_instance.generations_completed % 50):
        drawCirclesWithList(matrixCopy, RADIO, solution)

        numpy.savetxt("outputs/output" + str(ga_instance.generations_completed) + ".csv",
                      matrixCopy.astype(int), delimiter=",", fmt="%i") """

# Global variables


RADIO = 50
N_CIRCLES = 15

matrix = getMatrixFromCSV("piel-de-carnero_bin.csv")

# print(matrix)

# PyGAD config variables

""" fitness_function = fit1 """
""" fitness_function = fit2 """
fitness_function = fit3

on_generation = doInGeneration

num_generations = 10
num_parents_mating = 2

sol_per_pop = 25

# 0 720 0 720 0 720 0 720 0 720 0 720 0 720 0 720 0 720 0 720
# x  y  x  y  x  y  x  y  x  y  x  y  x  y  x  y  x  y  x  y
num_genes = N_CIRCLES * 2

gene_type = int

init_range_low = 0
init_range_high = 720

parent_selection_type = "sss"
keep_parents = 2

crossover_type = "single_point"

mutation_type = "random"
mutation_probability = 0.9
mutation_percent_genes = "default"

# Creation of GA model or instance

ga_instance = pygad.GA(num_generations=num_generations,
                       num_parents_mating=num_parents_mating,
                       fitness_func=fitness_function,
                       sol_per_pop=sol_per_pop,
                       num_genes=num_genes,
                       gene_type=gene_type,
                       init_range_low=init_range_low,
                       init_range_high=init_range_high,
                       parent_selection_type=parent_selection_type,
                       keep_parents=keep_parents,
                       crossover_type=crossover_type,
                       mutation_type=mutation_type,
                       mutation_probability=mutation_probability,
                       mutation_percent_genes=mutation_percent_genes,
                       on_generation=on_generation)

# GA search

ga_instance.run()

# Show solution

solution, solution_fitness, solution_idx = ga_instance.best_solution()
print("Parameters of the best solution : {solution}".format(solution=solution))
print(repr(solution))
print("Fitness value of the best solution = {solution_fitness}".format(
    solution_fitness=solution_fitness))

""" drawCirclesWithList(matrix, RADIO, solution) """

# Export matrix with circles

numpy.savetxt("outputs/output.csv", matrix.astype(int),
              delimiter=",", fmt="%i")
