import sys
import time
from xml.etree.ElementTree import ElementTree
import cppmodule

def buildMatrix(xmlFile, lang):
    with open (xmlFile, 'r') as inFile:
        elTree = ElementTree()
        elTree.parse(inFile)
    
    #number of nodes in net
    n = len(elTree.getroot().findall('net'))
    
    #building oriented graph
    A = [[float('inf') for j in range (n)] for i in range (n)]

    for resistors in elTree.getroot().findall('resistor'):
        i = int(resistors.attrib['net_from']) - 1
        j = int(resistors.attrib['net_to']) - 1
        r = float(resistors.attrib['resistance'])
        A[i][j] = 1/(1/A[i][j] + 1/r)
        A[j][i] = 1/(1/A[j][i] + 1/r)

    for diodes in elTree.getroot().findall('diode'):
        i = int(diodes.attrib['net_from']) - 1
        j = int(diodes.attrib['net_to']) - 1
        A[i][j] = 1/(1/A[i][j] + 1/float(diodes.attrib['resistance']))
        A[j][i] = 1/(1/A[j][i] + 1/float(diodes.attrib['reverse_resistance']))

    for capactors in elTree.getroot().findall('capactor'):
        i = int(capactors.attrib['net_from']) - 1
        j = int(capactors.attrib['net_to']) - 1
        r = float(capactors.attrib['resistance'])
        A[i][j] = 1/(1/A[i][j] + 1/r)
        A[j][i] = 1/(1/A[j][i] + 1/r)

    #graph is built
        
    #searching for the shortest ways

    for i in range(n):
        A[i][i] = 0;

    #тут был лишний цикл, который фактически приравнивал результирующую матрицу к начальной, его я вырезал
    if lang == 'python':
        
        for k in range(n):
            for i in range(n):
                for j in range(n):
                    d1 = A[i][j]
                    d2 = A[i][k] + A[k][j]
                
                    if d1 == 0 or d2 == 0:
                        A[i][j] = 0
                    else:
                        if (d1 != float('inf') or d2 != float('inf')):
                           A[i][j] = 1/(1/d1 + 1/d2)
    else:
        if lang == 'c++':
            A = cppmodule.faster_dot(A)               

    return n, A


def printToCSV(csvFile, n, A):
    with open(csvFile, 'w') as outFile:
        for i in range(n):
            for j in range(n):
                print("%.6f" % (A[i][j]), end=',', file=outFile)
            print('\n', end='', file=outFile)
    return


def main(xmlFile, csvFile):

    #comitting python code
    tstartpy = time.time()
    
    n, A = buildMatrix(xmlFile, 'python')   
    printToCSV(csvFile, n, A)
    
    tpy = time.time() - tstartpy

    #comitting pytthon and c++ code
    tstartcpp = time.time()

    n, A = buildMatrix(xmlFile, 'c++')   
    printToCSV(csvFile, n, A)
    
    tcpp = time.time() - tstartcpp

    return tpy/tcpp

if __name__ == "__main__":
    print(main(sys.argv[1], sys.argv[2]))
