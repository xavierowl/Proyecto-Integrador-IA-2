// Librerías que contienen funciones estándar de C++
#include <iostream>
#include <cstdlib>

#include "Cabecera.hpp"

// Librería que contiene funciones matemáticas
#include <cmath>

// Librería para trabajar con arvhivos
#include <fstream>

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

using namespace std;
using namespace cv; // Espacio de nombres de OpenCV (Contiene funciones y definiciones de varios elementos de OpenCV)

Rect seleccion(0,0,300,250);
Mat elemento = getStructuringElement(MORPH_CROSS, Size(6,6)) * 255;

// Valores para la segmentación de color en el espacio HSV
int minH = 0, minS = 10, minV = 0;
int maxH = 46, maxS = 227, maxV = 255;

// Valores para la segmentación de color en el espacio YCbCr
int minY = 0, minCb = 134, minCr =0;
int maxY = 255, maxCb = 255, maxCr=255;

void trackBarEventHSV(int v, void *p){
}

void crearTrackBarHSV(){
    createTrackbar("H-Min", "Frame", &minH, 180, trackBarEventHSV);
    createTrackbar("S-Min", "Frame", &minS, 255, trackBarEventHSV);
    createTrackbar("V-Min", "Frame", &minV, 255, trackBarEventHSV);
        
    createTrackbar("H-Max", "Frame", &maxH, 180, trackBarEventHSV);
    createTrackbar("S-Max", "Frame", &maxS, 255, trackBarEventHSV);
    createTrackbar("V-Max", "Frame", &maxV, 255, trackBarEventHSV);
}

void crearTrackBarYCbCr(){
    createTrackbar("Y-Min", "Frame", &minY, 180, trackBarEventHSV);
    createTrackbar("Cb-Min", "Frame", &minCb, 255, trackBarEventHSV);
    createTrackbar("Cr-Min", "Frame", &minCr, 255, trackBarEventHSV);
        
    createTrackbar("Y-Max", "Frame", &maxY, 255, trackBarEventHSV);
    createTrackbar("Cb-Max", "Frame", &maxCb, 255, trackBarEventHSV);
    createTrackbar("Cr-Max", "Frame", &maxCr, 255, trackBarEventHSV);
}


int main(int argc, char *argv[]){
    Mat frame;
    Mat imgHSV;
    Mat roi; // Region of Interes, Zona de Interés
    Mat objeto;
    Mat imgContornos;
    Mat imgYCrCb;
    Mat ecualizada;
    Mat ultimo = Mat(Size(seleccion.width, seleccion.height), CV_8UC3, Scalar(0,0,0));;
    
    Mat aviso = Mat(Size(500, 100), CV_8UC3, Scalar(0,0,0));
    
    int pixel = 0;
    int op;
    int gesto = 0;
    
    bool espacioHSV = true;
    bool grabacion = false;
    
    //Declaración de los vectores
    vector<Point> contorno;
    
    //Variable que guarda los momentos de Hu
    double huMoments[7];
    Moments momentos;
    
    //Se crea el arvhivo donde se guardaran los momentos de Hu
    ofstream archivoEscritura;    
    int lineas=0;
    
    //Creación de las ventanas
    namedWindow("Frame", WINDOW_AUTOSIZE);
    moveWindow("Frame",200,0);
    namedWindow("Espacio de color", WINDOW_AUTOSIZE);
    moveWindow("Espacio de color",1000, 0);
    namedWindow("ROI", WINDOW_AUTOSIZE);
    moveWindow("ROI",1000, 500);
    namedWindow("Contorno", WINDOW_AUTOSIZE);
    moveWindow("Contorno",1470, 500);
    namedWindow("Objeto", WINDOW_AUTOSIZE);
    moveWindow("Objeto",1470,800);
    namedWindow("Aviso", WINDOW_AUTOSIZE);
    moveWindow("Aviso",200,1000);
    namedWindow("Ultimo Gesto detectado", WINDOW_AUTOSIZE);
    moveWindow("Ultimo Gesto detectado",1000,1000);
    
    //Creación de los trackbars
    if(espacioHSV)
        crearTrackBarHSV();
    else
        crearTrackBarYCbCr();
    
    VideoCapture video(0);
    
    if(video.isOpened()){
        pixel = 0;
        
        while(3==3){
            video >> frame;
            
            imgContornos = Mat(Size(seleccion.width, seleccion.height), CV_8UC3, Scalar(0,0,0));
            
            resize(frame, frame, Size(), 0.60, 0.60);
            
            rectangle(frame,Point(seleccion.x,seleccion.y),Point(seleccion.width, seleccion.height), Scalar(0,255,0),3);
            
            //Ecualización de imágen
            vector<Mat> canales;
            split(frame,canales);
            
            equalizeHist(canales[0], canales[0]);
            equalizeHist(canales[1], canales[1]);
            equalizeHist(canales[2], canales[2]);
            merge(canales,frame);
            //Fin de ecualización
            
            cvtColor(frame, imgHSV, COLOR_BGR2HSV);
            
            inRange(imgHSV, Scalar(minH, minS, minV), Scalar(maxH, maxS, maxV), roi);
            
            if(espacioHSV){
                cvtColor(frame, imgHSV, COLOR_BGR2HSV);
                inRange(imgHSV, Scalar(minH, minS, minV), Scalar(maxH, maxS, maxV), roi);
            }
            else{
                cvtColor(frame, imgYCrCb, COLOR_RGB2YCrCb);
                inRange(imgYCrCb, Scalar(minY, minCr, minCb), Scalar(maxY, maxCr, maxCb), roi); 
            }
            
            objeto = Mat(Size(frame.cols, frame.rows), CV_8UC3, Scalar(255,255,255));            
            
            //Obtiene los pixeles en color de la ventana objeto
            sesgarObjeto(roi, objeto, frame);
            
            //Recorte de los objetos de interés
            roi = roi(seleccion);
            objeto = objeto(seleccion);
            
            dilate(roi, roi, elemento);
            erode(roi, roi, elemento);
            
            //Se obtiene y dibuja el contorno más grande; y el casco convexo
            contorno = obtenerContorno(roi, imgContornos, objeto);

            //Se calculan los complementos de la figura
            momentos = moments(roi, true);
            HuMoments(momentos, huMoments);
            
            calcularComplementos(objeto, roi, momentos);
            
            if(espacioHSV){
                imshow("Espacio de color", imgHSV);
            }
            else{
                imshow("Espacio de color", imgYCrCb);
            }
            imshow("Ultimo Gesto detectado", ultimo);
            imshow("Aviso", aviso);
            imshow("ROI", roi);
            imshow("Objeto", objeto);
            imshow("Frame", frame);
            imshow("Contorno", imgContornos);
            
            op = waitKey(23);
            
            if(op==27)
                break;
            if(op==71||op==103)
                grabacion = true;
            if(op==73||op==105)
                grabacion = false;
            
            aviso = Mat(Size(500, 100), CV_8UC3, Scalar(0,0,0));
            
            if(grabacion){
                putText(aviso, //target image
                "Modo registro!", //text                      
                Point(120, aviso.rows / 2),
                FONT_HERSHEY_DUPLEX,
                1.0,
                CV_RGB(0, 130, 255), 1);
                if(op==32){                    
                    
                    lineas = 0;
                    ifstream archivoLectura("figuras.txt", ifstream::in);
                    
                    while(archivoLectura.good()) if(archivoLectura.get()=='\n') lineas++;
                    
                    archivoLectura.close();                
                    
                    if(lineas < 3){
                        archivoEscritura.open("figuras.txt", ios_base::app);
                        for(int i = 0; i < 6; i++){
                            archivoEscritura<<huMoments[i]<<",";
                        }
                        archivoEscritura<<huMoments[6]<<";\n";
                        archivoEscritura.close();
                        
                        cout<<"Se ha registrado la figura!!"<<endl;
                    }
                    else{
                        cout<<"Ya no se pueden registrar más figuras!!"<<endl;
                        archivoEscritura.close();
                    }
                }
            }
            else{
                gesto = existe(huMoments);
                if(gesto != 0){
                    putText(aviso, //target image
                    "Gesto detectado!", //text
                    Point(120, aviso.rows / 2),
                    FONT_HERSHEY_DUPLEX,
                    1.0,
                    CV_RGB(0, 255, 0), 1);
                    ultimo = frame(seleccion);
                    putText(ultimo, //target image
                    "Gesto "+to_string(gesto)+" detectado!", //text
                    Point(5, aviso.rows / 2),
                    FONT_HERSHEY_DUPLEX,
                    1.0,
                    CV_RGB(0, 0, 255), 1);
                    ultimo = frame(seleccion);
                }
                else{
                    putText(aviso, //target image
                    "Esperando gesto", //text
                    Point(120, aviso.rows / 2),
                    FONT_HERSHEY_DUPLEX,
                    1.0,
                    CV_RGB(255, 0, 0), 1);
                }
            }
        }
        
        destroyAllWindows();  
        remove("figuras.txt");
    }

    return 0;
}
