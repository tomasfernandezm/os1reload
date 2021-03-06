#ifndef KEYFRAMETRIANGULACION_H
#define KEYFRAMETRIANGULACION_H

#define D(var) std::cout << #var << ":" << var << endl;

#include "KeyFrame.h"	// Incluye MapPoint.h, que incluye opencv

using namespace cv;
namespace ORB_SLAM2{


class KeyFrameTriangulacion{
public:
    // Propiedades efímeras, usadas durante la triangulación, y su mutex

	/**
	 * Punto singular a triangular.
	 */
    KeyFrame &kf;

    /**
     * Parámetros intrínsecos del keyframe.
     */
    float fx, fy, cx, cy, invfx, invfy,

    /**
     * Coordenada z del punto 3D triangulado, en el sistema de referencia del keyframe.
     *
     * Calculado en KeyFrameTriangulacion::coordenadaZ, y reusado en KeyFrameTriangulacion::errorReproyeccion.
     */
    z;

    /**
     * Pose del keyframe, copiado en el constructor.
     *
     * Mat 4x4 float, rototraslación 3D en coordenadas homogéneas, en el sistema de referencia del keyframe.
     */
    Mat Tcw,

    /**
     * Posición del keyframe en el mundo, copiada en el constructor.
     */
    Ow,

    /**
     * Rotación del keyframe, calculada en el constructor.
     */
    Rcw,

    /**
     * Rotación del keyframe respecto del mundo, traspuesta de Rcw, calculada en el constructor.
     */
    Rwc,

    /**
     * Posición del keyframe, calculado en el constructor.
     */
    tcw,

    /**
     * Punto singular normalizado según parámetros intrínsecos, y en coordenadas homogéneas.
     */
    xn;
    KeyPoint kp;


    /**
     * Segundo objeto, representando la otra pose para triangular un punto.
     *
     * Usado con el segundo constructor.
     */
    KeyFrameTriangulacion *kft2;

    /** Coseno de paralaje umbral. */
    float umbralCosParalaje = 0.9998;

    // Resultados parciales
    /** Resultado parcial, coseno del paralaje. */
    float cosParalaje;

    Mat ray, x3D;

    /** 0 para no infinito, 1 para infito por cos, 2 para infinito por SVD.*/
    int inf;

    /** Error.  0 si se trianguló exitosamente.  1 si falló Z.  2 si falló la distancia.*/
    int error;

    /**
     * Constructor que copia datos de pose del keyframe argumento.
     */
    KeyFrameTriangulacion(KeyFrame *pKF);

    /**
     * Constructor alternativo, que construye dos objetos, uno para cada pose, y procede con la triangulación.
     */
    KeyFrameTriangulacion(KeyFrame *pKF1, int indice1, KeyFrame *pKF2,int indice2);


    /**
     * Calcula el rayo de proyección de un punto singular, y otros datos de interés para la triangulación, que se guardan en propiedades efímeras.
     *
     * Lo presenta como versor en el sistema de referencia del mundo,
     * convenientemente normalizado para hacer un producto escalar con otro y obtener el coseno del ángulo.
     *
     * @param indice Índice del vector de puntos singulares KeyFrame::mvKeysUn
     * @returns Mat 3x1 float con el versor del rayo de proyección del punto singular indicado por el índice.
     *
     *
     */
    cv::Mat rayo(int indice);
    cv::Mat rayo();



    /**
     * Devuele el punto triangulado, en coordenadas homogéneas, matriz horizontal 1x3 x3Dt.
     *
     * Construye la matriz A de triangulación de punto y la descompone por SVD.
     */
    Mat triangular(KeyFrameTriangulacion &kft);

    /**
     * Realiza la triangulación de los dos keypoints, informando posibles errores y devolviendo el punto 3D calculado, incluyendo infinito.
     *
     * No implementado todavía
     */
    int triangularPuntos(Mat x3D);

    /**
     * @returns El error de reproyección al cuadrado.
     *
     * Utiliza parámetros efímeros de triangulación.
     */
    float errorReproyeccion(cv::Mat x3Dt);


    /**
     * @returns true si el error es aceptable.
     */
	inline bool validarErrorReproyeccion(cv::Mat x3Dt){
		return errorReproyeccion(x3Dt) <= 5.991 * kf.mvLevelSigma2[kp.octave];
	}


	/**
     * Calcula la distancia del punto 3D al centro de la cámara.
     */
    inline float distancia(cv::Mat punto3D){
    	return norm(punto3D-Ow);
    }


    /**
     * Coordenada z del punto, en el sistema de referencia de la cámara.
     *
     * Registra z para su posterior uso en KeyFrameTriangulacion::errorReproyeccion.
     */
    inline float coordenadaZ(cv::Mat x3Dt){
    	return z = Rcw.row(2).dot(x3Dt)+tcw.at<float>(2);
    }

    /**
     * Chequea que las distancias sean coherentes.
     *
     * @returns true si son coherentes, false si no superó la validación.
     *
     * Requiere kft2.
     */
    bool validarDistancias();

    /**
     * Cambia el segundo keyframe contra el que triangular.
     */
    inline void setKeyFrame2(KeyFrame *pKF2){
    	kft2 = new KeyFrameTriangulacion(pKF2);
    }

    /**
     * Define el punto singular que será usado en la triangulación.
     *
     * @param indice del vector de keypoints del keyframe.
     */
    inline void setKeyPoint(int indice){
    	kp = kf.mvKeysUn[indice];
    }

    /**
     * Define el punto singular que será usado en la triangulación.
     *
     * @param indice del vector de keypoints del keyframe.
     */
    inline void setKeyPoint2(int indice){
    	kft2->setKeyPoint(indice);
    }

    /**
     * Realiza toda la triangulación basado en las propiedades definidas previamente.
     * Devuelve los resultados en propiedades.
     *
     * @returns el error si no pudo triangular, 0 si lo logró.
     */
    int triangular();

};

}// ORB_SLAM2

#endif // KEYFRAMETRIANGULACION_H
