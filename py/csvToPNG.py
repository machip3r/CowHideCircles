from cv2 import bitwise_and
import numpy
import cv2

MAX_CSV = 50


def getMatrixFromCSV(filename):
    return numpy.loadtxt(open(filename, "rb"), delimiter=",", dtype=float, skiprows=1).astype(int)


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


""" for i in range(MAX_CSV):
    if i > 0 and i % 50 == 0:
        matrix = getMatrixFromCSV("outputs/output" + str(i) + ".csv")

        for i in range(len(matrix)):
            for j in range(len(matrix[0])):
                if(matrix[i][j] == 1):
                    matrix[i][j] = 200

        cv2.imwrite("img/generation" + str(i) + ".png", matrix) """

RADIO = 50
solution = [260, 505, 429, 587, 269, 284, 208, 294, 334, 363, 503, 376, 508, 434, 333, 538, 206, 419, 182,
            236, 497, 205, 328, 215, 272, 211, 184, 508, 298, 198, 388, 245, 384, 195, 559, 222, 597, 508, 410, 485]
matrix = getMatrixFromCSV("piel-de-carnero_bin.csv")

drawCirclesWithList(matrix, RADIO, solution)

for i in range(len(matrix)):
    for j in range(len(matrix[0])):
        if(matrix[i][j] == 1):
            matrix[i][j] = 200

cv2.imwrite("img/generationOutCPP.png", matrix)

matrixOut = getMatrixFromCSV("outputs/output.csv")

for i in range(len(matrixOut)):
    for j in range(len(matrixOut[0])):
        if(matrixOut[i][j] == 1):
            matrixOut[i][j] = 200

cv2.imwrite("img/generationOutPY.png", matrixOut)
