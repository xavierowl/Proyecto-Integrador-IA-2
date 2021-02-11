#include <iostream>
#include <cstdlib>

// Librería que contiene funciones matemáticas
#include <cmath>
// Librería para trabajar con archivos
#include <fstream>

//Librería para trabajar con vectores
#include <vector>

#include <string>

// Librerías de OpenCV

// Se pueden cargar todas las Librerías incluyendo
//#include <opencv2/opencv.hpp>

// Contiene las definiciones fundamentales de las matrices e imágenes 
#include <opencv2/core/core.hpp> 
// Procesamiento de imágenes
#include <opencv2/imgproc/imgproc.hpp>
// Códecs de imágenes
#include <opencv2/imgcodecs/imgcodecs.hpp>
// Manejo de ventanas y eventos de ratón, teclado, etc.
#include <opencv2/highgui/highgui.hpp>
// Lectura de video
#include <opencv2/video/video.hpp>
// Escritura de video
#include <opencv2/videoio/videoio.hpp>

using namespace std; // Permite cargar las constantes y métodos de la librería estándar de C++ (posee métodos para manejo de cadenas, búsquedas, etc.).
using namespace cv;

void sesgarObjeto(Mat roi, Mat objeto, Mat frame){
    int pixel = 0;
	for(int i=0;i<roi.rows;i++){
                for(int j=0;j<roi.cols;j++){
                    pixel = roi.at<uchar>(i,j);
                    if(pixel>0){
                        objeto.at<Vec3b>(i,j) = frame.at<Vec3b>(i,j);                        
                    }
                }
            }
    GaussianBlur(objeto, objeto, Size(9, 9),0,0);
}

vector<Point> obtenerContorno(Mat roi, Mat imgContornos, Mat objeto){    
    Mat bordes;
    int contornoPrincipal =  0;
    vector<vector<Point> > contornos;
    vector<Vec4i> jerarquia;
    vector<vector<Point> > cascos(1);
    vector<vector<int> > cascosI(1);
    vector<vector<Vec4i>> defectosConvexos(1);
    
    //Aplicación de un un filtro gausiano
    GaussianBlur(roi, roi, Size(3,3),0,0);
            
    //Obtención de los bordes de la zona de interés
    Laplacian(roi, bordes, CV_16S, 3);
    convertScaleAbs(bordes, bordes);
    
    //Umbralizaciónde los bordes encontrados
    threshold(bordes, bordes, 100, 255, THRESH_BINARY);
            
    //Obtención del contorno
    findContours(bordes,contornos,jerarquia,RETR_TREE,CHAIN_APPROX_SIMPLE);
            
    //Obtención del contorno más grande
    for(int i = 0; i < contornos.size(); i++){
        if(contornos[contornoPrincipal].size() < contornos[i].size()){
            contornoPrincipal = i;
        }
    }
    if(contornos.size()!=0){
        drawContours(imgContornos,contornos,contornoPrincipal,Scalar(255,255,255),2,LINE_8, jerarquia,3);
        convexHull(contornos[contornoPrincipal], cascos[0]);
        convexHull(contornos[contornoPrincipal], cascosI[0]);
        drawContours(objeto,cascos,0,Scalar(0,255,0),2,LINE_8);
        
        convexityDefects(contornos[contornoPrincipal], cascosI[0], defectosConvexos[0]);
        
        for(int i = 0; i < defectosConvexos[0].size(); i++){
            float depth = defectosConvexos[0][i][3] / 256;
            if (depth > 10){
                int inicio = defectosConvexos[0][i][0];
                Point ptInicio(contornos[contornoPrincipal][inicio]);
                
                int fin = defectosConvexos[0][i][1];
                Point ptFin(contornos[contornoPrincipal][fin]);
                
                int lenajo = defectosConvexos[0][i][2];
                Point ptLejano(contornos[contornoPrincipal][lenajo]);
                circle(objeto, ptFin, 4, Scalar(255, 133, 0), 2);
                circle(objeto, ptLejano, 4, Scalar(0, 0, 255), 2);
            }   
        }
        
        return contornos[contornoPrincipal];
    }
    else{
        return vector<Point>(1);
    }
}

void calcularComplementos(Mat objeto, Mat roi, Moments momentos){
    double cx;
    double cy;
            
    cx = momentos.m10/momentos.m00;
    cy = momentos.m01/momentos.m00;
            
    circle(objeto, Point(cx, cy), 5, Scalar(255,0,0),4);
}

int existe(double momentos[7]){    
     int gesto = 0;
     int contadorLinea = 1;
     string linea;     
     vector<double> base;
          
     string caracter;
     string numero = "";
     
     double suma = 0.0;
    
     ifstream archivoLectura("figuras.txt", ifstream::in);
     
     while(getline(archivoLectura, linea)){
         suma = 0;
         for(int i = 0; i < linea.length(); i++){
             caracter = linea.substr(i, 1);
             numero = numero + caracter;
             
              if(caracter == ","){
                  base.push_back(stod(numero.substr(0,-1)));
                  numero = "";
              }
              if(caracter == ";"){
                  base.push_back(stod(numero.substr(0,-1)));
                  numero = "";
                  break;
              }
        }
        
        //Distancia euclideana
//         for(int i=0;i<7;i++){
//             suma+=(base[i]-momentos[i])*(base[i]-momentos[i]);
//         }
//         suma = sqrt(suma);
//         
//         if(suma < 0.005){
//             cout<<suma<<"||"<<contadorLinea<<endl;
//             archivoLectura.close();
//             gesto = contadorLinea;
//             return gesto;
//         }
        
//         Distancia Manhattan
        for(int i=0;i<7;i++){
            suma+=abs(base[i]-momentos[i]);
        }
        suma = sqrt(suma);
        
        if(suma < 0.06){
             cout<<suma<<"||"<<contadorLinea<<endl;
            archivoLectura.close();
            gesto = contadorLinea;
            return gesto;
        }
        
        contadorLinea += 1;
        
        base.clear();
     }
     return gesto;
}
