    #include "opencv2/objdetect.hpp"
    #include "opencv2/highgui.hpp"
    #include "opencv2/imgproc.hpp"
    #include "opencv2/videoio.hpp"
    #include <iostream>
    #include <vector>
    #include <random>
    #include <iomanip>
    #include <chrono>
    #include <SFML/Audio.hpp>
    #include <thread>

    using namespace std;
    using namespace cv;

    // Declarações de funções
    void detectAndDraw(Mat& img, CascadeClassifier& cascade, double scale, bool tryflip, int& faceX);
    void drawImage(Mat frame, Mat img, int xPos, int yPos);
    void drawTransRect(Mat frame, Scalar color, double alpha, Rect region);
    bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
    void playMusic();

    string inputText = "";

    // Estrutura para representar objetos que caem
    struct FallingObject {
        int x;         // Posição no eixo X
        int y;         // Posição no eixo Y (fixo em 0)
        int speed;     // Velocidade de queda
        Mat image;     // Imagem do objeto
    };

    vector<FallingObject> fallingObjects; // Lista de objetos que caem
    vector<FallingObject> fallingObjectsM; // Vetor para objetos m.png
    string cascadeName;
    string wName = "Falling Objects Game";

    // Função para desenhar a caixa de texto
    void drawTextBox(Mat& img) {
        // Desenhar a caixa de texto (borda)
        rectangle(img, Point(160, 200), Point(450, 250), Scalar(0, 0, 0), 2);
        putText(img, "DIGITE O SEU NOME", Point(200, 190), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
        // Colocar o texto dentro da caixa
        putText(img, inputText, Point(250, 235), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        rectangle(img, Point(220, 260), Point(400, 300), Scalar(255, 0, 150), FILLED); // -1 ou FILLED para preencher
        putText(img, "ENTER", Point(260, 290), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0), 2);
    }

    void drawStringTextBox(Mat& img) {
        while (true) {
            int key = waitKey(0);  // Aguarda a entrada de uma tecla

            // Se pressionar a tecla ESC, sai do loop
            if (key == 27) {
                continue;
            }
            // Se pressionar a tecla ENTER, sai do loop
            else if (key == 13) {
                break;
            }
            // Se pressionar a tecla BACKSPACE, remove o último caractere
            else if (key == 8 && !inputText.empty()) {
                inputText.pop_back();  // Remove o último caractere
            }
            // Caso contrário, adicione o caractere digitado ao texto
            else if (key >= 32 && key <= 126) {  // Apenas caracteres imprimíveis
                inputText += (char)key;
            }

            // Limpa a imagem e desenha a nova caixa de texto
            Mat img = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
            resize(img, img, Size(640, 480));
            drawTextBox(img);

            // Atualiza a janela com o novo texto
            imshow(wName, img);
        }
    }

    int main(int argc, const char** argv) {
        VideoCapture capture;
        Mat frame;
        bool tryflip;
        CascadeClassifier cascade;
        double scale;
        char key = 0;

         // Iniciar a thread para tocar música
        std::thread musicThread(playMusic);

        // Defina um máximo de objetos 'm.png'
        const int maxMObjects = 2; // Máximo de objetos 'm.png' na tela

        // Aumente o tempo entre a criação dos objetos 'm.png'
        int tempoEntreM = 3000; // 3000 milissegundos (3 segundos) entre a aparição dos objetos 'm.png'

        cascadeName = "haarcascade_frontalface_default.xml";
        scale = 1; // usar 1, 2, 4.
        tryflip = true;
        Mat img = cv::Mat::zeros(cv::Size(640, 480), CV_8UC3);
        drawTextBox(img);
        imshow(wName, img);
        drawStringTextBox(img);

        // Carrega o classificador
        if (!cascade.load(cascadeName)) {
            cout << "ERROR: Could not load classifier cascade: " << cascadeName << endl;
            return -1;
        }

        // Captura de vídeo
        if (!capture.open(0)) {
            cout << "Capture from camera #0 didn't work" << endl;
            return 1;
        } else {
            cout << "Camera opened successfully!" << endl;
        }

        // Inicializa objetos que caem
        Mat objectImage = imread("cadeira.png", IMREAD_UNCHANGED); // Carrega a imagem do objeto
        if (objectImage.empty()) {
            cout << "ERROR: Could not load image: cadeira.png" << endl;
            return -1;
        }

        // Inicializa a imagem m.png
        Mat mImage = imread("m.png", IMREAD_UNCHANGED); // Carrega a imagem m.png
        if (mImage.empty()) {
            cout << "ERROR: Could not load image: m.png" << endl;
            return -1;
        }

        // Redimensiona a imagem do objeto para caber na tela
        double aspectRatio = static_cast<double>(objectImage.cols) / objectImage.rows;
        int newWidth = min(objectImage.cols, 50); // Largura máxima de 50 pixels
        int newHeight = static_cast<int>(newWidth / aspectRatio); // Mantém a proporção
        resize(objectImage, objectImage, Size(newWidth, newHeight));

        // Redimensiona a imagem 'm.png' para caber na tela
        double aspectRatioM = static_cast<double>(mImage.cols) / mImage.rows;
        int newWidthM = min(mImage.cols, 50); // Largura máxima de 50 pixels
        int newHeightM = static_cast<int>(newWidthM / aspectRatioM); // Mantém a proporção
        resize(mImage, mImage, Size(newWidthM, newHeightM));

        // Inicializa a contagem regressiva
        int countdownTime = 60; // 60 segundos
        auto startTime = chrono::steady_clock::now();

        // Variáveis para controle da criação de cadeiras
        const int maxCadeiras = 5; // Máximo de cadeiras
        int cadeirasCriadas = 0; // Contador de cadeiras
        auto lastCreationTime = chrono::steady_clock::now(); // Marca o tempo da última criação
        int tempoEntreCadeiras = 1000; // Tempo em milissegundos entre a aparição das cadeiras

        // Inicializa a imagem do Pablo Marçal
        Mat pabloImage = imread("pablomarçal.png", IMREAD_UNCHANGED); // Carrega a imagem
        if (pabloImage.empty()) {
            cout << "ERROR: Could not load image: pablomarçal.png" << endl;
            return -1;
        }

        // Redimensiona a imagem para caber na tela
        resize(pabloImage, pabloImage, Size(50, 100)); // Ajuste o tamanho conforme necessário

        // Inicializa a imagem do Datena
        Mat datenaImage = imread("datena.jpg", IMREAD_UNCHANGED); // Carrega a imagem do Datena
        if (datenaImage.empty()) {
            cout << "ERROR: Could not load image: datena.jpg" << endl;
            return -1;
        }

        // Redimensiona a imagem do Datena
        resize(datenaImage, datenaImage, Size(600, 600)); // Ajuste o tamanho conforme necessário

        int faceX = 0; // Variável para armazenar a posição X do rosto

        while (true) {
            capture >> frame;
            if (frame.empty()) {
                cout << "Captured empty frame!" << endl;
                break;
            }

            if (tryflip) cv::flip(frame, frame, 1);

            // Verifica se deve criar uma nova cadeira
            auto currentTime = chrono::steady_clock::now();
            chrono::duration<double> elapsedTime = currentTime - lastCreationTime;

            if (cadeirasCriadas < maxCadeiras && elapsedTime.count() >= tempoEntreCadeiras / 1000.0) {
                FallingObject obj;
                obj.x = rand() % (640 - objectImage.cols); // Posição X aleatória
                obj.y = 0; // Posição Y fixa em 0
                obj.speed = rand() % 5 + 2; // Velocidade aleatória
                obj.image = objectImage.clone();
                fallingObjects.push_back(obj);
                cadeirasCriadas++; // Incrementa o contador
                lastCreationTime = currentTime; // Atualiza o tempo da última criação
            }

            // Verifica se deve criar um novo 'm.png'
            if (fallingObjectsM.size() < maxMObjects && elapsedTime.count() >= tempoEntreM / 1000.0) {
                FallingObject objM;
                objM.x = rand() % (640 - mImage.cols); // Posição X aleatória
                objM.y = 0; // Posição Y fixa em 0
                objM.speed = rand() % 5 + 2; // Velocidade aleatória
                objM.image = mImage.clone();
                fallingObjectsM.push_back(objM); // Adiciona o objeto 'm.png'
                lastCreationTime = currentTime; // Atualiza o tempo da última criação
            }

            // Atualiza e desenha cada objeto que cai
            for (auto& obj : fallingObjects) {
                obj.y += obj.speed; // Move o objeto para baixo
                if (obj.y > frame.rows) { // Reinicia o objeto se sair da tela
                    obj.x = rand() % (640 - obj.image.cols); // Nova posição X aleatória
                    obj.y = 0; // Mantém a posição Y em 0
                    obj.speed = rand() % 5 + 2; // Nova velocidade
                }
                drawImage(frame, obj.image, obj.x, obj.y);
            }

            // Atualiza e desenha cada objeto 'm.png'
            for (auto& objM : fallingObjectsM) {
                objM.y += objM.speed; // Move o objeto para baixo
                if (objM.y > frame.rows) { // Reinicia o objeto se sair da tela
                    objM.x = rand() % (640 - objM.image.cols); // Nova posição X aleatória
                    objM.y = 0; // Mantém a posição Y em 0
                    objM.speed = rand() % 5 + 2; // Nova velocidade
                }
                drawImage(frame, objM.image, objM.x, objM.y); // Desenha o objeto na tela
            }

            // Atualiza a contagem regressiva
            auto currentTime2 = chrono::steady_clock::now();
            chrono::duration<double> elapsedSeconds = currentTime2 - startTime;

            if (elapsedSeconds.count() >= 1.0) { // A cada segundo
                countdownTime--;
                startTime = currentTime2; // Reinicia o temporizador
            }

            // Converte para um formato legível
            int seconds = countdownTime;
            if (seconds < 0) seconds = 0; // Garantir que não fique negativo
            int minutes = seconds / 60;
            seconds %= 60;

            // Formata o texto do cronômetro
            string timerText = format("Tempo: %02d:%02d", minutes, seconds);
            putText(frame, timerText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2); // Desenha o cronômetro

            // Chama a função de detecção e desenho, passando a posição X do rosto
            detectAndDraw(frame, cascade, scale, tryflip, faceX);

            // Desenhar a imagem do Pablo Marçal na posição X do rosto
            int pabloY = 370; // Posição fixa no eixo Y
            drawImage(frame, pabloImage, faceX - pabloImage.cols / 2, pabloY); // Centraliza em relação ao rosto

            // Verifica colisão entre o Pablo Marçal e as cadeiras
            for (const auto& obj : fallingObjects) {
                if (checkCollision(faceX - pabloImage.cols / 2, pabloY, pabloImage.cols, pabloImage.rows, 
                                obj.x, obj.y, obj.image.cols, obj.image.rows)) {
                    // Desenhar a imagem do Datena na tela
                    int datenaX = (frame.cols - datenaImage.cols) / 2; // Centraliza a imagem do Datena na tela
                    int datenaY = 350; // Ajuste a posição Y conforme necessário
                    drawImage(frame, datenaImage, datenaX, datenaY); // Desenha a imagem do Datena

                    // Desenhar a mensagem sobre a imagem do Datena
                    putText(frame, "CADEIRADA DO DATENA", Point(150, 300), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3); // Mensagem

                    imshow(wName, frame);
                    waitKey(3000); // Espera 3 segundos para mostrar a mensagem
                    return 0; // Encerra o programa
                }
            }

            // Verifica colisão entre o Pablo Marçal e os objetos 'm.png'
            for (const auto& objM : fallingObjectsM) {
                if (checkCollision(faceX - pabloImage.cols / 2, pabloY, pabloImage.cols, pabloImage.rows, 
                                objM.x, objM.y, objM.image.cols, objM.image.rows)) {
                    // Ação de colisão com 'm.png'
                    int datenaX = (frame.cols - datenaImage.cols) / 2; // Centraliza a imagem do Datena na tela
                    int datenaY = 350; // Ajuste a posição Y conforme necessário
                    drawImage(frame, datenaImage, datenaX, datenaY); // Desenha a imagem do Datena

                    // Desenhar a mensagem sobre a imagem do Datena
                    putText(frame, "CADEIRADA DO DATENA", Point(170, 300), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3); // Mensagem

                    imshow(wName, frame);
                    waitKey(3000); // Espera 3 segundos para mostrar a mensagem
                    return 0; // Encerra o programa
            }
                }

            // Mostra o frame na tela
            imshow(wName, frame);

            key = (char)waitKey(10);
            if (key == 27 || key == 'q') break; // Sai do loop se pressionar ESC ou 'q'
            }
            // Seu código OpenCV aqui
            cv::Mat image = cv::imread("caminho/para/sua/imagem.jpg");
            if (image.empty()) {
                std::cerr << "Imagem não encontrada!" << std::endl;
                return -1;
            }

            while (true) {
                cv::imshow("Janela", image);
                if (cv::waitKey(30) >= 0) break; // Sai ao pressionar qualquer tecla
                }

                // Finaliza a thread de música antes de sair
                musicThread.join();

                return 0;
    }

    // Função para desenhar a imagem sobre o frame
    void drawImage(Mat frame, Mat img, int xPos, int yPos) {
        // Verifica se a imagem não excede os limites da tela
        if (xPos < 0) xPos = 0;
        if (yPos < 0) yPos = 0;
        if (xPos + img.cols > frame.cols) xPos = frame.cols - img.cols;
        if (yPos + img.rows > frame.rows) yPos = frame.rows - img.rows;

        if (xPos >= 0 && yPos >= 0 && xPos + img.cols <= frame.cols && yPos + img.rows <= frame.rows) {
            Mat mask;
            vector<Mat> layers;

            split(img, layers); // Separa os canais
            if (layers.size() == 4) { // Se a imagem tiver canal alfa
                Mat rgb[3] = { layers[0], layers[1], layers[2] };
                mask = layers[3]; // O canal alfa é a máscara
                merge(rgb, 3, img); // Junta os canais RGB
                img.copyTo(frame(Rect(xPos, yPos, img.cols, img.rows)), mask); // Aplica a máscara
            } else {
                img.copyTo(frame(Rect(xPos, yPos, img.cols, img.rows))); // Se não houver canal alfa
            }
        }
    }

    void playMusic() {
    // Carregar a música
    sf::Music music;
    if (!music.openFromFile("musicadojogo.ogg")) {
        std::cerr << "Não foi possível carregar a música!" << std::endl;
        return;
    }
    music.setVolume(50); // Ajusta o volume (0-100)
    music.play(); // Inicia a música

    // Manter a thread viva enquanto a música toca
    while (music.getStatus() == sf::Music::Playing) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Espera para não sobrecarregar a CPU
    }
    }

    // Função de detecção de faces
    void detectAndDraw(Mat& img, CascadeClassifier& cascade, double scale, bool tryflip, int& faceX) {
        vector<Rect> faces;
        Mat gray;

        cvtColor(img, gray, COLOR_BGR2GRAY);
        resize(gray, gray, Size(), 1 / scale, 1 / scale, INTER_LINEAR);
        equalizeHist(gray, gray);

        cascade.detectMultiScale(gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

        for (const auto& r : faces) {
            faceX = cvRound(r.x * scale + r.width * 0.5); // Atualiza a posição X do rosto

            // Desenhar um retângulo ao redor da face
            Rect faceRect(cvRound(r.x * scale), cvRound(r.y * scale), 
                        cvRound(r.width * scale), cvRound(r.height * scale));
            rectangle(img, faceRect, Scalar(255, 0, 150), 3);  // Cor roxa (BGR: 255, 0, 150) e espessura 3
        }
    }

    // Função para desenhar um retângulo translúcido
    void drawTransRect(Mat frame, Scalar color, double alpha, Rect region) {
        Mat overlay;
        frame.copyTo(overlay);
        rectangle(overlay, region, color, FILLED);
        addWeighted(overlay, alpha, frame, 1 - alpha, 0, frame);
    }

    // Função para verificar colisão entre dois retângulos
    bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
        return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
    }