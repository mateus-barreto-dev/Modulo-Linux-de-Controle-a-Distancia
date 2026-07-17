package com.mateus.ihsremote;

//-----------------Coisas do Android------------------
import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;
//----------------------------------------------------
//---------------------TCP----------------------------
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
//----------------------------------------------------
//--------------------Depuração-----------------------
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
//----------------------------------------------------
//----------------MovimentoMouse----------------------
import android.view.GestureDetector;
import android.view.MotionEvent;
//----------------------------------------------------
//-----------------FilaComandos-----------------------
import java.net.SocketTimeoutException;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
//----------------------------------------------------
//----------------Teclado-----------------------------
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
//----------------------------------------------------
import androidx.appcompat.app.AlertDialog;

import android.widget.ImageView;
import android.widget.LinearLayout;
import android.text.InputType;
import com.google.android.material.snackbar.Snackbar;
import java.io.BufferedReader;
import java.io.InputStreamReader;
//--------------RecvImagens--------------------------
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class MainActivity extends AppCompatActivity {
    private final BlockingQueue<String> fila = new LinkedBlockingQueue<>(); //fila de comandos
    private float posX = 0, posY = 0, dX = 0, dY = 0, lastX = 0, lastY = 0; //Movimento do Mouse
    int N = 0;
    private float mediaY_scroll = 0, lastY_scroll = 0; //Movimento do Scroll
    private boolean primeiroScroll = true;
    Socket socket; //Cria o socket
    PrintWriter writer; //Cria uma classe para receber os comandos

    private Thread tcpThread;
    private GestureDetector detector; //Detector de Gestos

    private Thread udpThread;
    private DatagramSocket udpSocket;
    private ImageView imgScreen; // Mover para escopo global para facilitar o acesso

    private void offerCommand(String command) {

        fila.offer(command);
    }


    public void moverMouse(float dX, float dY){
        if ((Math.abs(dX) > 2 || Math.abs(dY) > 2)){
            offerCommand("MOVE " + Math.round(dX) +" "+ Math.round(dY));
            lastX = posX; lastY = posY;
        }
    }

    public void moverScroll(float dY){
        if ((Math.abs(dY) > 2)){
            offerCommand("WHEEL " + Math.round(dY/4));
            lastY_scroll = mediaY_scroll;
        }
    }

    public void apertarTecla(char c){
        switch (c){
            case 'a':
                offerCommand("KEY 30");
                break;
            case 'b':
                offerCommand("KEY 48");

                break;
            case 'c':
                offerCommand("KEY 46");

                break;
            case 'd':
                offerCommand("KEY 32");

                break;
            case 'e':
                offerCommand("KEY 18");

                break;
            case 'f':
                offerCommand("KEY 33");

                break;
            case 'g':
                offerCommand("KEY 34");

                break;
            case 'h':
                offerCommand("KEY 35");

                break;
            case 'i':
                offerCommand("KEY 23");

                break;
            case 'j':
                offerCommand("KEY 36");

                break;
            case 'k':
                offerCommand("KEY 37");

                break;
            case 'l':
                offerCommand("KEY 38");

                break;
            case 'm':
                offerCommand("KEY 50");

                break;
            case 'n':
                offerCommand("KEY 49");

                break;
            case 'o':
                offerCommand("KEY 24");

                break;
            case 'p':
                offerCommand("KEY 25");

                break;
            case 'q':
                offerCommand("KEY 16");

                break;
            case 'r':
                offerCommand("KEY 19");

                break;
            case 's':
                offerCommand("KEY 31");

                break;
            case 't':
                offerCommand("KEY 20");

                break;
            case 'u':
                offerCommand("KEY 22");
                break;
            case 'v':
                offerCommand("KEY 47");
                break;
            case 'w':
                offerCommand("KEY 17");
                break;
            case 'x':
                offerCommand("KEY 45");
                break;
            case 'y':
                offerCommand("KEY 21");
                break;
            case 'z':
                offerCommand("KEY 44");
                break;
            case '1':
                offerCommand("KEY 2");
                break;
            case '2':
                offerCommand("KEY 3");
                break;
            case '3':
                offerCommand("KEY 4");
                break;
            case '4':
                offerCommand("KEY 5");
                break;
            case '5':
                offerCommand("KEY 6");
                break;
            case '6':
                offerCommand("KEY 7");
                break;
            case '7':
                offerCommand("KEY 8");
                break;
            case '8':
                offerCommand("KEY 9");
                break;
            case '9':
                offerCommand("KEY 10");
                break;
            case '0':
                offerCommand("KEY 11");
                break;
            case 'A':
                offerCommand("DOWN 42");
                offerCommand("KEY 30");
                offerCommand("UP 42");
                break;
            case 'B':
                offerCommand("DOWN 42");
                offerCommand("KEY 48");
                offerCommand("UP 42");
                break;
            case 'C':
                offerCommand("DOWN 42");
                offerCommand("KEY 46");
                offerCommand("UP 42");
                break;
            case 'D':
                offerCommand("DOWN 42");
                offerCommand("KEY 32");
                offerCommand("UP 42");
                break;
            case 'E':
                offerCommand("DOWN 42");
                offerCommand("KEY 18");
                offerCommand("UP 42");
                break;
            case 'F':
                offerCommand("DOWN 42");
                offerCommand("KEY 33");
                offerCommand("UP 42");
                break;
            case 'G':
                offerCommand("DOWN 42");
                offerCommand("KEY 34");
                offerCommand("UP 42");
                break;
            case 'H':
                offerCommand("DOWN 42");
                offerCommand("KEY 35");
                offerCommand("UP 42");
                break;
            case 'I':
                offerCommand("DOWN 42");
                offerCommand("KEY 23");
                offerCommand("UP 42");
                break;
            case 'J':
                offerCommand("DOWN 42");
                offerCommand("KEY 36");
                offerCommand("UP 42");
                break;
            case 'K':
                offerCommand("DOWN 42");
                offerCommand("KEY 37");
                offerCommand("UP 42");
                break;
            case 'L':
                offerCommand("DOWN 42");
                offerCommand("KEY 38");
                offerCommand("UP 42");
                break;
            case 'M':
                offerCommand("DOWN 42");
                offerCommand("KEY 50");
                offerCommand("UP 42");
                break;
            case 'N':
                offerCommand("DOWN 42");
                offerCommand("KEY 49");
                offerCommand("UP 42");
                break;
            case 'O':
                offerCommand("DOWN 42");
                offerCommand("KEY 24");
                offerCommand("UP 42");
                break;
            case 'P':
                offerCommand("DOWN 42");
                offerCommand("KEY 25");
                offerCommand("UP 42");
                break;
            case 'Q':
                offerCommand("DOWN 42");
                offerCommand("KEY 16");
                offerCommand("UP 42");
                break;
            case 'R':
                offerCommand("DOWN 42");
                offerCommand("KEY 19");
                offerCommand("UP 42");
                break;
            case 'S':
                offerCommand("DOWN 42");
                offerCommand("KEY 31");
                offerCommand("UP 42");
                break;
            case 'T':
                offerCommand("DOWN 42");
                offerCommand("KEY 20");
                offerCommand("UP 42");
                break;
            case 'U':
                offerCommand("DOWN 42");
                offerCommand("KEY 22");
                offerCommand("UP 42");
                break;
            case 'V':
                offerCommand("DOWN 42");
                offerCommand("KEY 47");
                offerCommand("UP 42");
                break;
            case 'W':
                offerCommand("DOWN 42");
                offerCommand("KEY 17");
                offerCommand("UP 42");
                break;
            case 'X':
                offerCommand("DOWN 42");
                offerCommand("KEY 45");
                offerCommand("UP 42");
                break;
            case 'Y':
                offerCommand("DOWN 42");
                offerCommand("KEY 21");
                offerCommand("UP 42");
                break;
            case 'Z':
                offerCommand("DOWN 42");
                offerCommand("KEY 44");
                offerCommand("UP 42");
                break;
            case ' ':
                offerCommand("DOWN 57");
                offerCommand("UP 57");
                break;
            case '+':
                offerCommand("DOWN 15"); //TAB
                offerCommand("UP 15");
                break;
            case '!':
                offerCommand("DOWN 42"); //SHIFT
                break;
            case '@':
                offerCommand("UP 42");
                break;
            case '-':
                offerCommand("DOWN 29"); //CTRL
                break;
            case '\'':
                offerCommand("UP 29");
                break;
            case ',':
                offerCommand("DOWN 56"); //ALT
                break;
            case '\"':
                offerCommand("UP 56");
                break;
            case '?':
                offerCommand("DOWN 28"); //Enter
                offerCommand("UP 28");
                break;
            case ']':
                offerCommand("DOWN 14"); //Enter
                offerCommand("UP 14");
                break;
            default:
                Log.d("IHS", "Char Estranho: " + c);
        }
    }
    private void inicializarRecepcaoTCP() {
        udpThread = new Thread(() -> {
            Socket videoSocket = null;
            InputStream inputStream = null;

            try {
                Log.d("IHS_TCP", "Iniciando cliente TCP de vídeo na porta 6666...");
                videoSocket = new Socket(socket.getInetAddress().getHostAddress(), 6666);
                videoSocket.setTcpNoDelay(true);
                inputStream = videoSocket.getInputStream();

                byte[] headerBuffer = new byte[12];
                byte[] rawFrameBuffer = null;
                int currentAllocatedSize = 0;
                int[] linearPixels = null;

                while (!Thread.currentThread().isInterrupted()) {
                    // 1. Lê o cabeçalho descritivo do Frame (12 bytes)
                    int readHeaderBytes = 0;
                    while (readHeaderBytes < 12) {
                        int ret = inputStream.read(headerBuffer, readHeaderBytes, 12 - readHeaderBytes);
                        if (ret == -1) throw new IOException("Conexão fechada pelo Kernel ao ler cabeçalho.");
                        readHeaderBytes += ret;
                    }

                    java.nio.ByteBuffer wrapped = java.nio.ByteBuffer.wrap(headerBuffer).order(java.nio.ByteOrder.LITTLE_ENDIAN);
                    final int width = wrapped.getInt();
                    final int height = wrapped.getInt();
                    final int bpp = wrapped.getInt();
                    final int frameSize = width * height * bpp;

                    if (rawFrameBuffer == null || currentAllocatedSize != frameSize) {
                        rawFrameBuffer = new byte[frameSize];
                        linearPixels = new int[width * height];
                        currentAllocatedSize = frameSize;
                    }

                    // 2. Lê os pixels em formato RGB565
                    int readPixelsBytes = 0;
                    while (readPixelsBytes < frameSize) {
                        int ret = inputStream.read(rawFrameBuffer, readPixelsBytes, frameSize - readPixelsBytes);
                        if (ret == -1) throw new IOException("Conexão fechada pelo Kernel no meio do frame.");
                        readPixelsBytes += ret;
                    }

                    // 3. De-Tiling mantendo o tamanho físico do Tile (128x8)
                    // CORREÇÃO: O Tile-X continua com 128 pixels de largura física!
                    int tileWidthPixels = 128;
                    int tileHeightPixels = 8;
                    int tilesX = width / tileWidthPixels;
                    int tilesY = height / tileHeightPixels;

                    int rawIdx = 0;

                    for (int ty = 0; ty < tilesY; ty++) {
                        for (int tx = 0; tx < tilesX; tx++) {

                            for (int yLine = 0; yLine < tileHeightPixels; yLine++) {
                                int targetY = (ty * tileHeightPixels) + yLine;
                                int targetXStart = tx * tileWidthPixels;
                                int targetOffset = (targetY * width) + targetXStart;

                                for (int xCol = 0; xCol < tileWidthPixels; xCol++) {
                                    // Lê os 2 bytes do pixel RGB565 (Little Endian)
                                    int b1 = rawFrameBuffer[rawIdx++] & 0xFF;
                                    int b2 = rawFrameBuffer[rawIdx++] & 0xFF;
                                    int rgb565 = (b2 << 8) | b1;

                                    // Extrai os componentes R, G, B expandindo para 8 bits
                                    int r5 = (rgb565 >> 11) & 0x1F;
                                    int g6 = (rgb565 >> 5) & 0x3F;
                                    int b5 = rgb565 & 0x1F;

                                    // Escala de 5/6 bits de volta para 8 bits (0-255)
                                    int r = (r5 * 255) / 31;
                                    int g = (g6 * 255) / 63;
                                    int b = (b5 * 255) / 31;

                                    // Monta o pixel RGBA final para exibição no Android
                                    linearPixels[targetOffset + xCol] = (0xFF << 24) | (r << 16) | (g << 8) | b;
                                }
                            }
                        }
                    }

                    // 4. Desenha na tela do dispositivo
                    final int[] finalPixels = linearPixels;
                    final Bitmap bitmapFrame = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                    bitmapFrame.setPixels(finalPixels, 0, width, 0, 0, width, height);

                    runOnUiThread(() -> {
                        imgScreen.setImageBitmap(bitmapFrame);
                    });
                }

            } catch (IOException e) {
                Log.e("IHS_TCP", "Erro no processamento da conexão de vídeo: ", e);
            } finally {
                try {
                    if (inputStream != null) inputStream.close();
                    if (videoSocket != null) videoSocket.close();
                } catch (IOException e) {
                    Log.e("IHS_TCP", "Erro ao fechar conexões secundárias", e);
                }
            }
        });
        udpThread.start();
    }
    private void conectarServidor(String ip, int porta, String senha) {
        if (tcpThread != null) {
            tcpThread.interrupt();
        }
        if (udpThread != null) {
            udpThread.interrupt();
        }

        tcpThread = new Thread(() -> {
            try {
                Log.d("IHS", "Tentando conectar em " + ip + ":" + porta);
                socket = new Socket(ip, porta);
                Log.d("IHS", "Socket criado com sucesso!");

                writer = new PrintWriter(socket.getOutputStream(), true);
                BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));

                writer.println("AUTH " + senha);

                String ipCelular = socket.getLocalAddress().getHostAddress();
                Log.d("IHS", "Meu IP do celular enviado ao Kernel: " + ipCelular);
                writer.println("CONNECT_UDP " + ipCelular + " 6666");

                String respostaServidor = reader.readLine();
                Log.d("IHS", "Resposta recebida do Kernel: " + respostaServidor);

                if (respostaServidor != null && respostaServidor.contains("UDP_OK")) {
                    runOnUiThread(() -> {
                        Snackbar snackbar = Snackbar.make(findViewById(R.id.main), "Conexão TCP estabelecida com sucesso!", Snackbar.LENGTH_LONG);
                        snackbar.setAnimationMode(Snackbar.ANIMATION_MODE_FADE);
                        snackbar.show();
                    });

                    // Inicia o canal receptor UDP após o handshake TCP bem sucedido
                    inicializarRecepcaoTCP();
                }

                while (!Thread.currentThread().isInterrupted()) {
                    String sendCMD = fila.take();
                    writer.println(sendCMD);
                }
            } catch (IOException e) {
                Log.e("IHS", "Erro de IO na conexão", e);
            } catch (InterruptedException e) {
                Log.d("IHS", "Thread de conexão interrompida de forma limpa");
            }
        });
        tcpThread.start();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);

        FloatingActionButton btnKeyboard = findViewById(R.id.btnKeyboard);
        EditText hiddenEdit = findViewById(R.id.hiddenEdit);

        // Inicializa o atributo global
        imgScreen = findViewById(R.id.imgScreen);
        imgScreen.setImageResource(R.drawable.teste);

        btnKeyboard.setOnClickListener(v -> {

            hiddenEdit.requestFocus();

            InputMethodManager imm =
                    (InputMethodManager)getSystemService(INPUT_METHOD_SERVICE);

            imm.showSoftInput(hiddenEdit, InputMethodManager.SHOW_IMPLICIT);
        });
        hiddenEdit.addTextChangedListener(new TextWatcher() {

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {}

            @Override
            public void afterTextChanged(Editable s) {

                if (s.length() == 0)
                    return;

                char c = s.charAt(0);

                apertarTecla(c);

                hiddenEdit.post(() -> {
                    hiddenEdit.setText("");
                });
            }
        });

        FloatingActionButton btnDisconnect = findViewById(R.id.btnDisconnect);
        btnDisconnect.setOnClickListener(v -> {

            Log.d("IHS", "Desconectando...");
            try {
                if (writer != null) writer.close();
                if (socket != null) socket.close();
                if (udpSocket != null && !udpSocket.isClosed()) udpSocket.close();
                if (tcpThread != null) tcpThread.interrupt();
                if (udpThread != null) udpThread.interrupt();
            } catch (IOException e) {
                Log.e("IHS", "Erro ao fechar conexões", e);
            }
        });

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });







        detector = new GestureDetector(
                this,
                new GestureDetector.SimpleOnGestureListener() {


                    @Override
                    public boolean onDoubleTap(MotionEvent event) {

                        offerCommand("LBC");
                        Log.d("IHS", "Double Tap Left");
                        return true;
                    }

                    @Override
                    public void onLongPress(MotionEvent event){

                        Log.d("IHS", "Segurou a Tela");
                        offerCommand("RBC");

                    }
                }
        );

        findViewById(R.id.main).setOnTouchListener((v, event) -> { //Recebe eventos do meu celular


            if (event.getPointerCount() == 1) {
                posX = event.getX();
                posY = event.getY();
            }
            if (event.getPointerCount() == 2){
                mediaY_scroll = (event.getY(0)+ event.getY(1))/2.0f;
            }

            detector.onTouchEvent(event);

            switch (event.getActionMasked()) {

                case MotionEvent.ACTION_DOWN:

                    lastX = posX;
                    lastY = posY;
                    lastY_scroll = mediaY_scroll;
                    break;


                case MotionEvent.ACTION_MOVE:

                    if (event.getPointerCount() == 1) {
                        dX = posX - lastX;
                        dY = posY - lastY;
                        moverMouse(dX, dY);
                    }
                    if (event.getPointerCount() == 2){
                        moverScroll(mediaY_scroll - lastY_scroll);
                    }
                    break;
            }

            return true;
        });



        FloatingActionButton btnConnect = findViewById(R.id.btnConnect);
        btnConnect.setOnClickListener(v -> {
            // Cria um layout linear vertical para empilhar as caixas de texto no Pop-Up
            LinearLayout layoutDialog = new LinearLayout(MainActivity.this);
            layoutDialog.setOrientation(LinearLayout.VERTICAL);
            layoutDialog.setPadding(50, 40, 50, 10);

            final EditText inputIp = new EditText(MainActivity.this);
            inputIp.setHint("Endereço IP (Ex: 10.0.0.181)");
            inputIp.setInputType(InputType.TYPE_CLASS_TEXT);
            layoutDialog.addView(inputIp);

            final EditText inputPorta = new EditText(MainActivity.this);
            inputPorta.setHint("Porta TCP (Ex: 5558)");
            inputPorta.setInputType(InputType.TYPE_CLASS_NUMBER);
            layoutDialog.addView(inputPorta);

            final EditText inputSenha = new EditText(MainActivity.this);
            inputSenha.setHint("Senha de Autenticação");
            inputSenha.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
            layoutDialog.addView(inputSenha);

            // Constrói o AlertDialog com os inputs criados
            new AlertDialog.Builder(MainActivity.this)
                    .setTitle("Configurar Conexão IHS")
                    .setView(layoutDialog)
                    .setPositiveButton("Conectar", (dialog, which) -> {
                        String ip = inputIp.getText().toString().trim();
                        String portaStr = inputPorta.getText().toString().trim();
                        String senha = inputSenha.getText().toString().trim();

                        if (!ip.isEmpty() && !portaStr.isEmpty() && !senha.isEmpty()) {
                            int porta = Integer.parseInt(portaStr);

                            // Dispara a conexão usando as variáveis fornecidas pelo usuário
                            conectarServidor(ip, porta, senha);
                        }
                    })
                    .setNegativeButton("Cancelar", null)
                    .show();
        });
   }
}
