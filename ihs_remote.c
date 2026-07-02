#include <linux/module.h> //definições de módulos
#include <linux/kernel.h> //funções do kernel
#include <linux/init.h> // __init e __exit
#include <linux/input.h>
#include <linux/delay.h>

struct input_dev *mouse_dev;


static int __init hello_init(void) //static faz a memória se manter durante todo o programa
{
    int ret = 0;
    printk(KERN_INFO "IHS: Modulo carregado!\n"); //Vai printar quando  módulo for carregado no log do kernel
    printk(KERN_INFO "2 mensagens!\n"); //Vai printar quando  módulo for carregado no log do kernel


    mouse_dev = input_allocate_device();

    if (!mouse_dev)
    {
        printk(KERN_ERR "Falha ao criar dispositivo\n");
        return -ENOMEM;
    }

    mouse_dev->name = "IHS Virtual Mouse";

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

    int i;
    msleep(2000);
  for (i = 0; i < 20; i++)
{
    input_report_rel(mouse_dev, REL_X, 200);
    input_report_rel(mouse_dev, REL_Y, 100);
    input_sync(mouse_dev);

    msleep(500);
}

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "IHS: Modulo removido!\n"); //Vai printar quando o módulo for removido no log do kernel
    input_unregister_device(mouse_dev);
}

module_init(hello_init); //Quando carregar o módulo, chame i=__init
module_exit(hello_exit); //Quando remover, chame __exit

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Seu Nome");
MODULE_DESCRIPTION("Primeiro modulo Linux");