//include para adição das funções basicas 

#include "pico/stdlib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


//include para adição das funcoes relacionadas a conecxao a internet 
#include "client.h"


//incluir funcionalidades necessarias para o uso do display ssd1306

#include "ssd1306.h"
#include "ssd1306_i2c.h"


//include's para adição das funcionalidades presents na biblioteca hardware e funcionamento do display, joystick e temporzador

#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"



//definição dos pinos usados para o Joystick

#define ADC_Y 26
#define ADC_X 27


//variaveis do uso do display ssd1306

const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

ssd1306_t ssd;

/*definição de variaveis de largura e altura e tamanho de buffer
utlilizados para o funcionamento do display */
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_BUFFER_SIZE ((SSD1306_WIDTH * SSD1306_HEIGHT) / 8)


//variaveis para o controle do menu

int opcao_selecionada = 0;
int opcao_selecionada_1 = 0;

#define OPCOES 2
#define OPCOES_1 4


//variaveis necessarias para o uso de botoes 

#define BTN_A_PIN 5
#define BTN_B_PIN 6

bool estado_A = false;
bool estado_B = false;


//variavel usada no uso do temporizador

uint32_t ultima_vez_desconectado = 0;
uint32_t tempo_corrido = 0;


//definição das variaveis de entrada e saída
int entrada_1 = 0; 
int entrada_2 = 0;
int entrada_3 = 0;
int entrada_4 = 0;
int saida_1 = 0;
int saida_2 = 0;
int saida_3 = 0;
int saida_4 = 0;

//definicao das field's
const int field_1 = 1;
const int field_2 = 2;
const int field_3 = 3;
const int field_4 = 4;
const int field_5 = 5;
const int field_6 = 6;
const int field_7 = 7;
const int field_8 = 8;

//variaveis correspondentes ao id de usuario:

int id_1 = 1;
int id_2 = 2;
int id_3 = 3;
int id_4 = 4;



struct render_area frame_area = {
    .start_column = 0,
    .end_column = SSD1306_WIDTH - 1,
    .start_page = 0,
    .end_page = SSD1306_HEIGHT / 8 -1   
};



/*função que faz a requisição, ela é responsavel por "mandar" os dados para o ThingSpeak, ele funciona mandando 
uma requisição HTML do tipo GET que é disponibilizada pelo o ThingSpeak, minha API Write: GET https://api.thingspeak.com/update?api_key=QMOVRT1DELYENZM5&field1=0*/

void requisicao(const int field, int valor){
    ip_addr_t server_ip;
    resolve_name("api.thingspeak.com", &server_ip);
    printf("IP resolvido: %s\n", ipaddr_ntoa(&server_ip));
    struct tcp_pcb *pcb = tcp_new();
    client_create(pcb, &server_ip, 80);
    char request[256];
    snprintf(request, sizeof(request), 
    "GET /update.json?api_key=QMOVRT1DELYENZM5&field%d=%d HTTP/1.1\r\n"
    "Host: api.thingspeak.com\r\n"
    "Connection: close\r\n\r\n",
    field,valor);
    client_write(pcb, request);
}



/*Função usada para escrever texto no display*/

void EscreverTexto (const char *text[], int lines){
    uint8_t ssd_buffer[SSD1306_BUFFER_SIZE];
    memset (ssd_buffer, 0, SSD1306_BUFFER_SIZE);
  
    int y = 0;
    for (int i = 0; i < lines; i++){
      ssd1306_draw_string(ssd_buffer, 0, y, text[i]);
      y += 8;
    }
    render_on_display(ssd_buffer, &frame_area);
}



/*Funçoes usadas para verificar se os botoes A e B estao pressionados*/

void verifica_A(){
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    estado_A = !gpio_get(BTN_A_PIN);
}
void verifica_B(){
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
    estado_B = !gpio_get(BTN_B_PIN);
}


/*função responsavel por informar que está a realizando a 
requisição por http e avisar que os dados foram enviados*/

void envio(const int field, int entrada){
    requisicao(field, entrada);
    verifica_B();
    const char *text[] = {
    "ENVIANDO DADOS",
    "PARA O SERVIDOR"
    };
    EscreverTexto(text, sizeof (text) / sizeof (text[0]));
    sleep_ms(15000);
    do{
        verifica_B();
        const char *text[] = {
            "DADOS ENVIADOS",
            "PARA O SERVIDOR",
            "",
            "",
            "USE B PARA VOLTAR"
        };
        EscreverTexto(text, sizeof (text) / sizeof (text[0]));
    }while(estado_B != true);
    sleep_ms(200);
}



//função usada para não poluir e otimizar o codigo na escrita do id na tela

void escrever_id(char id){
    char id_str[1];
    sprintf(id_str, "%d", id);
    const char *text[] = {
        "SEU ID",
        id_str,
        "",
        "NAVEGUE COM",
        "EIXO Y",
        "DO JOYSTIK",
        "A PARA SELECIONAR",
        "B PARA VOLTAR"
    };
    EscreverTexto(text, sizeof (text) / sizeof (text[0]));
}





int main() {
    stdio_init_all();
    //inicialização do display

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    calculate_render_area_buffer_length(&frame_area);
    ssd1306_init(&ssd,SSD1306_WIDTH, SSD1306_HEIGHT, 0x3C, i2c1);



    //inicialização do joystick

    adc_init();
    adc_gpio_init(ADC_Y); 
    adc_gpio_init (ADC_X);

    //variaveis do joystick 
    uint16_t adc_y_raw, last_adc_y_raw = 2048; 


    /*while de reconexão a internet e varivel sendo guardada para ultima vez desconectado desde o boot, o sistema
    não explorou tanto essa variavel, devido a falta de utilização, mas em caso 
    de consulta ela pode ser utilizada, em caso de queda durante o laço principal o sistema deve ser reiniciado manualmente*/

    while (!connect_wifi("COSTA 2G", "segredo01")) {
        printf("Tentando novamente...\n");
        const char *text[] = {
            "CONECTANDO",
            "COM O WIFI"
        };
            EscreverTexto(text, sizeof(text) / sizeof (text[0]));
        tempo_corrido = to_ms_since_boot(get_absolute_time());
        if(tempo_corrido - ultima_vez_desconectado >= 1000){
            ultima_vez_desconectado = tempo_corrido;
        }
    }

    while (true) {
        //Parte do codigo destinado a mapeamento do joystick e escolha da opção

        adc_select_input(0); 
        adc_y_raw = adc_read();
        if (adc_y_raw > 3000 && last_adc_y_raw <= 3000) { 
            opcao_selecionada = (opcao_selecionada - 1 + OPCOES) % OPCOES;
        }
        if (adc_y_raw < 1000 && last_adc_y_raw >= 1000) { 
            opcao_selecionada = (opcao_selecionada + 1) % OPCOES;
        }
          
        last_adc_y_raw = adc_y_raw; 

        //menu de entrada e saida

        if(opcao_selecionada == 0){ //entrada
            verifica_A(); //função que verifica se o botão A está sendo pressionado
            sleep_ms(100);
            if (estado_A == true){ //ao ser pressionado o sistema vai entender que o usuario que marcar uma entrada
                do{
                    //novamente outro menu, que por sua vez é o de id
                    verifica_B(); // função que verifica se o botão B está sendo pressionado
                    adc_y_raw = adc_read();
                    if (adc_y_raw > 3000 && last_adc_y_raw <= 3000) { 
                        opcao_selecionada_1 = (opcao_selecionada_1 - 1 + OPCOES_1) % OPCOES_1;
                    }
                    if (adc_y_raw < 1000 && last_adc_y_raw >= 1000) { 
                        opcao_selecionada_1 = (opcao_selecionada_1 + 1) % OPCOES_1;
                    }
                    last_adc_y_raw = adc_y_raw; 

                    if(opcao_selecionada_1 == 0){ 
                        /*esse processo é repetido 8 vezes, 4 para entradas e 4 para saidas*/
                        escrever_id(id_1); // escreve o id 1 na tela
                        sleep_ms(200); // espera para nao haver um duplo clique ao apertar em marcar entrada
                        verifica_A(); // volta a verificar o estado de A
                        if(estado_A == true){ //quando for apertado ele envia uma requsição para o site do ThingSpeak
                            entrada_1 += 1;
                            envio(field_1, entrada_1); /*envia a entrada de acodo com a field e 
                            se pressionado o botao B ele volta para a parte inicial 
                            de escolha de entrada e saida*/
                        }
                    }else if(opcao_selecionada_1 == 1){
                        escrever_id(id_2);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            entrada_2 += 1;
                            envio(field_3, entrada_2);
                        }
                        
                    }else if(opcao_selecionada_1 == 2){
                        escrever_id(id_3);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            entrada_3 += 1;
                            envio(field_5, entrada_3);
                            
                        }
                    }else if(opcao_selecionada_1 == 3){
                        escrever_id(id_4);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            entrada_4 += 1;
                            envio(field_7, entrada_4);
                        }
                    }
                }while(estado_B != true); // O laço continua até o funcionario apertar B
            }else{
                /*se não foi pressionado é so apresentado marcar 
                entrada, até usuario mover o eixo y do joystick ou apertar A*/
                const char *text[] = {
                "MARCAR ENTRADA",
                "",
                "SAIDA",
                "",
                "",
                "NAVEGUE COM",
                "O JOYSTICK E",
                "SELECIONE COM A"
                };
                EscreverTexto(text, sizeof(text) / sizeof (text[0]));
            }
        }else if(opcao_selecionada == 1){ //saída e tudo é replicado, mas muda as variaveis
            verifica_A();
            sleep_ms(100);
            if (estado_A == true){
                do{
                    verifica_B();
                    adc_y_raw = adc_read();
                    if (adc_y_raw > 3000 && last_adc_y_raw <= 3000) { 
                        opcao_selecionada_1 = (opcao_selecionada_1 - 1 + OPCOES_1) % OPCOES_1;
                    }
                    if (adc_y_raw < 1000 && last_adc_y_raw >= 1000) { 
                        opcao_selecionada_1 = (opcao_selecionada_1 + 1) % OPCOES_1;
                    }
                    last_adc_y_raw = adc_y_raw; 
                    if(opcao_selecionada_1 == 0){
                        escrever_id(id_1);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            saida_1 += 1;
                            envio(field_2, saida_1);
                        }
                    }else if(opcao_selecionada_1 == 1){
                        escrever_id(id_2);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            saida_2 += 1;
                            envio(field_4, saida_2);
                        }
                    }else if(opcao_selecionada_1 == 2){
                        escrever_id(id_3);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            saida_3 += 1;
                            envio(field_6, saida_3);
                        }
                    }else if(opcao_selecionada_1 == 3){
                        escrever_id(id_4);
                        sleep_ms(200);
                        verifica_A();
                        if(estado_A == true){
                            saida_4 += 1;
                            envio(field_8, saida_4);
                        }
                    }
                }while(estado_B != true);
            }else{
                const char *text[] = {
                "ENTRADA",
                "",
                "MARCAR SAIDA",
                "",
                "",
                "NAVEGUE COM",
                "O JOYSTICK E",
                "SELECIONE COM A"
                };
                EscreverTexto(text, sizeof(text) / sizeof (text[0]));
            }
        }
    }

    return 0;
}