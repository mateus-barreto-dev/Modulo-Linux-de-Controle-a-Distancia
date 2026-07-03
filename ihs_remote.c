#include <linux/module.h> //definições de módulos
#include <linux/kernel.h> //funções do kernel
#include <linux/init.h> // __init e __exit
#include <linux/input.h> //mover o mouse
#include <linux/delay.h> //Dar um tempo para o mouse ser carregado
#include <linux/proc_fs.h> //Mexer com o proc
#include <linux/string.h>
#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>
#include <linux/kthread.h>

struct input_dev *mouse_dev;
struct input_dev *keyboard_dev;
struct socket *server_socket;
struct socket *client_socket; //Conexões com o socket
struct proc_dir_entry *proc_entry;
struct task_struct *tcp_thread;

static int verificar_nullidade(void){
    if(mouse_dev == NULL){
        printk(KERN_ERR "Erro: mouse removido\n");
        return 1;
    }
    return 0;
} 

//-----------------------Mouse------------------------------------
static int move_mouse(int dx, int dy) //static faz a memória se manter durante todo o programa
{
    if(verificar_nullidade()){
        return 0;
    }



    input_report_rel(mouse_dev, REL_X, dx);
    input_report_rel(mouse_dev, REL_Y, dy);
    input_sync(mouse_dev);

    return 0;
}

static int click_mouse(int button)
{
    if(verificar_nullidade()){
        return 0;
    }


    if(button == 0)
    {
        input_report_key(mouse_dev, BTN_LEFT, 1);
        input_sync(mouse_dev);
        
        input_report_key(mouse_dev, BTN_LEFT, 0);
        input_sync(mouse_dev);
        printk(KERN_INFO "Fui Excutado");
    }
    else
    {
        input_report_key(mouse_dev, BTN_RIGHT, 1);
        input_sync(mouse_dev);
        
        input_report_key(mouse_dev, BTN_RIGHT, 0);
        input_sync(mouse_dev);
        printk(KERN_INFO "Fui Excutado");

    }
    return 0;
}

static int wheel_mouse(int value)
{
    printk(KERN_INFO "Teste %d", value);

    input_report_rel(mouse_dev, REL_WHEEL, value);
    input_sync(mouse_dev);
  
    
    return 0;
}
//----------------------------------------------------------------------------------
//-------------------------------Teclado--------------------------------------------
static int ascii_to_keycode(char c)
{
    switch(c)
    {
        case 'a': return KEY_A;
        case 'b': return KEY_B;
        case 'c': return KEY_C;
        case 'd': return KEY_D;
        case 'e': return KEY_E;
        case 'f': return KEY_F;
        case 'g': return KEY_G;
        case 'h': return KEY_H;
        case 'i': return KEY_I;
        case 'j': return KEY_J;
        case 'k': return KEY_K;
        case 'l': return KEY_L;
        case 'm': return KEY_M;
        case 'n': return KEY_N;
        case 'o': return KEY_O;
        case 'p': return KEY_P;
        case 'q': return KEY_Q;
        case 'r': return KEY_R;
        case 's': return KEY_S;
        case 't': return KEY_T;
        case 'u': return KEY_U;
        case 'v': return KEY_V;
        case 'w': return KEY_W;
        case 'x': return KEY_X;
        case 'y': return KEY_Y;
        case ' ': return KEY_SPACE;
    }

    return -1;
}




static int key_press(int keycode)
{
    input_report_key(keyboard_dev, keycode, 1);
    input_sync(keyboard_dev);

    input_report_key(keyboard_dev, keycode, 0);
    input_sync(keyboard_dev);

    return 0;
}

static int key_down(int keycode)
{
    input_report_key(keyboard_dev, keycode, 1);
    input_sync(keyboard_dev);

    return 0;
}

static int key_up(int keycode)
{

    input_report_key(keyboard_dev, keycode, 0);
    input_sync(keyboard_dev);

    return 0;
}

static int type_text(char *text)
{
    int i;
    int keycode;

    for(i = 0; text[i] != '\0'; i++)
    {
        keycode = ascii_to_keycode(text[i]);

        if(keycode >= 0)
        {
            msleep(2);
            key_press(keycode);
            
        }
    }

    return 0;
}
//---------------------------------------------------------------------------------


static int parser_command(char *kbuffer){
     int dx, dy, keycode, wheel_value; char texto[128];
    if(kbuffer[0] == 'M'){
            if (sscanf(kbuffer, "MOVE %d %d", &dx, &dy) == 2)
            {
                printk(KERN_INFO "Recebi: %s, valor de dx = %d, valor de dy = %d\n", kbuffer, dx, dy);
                move_mouse(dx,dy);
            }
            else
            {
                printk(KERN_INFO "Número de Argumentos Inválido");
            }       
    }
    else if(kbuffer[0] == 'K'){
        if(sscanf(kbuffer,"KEY %d", &keycode) == 1) // Para um comando do tipo "LBC" Left Button Click
        {
            key_press(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'W'){
        if(sscanf(kbuffer,"WHEEL %d", &wheel_value) == 1) // Para um comando do tipo "LBC" Left Button Click
        {
            wheel_mouse(wheel_value);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'D'){
        if(sscanf(kbuffer,"DOWN %d", &keycode) == 1){ // Apertar um botão
            key_down(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'T'){
        if(sscanf(kbuffer,"TEXT %127s", texto) == 1) // Para um comando do tipo "LBC" Left Button Click
        {
            type_text(texto);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'U'){
        if(sscanf(kbuffer,"UP %d", &keycode) == 1) // Soltar um botão
        {
            key_up(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(strcmp(kbuffer,"RBC") == 0) // Para um comando do tipo "RBC" Right Button Click
    {

        click_mouse(1);
    }
    else if(strcmp(kbuffer,"LBC") == 0) // Para um comando do tipo "LBC" Left Button Click
    {

        click_mouse(0);
    }
    else{
        printk(KERN_INFO "Comando Inválido");
    }


    return 0;
}

static ssize_t proc_write( //Escreve no proc
    struct file *file,
    const char __user *buffer,
    size_t count,
    loff_t *pos)
{
    char kbuffer[128];
    int len = 0;
    len = (count > 127) ? 127 : count;

    if (copy_from_user(kbuffer, buffer, len)) { 
        printk(KERN_ERR "IHS: erro no copy_from_user\n"); 
        return -EFAULT; 
    }

    if(len > 0 && kbuffer[len - 1] == '\n')
    {
        kbuffer[len - 1] = '\0';
    }


    kbuffer[len] = '\0';

    parser_command(kbuffer);

    

    return count;

}

static const struct proc_ops ihs_proc_ops = { //Define a tabela de possibilidades para o kernel
    .proc_write = proc_write,
};

static int tcp_server_thread(void *data)
{
    int ret;
    struct kvec vec; //descreve para onde os bytes serão copiados.
    struct msghdr msg = {0}; //Estrutura da mensagem

    char buffer[128]; //buffer para colocar as mensagens

    vec.iov_base = buffer;
    vec.iov_len  = sizeof(buffer) - 1;

    printk(KERN_INFO "server_socket = %px\n", server_socket);

    if (!server_socket) {
        printk(KERN_ERR "server_socket é NULL!\n");
        return -EINVAL;
    }



    while(!kthread_should_stop())
    {

        ret = kernel_accept(
            server_socket,
            &client_socket,
            0
        );

        if(ret){

            if(kthread_should_stop())
                break;

            continue;
        }

        printk(KERN_INFO "Cliente conectado!\n");

        while(!kthread_should_stop())
        {
            ret = kernel_recvmsg(
                client_socket,
                &msg,
                &vec,
                1,
                sizeof(buffer) - 1,
                0
            );

            if(ret <= 0)
            {
                printk(KERN_INFO "Cliente desconectou\n");
                break;
            }

            buffer[ret] = '\0';

            char *ptr = buffer;
            char *cmd;

            while ((cmd = strsep(&ptr, "\n")) != NULL)
            {
                size_t len = strlen(cmd);
            
                if (len == 0)
                    continue;
            
                if (cmd[len-1] == '\r')
                    cmd[len-1] = '\0';
            
                printk(KERN_INFO "%s\n", cmd);
                parser_command(cmd);
            }
        }

        if (client_socket)
        {
            sock_release(client_socket);
            client_socket = NULL;
        }
    }
    printk(KERN_INFO "Thread TCP encerrada\n");
    return 0;
}




static int __init create_devices(void)
{
    printk(KERN_INFO "IHS: Modulo carregado!\n"); //Vai printar quando  módulo for carregado no log do kernel
    int ret = 0;

    //------------------Cria o Mouse Virtual---------------------
    
    mouse_dev = input_allocate_device();

    if (!mouse_dev)
    {
        printk(KERN_ERR "Falha ao criar dispositivo\n");
        return -ENOMEM;
    }

    mouse_dev->name = "IHS Virtual Mouse";

    set_bit(REL_WHEEL, mouse_dev->relbit);
    set_bit(EV_REL, mouse_dev->evbit); //Permite movimento relativo
    set_bit(REL_X, mouse_dev->relbit); //Relativo em X
    set_bit(REL_Y, mouse_dev->relbit); //Reativo em Y

    set_bit(EV_KEY, mouse_dev->evbit); //Permite acesso a teclas
    set_bit(BTN_LEFT, mouse_dev->keybit); //Permite o uso de botão esquerdo
    set_bit(BTN_RIGHT, mouse_dev->keybit); //direito
    __set_bit(BTN_MOUSE, mouse_dev->keybit);

    mouse_dev->id.bustype = BUS_USB;
    mouse_dev->id.vendor  = 0x1234;
    mouse_dev->id.product = 0x5678;
    mouse_dev->id.version = 1;

    ret = input_register_device(mouse_dev);

    if (ret)
    {
        printk(KERN_ERR "Falha ao registrar dispositivo\n");
        input_free_device(mouse_dev);
        return ret;
    }
    //---------------------------------------------------------------------

    //---------------------Cria o Teclado Virtual--------------------------
    keyboard_dev = input_allocate_device();

    if (!keyboard_dev)
    {
        printk(KERN_ERR "Falha ao criar dispositivo\n");
        return -ENOMEM;
    }

    keyboard_dev->name = "IHS Virtual Keyboard";

    set_bit(EV_KEY, keyboard_dev->evbit); //Permite acesso a teclas

    for(int i = 0; i < KEY_MAX; i++) 
        __set_bit(i, keyboard_dev->keybit); //Permite alocar todas as teclas

    keyboard_dev->id.bustype = BUS_USB;
    keyboard_dev->id.vendor  = 0x1234;
    keyboard_dev->id.product = 0x5678;
    keyboard_dev->id.version = 1;

    ret = input_register_device(keyboard_dev);

    if (ret)
    {
        printk(KERN_ERR "Falha ao registrar dispositivo\n");
        input_free_device(keyboard_dev);
        return ret;
    }
    printk(KERN_ERR "Sucesso ao Criar Teclado\n");
    msleep(200);
    //-------------------------------------------------------

    proc_entry = proc_create("ihs_remote", 0666, NULL, &ihs_proc_ops); //cria o proc/ihs_remote dando permissao de escrita e leitura
    if (!proc_entry)
    {
        printk(KERN_ERR "Falha ao criar /proc/ihs_remote\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "IHS: /proc/ihs_remote criado\n");


    //----------------------Cria o Socket-----------------------
    ret = sock_create_kern(
        &init_net, //Usar a rede local do linux
        AF_INET, //IPV4
        SOCK_STREAM, //Define o que será recebido, Stream => TCP
        IPPROTO_TCP, //Define o Protocolo => TCP
        &server_socket //Atribui a variável server_socket
    );

    if(ret)
    {
        printk(KERN_ERR "Erro ao criar socket\n");
        return ret;
    }
    printk(KERN_INFO "Socket TCP criado com sucesso\n"); //Vai indicar que a criação do socket foi bem sucedida
    msleep(50);
    //-----------------------------------------------------------
    //------------------Atribui Endereço ao Socket---------------
    struct sockaddr_in addr; //Cria a estrutura de endereço
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET; //Indica a família IPV4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); //Diz que pode receber conexões de qualquer canto
    addr.sin_port = htons(5558); //Indica a porta
    msleep(50);
    ret = kernel_bind( //Faz a ligação com o socket
        server_socket,
        (struct sockaddr *)&addr,
        sizeof(addr)
    );
    msleep(50);
    if(ret)
    {
        printk(KERN_ERR "Erro no bind: %d\n", ret);

        sock_release(server_socket); //Libera osocket caso dê algo errado
        server_socket = NULL;

        return ret;
    }

    printk(KERN_INFO "Bind realizado com sucesso\n");    
    //-----------------------------------------------------------
    //--------------Faz o Socket Aceitar Conexões----------------

    msleep(50);
    ret = kernel_listen(server_socket, 5); //5 amanho máximo da fila de solicitações
    msleep(50);
    if(ret)
    {
        sock_release(server_socket);
        server_socket = NULL;
        
        printk(KERN_ERR "Erro no listen\n");
        return ret;
    }

    printk(KERN_INFO "Listen realizado com sucesso\n");


    //-----------------------------------------------------------
    msleep(50);
    tcp_thread = kthread_run(
        tcp_server_thread,
        NULL,
        "ihs_tcp_server"
    );
    msleep(50);
    if(IS_ERR(tcp_thread))
    {
        printk(KERN_ERR "Erro ao criar thread TCP\n");
        return PTR_ERR(tcp_thread);
    }

    return 0;
}





static void __exit destroy_devices(void)
{
    printk(KERN_INFO "IHS: Modulo removido!\n"); //Vai printar quando o módulo for removido no log do kernel
    
    if(client_socket)
    {
        sock_release(client_socket);
        client_socket = NULL;
    }

    if(tcp_thread)
    {
        kthread_stop(tcp_thread);
        tcp_thread = NULL;
    }

    if(server_socket)
    {
        sock_release(server_socket);
        server_socket = NULL;
    }

    

    if(proc_entry)
        proc_remove(proc_entry);

    if(mouse_dev)
        input_unregister_device(mouse_dev);

    if(keyboard_dev)
        input_unregister_device(keyboard_dev);
}

module_init(create_devices); //Quando carregar o módulo, chame i=__init
module_exit(destroy_devices); //Quando remover, chame __exit

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seu Nome");
MODULE_DESCRIPTION("Primeiro modulo Linux");
