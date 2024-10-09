#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Tente abrir a câmera com o índice 0
    cv::VideoCapture cap(0);

    // Verifique se a câmera foi aberta corretamente
    if (!cap.isOpened()) {
        std::cerr << "Erro ao abrir a câmera!" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        // Capture um novo frame
        cap >> frame;

        // Verifique se o frame foi capturado corretamente
        if (frame.empty()) {
            std::cerr << "Erro ao capturar o frame!" << std::endl;
            break;
        }

        // Exiba o frame
        cv::imshow("Captura de Vídeo", frame);

        // Saia do loop ao pressionar 'q'
        if (cv::waitKey(30) >= 0) {
            break;
        }
    }

    // Libere os recursos
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
