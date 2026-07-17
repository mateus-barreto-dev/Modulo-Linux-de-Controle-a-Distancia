#include <linux/module.h> // definições de módulos
#include <linux/kernel.h> // funções do kernel
#include <linux/init.h>   // __init e __exit
#include <linux/input.h>  // mover o mouse
#include <linux/delay.h>  // dar um tempo para o mouse ser carregado
#include <linux/proc_fs.h> // mexer com o proc
#include <linux/string.h>
#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>
#include <linux/kthread.h>
#include <linux/inet.h>
#include <linux/fb.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pci.h>       // CORREÇÃO: Necessário para pci_get_class e struct pci_dev
#include <linux/iosys-map.h> // CORREÇÃO: Necessário para struct iosys_map

// ---------------Captura da tela---------------------
#include <drm/drm_drv.h>
#include <drm/drm_device.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_client.h>
#include <drm/drm_plane.h>
#include <drm/drm_crtc.h>
#include <drm/drm_gem.h>
// ---------------------------------------------------

/* ====== CONFIGURAÇÕES ====== */
#define TCP_SEND_CHUNK      32768
#define FRAME_DELAY_MS      33

struct input_dev *mouse_dev;
struct input_dev *keyboard_dev;

struct proc_dir_entry *proc_entry;
struct task_struct *tcp_thread;

struct socket *server_socket;
struct socket *client_socket; // Conexões com o socket de controle

// Rede - Vídeo (6666)
static struct socket *video_server_socket = NULL;
static struct socket *video_client_socket = NULL;
static struct task_struct *video_thread = NULL;

// DRM (Captura)
static struct drm_device *global_drm_dev = NULL;

static int verificar_nullidade(void){
    if(mouse_dev == NULL){
        printk(KERN_ERR "Erro: mouse removido\n");
        return 1;
    }
    return 0;
} 

//-----------------------Mouse------------------------------------
static int move_mouse(int dx, int dy)
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
        printk(KERN_INFO "Fui Executado");
    }
    else
    {
        input_report_key(mouse_dev, BTN_RIGHT, 1);
        input_sync(mouse_dev);
        
        input_report_key(mouse_dev, BTN_RIGHT, 0);
        input_sync(mouse_dev);
        printk(KERN_INFO "Fui Executado");
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

//---------------------------------IMAGENS-----------------------------------------


/* ====== FUNÇÃO DE ENVIO TCP (VÍDEO) ====== */
static int enviar_imagem_tcp(struct socket *sock, unsigned char *imagem_buffer, size_t tamanho_total) {
    struct msghdr msg;
    struct kvec vec;
    int ret;
    size_t enviado = 0;
    size_t restante = tamanho_total;

    if (!sock) return -EINVAL;

    memset(&msg, 0, sizeof(msg));

    while (restante > 0 && !kthread_should_stop()) {
        size_t chunk_size = (restante > TCP_SEND_CHUNK) ? TCP_SEND_CHUNK : restante;

        vec.iov_base = imagem_buffer + enviado;
        vec.iov_len = chunk_size;

        ret = kernel_sendmsg(sock, &msg, &vec, 1, chunk_size);
        if (ret < 0) {
            return ret; // Conexão fechada ou erro
        }

        enviado += ret;
        restante -= ret;
    }
    return 0;
}

/* ====== THREAD DE CAPTURA E STREAMING (VÍDEO) ====== */
static int video_stream_thread(void *data) {
    struct drm_device *drm = global_drm_dev;
    struct drm_crtc *crtc;
    struct drm_framebuffer *fb = NULL;
    struct drm_gem_object *gem;
    struct iosys_map map;
    int ret, ret_vmap;
    unsigned char *buffer_envio = NULL;
    size_t tamanho_alocado = 0;

    printk(KERN_INFO "IHS-DRM: Aguardando conexao do App (Video) na porta 6666...\n");

    // O kernel bloqueia aqui até o celular conectar no IP local porta 6666
    ret = kernel_accept(video_server_socket, &video_client_socket, 0);
    if (ret < 0) {
        printk(KERN_INFO "IHS-DRM: kernel_accept (video) interrompido.\n");
        goto finalizar_thread;
    }
    printk(KERN_INFO "IHS-DRM: App conectado para Video!\n");

    // Loop de streaming contínuo
    while (!kthread_should_stop()) {
        fb = NULL;

        // Encontra o CRTC primário ativo
        list_for_each_entry(crtc, &drm->mode_config.crtc_list, head) {
            if (crtc->primary && crtc->primary->state && crtc->primary->state->fb) {
                fb = crtc->primary->state->fb;
                break;
            }
        }

        if (!fb || !fb->obj[0]) {
            msleep(100);
            continue;
        }

        int real_width = fb->width;
        int real_height = fb->height;
        int real_pitch = fb->pitches[0];

        // Agora o envio será em RGB565 (2 bytes por pixel)
        int bpp_envio = 2;
        size_t tamanho_envio = real_width * real_height * bpp_envio;

        // Aloca ou realoca o buffer de envio conforme o tamanho da tela do host
        if (!buffer_envio || tamanho_alocado != tamanho_envio) {
            if (buffer_envio) vfree(buffer_envio);
            buffer_envio = vmalloc(tamanho_envio);
            if (!buffer_envio) continue;
            tamanho_alocado = tamanho_envio;
        }

        gem = fb->obj[0];
        ret_vmap = drm_gem_vmap(gem, &map);

        if (ret_vmap == 0) {
            unsigned char *pixel_ptr = map.vaddr;
            if (pixel_ptr) {
                int y, x;
                uint16_t *dst_ptr = (uint16_t *)buffer_envio;

                for (y = 0; y < real_height; y++) {
                    // Aponta para o início da linha no framebuffer original (BGRA - 4 bytes)
                    unsigned char *src_line = pixel_ptr + (y * real_pitch);

                    for (x = 0; x < real_width; x++) {
                        // Extrai componentes B, G, R individuais (4 bytes por pixel no original)
                        unsigned char b = src_line[x * 4 + 0];
                        unsigned char g = src_line[x * 4 + 1];
                        unsigned char r = src_line[x * 4 + 2];

                        // Compacta para RGB565 (5 bits Red, 6 bits Green, 5 bits Blue)
                        uint16_t r5 = (r >> 3) & 0x1F;
                        uint16_t g6 = (g >> 2) & 0x3F;
                        uint16_t b5 = (b >> 3) & 0x1F;

                        // Monta o pixel de 16 bits resultante
                        uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;

                        // Guarda no buffer de transmissão
                        dst_ptr[y * real_width + x] = rgb565;
                    }
                }
            }
            drm_gem_vunmap(gem, &map);

            if (pixel_ptr) {
                // Enviamos o cabeçalho indicando a resolução nativa e o novo bpp = 2 (RGB565)
                u32 header[3] = { (u32)real_width, (u32)real_height, (u32)bpp_envio };
                struct msghdr msg;
                struct kvec vec;

                memset(&msg, 0, sizeof(msg));
                vec.iov_base = header;
                vec.iov_len = sizeof(header);

                // Envia cabeçalho descritor
                ret = kernel_sendmsg(video_client_socket, &msg, &vec, 1, sizeof(header));
                if (ret >= 0) {
                    // Envia a carga de pixels convertidos
                    ret = enviar_imagem_tcp(video_client_socket, buffer_envio, tamanho_envio);
                    if (ret < 0) break; // Sai do loop se o celular desconectar
                } else {
                    break;
                }
            }
        }
        msleep(FRAME_DELAY_MS);
    }

finalizar_thread:
    if (buffer_envio) vfree(buffer_envio);

    // Garante a destruição física do socket do cliente para evitar portas presas
    if (video_client_socket) {
        sock_release(video_client_socket);
        video_client_socket = NULL;
    }

    video_thread = NULL;
    printk(KERN_INFO "IHS-DRM: Thread de video finalizada de forma segura.\n");
    return 0;
}

static void iniciar_thread_video(void) {
    // Se a thread de vídeo antiga já morreu e está NULL, criamos uma nova
    if (video_thread == NULL) {
        printk(KERN_INFO "IHS-DRM: Reiniciando thread de captura de vídeo...\n");
        
        // Se sobrou algum socket de cliente antigo aberto por erro, garante o fechamento
        if (video_client_socket) {
            sock_release(video_client_socket);
            video_client_socket = NULL;
        }

        video_thread = kthread_run(video_stream_thread, NULL, "ihs_video_thread");
        if (IS_ERR(video_thread)) {
            printk(KERN_ERR "IHS-DRM: Erro ao recriar a thread de vídeo!\n");
            video_thread = NULL;
        }
    } else {
        printk(KERN_INFO "IHS-DRM: Thread de vídeo já está ativa.\n");
    }
}

//---------------------------------------------------------------------------------

/* ====== FUNÇÃO AUXILIAR: CRIAR SOCKET SERVIDOR ====== */
static int criar_servidor_tcp(struct socket **sock, int porta) {
    int ret, reuse = 1;
    struct sockaddr_in addr;

    ret = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, sock);
    if(ret) return ret;

    ret = sock_setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR, KERNEL_SOCKPTR(&reuse), sizeof(reuse));
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(porta);

    ret = kernel_bind(*sock, (struct sockaddr *)&addr, sizeof(addr));
    if(ret) {
        sock_release(*sock);
        *sock = NULL;
        return ret;
    }

    ret = kernel_listen(*sock, 5);
    if(ret) {
        sock_release(*sock);
        *sock = NULL;
        return ret;
    }
    return 0;
}
//------------------------------------------------------------------------------

static int parser_command(char *kbuffer){
    int dx, dy, keycode, wheel_value; 
    char texto[128];

    if(kbuffer[0] == 'M'){
        if (sscanf(kbuffer, "MOVE %d %d", &dx, &dy) == 2)
        {
            move_mouse(dx,dy);
        }
        else
        {
            printk(KERN_INFO "Número de Argumentos Inválido");
        }       
    }
    else if(kbuffer[0] == 'K'){
        if(sscanf(kbuffer,"KEY %d", &keycode) == 1)
        {
            key_press(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'W'){
        if(sscanf(kbuffer,"WHEEL %d", &wheel_value) == 1)
        {
            wheel_mouse(wheel_value);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'D'){
        if(sscanf(kbuffer,"DOWN %d", &keycode) == 1){
            key_down(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'T'){
        if(sscanf(kbuffer,"TEXT %127s", texto) == 1)
        {
            type_text(texto);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(kbuffer[0] == 'U'){
        if(sscanf(kbuffer,"UP %d", &keycode) == 1)
        {
            key_up(keycode);
        }
        else{
            printk(KERN_INFO "Número de Argumentos Inválido");
        }
    }
    else if(strcmp(kbuffer,"RBC") == 0)
    {
        click_mouse(1);
    }
    else if(strcmp(kbuffer,"LBC") == 0)
    {
        click_mouse(0);
    }
    else if (kbuffer[0] == 'C') { // Comando: CONNECT_UDP %s %d
        int porta_udp_cliente = 0;
        char ip_cliente_str[32] = {0};

        printk(KERN_INFO "IHS: Identificado comando de conexão: %s\n", kbuffer);

        if (sscanf(kbuffer, "CONNECT_UDP %s %d", ip_cliente_str, &porta_udp_cliente) == 2) {
            // Prepara e envia a resposta de sucesso "UDP_OK\n" de volta pelo socket de controle (TCP)
            struct kvec msg_vec;
            struct msghdr msg = {0};
            char *resposta = "UDP_OK\n";

            msg_vec.iov_base = resposta;
            msg_vec.iov_len = strlen(resposta);

            if (client_socket) {
                kernel_sendmsg(client_socket, &msg, &msg_vec, 1, msg_vec.iov_len);
                printk(KERN_INFO "IHS: Handshake 'UDP_OK' enviado para o celular. Vídeo liberado!\n");
                
                // GATILHO RECONEXÃO: Garante que a thread de vídeo está de pé para receber o celular
                iniciar_thread_video();
            }
        } else {
            printk(KERN_ERR "IHS: Erro ao parsear o handshake CONNECT_UDP\n");
        }
    }
    else{
        printk(KERN_INFO "Comando Inválido");
    }

    return 0;
}

static ssize_t proc_write(
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

static const struct proc_ops ihs_proc_ops = {
    .proc_write = proc_write,
};

static int tcp_server_thread(void *data)
{
    int ret;
    struct kvec vec; 
    struct msghdr msg = {0}; 
    char buffer[128]; 

    vec.iov_base = buffer;
    vec.iov_len  = sizeof(buffer) - 1;

    printk(KERN_INFO "server_socket = %px\n", server_socket);

    if (!server_socket) {
        printk(KERN_ERR "server_socket é NULL!\n");
        return -EINVAL;
    }

    while (!kthread_should_stop())
    {
        ret = kernel_accept(server_socket, &client_socket, 0);

        if (ret) {
            if (kthread_should_stop() || ret == -EINTR || ret == -EBADF)
                break;
            continue;
        }

        bool autenticado = false;
        printk(KERN_INFO "Cliente conectado!\n");

        while (!kthread_should_stop())
        {
            ret = kernel_recvmsg(
                    client_socket,
                    &msg,
                    &vec,
                    1,
                    sizeof(buffer)-1,
                    0);

            if (ret <= 0) {
                printk(KERN_INFO "Cliente desconectou\n\n");
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

                if (!autenticado)
                {
                    if (strcmp(cmd, "AUTH senha") == 0)
                    {
                        autenticado = true;
                        printk(KERN_INFO "Cliente autenticado!\n");
                    }
                    else
                    {
                        printk(KERN_INFO "%s\n", cmd);
                        printk(KERN_WARNING "Cliente rejeitado!\n");
                        goto desconectar;
                    }
                    continue;
                }

                printk(KERN_INFO "%s\n", cmd);
                parser_command(cmd);
            }
        }

    desconectar:
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
    int ret = 0;
    struct pci_dev *pdev = NULL;
    printk(KERN_INFO "IHS: Modulo carregado!\n");

    // Busca GPU Intel (DRM)
    while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, pdev))) {
        if (pdev->vendor == PCI_VENDOR_ID_INTEL) {
            global_drm_dev = dev_get_drvdata(&pdev->dev);
            break;
        }
    }
    if (!global_drm_dev) {
        printk(KERN_ERR "IHS: GPU Intel compativel nao encontrada.\n");
        return -ENODEV;
    }

    //------------------Cria o Mouse Virtual---------------------
    mouse_dev = input_allocate_device();
    if (!mouse_dev)
    {
        printk(KERN_ERR "Falha ao criar dispositivo\n");
        return -ENOMEM;
    }

    mouse_dev->name = "IHS Virtual Mouse";
    set_bit(REL_WHEEL, mouse_dev->relbit);
    set_bit(EV_REL, mouse_dev->evbit); 
    set_bit(REL_X, mouse_dev->relbit); 
    set_bit(REL_Y, mouse_dev->relbit); 

    set_bit(EV_KEY, mouse_dev->evbit); 
    set_bit(BTN_LEFT, mouse_dev->keybit); 
    set_bit(BTN_RIGHT, mouse_dev->keybit); 
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

    //---------------------Cria o Teclado Virtual--------------------------
    keyboard_dev = input_allocate_device();
    if (!keyboard_dev)
    {
        printk(KERN_ERR "Falha ao criar dispositivo\n");
        return -ENOMEM;
    }

    keyboard_dev->name = "IHS Virtual Keyboard";
    set_bit(EV_KEY, keyboard_dev->evbit); 

    for(int i = 0; i < KEY_MAX; i++) 
        __set_bit(i, keyboard_dev->keybit); 

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
    printk(KERN_INFO "Sucesso ao Criar Teclado\n");
    msleep(200);

    //---------------Cria o Socket TCP(Vídeo)----------------
    ret = criar_servidor_tcp(&video_server_socket, 6666);
    if (ret) return ret;

    video_thread = kthread_run(video_stream_thread, NULL, "ihs_vid_thread");
    if (IS_ERR(video_thread)) {
        printk(KERN_ERR "IHS: Falha ao criar thread de video.\n");
        return PTR_ERR(video_thread);
    }

    //----------------------Cria o Proc-----------------------
    proc_entry = proc_create("ihs_remote", 0666, NULL, &ihs_proc_ops); 
    if (!proc_entry)
    {
        printk(KERN_ERR "Falha ao criar /proc/ihs_remote\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "IHS: /proc/ihs_remote criado\n");

    //----------------------Cria o Socket de Controle-----------------------
    ret = sock_create_kern(
        &init_net, 
        AF_INET, 
        SOCK_STREAM, 
        IPPROTO_TCP, 
        &server_socket 
    );
    if(ret)
    {
        printk(KERN_ERR "Erro ao criar socket\n");
        return ret;
    }
    printk(KERN_INFO "Socket TCP criado com sucesso\n"); 

    int reuse = 1;
    ret = sock_setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, KERNEL_SOCKPTR(&reuse), sizeof(reuse));
    if (ret < 0) {
        printk(KERN_ERR "Erro ao definir SO_REUSEADDR: %d\n", ret);
    }

    msleep(50);
    //------------------Atribui Endereço ao Socket de Controle---------------
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    addr.sin_port = htons(5558); 
    msleep(50);
    ret = kernel_bind( 
        server_socket,
        (struct sockaddr *)&addr,
        sizeof(addr)
    );
    msleep(50);
    if(ret)
    {
        printk(KERN_ERR "Erro no bind: %d\n", ret);
        sock_release(server_socket); 
        server_socket = NULL;
        return ret;
    }

    printk(KERN_INFO "Bind realizado com sucesso\n");    
    
    //--------------Faz o Socket de Controle Aceitar Conexões----------------
    msleep(50);
    ret = kernel_listen(server_socket, 5); 
    msleep(50);
    if(ret)
    {
        sock_release(server_socket);
        server_socket = NULL;
        printk(KERN_ERR "Erro no listen\n");
        return ret;
    }

    printk(KERN_INFO "Listen realizado com sucesso\n");

    //------------------Inicia Thread de Controle-------------------------
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
    printk(KERN_INFO "IHS: Modulo removido!\n"); 
    
    // 1. Fecha os sockets dos servidores PRIMEIRO. Isso aborta os kernel_accept bloqueantes.
    if (video_server_socket) {
        sock_release(video_server_socket);
        video_server_socket = NULL;
    }
    if(server_socket)
    {
        sock_release(server_socket);
        server_socket = NULL;
    }

    // 2. Com as chamadas acordadas, as kthreads saem do accept com erro e podem ser paradas.
    if(tcp_thread)
    {
        kthread_stop(tcp_thread);
        tcp_thread = NULL;
    }
    if (video_thread) {
        kthread_stop(video_thread);
        video_thread = NULL;
    }

    // 3. Limpa os sockets de cliente conectados remanescentes
    if(client_socket)
    {
        sock_release(client_socket);
        client_socket = NULL;
    }
    if (video_client_socket) {
        sock_release(video_client_socket);
        video_client_socket = NULL;
    }

    if(proc_entry)
        proc_remove(proc_entry);

    if(mouse_dev)
        input_unregister_device(mouse_dev);

    if(keyboard_dev)
        input_unregister_device(keyboard_dev);
}

module_init(create_devices); 
module_exit(destroy_devices);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seu Nome");
MODULE_DESCRIPTION("Primeiro modulo Linux");
