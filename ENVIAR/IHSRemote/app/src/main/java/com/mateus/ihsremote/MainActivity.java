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
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
//----------------------------------------------------
//----------------Teclado-----------------------------
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import android.view.KeyEvent;
//----------------------------------------------------

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
        }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) { //Cria tudo no aplicativo
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this); //ocupar a tela toda
        setContentView(R.layout.activity_main); //Carregar o Arquivo xml
        FloatingActionButton btnKeyboard = findViewById(R.id.btnKeyboard);
        EditText hiddenEdit = findViewById(R.id.hiddenEdit);

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

            Log.d("IHS", "Desconectando");

            try {

                if (writer != null)
                    writer.close();

                if (socket != null)
                    socket.close();

                if (tcpThread != null)
                    tcpThread.interrupt();

            } catch (IOException e) {
                Log.e("IHS", "Erro ao fechar conexão", e);
            }
            finish();
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



        new Thread(() -> { //Cria uma thread para ficar executando a parte de conexão

            try {

                Log.d("IHS", "Entrou no try");

                socket = new Socket("10.0.0.181", 5558); //bind do socket com meu hostname e porta

                Log.d("IHS", "Socket criado");

                writer = new PrintWriter(socket.getOutputStream(), true); //true faz com que assim que escrever envie para a saída
                writer.println("AUTH IHSREMOTE2026");

                Log.d("IHS", "Writer criado");

                while (!Thread.currentThread().isInterrupted()) {
                    String sendCMD = fila.take();
                    writer.println(sendCMD);
                }
            } catch (IOException e) {
                Log.e("IHS", "Erro de IO", e);
            } catch (InterruptedException e) {
                Log.e("IHS", "Erro de Runtime", e);
                throw new RuntimeException(e);
            }

        }).start();



















   }
}