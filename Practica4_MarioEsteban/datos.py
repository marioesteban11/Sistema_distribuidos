import numpy as np
import statistics

fichero_csv = open("results_latency.csv",'w+')
justo = 2
paralelo = 1
secuencial = 0

def get_datos(fichero, numero, cantidad): 
    fichero_open = open(fichero, "r")
    lineas_fichero = fichero_open.readlines()

    lol = 0
    fichero = []
    for i in lineas_fichero:
        # 
        if (i != "\n"):
            #print(lol)
            fichero.append(float(i))
            lol = lol+1

    #print(lol)
    fichero_open.close()


    minimo_fichero = min(fichero)
    maximo_fichero = max(fichero)
    dev_std_fichero = np.std(fichero)
    mean_fichero = statistics.mean(fichero)
    print(numero)
    print("Minimo: " + str(minimo_fichero)+" maximo: " + str(maximo_fichero) + " dev_standart: " +str(dev_std_fichero) + " media: "+ str(mean_fichero))

    datos_fichero = str(numero) +", " +str(cantidad) + ", "+ str(minimo_fichero)+ ", " + str(maximo_fichero) +", "+str(mean_fichero)+ ", "  +str(dev_std_fichero) + "\n"

    fichero_csv.write(datos_fichero)



secuencial_10 = "datos_secuencial_10.txt"
secuencial_100 = "datos_secuencial_100.txt"
secuencial_500 = "datos_secuencial_500.txt"
secuencial_1000 = "datos_secuencial_1000.txt"
paralelo_10 = "datos_paralelo_10.txt"
paralelo_100 = "datos_paralelo_100.txt"
paralelo_500 = "datos_paralelo_500.txt"
paralelo_1000 = "datos_paralelo_1000.txt"
justo_10 = "datos_justo_10.txt"
justo_100 = "datos_justo_100.txt"
justo_500 = "datos_justo_500.txt"
justo_1000 = "datos_justo_1000.txt"

get_datos(secuencial_10, secuencial, 10)
#get_datos(secuencial_100, secuencial, 100)
get_datos(secuencial_500, secuencial, 500)
get_datos(secuencial_1000, secuencial, 1000)
get_datos(paralelo_10, paralelo, 10)
#get_datos(paralelo_100, paralelo, 100)
get_datos(paralelo_500, paralelo, 500)
get_datos(paralelo_1000, paralelo, 1000)
get_datos(justo_10, justo, 10)
#get_datos(justo_100, justo, 100)
get_datos(justo_500, justo, 500)
get_datos(justo_1000, justo, 1000)













